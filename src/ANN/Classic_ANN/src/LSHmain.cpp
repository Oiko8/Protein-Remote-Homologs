// Usage : 
// MNIST : ./search -d ../MNIST_data/input.dat -q ../MNIST_data/query.dat -k 3 -L 12 -w 6.0 -N 5 -R 5 -type mnist -range true 
// SIFT  : ./search -d ../SIFT_data/input.dat -q ../SIFT_data/query.dat  -k 3 -L 8 -w 250.0 -N 5 -R 200 -type sift -range true

#include "Euclidean_Hashing.h"
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
    int L = 5;                // -L
    int khash = 4;            // -k
    int queries_num = 0;      // subset of queries you choose to run 
    float w = 4.0;            // -w
    bool norm = true;
};

// static bool str2bool(const string& s){
//     return (s=="1"||s=="true"||s=="True"||s=="TRUE");
// }

static Args parse_args(int argc, char** argv){
    Args a;
    for (int i=1; i<argc; ++i){
        string flag = argv[i];
        auto need = [&](int more){ if(i+more>=argc) { cerr<<"Missing value after "<<flag<<"\n"; exit(1);} };
        if (flag=="-d") { need(1); a.data_path=argv[++i]; }
        else if (flag=="-q") { need(1); a.query_path=argv[++i]; }
        else if (flag=="-type"){ need(1); a.type=argv[++i]; }
        else if (flag=="-N") { need(1); a.k=stoi(argv[++i]); }
        else if (flag=="-L") { need(1); a.L=stoi(argv[++i]); }
        else if (flag=="-k") { need(1); a.khash=stoi(argv[++i]); }
        else if (flag=="-w") { need(1); a.w=stod(argv[++i]); }
        else if (flag=="-subset"){ a.queries_num = stoi(argv[++i]); }
        else { cerr<<"Unknown flag: "<<flag<<"\n"; exit(1);}
    }
    if (a.data_path.empty()){
        cerr<<"Usage: "<<argv[0]<<" -d <input.dat> [-q <query.dat>] [-type mnist|sift] "
              "[-k <khash>] -L <L> -w <w> -N <N> -R <R> -range <true|false> -lsh\n";
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
        cout << "Loaded " << pts.size() << " images of dimension " << pts[0].size() << endl;
    
        cout << "*************************************************\n";
    
        // From the query file extract the data and transform the correct way
        string query_file = args.query_path;
        cout << "Loading queries: " << query_file << endl;
        queries = load_mnist_dataset(query_file);
        cout << "Loaded " << queries.size() << " test images of dimension " << queries[0].size() << endl;
        
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
        cout << "Loaded " << pts.size() << " images of dimension " << pts[0].size() << endl;

        cout << "*************************************************\n";

        // From the query file extract the data and transform the correct way
        string query_file = args.query_path;
        cout << "Loading queries: " << query_file << endl;
        queries = load_sift_dataset(query_file);
        cout << "Loaded " << queries.size() << " test images of dimension " << queries[0].size() << endl;

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

    int L = args.L, khash = args.khash, N = args.k;
    float w = args.w;
    
    // Prepare the tables according to the data
    build_hash_tables(pts, L, khash, w);


    // vector<float> q;

    // if a subset is not provided take all the set of queries
    int queries_num = (args.queries_num==0) ? (int)queries.size() : args.queries_num;
    
    #pragma omp parallel for schedule(dynamic)
    for (int i=0 ; i < queries_num; i++){
        vector <float> q = queries[i];

        // =====================================================================
        // ============================ NN Search===============================
        // =====================================================================

        // ============= Approximate search (LSH) and the time needed ==========
        vector<int> nn_idx = query_knn(pts, q, N);
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


//$./search –d <input file> –q <query file> –k <int> -L <int> -w <float> -ο <output file> -Ν <number of nearest> -R <radius> 
int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " -d <input file> -q <query file> -k <int> -L <int> -w <float>" 
                                     << " -o <output file> -N <number of nearest> -R <radius> " << endl;
        return 1;
    }
    Args args = parse_args(argc, argv);

    string type = args.type;
    search_in_dataset(args, type);
    return 0;
}
