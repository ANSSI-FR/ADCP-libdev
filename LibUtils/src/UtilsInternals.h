/******************************************************************************\
Author : Lucas. BOUILLOT
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __UTILS_INT_H__
#define __UTILS_INT_H__

/* --- INCLUDES ------------------------------------------------------------- */
/* --- DEFINES -------------------------------------------------------------- */
#define API_RETURN_SUCCESS()                return gs_dwLibLastError = (NO_ERROR), (SUCCESS_VALUE);
#define API_RETURN_ERROR(dwErrorCode)       return gs_dwLibLastError = (dwErrorCode), (ERROR_VALUE);
#define SAME_ERROR()                        (gs_dwLibLastError)

#define UTILS_HEAP_NAME                               _T("UTILSLIB")
#define FATAL_FRMT(frmt)                            _T("[Heap <%s> fatal error] [%s] ") ## frmt ## _T(": <%u>")
#define UTILS_HEAP_FATAL_IF(cond, heap, frmt, ...)  MULTI_LINE_MACRO_BEGIN                                                              \
                                                        if (cond) {                                                                     \
                                                            if (pHeap->pfnFatalErrorHandler != NULL) {                                  \
                                                                pHeap->pfnFatalErrorHandler();                                          \
                                                            }                                                                           \
                                                            FATAL_ERROR(FATAL_FRMT(frmt), pHeap->ptName, ptCaller, __VA_ARGS__, GLE()); \
                                                        }                                                                               \
                                                    MULTI_LINE_MACRO_END
/* --- TYPES ---------------------------------------------------------------- */
/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */

#endif // __UTILS_INT_H__
