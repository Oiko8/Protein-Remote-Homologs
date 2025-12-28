#include "euclid.h"

/* =============================== helper functions to calculate euclidean distance ============================ */

float eucliddistance(const float* a, const float* b, size_t d) {

    float s = 0.0f; // arxikopoihsh synolou (sum) me 0.0 float

    for(size_t i=0;i<d;i++){    // gia kathe diastash apo 0 mexri d-1
        float diff = a[i]-b[i]; // upologizoume th diafora metaksy tou stoixeiou a kai b
        s += diff*diff;         // prosthetoume sto sum to tetragwno ths diaforas
    }

    return s;                   // epistrefoume to athroisma twn tetragwnwn (oxi sqrt, ara squared L2)
}

float euclidean_distance(const vector<float> &p1, const vector<float> &p2) {
    float sum = 0.0;
    int size = p1.size();
    for (int i = 0 ; i < size ; i++){
        sum += (p1[i] - p2[i]) * (p1[i] - p2[i]);
    }
    return sqrt(sum);
}