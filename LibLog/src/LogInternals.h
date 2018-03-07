/******************************************************************************\
Author : Lucas. BOUILLOT
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __LOG_INT_H__
#define __LOG_INT_H__

/* --- INCLUDES ------------------------------------------------------------- */
/* --- DEFINES -------------------------------------------------------------- */
#define API_RETURN_SUCCESS()                return gs_dwLogLastError = (NO_ERROR), (SUCCESS_VALUE);
#define API_RETURN_ERROR(dwErrorCode)       return gs_dwLogLastError = (dwErrorCode), (ERROR_VALUE);
#define SAME_ERROR()                        (gs_dwLogLastError)

#define LOG_HEAP_NAME                       _T("LOGLIB")
#define LOG_TIME_FRMT                       _T("[%02u:%02u:%02u] %s")
#define LOG_TIME_LEN                        (1 + 2 + 1 + 2 + 1 + 2 + 1 + 1)

/* --- TYPES ---------------------------------------------------------------- */
/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */

#endif // __LOG_INT_H__
