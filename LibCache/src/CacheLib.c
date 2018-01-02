/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#define LIB_ERROR_VAL gs_dwCacheLastError
#include "CacheInternals.h"

#ifdef _WIN32
    #include "CacheWpp.h"
#endif

/* --- PRIVATE VARIABLES ---------------------------------------------------- */
static DWORD gs_dwCacheLastError = NO_ERROR;
static BOOL gs_bCacheLibInit = FALSE;
static RTLINITIALIZEGENERICTABLEAVL gs_pfnRtlInitializeGenericTableAvl = NULL;
static RTLINSERTELEMENTGENERICTABLEAVL gs_pfnRtlInsertElementGenericTableAvl = NULL;
static RTLLOOKUPELEMENTGENERICTABLEAVL gs_pfnRtlLookupElementGenericTableAvl = NULL;
static RTLENUMERATEGENERICTABLEAVL gs_pfnRtlEnumerateGenericTableAvl = NULL;
static RTLDELETEGENERICTABLEAVL gs_pfnRtlDeleteElementGenericTableAvl = NULL;

/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
static PVOID _Function_class_(RTL_AVL_ALLOCATE_ROUTINE) NTAPI CacheAvlAllocate(
    _In_ const PRTL_AVL_TABLE pTable,
    _In_ const CLONG lByteSize
    ) {
    return UtilsHeapAllocHelper(((PCACHE)pTable)->pHeap, lByteSize);
}

static VOID _Function_class_(RTL_AVL_FREE_ROUTINE) NTAPI CacheAvlFree(
    _In_ const PRTL_AVL_TABLE pTable,
    _In_ PVOID pvBuffer
    ) {
    UtilsHeapFreeAndNullHelper(((PCACHE)pTable)->pHeap, pvBuffer);
}

/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
BOOL
CacheLibInit (
)
{
   HMODULE hNtdll = NULL;

   if (gs_bCacheLibInit == TRUE) {
      API_RETURN_ERROR(CACHE_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
   }

   hNtdll = GetModuleHandle(NTDLL_MODULE_NAME);
   if (hNtdll == NULL) {
      API_RETURN_ERROR(CACHE_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
   }

   gs_pfnRtlInitializeGenericTableAvl = (RTLINITIALIZEGENERICTABLEAVL)GetProcAddress(hNtdll, "RtlInitializeGenericTableAvl");
   gs_pfnRtlInsertElementGenericTableAvl = (RTLINSERTELEMENTGENERICTABLEAVL)GetProcAddress(hNtdll, "RtlInsertElementGenericTableAvl");
   gs_pfnRtlLookupElementGenericTableAvl = (RTLLOOKUPELEMENTGENERICTABLEAVL)GetProcAddress(hNtdll, "RtlLookupElementGenericTableAvl");
   gs_pfnRtlEnumerateGenericTableAvl = (RTLENUMERATEGENERICTABLEAVL)GetProcAddress(hNtdll, "RtlEnumerateGenericTableAvl");
   gs_pfnRtlDeleteElementGenericTableAvl = (RTLDELETEGENERICTABLEAVL)GetProcAddress(hNtdll, "RtlDeleteElementGenericTableAvl");

   if (gs_pfnRtlInitializeGenericTableAvl == NULL
      || gs_pfnRtlInsertElementGenericTableAvl == NULL
      || gs_pfnRtlLookupElementGenericTableAvl == NULL
      || gs_pfnRtlEnumerateGenericTableAvl == NULL
      || gs_pfnRtlDeleteElementGenericTableAvl == NULL) {
      API_RETURN_ERROR(CACHE_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
   }

   gs_bCacheLibInit = TRUE;
   API_RETURN_SUCCESS();
}

BOOL
CacheLibCleanup (
)
{
   if (gs_bCacheLibInit == TRUE) {
      API_RETURN_ERROR(CACHE_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
   }

   gs_pfnRtlInitializeGenericTableAvl = NULL;
   gs_pfnRtlInsertElementGenericTableAvl = NULL;
   gs_pfnRtlLookupElementGenericTableAvl = NULL;
   gs_pfnRtlEnumerateGenericTableAvl = NULL;
   gs_pfnRtlDeleteElementGenericTableAvl = NULL;

   gs_bCacheLibInit = FALSE;
   API_RETURN_SUCCESS();
}

BOOL CacheCreate(
    _Out_ PCACHE *ppCache,
    _In_ const PTCHAR ptName,
    _In_ const PRTL_AVL_COMPARE_ROUTINE pfnCompare
    ) {
    BOOL bResult = FALSE;
    PUTILS_HEAP pHeap = NULL;

    (*ppCache) = NULL;

    bResult = UtilsHeapCreate(&pHeap, ptName, NULL);    // TODO!: callback
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());  // TODO: WPP

    (*ppCache) = UtilsHeapAllocStructHelper(pHeap, CACHE);
    (*ppCache)->pHeap = pHeap;
    (*ppCache)->ptName = UtilsHeapStrDupHelper(pHeap, ptName);

    gs_pfnRtlInitializeGenericTableAvl(&(*ppCache)->sAvlTable, pfnCompare, CacheAvlAllocate, CacheAvlFree, NULL);

    API_RETURN_SUCCESS();
}

BOOL CacheDestroy(
    _Inout_ PCACHE *ppCache,
    _In_opt_ const PFN_CACHE_ENTRY_DESTROY_CALLBACK pfnEntryDestroy,
    _In_opt_ const DWORD dwEntrySize
    ) {
    BOOL bResult = FALSE;
    PUTILS_HEAP pHeap = (*ppCache)->pHeap;
    PVOID pvCopiedEntry = NULL;

    AVL_FOREACH(&(*ppCache)->sAvlTable, PVOID, pvEntry) {
        if (pfnEntryDestroy != NULL) {
            pvCopiedEntry = UtilsHeapMemDupHelper((*ppCache)->pHeap, pvEntry, dwEntrySize);
        }
        bResult = gs_pfnRtlDeleteElementGenericTableAvl(&(*ppCache)->sAvlTable, pvEntry);
        API_RETURN_ERROR_IF_FAILED(bResult, CACHE_ERR_UNKNOWN_TODO); // TODO: WPP + leak
        pvEntry = NULL;
        if (pfnEntryDestroy != NULL) {
            pfnEntryDestroy((*ppCache), pvCopiedEntry);
            UtilsHeapFreeAndNullHelper((*ppCache)->pHeap, pvCopiedEntry);
        }
    }

    (*ppCache)->pHeap = NULL;
    UtilsHeapFreeAndNullHelper(pHeap, (*ppCache)->ptName);
    UtilsHeapFreeAndNullHelper(pHeap, (*ppCache));

    bResult = UtilsHeapDestroy(&pHeap);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP

    API_RETURN_SUCCESS();
}

BOOL CacheEntryInsert(
    _In_ const PCACHE pCache,
    _In_ const PVOID pvBuffer,
    _In_ const DWORD dwSize,
    _Out_opt_ PVOID *ppvInserted,
    _Out_opt_ PBOOL pbNewElement
    ) {
    PVOID pvInserted = NULL;
    BOOLEAN bNewElement = FALSE;

    SET_PTRVAL_IF_NOT_NULL(ppvInserted, NULL);
    SET_PTRVAL_IF_NOT_NULL(pbNewElement, FALSE);

    pvInserted = gs_pfnRtlInsertElementGenericTableAvl(&pCache->sAvlTable, pvBuffer, (CLONG)dwSize, &bNewElement);
    if (pvInserted == NULL) {
        API_RETURN_ERROR(CACHE_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
    }

    SET_PTRVAL_IF_NOT_NULL(ppvInserted, pvInserted);
    SET_PTRVAL_IF_NOT_NULL(pbNewElement, (BOOL)bNewElement);

    API_RETURN_SUCCESS();
}

BOOL CacheEntryLookup(
    _In_ const PCACHE pCache,
    _In_ const PVOID pvSearched,
    _Out_opt_ PVOID *ppvFound
    ) {
    PVOID pvFound = NULL;

    SET_PTRVAL_IF_NOT_NULL(ppvFound, NULL);

    pvFound = gs_pfnRtlLookupElementGenericTableAvl(&pCache->sAvlTable, pvSearched);
    if (pvFound == NULL) {
        API_RETURN_ERROR(CACHE_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
    }

    SET_PTRVAL_IF_NOT_NULL(ppvFound, pvFound);

    API_RETURN_SUCCESS();
}

BOOL CacheEntryRemove(
    _In_ const PCACHE pCache,
    _In_ const PVOID pvBuffer
    ) {
    BOOL bResult = FALSE;

    bResult = gs_pfnRtlDeleteElementGenericTableAvl(&pCache->sAvlTable, pvBuffer);
    API_RETURN_ERROR_IF_FAILED(bResult, CACHE_ERR_UNKNOWN_TODO); // TODO: WPP

    API_RETURN_SUCCESS();
}

DWORD CacheGetLastError(
    ) {
    return LIB_ERROR_VAL;
}

RTL_GENERIC_COMPARE_RESULTS CacheCompareNums(
    _In_ const int iFirst,
    _In_ const int iSecond
    ) {
    if (iFirst < iSecond)
        return GenericLessThan;

    if (iFirst > iSecond)
        return GenericGreaterThan;

    return GenericEqual;
}

RTL_GENERIC_COMPARE_RESULTS CacheCompareBinStruct(
    _In_ const PVOID pvFirst,
    _In_ const PVOID pvSecond,
    _In_ const DWORD dwLen
    ) {
    return CacheCompareNums(memcmp(pvFirst, pvSecond, dwLen), 0);
}

RTL_GENERIC_COMPARE_RESULTS CacheCompareStr(
    _In_ const PTCHAR ptFirst,
    _In_ const PTCHAR ptSecond
    ) {
    return CacheCompareNums(_tcsicmp(ptFirst, ptSecond), 0);
}

#ifdef DLL_MODE
BOOL WINAPI DllMain(
    _In_  HINSTANCE hinstDLL,
    _In_  DWORD fdwReason,
    _In_  LPVOID lpvReserved
    ) {
    BOOL bResult = FALSE;

    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: bResult = CacheLibInit(); break;
    case DLL_PROCESS_DETACH: bResult = CacheLibCleanup(); break;
    default: bResult = TRUE; break;
    }

    return bResult;
}
#endif