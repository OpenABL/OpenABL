#include <stdlib.h>
#include <math.h>

/*
 * Dynamic array
 */

typedef struct {
	void *values;
	size_t len;
	size_t cap;
} dyn_array;

#define DYN_ARRAY_CREATE_FIXED(elem_type, len) \
	dyn_array_create_fixed(sizeof(elem_type), len)

#define DYN_ARRAY_GET(ary, elem_type, idx) \
	(((elem_type *) (ary)->values)[idx])

static inline dyn_array dyn_array_create_fixed(size_t elem_size, size_t len) {
	return (dyn_array) {
		.values = calloc(elem_size, len),
		.len = len,
		.cap = len,
	};
}

static inline void dyn_array_release(dyn_array *ary) {
	free(ary->values);
}

/*
 * float2
 */

typedef struct {
	float x;
	float y;
} float2;

static inline float2 float2_add(float2 a, float2 b) {
	return (float2) { a.x + b.x, a.y + b.y };
}
static inline float2 float2_sub(float2 a, float2 b) {
	return (float2) { a.x - b.x, a.y - b.y };
}
static inline float2 float2_mul_scalar(float2 a, float s) {
	return (float2) { a.x * s, a.y * s };
}
static inline float2 float2_div_scalar(float2 a, float s) {
	return (float2) { a.x / s, a.y / s };
}

/*
 * float3
 */

typedef struct {
	float x;
	float y;
	float z;
} float3;

static inline float3 float3_add(float3 a, float3 b) {
	return (float3) { a.x + b.x, a.y + b.y, a.z + b.z };
}
static inline float3 float3_sub(float3 a, float3 b) {
	return (float3) { a.x - b.x, a.y - b.y, a.z - b.z };
}
static inline float3 float3_mul_scalar(float3 a, float s) {
	return (float3) { a.x * s, a.y * s, a.z * s };
}
static inline float3 float3_div_scalar(float3 a, float s) {
	return (float3) { a.x / s, a.y / s, a.z / s };
}

/*
 * Distances
 */

static inline float dist_float2(float2 a, float2 b) {
	float2 d = float2_sub(a, b);
	return sqrtf(d.x * d.x + d.y * d.y);
}
static inline float dist_float3(float3 a, float3 b) {
	float3 d = float3_sub(a, b);
	return sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
}
