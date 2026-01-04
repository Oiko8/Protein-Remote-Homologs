import subprocess
import os
from pathlib import Path

def run_make(build_dir, target="lsh"):
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


def LSH(exe_path, data_path, query_path, k_neighbors=10, khash=10,
                      L=18, w=5.5):
    """
    Run LSHmain with dataset=data_path and query=data_path and return
    neighbors[q] = list of k_neighbors indices.
    """
    # Build the command as it runs in terminal
    cmd = [
        exe_path,
        "-d", data_path,
        "-q", query_path,
        "-k", str(khash),
        "-L", str(L),
        "-w", str(w),
        "-N", str(k_neighbors)]
    


    # Run it in the correct directory (where LSHmain exists)
    cwd = os.path.dirname(exe_path) or None

    # add more threads for faster results
    env = os.environ.copy()
    env["OMP_NUM_THREADS"] = "8"

    result = subprocess.run(
        cmd,
        cwd=cwd,
        capture_output=True,
        text=True,
        env=env)

    if result.returncode != 0:
        print("LSHmain failed")
        print("STDOUT:\n", result.stdout)
        print("STDERR:\n", result.stderr)
        raise RuntimeError("LSHmain failed")

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


def run_lsh(data_file="protein_vectors.dat", query_file="protein_queries.dat", knn=5):
    # Path to the executable 
    base_dir = Path(__file__).resolve().parent.parent
    classic_ann_dir = base_dir / "ANN" / "Classic_ANN" 
    exe_path = classic_ann_dir / "src" / "bin" / "LSHmain"

    data_path = base_dir.parent / "artifacts" / "embeddings" / data_file
    query_path = base_dir.parent / "artifacts" / "embeddings" / query_file

    run_make(classic_ann_dir / "src")
    neighbors, tApprox, qps = LSH(exe_path=exe_path, query_path=query_path, data_path=data_path, k_neighbors=knn)
    
    run_make(classic_ann_dir / "src", "clean")

    return neighbors, tApprox, qps

def main():

    neighbors, tApprox, qps = run_lsh()
    print(f"tApprox: {tApprox}")
    print(f"QPS: {qps}")


if __name__=="__main__":
    main()