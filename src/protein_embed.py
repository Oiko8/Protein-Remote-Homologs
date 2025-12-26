import torch 
import esm
from utils import args_parser


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


def proteins_to_vectors(args):
    base = args.proteins_path
    embeds = []

    # 1. load model
    model, alphabet = esm.pretrained.esm2_t6_8M_UR50D()
    batch_converter = alphabet.get_batch_converter()

    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
    # Move model to device
    model = model.to(device)
    model.eval()

    # 2. load proteins from swissprot.fasta
    i = 1
    for protein_header, sequence in read_proteins(base):
        # print("ID:", protein_header)
        # print("Sequence length:", len(sequence))
        print(f"Process protein{i}...")
        if len(sequence) > 1022 :
            sequence = sequence[:1022]

        data = [(protein_header, sequence)]
        labels, strs, tokens = batch_converter(data)

        tokens = tokens.to(device)

        # 3. forward pass from the model to get the vector
        with torch.no_grad():
            results = model(tokens, repr_layers=[6])
        
        # 4. Mean pooling
        token_embeddings = results["representations"][6]
        embedding = token_embeddings.mean(dim=1)
        embeds.append(embedding)
        print("Extract embedding...")
        i+=1
    
    return embeds




def main():
    args = args_parser.parse_arguments_embed()

    embeddings = proteins_to_vectors(args)

    for i, embed in enumerate(embeddings[:10]):
        print(f"Protein{i}: {embed}")
 
if __name__ == "__main__":
    main()