#ifndef OPENBIMRL_C_API_H
#define OPENBIMRL_C_API_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* (*openbimrl_get_input_pointer)(uint32_t);
typedef double (*openbimrl_get_input_double)(uint32_t);
typedef uint32_t (*openbimrl_get_input_integer)(uint32_t);
typedef const char* (*openbimrl_get_input_string)(uint32_t);
typedef void (*openbimrl_set_output_pointer)(uint32_t, void*);
typedef void (*openbimrl_set_output_double)(uint32_t, double);
typedef void (*openbimrl_set_output_integer)(uint32_t, uint32_t);
typedef void (*openbimrl_set_output_string)(uint32_t, const char*);
typedef void* (*openbimrl_set_output_array)(uint32_t, size_t);

size_t request_ifc_object_json_size(void* ifc_pointer);
void ifc_object_to_json(void* buffer);

/** Load IFC file into the active session. Returns true on success. */
bool initIfc(const char* path);

size_t request_geometry_polygon(void* ifc_pointer);
void copy_geometry_polygon(void* buffer);

void calculate_path_edge_costs(const double* points_xy, uint32_t point_count,
                               const uint32_t* edge_point_indices,
                               uint32_t edge_count,
                               const void* const* passage_pointers,
                               uint32_t passage_count,
                               const void* const* obstacle_pointers,
                               uint32_t obstacle_count, double* output_costs);

void init_function(openbimrl_get_input_pointer gip,
                   openbimrl_get_input_double gid,
                   openbimrl_get_input_integer gii,
                   openbimrl_get_input_string gis,
                   openbimrl_set_output_pointer sop,
                   openbimrl_set_output_double sod,
                   openbimrl_set_output_integer soi,
                   openbimrl_set_output_string sos,
                   openbimrl_set_output_array soa);

void filterByElement(void);
void filterByGUID(void);
void getBoundingBox(void);
void getElementFrame(void);
void calculatingBuildingBounds(void);

#ifdef __cplusplus
}
#endif

#endif /* OPENBIMRL_C_API_H */
