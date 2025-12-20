#include "Hypercube.h"

/*=================================== Global definitions =========================================*/
random_generator gen_hc(42);
vector<Hash_hc> hash_functions_hc;
vector<BitMapper> bit_functions;
Table Hypercube_table;


/* =============================================================================================== */
/* ================================== Hash_hc Class Implementation ================================== */
/* =============================================================================================== */

Hash_hc::Hash_hc(float w, int dim) {
    w_ = w;
    dim_ = dim;
    t_ = generate_t(w_);
    v_ = vec_d();
} 
    
int Hash_hc::get_hash_id(const vector<float>& p) const {
    int h = static_cast<int>(floor((dot(v_, p) + t_) / w_));
    return h;
}

/* ======================= Helper functions ============================ */
/* ========= Creating a 2-vector with normal disrtibution ============== */

vector<float> Hash_hc::vec_d(){
    vector<float> v(dim_);
    for (int i=0 ; i < dim_ ; i++){
        v[i] = normal_distribution_generator();
    }
    return v;
}

/* ========= return a point with normal distribution =================== */
float Hash_hc::normal_distribution_generator(){
    normal_distribution<float> dist(0.0, 1.0);
    return dist(gen_hc);
}

/* ========== generate a small disturbance t =========================== */
float Hash_hc::generate_t(float w) {
    // keep your original definition (Uniform[0, w])
    uniform_real_distribution<float> dist(0.0, w);
    return dist(gen_hc);
}

float Hash_hc::dot(const vector<float>& v, const vector<float>& p){
    float dot_product = 0.0;
    int size = v.size();
    for (int i=0 ; i < size ; i++){
        dot_product += v[i]*p[i];
    } 
    return dot_product;
}

  

/* =============================================================================================== */
/* ================================== Bit Mapper Class Implementation ============================ */
/* =============================================================================================== */

BitMapper::BitMapper(int k, uint32_t seed)
: k_(k), A_(k), B_(k)
{
    mt19937 rng(seed);
    uniform_int_distribution<uint32_t> u32;
    for (int j = 0; j < k_; ++j) {
        A_[j] = (u32(rng) | 1u); // odd -> better spreading
        B_[j] = (u32(rng) | 1u);
    }
}


/* =============================================================================================== */
/* ===================================== Build the hypercude ===================================== */
/* =============================================================================================== */

void build_hypercube(const vector<vector<float>> &pts, int k, float w, uint32_t seed){
    int dim = static_cast<int>(pts[0].size());

    // (re)seed global RNG and reset state for rebuilds
    gen_hc.seed(seed);
    hash_functions_hc
.clear();
    Hypercube_table.clear();
    bit_functions.clear();

    for (int i = 0 ; i < k ; i++) {
        Hash_hc new_h(w, dim);
        hash_functions_hc
    .push_back(new_h);

        bit_functions.emplace_back(1, seed + i);
    }


    for (int idx = 0 ; idx < static_cast<int>(pts.size()) ; idx++) {
        string vertex = "";
        for (int i = 0 ; i < k ; i++) {
            int h_id = hash_functions_hc
        [i].get_hash_id(pts[idx]);
            auto bit = bit_functions[i].bit_for(0, h_id);
            vertex += (bit ? "1" : "0");
        }
        Hypercube_table[vertex].push_back(idx);
    }
}


/* =============================================================================================== */
/* ===================================== NN and Range Search ===================================== */
/* =============================================================================================== */


/* ============================== Helper: compute a vertex for a query =====================================  */

string vertex_for_point(const vector<float> &p){
    int k = (int)bit_functions.size();
    string v;
    v.reserve(k);
    for (int i = 0; i < k; ++i){
        int h_id = hash_functions_hc
    [i].get_hash_id(p);
        uint32_t b = bit_functions[i].bit_for(0, h_id);
        v += (b ? '1' : '0');
    }
    return v;
}

// /* =================================== Helper: Hamming-order neighbor masks ================================ */


static inline void flip_in_place(string &v, uint32_t mask){
    int k = static_cast<int>(v.size());
    for (int j = 0; j < k; ++j){
        if (mask & (1u << j)){
            v[j] = (v[j] == '0') ? '1' : '0';
        }
    }
}

static inline string xor_vertex_with_mask(const string &v, uint32_t mask){
    string out = v;
    flip_in_place(out, mask);
    return out;
}

// long long vertices_within_radius(int k, int r) {
//     if (k <= 0 || r < 0) return 0;
//     if (r > k) r = k;
//     long long sum = 0, C = 1; // C(k,0)
//     for (int i = 0; i <= r; ++i) {
//         if (i > 0) C = C * (k - (i - 1)) / i;
//         sum += C;
//     }
//     return sum;
// }

// Build best-first mask list up to 'probes' by bucket size.
// Order: weight 0 (the base), then best Hamming-1 by bucket size,
// then (if needed) best Hamming-2 ranked by estimated size.
vector<uint32_t> neighbor_masks_top_by_bucket(const string& base, int k, int probes){
    vector<uint32_t> out;
    out.reserve(probes);
    if (probes <= 0) return out;

    // always include base (mask 0)
    out.push_back(0u);
    if ((int)out.size() >= probes) return out;

    struct Item { uint32_t mask; size_t score; };
    // Hamming-1
    vector<Item> w1;
    w1.reserve(k);
    for (int j = 0; j < k; ++j){
        uint32_t m = (1u << j);
        string v1 = xor_vertex_with_mask(base, m);
        size_t sz = 0;
        auto it = Hypercube_table.find(v1);
        if (it != Hypercube_table.end()) sz = it->second.size();
        w1.push_back({m, sz});
    }
    sort(w1.begin(), w1.end(), [](const Item& a, const Item& b){ return a.score > b.score; });

    for (auto &it : w1){
        if ((int)out.size() >= probes) break;
        out.push_back(it.mask);
    }
    if ((int)out.size() >= probes) return out;

    // Hamming-2 (choose top pairs from the strongest w1 entries)
    vector<Item> w2;
    int top1 = min(k, max(2, probes)); // small cap guided by probes
    for (int a = 0; a < top1; ++a){
        for (int b = a + 1; b < top1; ++b){
            uint32_t m = w1[a].mask | w1[b].mask;
            // estimate: product (or min) of the two singles
            size_t score = (size_t)w1[a].score * (size_t)w1[b].score;
            w2.push_back({m, score});
        }
    }
    sort(w2.begin(), w2.end(), [](const Item& A, const Item& B){ return A.score > B.score; });
    for (auto &it : w2){
        if ((int)out.size() >= probes) break;
        out.push_back(it.mask);
    }
    return out;
}


/* =============================== Candidate gathering ============================================ */

static vector<int> gather_candidates(const vector<vector<float>> &pts,
                                     const vector<float> &q,
                                     int M_per_vertex, int probes)
{
    vector<int> cand;
    cand.reserve(M_per_vertex * max(1, probes));
    unordered_set<int> seen;

    const string base = vertex_for_point(q);
    const int k = (int)bit_functions.size();

    // size-guided neighbors (probes = count to visit)
    vector<uint32_t> masks = neighbor_masks_top_by_bucket(base, k, probes);

    for (uint32_t m : masks){
        string vtx = xor_vertex_with_mask(base, m);
        auto it = Hypercube_table.find(vtx);
        if (it == Hypercube_table.end()) continue;

        // select closest M_per_vertex inside this bucket
        const auto& bucket = it->second;
        vector<pair<float,int>> local;
        local.reserve(bucket.size());
        for (int id : bucket){
            if (seen.find(id) != seen.end()) continue;
            float d = euclidean_distance(q, pts[id]);
            local.emplace_back(d, id);
        }
        if (local.empty()) continue;

        if ((int)local.size() > M_per_vertex){
            nth_element(local.begin(), local.begin() + M_per_vertex, local.end(),
                        [](const auto& a, const auto& b){ return a.first < b.first; });
            local.resize(M_per_vertex);
            sort(local.begin(), local.end(),
                 [](const auto& a, const auto& b){ return a.first < b.first; });
        } else {
            sort(local.begin(), local.end(),
                 [](const auto& a, const auto& b){ return a.first < b.first; });
        }

        for (auto &p : local){
            if (seen.insert(p.second).second){
                cand.push_back(p.second);
            }
        }
    }
    return cand;
}


/* ========================================= KNN query ============================================ */
vector<int> cube_query_knn(const vector<vector<float>> &pts,
                           const vector<float> &q,
                           int N, int M, int probes)
{
    vector<int> cand = gather_candidates(pts, q, M, probes);
    if (cand.empty()) return {};

    // max-heap of (dist, id); keep only best N
    priority_queue<pair<float,int>> heap;

    for (int id : cand){
        float d = euclidean_distance(q, pts[id]);
        if ((int)heap.size() < N){
            heap.emplace(d, id);
        } else if (d < heap.top().first){
            heap.pop();
            heap.emplace(d, id);
        }
    }

    // extract in ascending distance
    vector<pair<float,int>> tmp;
    tmp.reserve(heap.size());
    while (!heap.empty()){
        tmp.push_back(heap.top());
        heap.pop();
    }
    sort(tmp.begin(), tmp.end(), [](const auto &a, const auto &b){
        return a.first < b.first;
    });

    vector<int> nn;
    nn.reserve(tmp.size());
    for (auto &p : tmp) nn.push_back(p.second);
    return nn;
}

/* ======================================== Range query =========================================== */
vector<int> cube_range_search(const vector<vector<float>> &pts,
                             const vector<float> &q,
                             float R, int M, int probes)
{
    vector<int> cand = gather_candidates(pts, q, M, probes);
    vector<int> inside;
    inside.reserve(cand.size());
    for (int id : cand){
        if (euclidean_distance(q, pts[id]) <= R){
            inside.push_back(id);
        }
    }
    return inside;
}


