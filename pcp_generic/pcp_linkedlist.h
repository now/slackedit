/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_linkedlist.h
 * Created    : not known (before 12/21/99)
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   : Header for the pcp doubly linked list 
 *              				  implementation
 *              
 *****************************************************************/

#ifndef __PCP_LINKEDLIST_H
#define __PCP_LINKEDLIST_H

#include "pcp_generic.h"

#define LL_UNLIMITEDSIZE	0

typedef struct tagLISTNODE
{
	PVOID				pData;
	struct tagLISTNODE	*pPrevNode;
	struct tagLISTNODE	*pNextNode;
} LISTNODE, *PLISTNODE;

typedef PVOID (*LISTCREATEDATAPROC)(PVOID pData);
typedef INT (*LISTDELETEDATAPROC)(PVOID pData);
typedef INT (*LISTDUPLICATENODEPROC)(PLISTNODE pNode1, PLISTNODE pNode2);
typedef INT (*LISTCOMPAREDATAPROC)(const PVOID pData, const PVOID pFromCall, UINT uType);

typedef struct tagLINKEDLIST
{
	PLISTNODE				pHeadNode;
	PLISTNODE				pTailNode;
	INT						cItems;
	INT						nMaxItems;
	LISTCREATEDATAPROC		ListCreateData;
	LISTDELETEDATAPROC		ListDeleteData;
	LISTDUPLICATENODEPROC	ListDuplicateNode;
	LISTCOMPAREDATAPROC		ListCompareData;
} LINKEDLIST, *PLINKEDLIST;

/*	The four functions are:
	ListCreateData:		Called when data should be created.
						Application should return a pointer to the data created.
	ListDeleteData:		Called when data should be destroyed.
						Application should free/delete/destroy the node data.
	ListDuplicateNode:	Called when a duplicate node is found.
						Application can return one of the following:
						0	:	Do nothing.
						1	:	Remove duplicate.
						2	:	Add duplicate to the list.
	ListCompareData:	Called when two nodes are compared.
						Application should return a number of type,
						< 0, 0, > 0, depending on relationship of the two
						nodes (i.e. their data should be compared)
*/

/* Linked List Primitives */
FOREXPORT BOOL List_AddNodeAtHead(PLINKEDLIST pList, PVOID pData);
FOREXPORT BOOL List_AddNodeAtTail(PLINKEDLIST pList, PVOID pData);
FOREXPORT BOOL List_AddNodeAscend(PLINKEDLIST pList, PVOID pData, UINT uDataType);
FOREXPORT PLINKEDLIST List_CreateList(LISTCREATEDATAPROC lpfnListCreateData,
				LISTDELETEDATAPROC lpfnListDeleteData,
				LISTDUPLICATENODEPROC lpfnListDuplicateNode,
				LISTCOMPAREDATAPROC lpfnListCompareData,
				INT nMaxItems
				);
FOREXPORT void List_DestroyList(PLINKEDLIST pList);

FOREXPORT BOOL List_DeleteNode(PLINKEDLIST pList, PLISTNODE pNode);
FOREXPORT PLISTNODE List_FindNode(PLINKEDLIST pList, const PVOID pData, UINT uDataType);
FOREXPORT PLISTNODE List_FindNodeAscend(PLINKEDLIST pList, const PVOID pData, UINT uDataType);
FOREXPORT PLISTNODE List_PrevNode(PLISTNODE pNode, INT nDelta);
FOREXPORT PLISTNODE List_NextNode(PLISTNODE pNode, INT nDelta);
FOREXPORT PLISTNODE List_GetNode(PLINKEDLIST pList, INT iIndex);

FOREXPORT int List_GetNodeCount(PLINKEDLIST pList);

FOREXPORT __inline BOOL List_IsEmpty(PLINKEDLIST pList);
FOREXPORT __inline BOOL List_RemoveTail(PLINKEDLIST pList);
FOREXPORT __inline BOOL List_RemoveHead(PLINKEDLIST pList);

#endif /* __PCP_LINKEDLIST_H */
