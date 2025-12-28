from sklearn.neighbors import kneighbors_graph, NearestNeighbors
from collections import defaultdict
import numpy as np
import subprocess
import os
from pathlib import Path

def create_knn_graph(X, k=10, **nn_kwargs):
    nn = NearestNeighbors(n_neighbors=k+1, **nn_kwargs)
    nn.fit(X)
    # indices shape: (n_samples, k+1)
    # when querying on the training data, the first neighbor is always itself
    indices = nn.kneighbors(X, return_distance=False)

    # drop the first index (self) 
    neighbors = [row[1:] for row in indices]

    neighbors = [list(row) for row in neighbors]

    return neighbors


""" TRANSFORM THE KNN GRAPH TO UNDIRECTED GRAPH """
def to_undirected_weighted(neighbors):
    weights = defaultdict(int)
    n = len(neighbors)
    for u in range(n):
        for v in neighbors[u]:
            if u == v:
                continue
            a, b = sorted((u, v))
            weights[(a, b)] += 1
    # convert to list of (u, v, w)
    edges = [(a, b, 2 if c == 2 else 1) for (a, b), c in weights.items()]
    return edges


""" PREPARE THE UNDIRECTED GRAPH TO BE USED BY KAHIP ALGORITHM """
def ugraph_to_csr(num_nodes, edges):
    graph = [[] for _ in range(num_nodes)]
    for u, v, w in edges:
        graph[u].append((v, w))
        graph[v].append((u, w))
    vwgt = np.ones(num_nodes, dtype=np.int32)
    xadj = [0]
    adjncy = []
    adjcwgt = []
    cnt = 0
    for u in range(num_nodes):
        for v, w in graph[u]:
            adjncy.append(v)
            adjcwgt.append(w)
        cnt += len(graph[u])
        xadj.append(cnt)
    
    xadj, adjncy, adjcwgt = np.array(xadj, np.int32), np.array(adjncy, np.int32), np.array(adjcwgt, np.int32)

    return vwgt, xadj, adjncy, adjcwgt

