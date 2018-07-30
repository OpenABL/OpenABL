#ifndef LIBABL_FLAMEGPU_H
#define LIBABL_FLAMEGPU_H

static inline __device__ float random(RNG_rand48 *rand48, float min, float max) {
	return min + rnd<CONTINUOUS>(rand48) * (max - min);
}

static inline __device__ int randomInt(RNG_rand48 *rand48, int min, int max) {
	return min + (int) (rnd<CONTINUOUS>(rand48) * (max - min + 1));
}

#endif
