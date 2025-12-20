// Usage : 
// MNIST : ./main -d ../MNIST_data/input.dat -q ../MNIST_data/query.dat -kproj 10 -w 10.0 -M 20 -probes 8 -N 4 -R 5.0 -type mnist -range false
#include "Hypercube.h"
#include "utils_functions/Data_loader.h"
#include "utils_functions/euclid.h"
#include <fstream>

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
    if (type == "mnist"){
        // From the input file extract the data and transform the correct way
        string input_file = args.data_path;
        cout << "Loading dataset: " << input_file << endl;
        pts = load_mnist_dataset(input_file);
        cout << "Loaded " << pts.size() << " images of dimension " << (pts.empty()?0:pts[0].size()) << endl;
    
        cout << "*************************************************\n";
    
        // From the query file extract the data and transform the correct way
        string query_file = args.query_path;
        cout << "Loading queries: " << query_file << endl;
        queries = load_mnist_dataset(query_file);
        cout << "Loaded " << queries.size() << " test images of dimension " << (queries.empty()?0:queries[0].size()) << endl;
    
        // Normalize the vectors: 0-255 --> 0-1
        // find max number of the set
        float max = pts[0][0];
        for (auto p : pts) {
            for (auto v : p) {
                if (v > max) max = v;
            }
        }
        if (args.norm == true) {
            for (auto &point : pts){
                for (float &dim : point) {
                    dim /= max;
                }
            }
            for (auto &query : queries){
                for (float &dim : query) {
                    dim /= max;
                }
            }
        }
    }
    else if (type == "sift") {
        // From the input file extract the data and transform the correct way
        string input_file = args.data_path;
        cout << "Loading dataset: " << input_file << endl;
        pts = load_sift_dataset(input_file);
        cout << "Loaded " << pts.size() << " images of dimension " << (pts.empty()?0:pts[0].size()) << endl;

        cout << "*************************************************\n";

        // From the query file extract the data and transform the correct way
        string query_file = args.query_path;
        cout << "Loading queries: " << query_file << endl;
        queries = load_sift_dataset(query_file);
        cout << "Loaded " << queries.size() << " test images of dimension " << (queries.empty()?0:queries[0].size()) << endl;

        // Normalize the vectors: --> 0-1
        // find max number of the set
        float max = pts[0][0];
        for (auto p : pts) {
            for (auto v : p) {
                if (v > max) max = v;
            }
        }
        if (args.norm == true) {
            for (auto &point : pts){
                for (float &dim : point) {
                    dim /= max;
                }
            }
            for (auto &query : queries){
                for (float &dim : query) {
                    dim /= max;
                }
            }
        }
    }

    int kproj = args.kproj, N = args.k;
    int M = args.M;
    int probes = args.probes;
    float w = args.w;
    
    // Build hypercube index
    build_hypercube(pts, kproj, w);

    vector<float> q;

    // if a subset is not provided take all the set of queries
    int queries_num = (args.queries_num==0) ? (int)queries.size() : args.queries_num;

    #pragma omp parallel for schedule(dynamic)
    for (int i=0 ; i < queries_num; i++){
        vector <float> q = queries[i];

        // =====================================================================
        // ============================ NN Search===============================
        // =====================================================================

        // ============= Approximate search (LSH) and the time needed ==========
        vector<int> nn_idx = cube_query_knn(pts, q, N, M, probes);
        #pragma omp critical
        {
            // =====================================================================
            // ============================== Results ==============================
            // =====================================================================
            cout << "Query: " << i << "\n";
            for (int j = 0; j < (int)nn_idx.size(); ++j) {
                cout << "Nearest neighbor-" << (j+1) << ": " << nn_idx[j] << "\n";
            }
            
            cout << "\n";

        }
    }

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