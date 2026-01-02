#include "ivf_pq.h"                       // perilamvanei ton orismo tis klasis IVFPQ
#include "utils_functions/Data_loader.h"      // gia diavasma datasets
#include "vectors.h"                        // Dataset struct
#include "utils_functions/euclid.h"  // euclidean distance
#include "utils_functions/nearest_neighbor.h" // nearest neighbor brute force
#include "kmeans.h"                         // kmeans training
#include "ivf_main.h"                       // klases ivf_main
#include <chrono>                           // gia chronometry
#include <algorithm>                        // std::min, std::sort
#include <iostream>                         // cout, cerr
#include <fstream>                          // ofstream
#include <cmath>                            // sqrt
#include <cstdlib>                          // exit
#include <vector>                           // vector
#include <string>                           // string

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


using clock_type = std::chrono::high_resolution_clock; // tipoi gia chronos
using ms = std::chrono::milliseconds;                  // tipoi gia milliseconds

// ------------------- struct gia ta command line arguments -------------------
struct Args {
    std::string method = "ivfpq";       // default method
    std::string data_path;              // -d <dataset path>
    std::string query_path;             // -q <query path>
    std::string type = "mnist";         // -type mnist|sift
    int N = 5;                          // -N number of nearest neighbors
    int kclusters = 50;                 // -kclusters
    int nprobe = 5;                     // -nprobe
    int M = 16;                          // -M subquantizers
    int nbits = 8;                        // -nbits bits per subvector
    double R = -1.0;                     // -R radius for range search
    unsigned seed = 1;                   // -seed
    bool range = false;                  // -range true|false
    std::string output_path;             // -o <output file>
};

// ------------------- parsing arguments -------------------
static Args parse_args(int argc, char** argv) {
    Args a;                              // dimiourgia struct me default times
    for (int i = 1; i < argc; ++i) {    // loop panw se ola ta args
        std::string f = argv[i];        // pairnoume to trexon arg
        auto need = [&](int more){ if (i + more >= argc) { std::cerr << "Missing value after " << f << "\n"; exit(1); } }; // elegxos an leiptei timi meta to flag

        // if (f == "-method") { need(1); a.method = argv[++i]; }   // an -method diavase thn epomenh timi
        if (f == "-ivfflat") { a.method = "ivfflat"; }
        else if (f == "-ivfpq") {a.method = "ivfpq" ;}
        else if (f == "-d") { need(1); a.data_path = argv[++i]; }  // -d dataset path
        else if (f == "-q") { need(1); a.query_path = argv[++i]; } // -q query path
        else if (f == "-N") { need(1); a.N = std::stoi(argv[++i]); } // -N number of neighbors
        else if (f == "-kclusters") { need(1); a.kclusters = std::stoi(argv[++i]); } // -kclusters
        else if (f == "-nprobe") { need(1); a.nprobe = std::stoi(argv[++i]); } // -nprobe
        else if (f == "-seed") { need(1); a.seed = static_cast<unsigned>(std::stoul(argv[++i])); } // -seed
        else if (f=="-M"){ need(1); a.M = std::stoi(argv[++i]); } // -M number of subquantizers
        else if (f=="-nbits"){ need(1); a.nbits = std::stoi(argv[++i]); } // -nbits bits per subvector
        else { std::cerr << "Unknown flag: " << f << "\n"; exit(1); } // unknown flag
    }

    if (a.method != "ivfflat" && a.method != "ivfpq") { // elegxos method
        std::cerr<<"Error: only 'ivfflat' and 'ivfpq' supported currently\n"; exit(1);
    }
    if (a.data_path.empty() || a.query_path.empty()) { // elegxos paths
        std::cerr << "Usage: ... \n"; exit(1);
    }
    return a; // epistrefei struct
}

// ------------------- main function -------------------
int main(int argc, char** argv) {
    Args args = parse_args(argc, argv);   // diabazei args

    Dataset base, queries;

    try {
        std::cout << "Loading dataset: " << args.data_path << std::endl;
        load_fvecs_into_dataset(args.data_path, base);

        std::cout << "Loading queries: " << args.query_path << std::endl;
        load_fvecs_into_dataset(args.query_path, queries);
    }
    catch (const std::exception& e) {
        std::cerr << "Error while loading fvecs: " << e.what() << "\n";
        return 1;
    }

    std::cout << "Loaded " << base.n << " vectors of dimension " << base.d << "\n";
    std::cout << "Loaded " << queries.n << " query vectors of dimension " << queries.d << "\n";


    int maxQ = std::min((size_t)100, queries.n); // periorismos 100 queries
    
    // ================= METRICS =================
    double sum_tApprox_ms = 0.0;
    int qcount = 0;
    // ===========================================

    // ------------------- ivfpq training -------------------
    std::cout << "Training IVFPQ with C=" << args.kclusters
              << ", M=" << args.M << ", nbits=" << args.nbits
              << ", seed=" << args.seed << " ..." << std::endl; // print training info

    IVFPQ pqindex; 
    pqindex.train(base, static_cast<size_t>(args.kclusters),
                      static_cast<size_t>(args.M),
                      static_cast<size_t>(args.nbits), args.seed); // train index
    pqindex.index_dataset(base); // index dataset

    std::cout << "IVFPQ index built. Number of lists (clusters): " << args.kclusters << std::endl; // info

    // ------------------- loop queries -------------------
    for (int qi = 0; qi < maxQ; ++qi) {
        const float* qptr = queries.row(qi); // pointer sto query dianisma

        // approximate search
        auto t0 = clock_type::now(); // start chrono
        std::vector<std::pair<uint32_t,float>> approx = pqindex.search(qptr, base, static_cast<size_t>(args.N),
                                                                       static_cast<size_t>(args.nprobe), args.R); // search
        auto t1 = clock_type::now(); // stop chrono
        double t_query_ms =
            std::chrono::duration_cast<ms>(t1 - t0).count();

        sum_tApprox_ms += t_query_ms;
        qcount++;

        std::vector<uint32_t> approx_ids; approx_ids.reserve(approx.size()); // ids
        std::vector<float> approx_dists;  approx_dists.reserve(approx.size()); // distances
        for (auto &p : approx) { // convert results
            approx_ids.push_back(p.first); // push id
            approx_dists.push_back(std::sqrt(p.second)); // push distance sqrt
        }

        // ------------------- per query output -------------------
        cout << "Query: " << (qi) << "\n"; // print query number
        int outN = std::min((int)approx_ids.size(), args.N); // output N
        for (int i = 0; i < outN; ++i) {
            float dA = approx_dists[i]; // approximate
            cout << "Nearest neighbor-" << (i+1) << ": " << approx_ids[i] << "\n"; // print id
            cout << "distanceApproximate: " << dA << "\n"; // print approximate dist
        }

        cout << "\n";
    }

    // ================= FINAL METRICS =================
    double tApprox = (sum_tApprox_ms / qcount) / 1000.0;   // seconds
    double QPS = qcount / (sum_tApprox_ms / 1000.0);

    std::cout << "tApprox: " << tApprox << " s\n";
    std::cout << "QPS: " << QPS << "\n";

    return 0; // telos main
}
