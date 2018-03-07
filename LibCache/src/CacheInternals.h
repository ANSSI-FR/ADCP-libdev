/******************************************************************************\
Author : Lucas. BOUILLOT
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __CACHE_INT_H__
#define __CACHE_INT_H__

/* --- INCLUDES ------------------------------------------------------------- */
#include "CacheLib.h"

/* --- DEFINES -------------------------------------------------------------- */
#define API_RETURN_SUCCESS()                return gs_dwCacheLastError = (NO_ERROR), (SUCCESS_VALUE);
#define API_RETURN_ERROR(dwErrorCode)       return gs_dwCacheLastError = (dwErrorCode), (ERROR_VALUE);
#define SAME_ERROR()                        (gs_dwCacheLastError)

#define NTDLL_MODULE_NAME                   _T("ntdll.dll")
#define AVL_FOREACH(table, ptype, element)  ptype element = NULL;                                           \
                                            for (element = (ptype)gs_pfnRtlEnumerateGenericTableAvl(table, TRUE); \
                                                element != NULL;                                            \
                                                element = (ptype)gs_pfnRtlEnumerateGenericTableAvl(table, FALSE)  \
                                                )

/* --- TYPES ---------------------------------------------------------------- */
/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */

#endif // __CACHE_INT_H__
