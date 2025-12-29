# Protein_Remote_Homologs


### Plan
My plan : 
1. Make the structure of my project, including the ANN algorithms (LSH, HYPERCUB, INVFLAT, NEURAL LSH) from the previous assingments.  
2. Creating the embeddings of the proteins using esm2 in the file protein_embed.py 
3. Update the project 1 to take as base the generated protein vectors from step 2 and the queries so i can test how the algorithms works. for this new dataset and choose the best set of parameters. 
4. Train the model of neural LSH on the new dataset of proteins' vectors as well to find the best configuration of parameters. 
5. Making wrappers for each ANN algorithm. Four wrappers that the will run using subprocess for the algorithms written in C++ from the project 1 (LSH, HYPERCUBE, INVFLAT, INVFLATQP) and one wrapper for the neural LSH that is already in python.
6. Include neural LSH in the project, test it for the best hyperparameters. Update the graph creation to use knn-neighbors from sklearn and not the LSH.
7. Polish the wrappers for each algorithm
8. Implement search.py that calls each wrapper depending on the flag it receives.
9. Install blast, make blast running in the root directory of the project: 
  ```
  makeblastdb -in data/swissprot_50k.fasta -dbtype prot -out artifacts/blast/swissprot_db
  ```
10. Search on the blast database for the queries with command: 
  ```
  blastp -db artifacts/blast/swissprot_db -query data/targets.fasta -outfmt 6 -out artifacts/blast/blast_results.tsv
  ```

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
    blast/
      swissprot_db/        # makeblastdb outputs
      blast_results.tsv

  results/

  plots/
    HC_plot.png
    LSH_plot.png
    IVFFlat_plot.png
    IVFPQ_plot.png

  README.md
  Report.md
  requirements.txt

```