import subprocess
import os
from pathlib import Path

def run_make(build_dir, target="ivfflat"):
    """ run make file to create the executables """
    result = subprocess.run(
        ["make", target],
        cwd=build_dir,      # directory where Makefile is located
        capture_output=True,
        text=True
    )

    if result.returncode != 0:
        print("Make failed!")
        print("STDOUT:\n", result.stdout)
        print("STDERR:\n", result.stderr)
        raise RuntimeError("Compilation failed")


def IVFFlat(exe_path, data_path, query_path, k_neighbors=10,
            kclusters=64, nprobe=4):
    """
    Run IVFFlat executable and return neighbors[q] = list of indices.
    Assumes stdout format:
      Query: i
      Nearest neighbor-...: idx
    """
    exe_path = str(exe_path)
    data_path = str(data_path)
    query_path = str(query_path)

    cmd = [
        exe_path,
        "-d", data_path,
        "-q", query_path,
        "-kclusters", str(kclusters),
        "-nprobe", str(nprobe),
        "-N", str(k_neighbors),
    ]

    cwd = os.path.dirname(exe_path) or None

    # add more threads for faster results
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = "8"

    result = subprocess.run(
        cmd,
        cwd=cwd,
        capture_output=True,
        text=True,
        env=env
    )

    if result.returncode != 0:
        print("IVFFlat failed")
        print("STDOUT:\n", result.stdout)
        print("STDERR:\n", result.stderr)
        raise RuntimeError("IVFFlat failed")

    # Parse output
    neighbors = []
    current_q = None

    for line in result.stdout.splitlines():
        line = line.strip()
        if not line:
            continue

        if line.startswith("Query:"):
            q_idx = int(line.split()[1])
            while len(neighbors) <= q_idx:
                neighbors.append([])
            current_q = q_idx

        elif line.startswith("Nearest neighbor-") and current_q is not None:
            idx = int(line.split(":")[1].strip())
            neighbors[current_q].append(idx)

    return neighbors


def run_ivfflat(data_file="protein_vectors.dat", query_file="protein_queries.dat", knn=5):
    # Path to the executable
    base_dir = Path(__file__).resolve().parent.parent
    classic_ann_dir = base_dir / "ANN" / "Classic_ANN"

    exe_path = classic_ann_dir / "src" / "bin" / "IVFFlatMain"

    data_path  = base_dir.parent / "artifacts" / "embeddings" / data_file
    query_path = base_dir.parent / "artifacts" / "embeddings" / query_file

   
    run_make(classic_ann_dir / "src", target="ivfflat")

    neighbors = IVFFlat(exe_path=exe_path, query_path=query_path, data_path=data_path,
                        k_neighbors=knn, kclusters=128, nprobe=8)

    run_make(classic_ann_dir / "src", "clean")

    return neighbors

def main():
    run_ivfflat()


if __name__ == "__main__":
    main()
