#include "nearest_neighbor.h"   // to header me tin dilwsi tis nearest_neighbor
#include "euclid.h"             // perilambanei tin sinartisi eucliddistance pou ypologizei L2 (squared)
#include <algorithm>            // gia nth_element kai sort
#include <cmath>                // gia sqrt kai alles math synartiseis
#include <utility>              // gia std::pair (prepei kai se kapoies staseis)

void nearest_neighbor(const Dataset &ds,             // ds: to dataset me ta dianusmata
                      const float* q,                // q: pointer sto query vector
                      int N,                         // N: posoi geitones theloume
                      std::vector<uint32_t> &out_ids,// out_ids: exodos me ta ids twn geitonwn
                      std::vector<float> &out_dists) // out_dists: exodos me tis apostaseis (L2)
{
    std::vector<std::pair<float,uint32_t>> all;     // "all": pair(distsq, id) gia ola ta dianusmata
    all.reserve(ds.n);                              // krataei capacity gia olo to dataset gia apodosh

    for (size_t i = 0; i < ds.n; ++i) {              // gia kathe dianisma sto dataset
        float dsq = eucliddistance(q, ds.row(i), ds.d); // ypologismos squared euclidean dist (sum of squares)
        all.emplace_back(dsq, static_cast<uint32_t>(i)); // prosthiki (distSquared, id) ston pinaka all
    }

    if ((int)all.size() > N) {                       // an exoume perissotera apo N apotelesmata
        std::nth_element(all.begin(), all.begin() + N, all.end(),
                         [](const auto &a, const auto &b){ return a.first < b.first; }); 
        // nth_element: top-N partition — ta prwta N stoixeia einai ta N mikrotera (alla oxi taksinomhmena)
        all.resize(N);                              // kratame mono ta prwta N stoixeia
    }

    std::sort(all.begin(), all.end(),
              [](const auto &a, const auto &b){ return a.first < b.first; });
    // teleia taksinomhsh (apotelesmata apo mikrotero se megalytero) me vasi to squared distance

    out_ids.clear(); out_dists.clear();              // katharizoume ta outputs prin gemisoume
    out_ids.reserve(all.size());                     // kratame capacity gia apodosh
    out_dists.reserve(all.size());

    for (auto &p : all) {                            // gia kathe (distSquared, id) sto all
        out_ids.push_back(p.second);                 // prosthetoume to id sti lista out_ids
        out_dists.push_back(std::sqrt(p.first));     // prosthetoume tin pragmatiki apostasi (sqrt tou squared)
    }

}
