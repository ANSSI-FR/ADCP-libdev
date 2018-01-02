/******************************************************************************\
Author : Lucas. BOUILLOT
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __CACHE_LIB_H__
#define __CACHE_LIB_H__

/* --- INCLUDES ------------------------------------------------------------- */
#include <Windows.h>
#include <tchar.h>
#include "CacheAvlTable.h"
#include "..\..\LibUtils\src\UtilsLib.h"

#ifdef VLD_DBG
    #define VLD_FORCE_ENABLE
    #include <vld.h>
    #pragma comment(lib,"vld.lib")
#endif

#undef DLL_FCT
#ifdef DLL_MODE
    #ifdef __CACHE_INT_H__
        #define DLL_FCT __declspec(dllexport)
    #else
        #define DLL_FCT __declspec(dllimport)
    #endif
#else
    #define DLL_FCT
#endif

/* --- DEFINES -------------------------------------------------------------- */
#define CACHE_LIBERR                (0xDDDD)
#define CACHE_ERR_UNKNOWN_TODO      ERROR_CODE(CACHE_LIBERR, 1)

/* --- TYPES ---------------------------------------------------------------- */
typedef struct _CACHE {
    RTL_AVL_TABLE sAvlTable; // _MUST_ be the first member of this struct, so can be casted as CACHE
    PTCHAR ptName;
    PUTILS_HEAP pHeap;
} CACHE, *PCACHE;

typedef void (FN_CACHE_ENTRY_DESTROY_CALLBACK)(
    _In_opt_ const PCACHE pCache,
    _In_ const PVOID pvEntry
    );
typedef FN_CACHE_ENTRY_DESTROY_CALLBACK *PFN_CACHE_ENTRY_DESTROY_CALLBACK;

/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */
//
// Init/Cleanup
//
DLL_FCT
BOOL
CacheLibInit(
);

DLL_FCT
BOOL
CacheLibCleanup(
);

DLL_FCT BOOL CacheCreate(
    _Out_ PCACHE *ppCache,
    _In_ const PTCHAR ptName,
    _In_ const PRTL_AVL_COMPARE_ROUTINE pfnCompare
    );

DLL_FCT BOOL CacheDestroy(
    _Inout_ PCACHE *ppCache,
    _In_opt_ const PFN_CACHE_ENTRY_DESTROY_CALLBACK pfnEntryDestroy,
    _In_opt_ const DWORD dwEntrySize
    );

DLL_FCT BOOL CacheEntryInsert(
    _In_ const PCACHE pCache,
    _In_ const PVOID pvBuffer,
    _In_ const DWORD dwSize,
    _Out_opt_ PVOID *ppvInserted,
    _Out_opt_ PBOOL pbNewElement
    );

DLL_FCT BOOL CacheEntryLookup(
    _In_ const PCACHE pCache,
    _In_ const PVOID pvSearched,
    _Out_opt_ PVOID *ppvFound
    );

DLL_FCT BOOL CacheEntryRemove(
    _In_ const PCACHE pCache,
    _In_ const PVOID pvBuffer
    );

DLL_FCT DWORD CacheGetLastError(
    );

DLL_FCT RTL_GENERIC_COMPARE_RESULTS CacheCompareNums(
    _In_ const int iFirst,
    _In_ const int iSecond
    );

DLL_FCT RTL_GENERIC_COMPARE_RESULTS CacheCompareBinStruct(
    _In_ const PVOID pvFirst,
    _In_ const PVOID pvSecond,
    _In_ const DWORD dwLen
    );

DLL_FCT RTL_GENERIC_COMPARE_RESULTS CacheCompareStr(
    _In_ const PTCHAR ptFirst,
    _In_ const PTCHAR ptSecond
    );

#endif // __CACHE_LIB_H__
