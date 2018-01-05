/******************************************************************************\
Author : Lucas. BOUILLOT
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __CSV_LIB_H__
#define __CSV_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- INCLUDES ------------------------------------------------------------- */
/* --- DEFINES -------------------------------------------------------------- */
#ifdef VLD_DBG
    #define VLD_FORCE_ENABLE
    #include <vld.h>
    #pragma comment(lib,"vld.lib")
#endif

#undef DLL_FCT
#ifdef DLL_MODE
    #ifdef __CSV_INT_H__
        #define DLL_FCT __declspec(dllexport)
    #else
        #define DLL_FCT __declspec(dllimport)
    #endif
#else
    #define DLL_FCT
#endif

//
// Custom LibCsv error codes
//
#define CSV_LIBERR                      (0xEEEE)
#define CSV_ERR_UNKNOWN_TODO            ERROR_CODE(CSV_LIBERR, 1)
#define CSV_ERROR_END_OF_FILE           ERROR_CODE(CSV_LIBERR, 2)
#define CSV_ERROR_UNKNOW_OPTION         ERROR_CODE(CSV_LIBERR, 3)
#define CSV_ERROR_INVALID_HANDLE        ERROR_CODE(CSV_LIBERR, 4)
#define CSV_ERROR_END_OF_LINE           ERROR_CODE(CSV_LIBERR, 5)
#define CSV_ERROR_MEMORY_OPERATION      ERROR_CODE(CSV_LIBERR, 6)
#define CSV_ERROR_ALREADY_STARTED       ERROR_CODE(CSV_LIBERR, 7)
#define CSV_ERROR_NOT_IMPLEMENTED       ERROR_CODE(CSV_LIBERR, 8)
#define CSV_ERROR_INVALID_OPERATION     ERROR_CODE(CSV_LIBERR, 9)
#define CSV_ERROR_INVALID_PARAMETER     ERROR_CODE(CSV_LIBERR, 10)

#define CSV_ERROR_READ_UNEXPECTED_EOF               ERROR_CODE(CSV_LIBERR, 11)
#define CSV_ERROR_READ_QUOTE_IN_NON_ENQUOTED_FIELD  ERROR_CODE(CSV_LIBERR, 12)
#define CSV_ERROR_READ_INVALID_FIELD_TERMINATOR     ERROR_CODE(CSV_LIBERR, 13)

//
// Custom LibCsv 'Return' macros
//
#define CSV_API_RETURN_SUCCESS(pCsv)            return pCsv->infos.dwLastError = (NO_ERROR), (SUCCESS_VALUE);
#define CSV_API_RETURN_ERROR(pCsv, dwErrorCode) return pCsv->infos.dwLastError = (dwErrorCode), (ERROR_VALUE);
#define CSV_API_SAME_ERROR(pCsv)                (pCsv->infos.dwLastError)
#define CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bValue, dwErr)    MULTI_LINE_MACRO_BEGIN              \
                                                               if (API_FAILED(bValue)) {           \
                                                                CSV_API_RETURN_ERROR(pCsv, dwErr); \
                                                               }                                   \
                                                               MULTI_LINE_MACRO_END
#define CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bValue, dwErr)    MULTI_LINE_MACRO_BEGIN        \
                                                               if (API_FAILED(bValue)) {           \
                                                                return ERROR_VALUE;                \
                                                               }                                   \
                                                               MULTI_LINE_MACRO_END

//
// Misc.
//
#define CSV_INVALID_HANDLE_VALUE    ((CSV_HANDLE)-1)

/* --- TYPES ---------------------------------------------------------------- */
typedef DWORD CSV_HANDLE;
typedef CSV_HANDLE *PCSV_HANDLE;

typedef enum _CSV_OPTION {
    CsvOptionCustomFieldSeparator,
    CsvOptionAlwaysEnquoteFields,
    CsvOptionBufferedOperations,
    CsvOptionThreadedOperations
} CSV_OPTION, *PCSV_OPTION;

typedef enum _CSV_FIELD_TYPE {
    CsvFieldTypeStr = 0,
    CsvFieldTypeInt,
    CsvFieldTypeHexInt,
    CsvFieldTypeBool,
    CsvFieldTypeSid,
    CsvFieldTypeGuid,
} CSV_FIELD_TYPE, *PCSV_FIELD_TYPE;

/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */
//
// Init/Cleanup
//
DLL_FCT
BOOL
CsvLibInit(
);

DLL_FCT
BOOL
CsvLibCleanup(
);

//
// Read operations
//
DLL_FCT BOOL CsvOpenReadW(
    _In_ const LPWSTR ptCsvFilename,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPWSTR *ppptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    );

DLL_FCT BOOL CsvOpenReadA(
    _In_ const LPSTR ptCsvFilename,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPSTR *ppptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    );

#ifdef UNICODE
#define CsvOpenRead  CsvOpenReadW
#else
#define CsvOpenRead  CsvOpenReadA
#endif // !UNICODE

DLL_FCT BOOL CsvOpenReadWithFilteredSetW(
    _In_ const LPWSTR ptCsvFilename,
    _In_ const LPWSTR pptFilteredSet[],
    _In_ const DWORD dwFilteredSetCount,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPWSTR *ppptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    );

DLL_FCT BOOL CsvOpenReadWithFilteredSetA(
    _In_ const LPSTR ptCsvFilename,
    _In_ const LPSTR pptFilteredSet[],
    _In_ const DWORD dwFilteredSetCount,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPSTR *ppptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    );

#ifdef UNICODE
#define CsvOpenReadWithFilteredSet  CsvOpenReadWithFilteredSetW
#else
#define CsvOpenReadWithFilteredSet  CsvOpenReadWithFilteredSetA
#endif // !UNICODE

DLL_FCT BOOL CsvGetNextRecordW(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ LPWSTR *pptCsvRecordValues[],
    _Out_opt_ PDWORD pdwCsvRecordNumber
    );

DLL_FCT BOOL CsvGetNextRecordA(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ LPSTR **pptCsvRecordValues,
    _Out_opt_ PDWORD pdwCsvRecordNumber
    );

#ifdef UNICODE
#define CsvGetNextRecord  CsvGetNextRecordW
#else
#define CsvGetNextRecord  CsvGetNextRecordA
#endif // !UNICODE

//
// Write operations
//
DLL_FCT BOOL CsvOpenWriteW(
    _In_ const LPWSTR ptCsvFilename,
    _In_ const DWORD dwCsvHeaderCount,
    _In_ const LPWSTR pptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    );

DLL_FCT BOOL CsvOpenWriteA(
    _In_ const LPSTR ptCsvFilename,
    _In_ const DWORD dwCsvHeaderCount,
    _In_ const LPSTR pptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    );

#ifdef UNICODE
#define CsvOpenWrite  CsvOpenWriteW
#else
#define CsvOpenWrite  CsvOpenWriteA
#endif // !UNICODE

DLL_FCT BOOL CsvOpenAppendW(
    _In_ const LPWSTR lpcwCsvFilename,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPWSTR *pplpwCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    );

DLL_FCT BOOL CsvOpenAppendA(
    _In_ const LPSTR lpCsvFilename,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPSTR *pplpCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    );

#ifdef UNICODE
#define CsvOpenAppend  CsvOpenAppendW
#else
#define CsvOpenAppend  CsvOpenAppendA
#endif // !UNICODE

DLL_FCT BOOL CsvWriteNextRecordW(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const LPWSTR pptCsvRecordValues[],
    _Out_opt_ PDWORD pdwCsvRecordNumber
    );

DLL_FCT BOOL CsvWriteNextRecordA(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const LPSTR pptCsvRecordValues[],
    _Out_opt_ PDWORD pdwCsvRecordNumber
    );

#ifdef UNICODE
#define CsvWriteNextRecord  CsvWriteNextRecordW
#else
#define CsvWriteNextRecord  CsvWriteNextRecordA
#endif // !UNICODE

DLL_FCT BOOL CsvWriteNextRecordWithTypes(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const DWORD_PTR ppCsvRecordValues[],
    _In_ const CSV_FIELD_TYPE pCsvRecordTypes[],
    _Out_opt_ PDWORD pdwCsvRecordNumber
    );

//
// Both Read/Write operations
//
DLL_FCT BOOL CsvClose(
    _Inout_ PCSV_HANDLE phCsvHandle
    );

DLL_FCT BOOL CsvResetFile(
    _In_ const CSV_HANDLE hCsvHandle
    );

//
// Options
//
DLL_FCT BOOL CsvSetOption(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const CSV_OPTION eCsvOption,
    _In_ const DWORD dwCsvOptionValue
    );

DLL_FCT BOOL CsvUnsetOption(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const CSV_OPTION eCsvOption
    );

//
// Other
//
DLL_FCT DWORD CsvGetLastError(
   _In_ const CSV_HANDLE hCsvHandle
   );

DLL_FCT BOOL CsvGetHeaderNumberOfFields(
   _In_ const CSV_HANDLE hCsvHandle,
   _Inout_ PDWORD pdwHeaderNumberOfFields
   );

DLL_FCT VOID CsvHeapFree(
    _In_ PVOID pMem
   );

DLL_FCT VOID CsvRecordArrayHeapFree(
    _In_ PVOID *ppMemArr,
    _In_ DWORD dwCount
);

#ifdef __cplusplus
}
#endif

#endif // __CSV_LIB_H__
