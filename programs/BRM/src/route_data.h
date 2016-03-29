#pragma once

/*****************************************************************************
 * Copyright(c) shanghai... 2015-2100
 * Filename: route_data.h
 * Author: shaozhenyu 18217258834@163.com
 * History:
 *        created by shaozhenyu 2015-10-23
 * Description:
 *
 ******************************************************************************/

#include "data_model.h"
#include "libmini.h"

__BEGIN_DECLS
/*entry function*/
void *route_data(void *procentry);

void start_route_data(struct taskdata *data);

__END_DECLS

