import time
from pathlib import Path
import sys

from utils.args_parser import parse_arguments_search

from ann_wrappers.lsh_wrapper import run_lsh
from ann_wrappers.hc_wrapper import run_hc
from ann_wrappers.ivfflat_wrapper import run_ivfflat
from ann_wrappers.ivfpq_wrapper import run_ivfpq
from ann_wrappers.neural_wrapper import run_neural


def write_neighbors(path: Path, neighbors):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        for qi, lst in enumerate(neighbors):
            f.write(f"Query: {qi}\n")
            for r, idx in enumerate(lst, start=1):
                f.write(f"Nearest neighbor-{r}: {idx}\n")
            f.write("\n")


def main():
    args = parse_arguments_search()

    out_path = Path(args.o)

    data_file = args.data
    query_file = args.queries
    knn = args.N

    t0 = time.perf_counter()

    if args.method == "lsh":
        neighbors = run_lsh(data_file, query_file, knn)

    elif args.method == "hypercube":
        neighbors = run_hc(data_file, query_file, knn)

    elif args.method == "ivfflat":
        neighbors = run_ivfflat(data_file, query_file, knn)

    elif args.method == "ivfpq":
        neighbors = run_ivfpq(data_file, query_file, knn)

    elif args.method == "neural":
        neighbors = run_neural(data_file, query_file, knn)
            

    else:
        raise ValueError("Unknown method")

    dt = time.perf_counter() - t0

    write_neighbors(out_path, neighbors)

    perf_path = out_path.with_suffix(out_path.suffix + ".perf")
    with open(perf_path, "w", encoding="utf-8") as f:
        f.write(f"method {args.method}\n")
        f.write(f"queries {len(neighbors)}\n")
        f.write(f"time_sec {dt:.6f}\n")
        f.write(f"qps {len(neighbors)/dt:.6f}\n")


if __name__ == "__main__":
    main()
