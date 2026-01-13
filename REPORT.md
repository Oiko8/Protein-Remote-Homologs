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

### 1. *Query Protein: A0A009I3Y5 (Protein kinase domain-containing protein)* 
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
  

---
---

### 2. *Query Protein: A0A009PCK4 (Protein kinase domain-containing protein)*
| Property           | Description                                                    |
| ------------------ | -------------------------------------------------------------- |
| Biological process | Protein phosphorylation (GO:0006468)                           |
| Molecular function | Protein kinase activity (GO:0004672), ATP binding (GO:0005524) |
| Cellular component | –                                                              |
| Pfam               | PF00069                                                        |
| SUPERFAMILY        | SSF56112 (Protein kinase-like)                                 |


**Candidate neighbors**

| Neighbor Protein | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                                               | Conclusion                                                                                                                                                                                                                                          |
| ---------------- | ------: | :------: | :-------: | ------------------------------------------------------------------------------------------------------------------------------------------ | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **P14181**       |  1.6715 |     –    |     No    | Same SUPERFAMILY (SSF56112, protein kinase-like); kinase-related domain                                                                    | The proteins are possibly remote homologs and that is why they are close in the embedding space. They share the same structural superfamily according to SUPERFAMILY, supporting a remote evolutionary relationship despite the lack of BLAST hits. |
| **P44033**       |  1.8396 |     –    |     No    | Kinase-related but different Pfam and Gene Ontology annotations                                                                            | **False positive.** Although it is kinase-related, the differences in Pfam and Gene Ontology descriptions weaken the homology hypothesis.                                                                                                           |
| **Q6C2A3**       |  2.0015 |   <30%   |    Yes    | Protein phosphorylation; protein kinase activity; ATP binding; protein serine/threonine kinase activity (GO:0004674); SUPERFAMILY SSF56112 | **Remote homolog.** Functional annotations and shared structural superfamily support a remote evolutionary relationship despite low sequence identity.                                                                                              |

---
---

### 3. *Query Protein: A0A001 (ABC transporter)*
| Property           | Description                                                                                                |
| ------------------ | ---------------------------------------------------------------------------------------------------------- |
| Biological process | Transmembrane transport (GO:0055085)                                                                       |
| Molecular function | ATP binding (GO:0005524), ABC-type transporter activity (GO:0140359), ATP hydrolysis activity (GO:0016887) |
| Cellular component | Membrane (GO:0016020)                                                                                      |
| Pfam               | PF00664 (ABC_membrane), PF00005 (ABC_tran)                                                                 |
| SUPERFAMILY        | SSF90123, SSF52540                                                                                         |



**Candidate neighbors**
| Neighbor Protein | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                                                | Conclusion                                                                                                                                                                                                                                                     |
| ---------------- | ------: | :------: | :-------: | ------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Q51719**       |  1.7701 |    30%   |     No    | Transmembrane transport; cobalt ion transport (GO:0006824); ATP binding; ATP hydrolysis; membrane; Pfam PF00005; SUPERFAMILY SSF52540       | **Remote homolog.** Strong agreement in transport function, ATP-related activity, membrane association, shared Pfam domain, and shared structural superfamily supports remote homology despite not appearing in the BLAST Top-N results.                       |
| **Q13BH6**       |  1.4232 |    33%   |    Yes    | Transmembrane transport; ATP binding; ABC-type transporter activity; ATP hydrolysis; membrane; Pfam PF00664 & PF00005; SUPERFAMILY SSF52540 | **Remote homolog.** Both BLAST and ANN retrieve this neighbor and the functional/domain evidence matches strongly. This is a case where both sequence- and embedding-based methods agree.                                                                      |
| **Q3JSR6**       |  1.7942 |    37%   |     No    | ATP binding; ATP hydrolysis; SUPERFAMILY SSF52540 (no matching Pfam reported)                                                               | **Remote homolog.** Even without matching Pfam, the shared ATP-binding function and shared structural superfamily support a distant evolutionary relationship. The relatively high identity suggests the relationship is real, even if BLAST ranking was weak. |

---
---

### 4. *Query Protein: A0A002 (ABC transporter ATP-binding protein)*
| Property           | Description                                                                                                |
| ------------------ | ---------------------------------------------------------------------------------------------------------- |
| Biological process | Transmembrane transport (GO:0055085)                                                                       |
| Molecular function | ATP binding (GO:0005524), ABC-type transporter activity (GO:0140359), ATP hydrolysis activity (GO:0016887) |
| Cellular component | Membrane (GO:0016020)                                                                                      |
| Pfam               | PF00664 (ABC_membrane), PF00005 (ABC_tran)                                                                 |
| SUPERFAMILY        | SSF90123, SSF52540                                                                                         |  




**Candidate neighbors**

| Neighbor Protein | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                                                          | Conclusion                                                                                                                                                                                                 |
| ---------------- | ------: | :------: | :-------: | ----------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **P9WQJ3**       |  1.3344 |    27%   |    Yes    | Transmembrane transport; ATP binding; ABC-type transporter activity; ATP hydrolysis; membrane; Pfam PF00664 & PF00005; SUPERFAMILY SSF90123, SSF52540 | **Remote homolog.** ANN, BLAST, and biological evidence (Pfam, SUPERFAMILY, Gene Ontology) all agree that the two proteins are remote homologs.                                                            |
| **Q63NR0**       |  2.0712 |    29%   |     No    | ATP binding; ATP hydrolysis; Pfam PF00005; SUPERFAMILY SSF52540                                                                                       | **Remote homolog.** Shared Pfam domain and structural superfamily plus embedding proximity support remote homology, representing a case where ANN retrieves a candidate that BLAST does not rank in Top-N. |
| **Q82MV1**       |  1.7931 |    30%   |     No    | ATP binding; ATP hydrolysis; Pfam PF00005; SUPERFAMILY SSF52540                                                                                       | **Remote homolog.** Functional similarity and conserved Pfam/SUPERFAMILY support a remote relationship even though BLAST does not return a strong ranked hit.                                              |


### 5. *Query Protein: A0A009HL96 (Response regulator)*
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

---
---

### 6. *Query Protein: A0A009HLV9 (DNA-binding transcriptional regulator NtrC)*
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
  
### 7. *Query Protein: A0A009HN45*
sp|P52126|ABPB_ECOLI
sp|Q99ZA5|UVRB_STRP1
sp|Q1JBD1|UVRB_STRPB
### 8. *Query Protein: A0A009HQC9*
sp|C5BEJ2|UVRB_EDWI9 se ola
sp|C0PWY7|UVRB_SALPC oxi se hyper 
sp|A7FKM4|UVRB_YERP3 oxi se hyper
### 9. *Query Protein: A0A009HPM0*
ivfpq kanena koino 
sp|B2TWG0|TRMA_SHIB3 → lsh, hypercube, ivfflat, neural
sp|B1LNS5|TRMA_ECOSM → lsh, hypercube, ivfflat, neural
sp|Q5PK68|TRMA_SALPA → lsh, hypercube, ivfflat, neural
### 10. *Query Protein: A0A009IB02*
hyper kanena koino 
sp|Q9SQI8|ODP24_ARATH
sp|P49786|BCCP_BACSU
sp|P0ABE1|BCCP_SHIFL
### 11. *Query Protein: A0A010Q3W2*
sp|Q2UUT4|CMR1_ASPOR
sp|A2QI22|YTM1_ASPNC oxi lsh
sp|P0CS56|CMR1_CRYNJ oxi ivfpq

---
---
### 12. *Query Protein: A0A010Q8R4*
| Property           | Description                                                                   |
| ------------------ | ----------------------------------------------------------------------------- |
| Molecular function | Protein binding (GO:0005515); transcription corepressor activity (GO:0003714) |
| Pfam               | PF08512, PF00400                                                              |
| SUPERFAMILY        | SSF50978                                                                      |



**Candidate neighbors**

| Neighbor Protein | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                    | Conclusion                                                                                                                                                |
| ---------------- | ------: | :------: | :-------: | --------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Q652L2**       |  1.2259 |     –    |     No    | Chromatin remodeling and transcription regulation; protein binding; Pfam PF24105, PF07569; SUPERFAMILY SSF50978 | **Remote homolog.** Shared structural superfamily and overlapping functional annotations explain close embedding proximity despite absence of BLAST hits. |
| **P0CS56**       |  1.2439 |     –    |     No    | Protein binding; SUPERFAMILY SSF50978                                                                           | **Remote homolog.** Structural superfamily conservation supports a remote evolutionary relationship even when BLAST provides no evidence.                 |
| **Q9P4R5**       |  1.2205 |     –    |     No    | Protein binding; Pfam PF00400; SUPERFAMILY SSF50978                                                             | **Remote homolog.** Shared Pfam domain and shared structural superfamily provide strong evidence of remote homology.                                      |

---
---
έναν πίνακα με όλα τα κοινά Neighbor ανά query και μέθοδο, για να φαίνεται καθαρά ποιος γείτονας εμφανίζεται σε ποιες μεθόδους
