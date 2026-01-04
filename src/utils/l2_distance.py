import numpy as np


def load_fvecs(path):
    a = np.fromfile(path, dtype=np.int32)
    if a.size == 0:
        return np.empty((0, 0), dtype=np.float32)
    d = int(a[0])
    assert d > 0, f"Invalid dimension {d} read from {path}"
    a = a.reshape(-1, d + 1)
    X = a[:, 1:].view(np.float32)
    return X


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
