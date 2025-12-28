import numpy as np, struct

def load_fvecs_data(path):
    a = np.fromfile(path, dtype=np.int32)
    if a.size == 0:
        return np.empty((0, 0), dtype=np.float32)
    d = int(a[0])
    assert d > 0, f"Invalid dimension {d} read from {path}"
    a = a.reshape(-1, d + 1)
    X = a[:, 1:].view(np.float32)
    return X

