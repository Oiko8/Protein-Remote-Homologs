#pragma once    // Gia apofygh pollaplasiwn eisagwgwn tou idiou header

#include <vector>   // Eisagwgh vector container apo STL
#include <cstdint>  // Typoi statherou megethous (uint32_t)
#include <cstddef>  // Typoi opws size_t

using Idx = uint32_t;   // Typos gia indexes
using Vec = std::vector<float>;     // Typos gia vector apo floats

struct Dataset {    // Struct pou apothikeuei dataset me n deigmata kai d diastaseis
    size_t n = 0;   // Arithmos deigmatwn
    size_t d = 0;   // Diastasi tou kathe deigmatos
    std::vector<float> data;    // Apothikeush se row-major: data[i*d + j]

    void reserve(size_t N, size_t D){ n=N; d=D; data.resize(N*D); } // Krataei mnhmh gia N deigmata kai D diastaseis
    const float* row(size_t i) const { return &data[i * d]; }   // Epistrefei anaparastash twn stoixeiwn ths grammhs i (const)
    float* row_mut(size_t i) { return &data[i * d]; }   // Epistrefei anaparastash twn stoixeiwn ths grammhs i gia tropopoihsh
};
