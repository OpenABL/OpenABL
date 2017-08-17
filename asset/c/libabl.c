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

abl_float random_float(abl_float min, abl_float max) {
	uint64_t x = xorshift128plus();
	// This is a horrible way of generating a random float.
	// It will do for now.
	return min + (abl_float) x / (abl_float) (UINT64_MAX / (max - min));
}

static size_t type_info_get_size(const type_info *info) {
	while (info->type != TYPE_END) ++info;
	return info->offset;
}

static void save_json_agent(FILE *file, const char *agent, const type_info *info) {
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
				abl_float f = *(abl_float *) (agent + info->offset);
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

static void save_json_agents(FILE *file, dyn_array *arr, const type_info *info) {
	size_t elem_size = type_info_get_size(info);
	bool first = true;

	fputs("[", file);
	for (size_t i = 0; i < arr->len; i++) {
		if (!first) fputs(",", file);
		first = false;

		char *agent = ((char *) arr->values) + elem_size * i;
		save_json_agent(file, agent, info);
	}
	fputs("]", file);
}

void save_json(void *agents, const agent_info *info, FILE *file) {
	bool first = true;
	fputs("{", file);
	while (info->name) {
		if (!first) fputs(",", file);
		first = false;

		dyn_array *arr = (dyn_array *) ((char *) agents + info->offset);
		fprintf(file, "\"%s\":", info->name);
		save_json_agents(file, arr, info->info);
		info++;
	}
	fputs("}", file);
}

static void save_flame_xml_member(FILE *file, const char *agent, const type_info *info) {
	const char *name = info->name;
	switch (info->type) {
		case TYPE_INT:
		{
			int i = *(int *) (agent + info->offset);
			fprintf(file, "<%s>%d</%s>\n", name, i, name);
			break;
		}
		case TYPE_FLOAT:
		{
			abl_float f = *(abl_float *) (agent + info->offset);
			fprintf(file, "<%s>%f</%s>\n", name, f, name);
			break;
		}
		case TYPE_FLOAT2:
		{
			float2 *f = (float2 *) (agent + info->offset);
			fprintf(file, "<%s_x>%f</%s_x>\n<%s_y>%f</%s_y>\n",
				name, f->x, name, name, f->y, name);
			break;
		}
		case TYPE_FLOAT3:
		{
			float3 *f = (float3 *) (agent + info->offset);
			fprintf(file, "<%s_x>%f</%s_x>\n<%s_y>%f</%s_y>\n<%s_z>%f</%s_z>",
				name, f->x, name, name, f->y, name, name, f->z, name);
			break;
		}
		case TYPE_BOOL:
		default:
			assert(0);
			break;
	}
}

static void save_flame_xml_agents(
		FILE *file, dyn_array *arr, const char *name, const type_info *info_start) {
	size_t elem_size = type_info_get_size(info_start);
	for (size_t i = 0; i < arr->len; i++) {
		char *agent = ((char *) arr->values) + elem_size * i;
		fputs("<xagent>\n", file);
		fprintf(file, "<name>%s</name>\n", name);

		for (const type_info *info = info_start; info->type != TYPE_END; info++) {
			save_flame_xml_member(file, agent, info);
		}

		fputs("</xagent>\n", file);
	}
}

void save_flame_xml(void *agents, const agent_info *info, FILE *file) {
	fputs("<states>\n", file);
	fputs("<itno>0</itno>\n", file);
	// TODO: Necessary?
	/*fputs("<environment>\n", file);
	fputs("</environment>\n", file);*/

	while (info->name) {
		dyn_array *arr = (dyn_array *) ((char *) agents + info->offset);
		save_flame_xml_agents(file, arr, info->name, info->info);
		info++;
	}

	fputs("</states>\n", file);
}

void save(void *agents, const agent_info *info, const char *path, save_type type) {
	(void) type; // Ignored for now

	FILE *file = fopen(path, "w");
	if (!file) {
		fprintf(stderr, "save(): Count not open \"%s\" for writing\n", path);
		return;
	}

	if (type == SAVE_JSON) {
		save_json(agents, info, file);
	} else if (type == SAVE_FLAME_XML) {
		save_flame_xml(agents, info, file);
	}

	fclose(file);
}
