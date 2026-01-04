import pathlib
from pathlib import Path

from utils.args_parser import parse_arguments_search

from ann_wrappers.lsh_wrapper import run_lsh
from ann_wrappers.hc_wrapper import run_hc
from ann_wrappers.ivfflat_wrapper import run_ivfflat
from ann_wrappers.ivfpq_wrapper import run_ivfpq
from ann_wrappers.neural_wrapper import run_neural


def read_ids(path: Path):
    ids = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            s = line.strip()
            if s:
                ids.append(s)
    return ids


def write_neighbors(path: Path, neighbors):
    path.parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", encoding="utf-8") as f:
        for qi, lst in enumerate(neighbors):
            f.write(f"Query: {qi}\n")
            for r, it in enumerate(lst, start=1):
                if isinstance(it, (tuple, list)) and len(it) >= 1:
                    idx = int(it[0])
                else:
                    idx = int(it)
                f.write(f"Nearest neighbor-{r}: {idx}\n")
            f.write("\n")


def load_ann_neighbors(path: Path):
    neighbors = []
    cur = None
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            s = line.strip()
            if not s:
                continue
            if s.startswith("Query:"):
                if cur is not None:
                    neighbors.append(cur)
                cur = []
                continue
            if s.startswith("Nearest neighbor-"):
                v = s.split(":", 1)[1].strip()
                cur.append(int(v))
    if cur is not None:
        neighbors.append(cur)
    return neighbors


def parse_blast_outfmt6(blast_tsv: Path):
    hits = {}
    with open(blast_tsv, "r", encoding="utf-8") as f:
        for line in f:
            s = line.strip()
            if not s:
                continue
            parts = s.split("\t")
            if len(parts) < 12:
                continue
            qseqid = parts[0]
            sseqid = parts[1]
            pident = float(parts[2])
            evalue = float(parts[10])
            bitscore = float(parts[11])
            hits.setdefault(qseqid, []).append((sseqid, pident, evalue, bitscore))
    return hits


def build_blast_ground_truth(hits_by_q, n, max_evalue):
    gt = {}
    pid = {}
    for q, lst in hits_by_q.items():
        flt = [x for x in lst if x[2] <= max_evalue]
        flt.sort(key=lambda x: x[3], reverse=True)
        top = flt[:n]
        gt[q] = {s for (s, _, _, _) in top}
        for s, p, _, _ in flt:
            k = (q, s)
            if k not in pid or p > pid[k]:
                pid[k] = p
    return gt, pid


def unwrap_neighbor(it):
    if isinstance(it, (tuple, list)) and len(it) >= 2:
        return int(it[0]), float(it[1])
    if isinstance(it, (tuple, list)) and len(it) == 1:
        return int(it[0]), None
    return int(it), None


def run_method(method: str, data_file: str, query_file: str, knn: int):
    if method == "lsh":
        return run_lsh(data_file, query_file, knn)
    if method == "hypercube":
        return run_hc(data_file, query_file, knn)
    if method == "ivfflat":
        return run_ivfflat(data_file, query_file, knn)
    if method == "ivfpq":
        return run_ivfpq(data_file, query_file, knn)
    if method == "neural":
        return run_neural(data_file, query_file, knn)
    raise ValueError("Unknown method")


def fmt_float(x, nd=3):
    if x is None:
        return "--"
    return f"{x:.{nd}f}"


def fmt_pct(x):
    if x is None:
        return "--"
    return f"{x:.0f}%"


def main():
    args = parse_arguments_search()

    out_path = Path(args.o)
    data_file = args.data
    query_file = args.queries
    knn = args.N
    method = args.method

    if not out_path.exists():
        neighbors, tApprox, qps = run_method(method, data_file, query_file, knn)
        write_neighbors(out_path, neighbors)

        perf_path = out_path.with_suffix(out_path.suffix + ".perf")
        with open(perf_path, "w", encoding="utf-8") as f:
            f.write(f"method {method}\n")
            f.write(f"queries {len(neighbors)}\n")
            f.write(f"time_sec {tApprox:.6f}\n")
            f.write(f"qps {qps:.6f}\n")

    base_dir = pathlib.Path(__file__).resolve().parent.parent
    blast_tsv = base_dir / "artifacts" / "blast" / "blast_results.tsv"
    protein_ids_path = base_dir / "artifacts" / "indices" / "protein_ids"
    queries_ids_path = base_dir / "artifacts" / "indices" / "queries_ids"

    protein_ids = read_ids(protein_ids_path)
    query_ids = read_ids(queries_ids_path)

    ann_neighbors_idx = load_ann_neighbors(out_path)

    hits_by_q = parse_blast_outfmt6(blast_tsv)
    max_evalue = 0.01
    blast_topn, blast_pident = build_blast_ground_truth(hits_by_q, knn, max_evalue)

    perf_path = out_path.with_suffix(out_path.suffix + ".perf")
    tApprox = None
    qps = None
    if perf_path.exists():
        with open(perf_path, "r", encoding="utf-8") as f:
            for line in f:
                s = line.strip()
                if s.startswith("time_sec "):
                    tApprox = float(s.split()[1])
                elif s.startswith("qps "):
                    qps = float(s.split()[1])

    recalls = []
    for qi, neigh_list in enumerate(ann_neighbors_idx):
        qid = query_ids[qi] if qi < len(query_ids) else str(qi)
        ann_set = set()
        for it in neigh_list[:knn]:
            idx, _ = unwrap_neighbor(it)
            if 0 <= idx < len(protein_ids):
                ann_set.add(protein_ids[idx])
        gt = blast_topn.get(qid, set())
        r = (len(ann_set & gt) / len(gt)) if len(gt) > 0 else 0.0
        recalls.append(r)

    avg_recall = sum(recalls) / len(recalls) if recalls else 0.0

    for qi, neigh_list in enumerate(ann_neighbors_idx):
        qid = query_ids[qi] if qi < len(query_ids) else str(qi)
        gt = blast_topn.get(qid, set())

        print(f"Query Protein: {qid}")
        print(f"N = {knn} (μέγεθος λίστας Top-N για την αξιολόγηση Recall@N)")
        print("[1] Συνοπτική σύγκριση μεθόδων")
        print("----------------------------------------------------------------------")
        print("Method      | Time/query (s) | QPS | Recall@N vs BLAST Top-N")
        print("----------------------------------------------------------------------")
        print(f"{method}   | {fmt_float(tApprox, 6)} | {fmt_float(qps, 2)} | {avg_recall:.4f}")
        print("BLAST (Ref) |      --        | --  | 1.0000 (ορίζει το Top-N)")
        print("----------------------------------------------------------------------")
        print()
        print(f"[2] Top-N γείτονες ανά μέθοδο (εδώ π.χ. N = {min(20, knn)} για εκτύπωση)")
        print(f"Method: {method}")
        print("Rank |     Neighbor ID    | L2 Dist | BLAST Identity | In BLAST Top-N? | Bio comment")
        print("-----------------------------------------------------------------------------")

        to_print = min(20, knn)
        for rank in range(1, to_print + 1):
            if rank - 1 >= len(neigh_list):
                break
            idx, dist = unwrap_neighbor(neigh_list[rank - 1])
            nid = protein_ids[idx] if 0 <= idx < len(protein_ids) else str(idx)

            pid = blast_pident.get((qid, nid), None)
            in_blast = "Yes" if nid in gt else "No"

            bio = "--"
            if pid is not None and pid < 30.0 and in_blast == "Yes":
                bio = "Remote homolog? (Twilight Zone, check Pfam/GO/EC)"
            elif pid is not None and pid >= 30.0 and in_blast == "Yes":
                bio = "Close homolog (>=30%)"

            print(f"{rank} | {nid} |   {fmt_float(dist, 4)}   |      {fmt_pct(pid)}       |        {in_blast}          | {bio}")

        print()


if __name__ == "__main__":
    main()
