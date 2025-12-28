#include "ivf_flat.h"                               // perilamvanei ton orismo tis klasis IVFFlat
#include "utils_functions/Data_loader.h"               // leitourgies gia read/write dataset
#include "vectors.h"                                 // orismos tou Dataset struct
#include "utils_functions/euclid.h"           // eucliddistance function
#include "utils_functions/nearest_neighbor.h" // nearest_neighbor function prototype
#include "kmeans.h"                                  // kmeans gia training clusters
#include "ivf_main.h"                                // exoteriko header gia main ivf
#include <chrono>                                    // chrono gia chronometrisi
#include <algorithm>                                 // std::sort, std::nth_element
#include <iostream>                                  // cout, cerr
#include <fstream>                                   // ofstream gia arxeio output
#include <cmath>                                     // sqrt kai allo
#include <cstdlib>                                   // exit, atoi
#include <vector>                                    // vector
#include <string>                                    // string


// helper for reading protein vectors
static void load_fvecs_into_dataset(const std::string& path, Dataset& out) {
    int d = 0;
    std::vector<std::vector<float>> vecs = load_fvecs(path, d);

    if (vecs.empty()) {
        out.reserve(0, 0);
        return;
    }

    out.reserve(vecs.size(), (size_t)d);

    for (size_t i = 0; i < vecs.size(); ++i) {
        if ((int)vecs[i].size() != d) {
            throw std::runtime_error("Inconsistent vector dimension in " + path);
        }
        std::copy(vecs[i].begin(), vecs[i].end(), out.row_mut(i));
    }
}


using clock_type = std::chrono::high_resolution_clock; // tipoi gia chronos me high resolution
using ms = std::chrono::milliseconds;                  // milliseconds
using ns = std::chrono::nanoseconds;

struct Args { 
    std::string method = "ivfflat"; // default method
    std::string data_path;          // -d <path> dataset
    std::string query_path;         // -q <path> queries
    std::string type = "mnist";     // -type mnist|sift
    int N = 5;                      // -N number of neighbors
    int kclusters = 50;             // -kclusters
    int nprobe = 5;                 // -nprobe poses listes tha elegxoume
    double R = -1.0;                // -R radius (optional)
    unsigned seed = 1;              // -seed gia random generators
    bool range = false;             // -range true|false
    std::string output_path;        // -o <output file>
};

static Args parse_args(int argc, char** argv) {
    Args a; // dimiourgia struct me default values
    for (int i = 1; i < argc; ++i) { // loop panw se ola ta args
        std::string f = argv[i];      // pairnoume to trexon arg
        auto need = [&](int more){ if (i + more >= argc) { std::cerr << "Missing value after " << f << "\n"; exit(1); } }; // elegxos oti yparxei timi meta to flag

        // if (f == "-method") { need(1); a.method = argv[++i]; }       // diavase -method
        if (f == "-ivfflat") { a.method = "ivfflat"; }
        else if (f == "-ivfpq") {a.method = "ivfpq" ;}
        else if (f == "-d") { need(1); a.data_path = argv[++i]; }    // diavase -d dataset
        else if (f == "-q") { need(1); a.query_path = argv[++i]; }   // diavase -q queries
        else if (f == "-N") { need(1); a.N = std::stoi(argv[++i]); } // diavase -N number of neighbors
        else if (f == "-kclusters") { need(1); a.kclusters = std::stoi(argv[++i]); } // -kclusters
        else if (f == "-nprobe") { need(1); a.nprobe = std::stoi(argv[++i]); } // -nprobe
        else if (f == "-seed") { need(1); a.seed = static_cast<unsigned>(std::stoul(argv[++i])); } // -seed
        else { std::cerr << "Unknown flag: " << f << "\n"; exit(1); } // unknown flag
    }
    if (a.method != "ivfflat" && a.method != "ivfpq") { // elegxos method
        std::cerr<<"Error: only 'ivfflat' and 'ivfpq' supported currently\n";
        exit(1);
    }
    if (a.data_path.empty() || a.query_path.empty()) { // elegxos paths
        std::cerr << "Usage: ... \n"; exit(1);
    }
    return a; // epistrefei struct me args
}

int main(int argc, char** argv) {
    Args args = parse_args(argc, argv);   // diabazei arguments


    Dataset base, queries;

    try {
        cout << "Loading dataset: " << args.data_path << std::endl;
        load_fvecs_into_dataset(args.data_path, base);

        cout << "Loading queries: " << args.query_path << std::endl;
        load_fvecs_into_dataset(args.query_path, queries);
    }
    catch (const std::exception& e) {
        std::cerr << "Error while loading fvecs: " << e.what() << "\n";
        return 1;
    }

    cout << "Loaded " << base.n << " vectors of dimension " << base.d << "\n";
    cout << "Loaded " << queries.n << " query vectors of dimension " << queries.d << "\n";


    int maxQ = std::min((size_t)100, queries.n);       // max 100 queries
    double sum_AF = 0.0; int hits_at_N = 0;           // metrics
    double sum_tApprox_ms = 0.0, sum_tTrue_ms = 0.0; int qcount = 0; // timing
    if (args.method == "ivfflat") {                                 // an to method einai ivfflat
    cout << "Training IVFFlat with C=" << args.kclusters << "  seed=" << args.seed << " ..." << std::endl; // emfanisi info

    IVFFlat index;                                               // dhmiourgia antikeimenou index IVFFlat
    index.train_and_index(base, static_cast<size_t>(args.kclusters), args.seed); // ekpaideush + index

    cout << "Index built. Number of lists (clusters): " << args.kclusters << std::endl; // emfanisi clusters

    for (int qi = 0; qi < maxQ; ++qi) {                        // loop panw se queries
        const float* qptr = queries.row(qi);                   // pointer sto query dianisma

        auto t0 = clock_type::now();                           // arxikos xronos
        std::vector<std::pair<uint32_t,float>> approx = index.search(qptr, base, static_cast<size_t>(args.N),
                                                                     static_cast<size_t>(args.nprobe), args.R); // approximate search
        auto t1 = clock_type::now();                           // telikos xronos
        double approx_ms = std::chrono::duration_cast<ns>(t1 - t0).count(); // xronos se ms

        std::vector<uint32_t> approx_ids; approx_ids.reserve(approx.size()); // ids twn approximate apotelesmatwn
        std::vector<float> approx_dists; approx_dists.reserve(approx.size()); // apostaseis twn approximate apotelesmatwn
        for (auto &p : approx) {                               // loop panw sta results
            approx_ids.push_back(p.first);                     // prosthese id
            approx_dists.push_back(std::sqrt(p.second));      // prosthese apostasi (sqrt apo squared)
        }

        t0 = clock_type::now();                                // arxikos xronos true search
        std::vector<uint32_t> true_ids; std::vector<float> true_dists; // containers gia true nearest
        nearest_neighbor(base, qptr, args.N, true_ids, true_dists); // brute-force true top-N
        t1 = clock_type::now();                                // telikos xronos
        double true_ms = std::chrono::duration_cast<ns>(t1 - t0).count(); // xronos se ms

        if (!approx_dists.empty() && !true_dists.empty()) {    // elegxos na min einai empty
            float min_approx = *std::min_element(approx_dists.begin(), approx_dists.end()); // elaxisti aprox apostasi
            float true_min = true_dists[0];                   // mikroteri true apostasi
            sum_AF += (true_min > 0.0f) ? (min_approx / true_min) : 1.0; // upologismos AF
        }
        if (!true_ids.empty()) {                               // elegxos an true_ids den einai empty
            uint32_t t0id = true_ids[0];                       // prwto true id
            for (auto id : approx_ids) {                       // loop panw sta approx ids
                if (id == t0id) { hits_at_N++; break; }       // an breTthei prwto true id, increment hits
            }
        }

        sum_tApprox_ms += approx_ms;                           // akolouthisi xronou approximate
        sum_tTrue_ms += true_ms;                               // akolouthisi xronou true
        qcount++;                                              // metritis queries

        cout << "Query: " << (qi) << "\n";              // emfanisi query number
        int outN = std::min((int)approx_ids.size(), args.N);  // emfanizomena apotelesmata
        for (int i = 0; i < outN; ++i) {                      
            float dA = approx_dists[i];                       
            float dT = (i < (int)true_dists.size()) ? true_dists[i] : (true_dists.empty() ? 0.0f : true_dists.back()); // distance true
            cout << "Nearest neighbor-" << (i+1) << ": " << approx_ids[i] << "\n"; // emfanisi id
            cout << "distanceApproximate: " << dA << "\n";   // emfanisi aprox distance
            cout << "distanceTrue: " << dT << "\n";          // emfanisi true distance
        }

        if (args.range && args.R > 0.0) {                       // an exoume range search
            std::vector<int> in_range;                          // container gia R-near ids
            auto allcand = index.search(qptr, base, base.n, static_cast<size_t>(args.nprobe), args.R); // ola ta candidates
            for (auto &p : allcand) {                           
                float d = std::sqrt(p.second);                  // apostasi
                if (d <= static_cast<float>(args.R)) in_range.push_back(static_cast<int>(p.first)); // prosthese an mesa sto R
            }
            std::sort(in_range.begin(), in_range.end());        // sort ids
            in_range.erase(std::unique(in_range.begin(), in_range.end()), in_range.end()); // remove duplicates
            cout << "\nR-near neighbors:\n";                 // emfanisi header
            for (int id : in_range) cout << id << "\n";      // emfanisi ids
        }

        cout << "\n";                                         // empty line
    }
}

double avgAF = (qcount > 0) ? (sum_AF / qcount) : 0.0;       // average AF
double recall = (qcount > 0) ? ((double)hits_at_N / qcount) : 0.0; // recall@N
double qps = (sum_tApprox_ms > 0.0 && qcount > 0) ? (qcount / (sum_tApprox_ms / 1000000.0)) : 0.0; // queries/sec
double avgApproxMs = (qcount > 0) ? (sum_tApprox_ms / qcount) : 0.0; // avg approximate ms
double avgTrueMs = (qcount > 0) ? (sum_tTrue_ms / qcount) : 0.0;     // avg true ms

cout << "Average AF: " << avgAF << "\n";                   // emfanisi AF
cout << "Recall@N: "   << recall << "\n";                 // emfanisi recall
cout << "QPS: " << qps << "\n";                            // emfanisi QPS
cout << "tApproximateAverage: " << avgApproxMs << " ms\n"; // emfanisi tApprox
cout << "tTrueAverage: "        << avgTrueMs << " ms\n";   // emfanisi tTrue

return 0;                                                     // telos main function
}