import time
from pathlib import Path
import sys
import pathlib
from utils.args_parser import parse_arguments_search

from ann_wrappers.lsh_wrapper import run_lsh
from ann_wrappers.hc_wrapper import run_hc
from ann_wrappers.ivfflat_wrapper import run_ivfflat
from ann_wrappers.ivfpq_wrapper import run_ivfpq
from ann_wrappers.neural_wrapper import run_neural
from utils.blast_output_parser import blast_ann_comparison


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

    if not out_path.exists():

        if args.method == "lsh":
            neighbors, tApprox, qps = run_lsh(data_file, query_file, knn)

        elif args.method == "hypercube":
            neighbors, tApprox, qps = run_hc(data_file, query_file, knn)

        elif args.method == "ivfflat":
            neighbors, tApprox, qps = run_ivfflat(data_file, query_file, knn)

        elif args.method == "ivfpq":
            neighbors, tApprox, qps = run_ivfpq(data_file, query_file, knn)

        elif args.method == "neural":
            neighbors, tApprox, qps = run_neural(data_file, query_file, knn)
                

        else:
            raise ValueError("Unknown method")


        write_neighbors(out_path, neighbors)

        perf_path = out_path.with_suffix(out_path.suffix + ".perf")
        with open(perf_path, "w", encoding="utf-8") as f:
            f.write(f"method {args.method}\n")
            f.write(f"queries {len(neighbors)}\n")
            f.write(f"time_sec {tApprox:.6f}\n")
            f.write(f"qps {qps:.6f}\n")


    base_dir = pathlib.Path(__file__).resolve().parent.parent
    blast_tsv = base_dir / "artifacts" / "blast" / "blast_results.tsv"
    protein_ids = base_dir / "artifacts" / "indices" / "protein_ids"
    queries_ids = base_dir / "artifacts" / "indices" / "queries_ids"
    avg, per_query, info = blast_ann_comparison(blast_tsv, out_path, protein_ids, queries_ids, max_evalue=0.01, 
                                                n_ground_truth=knn, ann_n=knn, return_identity_metrics=True, close_min_pident=30)

    # avg, per_query = blast_ann_comparison(blast_tsv, out_path, protein_ids, queries_ids, max_evalue=0.001, 
    #                                             n_ground_truth=knn, ann_n=knn, return_identity_metrics=False)
    
    print(f"Average Recall@{knn}: {avg}")
    print(f"Average RemoteRecall@{knn} (20-30%): {info['avg_remote']}")
    print(f"Average CloseRecall@{knn} (>=30%): {info['avg_close']}")


    for q, r in per_query.items():
        print(q, r)


if __name__ == "__main__":
    main()
