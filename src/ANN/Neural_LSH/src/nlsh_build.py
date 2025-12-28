import kahip 
from utils import graph_utils as utils
from utils import load_files, dataloaders
from mlp_model import MLP_net, training
import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from utils.args_parser import parse_arguments_build
import os

def data_clustering(data, args):

    # neighbors for the creation of the graph
    k = args.knn

    # kahip arguments
    nblocks = args.m
    imbalance = args.imbalance
    mode = args.kahip_mode
    seed = args.seed

    nodes = len(data)
    print("Building k-NN graph...")
    knn_graph = utils.create_knn_graph(X=data, k=k)

    print("Making graph undirected + weighted...")
    undirected_graph = utils.to_undirected_weighted(knn_graph)

    print("Building CSR from undirected graph for", nodes, "points...")
    vwgt, xadj, adjncy, adjcwgt = utils.ugraph_to_csr(nodes, undirected_graph)

    suppress_output = 0
    print("Ready to run kahip algorithm...")
    edgecut, blocks = kahip.kaffpa(
        vwgt,
        xadj,
        adjcwgt,
        adjncy,
        nblocks,
        imbalance,
        suppress_output,
        seed,
        mode,
    )

    print("Clustering finished...")
    labels = np.asarray(blocks, dtype=np.int64)
    return edgecut, labels

def save_ivf_index(labels, m, filename="index_file.txt"):
    """ creating an inverted file index """
    # labels = np.asarray(labels, dtype=np.int64)

    with open(filename, "w") as f:
        for r in range(m):
            # for every label we keep the indice of the point
            idx = np.where(labels == r)[0]
            idx_str = " ".join(map(str, idx))
            f.write(f"{r}: {idx_str}\n")


def train_model(data, labels, args, data_type):
    # creating the dataloaders
    batch_size = args.batch_size
    train_loader, valid_loader = dataloaders.create_loaders(data, labels, batch_size)

    # set to gpu if available
    device = torch.device("cuda" if torch.cuda.is_available() else "cpu")

    # arguments for the model
    layers = args.layers
    hidden_layer = args.nodes
    
    # define the model
    input_size = data.shape[1]
    output_size = int(labels.max()) + 1
    model = MLP_net(input_size=input_size,
                    output_size=output_size,
                    num_layers=layers,
                    hidden_layer=hidden_layer).to(device)

    # arguments for the training
    lr = args.lr
    epochs = args.epochs

    # define loss function and optimizer
    # choose cross entropy function as this function in Pytorch applies a softmax function to the output layer and calculates the log loss
    loss_function = nn.CrossEntropyLoss()

    # specify optimizer (stochastic gradient descent) and learning rate = 0.01
    # optimizer = torch.optim.SGD(model.parameters(), lr=lr, momentum=0.9)
    optimizer = torch.optim.Adam(model.parameters(), lr=lr, weight_decay=1e-4)

    # train the model
    training(model=model, loss_function=loss_function, optimizer=optimizer, train_loader=train_loader, valid_loader=valid_loader, epochs=epochs, data_type=data_type, device=device)

def main():
    args = parse_arguments_build()

    print("Loading datasets...")
    data = load_files.load_fvecs_data(args.data)


    if args.load_labels:
        print(f"Loading precomputed labels from {args.load_labels}...")
        labels = np.load(args.load_labels)
    else:
        _, labels = data_clustering(data=data, args=args)

        # save the labels for future use
        labels_dir = os.path.dirname(args.index_path) or "."
        labels_path = os.path.join(labels_dir, f"labels_{args.type}.npy")
        print(f"Saving labels to {labels_path}...")
        np.save(labels_path, labels)

        print("Saving the index file...")
        save_ivf_index(labels=labels, m=args.m, filename=args.index_path)
    

    train_model(data=data, labels=labels, args=args, data_type=args.type)


if __name__ == "__main__":
    main()