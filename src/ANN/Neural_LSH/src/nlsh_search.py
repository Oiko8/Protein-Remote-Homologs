import numpy as np
import torch
import torch.nn as nn
import torch.nn.functional as F
from mlp_model import MLP_net
from utils import load_files
from utils.args_parser import parse_arguments_search
from time import time_ns
import os
from pathlib import Path


MODEL = "model/model_protein.pth"
INDEX_PATH = "index_ivf_mnist.txt"
DATA_PATH = "data/MNIST/input.dat"
QUERIES_PATH = "data/MNIST/query.dat"

""" function to load the model """
def load_model(model_path, device="cpu"):
    base_dir = Path(__file__).resolve().parent.parent
    path = base_dir / model_path
    model = torch.load(path, map_location=device, weights_only=False)
    model.eval()
    return model

""" function to load the inverted file """
def load_ivf_index(index_file):
    inv_file = {}
    with open(index_file, 'r') as file:
        for line in file:
            line = line.strip()
            if not line:
                continue

            label_raw, ids_raw = line.split(":")
            label = int(label_raw)
            ids = ids_raw.strip()
            if ids == "":
                inv_file[label] = []
            else:
                inv_file[label] = list(map(int, ids.split()))

    return inv_file


"""
Returns the labels with the most probabilities
@Args:
    model
    queries: an np.array of the queries
    L: number of top labels we want to find
"""
def predict_top_labels(model, query, T, device='cpu'):
    with torch.no_grad():
        query_t = torch.from_numpy(query).float().to(device)
        logs = model(query_t.unsqueeze(0))  # squeeze queries to dimension 1*m
        probs = F.softmax(logs, dim=1).squeeze(0)
        probs_nparray = probs.cpu().numpy()

    top_labels = np.argsort(-probs_nparray)[:T]  # sort in descending order, keep L first
    top_probs = probs_nparray[top_labels]

    return top_labels, top_probs, probs_nparray


"""
Gather the candidates from the top labels bins
NOTE: keep only the unique ones
"""

def gather_candidates(inv_file, top_labels):
    candidates = []
    for r in top_labels:
        candidates.extend(inv_file.get(int(r), []))

    candidates = np.unique(np.array(candidates, dtype=np.int64))
    return candidates

"""
Search in the candidates for the K nearest neighbors
"""
def knn_search(dataset, query, cand_ids, k):
    if len(cand_ids) == 0:
        return np.array([], dtype=np.int64), np.array([], dtype=np.float32)

    C = dataset[cand_ids]
    diffs = C - query[None, :]
    dists = np.sum(diffs * diffs, axis=1)  # squared L2

    if len(dists) > k:
        idx = np.argpartition(dists, kth=k-1)[:k]
        order = idx[np.argsort(dists[idx])]
    else:
        order = np.argsort(dists)

    nn_ids = cand_ids[order]
    nn_dists = np.sqrt(dists[order])  # true L2
    return nn_ids, nn_dists


def neural_search(data, q, model, inv_file, T=5, k=5, device="cpu"):
    top_labels, _, _ = predict_top_labels(model, q, T=T)
    # print(top_labels)
    candidates = gather_candidates(inv_file=inv_file, top_labels=top_labels)
    nn_ids, nn_dists = knn_search(dataset=data, query=q, cand_ids=candidates, k=k)
    
    return nn_ids, nn_dists

def exhaustive_search(data, q, k):
    C = data 
    diffs = C - q[None, :]
    dists = np.linalg.norm(diffs, axis=1)  
    order = np.argsort(dists)[:k]
    nn_ids = order
    nn_dists = dists[order]
    return nn_ids, nn_dists

def range_seach(q, R, data, inv_file, model, T, device="cpu"):
    top_labels, _, _ = predict_top_labels(model, q, T=T, device=device)
    candidates = gather_candidates(inv_file, top_labels)
    if len(candidates) == 0:
        return np.array([], dtype=np.int64)

    ids_in_range = []
    q_vec = np.asarray(q, dtype=np.float32)

    for d in candidates:
        x = data[d]                
        diff = x - q_vec
        dist = np.linalg.norm(diff)
        if dist <= R:
            ids_in_range.append(d)  

    return ids_in_range


def main():
    args = parse_arguments_search()

    data = load_files.load_fvecs_data(path=args.data)
    queries = load_files.load_fvecs_data(path=args.query)
    model = load_model(MODEL)
    
    inv_file = load_ivf_index(args.index_path)
    number_of_nn = args.k
    range_on = False
    # open output file for the results
    out = open(args.output, "w")

    # preparing the metrics
    sum_appox_time = 0
    sum_true_time = 0
    sum_recall = 0
    sum_AF = 0
    qcount = 0

    # print("[Neural LSH]\n\n")
    out.write("[Neural LSH]\n\n")
    for qi, q in enumerate(queries[:args.set]):
        
        #### approx search ####
        start = time_ns()
        approx_ids, approx_dists = neural_search(data, q, model, inv_file, T=args.bins, k=number_of_nn)
        end = time_ns()
        approx_search_time = (end-start)/1000000


        #### search with brute force ####
        start = time_ns()
        true_ids, true_dists = exhaustive_search(data, q, k=number_of_nn)
        end = time_ns()
        true_search_time = (end-start)/1000000

        #### range search ####
        if range_on:
            ids_in_range = range_seach(q=q, inv_file=inv_file, data=data, R=args.range, model=model, T=args.bins)
        else:
            ids_in_range = []

        
        #################
        #### metrics ####
        #################

        # recall
        hits_at_N = 0
        for i in range(number_of_nn):
            if (approx_ids[i] == true_ids[i]):
                hits_at_N += 1

        sum_recall += hits_at_N/number_of_nn
        
        # approximation fraction
        AF = approx_dists[0]/true_dists[0]
        sum_AF += AF

        # total time (approx/true search)
        sum_appox_time += approx_search_time
        sum_true_time += true_search_time
        qcount += 1
        

        ########################
        ####### results ########
        ########################
        out.write(f"Query: {qi+1}\n")

        for i in range(number_of_nn):
            out.write(f"Nearest neighbor-{i+1}: {approx_ids[i]}\n")
            out.write(f"DistanceApproximate: {approx_dists[i]:.3f}\n")
            out.write(f"DistanceTrue: {true_dists[i]:.3f}\n")
        out.write("R-near neighbors:\n")
        for id in ids_in_range:
            out.write(f"{id}\n")
     
        out.write("\n")

    
    ##########################
    ######## metrics #########
    ##########################
    avg_approx_time =  sum_appox_time/qcount if (qcount>0) else 0.0
    avg_true_time = sum_true_time/qcount if (qcount>0) else 0.0
    avg_AF = sum_AF/qcount if (qcount>0) else 0.0
    qps = qcount/(sum_appox_time/1000) if (sum_appox_time>0) else 0
    recall = sum_recall/qcount if (qcount>0) else 0.0

    print(f"Average AF: {avg_AF:.2f}")
    print(f"Recall@N: {recall:.2f}")
    print(f"QPS: {qps:.2f}")
    print(f"tApproximateAverage: {avg_approx_time:.2f}")
    print(f"tTrueAverage: {avg_true_time:.2f}")
    out.write(f"Average AF: {avg_AF:.2f}\n")
    out.write(f"Recall@N: {recall:.2f}\n")
    out.write(f"QPS: {qps:.2f}\n")
    out.write(f"tApproximateAverage: {avg_approx_time:.2f}\n")
    out.write(f"tTrueAverage: {avg_true_time:.2f}\n")

    # close the open output file
    out.close()
    

if __name__ == "__main__":
    main()
