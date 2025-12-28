#pragma once                            // apotrefei na ginei to include tou arxeiou parapano apo mia fora

#include <vector>                       // perilamvanei to std::vector
#include <cstdint>                      // gia typous opws uint32_t

// Dataset orizetai sto ivf/vectors.h — to compiler tha to vrei logo tou -Iivf sto Makefile
#include "../src/ivf/vectors.h"             // perilamvanei ton orismo tou struct Dataset

// Dilwsi tis sinartisis nearest_neighbor (oxi ylopoihsh, mono header)
void nearest_neighbor(const Dataset &ds,   // ds: to dataset me ta dianusmata
                      const float* q,      // q: pointer sto query dianisma
                      int N,               // N: arithmos geitonwn pou theloume
                      std::vector<uint32_t> &out_ids, // exodos: ids twn kontinoterwn dianismatwn
                      std::vector<float> &out_dists); // exodos: apostaseis twn geitonwn
