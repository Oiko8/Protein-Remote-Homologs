import argparse

def parse_arguments_embed():
    parser = argparse.ArgumentParser(description="nlsh_build arguments")

    # required 
    parser.add_argument("-i", "--proteins_path", required=True, type=str, help="Input file for the proteins")
    # parser.add_argument("-o", "--embeddings", required=True, type=str, help="Output file with the generated embeddings from the proteins")

    parser.add_argument("-model", type=str, help="ESM2 model to be used")

    return parser.parse_args()



def parse_arguments_search():
    parser = argparse.ArgumentParser(description="nlsh_search arguments")

    return parser.parse_args()