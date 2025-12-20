#pragma once    // apofeugoume pollapli eisagwgh tou arxeiou (header guard)

#include <fstream>
#include <iostream>
#include <cstdlib>    
#include <random>      
#include <vector>
using namespace std;


// load the mnist dataset
vector<vector<float>> load_mnist_dataset(const string &filename);
// load the sift dataset
vector<vector<float>> load_sift_dataset(const string& filename);

