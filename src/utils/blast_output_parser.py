from __future__ import annotations
from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, List, Set


@dataclass(frozen=True)
class BlastHit:
    qseqid: str
    sseqid: str
    pident: float
    length: int
    mismatch: int
    gapopen: int
    qstart: int
    qend: int
    sstart: int
    send: int
    evalue: float
    bitscore: float


def parse_blast_outfmt6_line(line):
    line = line.strip()
    if not line or line.startswith("#"):
        return None
    parts = line.split("\t")
    if len(parts) != 12:
        return None
    try:
        return BlastHit(
            qseqid=parts[0],
            sseqid=parts[1],
            pident=float(parts[2]),
            length=int(parts[3]),
            mismatch=int(parts[4]),
            gapopen=int(parts[5]),
            qstart=int(parts[6]),
            qend=int(parts[7]),
            sstart=int(parts[8]),
            send=int(parts[9]),
            evalue=float(parts[10]),
            bitscore=float(parts[11]),
        )
    except ValueError:
        return None


def parse_blast_outfmt6_file(path):
    hits: List[BlastHit] = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            h = parse_blast_outfmt6_line(line)
            if h is not None:
                hits.append(h)
    return hits


def group_blast_by_query(hits):
    out: Dict[str, List[BlastHit]] = {}
    for h in hits:
        out.setdefault(h.qseqid, []).append(h)
    return out


def build_sblast_sets(blast_by_query, n_ground_truth, max_evalue=1e-3):
    out: Dict[str, Set[str]] = {}
    for q, hits in blast_by_query.items():
        filtered = [h for h in hits if h.evalue <= max_evalue]
        filtered.sort(key=lambda h: h.bitscore, reverse=True)
        top = filtered[:n_ground_truth] if n_ground_truth > 0 else []
        out[q] = {h.sseqid for h in top}
    return out

def build_sblast_sets_identity_band(blast_by_query, n_ground_truth,min_pident=None,
                                    max_pident=None,max_evalue=1e-3):
    out: Dict[str, Set[str]] = {}
    for q, hits in blast_by_query.items():
        filtered = [h for h in hits if h.evalue <= max_evalue]
        if min_pident is not None:
            filtered = [h for h in filtered if h.pident >= min_pident]
        if max_pident is not None:
            filtered = [h for h in filtered if h.pident < max_pident]
        filtered.sort(key=lambda h: h.bitscore, reverse=True)
        top = filtered[:n_ground_truth] if n_ground_truth > 0 else []
        out[q] = {h.sseqid for h in top}
    return out



def parse_ann_neighbors_file(path):
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
    return out


def load_protein_ids(path):
    ids: List[str] = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            s = line.strip()
            if s:
                ids.append(s)
    return ids


def load_query_ids(path):
    return load_protein_ids(path)


def ann_indices_to_sseqids(ann_by_qidx, db_ids):
    out: Dict[int, List[str]] = {}
    n = len(db_ids)
    for qidx, neigh_idxs in ann_by_qidx.items():
        mapped: List[str] = []
        for i in neigh_idxs:
            if 0 <= i < n:
                mapped.append(db_ids[i])
            else:
                mapped.append(f"__OOB__:{i}")
        out[qidx] = mapped
    return out


def remap_ann_queries_to_qseqid(ann_by_qidx_sseqids, query_ids):
    out: Dict[str, List[str]] = {}
    for qidx, neighs in ann_by_qidx_sseqids.items():
        qseqid = query_ids[qidx] if 0 <= qidx < len(query_ids) else f"__MISSING_QID__:{qidx}"
        out[qseqid] = neighs
    return out


def recall_at_n(ann_ranked_sseqids, blast_top_set, n):
    if n <= 0:
        return 0.0
    if not blast_top_set:
        return 0.0
    ann_top = set(ann_ranked_sseqids[:n])
    print(ann_top)
    print("---------------------------------------------------------------------------------")
    print(blast_top_set)
    print("==================================================================================")
    return len(ann_top & blast_top_set) / len(blast_top_set)


def average_recall(ann_by_qseqid, sblast_by_qseqid, n):
    total = 0.0
    count = 0
    for q, blast_set in sblast_by_qseqid.items():
        ann = ann_by_qseqid.get(q, [])
        total += recall_at_n(ann, blast_set, n)
        count += 1
    return total / count if count else 0.0


def blast_ann_comparison(blast_tsv, ann_results, db_ids_path, query_ids_path, n_ground_truth=5, ann_n=5,
                         max_evalue=1e-3, return_identity_metrics=False, remote_min_pident=20.0,
                         remote_max_pident=30.0, close_min_pident=40.0):
    blast_hits = parse_blast_outfmt6_file(blast_tsv)
    blast_by_q = group_blast_by_query(blast_hits)

    sblast_sets = build_sblast_sets(
        blast_by_q, n_ground_truth=n_ground_truth, max_evalue=max_evalue
    )

    remote_sets = build_sblast_sets_identity_band(
        blast_by_q,
        n_ground_truth=n_ground_truth,
        min_pident=remote_min_pident,
        max_pident=remote_max_pident,
        max_evalue=max_evalue,
    )

    close_sets = build_sblast_sets_identity_band(
        blast_by_q,
        n_ground_truth=n_ground_truth,
        min_pident=close_min_pident,
        max_pident=None,
        max_evalue=max_evalue,
    )

    ann_by_qidx = parse_ann_neighbors_file(ann_results)
    db_ids = load_protein_ids(db_ids_path)
    query_ids = load_query_ids(query_ids_path)

    ann_by_qidx_sseqids = ann_indices_to_sseqids(ann_by_qidx, db_ids)
    ann_by_qseqid = remap_ann_queries_to_qseqid(ann_by_qidx_sseqids, query_ids)

    per_query: Dict[str, float] = {}
    per_query_remote: Dict[str, float] = {}
    per_query_close: Dict[str, float] = {}

    for q, blast_set in sblast_sets.items():
        per_query[q] = recall_at_n(ann_by_qseqid.get(q, []), blast_set, ann_n)

    # for q, blast_set in remote_sets.items():
    #     per_query_remote[q] = recall_at_n(ann_by_qseqid.get(q, []), blast_set, ann_n)

    # for q, blast_set in close_sets.items():
    #     per_query_close[q] = recall_at_n(ann_by_qseqid.get(q, []), blast_set, ann_n)

    avg = average_recall(ann_by_qseqid, sblast_sets, n=ann_n)
    avg_remote = average_recall(ann_by_qseqid, remote_sets, n=ann_n)
    avg_close = average_recall(ann_by_qseqid, close_sets, n=ann_n)

    if not return_identity_metrics:
        return avg, per_query

    info = {
        "avg_remote": avg_remote,
        "avg_close": avg_close,
        "per_query_remote": per_query_remote,
        "per_query_close": per_query_close,
        "remote_band": (remote_min_pident, remote_max_pident),
        "close_min_pident": close_min_pident,
        "max_evalue": max_evalue,
        "n_ground_truth": n_ground_truth,
        "ann_n": ann_n,
    }
    return avg, per_query, info
