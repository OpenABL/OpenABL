#include "libabl.h"
#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

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

void save(dyn_array *arr, const char *path, const type_info *info) {
	FILE *file = fopen(path, "w");
	if (!file) {
		fprintf(stderr, "save(): Count not open \"%s\" for writing\n", path);
		return;
	}

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

	fclose(file);
}
