import numpy as np

def load_fvecs(path):
    path = str(path)
    a = np.fromfile(path, dtype=np.float32)
    if a.size == 0:
        return np.empty((0, 0), dtype=np.float32)
    dim = int(a[0])
    a = a.reshape(-1, dim + 1)
    return a[:, 1:].copy()

def add_l2_distances(neighbors_idx, data_path, query_path):
    X = load_fvecs(data_path)
    Q = load_fvecs(query_path)

    out = []
    for qi, lst in enumerate(neighbors_idx):
        q = Q[qi]
        row = []
        for idx in lst:
            v = X[idx]
            dist = float(np.linalg.norm(v - q))
            row.append((idx, dist))
        out.append(row)
    return out
