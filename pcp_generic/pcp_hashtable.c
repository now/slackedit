/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_hashtable.c
 * Created    : 07/06/00
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   : A more or less complete rip from glib
 *              
 *              
 *              
 *****************************************************************/

#include "pcp_includes.h"

#include "pcp_hashtable.h"
#include "pcp_math.h"
#include "pcp_mem.h"

/****************************************************************
 * Type Definitions
 ****************************************************************/

#define HASH_TABLE_MIN_SIZE 11
#define HASH_TABLE_MAX_SIZE 13845163

/****************************************************************
 * Function Definitions
 ****************************************************************/

UINT HashTable_DefaultHashFunction(const PVOID pVoid);
static void HashTable_Resize(PHASHTABLE pHashTable);
static PHASHTABLENODE HashTable_CreateNode(PHASHTABLE pHashTable, PVOID pKey, PVOID pData);
static void HashTable_DeleteNode(PHASHTABLE pHashTable, PHASHTABLENODE pNode);
static void HashTable_DeleteNodes(PHASHTABLE pHashTable, PHASHTABLENODE pNode);
static __inline PHASHTABLENODE *HashTable_LookupNode(PHASHTABLE pHashTable, const PVOID pKey);

/****************************************************************
 * Global Variables
 ****************************************************************/

/****************************************************************
 * Function Implementations
 ****************************************************************/

UINT HashTable_HashString(LPTSTR pszString, UINT uSize)
{
    LPTSTR psz = pszString;
    UINT uHash = 0;
    UINT uValue;

    if (pszString == NULL)
        return (-1);

    while (*psz != _T('\0'))
    {
        uHash = (uHash << 4) + ((*psz >= _T('A') && *psz <= _T('Z')) ? (*psz+32) : *psz);

        if ((uValue = uHash & 0xF0000000))
            uHash ^= uValue >> 24;

        uHash &= ~uValue;

        psz++;
    }

    return (uHash %= uSize);
}

UINT HashTable_DefaultHashFunction(const PVOID pVoid)
{
    return ((UINT)pVoid);
}

PHASHTABLE HashTable_Create(HASHFUNC lpfnHashFunc, HASHCOMPAREFUNC lpfnCompareFunc)
{
    PHASHTABLE pHashTable;
    int i;

    pHashTable          = (PHASHTABLE)Mem_Alloc(sizeof(HASHTABLE));
    pHashTable->nSize   = HASH_TABLE_MIN_SIZE;
    pHashTable->nNodes  = 0;
    pHashTable->fFrozen = FALSE;
    pHashTable->lpfnHashFunction    = (lpfnHashFunc != NULL) ? lpfnHashFunc : HashTable_DefaultHashFunction;
    pHashTable->lpfnHashCompareFunc = lpfnCompareFunc;
    pHashTable->ppNodes = (PHASHTABLENODE *)Mem_Alloc(sizeof(PHASHTABLENODE) * pHashTable->nSize);

    // Not necessary (Mem_Alloc zero's memory)
    for (i = 0; i < pHashTable->nSize; i++)
        pHashTable->ppNodes[i] = NULL;

    return (pHashTable);
}

void HashTable_Destroy(PHASHTABLE pHashTable)
{
    int i;
    PHASHTABLENODE pNode;

    ASSERT(pHashTable != NULL);

    for (i = 0; i < pHashTable->nSize; i++)
        HashTable_DeleteNodes(pHashTable, pHashTable->ppNodes[i]);

    for (pNode = pHashTable->pFreeNodesList; pNode != NULL; )
    {
        PHASHTABLENODE pTempNode;
        
        pTempNode = pNode->pNextNode;
        Mem_Free(pNode);
        pNode = pTempNode;
    }

    Mem_Free(pHashTable->ppNodes);
    Mem_Free(pHashTable);
}

static void HashTable_Resize(PHASHTABLE pHashTable)
{
    PHASHTABLENODE *ppNewNodes;
    PHASHTABLENODE pNode;
    PHASHTABLENODE pNextNode;
    float   flNodesPerList;
    UINT    uHashValue;
    int     nNewSize;
    int     i;

    flNodesPerList = (float)pHashTable->nNodes / (float)pHashTable->nSize;

    // Check that we really need to resize
    if ((flNodesPerList > 0.3 || pHashTable->nSize <= HASH_TABLE_MIN_SIZE) &&
        (flNodesPerList < 3.0 || pHashTable->nSize >= HASH_TABLE_MAX_SIZE))
        return;

    nNewSize = Math_Clamp(Math_ClosestPrime(pHashTable->nNodes),
                        HASH_TABLE_MIN_SIZE, HASH_TABLE_MAX_SIZE);

    ppNewNodes = (PHASHTABLENODE *)Mem_Alloc(sizeof(PHASHTABLENODE) * nNewSize);

    for (i = 0; i < pHashTable->nSize; i++)
    {
        for (pNode = pHashTable->ppNodes[i]; pNode != NULL; pNode = pNextNode)
        {
            pNextNode = pNode->pNextNode;

            uHashValue = pHashTable->lpfnHashFunction(pNode->pKey) % nNewSize;

            pNode->pNextNode = ppNewNodes[uHashValue];
            ppNewNodes[uHashValue] = pNode;
        }
    }

    Mem_Free(pHashTable->ppNodes);
    pHashTable->ppNodes = ppNewNodes;
    pHashTable->nSize   = nNewSize;
}

/* Returns the number of elements contained in the hash table. */
int HashTable_GetSize(PHASHTABLE pHashTable)
{
    ASSERT(pHashTable != NULL);

    return (pHashTable->nNodes);
}

void HashTable_Freeze(PHASHTABLE pHashTable)
{
    ASSERT(pHashTable != NULL);

    pHashTable->fFrozen++;
}

void HashTable_Thaw(PHASHTABLE pHashTable)
{
    ASSERT(pHashTable != NULL);

    if (pHashTable->fFrozen)
    {
        if ((--pHashTable->fFrozen) == 0)
            HashTable_Resize(pHashTable);
    }
}

static PHASHTABLENODE HashTable_CreateNode(PHASHTABLE pHashTable, PVOID pKey, PVOID pData)
{
    PHASHTABLENODE pNode;

    if (pHashTable->pFreeNodesList != NULL)
    {
        pNode = pHashTable->pFreeNodesList;
        pHashTable->pFreeNodesList = pHashTable->pFreeNodesList->pNextNode;
    }
    else
    {
        pNode = (PHASHTABLENODE)Mem_Alloc(sizeof(HASHTABLENODE));
    }

    pNode->pKey         = pKey;
    pNode->pData        = pData;
    pNode->pNextNode    = NULL;

    return (pNode);
}

static void HashTable_DeleteNode(PHASHTABLE pHashTable, PHASHTABLENODE pNode)
{
    pNode->pNextNode = pHashTable->pFreeNodesList;
    pHashTable->pFreeNodesList = pNode;
}

static void HashTable_DeleteNodes(PHASHTABLE pHashTable, PHASHTABLENODE pNode)
{
    if (pNode)
    {
        PHASHTABLENODE pTempNode = pNode;

        while (pTempNode->pNextNode != NULL)
            pTempNode = pTempNode->pNextNode;

        pTempNode->pNextNode = pHashTable->pFreeNodesList;
        pHashTable->pFreeNodesList = pNode;
    }
}

void HashTable_Insert(PHASHTABLE pHashTable, PVOID pKey, PVOID pData)
{
    PHASHTABLENODE *ppNode;

    ASSERT(pHashTable != NULL);

    ppNode = HashTable_LookupNode(pHashTable, pKey);

    if (*ppNode != NULL)
    {
        (*ppNode)->pData = pData;
    }
    else
    {
        *ppNode = HashTable_CreateNode(pHashTable, pKey, pData);
        pHashTable->nNodes++;

        if (!pHashTable->fFrozen)
            HashTable_Resize(pHashTable);
    }
}

void HashTable_Remove(PHASHTABLE pHashTable, const PVOID pKey)
{
    PHASHTABLENODE *ppNode;
    PHASHTABLENODE pDest;

    ASSERT(pHashTable != NULL);

    ppNode = HashTable_LookupNode(pHashTable, pKey);

    if (*ppNode != NULL)
    {
        pDest = *ppNode;
        *ppNode = pDest->pNextNode;
        HashTable_DeleteNode(pHashTable, pDest);
        pHashTable->nNodes--;

        if (!pHashTable->fFrozen)
            HashTable_Resize(pHashTable);
    }
}

PVOID HashTable_Lookup(PHASHTABLE pHashTable, const PVOID pKey)
{
    PHASHTABLENODE pNode;

    ASSERT(pHashTable != NULL);

    pNode = *HashTable_LookupNode(pHashTable, pKey);

    return ((pNode != NULL) ? pNode->pData : NULL);
}

BOOL HashTable_LookupEx(PHASHTABLE pHashTable, const PVOID pLookupKey, PVOID *pKey, PVOID *pData)
{
    PHASHTABLENODE pNode;

    ASSERT(pHashTable != NULL);

    pNode = *HashTable_LookupNode(pHashTable, pLookupKey);

    if (pNode != NULL)
    {
        if (pKey != NULL)
            *pKey = pNode->pKey;

        if (pData != NULL)
            *pData = pNode->pData;

        return (TRUE);
    }

    return (FALSE);
}

static __inline PHASHTABLENODE *HashTable_LookupNode(PHASHTABLE pHashTable, const PVOID pKey)
{
    PHASHTABLENODE *ppNode;

    ppNode = &pHashTable->ppNodes[pHashTable->lpfnHashFunction(pKey) % pHashTable->nSize];

    if (pHashTable->lpfnHashCompareFunc != NULL)
    {
        while (*ppNode != NULL && !pHashTable->lpfnHashCompareFunc((*ppNode)->pKey, pKey))
            ppNode = &(*ppNode)->pNextNode;
    }
    else
    {
        while (*ppNode != NULL && (*ppNode)->pKey != pKey)
            ppNode = &(*ppNode)->pNextNode;
    }

    return (ppNode);
}

UINT HashTable_ForeachRemove(PHASHTABLE pHashTable,
                             HASHTABLEREMOVEFUNC lpfnHashTableRemove,
                             PVOID pUserData)
{
    PHASHTABLENODE pNode;
    PHASHTABLENODE pPrev;
    int i;
    int cDeleted = 0;

    ASSERT(pHashTable != NULL);
    ASSERT(lpfnHashTableRemove != NULL);

    for (i = 0; i < pHashTable->nSize; i++)
    {
restart:
        pPrev = NULL;

        for (pNode = pHashTable->ppNodes[i]; pNode != NULL; pPrev = pNode, pNode = pNode->pNextNode)
        {
            if (lpfnHashTableRemove(pNode->pKey, pNode->pData, pUserData))
            {
                cDeleted++;

                pHashTable->ppNodes -= 1;

                if (pPrev != NULL)
                {
                    pPrev->pNextNode = pNode->pNextNode;
                    HashTable_DeleteNode(pHashTable, pNode);
                    pNode = pPrev;
                }
                else
                {
                    pHashTable->ppNodes[i] = pNode->pNextNode;
                    HashTable_DeleteNode(pHashTable, pNode);

                    goto restart;
                }
            }
        }
    }

    if (!pHashTable->fFrozen)
        HashTable_Resize(pHashTable);

    return (cDeleted);
}

void HashTable_Foreach(PHASHTABLE pHashTable,
                       HASHTABLEFOREACHFUNC lpfnForeach, PVOID pUserData)
{
    PHASHTABLENODE pNode;
    int i;

    ASSERT(pHashTable != NULL);
    ASSERT(lpfnForeach != NULL);

    for (i = 0; i < pHashTable->nSize; i++)
    {
        for (pNode = pHashTable->ppNodes[i]; pNode != NULL; pNode = pNode->pNextNode)
            lpfnForeach(pNode->pKey, pNode->pData, pUserData);
    }
}
