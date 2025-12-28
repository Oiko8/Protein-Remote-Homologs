#pragma once    // apofeugoume pollapli eisagwgh tou arxeiou sto compile (header guard)

#include "vectors.h"    // perilamvanei ton orismo tou Dataset (opou einai ta dedomena)
#include <vector>   // periexei tin klasi std::vector gia pinaka me metavlito megethos

// orismos tis domis IVFFlat pou ylopoiei ton IVF-Flat index gia Approximate Nearest Neighbor
struct IVFFlat {

	size_t kclusters = 0;       // plithos cluster (centroids) pou tha dimiourgithoun apo k-means
	size_t d = 0;       // diastasi twn dianismatwn (arithmos stoixeiwn ana vector)
	std::vector<float> centroids;       // pinakas me ola ta centroids (kclusters * d stoixeia)
	std::vector<std::vector<uint32_t>> lists;       // kathe centroid exei lista apo ids (indexes) twn dianismatwn pou anikoun ekei

	// synartisi pou kanei training tou IVF index (k-means gia ta centroids + anathesi twn dianismatwn sta clusters)
	void train_and_index(const Dataset &ds, size_t kclusters, unsigned seed=1);

	// synartisi pou kanei anazitisi (search) ston IVF index gia ena query q
	// epistrefei ta N pio kontina dianismata me tis apostaseis tous
	std::vector<std::pair<uint32_t, float>> search(const float* q, const Dataset &ds, size_t N, size_t nprobe, float R = -1.0f) const;
};
