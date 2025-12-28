#include "ivf_pq.h"
#include "kmeans.h"
#include "utils_functions/euclid.h"
#include <algorithm>
#include <queue>
#include <cstring>
#include <iostream>

// helper: brei to kontinotero centroid apo flat centroids
static size_t argmin_centroid_flat(const float* v, const std::vector<float> &centroids, size_t k, size_t d){
    size_t best = 0;
    float bestd = eucliddistance(v, &centroids[0], d);
    for(size_t i=1;i<k;i++){
        float dd = eucliddistance(v, &centroids[i*d], d);
        if(dd < bestd){ bestd = dd; best = i; }
    }
    return best;
}

// train: ekpaideush IVf centroids + PQ codebooks gia kathe subspace
void IVFPQ::train(const Dataset &ds, size_t kclusters_, size_t M_, size_t nbits_, unsigned seed){
    // set params
    kclusters = kclusters_;
    d = ds.d;
    M = M_;
    nbits = nbits_;
    K = 1u << nbits;                           // clusters ana subvector
    subd = (d + M - 1) / M;                    // ceil(d / M) -> padding gia teleutaio subvector

    // 1) train ivf centroids me kmeans pano sto oloklhrw dataset
    ivf_centroids = KMeans::train(ds, kclusters, seed);

    // 2) train pq codebooks: gia ka8e subspace ftiaxnoume sub-dataset kai trexoume kmeans(K)
    pq_codebooks.assign(M * K * subd, 0.0f);   // reserve xrisi

    Dataset subds;                              // sub-dataset gia ka8e m
    subds.n = ds.n;
    subds.d = subd;
    subds.data.resize(subds.n * subds.d);

    for(size_t m=0;m<M;m++){
        // gemizoume subds me τα υπο-διανύσματα (padding με 0 αν χρειάζεται)
        for(size_t i=0;i<ds.n;i++){
            const float* src = ds.row(i) + m*subd; // pointer sto subvector
            float* dst = &subds.data[i*subd];
            for(size_t j=0;j<subd;j++){
                size_t idx = m*subd + j;
                if (idx < d) dst[j] = src[j];      // αν εντος original διαστασης, αντιγραφουμε
                else dst[j] = 0.0f;                // αλλιως padding 0
            }
        }
        // train kmeans sto subds gia K clusters (seed διαφορο για καθε m)
        std::vector<float> codebook = KMeans::train(subds, K, seed + (unsigned)m, 50, 1e-3f);

        // αντιγραφη codebook sto pq_codebooks flat buffer
        for(size_t k=0;k<K;k++){
            for(size_t j=0;j<subd;j++){
                pq_codebooks[(m * K + k) * subd + j] = codebook[k*subd + j];
            }
        }
    }

    // init inverted lists kenes
    lists.assign(kclusters, {});
}

// index_dataset: gia ka8e vector vriskoume to ivf centroid kai ton kvkodikopoihoume me PQ
void IVFPQ::index_dataset(const Dataset &ds){
    lists.assign(kclusters, {});                // clear lists
    for(size_t i=0;i<ds.n;i++){
        const float* v = ds.row(i);
        // brei ivf centroid (argmin)
        size_t c = argmin_centroid_flat(v, ivf_centroids, kclusters, d);

        // ypologise PQ code (ena byte ana subspace)
        PQCode code;
        code.codes.resize(M);
        for(size_t m=0;m<M;m++){
            const float* subptr = v + m*subd;    // pointer sto subvector (padding not real but codebook has padded values)
            // find nearest codebook centroid gia to subspace m
            size_t bestk = 0;
            float bestd = eucliddistance(subptr, &pq_codebooks[(m*K + 0)*subd], subd);
            for(size_t kk=1; kk<K; kk++){
                float dd = eucliddistance(subptr, &pq_codebooks[(m*K + kk)*subd], subd);
                if(dd < bestd){ bestd = dd; bestk = kk; }
            }
            code.codes[m] = static_cast<uint8_t>(bestk);
        }
        // prosthetoume to (id, code) stin lista tou cluster c
        lists[c].push_back({(uint32_t)i, std::move(code)});
    }
}

// search: ypologizoume LT tables gia to q kai scanaroume ta probe lists
std::vector<std::pair<uint32_t, float>> IVFPQ::search(const float* q, const Dataset &/*ds*/, size_t N, size_t nprobe, float R) const {
    // 1) epikekrimenoi kontinoteri centroids (nprobe)
    struct C { size_t idx; float dist; };
    std::vector<C> cent_d(kclusters);
    for(size_t c=0;c<kclusters;c++){
        cent_d[c] = {c, eucliddistance(q, &ivf_centroids[c*d], d)};
    }
    size_t probe = std::min(nprobe, kclusters);
    std::nth_element(cent_d.begin(), cent_d.begin() + probe, cent_d.end(), [](const C&a,const C&b){return a.dist<b.dist;});
    std::vector<size_t> probe_idxs;
    for(size_t i=0;i<probe;i++) probe_idxs.push_back(cent_d[i].idx);

    // 2) precompute lookup table LT[m][k] = squared-dist(tou qsub, codebook_m[k])
    std::vector<std::vector<float>> LT(M, std::vector<float>(K));
    for(size_t m=0;m<M;m++){
        const float* qsub = q + m*subd;             // pointer sto subvector tou query
        for(size_t k=0;k<K;k++){
            const float* cb = &pq_codebooks[(m*K + k) * subd];
            LT[m][k] = eucliddistance(qsub, cb, subd); // squared dist
        }
    }

    // 3) scan probe lists kai upologismos approx dist me summation tou LT[m][code_m]
    using PI = std::pair<float, uint32_t>; // (distSq, id)
    std::priority_queue<PI> heap;

    for(size_t pidx : probe_idxs){
        for(const auto &entry : lists[pidx]){
            uint32_t id = entry.first;
            const PQCode &code = entry.second;
            float dist_approx = 0.0f;
            for(size_t m=0;m<M;m++){
                uint8_t k = code.codes[m];
                dist_approx += LT[m][k];             // sum LT entries ana subvector
            }
            if(R>0 && dist_approx > (float)(R*R)) continue; // filter me R an oristei
            if(heap.size() < N) heap.emplace(dist_approx, id);
            else if(dist_approx < heap.top().first){
                heap.pop(); heap.emplace(dist_approx, id);
            }
        }
    }

    // 4) exagogi apotelesmatwn
    std::vector<std::pair<uint32_t, float>> result;
    while(!heap.empty()){
        result.emplace_back(heap.top().second, heap.top().first);
        heap.pop();
    }
    std::reverse(result.begin(), result.end()); // small->big -> reverse gia small prwta
    return result;
}
