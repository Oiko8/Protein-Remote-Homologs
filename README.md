# Protein_Remote_Homologs


### Plan
My plan : 
1. Make the structure of my project, including the ANN algorithms (LSH, HYPERCUB, INVFLAT, NEURAL LSH) from the previous assingments.  
2. Creating the embeddings of the proteins using esm2 in the file protein_embed.py 
3. Update the project 1 to take as base the generated protein vectors from step 2 and the queries so i can test how the algorithms works. for this new dataset and choose the best set of parameters. 
4. Train the model of neural LSH on the new dataset of proteins' vectors as well to find the best configuration of parameters. 
5. Making wrappers for each ANN algorithm. Four wrappers that the will run using subprocess for the algorithms written in C++ from the project 1 (LSH, HYPERCUBE, INVFLAT, INVFLATQP) and one wrapper for the neural LSH that is already in python.
6. Include neural LSH in the project, test it for the best hyperparameters. Update the graph creation to use knn-neighbors from sklearn and not the LSH.

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

  plots/
    HC_plot.png
    LSH_plot.png
    IVFFlat_plot.png
    IVFPQ_plot.png

  README.md
  Report.md
  requirements.txt

```