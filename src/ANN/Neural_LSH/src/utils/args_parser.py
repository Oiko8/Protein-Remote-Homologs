import argparse

def parse_arguments_build():
    parser = argparse.ArgumentParser(description="nlsh_build arguments")

    # required 
    parser.add_argument("-d", "--data", required=True, type=str, help="Input dataset file (e.g., input.dat)")
    parser.add_argument("-i", "--index_path", required=True, type=str, help="Output index path (directory or prefix)")
    parser.add_argument("-type", required=True, choices=["sift", "mnist", "protein"], help="Dataset type")
    
    # optional 
    parser.add_argument("--knn", type=int, default=10, help="Number of neighbors k for k-NN graph")
    parser.add_argument("-m", type=int, default=100, help="Number of KaHIP partitions")
    parser.add_argument("--imbalance", type=float, default=0.03, help="KaHIP imbalance factor ε")
    parser.add_argument("--kahip_mode", type=int, default=0, help="KaHIP mode: 0=FAST, 1=ECO, 2=STRONG")

    parser.add_argument("--layers", type=int, default=3, help="Number of MLP layers")
    parser.add_argument("--nodes", type=int, default=512, help="Hidden layer width")
    parser.add_argument("--epochs", type=int, default=50, help="Training epochs")
    parser.add_argument("--batch_size", type=int, default=128, help="Batch size")
    parser.add_argument("--lr", type=float, default=0.001, help="Learning rate")
    parser.add_argument("--seed", type=int, default=1, help="Random seed")

    parser.add_argument("-lsh_build", type=int, choices=[0,1], default=1, help="how accurate is the lsh algorithm")
    parser.add_argument("--load_labels", type=str, default="", help="load the presaved label to not run k-nn graph algorithm")

    return parser.parse_args()



def parse_arguments_search():
    parser = argparse.ArgumentParser(description="nlsh_search arguments")

    # required 
    parser.add_argument("-d", "--data", required=True, type=str, help="Input dataset file (e.g., input.dat)")
    parser.add_argument("-q", "--query", required=True, type=str, help="query dataset file (e.g., query.dat)")
    parser.add_argument("-i", "--index_path", required=True, type=str, help="Created index path (directory or prefix)")

    parser.add_argument("-type", required=True, choices=["sift", "mnist", "protein"], help="Dataset type")
    
    # optional 
    parser.add_argument("-N", "--k", type=int, default=1, help="Number of nearest neighbors")
    parser.add_argument("-R", "--range", type=float, default=2.0, help="Distance for range search")
    parser.add_argument("-T", "--bins", type=int, default=5, help="Number of bins to check")

    # extra
    parser.add_argument("-subset", "--set", type=int, default=None, help="Sets a subset of queries for searching")

    return parser.parse_args()