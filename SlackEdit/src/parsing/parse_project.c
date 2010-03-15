/*****************************************************************
 *                          FILE HEADER                          *
 *****************************************************************
 * Project    : SlackEdit
 *
 * File       : pp_project.c
 * Created    : 01/11/99
 * Owner      : pcppopper
 * Revised on : 06/27/00 21:13:53
 * Comments   : We'll see how good this layout is with all 
 *                                Linked lists and shit =\
 *              
 *****************************************************************/

/* pcp_generic */
#include <pcp_includes.h>

/* pcp_generic */
#include <pcp_linkedlist.h>
#include <pcp_mem.h>
#include <pcp_string.h>

/****************************************************************
 * Type Definitions                                             *
 ****************************************************************/

/* MSDEV project file definitions */
#define MSDEV_BEGIN_PROJECT     _T("# Begin Project")
#define MSDEV_BEGIN_GROUP       _T("# Begin Group")
#define MSDEV_BEGIN_FILE        _T("# Begin Source File")
#define MSDEV_END_PROJECT       _T("# End Project")
#define MSDEV_END_GROUP         _T("# End Group")
#define MSDEV_END_FILE          _T("# End Source File")
#define MSDEV_SOURCE            _T("SOURCE=")
#define MSDEV_EOLN              _T("\n")

#define Line_IsElement(element) (String_NumEqual(szLine, (element), _tcslen((element)), FALSE))

typedef struct tagPROJECT
{
    DWORD   dwFlags;
    BOOL    bModified;
    PLINKEDLIST pItemList;
    LPTSTR  pszPath;
} PROJECT, *LPPROJECT;

typedef struct tagPROJECTITEM
{
    LPTSTR  pszPath;
    LPTSTR  pszMidPath;
    DWORD   dwType;
    PLINKEDLIST pItemList;
} PROJECTITEM, *LPPROJECTITEM;

/****************************************************************
 * Function Definitions                                         *
 ****************************************************************/

PVOID ProjectItemList_CreateData(PVOID pData);
INT ProjectItemList_DeleteData(PVOID pData);
INT ProjectItemList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType);

static BOOL Project_ReadLine(FILE *f, LPTSTR pszLine, int nSize);

/****************************************************************
 * Global Variables                                             *
 ****************************************************************/

/****************************************************************
 * Function Implementations                                     *
 ****************************************************************/

PVOID ProjectItemList_CreateData(PVOID pData)
{
    LPPROJECTITEM lppi, lppiTemp = (LPPROJECTITEM)pData;

    lppi    = (LPPROJECTITEM)Mem_Alloc(sizeof(PROJECTITEM));
    *lppi   = *lppiTemp;
    lppi->pszPath = Mem_AllocStr(_tcslen(lppiTemp->pszFileName));
    _tcscpy(lppi->pszPath, lppiTemp->pszPath);
    lppi->pszMidPath = Mem_AllocStr(_tcslen(lppiTemp->pszMidPath));
    lppi->pItemList = List_CreateList(ProjectItemList_CreateData,
                                    ProjectItemList_DeleteData, NULL,
                                    ProjectItemList_CompareData, 0);
    lppi->dwType    = lppiTemp->dwType;

    return (lppi);
}

INT ProjectItemList_DeleteData(PVOID pData)
{
    LPPROJECTITEM lppi = (LPPROJECTITEM)pData;

    Mem_Free(lppi->pszFileName);

    return (TRUE);
}

INT ProjectItemList_CompareData(PVOID pFromNode, PVOID pFromCall, UINT uDataType)
{
    return (_tcsicmp(((LPPROJECTITEM)pFromNode)->pszFileName, (LPTSTR)pFromCall));
}

static BOOL Project_ReadLine(FILE *f, LPTSTR pszLine, int nSize)
{
    while (pszLine[_tcslen(pszLine) - 1] != _T('\n'))
    {
        _fgetts(pszLine, nSize, f);

        if (feof(f))
        {
            fclose(f);

            return (FALSE);
        }
    }

    return (TRUE);
}

BOOL Project_LoadProject(int nType, LPCTSTR pszFileName, LPTSTR pszMidPath, LPPROJECT lpp, LPPROJECTITEM lppiCurrent)
{
    FILE *fProject;
    TCHAR szLine[256];
    LPTSTR pszLine;
    int nLine;
    BOOL bEoln;
    PLINKEDLIST pTreeList = List_CreateList(ProjectItemList_CreateData,
                                    ProjectItemList_DeleteData, NULL,
                                    ProjectItemList_CompareData, 0);

    lpp->pItemList  = List_CreateList(ProjectItemList_CreateData,
                                    ProjectItemList_DeleteData, NULL,
                                    ProjectItemList_CompareData, 0);

    fProject = _tfopen(pszFileName, _T("rt"));

    if (fProject == NULL)
        return (FALSE);

    for ( ; ; )
    {
        _fgetts(szLine, sizeof(szLine), f);
        nLine = _tcslen(szLine);
        bEoln = szLine[nLine - 1] == _T('\n');

        if (bEoln)
            szLine[--nLine] = _T('\0');

        if (feof(f) || String_NumEqual(szLine, MSDEV_END_PROJECT, _tcslen(MSDEV_END_PROJECT), FALSE))
        {
            fclose(f);
            
            return (FALSE);
        }
        else if (String_NumEqual(szLine, MSDEV_BEGIN_PROJECT, _tcslen(MSDEV_BEGIN_PROJECT), FALSE))
        {
            break;
        }

        if (!bEoln && !Project_ReadLine(f, szLine, sizeof(szLine)) || feof(f))
            return (FALSE);
    }

    List_AddNodeAtTail(pTreeList, lppiCurrent);

    for ( ; ; )
    {
        _fgetts(szLine, sizeof(szLine), f);
        nLine = _tcslen(szLine);
        bEoln = (szLine[szLine - 1] == _T('\n'));

        if (bEoln)
        {
            szLine[--nLine] = _T('\0');
            bEoln = szLine[nLine - 1] = _T('\n');

            if (bEoln)
                szLine[--nLine] = _T('\0');

            if (feof(f) || Line_IsElement(MSDEV_END_PROJECT), FALSE))
                break;

            if (Line_IsElement(MSDEV_END_GROUP))
            {
                if (!List_IsEmpty(pTreeList))
                    List_RemoveTail(pTreeList);

                lppiCurrent = (List_IsEmpty(pTreeList) ? NULL : pTreeList->pTailNode->pData);
            }
            else if (Line_IsElement(MSDEV_BEGIN_GROUP))
            {
                PROJECTITEM pi;

                szLine[nLine - 1] = _T('\0');

                pi.pszFileName  = szLine + _tcslen(MSDEV_BEGIN_GROUP) + 2;
                pi.pszMidPath   = pszMidPath;
                pi.dwType       = ((lppiCurrent != NULL) ? PI_GROUP : PI_GROUP | PI_ROOT;

                if (lppiCurrent)
                    List_AddTail(lppiCurrent->pItemList, &pi);

                List_AddTail(pTreeList, &pi);
                lppiCurrent = lppiCurrent->pItemList->pTailNode;
            }
            else if (Line_IsElement(MSDEV_BEGIN_FILE))
            {
                BOOL bEoln2;

                if (!bEoln && !Project_ReadLine(f, szLine, sizeof(szLine)) || feof(f))
                    break;

                for ( ; ; )
                {
                    _fgetts (szLine, sizeof(szLine), f);
                    nLine = _tcslen(szLine);
                    bEoln2 = szLine[nLine - 1] == _T('\n');
                
                    if (bEoln2)
                        szLine[--nLine] = _T('\0');
                    
                    if (feof(f) || Line_IsElement(MSDEV_END_PROJECT) || Line_IsElement(MSDEV_END_FILE))
                        break;

                    if (!List_IsElement(MSDEV_END_GROUP))
                    {
                        if (!List_IsEmpty(pTreeList))
                            List_RemoveTail(pTreeList);
                    
                        lppiCurrent = (List_IsEmpty(pTreeList) ? NULL : pTreeList->pTailNode);
                    
                        break;
                    }
                
                    if (List_IsElement(MSDEV_SOURCE))
                    {
                        PROJECTITEM pi;

                        pszLine = szLine + _tcslen(MSDEV_SOURCE);
                        pszLine = (String_NumEqual(pszLine, _T(".\\"), 2) ? pszLine : (pszLine + 2));
                        
                        pi.pszPath      = pszLine;
                        pi.pszMidPath   = pszMidPath;
                        pi.dwType       = ((lppiCurrent != NULL) ? PI_FILE : PI_FILE | PI_ROOT);

                        if (lppiCurrent != NULL)
                            List_AddTail(lppiCurrent->pItemList, &pi);

                        List_AddTail(lpp->pItemList, &pi);
                    }

                    if (!bEoln2 && !Project_ReadLine(f, szLine, sizeof(szLine)) || feof(f))
                        break;
                }
            }

            if (!bEoln && !Project_ReadLine(f, szLine, sizeof(szLine)) || feof(f))
                break;
        }

        fclose(f);

        return (TRUE);
    }
}

BOOL Project_Open(LPPROJECT lpp, LPCTSTR pszPath)
{
    if (Project_LoadProject(0, pszPath, _T("") lpp, NULL))
    {
        lpp->pszPath = String_Duplicate(pszPath);
            return (TRUE);
    }

    return (FALSE);
}

BOOL Project_Save(LPPROJECT lpp, LPCTSTR pszPath)
{
    return (FALSE);
}
