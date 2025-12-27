import torch 
import esm
from utils import args_parser
import numpy as np
import pathlib


def read_proteins(filename):
    with open(filename) as f:
        header = None
        sequence = []

        for line in f:
            line = line.strip()

            if not line:
                continue  # skip empty lines

            if line.startswith(">"):
                # If we already have a previous sequence, yield it
                if header and sequence:
                    yield header, "".join(sequence)
                
                # Start a new record
                header = line[1:]      # remove '>'
                sequence = []
            else:
                sequence.append(line)

        # yield the last record
        if header and sequence:
            yield header, "".join(sequence)

def write_fvec(f, vec):
    """
    Write vector in fvecs style: int32 d + float32[d].
    Similar style to SIFT vectors
    """
    if isinstance(vec, torch.Tensor):
        vec = vec.detach().cpu().numpy()
    vec = np.asarray(vec, dtype=np.float32).ravel()
    d = np.int32(vec.shape[0])
    np.array([d], dtype=np.int32).tofile(f)
    vec.tofile(f)


def proteins_to_vectors(args):
    base = args.proteins_path     # the path to the proteins base
    out_embeds = args.out_embeds  # the grenerated embeddings 
    out_ids = args.out_ids        # mapping between indices and proteins


    # 1. load model
    model, alphabet = esm.pretrained.esm2_t6_8M_UR50D()
    batch_converter = alphabet.get_batch_converter()

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    # Move model to device
    model = model.to(device)
    model.eval()

    pad = alphabet.padding_idx

    # 2. load proteins from swissprot.fasta
    i = 1
    with open(out_embeds, "wb") as file_out, open(out_ids, "w", encoding="utf-8") as file_ids:
        for protein_header, sequence in read_proteins(base):
            # print("ID:", protein_header)
            # print("Sequence length:", len(sequence))
            # print(f"Process protein{i}...")
            if len(sequence) > 1022 :
                sequence = sequence[:1022]

            data = [(protein_header, sequence)]
            _, _, tokens = batch_converter(data)
            
            # TESTING: the "batch_converter" adds 2 tokens at the start and the end of the tokens sequence.
            # print(tokens.shape)
            # print(len(sequence))
            # break

            tokens = tokens.to(device)

            # 3. forward pass from the model to get the vector
            with torch.no_grad():
                results = model(tokens, repr_layers=[6])
            
            # 4. Mean pooling
            token_embeddings = results["representations"][6]
            embedding = token_embeddings[0, 1 : len(sequence) + 1].mean(dim=0)   # get rid of [BOS] and [EOS] tokens. 
            # print("Extract embedding...")
            file_ids.write(protein_header + "\n")
            write_fvec(file_out, embedding)
            
            if i%100 == 0:
                print(f"Processed protein {i}...")
            i+=1




def main():
    args = args_parser.parse_arguments_embed()

    proteins_to_vectors(args)

    print(f"[OK] wrote embeddings to: {args.out_embeds}")
    print(f"[OK] wrote ids to: {args.out_ids}")
 
if __name__ == "__main__":
    main()