import argparse

def parse_arguments_embed():
    parser = argparse.ArgumentParser(description="nlsh_build arguments")

    # required 
    parser.add_argument("-i", "--proteins_path", required=True, type=str, help="Input file for the proteins")
    parser.add_argument("-o", "--out_embeds", required=True, type=str, help="Output file with the generated embeddings from the proteins")
    parser.add_argument("-out_ids", help="Output txt file for mapping index->protein ids")

    parser.add_argument("-model", type=str, help="ESM2 model to be used")

    return parser.parse_args()



def parse_arguments_search():
    parser = argparse.ArgumentParser(description="protein_search arguments")

    parser.add_argument("-d", "--data", required=True, type=str)
    parser.add_argument("-q", "--queries", required=True, type=str)
    parser.add_argument("-o", required=True, type=str)
    parser.add_argument("-N", required=True, type=int)
    parser.add_argument("-method", required=True, type=str,
                        choices=["lsh", "hypercube", "ivfflat", "ivfpq", "neural"])

    return parser.parse_args()