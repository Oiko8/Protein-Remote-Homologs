#include "Data_loader.h"
#include <fstream>  // Gia xeirismo arxeiwn se binary mode
#include <iostream> // Gia emfanisi minimatwn sfalmatos
#include <arpa/inet.h> // Gia ntohl (network to host long)
#include <cstring> // Gia diaxeirisi bytes kai strings

// reading the data for lsh and hypercube
// load the mnist dataset
vector<vector<float>> load_mnist_dataset(const string &filename) {
    ifstream f(filename, ios::binary);
    if (!f.is_open()) {
        cerr << "Error: cannot open " << filename << endl;
        exit(1);
    }

    uint32_t magic, num_images, rows, cols;
    f.read(reinterpret_cast<char*>(&magic), 4);
    f.read(reinterpret_cast<char*>(&num_images), 4);
    f.read(reinterpret_cast<char*>(&rows), 4);
    f.read(reinterpret_cast<char*>(&cols), 4);

    // convert from big endian to host
    auto swap32 = [](uint32_t x) {
        return ((x & 0xFF) << 24) |
               ((x & 0xFF00) << 8) |
               ((x & 0xFF0000) >> 8) |
               ((x >> 24) & 0xFF);
    };
    magic = swap32(magic);
    num_images = swap32(num_images);
    rows = swap32(rows);
    cols = swap32(cols);

    int dim = rows * cols;
    vector<vector<float>> data(num_images, vector<float>(dim));

    for (uint32_t i = 0; i < num_images; ++i) {
        for (int j = 0; j < dim; ++j) {
            unsigned char pixel;
            f.read(reinterpret_cast<char*>(&pixel), 1);
            data[i][j] = static_cast<float>(pixel);
        }
    }
    return data;
}

// load the sift dataset
vector<vector<float>> load_sift_dataset(const string& filename) {
    ifstream f(filename, ios::binary);
    if (!f) { cerr << "Error: cannot open " << filename << "\n"; exit(1); }

    vector<vector<float>> data;
    while (true) {
        uint32_t d;
        if (!f.read(reinterpret_cast<char*>(&d), 4)) break;         // EOF cleanly

        // d should be 128 per spec
        if (d != 128) {
            cerr << "Warning: vector dimension " << d << " != 128 in " << filename << "\n";
        }

        vector<float> buf_f(d);
        if (!f.read(reinterpret_cast<char*>(buf_f.data()), d * sizeof(float))) {
            cerr << "Truncated file while reading vector\n";
            break;
        }
        vector<float> v(d);
        for (uint32_t i = 0; i < d; ++i) v[i] = buf_f[i];
        data.emplace_back(std::move(v));
    }
    return data;
}