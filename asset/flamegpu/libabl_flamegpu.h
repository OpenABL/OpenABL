#ifndef LIBABL_FLAMEGPU_H
#define LIBABL_FLAMEGPU_H

static inline __device__ float random_float(RNG_rand48 *rand48, float min, float max) {
	return min + rnd(rand48) * (max - min);
}

static inline __device__ glm::vec2 random_float2(
		RNG_rand48 *rand48, glm::vec2 min, glm::vec2 max) {
	return glm::vec2(
		random_float(rand48, min.x, max.x),
		random_float(rand48, min.y, max.y)
	);
}
static inline __device__ glm::vec3 random_float3(
		RNG_rand48 *rand48, glm::vec3 min, glm::vec3 max) {
	return glm::vec3(
		random_float(rand48, min.x, max.x),
		random_float(rand48, min.y, max.y),
		random_float(rand48, min.z, max.z)
	);
}

#endif
