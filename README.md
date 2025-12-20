# Protein_Remote_Homologs


### Structure
```
project3/
  src/
    protein_embed.py
    protein_search.py

    ann/
      __init__.py
      base_wrapper.py
      lsh_wrapper.py
      hypercube_wrapper.py
      ivfflat_wrapper.py
      ivfpq_wrapper.py
      neural_wrapper.py

    embedding/
      __init__.py
      esm2_embedder.py

    utils/
      __init__.py
      fasta.py
      fvecs.py
      ids.py
      blast.py
      timing.py
      parsing.py

    ANN/
      Classic_ANN/
        CMakeLists.txt (ή Makefile)
        include/
        src/
          utils_functions/
        bin/               # binaries
          lsh_main
          hypercube_main
          ivfflat_main
          ivfpq_main

      Neural_LSH/
        # expose Python API ή call it from neural_wrapper

  data/
    # swissprot.fasta
    # targets.fasta

  artifacts/
    embeddings/
      protein_vectors.fvecs
      ids.txt
    indices/
      lsh/
      hypercube/
      ivf/
      neural/
    blast/
      swissprot_db/        # makeblastdb outputs
      blast_results.tsv

  results/
    runs/
      2025-12-20_001/
        results.txt
        summary.csv
        params.json
    plots/ 

  README.md
  Report.md
  requirements.txt

```