// Usage : 
// MNIST : ./main -d ../MNIST_data/input.dat -q ../MNIST_data/query.dat -kproj 10 -w 10.0 -M 20 -probes 8 -N 4 -R 5.0 -type mnist -range false
#include "Hypercube.h"
#include "utils_functions/Data_loader.h"
#include "utils_functions/euclid.h"
#include <fstream>
#include <chrono>

using clock_type = std::chrono::steady_clock;
using ms = std::chrono::duration<float, std::milli>;

struct Args {
    string data_path;         // -d
    string query_path;        // -q (optional)
    string type = "mnist";    // -type mnist|sift
    string algorithm;         // which algorithm we use
    string output_file = "output.txt";  // -o
    int k = 1;                // -N
    int kproj = 14;           // -kproj
    int M = 5;                // -M (per-vertex or total depending on config)
    int probes = 4;           // -probes (number of vertices to visit)
    int queries_num = 0;      // subset of queries you choose to run 
    float w = 4.0;            // -w
    bool norm = true;
};

static bool str2bool(const string& s){
    return (s=="1"||s=="true"||s=="True"||s=="TRUE");
}

static Args parse_args(int argc, char** argv){
    Args a;
    for (int i=1; i<argc; ++i){
        string flag = argv[i];
        auto need = [&](int more){ if(i+more>=argc) { cerr<<"Missing value after "<<flag<<"\n"; exit(1);} };
        if (flag=="-d") { need(1); a.data_path=argv[++i]; }
        else if (flag=="-q") { need(1); a.query_path=argv[++i]; }
        else if (flag=="-type"){ need(1); a.type=argv[++i]; }
        else if (flag=="-N") { need(1); a.k=stoi(argv[++i]); }
        else if (flag=="-kproj") { need(1); a.kproj=stoi(argv[++i]); }
        else if (flag=="-M") { need(1); a.M=stoi(argv[++i]); }
        else if (flag=="-probes") { need(1); a.probes=stoi(argv[++i]); }
        else if (flag=="-w") { need(1); a.w=stod(argv[++i]); }
        else if (flag=="-subset"){ a.queries_num = stoi(argv[++i]); }
        else { cerr<<"Unknown flag: "<<flag<<"\n"; exit(1);}
    }
    if (a.data_path.empty()){
        cerr<<"Usage: "<<argv[0]<<" -d <input.dat> [-q <query.dat>] [-type mnist|sift] "
              " -kproj <kproj> -w <w> -M <M> -probes <probes> -N <N> -R <R> -range <true|false> -hypercube\n";
        exit(1);
    }
    
    return a;
}

static void search_in_dataset(Args args , string type){

    vector<vector<float>> pts;
    vector<vector<float>> queries;

    // From the input file extract the data and transform the correct way
    string input_file = args.data_path;
    int out_dim = 0;
    cout << "Loading dataset: " << input_file << endl;
    pts = load_fvecs(input_file, out_dim);
    cout << "Loaded " << pts.size() << " proteins' vectors of dimension " << (pts.empty()?0:out_dim) << endl;

    cout << "*************************************************\n";

    // From the query file extract the data and transform the correct way
    string query_file = args.query_path;
    cout << "Loading queries: " << query_file << endl;
    queries = load_fvecs(query_file, out_dim);
    cout << "Loaded " << queries.size() << " test proteins' vectors of dimension " << (queries.empty()?0:out_dim) << endl;
    

    int kproj = args.kproj, N = args.k;
    int M = args.M;
    int probes = args.probes;
    float w = args.w;
    
    // Build hypercube index
    build_hypercube(pts, kproj, w);

    vector<float> q;

    // if a subset is not provided take all the set of queries
    int queries_num = (args.queries_num==0) ? (int)queries.size() : args.queries_num;

    // ========== METRICS ==========
    double sum_tApprox_ms = 0.0;
    int qcount =0 ;
    // =============================
    #pragma omp parallel for schedule(dynamic)
    for (int i=0 ; i < queries_num; i++){
        vector <float> q = queries[i];

        // =====================================================================
        // ============================ NN Search===============================
        // =====================================================================

        // ============= Approximate search (LSH) and the time needed ==========
        #pragma omp parallel for schedule(dynamic)
        for (int i=0 ; i < queries_num; i++){
            vector<float> q = queries[i];

            auto t_start = clock_type::now();
            vector<int> nn_idx = cube_query_knn(pts, q, N, M, probes);
            auto t_end = clock_type::now();

            double t_query_ms =
                std::chrono::duration_cast<ms>(t_end - t_start).count();
        #pragma omp critical
        {
            // =====================================================================
            // ============================== Results ==============================
            // =====================================================================
            sum_tApprox_ms += t_query_ms;
            qcount++;

            cout << "Query: " << i << "\n";
            for (int j = 0; j < (int)nn_idx.size(); ++j) {
                cout << "Nearest neighbor-" << (j+1) << ": " << nn_idx[j] << "\n";
            }
            
            cout << "\n";

        }
    }
    // ================= FINAL METRICS =================
    double tApprox = (sum_tApprox_ms / qcount) / 1000.0;
    double QPS = qcount / (sum_tApprox_ms / 1000.0);

    cout << "tApprox: " << tApprox << " s\n";
    cout << "QPS: " << QPS << "\n";

}

//$./main -d <input file> -q <query file> -kproj <int> -w <float> -M <int> -probes <int> -N <number of nearest> -R <radius> 
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " -d <input file> -q <query file> -kproj <int> -w <float>" 
                                     << " -M <per-vertex> -probes <int> -N <number of nearest> -R <radius> " << endl;
        return 1;
    }
    Args args = parse_args(argc, argv);

    string type = args.type;
    search_in_dataset(args, type);
    return 0;
}