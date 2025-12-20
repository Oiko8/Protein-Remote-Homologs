#include "Euclidean_Hashing.h"
#include "utils_functions/euclid.h"


/*=================================== Global definitions =========================================*/
vector<AmplifiedHash> amplified_functions;
vector<Hash> hash_functions;
vector<Table> tables;
vector<vector<unsigned int>> point_ids;
// vector<vector<int>> point_bucket_ids;
random_generator gen(42);


/* =============================================================================================== */
/* ========================================= NN Search =========================================== */
/* =============================================================================================== */
vector<int> query_knn(const vector<vector<float>> &pts, vector<float> &q, int k){
    int L = static_cast<int>(tables.size());
    if (k <= 0 || L == 0) return {};
    
    vector<pair<float,int>> all_candidates;
    all_candidates.reserve(L * k);
    unordered_set<int> seen; // to avoid duplicates
    
    for (int i = 0; i < L; ++i) {
        unsigned int bucket_of_query = amplified_functions[i].get_amplified_id(q);
        // unsigned int query_id = amplified_functions[i].get_point_id(q);
    
        auto it = tables[i].find(bucket_of_query);
        if (it == tables[i].end()) continue;
    
        // keep the k closest in THIS table
        priority_queue<pair<float,int>> local; // (dist, id)
        for (int id : it->second) {
            // skip duplicates (already seen from other tables)
            if (seen.count(id)) continue;

            // checking the ID(p)=(r1*h1(p) + r2*h2(p) + ... ) mod M instead of bucket = ((r1*h1(p) + r2*h2(p) + ... ) mod M) mod tablesize
            // if (point_ids[i][id] != query_id) continue; 

            float d = euclidean_distance(q, pts[id]);
            if (d == 0.0f) continue;
            if ((int)local.size() < k) {
                local.emplace(d, id);
            } else if (d < local.top().first) {
                local.pop();
                local.emplace(d, id);
            }
        }
    
        // add this table’s k best (marking them as seen)
        while (!local.empty()) {
            auto p = local.top();
            local.pop();
            if (!seen.count(p.second)) {
                all_candidates.push_back(p);
                seen.insert(p.second);
            }
        }
    }
    
    if (all_candidates.empty()) return {};
    
    // now select global k closest
    priority_queue<pair<float,int>> global;
    for (const auto &p : all_candidates) {
        if ((int)global.size() < k) {
            global.push(p);
        } else if (p.first < global.top().first) {
            global.pop();
            global.push(p);
        }
    }
    
    // sort and return ids
    vector<pair<float,int>> best;
    while (!global.empty()) { best.push_back(global.top()); global.pop(); }
    sort(best.begin(), best.end(), [](auto &a, auto &b){ return a.first < b.first; });
    
    vector<int> nn_idx;
    nn_idx.reserve(best.size());
    for (auto &p : best) nn_idx.push_back(p.second);
    
    return nn_idx;
}

// vector<int> query_knn(const vector<vector<float>> &pts, vector<float> &q, int k){}
/* =============================================================================================== */
/* ========================================= Range Search ======================================== */
/* =============================================================================================== */

vector<int> range_search(const vector<vector<float>> &pts, vector<float> &q, float R){
    // int dim = static_cast<int>(q.size());
    int L = static_cast<int>(tables.size());

    vector<int> pts_idx_in_range;
    
    float dist;

    for (int i = 0 ; i < L ; i++) {
        int bucket_of_query = amplified_functions[i].get_amplified_id(q);
        for (int id : tables[i][bucket_of_query]) {
            dist = euclidean_distance(q, pts[id]);
            if (dist < R){
                pts_idx_in_range.push_back(id);
            }
        }        
    }

    sort(pts_idx_in_range.begin(), pts_idx_in_range.end());               
    pts_idx_in_range.erase(unique(pts_idx_in_range.begin(), pts_idx_in_range.end()), pts_idx_in_range.end());
    return pts_idx_in_range;

}


/* =============================================================================================== */
/* ===================================== Creating the Tables ===================================== */
/* =============================================================================================== */
void build_hash_tables(vector<vector<float>> &pts, int L, int khash, float w){
    int tableSize = pts.size()/8;   // no of buckets in each table
    int dim = pts[0].size();

    tables.clear();
    tables.resize(L);

    amplified_functions.clear();
    amplified_functions.resize(L);

    point_ids.clear();
    point_ids.resize(L);
    for (int l = 0; l < L; ++l) point_ids[l].resize(pts.size());

    for (int i = 0 ; i < L ; i ++) {
        amplified_functions[i] = AmplifiedHash(khash, w, tableSize, dim);
    }
    
    for (int l = 0; l < L ; l++) {
        for (int i = 0; i < (int)pts.size(); ++i) {
            // creating the tables using the g(p)
            tables[l][amplified_functions[l].get_amplified_id(pts[i])].push_back(i);
            // keeping the ID(p)
            // checking the ID(p)=(r1*h1(p) + r2*h2(p) + ... ) mod M instead of g(p) = ((r1*h1(p) + r2*h2(p) + ... ) mod M) mod tablesize
            // point_ids[l][i] = amplified_functions[l].get_point_id(pts[i]);

        }
    } 
}



/* =============================================================================================== */
/* ================================== Hash Class Implementation ================================== */
/* =============================================================================================== */

Hash::Hash(float w, int dim) {
    w_ = w;
    dim_ = dim;
    t_ = generate_t(w_);
    v_ = vec_d();
} 
    
int Hash::get_hash_id(vector<float>& p) const {
    int h = static_cast<int>(floor((dot(v_, p) + t_) / w_));
    return h;
}

/* ======================= Helper functions ============================ */
/* ========= Creating a 2-vector with normal disrtibution ============== */

vector<float> Hash::vec_d(){
    vector<float> v(dim_);
    for (int i=0 ; i < dim_ ; i++){
        v[i] = normal_distribution_generator();
    }
    return v;
}

/* ========= return a point with normal distribution =================== */
float Hash::normal_distribution_generator(){
    normal_distribution<float> dist(0.0, 1.0);
    return dist(gen);
}

/* ========== generate a small disturbance t =========================== */
float Hash::generate_t(float w) {
    // keep your original definition (Uniform[0, w])
    uniform_real_distribution<float> dist(0.0, w);
    return dist(gen);
}

float Hash::dot(const vector<float>& v, const vector<float>& p){
    float dot_product = 0.0;
    int size = v.size();
    for (int i=0 ; i < size ; i++){
        dot_product += v[i]*p[i];
    } 
    return dot_product;
}

  
/* =============================================================================================== */
/* ============================ Amplified Hash Class Implementation ============================== */
/* =============================================================================================== */

AmplifiedHash::AmplifiedHash(int k, float w, int tableSize, int dim) {
    tableS_ = tableSize;
    k_ = k;
    h_.reserve(k_);
    r_.reserve(k_);
    dim_ = dim;

    for (int i = 0; i < k_; i++) {
        // create new hash function
        Hash new_h(w, dim_);
        h_.push_back(new_h);
        // generate new r for the hash function
        r_.push_back(generate_r(M_));   // r_i ~ [0, M-1]
    }
}

int AmplifiedHash::getTableSize() const { return tableS_; }


unsigned int AmplifiedHash::get_point_id(vector<float> &p) const {
    long long acc = 0;                           
    for (int i = 0; i < k_; ++i) {
        int hi = h_[i].get_hash_id(p);          // may be negative
        // cout << hi % M_ << endl;
        // modulo between a and b to support negative numbers: (a % b + b) %b
        long long htemp = ((long long)hi % (long long)M_ + (long long)M_) % (long long)M_;
        // cout << htemp << endl;
        unsigned int hnorm = (unsigned int)htemp; 
        // cout << hnorm << endl;
        // if (hnorm < 0) hnorm += M_;             // now hnorm in [0, M-1]
        long long curr = static_cast<long long>(r_[i]) * static_cast<long long>(hnorm);

        acc += (curr % M_);

    }
    unsigned int id = static_cast<unsigned int>(static_cast<long long>(acc % M_));
    return id;
}

unsigned int AmplifiedHash::get_amplified_id(vector<float> &p) const{
    unsigned int id_ = get_point_id(p);

    return id_% tableS_;
}

/* =============== generate r~[0, M-1) to multiply each hash result =========================== */
int AmplifiedHash::generate_r(int M) {
    // integer coefficients r_i ∈ {0, …, M-1}
    uniform_int_distribution<int> dist(0, M - 1);
    return dist(gen);
}



/* =============================================================================================== */
/* ====================================== Helper Functions ======================================= */
/* =============================================================================================== */





