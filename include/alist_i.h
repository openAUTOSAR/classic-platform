/*-------------------------------- Arctic Core ------------------------------
 * Copyright (C) 2013, ArcCore AB, Sweden, www.arccore.com.
 * Contact: <contact@arccore.com>
 * 
 * You may ONLY use this file:
 * 1)if you have a valid commercial ArcCore license and then in accordance with  
 * the terms contained in the written license agreement between you and ArcCore, 
 * or alternatively
 * 2)if you follow the terms found in GNU General Public License version 2 as 
 * published by the Free Software Foundation and appearing in the file 
 * LICENSE.GPL included in the packaging of this file or here 
 * <http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt>
 *-------------------------------- Arctic Core -----------------------------*/








#ifndef ALIST_I_H_
#define ALIST_I_H_

/*
 * Some macros to handle a static array and it's data.
 *
 *
 * Usage:
 * struct foo_s {
 *   int foo_data;
 * };
 *
 * struct foo_s my_data[5];
 *
 * // Create the head
 * SA_LIST_HEAD(foo,foo_s) arr_list;
 * // Init the head with data.
 * arr_list = SA_LIST_HEAD_INITIALIZER(5,my_data);
 *
 */


/**
 * @def ALIST_HEAD(name, type)
 * Declare the head for the static list
 *
 * @param name - name of the struct
 * @param type - struct type for the array
 */


#define	SA_LIST_HEAD(name, type)	\
struct name {						\
	int cnt;						\
	struct type *data;				\
}

#define SA_LIST_HEAD_INITIALIZER(elem_cnt, data_p ) \
{													\
	.cnt = (elem_cnt),								\
	.data = (data_p)								\
}

#define SA_LIST_CNT(head)					(head)->cnt
#define SA_LIST_GET( head, index ) 			(&(head)->data[(index)])
#define SA_LIST_FOREACH( head, ivar)		for( (ivar)=0;(ivar)<SA_LIST_CNT(head);(ivar)++)

#endif /*ALIST_I_H_*/
