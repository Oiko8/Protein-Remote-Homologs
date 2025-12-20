#include "euclid.h"

/* =============================== helper function to calculate euclidean distance ============================ */
float euclidean_distance(const vector<float> &p1, const vector<float> &p2) {
    float sum = 0.0;
    int size = p1.size();
    for (int i = 0 ; i < size ; i++){
        sum += (p1[i] - p2[i]) * (p1[i] - p2[i]);
    }
    return sqrt(sum);
}