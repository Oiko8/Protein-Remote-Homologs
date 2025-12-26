#include "Data_loader.h"
#include <fstream>
#include <stdexcept>


vector<vector<float>> load_fvecs(const string& filename, int& out_dim) {
    ifstream in(filename, ios::binary);
    if (!in) throw runtime_error("Cannot open file: " + filename);

    vector<vector<float>> ds;
    out_dim = -1;

    while (true) {
        int d;
        in.read(reinterpret_cast<char*>(&d), sizeof(int));
        if (!in) break; // EOF

        if (d <= 0) throw runtime_error("Invalid dim in fvecs: " + filename);

        if (out_dim == -1) out_dim = d;
        if (d != out_dim) throw runtime_error("Inconsistent dims in fvecs: " + filename);

        vector<float> v(d);
        in.read(reinterpret_cast<char*>(v.data()), sizeof(float) * d);
        if (!in) throw runtime_error("Truncated fvecs: " + filename);

        ds.push_back(move(v));
    }

    if (out_dim == -1) out_dim = 0; // empty file
    return ds;
}
