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
| Property           | Description                                                                                                     |
| ------------------ | --------------------------------------------------------------------------------------------------------------- |
| Biological process | Phosphorelay signal transduction system (GO:0000160); regulation of DNA-templated transcription (GO:0006355)    |
| Molecular function | DNA binding (GO:0003677); transcription cis-regulatory region binding (GO:0000976)                               |
| Cellular component | NONE                                                                                                           |
| Pfam               | PF00072 Response_reg, PF00486 Trans_reg_C                                                                      |
| SUPERFAMILY        | SSF46894, SSF52172                                                                                             |

**Candidate neighbors**

| Neighbor Protein              | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                    | Conclusion                                                                                                                                                |
| ----------------------------- | ------: | :------: | :-------: | --------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Q9AE24** (RPRY_BACFR)      | 0.7792  | 28%      | Yes       | Phosphorelay signal transduction; regulation of DNA-templated transcription; DNA binding; transcription cis-regulatory region binding; Pfam PF00072, PF00486; SUPERFAMILY SSF46894, SSF52172 | **Remote homolog.** Both the ANN and the BLAST along with the Pfam, SUPERFAMILY, and GO descriptions agree that the two proteins are remote homologs. |
| **Q5HI09** (GRAR_STAAC)      | 0.6392  | 26%      | Yes       | Phosphorelay signal transduction; regulation of DNA-templated transcription; DNA binding; transcription cis-regulatory region binding; Pfam PF00072, PF00486; SUPERFAMILY SSF46894, SSF52172 | **Remote homolog.** Both the ANN and the BLAST along with the Pfam, SUPERFAMILY, and GO descriptions agree that the two proteins are remote homologs. |
| **Q99VW2** (GRAR_STAAN)      | 0.6598  | 26%      | Yes       | Phosphorelay signal transduction; regulation of DNA-templated transcription; DNA binding; transcription cis-regulatory region binding; Pfam PF00072, PF00486; SUPERFAMILY SSF46894, SSF52172 | **Remote homolog.** Both the ANN and the BLAST along with the Pfam, SUPERFAMILY, and GO descriptions agree that the two proteins are remote homologs. |


### 6. *Query Protein: A0A009HLV9 (DNA-binding transcriptional regulator NtrC)*
| Property           | Description                                                                                                           |
| ------------------ | --------------------------------------------------------------------------------------------------------------------- |
| Biological process | Phosphorelay signal transduction system (GO:0000160); regulation of DNA-templated transcription (GO:0006355); regulation of nitrogen utilization (GO:0006808) |
| Molecular function | DNA binding (GO:0003677); ATP binding (GO:0005524); transcription factor binding (GO:0008134); sequence-specific DNA binding (GO:0043565); phosphorelay response regulator activity (GO:0000156) |
| Cellular component | NONE                                                                                                                 |
| Pfam               | PF25601 AAA_lid_14, PF02954 HTH_8, PF00072 Response_reg, PF00158 Sigma54_activat                                      |
| SUPERFAMILY        | SSF52172, SSF46689, SSF52540                                                                                          |

**Candidate neighbors**

| Neighbor Protein              | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                                 | Conclusion                                                                                                                                                      |
| ----------------------------- | ------: | :------: | :-------: | ---------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **A1KU52** (RUVB_NEIMF)      | 1.0106  | –        | No        | DNA repair; DNA recombination; DNA binding; ATP binding; four-way junction helicase activity; Pfam PF17864, PF05491, PF05496; SUPERFAMILY SSF46894, SSF52172 | **False positive.** ANN retrieves the neighbor due to shared generic structural features, such as ATP-binding, rather than true evolutionary homology. The lack of shared Pfam domains and distinct biological functions is consistent with BLAST. |
| **A8A3I6** (NORR_ECOHS)      | 0.9989  | 38%      | Yes       | Regulation of DNA-templated transcription; protein binding; ATP binding; transcription factor binding; DNA-binding transcription factor activity; Pfam PF01590, PF00158; SUPERFAMILY SSF55781, SSF46689, SSF52540 | **Conventional homolog.** The neighbor is retrieved by ANN due to shared regulatory features, which are also detected by BLAST. Overlapping Pfam and GO annotations and 38% BLAST identity support conventional homology. |
| **P66756** (RUVB_SALTI)      | 1.0884  | –        | No        | DNA repair; DNA recombination; four-way junction helicase activity; ATP binding; DNA binding; Pfam PF17864, PF05491, PF05496; SUPERFAMILY SSF46785, SSF52540 | **Distant / analogous.** ANN detects this neighbor likely due to functional or structural similarity, while BLAST misses it because of low sequence identity. This indicates a distant or analogous relationship rather than a true homolog, highlighting the different sensitivities of the methods. |


  
### 7. *Query Protein: A0A009HN45 (DEAD/DEAH box helicase family protein)*
| Property           | Description                                                                 |
| ------------------ | --------------------------------------------------------------------------- |
| Biological process | NONE                                                                        |
| Molecular function | ATP binding (GO:0005524)                                                    |
| Cellular component | NONE                                                                        |
| Pfam               | PF00271 Helicase_C, PF00176 SNF2-rel_dom                                     |
| SUPERFAMILY        | SSF52540                                                                    |

**Candidate neighbors**

| Neighbor Protein              | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                  | Conclusion                                                                                                                                        |
| ----------------------------- | ------: | :------: | :-------: | ------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **P52126** (ABPB_ECOLI)      | 0.7920  | –        | No        | Nucleic acid binding; ATP binding; Pfam PF00270, PF00271; SUPERFAMILY SSF52540                               | **Remote homolog.** Shared helicase-related Pfam domains and ATP-binding activity support remote homology, despite no BLAST match.             |
| **Q99ZA5** (UVRB_STRP1)      | 0.8060  | –        | No        | Protein binding; DNA binding; ATP binding; ATP hydrolysis; hydrolase activity; excinuclease repair complex; Pfam PF00271, PF04851, PF02151, PF12344, PF17757; SUPERFAMILY SSF52540, SSF46600 | **Remote homolog.** Shared helicase-related domains and ATP-binding indicate a conserved functional and structural core, captured by ANN.        |
| **Q0HMR2** (RAPA_SHESM)      | 0.7921  | 30%      | Yes       | ATP binding; hydrolase activity; Pfam PF00271, PF12137, PF00176, PF18339, PF18337; SUPERFAMILY SSF52540      | **Remote homolog.** Shared helicase-related domains and ATP-binding function suggest a conserved core, detected by both ANN and BLAST.         |


### 8. *Query Protein: A0A009HQC9 (RNA polymerase-associated protein RapA)*
| Property           | Description                                                                                           |
| ------------------ | ----------------------------------------------------------------------------------------------------- |
| Biological process | Regulation of DNA-templated transcription (GO:0006355)                                                |
| Molecular function | ATP binding (GO:0005524); hydrolase activity acting on acid anhydrides (GO:0016817)                   |
| Cellular component | NONE                                                                                                  |
| Pfam               | PF00271 Helicase_C, PF00176 SNF2-rel_dom, PF12137 RapA_C, PF18339 Tudor_1_RapA, PF18337 Tudor_RapA  |
| SUPERFAMILY        | SSF52540                                                                                               |

**Candidate neighbors**

| Neighbor Protein              | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                  | Conclusion                                                                                                                                        |
| ----------------------------- | ------: | :------: | :-------: | ------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **C5BEJ2** (UVRB_EDWI9)      | 0.9428  | –        | No        | Protein binding; DNA binding; ATP binding; ATP hydrolysis activity; hydrolase activity; excinuclease repair complex; Pfam PF00271, PF04851, PF02151, PF12344, PF17757; SUPERFAMILY SSF52540, SSF46600 | **Remote homolog.** Shared helicase-related domains and ATP-binding/hydrolase functions indicate a conserved functional core, captured by ANN despite BLAST not detecting it. |
| **C0PWY7** (UVRB_SALPC)      | 0.9867  | –        | No        | Protein binding; DNA binding; ATP binding; ATP hydrolysis activity; hydrolase activity; excinuclease repair complex; Pfam PF00271, PF04851, PF02151, PF12344, PF17757; SUPERFAMILY SSF52540, SSF46600 | **Remote homolog.** Helicase-related domains with ATP-binding and hydrolase activity reveal a conserved functional core, effectively identified by ANN even though BLAST misses it. |
| **A7FKM4** (UVRB_YERP3)      | 0.9403  | –        | No        | Protein binding; DNA binding; ATP binding; ATP hydrolysis activity; hydrolase activity; excinuclease repair complex; Pfam PF00271, PF04851, PF02151, PF12344, PF17757; SUPERFAMILY SSF52540, SSF46600 | **Remote homolog.** Helicase-related domains and ATP-binding/hydrolase functions reveal a conserved functional core, captured more reliably by ANN despite BLAST failing. |

### 9. *Query Protein: A0A009HPM0 (Biotin carboxylase)*
| Property           | Description                                                                                           |
| ------------------ | ----------------------------------------------------------------------------------------------------- |
| Biological process | NONE                                                                                                  |
| Molecular function | ATP binding (GO:0005524); metal ion binding (GO:0046872)                                             |
| Cellular component | NONE                                                                                                  |
| Pfam               | PF02785 Biotin_carb_C, PF00289 Biotin_carb_N, PF00364 Biotin_lipoyl, PF02786 CPSase_L_D2             |
| SUPERFAMILY        | SSF56059, SSF52440, SSF51246, SSF51230                                                               |

**Candidate neighbors**

| Neighbor Protein              | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                  | Conclusion                                                                                                                                        |
| ----------------------------- | ------: | :------: | :-------: | ------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **B2TWG0** (TRMA_SHIB3)      | 1.4365  | –        | No        | RNA processing; RNA methyltransferase activity; tRNA (uracil(54)-C5)-methyltransferase; S-adenosyl methionine-dependent; Pfam PF05958; SUPERFAMILY SSF53335 | **False positive.** Despite being retrieved by the ANN algorithm, the complete lack of shared Pfam domains, superfamilies, and biological functions indicates a false positive, which BLAST correctly excludes by sequence-based similarity. |
| **B1LNS5** (TRMA_ECOSM)      | 1.4492  | –        | No        | RNA processing; RNA methyltransferase activity; tRNA (uracil(54)-C5)-methyltransferase; S-adenosyl methionine-dependent; Pfam PF05958; SUPERFAMILY SSF53335 | **False positive.** Even though it is retrieved by the ANN method, the lack of common domains, superfamily membership, and functional similarity suggests a spurious match that BLAST appropriately filters out based on sequence similarity. |
| **Q9CMJ8** (RIMK_PASMU)      | 1.3521  | –        | No        | Protein modification process; ATP binding; metal ion binding; Pfam PF08443, PF18030; SUPERFAMILY SSF56059   | **False positive.** Despite ANN detection based on generic ATP-binding features, the lack of shared Pfam domains and functional divergence supports a false positive that BLAST correctly filters out. |

### 10. *Query Protein: A0A009IB02 (Dihydrolipoamide acetyltransferase component of pyruvate dehydrogenase complex)*
| Property           | Description                                                                                           |
| ------------------ | ----------------------------------------------------------------------------------------------------- |
| Biological process | NONE                                                                                                  |
| Molecular function | Acyltransferase activity (GO:0016746)                                                                |
| Cellular component | NONE                                                                                                  |
| Pfam               | PF00198 2-oxoacid_dh, PF00364 Biotin_lipoyl, PF02817 E3_binding                                       |
| SUPERFAMILY        | SSF52777, SSF51230, SSF47005                                                                         |

**Candidate neighbors**

| Neighbor Protein              | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                  | Conclusion                                                                                                                                        |
| ----------------------------- | ------: | :------: | :-------: | ------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Q9SQI8** (ODP24_ARATH)     | 2.0060  | 29%      | Yes       | Pyruvate decarboxylation to acetyl-CoA; acyltransferase activity; pyruvate dehydrogenase complex; Pfam PF00198, PF00364, PF02817; SUPERFAMILY SSF52777, SSF51230, SSF47005 | **Conventional homolog.** Clear homolog rather than a remote one. The strong overlap in Pfam domains, superfamily membership, and enzymatic function, together with BLAST detection, indicates a conserved and well-established evolutionary relationship. |
| **P49786** (BCCP_BACSU)      | 2.0085  | –        | No        | Fatty acid biosynthetic process; acetyl-CoA carboxylase activity; acetyl-CoA carboxylase complex; Pfam PF00364; SUPERFAMILY SSF51230 | **False positive.** ANN detects it due to the shared Biotin_lipoyl domain, but the limited domain overlap and functional differences indicate a false positive that BLAST correctly ignores. |
| **P11961** (ODP2_GEOSE)      | 1.9830  | 38%      | Yes       | Acyltransferase activity; Pfam PF00198, PF00364, PF02817; SUPERFAMILY SSF52777, SSF51230, SSF47005             | **Remote homolog.** Both BLAST and ANN captured this as remote homolog due to shared Pfam domains, superfamily membership, and acyltransferase function. |

### 11. *Query Protein: A0A010Q3W2 (U3 small nucleolar RNA-associated protein 15 C-terminal domain-containing protein)*
| Property           | Description                                                                                           |
| ------------------ | ----------------------------------------------------------------------------------------------------- |
| Biological process | rRNA processing (GO:0006364)                                                                          |
| Molecular function | Protein binding (GO:0005515)                                                                          |
| Cellular component | Nucleolus (GO:0005730)                                                                                |
| Pfam               | PF09384 UTP15_C, PF00400 WD40                                                                         |
| SUPERFAMILY        | SSF50978                                                                                               |

**Candidate neighbors**

| Neighbor Protein              | L2 Dist | Identity | BLAST hit | Functional / Domain Evidence                                                                                  | Conclusion                                                                                                                                        |
| ----------------------------- | ------: | :------: | :-------: | ------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Q2UUT4** (CMR1_ASPOR)      | 1.1791  | –        | No        | Protein binding; Pfam PF00400; SUPERFAMILY SSF50978                                                         | **False positive.** Despite ANN detection via the shared WD40 domain and superfamily, the lack of overlapping biological process and low sequence similarity suggest a spurious match. |
| **A2QI22** (YTM1_ASPNC)      | 1.2346  | –        | No        | Ribosome biogenesis; protein binding; Pfam PF00400, PF08154; SUPERFAMILY SSF50978                             | **False positive.** Despite ANN detection via the shared WD40 domain and superfamily, the lack of overlapping biological process and low sequence similarity suggest a spurious match. |
| **Q9NRL3** (STRN4_HUMAN)     | 1.3099  | 28%      | No        | Protein binding; Pfam PF00400, PF08232; SUPERFAMILY SSF50978                                                | **False positive.** While ANN picks up the common WD40 domain there is low sequence identity and different biological roles.                        |


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

## Conclusion

In this assignment, we investigated the problem of protein homology detection with an emphasis on remote homologs, comparing traditional sequence-based approaches (BLAST) with embedding-based Approximate Nearest Neighbor (ANN) methods. The results clearly demonstrate that these two approaches are complementary rather than competitive, each excelling in different regions of the homology detection space.

BLAST remains the gold standard for identifying conventional homologs, offering high precision when sufficient sequence similarity is present. However, as confirmed by the Twilight Zone analysis (sequence identity < 30%), its sensitivity drops substantially for distantly related proteins whose sequences have diverged while their structural or functional cores remain conserved.

In contrast, ANN methods operating in the embedding space successfully retrieved many biologically meaningful candidates that were weakly detected or entirely missed by BLAST. Manual biological validation using Pfam domains, SUPERFAMILY classifications, and Gene Ontology annotations showed that, in numerous cases—such as protein kinases, ABC transporters, response regulators, and helicase-related proteins—embedding proximity reflects conserved structural and functional features, supporting true remote evolutionary relationships.

At the same time, the analysis highlighted the limitations of embedding-based approaches. Proteins sharing generic motifs or broad functional features (e.g., ATP-binding, WD40 repeats) occasionally led to false positives, where embedding similarity did not correspond to true homology. In such cases, BLAST effectively filtered out spurious matches based on sequence-level evidence, underscoring the importance of biological validation.

From a performance perspective, ANN algorithms achieved orders-of-magnitude faster query times compared to BLAST, enabling high-throughput similarity searches and large-scale exploratory analyses. Although ANN recall with respect to BLAST Top-N results is lower by design, this does not represent a weakness when the objective is the discovery of novel or difficult-to-detect remote homologs rather than reproducing sequence-based rankings.

Overall, the findings indicate that BLAST is best suited for reliable detection of close and moderately distant homologs, while embedding-based ANN methods provide a powerful means to explore the remote homology landscape, particularly within the Twilight Zone. A combined workflow—using ANN for candidate discovery and BLAST together with biological annotations for validation—emerges as an effective and biologically meaningful strategy for comprehensive protein homology analysis.
