#include "ivf_flat.h"  // Header me orismo tis klasis IVFFlat
#include "kmeans.h"    // Gia xrisi tou KMeans gia clustering
#include "utils_functions/euclid.h"  // Gia ypologismo apostasewn me Euclidean distance
#include <iostream>    // Gia debug & ektypwseis
#include <queue>       // Gia xrisi priority queue
#include <functional>  // Gia lambdas kai synartiseis sygkrisis
#include <algorithm>   // Gia sort kai nth_element
#include <cassert>     // Gia elegxous me assert

// Ekpaidevei ton IVF Flat index kai to kanei indexing
void IVFFlat::train_and_index(const Dataset &ds, size_t kclusters_, unsigned seed){
    kclusters = kclusters_;  // Orizei ton arithmo twn clusters
    d = ds.d;  // Apothikeuei tis diastaseis twn vectors
    centroids = KMeans::train(ds, kclusters, seed);  // Kalei ton KMeans gia na parei ta kentra
    assert(centroids.size() == kclusters*d);  // Elegxos oti exoun swsto megethos
    lists.assign(kclusters, {});  // Ftiaxnei k adeies listes gia ta clusters

    std::vector<uint32_t> assign(ds.n);  // Pinakas gia na kratisei se poio cluster anoikei kathe deigma
    KMeans::assign_all(ds, centroids, assign);  // Antistoixizei kathe deigma sto kontinotero kentro

    for(size_t i=0;i<ds.n;i++){  // Gia kathe deigma
        if(assign[i] >= kclusters){ // Elegxos oti to cluster index einai egkyro
            std::cerr << "Error: assignment out of bounds! i=" << i << " assign[i]=" << assign[i] << " kclusters=" << kclusters << "\n";
        }
        // assert(assign[i] < kclusters); // Elegxos me assert
        lists[ assign[i] ].push_back((uint32_t)i); // Prosthetei to id tou deigmatos stin lista tou cluster
    }
}

// Epistrofi top-N pairs (id, dist)
std::vector<std::pair<uint32_t, float>> IVFFlat::search(const float* q, const Dataset &ds, size_t N, size_t nprobe, float R) const {
    // Evresi nprobe nearest centroids (linear scan)
    struct C { size_t idx; float dist; }; // Mikri domi gia apothikeusi (centroid index, apostasi)
    std::vector<C> cent_d(kclusters); // Pinakas gia apostaseis apo ola ta centroids
    for(size_t c=0;c<kclusters;c++){ // Ypologismos apostasis apo kathe kentro
        cent_d[c] = {c, eucliddistance(q, &centroids[c*d], d)};
    }
    // Krataei ta nprobe pio kontina centroids
    std::nth_element(cent_d.begin(), cent_d.begin() + std::min(nprobe, cent_d.size()), cent_d.end(),
                     [](const C&a,const C&b){return a.dist<b.dist;});
    std::vector<size_t> probe_idxs; // Ta indexes twn centroids pou tha elegxthoun
    for(size_t i=0;i<std::min(nprobe,cent_d.size());i++) probe_idxs.push_back(cent_d[i].idx);

    // max-heap for top-N (dist, id)
    using PI = std::pair<float, uint32_t>; // Zevgari apostasis-id
    std::priority_queue<PI> heap; // Max heap gia ta kalytera apotelesmata

    for(size_t p : probe_idxs){ // Gia kathe epilegmeno cluster
        assert(p < lists.size()); // Elegxos oti einai egkyro
        for(uint32_t id : lists[p]){ // Gia kathe deigma sto cluster
            if(id >= ds.n){ std::cerr << "Error: id out of bounds! id=" << id << " ds.n=" << ds.n << "\n"; }
            assert(id < ds.n);
            float dist = eucliddistance(q, ds.row(id), d); // Ypologizei tin apostasi apo to query
            if(R>0 && dist > R*R) continue; // An yparxei aktina R, agnoei osa einai ektos
            if(heap.size() < N) heap.emplace(dist, id); // Prosthetei an den exei gemisei
            else if(dist < heap.top().first){ heap.pop(); heap.emplace(dist, id); } // An vrethei kalytero ginetai antikatastasi
        }
    }
    std::vector<std::pair<uint32_t,float>> result; // Telikos pinakas apotelesmatwn
    while(!heap.empty()){ // Afairoume ola ta stoixeia apo to heap (apo mikroteri se megaliteri apostasi)
        result.emplace_back(heap.top().second, heap.top().first);
        heap.pop();
    }
    std::reverse(result.begin(), result.end()); // Antistrefei gia na exoun afksousa seira apostasewn
    return result; // Epistrefei ta top-N apotelesmata
}
