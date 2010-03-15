/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : pcp_generic
 *
 * File       : pcp_hashtable.h
 * Created    : 07/06/00
 * Owner      : pcppopper
 * Revised on : 07/06/00
 * Comments   : 
 *              
 *              
 ****************************************************************/

#ifndef __PCP_HASHTABLE_H
#define __PCP_HASHTABLE_H

#include "pcp_generic.h"

typedef UINT (*HASHFUNC)(const PVOID pKey);
typedef BOOL (*HASHTABLEREMOVEFUNC)(PVOID pKey, PVOID pData, PVOID pUserData);
typedef void (*HASHTABLEFOREACHFUNC)(PVOID pKey, PVOID pData, PVOID pUserData);
typedef int (*HASHCOMPAREFUNC)(const PVOID pOne, const PVOID pTwo);

typedef struct tagHASHTABLENODE
{
	LPVOID pKey;
	LPVOID pData;
	struct tagHASHTABLENODE *pNextNode;
} HASHTABLENODE, *PHASHTABLENODE;

typedef struct tagHASHTABLE
{
	int				nSize;
	int				nNodes;
	UINT			fFrozen;
	PHASHTABLENODE	pFreeNodesList;
	PHASHTABLENODE *ppNodes;
	HASHFUNC		lpfnHashFunction;
	HASHCOMPAREFUNC	lpfnHashCompareFunc;
} HASHTABLE, *PHASHTABLE;

FOREXPORT UINT HashTable_HashString(LPTSTR pszString, UINT uSize);
FOREXPORT PHASHTABLE HashTable_Create(HASHFUNC lpfnHashFunc, HASHCOMPAREFUNC lpfnCompareFunc);
FOREXPORT void HashTable_Destroy(PHASHTABLE pHashTable);
FOREXPORT int HashTable_GetSize(PHASHTABLE pHashTable);
FOREXPORT void HashTable_Freeze(PHASHTABLE pHashTable);
FOREXPORT void HashTable_Thaw(PHASHTABLE pHashTable);
FOREXPORT void HashTable_Insert(PHASHTABLE pHashTable, PVOID pKey, PVOID pData);
FOREXPORT void HashTable_Remove(PHASHTABLE pHashTable, const PVOID pKey);
FOREXPORT PVOID HashTable_Lookup(PHASHTABLE pHashTable, const PVOID pKey);
FOREXPORT BOOL HashTable_LookupEx(PHASHTABLE pHashTable, const PVOID pLookupKey, PVOID *pKey, PVOID *pData);
FOREXPORT UINT HashTable_ForeachRemove(PHASHTABLE pHashTable,
							 HASHTABLEREMOVEFUNC lpfnHashTableRemove,
							 PVOID pUserData);
FOREXPORT void HashTable_Foreach(PHASHTABLE pHashTable,
					   HASHTABLEFOREACHFUNC lpfnForeach, PVOID pUserData);

#endif /* __PCP_HASHTABLE_H */
