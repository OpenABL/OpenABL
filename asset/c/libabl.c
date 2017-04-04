#include "libabl.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

static uint64_t xorshift_state[2] = { 0xdeadbeef, 0xbeefdead };

static uint64_t xorshift128plus() {
	uint64_t x = xorshift_state[0];
	uint64_t const y = xorshift_state[1];
	xorshift_state[0] = y;
	x ^= x << 23; // a
	xorshift_state[1] = x ^ y ^ (x >> 17) ^ (y >> 26); // b, c
	return xorshift_state[1] + y;
}

float random_float(float min, float max) {
	uint64_t x = xorshift128plus();
	// This is a horrible way of generating a random float.
	// It will do for now.
	return min + (float) x / (float) (UINT64_MAX / (max - min));
}

static size_t type_info_get_size(const type_info *info) {
	while (info->type != TYPE_END) ++info;
	return info->offset;
}

static void save_agent(FILE *file, char *agent, const type_info *info) {
	bool first = true;
	fputs("{", file);
	while (info->type != TYPE_END) {
		if (!first) fputs(",", file);
		first = false;

		fprintf(file, "\"%s\":", info->name);

		switch (info->type) {
			case TYPE_BOOL:
			{
				bool b = *(bool *) (agent + info->offset);
				fputs(b ? "true" : "false", file);
				break;
			}
			case TYPE_INT:
			{
				int i = *(int *) (agent + info->offset);
				fprintf(file, "%d", i);
				break;
			}
			case TYPE_FLOAT:
			{
				float f = *(float *) (agent + info->offset);
				fprintf(file, "%f", f);
				break;
			}
			case TYPE_FLOAT2:
			{
				float2 *f = (float2 *) (agent + info->offset);
				fprintf(file, "[%f,%f]", f->x, f->y);
				break;
			}
			case TYPE_FLOAT3:
			{
				float3 *f = (float3 *) (agent + info->offset);
				fprintf(file, "[%f,%f,%f]", f->x, f->y, f->z);
				break;
			}
			//case TYPE_STRING:
			default:
				assert(0);
				break;
		}
		info++;
	}
	fputs("}", file);
}

static void save_agents(FILE *file, dyn_array *arr, const type_info *info) {
	size_t elem_size = type_info_get_size(info);
	bool first = true;

	fputs("[", file);
	for (size_t i = 0; i < arr->len; i++) {
		if (!first) fputs(",", file);
		first = false;

		char *agent = ((char *) arr->values) + elem_size * i;
		save_agent(file, agent, info);
	}
	fputs("]", file);
}

void save(void *agents, const agent_info *info, const char *path) {
	FILE *file = fopen(path, "w");
	if (!file) {
		fprintf(stderr, "save(): Count not open \"%s\" for writing\n", path);
		return;
	}

	bool first = true;
	fputs("{", file);
	while (info->name) {
		if (!first) fputs(",", file);
		first = false;

		dyn_array *arr = (dyn_array *) ((char *) agents + info->offset);
		fprintf(file, "\"%s\":", info->name);
		save_agents(file, arr, info->info);
		info++;
	}
	fputs("}", file);

	fclose(file);
}
