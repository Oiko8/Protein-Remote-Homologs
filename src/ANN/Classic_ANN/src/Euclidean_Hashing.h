#ifndef EUCLIDEAN_HASHING_H
#define EUCLIDEAN_HASHING_H

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

/* ============================= common definitions ========================= */
using namespace std;
using random_generator = mt19937_64;
using Table = unordered_map<int, vector<int>>;

// extern random_generator gen(42);


/* ============================== hash class ================================ */
class Hash {
    public:
    Hash(float w, int dim);
    int get_hash_id(vector<float>& p) const;
    
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


/* ======================== amplified hash class ============================= */
class AmplifiedHash {
    public:
    AmplifiedHash() = default;
    AmplifiedHash(int k, float w, int tableSize, int dim);
    unsigned int get_amplified_id(vector<float>& p) const;
    unsigned int get_point_id(vector<float>& p) const;
    int getTableSize() const;
    
    private:
    int tableS_;
    int k_;
    int dim_;
    vector<int> r_;
    unsigned int M_ = 4294967291; // exact (2^32 - 5)
    vector<Hash> h_;
    
    static int generate_r(int M);
};



/* ========================= helper functions =============================== */
void build_hash_tables(vector<vector<float>> &pts, int L, int khash, float w);
vector<int> collect_possible_nn(const vector<float> &q);
vector<int> query_knn(const vector<vector<float>> &pts, vector<float> &q, int k);
vector<int> range_search(const vector<vector<float>> &pts, vector<float> &q, float R);




/* ======================== global variables ================================ */
extern vector<AmplifiedHash> amplified_functions;
extern vector<Hash> hash_functions;
extern vector<Table> tables;
extern vector<vector<unsigned int>> point_ids;
// extern vector<vector<int>> point_bucket_ids;


#endif