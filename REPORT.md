# Remote Homologs Report

## **Introduction**
In this assignment, we study the problem of protein homology detection, with a particular focus on remote homologs, i.e. proteins that share a common evolutionary origin but exhibit low sequence identity and are therefore difficult to detect using traditional alignment-based methods alone.

To address this challenge, we represent proteins using embedding vectors derived from a pretrained protein language model and perform Approximate Nearest Neighbor (ANN) search in the embedding space. The goal is to evaluate whether embedding-based similarity can retrieve biologically meaningful neighbors that are missed or weakly detected by BLAST, especially in the so-called ***Twilight Zone*** (BLAST identity < 30%).

We evaluate several ANN algorithms, including LSH, Hypercube, IVF-Flat, IVF-PQ, and Neural ANN, comparing their efficiency (time per query, QPS) and their agreement with BLAST results. For each query protein, the search retrieves the Top-50 nearest neighbors in the embedding space. From these neighbors, we retain only those with BLAST percentage identity below 30%, as these represent candidate remote homologs.


***Remote Homologs*** :  
```
In this assignment, a protein is considered a candidate remote homolog when it satisfies the following conditions:

1. It is retrieved among the Top-N nearest neighbors of a query protein in the embedding space, as determined by an Approximate Nearest Neighbor (ANN) algorithm, indicating high similarity at the representation level.

2. Sequence-based comparison using BLAST shows low similarity, either with percentage identity below 30% (Twilight Zone) or with no detectable BLAST hit.

3. The potential evolutionary relationship is subsequently evaluated through independent biological evidence, such as shared Pfam domains, similar UniProt functional annotations, or related GO terms.

Only proteins that combine embedding-level proximity with weak or absent sequence similarity and are supported by biological annotations are treated as remote homolog candidates.
```
 
The final step of the assignment involves the manual biological validation of selected candidate proteins using external annotations (UniProt, Pfam, GO), in order to assess whether the retrieved neighbors can be considered true remote homologs.

The total reports for each algorithm can be found on [reports](reports/).

## **ANN Algorithms - Blast comparison**
----------------------------------------------------------------------
Method       | Time/query (s) |     QPS |  Recall@N vs BLAST Top-N
------------ |----------------|---------|-----------------------------
lsh          |       0.003712 |  269.42 |                   0.3090
hypercube    |       0.003208 |  311.73 |                   0.1502
ivfflat      |       0.001186 |  843.28 |                   0.2897
ivfpq        |       0.000114 | 8755.51 |                   0.2129
neural LSH   |       0.011608 |   86.15 |                   0.3194
BLAST (Ref)  |         0.3277 |    3.05 |                   1.0000
----------------------------------------------------------------------


## **Twilight Zone Analysis**

### *Query Protein: A0A009I3Y5*
### *Query Protein: A0A009PCK4*
### *Query Protein: A0A001*
### *Query Protein: A0A002*
### *Query Protein: A0A009HL96*
### *Query Protein: A0A009HLV9*
### *Query Protein: A0A009HN45*
### *Query Protein: A0A009HQC9*
### *Query Protein: A0A009HPM0*
### *Query Protein: A0A009IB02*
### *Query Protein: A0A010Q3W2*
### *Query Protein: A0A010Q8R4*

