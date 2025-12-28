#include "kmeans.h"   // Header me dilwseis gia ton algorithmo KMeans
#include "utils_functions/euclid.h"   // Gia ypologismo apostasis (eucliddistance)
#include <limits>     // Gia orismous arithmitikwn oriwn
#include <algorithm>  // Gia leitourgies opws fill kai min
#include <iostream>   // Gia debug ektypwseis
#include <random>     // Gia tuxaious arithmous
#include <cstring>    // Gia memcpy
#include <cassert>    // Gia elegxous me assert

// Epistrefei to index tou kontinoterou centroid
static size_t argmin_centroid(const float* v, const std::vector<float> &centroids, size_t k, size_t d){
    size_t best = 0; 
    float bestd = eucliddistance(v, &centroids[0], d); // Apostasi apo to prwto kentro
    for(size_t c=1;c<k;c++){ // Elegxei ola ta ypoloipa kentra
        float dcur = eucliddistance(v, &centroids[c*d], d);
        if(dcur < bestd){ bestd = dcur; best = c; } // An brei mikroteri apostasi, kratame to neo kalytero
    }
    return best; // Epistrefei to index tou kontinoterou centroid
}

// Ekpaideusi tou KMeans kai epistrefei ta kentra
std::vector<float> KMeans::train(const Dataset &ds, size_t k, unsigned seed, size_t max_iters, float tol){
    size_t n = ds.n, d = ds.d; // Arithmos deigmatwn kai diastasi
    std::mt19937 rng(seed); // Random generator me seed
    std::uniform_int_distribution<size_t> unif(0, n-1); // Gia epilogi tuxaiwn deigmatwn

    std::vector<float> centroids(k * d); // Apothikeuei ta kentra

    // tyxaia arxikopoiisi tou prwtou centroid
    std::vector<size_t> centers_idx; centers_idx.reserve(k); // Pinakas me indexes twn epilegmenwn kentrwn
    size_t first = unif(rng); centers_idx.push_back(first); // Epilegei tuxaia ena deigma ws prwto kentro
    memcpy(&centroids[0*d], ds.row(first), sizeof(float)*d); // Antigrafei ta stoixeia tou deigmatos

    // kmeans++ arxikopoiisi twn upoloipown centroids
    for(size_t c=1; c<k; c++){ // Gia kathe kentro pou apomenei
        std::vector<double> dist(n); double sum = 0; // Pinakas gia apostaseis kai to athroisma tous
        for(size_t i=0; i<n; i++){ // Gia kathe deigma
            double mind = eucliddistance(ds.row(i), &centroids[0*d], d); // Apostasi apo to prwto kentro
            for(size_t j=1; j<centers_idx.size(); j++){ // Elegxei ola ta hdh epilegmena kentra
                double dd = eucliddistance(ds.row(i), &centroids[j*d], d);
                if(dd < mind) mind = dd; // Krataei tin mikroteri apostasi
            }
            dist[i] = mind; sum += mind; // Apothikeuei kai auksanei to athroisma
        }

        // Epilogh neou kentrou me pithanotita analogi tis apostasis
        std::uniform_real_distribution<double> r(0, sum); double target = r(rng);
        double acc = 0; size_t pick = 0;
        for(size_t i=0; i<n; i++){ acc += dist[i]; if(acc >= target){ pick = i; break; } }
        centers_idx.push_back(pick); // Apothikeuei to epilegmeno deigma ws kentro
        memcpy(&centroids[c*d], ds.row(pick), sizeof(float)*d); // Antigrafei ta stoixeia tou
    }

    // Epanalamvanoume ton KMeans
    std::vector<uint32_t> assign(n); // Pinakas pou deixnei se poio kentro anoikei kathe deigma
    std::vector<float> newc(k*d);    // Prosorina nea kentra
    std::vector<size_t> counts(k);   // Plithos deigmatwn ana kentro

    for(size_t it=0; it<max_iters; ++it){ // Epanalipseis mexri sygklisi i sto orio
        for(size_t i=0; i<n; i++) assign[i] = argmin_centroid(ds.row(i), centroids, k, d); // Anathesi se kontinotero kentro

        std::fill(newc.begin(), newc.end(), 0.0f); std::fill(counts.begin(), counts.end(), 0); // Midenismos gia neo ypologismo

        for(size_t i=0; i<n; i++){ // Prosthiki twn deigmatwn sto antistoixo kentro
            size_t c = assign[i]; counts[c]++;
            float* dst = &newc[c*d]; const float* src = ds.row(i);
            for(size_t j=0; j<d; j++) dst[j] += src[j]; // Athroisma twn suntetagmenwn
        }

        bool converged = true; // Elegxei an exei ginei sygklisi
        for(size_t c=0; c<k; c++){
            if(counts[c] == 0){ // An ena kentro einai adeio
                size_t ridx = unif(rng); // Epilegei tuxaio deigma
                memcpy(&centroids[c*d], ds.row(ridx), sizeof(float)*d); // To antikathista
                converged = false; continue;
            }
            for(size_t j=0; j<d; j++){
                float val = newc[c*d + j] / static_cast<float>(counts[c]); // Mesos oros
                if(std::fabs(val - centroids[c*d + j]) > tol) converged = false; // Elegxos allagis
                centroids[c*d + j] = val; // Enimerwsi kentrou
            }
        }
        if(converged) break; // Termatismos an den yparxoun allages
    }
    return centroids; // Epistrefei ta telika kentra
}

// Anathesi kathe deigmatos sto kontinotero kentro
void KMeans::assign_all(const Dataset &ds, const std::vector<float> &centroids, std::vector<uint32_t> &assignments){
    size_t n = ds.n, k = centroids.size()/ds.d; assignments.resize(n); // Proetoimasia megethwn
    for(size_t i=0;i<n;i++) assignments[i] = argmin_centroid(ds.row(i), centroids, k, ds.d); // Ypologismos anathesis
}
