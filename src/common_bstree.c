/*
 * common_bstree.c
 *
 * binary search tree
 *
 *  Created on: 25 янв. 2017 г.
 *      Author: mastersan
 */

#include <common_bstree.h>
#include "common_hash.h"

#include <stdlib.h>

static node_t * node_insert(node_t ** root, node_t * node)
{
	if((*root) == NULL)
	{
		(*root) = node;
		return node;
	}

	node_t * tmp = (*root);

	for(;;)
	{
		if(!tmp || node->nodeId == tmp->nodeId)
			return NULL; /* collision */
		if(node->nodeId < tmp->nodeId)
		{
			if(!tmp->l)
			{
				tmp->l = node;
				return node;
			}
			tmp = tmp->l;
		}
		else
		{
			if(!tmp->r)
			{
				tmp->r = node;
				return node;
			}
			tmp = tmp->r;
		}
	}
}

node_t * tree_node_insert(node_t ** root, nodeId_t nodeId, void * data)
{
	node_t * node = calloc(1, sizeof(node_t));
	if(!node)
		return NULL;
	node->nodeId = nodeId;
	node->data = data;

	node_t * n = node_insert(root, node);
	if(n == NULL)
	{
		free(node);
		node = NULL;
	}
	return node;
}

void tree_delete(node_t * root, void (*data_delete_cb)(void * nodedata))
{
	if(!root)
		return;
	tree_delete(root->l, data_delete_cb);
	tree_delete(root->r, data_delete_cb);
	if(data_delete_cb)
		(*data_delete_cb)(root->data);
	free(root);
}

node_t * tree_node_find(const node_t * root, nodeId_t nodeId)
{
	for(;;)
	{
		if(!root || nodeId == root->nodeId)
			break;
		if(nodeId < root->nodeId)
			root = root->l;
		else
			root = root->r;
	}
	return (node_t*)root;
}

void tree_foreach(node_t * root, void (*action_cb)(void * nodedata, void * args), void * args)
{
	if(!root)
		return;
	if(!action_cb)
		return;
	tree_foreach(root->l, action_cb, args);
	tree_foreach(root->r, action_cb, args);
	(*action_cb)(root->data, args);
}
