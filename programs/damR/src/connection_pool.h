//
//  network_connect_pool.h
//  supex
//
//  Created by 周凯 on 15/9/19.
//  Copyright © 2015年 zk. All rights reserved.
//

#ifndef connection_pool_h
#define connection_pool_h

#include "data_model.h"

__BEGIN_DECLS

/*
 * 添加连接
 */
bool add_connection(struct iohandle *io);

/*
 * 删除连接
 */
void delete_connection(struct iohandle *io);

__END_DECLS
#endif	/* connection_pool_h */

