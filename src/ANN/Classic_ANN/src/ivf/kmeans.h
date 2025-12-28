#pragma once    // gia apofygh pollaplwn eisagwgwn tou idiou header
#include "vectors.h"    // Eisagwgh tou arxeiou vectors.h pou periexei typous kai leitourgies gia vectors
#include <vector>   // Eisagwgh tou standard vector container apo thn STL
#include <random>   // Eisagwgh tou random gia tuxaious arithmous

// Orismos struct KMeans gia tin ylopoihsh tou algorithimou k-means clustering
// epistrefei ta centroids ws flat vector (k * d)
struct KMeans {
    static std::vector<float> train(const Dataset &ds, size_t k, unsigned seed=1, size_t max_iters=100, float tol=1e-4);
    // H train, pairnei dataset, ton arithmo clusters k, tuxaio seed, megisto ari8mo epanalipsewn kai tolerance
    // & epistrefei ta centroids meta apo ton algorithmo k-means

    static void assign_all(const Dataset &ds, const std::vector<float> &centroids, std::vector<uint32_t> &assignments);
    // H assign_all, pairnei dataset kai centroids
    // & anakoinwnei gia kathe deigma poio centroid tou antistoixei (assignments)
};
