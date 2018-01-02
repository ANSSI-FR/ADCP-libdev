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
