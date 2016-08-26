/* --- INCLUDES ------------------------------------------------------------- */
#include "..\..\src\CsvLib.h"

/* --- DEFINES -------------------------------------------------------------- */
#define TEST_OUTFILE                    _T("test.libcsv.")
#define TEST_SUCCESS()                  return TRUE;
#define TEST_ERROR(frmt, ...)           MULTI_LINE_MACRO_BEGIN                                      \
                                            _tprintf(_T("\t") ## frmt ## _T("\r\n"), __VA_ARGS__);  \
                                            return FALSE;                                           \
                                        MULTI_LINE_MACRO_END

#define TEST_ERROR_IF(cond, frmt, ...)  MULTI_LINE_MACRO_BEGIN                  \
                                            if (cond) {                         \
                                                TEST_ERROR(frmt, __VA_ARGS__);  \
                                            }                                   \
                                        MULTI_LINE_MACRO_END

/* --- TYPES ---------------------------------------------------------------- */
/* --- PRIVATE VARIABLES ---------------------------------------------------- */
static PUTILS_HEAP gs_pHeap = NULL;

/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */

/* 
Tests of LibCsv :
    
[+] Tests which must always SUCCEED:

    => Tests on field number:
        - Test to write/read csv files with 1 field only
        - Test to write/read csv files with a random number (>1) of fields

    => Tests on fields:
        - Test to write/read random csv fields
        - Test to write/read csv fields containing combinaisons of:
            - separator (with a randomly chosen separator)
            - double quotes
            - line feed
        - Test to write/read csv fields containing special chars (sep, quotes, LF) at the following positions:
            - first char
            - last char
        - Test to write/read csv empty fields
        - Test to write/read csv fields 1 char only
        - Test to write/read csv fields of very long length

    => Tests of options:
        - Test to write/read always enquoted fields (option)
        - Test to write/read fields with separators : , \t ; :
        - Test to write/read fields with a random separator
        - both

[+] Tests which must always FAIL:

    => Tests on fields:

    => Tests of options:

[+] Tests which can fail or not:

    => TODO

*/

void TestCompareRecords(
    ) {

}


BOOL Test(
    _In_ const DWORD dwNumberOfFields,
    _In_ const PTCHAR pptHeader[],
    _In_ const DWORD dwNumberOfRecords,
    _In_ const BOOL bAlwaysEnquoted,
    _In_ const TCHAR tSeparator
    ) {
    BOOL bResult = FALSE;
    CSV_HANDLE hRead = CSV_INVALID_HANDLE_VALUE;
    CSV_HANDLE hWrite = CSV_INVALID_HANDLE_VALUE;
    DWORD i = 0;
    PTCHAR **ppptRecords = NULL;
    DWORD dwRecordNumber = 0;

    //
    // WRITE FILE
    //
    bResult = CsvOpenWrite(TEST_OUTFILE, dwNumberOfFields, pptHeader, &hWrite);
    TEST_ERROR_IF(API_FAILED(bResult), _T("CsvOpenWrite: <err%#08x>"), CsvGetLastError());

    if (bAlwaysEnquoted == TRUE) {
        bResult = CsvSetOption(hWrite, CsvOptionAlwaysEnquoteFields, 0);
        TEST_ERROR_IF(API_FAILED(bResult), _T("CsvOptionAlwaysEnquoteFields: <err%#08x>"), CsvGetLastError());
    }

    if (tSeparator != NULL_CHAR) {
        bResult = CsvSetOption(hWrite, CsvOptionCustomFieldSeparator, (DWORD)tSeparator);
        TEST_ERROR_IF(API_FAILED(bResult), _T("CsvOptionCustomFieldSeparator: <sep:%c> <err%#08x>"), tSeparator, CsvGetLastError());
    }

    for (i = 0; i < dwNumberOfRecords; i++) {
        // TODO : generate record
        bResult = CsvWriteNextRecord(hWrite, ppptRecords[i], &dwRecordNumber);
        TEST_ERROR_IF(API_FAILED(bResult), _T("CsvWriteNextRecord: <err%#08x>"), CsvGetLastError());
        TEST_ERROR_IF(dwRecordNumber != (i + 1), _T("CsvWriteNextRecord: dwRecordNumber != (i+1): <dwRecordNumber:%u> <(i+1):%u>"), dwRecordNumber, (i + 1), DumpRecord(dwNumberOfFields, ppptRecords[i]));
    }

    bResult = CsvClose(&hWrite);
    TEST_ERROR_IF(API_FAILED(bResult), _T("CsvClose hWrite: <err%#08x>"), CsvGetLastError());

    //
    // READ FILE
    //
    DWORD dwReadHeaderCount = 0;
    PTCHAR *pptReadHeader = 0;
    PTCHAR *pptReadRecord = UtilsHeapAllocArrayHelper(gs_pHeap, PTCHAR, dwNumberOfFields);

    bResult = CsvOpenRead(TEST_OUTFILE, &dwReadHeaderCount, &pptReadHeader, &hRead);
    TEST_ERROR_IF(API_FAILED(bResult), _T("CsvOpenRead: <err%#08x>"), CsvGetLastError());
    TEST_ERROR_IF(dwNumberOfFields != dwReadHeaderCount, _T("CsvOpenRead: dwNumberOfFields != dwReadHeaderCount: <dwNumberOfFields:%u> <dwReadHeaderCount:%u>"), dwNumberOfFields, dwReadHeaderCount, DumpRecord(dwNumberOfFields, pptHeader[i]), DumpRecord(dwReadHeaderCount, pptReadHeader[i]));
    bResult = TestCompareRecords(dwReadHeaderCount, pptReadHeader, pptHeader);
    TEST_ERROR_IF(API_FAILED(bResult), _T("TestCompareRecords: Header"));

    while (API_SUCCEEDED(CsvGetNextRecord(hRead, pptReadRecord, &dwRecordNumber))) {

    }

    DWORD dwLastErr = CsvGetLastError();
    if (dwLastErr != CSV_ERROR_END_OF_FILE) {
        TEST_ERROR(_T("CsvGetNextRecord: <err:%#08x>"), dwLastErr);
    }

    TEST_ERROR_IF(dwRecordNumber != dwNumberOfRecords, _T("CsvGetNextRecord: dwRecordNumber != dwNumberOfRecords: <dwRecordNumber:%u> <dwNumberOfRecords:%u>"), dwRecordNumber, dwNumberOfRecords);

    bResult = CsvClose(&hRead);
    TEST_ERROR_IF(API_FAILED(bResult), _T("CsvClose hRead: <err%#08x>"), CsvGetLastError());

    TEST_SUCCESS();
}


void main(
    ) {

}
