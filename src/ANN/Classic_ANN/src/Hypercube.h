#ifndef HYPERCUBE_H
#define HYPERCUBE_H

#include <iostream>
#include <cstdlib>    
#include <random>      
#include <ctime>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <queue>
#include <climits>
#include "utils_functions/euclid.h"

/* ===================================== common definitions ====================================== */
using namespace std;
using random_generator = mt19937_64;
using Table = unordered_map<string, vector<int>>;

/* ======================================== hash class =========================================== */
class Hash_hc {
    public:
    Hash_hc(float w, int dim);
    int get_hash_id(const vector<float>& p) const;
    
    private:
    vector<float> v_;
    float t_;
    float w_;
    int dim_;
    
    // helpers (declared here, defined in .cpp)
    vector<float> vec_d();
    static float normal_distribution_generator();
    static float generate_t(float w);
    static float dot(const vector<float>& v, const vector<float>& p);
};



/* ================================== Bit Mapper Class Implementation ============================ */
// functions f family to convert a hash function result to 0 or 1
// fi(hi) = {0,1}

class BitMapper {
public:
    // k = number of hypercube bits (same as your k'), seed for determinism
    BitMapper(int k, uint32_t seed = 1);

    // Map the j-th hash output h (an integer) to {0,1}
    // 0 <= j < k
    inline uint32_t bit_for(int j, int h) const {
        // Fast, stateless per-call mix using per-bit random A[j], B[j]
        uint32_t u = static_cast<uint32_t>(h);
        uint32_t x = A_[j] * u + B_[j];
        return mix32(x) & 1u; // 0 or 1
    }

    inline int bits() const { return k_; }

private:
    int k_;
    vector<uint32_t> A_, B_; // per-bit random coefficients

    static inline uint32_t mix32(uint32_t x){
        // 32-bit mixer (xorshift* / wyhash-inspired)
        x ^= x >> 16; // XOR the high 16 bits into low 16 bits
        x *= 0x7feb352dU; // multiply with 2^32
        x ^= x >> 15;  // xor and shift again with diff amount
        x *= 0x846ca68bU; // constant chose by mixers like wihash
        x ^= x >> 16; // final xorshift
        return x;
    }
};


/* ==================================== Build & Query API ======================================== */
// build the hypercube table
void build_hypercube(const vector<vector<float>> &pts, int kproj, float w, uint32_t seed = 1);

// KNN / Range (M = per-vertex; probes = count of vertices to visit)
vector<int> cube_query_knn(const vector<vector<float>> &pts, const vector<float> &q, int N, int M, int probes);
vector<int> cube_range_search(const vector<vector<float>> &pts, const vector<float> &q, float R, int M, int probes);


/* =================================== Helper Function ========================================== */
// Compute the k-bit vertex for a point with current hash_functions_hc/bit_function
string vertex_for_point(const vector<float> &p);

// Enumerate all masks with Hamming weight <= radius, in increasing weight
vector<uint32_t> neighbor_masks_within_radius(int k, int radius);

// Count how many vertices exist within Hamming radius r
long long vertices_within_radius(int k, int r);

// Enumerate up to 'probes' masks prioritized by bucket size around 'base'
vector<uint32_t> neighbor_masks_top_by_bucket(const string& base, int k, int probes);



/* ======================== global variables ================================ */
extern vector<Hash_hc> hash_functions_hc;
extern vector<BitMapper> bit_functions;
extern Table Hypercube_table;

#endif