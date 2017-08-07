#ifndef LIBABL_FLAMEGPU_H
#define LIBABL_FLAMEGPU_H

static inline __device__ float random(RNG_rand48 *rand48, float min, float max) {
	return min + rnd(rand48) * (max - min);
}

#endif
