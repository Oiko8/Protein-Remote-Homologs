# Protein Remote Homolog Search with ESM-2 and ANN
---

---
## Introduction

Traditional sequence alignment tools such as BLAST are highly effective for detecting close and medium-range protein homologs. However, they often fail in the Twilight Zone of sequence similarity (identity < 30%), where proteins may share conserved structure and function despite strong sequence divergence.  

In this project, we address the problem of remote homolog detection by combining:  

- Protein embeddings generated with the pretrained ESM-2 protein language model, and
- Approximate Nearest Neighbor (ANN) search algorithms adapted from previous assignments.  

By searching in the embedding space rather than directly in sequence space, we evaluate whether ANN methods can retrieve biologically meaningful neighbors that are weakly detected or completely missed by BLAST. The ANN results are quantitatively compared against BLAST and qualitatively validated using biological annotations (Pfam, GO, SUPERFAMILY).

---
## Datasets and Project Structure

### Datasets

The project uses the following datasets:  

- `swissprot.fasta`  
A curated protein sequence database used to build the embedding index.

- `targets.fasta`  
A smaller set of query proteins used for ANN search and BLAST comparison.

- `BLAST database`  
Built locally from the SwissProt dataset and used as a reference (ground truth).

### Structure
```
project3/
  src/
    protein_embed.py
    protein_search.py

    ann_wrappers/
      lsh_wrapper.py
      hypercube_wrapper.py
      ivfflat_wrapper.py
      ivfpq_wrapper.py
      neural_wrapper.py

    utils/
      args_parser.py

    ANN/
      Classic_ANN/
      Neural_LSH/

  data/
    # swissprot.fasta
    # targets.fasta

  artifacts/
    embeddings/
      protein_vectors.dat
      protein_queries.dat
      ids.txt
    indices/
      protein_ids
      queries_ids
    blast/
      swissprot_db/        # makeblastdb outputs
      blast_results.tsv

  results/

  reports/

  plots/
    HC_plot.png
    LSH_plot.png
    IVFFlat_plot.png
    IVFPQ_plot.png

  README.md
  REPORT.md
  ANN_Report.md
  requirements.txt

```

---
## Implementation

The project is organized into two main execution stages:  

### 1. Protein Embedding Generation

- Implemented in `protein_embed.py`

- Uses the pretrained model: `facebook/esm2_t6_8M_UR50D`

- For each protein: 
  - The amino-acid sequence is tokenized
  - Representations from the final transformer layer are extracted
  - Mean pooling is applied (excluding special tokens)

- Embeddings are stored in fvecs-style binary format, compatible with ANN algorithms

- Protein IDs are stored separately for index-to-protein mapping  

This stage converts biological sequences into a high-dimensional vector space suitable for similarity search.

--- 

### 2. ANN Search and BLAST Comparison

- Implemented in `protein_search.py`

- Supports the following ANN methods:
  - Euclidean LSH
  - Hypercube Projection
  - IVF-Flat
  - IVF-PQ
  - Neural LSH

- Each method is wrapped via Python wrappers that call either:
  - C++ implementations from previous assignments, or
  - A Python Neural LSH implementation

- For each query protein:
  1. ANN retrieves the Top-N nearest neighbors in embedding space

  2. BLAST is executed against the same protein database

  3. Recall@N is computed using BLAST Top-N as ground truth

  4. Runtime metrics (time/query, QPS) are recorded

  5. Neighbors with low BLAST identity (< 30%) are highlighted as candidate remote homologs  

All outputs are formatted according to the assignment specification and saved in the [reports/](reports/) directory.

---


## Compilation and Execution

### Environment Setup

Install dependencies:

```pip install -r requirements.txt```


Make sure BLAST+ is installed and available in your PATH.

### Generate Protein Embeddings

```
python src/protein_embed.py \
  -i data/swissprot.fasta \
  -o artifacts/embeddings/protein_vectors.dat
```
This produces:
- protein_vectors.dat (embeddings)
- protein_ids (index-to-protein mapping)


### Build BLAST Database and Run BLAST Queries

```
makeblastdb \
  -in data/swissprot.fasta \
  -dbtype prot \
  -out artifacts/blast/swissprot_db

```
```
blastp \
  -db artifacts/blast/swissprot_db \
  -query data/targets.fasta \
  -outfmt 6 \
  -out artifacts/blast/blast_results.tsv

```

### Run ANN Search

```
python src/protein_search.py \
  -d artifacts/embeddings/protein_vectors.dat \
  -q artifacts/embeddings/protein_queries.dat \
  -o results/{ANN algorithm}.txt \
  -method {ANN algorithm} \
  -N 50

```
- Options for ANN algorithm: `lsh` , `ivfflat`, `hypercube` , `ivfpq`, `neural`
- Each method produces:
  - Neighbor lists
  - Performance metrics
  - Detailed per-query reports

---

## Report

The project includes two complementary reports:

### [1. ANN_Report.md](ANN_Report.md)

- Focuses on algorithmic evaluation

- Hyperparameter tuning for each ANN method

- Trade-offs between Recall@N and QPS

- Comparative performance plots


### [2. REPORT.md](REPORT.md)

- Focuses on biological evaluation

- Defines and analyzes remote homologs

- Detailed Twilight Zone case studies

- Manual validation using Pfam, GO, and SUPERFAMILY annotations

- Discussion of false positives and method limitations

 
---
### *Sources*
- https://blast.ncbi.nlm.nih.gov/Blast.cgi
- https://www.uniprot.org/
- https://www.ebi.ac.uk/interpro/
- [Dive into Deep Learning](https://d2l.ai/)