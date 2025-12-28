#pragma once    // apofevgoume na perilifthei to arxeio pollaples fores (header guard)

#include <cstddef>  // periexei orismous typwn opws to size_t
#include <cmath>    // periexei mathimatikes synarthseis (px pow, sqrt)
#include <vector>
using namespace std;

// dilwsi inline synarthshs gia ypologismo tetragwnikhs apostashs L2 (euclidean distance)
float eucliddistance(const float* a, const float* b, size_t d); 
float euclidean_distance(const vector<float> &p1, const vector<float> &p2);