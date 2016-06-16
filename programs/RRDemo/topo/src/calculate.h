#pragma once

typedef struct vehicle_obj VEHICLE_OBJ;

int calculate_priority(VEHICLE_OBJ *veh);

int update_priority(VEHICLE_OBJ *veh);

int decode_data(const char *data, VEHICLE_OBJ *veh);

void update_call();

