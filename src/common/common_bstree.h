/*
 * common_bstree.h
 *
 * binary search tree
 *
 *  Created on: 25 янв. 2017 г.
 *      Author: mastersan
 */

#ifndef SRC_COMMON_BSTREE_H_
#define SRC_COMMON_BSTREE_H_

#include <inttypes.h>

typedef uint32_t nodeId_t;

typedef struct node_s
{
	struct node_s * l;
	struct node_s * r;
	nodeId_t nodeId;
	void * data;
} node_t;

extern node_t * tree_node_insert(node_t ** root, nodeId_t nodeId, void * data);
extern void tree_delete(node_t * root, void (*data_delete_cb)(void * data));
extern node_t * tree_node_find(const node_t * root, nodeId_t nodeId);
extern void tree_foreach(node_t * root, void (*action_cb)(void * nodedata, void * args), void * args);

#endif /* SRC_COMMON_BSTREE_H_ */
