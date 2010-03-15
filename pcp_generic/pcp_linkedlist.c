/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_linkedlist.c
 * Created    : not known (before 12/21/99)
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   : 
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"

#include "pcp_linkedlist.h"
#include "pcp_mem.h"

PLISTNODE List_CreateNode(PLINKEDLIST pList, PVOID pData);

BOOL List_AddNodeAtHead(PLINKEDLIST pList, PVOID pData)
{
    PLISTNODE pNode;

    pNode = List_CreateNode(pList, pData);

    if (pNode == NULL)
        return (FALSE);

    // Add the node (at head of list)
    if (pList->pHeadNode == NULL)
    {
        // become head (and tail)
        pList->pHeadNode = pList->pTailNode = pNode;
    }
    else
    {
        // We're putting it at the head so move down the previous head
        pList->pHeadNode->pPrevNode = pNode;
        pNode->pNextNode = pList->pHeadNode;
        pList->pHeadNode = pNode;
    }

    pList->cItems++;

    // Has the maximum amount of items been reached? (0 for unlimited)
    if (pList->nMaxItems != 0 && pList->cItems > pList->nMaxItems)
    {
        // Remove the tail (last item is pushed off)
        List_DeleteNode(pList, pList->pTailNode);
    }

    return (TRUE);
}

BOOL List_AddNodeAtTail(PLINKEDLIST pList, PVOID pData)
{
    PLISTNODE pNode;

    pNode = List_CreateNode(pList, pData);

    if (pNode == NULL)
        return (FALSE);

    // Add the node (at tail of list)
    if (pList->pTailNode == NULL)
    {
        // become head and tail
        pList->pHeadNode = pList->pTailNode = pNode;
    }
    else
    {
        pList->pTailNode->pNextNode = pNode;
        pNode->pPrevNode = pList->pTailNode;
        pList->pTailNode = pNode;
    }

    pList->cItems++;

    // Has the maximum amount of items been reached? (0 for unlimited)
    if (pList->nMaxItems != 0 && pList->cItems > pList->nMaxItems)
    {
        // Remove the head (first item is pushed off)
        List_DeleteNode(pList, pList->pHeadNode);
    }

    return (TRUE);
}

BOOL List_AddNodeAscend(PLINKEDLIST pList, PVOID pData, UINT uDataType)
{
    PLISTNODE   pNode;
    PLISTNODE   pPrevNode;
    PLISTNODE   pCurrentNode;
    LISTNODE    lnDummyNode;
    INT         iCompare = 0;

    pNode = List_CreateNode(pList, pData);

    if (pNode == NULL)
        return (FALSE);

    // Attach dummy node at head
    lnDummyNode.pNextNode   = pList->pHeadNode;
    lnDummyNode.pPrevNode   = NULL;
    if (lnDummyNode.pNextNode != NULL)
        lnDummyNode.pNextNode->pPrevNode = &lnDummyNode;

    pPrevNode       = &lnDummyNode;
    pCurrentNode    = lnDummyNode.pNextNode;


    for ( ; pCurrentNode != NULL; pPrevNode = pCurrentNode, pCurrentNode->pNextNode)
    {
        iCompare = pList->ListCompareData(pNode->pData, pCurrentNode->pData, uDataType);
        if (iCompare <= 0)
            break; // pNode equals or precedes pCurrentNode
    }

    if (pCurrentNode != NULL && iCompare == 0)
    {
        iCompare = pList->ListDuplicateNode(pNode, pCurrentNode);
        if (iCompare == 2)
        {
            ; // (Do Nothing) Insert
        }
        else
        {
            // Restore linked list
            pList->pHeadNode = lnDummyNode.pNextNode;
            pList->pHeadNode->pPrevNode = NULL;

            // Remove duplicate node ?
            if (iCompare == 1)
            {
                pList->ListDeleteData(pNode->pData);
                Mem_Free(pNode);
            }

            return (TRUE);
        }
    }

    pPrevNode->pNextNode    = pNode;
    pNode->pPrevNode        = pPrevNode;
    pNode->pNextNode        = pCurrentNode;
    if (pCurrentNode != NULL)
        pCurrentNode->pPrevNode = pNode;
    else
        pList->pTailNode = pNode; // Put the node as tail

    pList->cItems++;

    // Remove dummy node
    pList->pHeadNode = lnDummyNode.pNextNode;
    pList->pHeadNode->pPrevNode = NULL;

    return (TRUE);
}

PLINKEDLIST List_CreateList(LISTCREATEDATAPROC lpfnListCreateData,
                LISTDELETEDATAPROC lpfnListDeleteData,
                LISTDUPLICATENODEPROC lpfnListDuplicateNode,
                LISTCOMPAREDATAPROC lpfnListCompareData,
                INT nMaxItems
                )
{
    PLINKEDLIST pList;

    pList = (PLINKEDLIST)Mem_Alloc(sizeof(LINKEDLIST));
    if (pList == NULL)
        return (NULL);

    pList->pHeadNode    = NULL;
    pList->pTailNode    = NULL;
    pList->cItems       = 0;
    pList->nMaxItems    = nMaxItems;

    pList->ListCreateData       = lpfnListCreateData;
    pList->ListDeleteData       = lpfnListDeleteData;
    pList->ListDuplicateNode    = lpfnListDuplicateNode;
    pList->ListCompareData      = lpfnListCompareData;

    return (pList);
}

void List_DestroyList(PLINKEDLIST pList)
{
    PLISTNODE pNode, pTempNode;

    ASSERT(pList != NULL);

    if (pList->pHeadNode == NULL)
        return;

    for (pNode = pList->pHeadNode; pNode != NULL;)
    {
        pTempNode = pNode->pNextNode;
        List_DeleteNode(pList, pNode);
        pNode = pTempNode;
    }

    Mem_Free(pList);
}

PLISTNODE List_CreateNode(PLINKEDLIST pList, PVOID pData)
{
    PLISTNODE pNode;

    pNode = (PLISTNODE)Mem_Alloc(sizeof(LISTNODE));
    if (pNode == NULL)
        return (NULL);

    pNode->pPrevNode    = NULL;
    pNode->pNextNode    = NULL;
    pNode->pData        = ((pList->ListCreateData != NULL) ? pList->ListCreateData(pData) : pData);

    if (pNode->pData == NULL)
    {
        Mem_Free(pNode);
        return (NULL);
    }

    return (pNode);
}

BOOL List_DeleteNode(PLINKEDLIST pList, PLISTNODE pNode)
{
    PLISTNODE pTempNode;

    if (pNode == NULL)
        return (FALSE);

    if (pNode->pPrevNode == NULL) // Head
    {
        pList->pHeadNode = pNode->pNextNode;
        if (pList->pHeadNode != NULL)
            pList->pHeadNode->pPrevNode = NULL;
    }
    else if (pNode->pNextNode == NULL) // Tail
    {
        pTempNode = pNode->pPrevNode;
        pTempNode->pNextNode = NULL;
        pList->pTailNode = pTempNode;
    }
    else
    {
        pTempNode = pNode->pPrevNode;
        pTempNode->pNextNode = pNode->pNextNode;

        pTempNode = pNode->pNextNode;
        pTempNode->pPrevNode = pNode->pPrevNode;
    }

    pList->ListDeleteData(pNode->pData);
    Mem_Free(pNode);
    pList->cItems--;

    return (TRUE);
}

PLISTNODE List_FindNode(PLINKEDLIST pList, const PVOID pData, UINT uDataType)
{
    PLISTNODE pCurrentNode;

    if (pList->pHeadNode == NULL) // Empty list
        return (NULL);

    for (pCurrentNode = pList->pHeadNode; pCurrentNode != NULL; pCurrentNode = pCurrentNode->pNextNode)
    {
        // This is highly unoptimzied =\...

        if (pList->ListCompareData != NULL)
        {
            if (pList->ListCompareData(pCurrentNode->pData, pData, uDataType) == 0) // exact match
                return (pCurrentNode);
        }
        else
        {
            if (pCurrentNode->pData == pData)
                return (pCurrentNode);
        }
    }

    return (NULL);
}

PLISTNODE List_FindNodeAscend(PLINKEDLIST pList, PVOID pData, UINT uDataType)
{
    PLISTNODE pCurrentNode;
    int iCompare;

    if (pList->pHeadNode == NULL) // Empty list
        return (NULL);

    for (pCurrentNode = pList->pHeadNode; pCurrentNode != NULL; pCurrentNode = pCurrentNode->pNextNode)
    {
        iCompare = pList->ListCompareData(pData, pCurrentNode->pData, uDataType);

        if (iCompare < 0)
            return (NULL);

        if (iCompare == 0)
            return (pCurrentNode);
    }

    return (NULL);
}

PLISTNODE List_GetNode(PLINKEDLIST pList, INT iIndex)
{
    PLISTNODE pTempNode;
    int i;

    if (pList->pHeadNode == NULL)
        return (NULL);

    for (i = 0, pTempNode = pList->pHeadNode; pTempNode != NULL && i < iIndex; pTempNode = pTempNode->pNextNode, i++)
    {
        if (pList->nMaxItems != 0 && (i + 1) == pList->nMaxItems)
            break;
    }

    return (pTempNode);
}

int List_GetNodeCount(PLINKEDLIST pList)
{
    int i;
    PLISTNODE pNode = pList->pHeadNode;

    for (i = 0; pNode != NULL; i++)
        pNode = pNode->pNextNode;

    return (i);
}


PLISTNODE List_NextNode(PLISTNODE pNode, INT nDelta)
{
    ASSERT(nDelta > 0);

    while (nDelta-- > 0)
    {
        // Tail?
        if (pNode->pNextNode == NULL)
            return (pNode);

        pNode = pNode->pNextNode;
    }

    return (pNode);
}

PLISTNODE List_PrevNode(PLISTNODE pNode, INT nDelta)
{
    ASSERT(nDelta < 0);


    while (nDelta-- < 0)
    {
        // Head?
        if (pNode->pPrevNode == NULL)
            return (pNode);

        pNode = pNode->pPrevNode;
    }

    return (pNode);
}

__inline BOOL List_IsEmpty(PLINKEDLIST pList)
{
    return (pList->pHeadNode == NULL && pList->pTailNode == NULL);
}

__inline BOOL List_RemoveTail(PLINKEDLIST pList)
{
    return (List_DeleteNode(pList, pList->pTailNode));
}

__inline BOOL List_RemoveHead(PLINKEDLIST pList)
{
    return (List_DeleteNode(pList, pList->pHeadNode));
}
