#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * Dynamic array
 */

typedef struct {
	void *values;
	size_t len;
	size_t cap;
} dyn_array;

#define DYN_ARRAY_INIT_CAPACITY 128

#define DYN_ARRAY_CREATE_FIXED(elem_type, len) \
	dyn_array_create_fixed(sizeof(elem_type), len)

#define DYN_ARRAY_COPY_FIXED(elem_type, other) \
	dyn_array_copy_fixed(sizeof(elem_type), other)

#define DYN_ARRAY_GET(ary, elem_type, idx) \
	(&((elem_type *) (ary)->values)[idx])

#define DYN_ARRAY_PLACE(ary, elem_type) \
	((elem_type *) dyn_array_place(ary, sizeof(elem_type)))

static inline dyn_array dyn_array_create_fixed(size_t elem_size, size_t len) {
	return (dyn_array) {
		.values = calloc(elem_size, len),
		.len = len,
		.cap = len,
	};
}

static inline dyn_array dyn_array_copy_fixed(size_t elem_size, const dyn_array *other) {
	dyn_array ary = dyn_array_create_fixed(elem_size, other->len);
	memcpy(ary.values, other->values, elem_size * other->len);
	return ary;
}

/* Returns place for new element */
static inline void *dyn_array_place(dyn_array *ary, size_t elem_size) {
	if (ary->len == ary->cap) {
		ary->cap = ary->cap ? ary->cap * 2 : DYN_ARRAY_INIT_CAPACITY;
		ary->values = realloc(ary->values, elem_size * ary->cap);
	}
	return (char *) ary->values + elem_size * ary->len++;
}

static inline void dyn_array_clean(dyn_array *ary) {
	free(ary->values);
	ary->len = ary->cap = 0;
}

#ifdef LIBABL_USE_FLOAT
typedef float abl_float;
#else
typedef double abl_float;
#endif

/*
 * Misc
 */

static inline abl_float min(abl_float x, abl_float y) {
	return fmin(x, y);
}
static inline abl_float max(abl_float x, abl_float y) {
	return fmax(x, y);
}

/*
 * float2
 */

typedef struct {
	abl_float x;
	abl_float y;
} float2;

static inline float2 float2_create(abl_float x, abl_float y) {
	return (float2) { x, y };
}
static inline float2 float2_fill(abl_float x) {
	return (float2) { x, x };
}
static inline float2 float2_add(float2 a, float2 b) {
	return (float2) { a.x + b.x, a.y + b.y };
}
static inline float2 float2_sub(float2 a, float2 b) {
	return (float2) { a.x - b.x, a.y - b.y };
}
static inline float2 float2_mul_scalar(float2 a, abl_float s) {
	return (float2) { a.x * s, a.y * s };
}
static inline float2 float2_div_scalar(float2 a, abl_float s) {
	return (float2) { a.x / s, a.y / s };
}
static inline bool float2_equals(float2 a, float2 b) {
	return a.x == b.x && a.y == b.y;
}
static inline bool float2_not_equals(float2 a, float2 b) {
	return a.x != b.x || a.y != b.y;
}

/*
 * float3
 */

typedef struct {
	abl_float x;
	abl_float y;
	abl_float z;
} float3;

static inline float3 float3_create(abl_float x, abl_float y, abl_float z) {
	return (float3) { x, y, z };
}
static inline float3 float3_fill(abl_float x) {
	return (float3) { x, x, x };
}
static inline float3 float3_add(float3 a, float3 b) {
	return (float3) { a.x + b.x, a.y + b.y, a.z + b.z };
}
static inline float3 float3_sub(float3 a, float3 b) {
	return (float3) { a.x - b.x, a.y - b.y, a.z - b.z };
}
static inline float3 float3_mul_scalar(float3 a, abl_float s) {
	return (float3) { a.x * s, a.y * s, a.z * s };
}
static inline float3 float3_div_scalar(float3 a, abl_float s) {
	return (float3) { a.x / s, a.y / s, a.z / s };
}
static inline bool float3_equals(float3 a, float3 b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}
static inline bool float3_not_equals(float3 a, float3 b) {
	return a.x != b.x || a.y != b.y || a.z != b.z;
}

/*
 * Lengths and distances
 */

static inline abl_float dot_float2(float2 a, float2 b) {
	return a.x * b.x + a.y * b.y;
}
static inline abl_float dot_float3(float3 a, float3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline abl_float length_float2(float2 v) {
	return sqrtf(v.x * v.x + v.y * v.y);
}
static inline abl_float length_float3(float3 v) {
	return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

static inline abl_float dist_float2(float2 a, float2 b) {
	return length_float2(float2_sub(a, b));
}
static inline abl_float dist_float3(float3 a, float3 b) {
	return length_float3(float3_sub(a, b));
}

static inline float2 normalize_float2(float2 v) {
	return float2_div_scalar(v, length_float2(v));
}
static inline float3 normalize_float3(float3 v) {
	return float3_div_scalar(v, length_float3(v));
}

/*
 * Random numbers
 */

abl_float random_float(abl_float min, abl_float max);
int random_int(int min, int max);

/*
 * Runtime type information
 */

typedef enum {
	TYPE_END,
	TYPE_BOOL,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING,
	TYPE_FLOAT2,
	TYPE_FLOAT3,
} type_id;

typedef struct {
	type_id type;
	unsigned offset;
	const char *name;
	bool is_pos;
} type_info;

typedef struct {
	const type_info *info;
	unsigned offset;
	const char *name;
} agent_info;

typedef enum {
	SAVE_JSON,
	SAVE_FLAME_XML,
	SAVE_FLAMEGPU_XML,
} save_type;

void save(void *agents, const agent_info *info, const char *path, save_type type);
