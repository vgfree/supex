/*********************************************************************************************/
/************************	Created by 许莉 on 16/04/12.	******************************/
/*********	 Copyright © 2016年 xuli. All rights reserved.	******************************/
/*********************************************************************************************/
#ifndef __COMM_LIST_H__
#define __COMM_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef container_of

/*
 *   * container_of - cast a member of a structure out to the containing structure
 *   * @ptr: the pointer to the member.
 *   * @type: the type of the container struct this is embedded in.
 *   * @member: the name of the member within the struct.
 */
  #define container_of(ptr, type, member)			    \
	({							    \
		const typeof(((type *)0)->member) * __mptr = (ptr); \
		(type *)((char *)__mptr - offsetof(type, member));  \
	})
#endif

/**
 *   * list_entry - get the struct for this entry
 *   * @ptr: the &struct list_head pointer.
 *   * @type: the type of the struct this is embedded in.
 *   * @member: the name of the list_struct within the struct.
 **/
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#ifdef __cplusplus
}
#endif
#endif	/* ifndef __COMM_LIST_H__ */

