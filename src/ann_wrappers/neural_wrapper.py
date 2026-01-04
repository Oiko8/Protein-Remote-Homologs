import subprocess
import os
import sys
from pathlib import Path


def nlsh_search(exe_python, nlsh_search_py, data_path, query_path, index_path,
                k_neighbors=5, T=10, data_type="protein",
                extra_args=None):
    """
    Run Neural LSH search script and return neighbors[q] = list of indices.

    """

    cmd = [
        str(exe_python),
        str(nlsh_search_py),
        "-d", str(data_path),
        "-q", str(query_path),
        "-i", str(index_path),
        "-N", str(k_neighbors),
        "-type", str(data_type),
        "-T", str(T),
    ]

    if extra_args:
        cmd += list(map(str, extra_args))

    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = "8"

    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        env=env
    )

    if result.returncode != 0:
        print("Neural LSH search failed!")
        print("CMD:\n", " ".join(cmd))
        print("STDOUT:\n", result.stdout)
        print("STDERR:\n", result.stderr)
        raise RuntimeError("Neural LSH search failed")

    # Parse output
    neighbors = []
    current_q = None

    for line in result.stdout.splitlines():
        line = line.strip()
        if not line:
            continue

        if line.startswith("Query:"):
            # e.g. "Query: 37"
            q_idx = int(line.split()[1])
            # ensure list is long enough
            while len(neighbors) <= q_idx:
                neighbors.append([])
            current_q = q_idx

        elif line.startswith("Nearest neighbor-") and current_q is not None:
            idx = int(line.split(":")[1].strip())
            neighbors[current_q].append(idx)

        elif line.startswith("tApprox"):
            tApprox = float(line.split(":")[1].strip())
        
        elif line.startswith("QPS"):
            qps = float(line.split(":")[1].strip())

    return neighbors, tApprox, qps



def run_neural(data_file="protein_vectors.dat", query_file="protein_queries.dat", knn=5):
    base_dir = Path(__file__).resolve().parent.parent
    neural_root = base_dir / "ANN" / "Neural_LSH"

    exe_python = Path(sys.executable)


    nlsh_search_py = neural_root / "src" / "nlsh_search.py"

    data_path = base_dir.parent / "artifacts" / "embeddings" / data_file
    query_path = base_dir.parent / "artifacts" / "embeddings" / query_file
    index_path = neural_root / "index_files" / "index_ivf_protein.txt"

    neighbors, tApprox, qps = nlsh_search(
        exe_python=exe_python,
        nlsh_search_py=nlsh_search_py,
        data_path=data_path,
        query_path=query_path,
        index_path=index_path,
        k_neighbors=knn,
        T=7,
        data_type="protein"
    )

    return neighbors, tApprox, qps

def main():
    neighbors, tApprox, qps = run_neural()

    print(f"tApprox: {tApprox}")
    print(f"QPS: {qps}")

if __name__ == "__main__":
    main()
