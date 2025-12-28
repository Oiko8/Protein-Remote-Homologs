#pragma once
#include "vectors.h"
#include <vector>
#include <cstdint>

// PQCode: kataxwrisi kodikou gia ena vector (M codes, ena byte ka8e ena)
struct PQCode {
    std::vector<uint8_t> codes; // M bytes (ena ana subvector)
};

// IVFPQ struct
struct IVFPQ {
    size_t kclusters = 0;                   // IVF clusters
    size_t d = 0;                           // diastash
    size_t M = 0;                           // posoi sub-vectors
    size_t nbits = 8;                       // bits ana subvector
    size_t K = 256;                         // clusters ana subvector (1<<nbits)
    size_t subd = 0;                        // diastash ana subvector (padded)
    std::vector<float> ivf_centroids;       // kclusters * d
    std::vector<float> pq_codebooks;        // M * K * subd  (flat)
    std::vector<std::vector<std::pair<uint32_t, PQCode>>> lists; // inverted lists : gia kathe ivf cluster lista apo (id, pqcode)

    // training kai indexing
    void train(const Dataset &ds, size_t kclusters_, size_t M_, size_t nbits_, unsigned seed=1);
    void index_dataset(const Dataset &ds);

    // search : epistrefei top-N (id, squaredDist approx)
    std::vector<std::pair<uint32_t, float>> search(const float* q, const Dataset &ds, size_t N, size_t nprobe, float R = -1.0f) const;
};
