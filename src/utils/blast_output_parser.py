from __future__ import annotations

from dataclasses import dataclass
from typing import Dict, Iterable, List, Optional
import math
import pathlib

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


_OUTFMT6_DEFAULT_COLS = [
    "qseqid", "sseqid", "pident", "length", "mismatch", "gapopen",
    "qstart", "qend", "sstart", "send", "evalue", "bitscore",
]


def parse_blast_outfmt6_line(line: str) -> Optional[BlastHit]:
    line = line.strip()
    if not line or line.startswith("#"):
        return None

    parts = line.split("\t")
    if len(parts) != 12:
        return None

    qseqid = parts[0]
    sseqid = parts[1]

    try:
        pident = float(parts[2])
        length = int(parts[3])
        mismatch = int(parts[4])
        gapopen = int(parts[5])
        qstart = int(parts[6])
        qend = int(parts[7])
        sstart = int(parts[8])
        send = int(parts[9])
        evalue = float(parts[10])
        bitscore = float(parts[11])
    except ValueError:
        return None

    if math.isnan(evalue) or math.isnan(bitscore):
        return None

    return BlastHit(
        qseqid=qseqid,
        sseqid=sseqid,
        pident=pident,
        length=length,
        mismatch=mismatch,
        gapopen=gapopen,
        qstart=qstart,
        qend=qend,
        sstart=sstart,
        send=send,
        evalue=evalue,
        bitscore=bitscore,
    )


def parse_blast_outfmt6_file(path: str) -> List[BlastHit]:
    hits: List[BlastHit] = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            hit = parse_blast_outfmt6_line(line)
            if hit is not None:
                hits.append(hit)
    return hits


def group_hits_by_query(hits: Iterable[BlastHit]) -> Dict[str, List[BlastHit]]:
    grouped: Dict[str, List[BlastHit]] = {}
    for h in hits:
        grouped.setdefault(h.qseqid, []).append(h)
    return grouped


def main():
    base_dir = pathlib.Path(__file__).resolve().parent.parent.parent
    results_path = base_dir / "artifacts" / "blast" / "blast_results.tsv"

    hits = parse_blast_outfmt6_file(results_path)
    by_q = group_hits_by_query(hits)

    print(len(hits))
    print(len(by_q), "queries")
    print(by_q[list(by_q.keys())[0]][0])


if __name__=="__main__":
    main()