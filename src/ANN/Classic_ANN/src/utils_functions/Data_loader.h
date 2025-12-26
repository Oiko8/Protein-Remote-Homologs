// fvecs_loader.h
#pragma once
#include <string>
#include <vector>

using namespace std;

vector<vector<float>> load_fvecs(const std::string& filename, int& out_dim);
