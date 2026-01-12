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


In addition to the focused Twilight Zone analysis presented below, we retain the full, detailed output of each ANN algorithm in the [reports](reports/) directory, including all retrieved neighbors and performance metrics.

From these complete reports, we select 3–4 representative examples per query protein to highlight characteristic cases in which embedding-based ANN methods retrieve biologically meaningful candidates that are weakly detected or entirely missed by BLAST.

For each selected example, we provide biological justification based on independent annotations, such as shared Pfam domains, similar GO terms, UniProt functional descriptions, or common SUPERFAMILY assignments, in order to assess whether the retrieved neighbors represent plausible cases of remote homology.

## **Twilight Zone Analysis**

### *Query Protein: A0A009I3Y5 (Protein kinase domain-containing protein)* 
| Property           | Description                                                    |
| ------------------ | -------------------------------------------------------------- |
| Biological process | Protein phosphorylation (GO:0006468)                           |
| Molecular function | Protein kinase activity (GO:0004672), ATP binding (GO:0005524) |
| Cellular component | –                                                              |
| Pfam               | PF00069                                                        |
| SUPERFAMILY        | SSF56112 (Protein kinase-like)                                 |


**Candidate neighbors**

| Neighbor Protein | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                        | Conclusion                                                                                                                                                                                              |
| ---------------- | ------: | :------: | :-------: | ----------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **A5UG81**       |  1.7496 |     –    |     No    | Phosphotransferase activity (GO:0016773); Pfam PF06293 (different from kinase Pfam) | **False positive.** The protein lacks the protein kinase Pfam domain and does not participate in protein phosphorylation, indicating that embedding proximity is not due to shared evolutionary origin. |
| **P33800**       |  2.0267 |     –    |     No    | Protein phosphorylation; protein kinase activity; ATP binding; Pfam PF00069         | **Remote homolog.** The shared biological role, molecular function, and conserved kinase Pfam domain strongly support a remote evolutionary relationship despite the absence of BLAST hits.             |
| **P14181**       |  1.6867 |     –    |     No    | Same SUPERFAMILY (SSF56112, protein kinase-like); kinase-related domain             | **Remote homolog.** Structural superfamily conservation explains the close embedding distance and supports remote homology despite low sequence similarity.                                             |


### *Query Protein: A0A009PCK4 (Protein kinase domain-containing protein)*
- biological process : protein phosphorylation (GO:0006468)
- molecular function : protein kinase activity (GO:0004672)  ATP binding (GO:0005524)
- cellular component : none
- Pfam : PF00069
- SUPFAM : SSF56112 Protein kinase-like

Potential homologs  

1. sp|P14181|LICA2_HAEIF
- L2: 1.6715
- no in blast top n and no blast ident
- SUPFAM : SSF56112 Protein kinase-like
- Domain : Phosphorylase , same like the query  
- Conclusion : the proteins are possibly remote homologs and that's why they are so near in the embedding space, they share same SuperFamily (SUPERFAMILY provides structure-level annotations, which are especially useful for remote homology detection.). The two proteins belong to the same structural superfamily according to SUPERFAMILY, supporting a remote evolutionary relationship despite low sequence identity.

2. sp|P44033|Y665_HAEIN  Putative kinase HI_0665
- L2: 1.8396
- no in blast top n and no blast ident
- Seems like they have a small similarity because they both are kinase related, but they have significant differences in pfam and GO descriptions.
- Conclusion: False positive.


3. sp|Q6C2A3|BUD32_YARLI (EKC/KEOPS complex subunit BUD32)
- L2: 2.0015
- biological process : protein phosphorylation (GO:0006468)
- molecular function : protein kinase activity (GO:0004672), ATP binding (GO:0005524), protein serine/threonine kinase activity (GO:0004674)
- SUPFAM : SSF56112 Protein kinase-like
- Conclusion:  Remote homolog with similar biological process, molecular function. Moreover, the two proteins belong to the same structural superfamily according to SUPERFAMILY, supporting a remote evolutionary relationship despite low sequence identity.

### *Query Protein: A0A001 (ABC transporter)*
- biological process : transmembrane transport (GO:0055085)
- molecular function : ATP binding (GO:0005524), ABC-type transporter activity (GO:0140359), ATP hydrolysis activity (GO:0016887)
- cellular component : membrane (GO:0016020)
- pfam : PF00664 ABC_membrane, PF00005 ABC_tran 
- supfam : SSF90123, SSF52540

1. sp|Q51719|YCOBA_PROFR (Putative ABC transporter ATP-binding protein in cobA 5'region)
- L2 : 1.7701
- no in blast top N, 30% identity
- biological process : transmembrane transport (GO:0055085), cobalt ion transport (GO:0006824)
- molecular function : ATP binding (GO:0005524), ATP hydrolysis activity (GO:0016887)
- cellular component : membrane (GO:0016020)
- pfam : PF00005
- supfam: SSF52540
- Conclusion :  Remote homolog with similar biological process, molecular function and cellular component. Moreover, the two proteins belong to the same structural superfamily according to SUPERFAMILY, supporting a remote evolutionary relationship despite low sequence identity. Also the share the same pfam.

2. sp|Q13BH6|NDVA_RHOPS (Beta-(1-->2)glucan export ATP-binding/permease protein NdvA)
- L2 : 1.4232
- in blast top50, 33% identity, also very close in embedding space
- biological process : transmembrane transport (GO:0055085)
- molecular function : ATP binding (GO:0005524), ABC-type transporter activity (GO:0140359), ATP hydrolysis activity (GO:0016887)
- cellular component : membrane (GO:0016020)
- pfam : PF00664 ABC_membrane, PF00005 ABC_tran 
- supfam : SSF52540
- Conclusion :  Remote homolog with similar biological process, molecular function and cellular component. Moreover, the two proteins belong to the same structural superfamily according to SUPERFAMILY, supporting a remote evolutionary relationship despite low sequence identity. Also the share the same pfam. Both blast and ANN works good on this one.

3. sp|Q3JSR6|SSUB_BURP1 (Aliphatic sulfonates import ATP-binding protein SsuB)
- L2 : 1.7942
- no in blast top50, 37% identity.
- molecular function : ATP binding (GO:0005524), ATP hydrolysis activity (GO:0016887)
- supfam : SSF52540
- conclusion : no the same pfam, but the same molecular function and same supfam. the two proteins belong to the same structural superfamily according to SUPERFAMILY, supporting a remote evolutionary relationship. Also high indentity sequence despite the low blast score. The two proteins can be named remote homologs.


### *Query Protein: A0A002 (ABC transporter ATP-binding protein)*
- biological process : transmembrane transport (GO:0055085)
- molecular function : ATP binding (GO:0005524), ABC-type transporter activity (GO:0140359), ATP hydrolysis activity (GO:0016887)
- cellular component : membrane (GO:0016020)
- pfam : PF00664 ABC_membrane, PF00005 ABC_tran 
- supfam : SSF90123, SSF52540


1. sp|P9WQJ3|FATRP_MYCTU (Fatty acid ABC transporter ATP-binding/permease protein)
- L2 : 1.3344 
- in blast top50 and 27% identity.
- biological process : transmembrane transport (GO:0055085)
- molecular function : ATP binding (GO:0005524), ABC-type transporter activity (GO:0140359), ATP hydrolysis activity (GO:0016887)
- cellular component : membrane (GO:0016020)
- pfam : PF00664 ABC_membrane, PF00005 ABC_tran 
- supfam : SSF90123, SSF52540
- conclusion : both the ann and the blast along with the pfam,supfam and go descriptions agree that the two proteins are remote homologs.

2. sp|Q63NR0|HMUV_BURPS (Hemin import ATP-binding protein HmuV)
- L2 : 2.0712
- no in blast topN, 29% identity
- molecular function : ATP binding (GO:0005524), ATP hydrolysis activity (GO:0016887)
- pfam :  PF00005 ABC_tran 
- supfam : SSF52540
- Conclusion : similar pfam and supfam, very close in the embedding space, similar molecular function. Another case that the ANN algorithms works better than the blast.

3. sp|Q82MV1|SSUB1_STRAW (Aliphatic sulfonates import ATP-binding protein SsuB 1)
- L2 : 1.7931
- no in blast topn, 30% identity match
- molecular function : ATP binding (GO:0005524), ATP hydrolysis activity (GO:0016887)
- pfam :  PF00005 ABC_tran 
- supfam : SSF52540
- Conclusion : similar pfam and supfam, very close in the embedding space, similar molecular function. Another case that the ANN algorithms works better than the blast

### *Query Protein: A0A009HL96 (Response regulator)*
- biological process : phosphorelay signal transduction system(GO:0000160), regulation of DNA-templated transcription (GO:0006355)
- molecular function : DNA binding (GO:0003677), transcription cis-regulatory region binding (GO:0000976)
- cellular component : NONE
- pfam : PF00072 Response_reg, PF00486 Trans_reg_C 
- supfam : SSF46894, SSF52172

1. sp|Q9AE24|RPRY_BACFR (Transcriptional regulatory protein RprY)
- L2 : 0.7792
- Yes in BLAST topn, 28% identity match
- biological process : phosphorelay signal transduction system(GO:0000160), regulation of DNA-templated transcription (GO:0006355)
- molecular function : DNA binding (GO:0003677), transcription cis-regulatory region binding (GO:0000976)
- cellular component : NONE
- pfam :  PF00072 Response_reg, PF00486 Trans_reg_C  
- supfam : SSF46894, SSF52172
- Conclusion : Both the ANN and the BLAST along with the pfam,supfam and GO descriptions agree that the two proteins are remote homologs.

2. sp|Q5HI09|GRAR_STAAC (Response regulator protein GraR)
- L2 : 0.6392
- Yes in BLAST topn, 26% identity match
- biological process : phosphorelay signal transduction system(GO:0000160), regulation of DNA-templated transcription (GO:0006355)
- molecular function : DNA binding (GO:0003677), transcription cis-regulatory region binding (GO:0000976)
- cellular component : NONE
- pfam :  PF00072 Response_reg, PF00486 Trans_reg_C  
- supfam : SSF46894, SSF52172
- Conclusion : Both the ANN and the BLAST along with the pfam,supfam and GO descriptions agree that the two proteins are remote homologs.

3. sp|Q99VW2|GRAR_STAAN (Response regulator protein GraR)
- L2 : 0.6598
- Yes in BLAST topn, 26% identity match
- biological process : phosphorelay signal transduction system(GO:0000160), regulation of DNA-templated transcription (GO:0006355)
- molecular function : DNA binding (GO:0003677), transcription cis-regulatory region binding (GO:0000976)
- cellular component : NONE
- pfam :  PF00072 Response_reg, PF00486 Trans_reg_C  
- supfam : SSF46894, SSF52172
- Conclusion : Both the ANN and the BLAST along with the pfam,supfam and GO descriptions agree that the two proteins are remote homologs.

### *Query Protein: A0A009HLV9 (DNA-binding transcriptional regulator NtrC)*
- biological process : phosphorelay signal transduction system(GO:0000160), regulation of DNA-templated transcription (GO:0006355), regulation of nitrogen utilization (GO:0006808)
- molecular function : DNA binding (GO:0003677), ATP binding (GO:0005524), transcription factor binding (GO:0008134), sequence-specific DNA binding (GO:0043565), phosphorelay response regulator activity (GO:0000156)
- cellular component : NONE
- pfam : PF25601 AAA_lid_14, PF02954 HTH_8, PF00072 Response_reg, PF00158 Sigma54_activat  
- supfam : SSF52172, SSF46689, SSF52540

1. sp|A1KU52|RUVB_NEIMF (Holliday junction branch migration complex subunit RuvB)
- L2 : 1.0106
- No in BLAST topn, No BLAST identity match
- biological process : DNA repair (GO:0006281), DNA recombination (GO:0006310)
- molecular function : DNA binding (GO:0003677), ATP binding (GO:0005524), four-way junction helicase activity (GO:0009378)
- cellular component : NONE
- pfam :  PF17864 AAA_lid_4, PF05491 RuvB_C, PF05496 RuvB_N
- supfam : SSF46894, SSF52172
- Conclusion : Τhe ANN methods retrieve the neighbor due to shared generic structural features, such as ATP-binding, rather than true evolutionary homology. The lack of shared Pfam domains and distinct biological functions indicate a false positive for remote homolog detection, consistent with the BLAST results.

2. sp|A8A3I6|NORR_ECOHS (Anaerobic nitric oxide reductase transcription regulator NorR)
- L2 : 0.9989
- Yes in BLAST topn, 38% BLAST identity match
- biological process : regulation of DNA-templated transcription (GO:0006355)
- molecular function : protein binding (GO:0005515), ATP binding (GO:0005524), transcription factor binding (GO:0008134), DNA-binding transcription factor activity (GO:0003700)
- cellular component : NONE
- pfam :  PF01590 GAF, PF00158 Sigma54_activat
- supfam : SSF55781, SSF46689, SSF52540
- Conclusion : The neighbor is retrieved by ANN due to shared regulatory features that are also detected by BLAST. The relatively high BLAST identity (38%) and overlapping Pfam and GO annotations indicate a conventional homolog rather than a remote one, showing agreement between ANN and BLAST.

3. sp|P66756|RUVB_SALTI (Anaerobic nitric oxide reductase transcription regulator NorR)
- L2 : 1.0884
- Νο in BLAST topn, Νο BLAST identity match
- biological process : DNA repair (GO:0006281), DNA recombination (GO:0006310)
- molecular function : four-way junction helicase activity (GO:0009378), ATP binding (GO:0005524), DNA binding (GO:0003677)
- cellular component : NONE
- pfam :  PF17864 AAA_lid_4, PF05491 RuvB_C, PF05496 RuvB_N
- supfam : SSF46785, SSF52540
- Conclusion : The ANN algorithms detect this neighbor likely due to functional or structural similarity, while BLAST misses it because of low sequence identity. This suggests a distant or analogous relationship rather than a true homolog, highlighting the different sensitivities of the methods.
  
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
- molecular function : protein binding (GO:0005515), transcription corepressor activity (GO:0003714)
- pfam : PF08512, PF00400
- supfam : SSF50978

1. sp|Q652L2|HIRA_ORYSJ  (Protein HIRA)
- L2 : 1.2259
- no in blast topn and no identity, super close in embedding space
- biological process : chromatin remodeling (GO:0006338), regulation of DNA-templated transcription (GO:0006355), chromatin organization (GO:0006325), DNA-templated transcription (GO:0006351)
- molecular function : protein binding (GO:0005515)
- Pfam: PF24105Beta-prop_CAF1B_HIR1 1 hit , PF07569
- SUPFAM : SSF50978
- Conclusion: the two proteins belong to the same structural superfamily according to SUPERFAMILY, supporting a remote evolutionary relationship. Also the same molecular function supports strongly this belief. Despite not be in the blast topN, the protein seems to be remote homolog with the query and the ANN algorithms work better.

2. sp|P0CS56|CMR1_CRYNJ (DNA damage-binding protein CMR1)
- L2 : 1.2439 
- no in blast topn and no identity, super close in embedding space
- molecular function : protein binding (GO:0005515)
- SUPFAM : SSF50978
- Conclusion: the two proteins belong to the same structural superfamily according to SUPERFAMILY, supporting a remote evolutionary relationship. Also the same molecular function supports strongly this belief. Despite not be in the blast topN, the protein seems to be remote homolog with the query and the ANN algorithms work better.


3. sp|Q9P4R5|CREC_EMENI (Catabolite repression protein creC)
- L2 : 1.2205
- no in blast topn and no identity, super close in embedding space
- molecular function : protein binding (GO:0005515)
- pfam :  PF00400
- supfam : SSF50978
- Conclusion : same pfam and sam supfam, superr close in embedding space, same molecular function. Remote homologs

έναν πίνακα με όλα τα κοινά Neighbor ανά query και μέθοδο, για να φαίνεται καθαρά ποιος γείτονας εμφανίζεται σε ποιες μεθόδους
