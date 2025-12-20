#pragma once    // apofevgoume na perilifthei to arxeio pollaples fores (header guard)

#include <cstddef>  // periexei orismous typwn opws to size_t
#include <cmath>    // periexei mathimatikes synarthseis (px pow, sqrt)
#include <vector>
using namespace std;
/* =============================== helper function to calculate euclidean distance ============================ */
float euclidean_distance(const vector<float> &p1, const vector<float> &p2);