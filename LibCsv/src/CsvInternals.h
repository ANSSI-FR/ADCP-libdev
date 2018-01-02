/******************************************************************************\
Author : Lucas. BOUILLOT
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __CSV_INT_H__
#define __CSV_INT_H__

/* --- INCLUDES ------------------------------------------------------------- */
#include "CsvLib.h"

/* --- DEFINES -------------------------------------------------------------- */
//
// 'Options' macros
//
#define CSV_SET_OPTION(csv, opt)        SET_BIT((csv)->options.dwOptionsSet, opt)
#define CSV_UNSET_OPTION(csv, opt)      CLR_BIT((csv)->options.dwOptionsSet, opt)
#define CSV_HAS_OPTION(csv, opt)        HAS_BIT((csv)->options.dwOptionsSet, opt)

//
// Misc
//
#define CSV_EOF(csv)                    ((csv)->file.bEofReached == TRUE)
#define CSV_DEFAULT_SEPARATOR           _T(',')
#define CSV_END_OF_LINE_CHAR            _T('\n')
#define CSV_QUOTE_CHAR                  _T('"')
#define CSV_NO_OPTIONS                  (0)
#define CSV_HANDLE_TABLE_DELTA          (0x10)
#define CSV_BUFFER_SIZE                 (0x400)
#define CSV_HEAP_NAME                   _T("CSVLIB")
#define CSV_CHARS_AVAILABLE(csv)        ((csv)->file.read.llFileSize - (csv)->file.read.llPosition)
#define CSV_CURRENT_POS(csv)            ((LPWSTR)((csv)->file.read.pvFileMappedView) + (csv)->file.read.llPosition)
#define CSV_CONSUME_CHAR(csv)           ((LPWSTR)((csv)->file.read.pvFileMappedView))[(csv)->file.read.llPosition++]
#define CSV_IS_LAST_FIELD(csv)          (BOOL)(((csv)->file.dwCurrentField + 1) % (csv)->header.dwNumberOfFields == 0)

/* --- TYPES ---------------------------------------------------------------- */
typedef enum _CSV_FILE_OPERATION {
    CsvFileOperationRead,
    CsvFileOperationWrite,
    CsvFileOperationAppend
} CSV_FILE_OPERATION, *PCSV_FILE_OPERATION;

typedef struct _CSV_OBJECT {
    struct {
        LPWSTR ptFileName;
        HANDLE hFileHandle;
        CSV_FILE_OPERATION eOperationType;
        DWORD dwCurrentField;
        struct {
            HANDLE hFileMapping;
            LPVOID pvFileMappedView;
            LONGLONG llFileSize;
            LONGLONG llPosition;
            BOOL bEofReached;
        } read;
        //struct {
            // NONE
        //} write;
    } file;

    struct {
        DWORD dwNumberOfFields;
        LPWSTR *pptHeaderValues;
        LONGLONG llPositionAfterHeader;
        DWORD dwNumberOfFilteredFields;
        PDWORD pdwFilteredSetIndexes;
    } header;

    struct {
        DWORD dwNumberOfRecords;
        DWORD dwLastError;
    } infos;

    struct {
        DWORD dwOptionsSet;
        WCHAR tSeparator;
        /*
        struct {
            HANDLE hThread;
            HANDLE hEventWrite;
            HANDLE hEventTerminate;
        } thread;
        struct {
            DWORD dwBufferSize;
            LPWSTR pBuffer;
        } buff;
        */
    } options;
} CSV_OBJECT, *PCSV_OBJECT;

typedef struct _CSV_HANDLE_TABLE {
    DWORD dwTableSize;
    DWORD dwEntryCount;
    PCSV_OBJECT *pEntries;
} CSV_HANDLE_TABLE, *PCSV_HANDLE_TABLE;

/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */

#endif // __CSV_INT_H__
