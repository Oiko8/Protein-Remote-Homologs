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

### *Query Protein: A0A009I3Y5 (Protein kinase domain-containing protein)* 
- biological process : protein phosphorylation (GO:0006468)
- molecular function : protein kinase activity (GO:0004672)  ATP binding (GO:0005524)
- cellular component : none
- Pfam : PF00069
- SUPFAM : SSF56112 Protein kinase-like

Potential homologs
1. sp|A5UG81|KDKA_HAEIG
- L2: 1.7496
- no in blast top n and no blast ident
- molecular function: phosphotransferase activity, alcohol group as acceptor (GO:0016773)  
- Pfam : PF06293  
Conclusion : False positive (more comments why?) 

2. sp|P33800|PG187_VAR67  : B1 Kinase
- L2:  
- no in blast top n and no blast ident
- biological process : protein phosphorylation (GO:0006468)
- molecular function : protein kinase activity (GO:0004672) ATP binding (GO:0005524)
- Pfam: PF00069  
- Conclusion : Remote homolog with similar biological process, molecular function, pfam

3. sp|P14181|LICA2_HAEIF
- 1.6867
- no in blast top n and no blast ident
- SUPFAM : SSF56112 Protein kinase-like
- Domain : Phosphorylase , same like the query  
- Conclusion : the proteins are possibly remote homologs and that's why they are so near in the embedding space, they share same SuperFamily (SUPERFAMILY provides structure-level annotations, which are especially useful for remote homology detection.). The two proteins belong to the same structural superfamily according to SUPERFAMILY, supporting a remote evolutionary relationship despite low sequence identity.

### *Query Protein: A0A009PCK4*
sp|A5UG81|KDKA_HAEIG
sp|P14181|LICA2_HAEIF
sp|Q6C2A3|BUD32_YARLI
### *Query Protein: A0A001*
kamia protein koini stoys 5
sp|P9WQJ3|FATRP_MYCTU lsh, hypercube, ivfflat, neural
sp|Q13BH6|NDVA_RHOPS
sp|Q03024|APRD_PSEAE lsh, ivfflat, ivfpq, neural
sp|Q51719|YCOBA_PROFR 
### *Query Protein: A0A002*
sp|P9WQJ3|FATRP_MYCTU kai stoys 5
sp|Q13BH6|NDVA_RHOPS
sp|Q20Z38|NDVA_RHOPB oxi ston ivfpq
sp|Q51719|YCOBA_PROFR 
### *Query Protein: A0A009HL96*
sp|Q9AE24|RPRY_BACFR koina kai stous 5
sp|Q5HI09|GRAR_STAAC oxi ston ivfpq
sp|Q99VW2|GRAR_STAAN oxi ston ivfpq
### *Query Protein: A0A009HLV9*
o IVFpq den parousiazei kanena koino 
sp|A1KU52|RUVB_NEIMF stous 4
sp|Q8EEF3|RUVB_SHEON
sp|P66756|RUVB_SALTI
### *Query Protein: A0A009HN45*
sp|P52126|ABPB_ECOLI
sp|Q99ZA5|UVRB_STRP1
sp|Q1JBD1|UVRB_STRPB
### *Query Protein: A0A009HQC9*
sp|C5BEJ2|UVRB_EDWI9 se ola
sp|C0PWY7|UVRB_SALPC oxi se hyper 
sp|A7FKM4|UVRB_YERP3 oxi se hyper
### *Query Protein: A0A009HPM0*
ivfpq kanena koino 
sp|B2TWG0|TRMA_SHIB3 → lsh, hypercube, ivfflat, neural
sp|B1LNS5|TRMA_ECOSM → lsh, hypercube, ivfflat, neural
sp|Q5PK68|TRMA_SALPA → lsh, hypercube, ivfflat, neural
### *Query Protein: A0A009IB02*
hyper kanena koino 
sp|Q9SQI8|ODP24_ARATH
sp|P49786|BCCP_BACSU
sp|P0ABE1|BCCP_SHIFL
### *Query Protein: A0A010Q3W2*
sp|Q2UUT4|CMR1_ASPOR
sp|A2QI22|YTM1_ASPNC oxi lsh
sp|P0CS56|CMR1_CRYNJ oxi ivfpq
### *Query Protein: A0A010Q8R4*
sp|Q652L2|HIRA_ORYSJ
sp|P0CS56|CMR1_CRYNJ
sp|Q2UUT4|CMR1_ASPOR

έναν πίνακα με όλα τα κοινά Neighbor ανά query και μέθοδο, για να φαίνεται καθαρά ποιος γείτονας εμφανίζεται σε ποιες μεθόδους
