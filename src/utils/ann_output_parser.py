from __future__ import annotations

from typing import Dict, List, Sequence, Tuple
import pathlib


def load_protein_ids(path: str) -> List[str]:
    ids: List[str] = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            s = line.strip()
            if s:
                ids.append(s)
    return ids


def parse_ann_neighbors_file(path: str) -> Dict[int, List[int]]:
    out: Dict[int, List[int]] = {}
    q: int | None = None

    with open(path, "r", encoding="utf-8") as f:
        for raw in f:
            line = raw.strip()
            if not line:
                continue
            if line.startswith("Query:"):
                q = int(line.split(":", 1)[1].strip())
                out.setdefault(q, [])
                continue
            if line.startswith("Nearest neighbor-"):
                if q is None:
                    continue
                idx = int(line.split(":", 1)[1].strip())
                out[q].append(idx)
                continue

    return out


def ann_indices_to_ids(
    ann_by_query_idx: Dict[int, Sequence[int]],
    protein_ids: Sequence[str],
) -> Dict[int, List[str]]:
    out: Dict[int, List[str]] = {}
    n = len(protein_ids)

    for q, neigh_idxs in ann_by_query_idx.items():
        mapped: List[str] = []
        for i in neigh_idxs:
            if 0 <= i < n:
                mapped.append(protein_ids[i])
            else:
                mapped.append(f"__OOB__:{i}")
        out[q] = mapped

    return out


def invert_query_ids(query_ids_path: str) -> Dict[int, str]:
    qids = load_protein_ids(query_ids_path)
    return {i: qid for i, qid in enumerate(qids)}


def remap_ann_to_query_ids(
    ann_by_qidx_to_sids: Dict[int, List[str]],
    query_ids_path: str,
) -> Dict[str, List[str]]:
    qid_map = invert_query_ids(query_ids_path)
    out: Dict[str, List[str]] = {}
    for qidx, sids in ann_by_qidx_to_sids.items():
        qid = qid_map.get(qidx, f"__MISSING_QID__:{qidx}")
        out[qid] = list(sids)
    return out


def main():
    base_dir = pathlib.Path(__file__).resolve().parent.parent.parent
    results = base_dir / "results/lsh.txt"
    protein_ids = base_dir / "artifacts" / "indices" / "protein_ids"
    queries_ids = base_dir / "artifacts" / "indices" / "queries_ids"

    ann = parse_ann_neighbors_file(results)
    db_ids = load_protein_ids(protein_ids)

    ann_sids_by_qidx = ann_indices_to_ids(ann, db_ids)
    ann_sids_by_qid = remap_ann_to_query_ids(ann_sids_by_qidx, queries_ids)
    for key, values in ann_sids_by_qid.items():
        print(f"{key}:{values}")



if __name__ == "__main__":
    main()