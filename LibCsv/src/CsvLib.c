/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#include "CsvInternals.h"
#include "CsvLib.h"

#ifdef _WIN32
    #include "CsvWpp.h"
    #include "CsvLib.tmh"
#endif

/* --- PRIVATE VARIABLES ---------------------------------------------------- */
static PUTILS_HEAP gs_pCsvHeap = NULL;
static CSV_HANDLE_TABLE gs_sCsvHandleTable = { .dwTableSize = 0, .dwEntryCount = 0, .pEntries = NULL };
static HANDLE gs_hCsvHandletableMutex = INVALID_HANDLE_VALUE;


/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
static void CsviWppInit(
    ) {
    WPP_INIT_TRACING(CSV_WPP_APP_NAME);
}

static void CsviWppClean(
    ) {
    WPP_CLEANUP();
}

static BOOL CsviLibInit(
    ) {
    BOOL bResult = FALSE;

    CsviWppInit();

    bResult = UtilsHeapCreate(&gs_pCsvHeap, CSV_HEAP_NAME, NULL);
    if (API_FAILED(bResult)) {
       return ERROR_VALUE;
    }

    gs_hCsvHandletableMutex = CreateMutex(NULL, FALSE, NULL);
    if (gs_hCsvHandletableMutex == NULL)
    {
       return ERROR_VALUE;
    }

    return SUCCESS_VALUE;
}


static BOOL CsviLibCleanup(
    ) {
    BOOL bResult = FALSE, bReturnValue = SUCCESS_VALUE;
    DWORD i = 0, dwWaitResult = 0;
    CSV_HANDLE hCsv = CSV_INVALID_HANDLE_VALUE;

    // Lock the CsvHandleTable
    dwWaitResult = WaitForSingleObject(gs_hCsvHandletableMutex, INFINITE);
    if (dwWaitResult != WAIT_OBJECT_0) {
       bReturnValue = ERROR_VALUE;
       goto exit;
    }

    if (gs_sCsvHandleTable.dwEntryCount > 0) {
        for (i = 0; i < gs_sCsvHandleTable.dwTableSize; i++) {
            if (gs_sCsvHandleTable.pEntries[i] != NULL) {
                hCsv = (CSV_HANDLE)i;
                CsvClose(&hCsv);
            }
        }
    }
    UtilsHeapFreeAndNullHelper(gs_pCsvHeap, gs_sCsvHandleTable.pEntries);

    bResult = UtilsHeapDestroy(&gs_pCsvHeap);
    if (API_FAILED(bResult)) {
       bReturnValue = ERROR_VALUE;
       goto exit;
    }

    CsviWppClean();

 exit:
    // Release the global mutex
    ReleaseMutex(gs_hCsvHandletableMutex);
    return bReturnValue;
}


static BOOL CsviGetObjectByHandle(
    _In_ const CSV_HANDLE hHandle,
    _Out_ PCSV_OBJECT *ppObject
    ) {
   DWORD dwWaitResult = 0;
   BOOL bReturnValue = SUCCESS_VALUE;

   // Lock the CsvHandleTable
   dwWaitResult = WaitForSingleObject(gs_hCsvHandletableMutex, INFINITE);
   if (dwWaitResult != WAIT_OBJECT_0) {
      bReturnValue = ERROR_VALUE;
      goto exit;
   }
   
   (*ppObject) = NULL;

    if (hHandle != CSV_INVALID_HANDLE_VALUE && hHandle <= gs_sCsvHandleTable.dwTableSize && gs_sCsvHandleTable.pEntries[hHandle] != NULL) {
        (*ppObject) = gs_sCsvHandleTable.pEntries[hHandle];
        goto exit;
    }

    bReturnValue = ERROR_VALUE;
exit :
    // Release the global mutex
    ReleaseMutex(gs_hCsvHandletableMutex);
    if (bReturnValue == SUCCESS_VALUE)
       CSV_API_RETURN_SUCCESS((*ppObject));
    return bReturnValue;
}


static BOOL CsviExtendHandleTable(
    ) {
    PCSV_OBJECT *pEntriesNew = NULL;
    DWORD dwTableSizeNew = 0;

    // CsvHandleTable should be lock when using this method

    dwTableSizeNew = gs_sCsvHandleTable.dwTableSize + CSV_HANDLE_TABLE_DELTA;
    pEntriesNew = UtilsHeapAllocOrReallocHelper(gs_pCsvHeap, gs_sCsvHandleTable.pEntries, SIZEOF_ARRAY(PCSV_OBJECT, dwTableSizeNew));

    gs_sCsvHandleTable.pEntries = pEntriesNew;
    gs_sCsvHandleTable.dwTableSize = dwTableSizeNew;

    return SUCCESS_VALUE;
}


static BOOL CsviNewObject(
    _Out_ PCSV_OBJECT *ppObject,
    _Out_ PCSV_HANDLE pCsvHandle
    ) {
    BOOL bResult = ERROR_VALUE, bReturnValue = SUCCESS_VALUE;
    DWORD i = 0, dwWaitResult = 0;

    // Lock the CsvHandleTable
    dwWaitResult = WaitForSingleObject(gs_hCsvHandletableMutex, INFINITE);
    if (dwWaitResult != WAIT_OBJECT_0) {
       bReturnValue = ERROR_VALUE;
       goto exit;
    }

    (*pCsvHandle) = 0;
    (*ppObject) = NULL;

    if (gs_sCsvHandleTable.dwEntryCount == gs_sCsvHandleTable.dwTableSize) {
        bResult = CsviExtendHandleTable();
        if (API_FAILED(bResult)) {
           bReturnValue = ERROR_VALUE; // TODO: WPP
           goto exit;
        }
    }

    for (i = 0; i < gs_sCsvHandleTable.dwTableSize && gs_sCsvHandleTable.pEntries[i] != NULL; i++) {};

    gs_sCsvHandleTable.pEntries[i] = UtilsHeapAllocStructHelper(gs_pCsvHeap, CSV_OBJECT);
    gs_sCsvHandleTable.dwEntryCount += 1;

    (*pCsvHandle) = i;
    (*ppObject) = gs_sCsvHandleTable.pEntries[i];

 exit:
    // Release the global mutex
    ReleaseMutex(gs_hCsvHandletableMutex);
    if (bReturnValue == SUCCESS_VALUE)
      CSV_API_RETURN_SUCCESS((*ppObject))
    else
       return bReturnValue;
}


static BOOL CsviOpenFile(
    _In_ const LPWSTR ptFilename,
    _Out_ PCSV_OBJECT *ppObject,
    _Out_ PCSV_HANDLE pHandle,
    _In_ const CSV_FILE_OPERATION eFileOperation
    ) {
    BOOL bResult = ERROR_VALUE;
    HANDLE hFileHandle = INVALID_HANDLE_VALUE;
    DWORD dwDesiredAccess = (eFileOperation == CsvFileOperationWrite ? GENERIC_WRITE : (eFileOperation == CsvFileOperationAppend ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ));
    DWORD dwCreationDisposition = (eFileOperation == CsvFileOperationWrite ? CREATE_ALWAYS : OPEN_EXISTING);

    (*pHandle) = 0;
    (*ppObject) = NULL;

    hFileHandle = CreateFileW(ptFilename, dwDesiredAccess, FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFileHandle == INVALID_HANDLE_VALUE) {
        DWORD dwErr = GetLastError();
        UNREFERENCED_PARAMETER(dwErr);
        API_RETURN_SAME_ERROR(); // TODO: WPP
    }

    bResult = CsviNewObject(ppObject, pHandle);
    if (API_FAILED(bResult)) {
       return ERROR_VALUE;  // TODO: WPP
    }

    (*ppObject)->file.ptFileName = UtilsHeapStrDupHelper(gs_pCsvHeap, ptFilename);
    (*ppObject)->file.eOperationType = eFileOperation;
    (*ppObject)->file.hFileHandle = hFileHandle;
    (*ppObject)->file.dwCurrentField = 0;
    (*ppObject)->file.read.bEofReached = FALSE;
    (*ppObject)->file.read.pvFileMappedView = NULL;
    (*ppObject)->file.read.hFileMapping = NULL;

    (*ppObject)->header.dwNumberOfFields = 0;
    (*ppObject)->header.pptHeaderValues = NULL;

    (*ppObject)->infos.dwNumberOfRecords = 0;
    (*ppObject)->infos.dwLastError = NO_ERROR;

    (*ppObject)->options.dwOptionsSet = CSV_NO_OPTIONS;
    (*ppObject)->options.tSeparator = CSV_DEFAULT_SEPARATOR;

    /*
    (*ppObject)->options.buff.dwBufferSize = 0;
    (*ppObject)->options.buff.pBuffer = NULL;
    (*ppObject)->options.thread.hThread = NULL;
    (*ppObject)->options.thread.hEventWrite = NULL;
    (*ppObject)->options.thread.hEventTerminate = NULL;
    */

    CSV_API_RETURN_SUCCESS((*ppObject));
}


static BOOL CsviCreateMapping(
    _In_ const PCSV_OBJECT pCsv
    ) {
    BOOL bResult = FALSE;
    LARGE_INTEGER liFileSize = { 0 };
	DWORD flProtect = (pCsv->file.eOperationType == CsvFileOperationWrite ? PAGE_READWRITE : pCsv->file.eOperationType == CsvFileOperationAppend ? PAGE_READWRITE : PAGE_READONLY);

    bResult = GetFileSizeEx(pCsv->file.hFileHandle, &liFileSize);
    CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, GLE());
	//works with unicode input. Always works with csv files produced by the lib
	//TODO: throw if ANSI is detected or extend api with CsvOpenReadEx to double llFileSize if input is ANSI
    pCsv->file.read.llFileSize = liFileSize.QuadPart/2; 

    // TODO : it would be cleaner here to map a small portion of the file, and then map the next parts when needed, than map the entire file...

    pCsv->file.read.hFileMapping = CreateFileMapping(pCsv->file.hFileHandle, NULL, flProtect, 0, 0, NULL);
    if (pCsv->file.read.hFileMapping == NULL) {
       CSV_API_RETURN_ERROR(pCsv, GLE()); // TODO: ERR+WPP 
    }

    pCsv->file.read.pvFileMappedView = MapViewOfFile(pCsv->file.read.hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (pCsv->file.read.pvFileMappedView == NULL) {
       CSV_API_RETURN_ERROR(pCsv, GLE()); // TODO: ERR+WPP 
    }

    CSV_API_RETURN_SUCCESS(pCsv);
}

static void CsviCopyFieldPart(
    _Inout_opt_ LPWSTR *pptOutBuff,
    _In_opt_ const LPWSTR ptFieldPartStart,
    _In_ const LONGLONG llFieldNewLen,
    _In_ const LONGLONG llFieldOldLen
    ) {
    // When reading a filtered set, some fields are read, but not actually copied (pptOutBuff == NULL)
    if (pptOutBuff != NULL) {
        if (llFieldNewLen > 0 && ptFieldPartStart != NULL) {
            (*pptOutBuff) = UtilsHeapAllocOrReallocHelper(gs_pCsvHeap, (*pptOutBuff), SIZEOF_TSTR((DWORD)llFieldNewLen));
            CopyMemory((*pptOutBuff) + llFieldOldLen, ptFieldPartStart, (SIZE_T)(llFieldNewLen - llFieldOldLen)*sizeof(WCHAR)); // TODO: concat
            (*pptOutBuff)[llFieldNewLen] = NULL_CHAR;
        }
        else {
            (*pptOutBuff) = UtilsHeapStrDupHelper(gs_pCsvHeap, EMPTY_STR);
        }
    }
}

static BOOL CsviReadField(
    _In_ const PCSV_OBJECT pCsv,
    _Out_opt_ LPWSTR *pptBuffer,
    _In_ const BOOL bHeaderKnown
    ) {
    BOOL bFieldEnquoted = FALSE;
    LONGLONG llRemaining = 0;
    LONGLONG llFieldLen = 0;
    LONGLONG llFieldNewLen = 0;
    WCHAR tCurrentChar = 0;
    LPWSTR ptFieldStart = NULL;
    LONGLONG i = 0;

    SET_PTRVAL_IF_NOT_NULL(pptBuffer, NULL);

    // TODO: problem of using LONGLONG when all API use DWORD (could be a pb if a 4GB+ field exists...)

    llRemaining = CSV_CHARS_AVAILABLE(pCsv);

    if (pCsv->file.read.bEofReached == TRUE) {
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_END_OF_FILE); // TODO: ERR+WPP
    }

    if (llRemaining <= 0) {
        pCsv->file.read.bEofReached = TRUE;
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_END_OF_FILE); // TODO: ERR+WPP
    }

    if ((ULONGLONG)llRemaining < sizeof(WCHAR)) {
        pCsv->file.read.llPosition = pCsv->file.read.llFileSize;
        pCsv->file.read.bEofReached = TRUE;
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_READ_UNEXPECTED_EOF); // TODO: ERR+WPP
    }

    //
    // Read first char, 4 possibilities :
    //  - first char is a newline       => return END OF LINE
    //  - first char is a separator     => current field is empty
    //  - first char is a quote         => the current field is enquoted
    //  - first char is anything else   => the current field is not enquoted (we must not encounter : separator (comma by default), LF (\n) or quote (") characters)
    //
    tCurrentChar = CSV_CONSUME_CHAR(pCsv);
    
    switch (tCurrentChar) {
    case CSV_END_OF_LINE_CHAR: 
        if (bHeaderKnown == TRUE) {
            if (CSV_IS_LAST_FIELD(pCsv) == FALSE) {
                CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_READ_INVALID_FIELD_TERMINATOR); // TODO: ERR+WPP
            }
            else {
                llFieldLen = 0;
                goto LABEL_READ_FIELD_COPY;
            }
        }
        else {
            CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_END_OF_LINE);
        }
        break;

    case CSV_QUOTE_CHAR:
        bFieldEnquoted = TRUE;
        break;

    default:
        if (tCurrentChar == pCsv->options.tSeparator) {
            if (bHeaderKnown == TRUE && CSV_IS_LAST_FIELD(pCsv) == TRUE){
                CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_READ_INVALID_FIELD_TERMINATOR); // TODO: ERR+WPP
            }
            else {
                llFieldLen = 0;
                goto LABEL_READ_FIELD_COPY;
            }
        }
        else {
            bFieldEnquoted = FALSE;
        }
        break;
    }

    // 
    // The field is not empty (otherwise the first char would have been a LF or a separator)
    // So there must remain at least 1 char (the separator or newline)
    //
    llRemaining = CSV_CHARS_AVAILABLE(pCsv);

    if ((ULONGLONG)llRemaining < sizeof(WCHAR)) {
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_READ_UNEXPECTED_EOF); // TODO: ERR+WPP
    }

    //
    // Now seek the end of the field to compute the field size
    // For non-enquoted fields:
    //      - separator or LF chars             => end of field (ERROR with LF if this is not the last field, and inversely with sep.)
    //      - quote char                        => forbidden in a non-enquoted field: ERROR
    //      - end of the file (remaining=0)     => end of the file, last record not terminated with a LF: ERROR
    //      - other chars                       => still in the field
    // For enquoted fields:
    //      - quote char                        => escaped quote or end of field
    //      - end of the file (remaining=0)     => end of the file, last record not properly enquoted & not terminated with a LF: ERROR
    //      - other chars (including sep & LF)  => still in the field
    //
    
    // if the field is not enquoted, we already consumed 1 char of it
    ptFieldStart = CSV_CURRENT_POS(pCsv) - (bFieldEnquoted ? 0 : 1);

    for (i = 0; i < llRemaining; i++) {
        tCurrentChar = CSV_CONSUME_CHAR(pCsv);
        
        if (bFieldEnquoted == FALSE) {
            // quotes are forbidden in non-enquoted fields
            if (tCurrentChar == CSV_QUOTE_CHAR) {
                CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_READ_QUOTE_IN_NON_ENQUOTED_FIELD); // TODO: ERR+WPP
            }

            // end of the field
            if ((tCurrentChar == pCsv->options.tSeparator || tCurrentChar == CSV_END_OF_LINE_CHAR)) {
                // Last field must be terminated with a CSV_END_OF_LINE_CHAR, others by a separator
                if ((bHeaderKnown == TRUE) && (
                    (tCurrentChar == pCsv->options.tSeparator && CSV_IS_LAST_FIELD(pCsv) == TRUE) ||
                    (tCurrentChar == CSV_END_OF_LINE_CHAR && CSV_IS_LAST_FIELD(pCsv) == FALSE)
                    )) {
                    CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_READ_INVALID_FIELD_TERMINATOR); // TODO: ERR+WPP
                }

                llFieldLen = CSV_CURRENT_POS(pCsv) - ptFieldStart - 1; // Do not copy the ending quote and the LF
                goto LABEL_READ_FIELD_COPY;
            }
        }

        if (bFieldEnquoted == TRUE && (tCurrentChar == CSV_QUOTE_CHAR)) {
            //
            // The only next we can encounter is :
            //      - another quote ("")    => escaped quote
            //      - a separator (",)      => ending quote for other-than-last fields
            //      - a LF ("\n)            => ending quote for last field
            // Other chars, or end of the file are considered as errors
            //

            if ((i + 1) >= llRemaining) {
                // reached end of the file with nothing after a quote: ERROR
                // TODO: do we tolerate the last record without LF ? (currently no)
                break;
            }
            else
            {
                // Here we must copy the field part by part, without the surrounding quotes
                // Also, we must copy only one quote if we encounter an escaped quote

                tCurrentChar = CSV_CONSUME_CHAR(pCsv);

                switch (tCurrentChar) {
                    // Escaped quote ("")
                    case CSV_QUOTE_CHAR:
                        llFieldNewLen = llFieldLen + CSV_CURRENT_POS(pCsv) - ptFieldStart - 1; // Copy only une quote
                        CsviCopyFieldPart(pptBuffer, ptFieldStart, llFieldNewLen, llFieldLen);

                        llFieldLen = llFieldNewLen;
                        ptFieldStart = CSV_CURRENT_POS(pCsv);
                        break;

                    // Ending quote for last field ("\n)
                    case CSV_END_OF_LINE_CHAR:
                        if (bHeaderKnown == FALSE || CSV_IS_LAST_FIELD(pCsv) == TRUE) {
                            llFieldNewLen = llFieldLen + CSV_CURRENT_POS(pCsv) - ptFieldStart - 2; // Do not copy the ending quote and the LF
                            CsviCopyFieldPart(pptBuffer, ptFieldStart, llFieldNewLen, llFieldLen);
                            goto LABEL_READ_FIELD_SUCCESS;
                        }
                        else {
                            CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_READ_INVALID_FIELD_TERMINATOR); // TODO: ERR+WPP
                        }
                        break;

                    default:
                        // Ending quote for other-thant-last fields (",)
                        if (tCurrentChar == pCsv->options.tSeparator) {
                            llFieldNewLen = llFieldLen + CSV_CURRENT_POS(pCsv) - ptFieldStart - 2; // Do not copy the ending quote and the separator
                            CsviCopyFieldPart(pptBuffer, ptFieldStart, llFieldNewLen, llFieldLen);
                            goto LABEL_READ_FIELD_SUCCESS;
                        }
                        else {
                            CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_READ_INVALID_FIELD_TERMINATOR); // TODO: ERR+WPP
                        }
                        break;
                }
            }
        }
    }

    // If we reached this point, we reached the end of the file without reading successfully the current field: ERROR
    CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_READ_UNEXPECTED_EOF); // TODO: ERR+WPP


LABEL_READ_FIELD_COPY:
    CsviCopyFieldPart(pptBuffer, ptFieldStart, llFieldLen, 0);

LABEL_READ_FIELD_SUCCESS:
    pCsv->file.dwCurrentField += 1;

    if (bHeaderKnown == FALSE && tCurrentChar == CSV_END_OF_LINE_CHAR) {
        // We are reading the header here, without knowing how many fields it has
        // So generate a EOL 'error' to let the caller know that the first line has ended
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_END_OF_LINE); 
    }
    else {
        CSV_API_RETURN_SUCCESS(pCsv);
    }
}


static BOOL CsviReadHeader(
    _In_ const PCSV_OBJECT pCsv
    ) {
    BOOL bResult = ERROR_VALUE;

    pCsv->header.pptHeaderValues = NULL;
    pCsv->header.dwNumberOfFields = 0;

    do {
        pCsv->header.dwNumberOfFields += 1;
        pCsv->header.pptHeaderValues = UtilsHeapAllocOrReallocHelper(gs_pCsvHeap, pCsv->header.pptHeaderValues, SIZEOF_ARRAY(LPWSTR, pCsv->header.dwNumberOfFields));
        bResult = CsviReadField(pCsv, &pCsv->header.pptHeaderValues[pCsv->header.dwNumberOfFields - 1], FALSE);
    } while (API_SUCCEEDED(bResult));
    
    if (pCsv->infos.dwLastError == CSV_ERROR_END_OF_LINE) {
        CSV_API_RETURN_SUCCESS(pCsv);
    }
    else {
        API_RETURN_SAME_ERROR(); // TODO: WPP
    }
}


static BOOL CsviWriteChar(
    _In_ const PCSV_OBJECT pCsv,
    _In_ const WCHAR tChar
    ) {
    BOOL bResult = FALSE;
    DWORD dwWritten = 0;

    bResult = WriteFile(pCsv->file.hFileHandle, &tChar, sizeof(WCHAR), &dwWritten, NULL);
    if (bResult == FALSE || dwWritten != sizeof(WCHAR)) {
        CSV_API_RETURN_ERROR(pCsv, GLE());
    }

    CSV_API_RETURN_SUCCESS(pCsv);
}

static BOOL CsviWriteField(
    _In_ const PCSV_OBJECT pCsv,
    _In_ const LPWSTR pField
    ) {
    BOOL bResult = FALSE;
    BOOL bEnquoted = FALSE;
    BOOL bEscaped = FALSE;
    DWORD dwLen = 0;
    DWORD dwWritten = 0;
    LPWSTR ptTmpBuff = NULL;
    LPWSTR ptQuote = NULL;


    //
    // Field will be:
    //      - enquoted if it contains: separator (comma by default), LF (\n) or quote (") characters
    //      - escaped if it contains quote (") characters
    //
    dwLen = (DWORD)_tcslen(pField);
    bEscaped = (_tcschr(pField, CSV_QUOTE_CHAR) != NULL);
    bEnquoted = bEscaped == TRUE || CSV_HAS_OPTION(pCsv, CsvOptionAlwaysEnquoteFields) == TRUE || (_tcschr(pField, CSV_END_OF_LINE_CHAR) != NULL) || (_tcschr(pField, pCsv->options.tSeparator) != NULL);

    // Write the opening quote if needed
    if (bEnquoted == TRUE) {
        bResult = CsviWriteChar(pCsv, CSV_QUOTE_CHAR);
        CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP
    }

    //
    // If the field doesnt need escaping, we can write it in one shot
    // Otherwise we write it part-by-part, and double each quote to escape them
    //
    if (bEscaped == FALSE) {
        bResult = WriteFile(pCsv->file.hFileHandle, pField, sizeof(WCHAR)*dwLen, &dwWritten, NULL);
        if (bResult == FALSE || dwWritten != sizeof(WCHAR)*dwLen) {
           CSV_API_RETURN_ERROR(pCsv, GLE());
        }
    }
    else {
        ptTmpBuff = pField;
        while ((ptQuote = _tcschr(ptTmpBuff, CSV_QUOTE_CHAR)) != NULL) {
            // Write the current field part, plus the first quote
            // The len to write is the len of the current "pre-quote" part, plus the quote
            dwLen = (DWORD)(ptQuote - ptTmpBuff) + 1;
            bResult = WriteFile(pCsv->file.hFileHandle, ptTmpBuff, sizeof(WCHAR)*dwLen, &dwWritten, NULL);
            if (bResult == FALSE || dwWritten != sizeof(WCHAR)*dwLen) {
               CSV_API_RETURN_ERROR(pCsv, GLE());
            }

            // Write the second quote
            bResult = CsviWriteChar(pCsv, CSV_QUOTE_CHAR);
            CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

            // Scan the next part of the field
            ptTmpBuff = ptQuote + 1;
        }
		// Finally write the remaining part of the field
		dwLen = (DWORD)_tcslen(ptTmpBuff);
		bResult = WriteFile(pCsv->file.hFileHandle, ptTmpBuff, sizeof(WCHAR)*dwLen, &dwWritten, NULL);
		if (bResult == FALSE || dwWritten != sizeof(WCHAR)*dwLen) {
			CSV_API_RETURN_ERROR(pCsv, GLE());
		}
    }

    // Write the closing quote if needed
    if (bEnquoted == TRUE) {
        bResult = CsviWriteChar(pCsv, CSV_QUOTE_CHAR);
        CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP
    }

    //
    // Write the field closing character: a LF if dwNumberOfFields has been reached or a field separator otherwise
    //
    bResult = CsviWriteChar(pCsv, (CSV_IS_LAST_FIELD(pCsv) == TRUE ? CSV_END_OF_LINE_CHAR : pCsv->options.tSeparator));
    CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    pCsv->file.dwCurrentField += 1;
    CSV_API_RETURN_SUCCESS(pCsv);
}


static BOOL CsviReleaseObject(
    _In_ const CSV_HANDLE hHandle
    ) {
    PCSV_OBJECT pCsv = NULL;
    DWORD i = 0, dwWaitResult = 0;
    BOOL bReturnValue = SUCCESS_VALUE;

    // Lock the CsvHandleTable
    dwWaitResult = WaitForSingleObject(gs_hCsvHandletableMutex, INFINITE);
    if (dwWaitResult != WAIT_OBJECT_0) {
       bReturnValue =  ERROR_VALUE;
       goto exit;
    }

    pCsv = gs_sCsvHandleTable.pEntries[hHandle];

    if (CSV_HAS_OPTION(pCsv, CsvOptionThreadedOperations)) {
        // TODO : signal & wait for thread
    }

    if (CSV_HAS_OPTION(pCsv, CsvOptionBufferedOperations)) {
        // TODO : flush and free buffer
    }

    if (pCsv->header.pptHeaderValues != NULL) {
        for (i = 0; i < pCsv->header.dwNumberOfFields; i++) {
            UtilsHeapFreeAndNullHelper(gs_pCsvHeap, pCsv->header.pptHeaderValues[i]);
        }
        UtilsHeapFreeAndNullHelper(gs_pCsvHeap, pCsv->header.pptHeaderValues);
    }
    
    UtilsHeapFreeAndNullHelper(gs_pCsvHeap, pCsv->file.ptFileName);
    UtilsHeapFreeAndNullHelper(gs_pCsvHeap, gs_sCsvHandleTable.pEntries[hHandle]);
    
    gs_sCsvHandleTable.dwEntryCount -= 1;

    // Release the global mutex
    ReleaseMutex(gs_hCsvHandletableMutex);

 exit:
    // Release the global mutex
    ReleaseMutex(gs_hCsvHandletableMutex);
    return bReturnValue;
}


static BOOL CsviOptionSetFieldSeparator(
    _In_ const PCSV_OBJECT pCsv,
    _In_ const WCHAR tFieldSeparator
    ) {
    if (tFieldSeparator == CSV_QUOTE_CHAR || tFieldSeparator == CSV_END_OF_LINE_CHAR) {
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_INVALID_PARAMETER);
    }
    pCsv->options.tSeparator = tFieldSeparator;
    CSV_API_RETURN_SUCCESS(pCsv);
}


/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
BOOL CsvOpenReadW(
    _In_ const LPWSTR ptCsvFilename,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPWSTR *ppptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    ) {
    BOOL bResult = ERROR_VALUE;
    PCSV_OBJECT pCsv = NULL;
    DWORD i = 0;
    DWORD dwErr = 0;

    (*pCsvHandle) = CSV_INVALID_HANDLE_VALUE;
    SET_PTRVAL_IF_NOT_NULL(pdwCsvHeaderCount, 0);
    SET_PTRVAL_IF_NOT_NULL(ppptCsvHeaderValues, NULL);

    bResult = CsviOpenFile(ptCsvFilename, &pCsv, pCsvHandle, CsvFileOperationRead);
    CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    bResult = CsviCreateMapping(pCsv);
    CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    bResult = CsviReadHeader(pCsv);
    if (API_FAILED(bResult)) {
        dwErr = pCsv->infos.dwLastError;
        CsvClose(pCsvHandle);
        CSV_API_RETURN_ERROR(pCsv, dwErr); // TODO: WPP
    }
    pCsv->header.llPositionAfterHeader = pCsv->file.read.llPosition;

    SET_PTRVAL_IF_NOT_NULL(pdwCsvHeaderCount, pCsv->header.dwNumberOfFields);
    if (ppptCsvHeaderValues != NULL) {
        //TODO!: caller cannot free memory allocated in LibCsv => CsvFree ?
        (*ppptCsvHeaderValues) = UtilsHeapAllocArrayHelper(gs_pCsvHeap, LPWSTR, pCsv->header.dwNumberOfFields);
        for (i = 0; i < pCsv->header.dwNumberOfFields; i++) {
            (*ppptCsvHeaderValues)[i] = UtilsHeapStrDupHelper(gs_pCsvHeap, pCsv->header.pptHeaderValues[i]);
        }
    }

    CSV_API_RETURN_SUCCESS(pCsv);
}

BOOL CsvOpenReadA(
    _In_ const LPSTR ptCsvFilename,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPSTR *ppptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    ) {
    BOOL bReturn = FALSE;
    LPWSTR lpwCsvFilename = NULL;
    LPWSTR *ppwCsvHeaderValues = NULL;

    UtilsHeapAllocWStrAndConvertAStr(gs_pCsvHeap, ptCsvFilename, &lpwCsvFilename);

    bReturn = CsvOpenReadW(lpwCsvFilename, pdwCsvHeaderCount, &ppwCsvHeaderValues, pCsvHandle);

    if (ppwCsvHeaderValues && pdwCsvHeaderCount && *pdwCsvHeaderCount) UtilsHeapAllocAStrArrAndConvertWStrArr(gs_pCsvHeap, ppwCsvHeaderValues, *pdwCsvHeaderCount, ppptCsvHeaderValues);
    UtilsHeapFreeAndNullHelper(gs_pCsvHeap, lpwCsvFilename);
    return bReturn;
}

BOOL CsvOpenReadWithFilteredSetW(
    _In_ const LPWSTR ptCsvFilename,
    _In_ const LPWSTR pptFilteredSet[],
    _In_ const DWORD dwFilteredSetCount,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPWSTR *ppptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    ) {
    BOOL bResult = FALSE;
    DWORD dwHeaderCount = 0;
    LPWSTR *pptCsvHeaderValues = NULL;
    PCSV_OBJECT pCsv = NULL;
    DWORD i = 0;
    DWORD j = 0;

    // Init output parameters
    (*pCsvHandle) = CSV_INVALID_HANDLE_VALUE;
    SET_PTRVAL_IF_NOT_NULL(pdwCsvHeaderCount, 0);
    SET_PTRVAL_IF_NOT_NULL(ppptCsvHeaderValues, NULL);

    // Verify that the filtered set is not empty
    if (dwFilteredSetCount == 0) {
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_INVALID_PARAMETER); // TODO: WPP
    }

    // Verify that the filtered set does not contain the same field multiple times
    for (i = 0; i < dwFilteredSetCount; i++) {
        for (j = i + 1; j < dwFilteredSetCount; j++) {
            if (STR_EQ(pptFilteredSet[i], pptFilteredSet[j])) {
               CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_INVALID_PARAMETER); // TODO: WPP
            }
        }
    }

    // Open the file & get the csv object
    bResult = CsvOpenRead(ptCsvFilename, &dwHeaderCount, &pptCsvHeaderValues, pCsvHandle);
    CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    bResult = CsviGetObjectByHandle((*pCsvHandle), &pCsv);
    CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP
    
    // Get the filtered set indexes
    pCsv->header.dwNumberOfFilteredFields = dwFilteredSetCount;
    pCsv->header.pdwFilteredSetIndexes = UtilsHeapAllocArrayHelper(gs_pCsvHeap, DWORD, dwFilteredSetCount);
    for (i = 0; i < dwFilteredSetCount; i++) {
        if (IsInSetOfStrings(pptFilteredSet[i], pptCsvHeaderValues, dwHeaderCount, &(pCsv->header.pdwFilteredSetIndexes[i])) == FALSE) {
            for (j = 0; j < dwHeaderCount; j++) {
                UtilsHeapFreeAndNullHelper(gs_pCsvHeap, pptCsvHeaderValues[j]);
            }
            UtilsHeapFreeAndNullHelper(gs_pCsvHeap, pptCsvHeaderValues);
            UtilsHeapFreeAndNullHelper(gs_pCsvHeap, pCsv->header.pdwFilteredSetIndexes);
            CsvClose(pCsvHandle);
            CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_INVALID_PARAMETER); // TODO: WPP
        }
    }

    // Write output parameters
    SET_PTRVAL_IF_NOT_NULL(pdwCsvHeaderCount, dwHeaderCount);
    SET_PTRVAL_IF_NOT_NULL(ppptCsvHeaderValues, pptCsvHeaderValues);

    CSV_API_RETURN_SUCCESS(pCsv);
}

BOOL CsvOpenReadWithFilteredSetA(
    _In_ const LPSTR ptCsvFilename,
    _In_ const LPSTR pptFilteredSet[],
    _In_ const DWORD dwFilteredSetCount,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPSTR *ppptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwCsvFilename = NULL;
    LPWSTR *ppwFilteredSet = NULL;
    LPWSTR *ppwCsvHeaderValues = NULL;

    UtilsHeapAllocWStrAndConvertAStr(gs_pCsvHeap, ptCsvFilename, &lpwCsvFilename);
    UtilsHeapAllocWStrArrAndConvertAStrArr(gs_pCsvHeap, pptFilteredSet, dwFilteredSetCount, &ppwFilteredSet);

    bReturn = CsvOpenReadWithFilteredSetW(lpwCsvFilename, ppwFilteredSet, dwFilteredSetCount, pdwCsvHeaderCount, &ppwCsvHeaderValues, pCsvHandle);

    if (ppwCsvHeaderValues && pdwCsvHeaderCount) UtilsHeapAllocAStrArrAndConvertWStrArr(gs_pCsvHeap, ppwCsvHeaderValues, *pdwCsvHeaderCount, ppptCsvHeaderValues);
    UtilsHeapFreeAndNullHelper(gs_pCsvHeap, lpwCsvFilename);
    return bReturn;
}

BOOL CsvGetNextRecordW(
    _In_ const CSV_HANDLE hCsvHandle,
    _Out_ LPWSTR **pptCsvRecordValues,
    _Out_opt_ PDWORD pdwCsvRecordNumber
    ) {
    BOOL bResult = ERROR_VALUE;
    PCSV_OBJECT pCsv = NULL;
    LPWSTR *pptFieldOutBuff = NULL;
    DWORD i = 0;
    DWORD j = 0;

    // Init output parameters
    SET_PTRVAL_IF_NOT_NULL(pdwCsvRecordNumber, 0);

    // Get the csv object
    bResult = CsviGetObjectByHandle(hCsvHandle, &pCsv);
    CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    // Only valid for csv files opened for reading
    if (pCsv->file.eOperationType != CsvFileOperationRead) {
       CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_INVALID_OPERATION); // TODO: WPP
    }

	// How many field pointers do we need.
	// Caller has no idea and should not be expected to allocate the returned set pointers array
	// Caller is expected to free it with CsvFree
	if (pCsv->header.dwNumberOfFields) *pptCsvRecordValues = UtilsHeapAllocArrayHelper(gs_pCsvHeap, LPWSTR, pCsv->header.dwNumberOfFields);

    // Read the next record
    // If we are in a filtered set, read but do not copy the unwanted fields
    if (pCsv->header.pdwFilteredSetIndexes == NULL) {
        for (i = 0; i < pCsv->header.dwNumberOfFields; i++) {
            bResult = CsviReadField(pCsv, &((*pptCsvRecordValues)[i]), TRUE);
            CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP //TODO: free already read fields on error
        }
    } else {
        for (i = 0; i < pCsv->header.dwNumberOfFields; i++) {
            pptFieldOutBuff = NULL;
            for (j = 0; j < pCsv->header.dwNumberOfFilteredFields; j++) {
                if (pCsv->header.pdwFilteredSetIndexes[j] == i) {
                    pptFieldOutBuff = pptCsvRecordValues[j];
                    break;
                }
            }
            bResult = CsviReadField(pCsv, pptFieldOutBuff, TRUE);
            CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP //TODO: free already read fields on error
        }
    }

    pCsv->infos.dwNumberOfRecords += 1;

    // Write output parameters
    SET_PTRVAL_IF_NOT_NULL(pdwCsvRecordNumber, pCsv->infos.dwNumberOfRecords);

    CSV_API_RETURN_SUCCESS(pCsv);
}

BOOL CsvGetNextRecordA(
    _In_ const CSV_HANDLE hCsvHandle,
    _Out_ LPSTR **pptCsvRecordValues,
    _Out_opt_ PDWORD pdwCsvRecordNumber
    ) {
    BOOL bReturn = FALSE;
	LPWSTR *ppwCsvRecordValues;
	BOOL bResult = FALSE;
	PCSV_OBJECT pCsv = NULL;

	bResult = CsviGetObjectByHandle(hCsvHandle, &pCsv);
	CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

	// How many field pointers do we need.
	// Caller has no idea and should not be expected to allocate the returned set pointers array
	// Caller is expected to free it with CsvFree
    if (pCsv->header.dwNumberOfFields) *pptCsvRecordValues = UtilsHeapAllocArrayHelper(gs_pCsvHeap, LPWSTR, pCsv->header.dwNumberOfFields);

    bReturn = CsvGetNextRecordW(hCsvHandle, &ppwCsvRecordValues, pdwCsvRecordNumber);

	for (DWORD i = 0; i < pCsv->header.dwNumberOfFields; i++) {
		UtilsHeapAllocAStrAndConvertWStr(gs_pCsvHeap, ppwCsvRecordValues[i], &((*pptCsvRecordValues)[i]));
    }

    if (pCsv->header.dwNumberOfFields) UtilsHeapFreeArrayHelper(gs_pCsvHeap, ppwCsvRecordValues, pCsv->header.dwNumberOfFields);
    return bReturn;
}

BOOL CsvOpenWriteW(
    _In_ const LPWSTR ptCsvFilename,
    _In_ const DWORD dwCsvHeaderCount,
    _In_ const LPWSTR pptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    ) {
    BOOL bResult = ERROR_VALUE;
    PCSV_OBJECT pCsv = NULL;
    DWORD i = 0;

    (*pCsvHandle) = CSV_INVALID_HANDLE_VALUE;

    bResult = CsviOpenFile(ptCsvFilename, &pCsv, pCsvHandle, CsvFileOperationWrite);
    CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    pCsv->header.dwNumberOfFields = dwCsvHeaderCount;
    pCsv->header.pptHeaderValues = UtilsHeapAllocArrayHelper(gs_pCsvHeap, LPWSTR, pCsv->header.dwNumberOfFields);

    for (i = 0; i < pCsv->header.dwNumberOfFields; i++) {
        pCsv->header.pptHeaderValues[i] = UtilsHeapStrDupHelper(gs_pCsvHeap, pptCsvHeaderValues[i]);
        bResult = CsviWriteField(pCsv, pCsv->header.pptHeaderValues[i]);
        CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP
    }

    CSV_API_RETURN_SUCCESS(pCsv);
}

BOOL CsvOpenWriteA(
    _In_ const LPSTR ptCsvFilename,
    _In_ const DWORD dwCsvHeaderCount,
    _In_ const LPSTR pptCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    ) {
    BOOL bReturn = FALSE;
    LPWSTR lpwCsvFilename = NULL;
    LPWSTR *ppwCsvHeaderValues = NULL;

    UtilsHeapAllocWStrAndConvertAStr(gs_pCsvHeap, ptCsvFilename, &lpwCsvFilename);
    UtilsHeapAllocWStrArrAndConvertAStrArr(gs_pCsvHeap, pptCsvHeaderValues, dwCsvHeaderCount, &ppwCsvHeaderValues);

    bReturn = CsvOpenWriteW(lpwCsvFilename, dwCsvHeaderCount, ppwCsvHeaderValues, pCsvHandle);

    UtilsHeapFreeArrayHelper(gs_pCsvHeap, ppwCsvHeaderValues, dwCsvHeaderCount);
    UtilsHeapFreeAndNullHelper(gs_pCsvHeap, lpwCsvFilename);
    return bReturn;
}

BOOL CsvOpenAppendW(
    _In_ const LPWSTR lpcwCsvFilename,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPWSTR *pplpwCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    ) {
    DWORD dwErr = 0;
    BOOL bResult = ERROR_VALUE;
    PCSV_OBJECT pCsv = NULL;
    DWORD i = 0;
    LARGE_INTEGER liDistanceToMove = { 0 };

    (*pCsvHandle) = CSV_INVALID_HANDLE_VALUE;

    bResult = CsviOpenFile(lpcwCsvFilename, &pCsv, pCsvHandle, CsvFileOperationAppend);
    CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    // Mapping needed due to CsviReadHeader call 
    bResult = CsviCreateMapping(pCsv);
    CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    bResult = CsviReadHeader(pCsv);
    if (API_FAILED(bResult)) {
        dwErr = pCsv->infos.dwLastError;
        CsvClose(pCsvHandle);
        CSV_API_RETURN_ERROR(pCsv, dwErr); // TODO: WPP
    }
    pCsv->header.llPositionAfterHeader = pCsv->file.read.llPosition;

    SET_PTRVAL_IF_NOT_NULL(pdwCsvHeaderCount, pCsv->header.dwNumberOfFields);
    if (pplpwCsvHeaderValues != NULL) {
        //TODO!: caller cannot free memory allocated in LibCsv => CsvFree ?
        (*pplpwCsvHeaderValues) = UtilsHeapAllocArrayHelper(gs_pCsvHeap, LPWSTR, pCsv->header.dwNumberOfFields);
        for (i = 0; i < pCsv->header.dwNumberOfFields; i++) {
            (*pplpwCsvHeaderValues)[i] = UtilsHeapStrDupHelper(gs_pCsvHeap, pCsv->header.pptHeaderValues[i]);
        }
    }

    // Set the file pointer to the end of the file to prepare append
    bResult = SetFilePointerEx(pCsv->file.hFileHandle, liDistanceToMove, NULL, FILE_END);
    if (bResult == 0) {
        API_RETURN_SAME_ERROR(); // TODO: WPP
    }

    CSV_API_RETURN_SUCCESS(pCsv);
}

BOOL CsvOpenAppendA(
    _In_ const LPSTR lpCsvFilename,
    _Out_opt_ PDWORD pdwCsvHeaderCount,
    _Out_opt_ LPSTR *pplpCsvHeaderValues[],
    _Out_ PCSV_HANDLE pCsvHandle
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwCsvFilename = NULL;
    LPWSTR *ppwCsvHeaderValues = NULL;

    UtilsHeapAllocWStrAndConvertAStr(gs_pCsvHeap, lpCsvFilename, &lpwCsvFilename);

    bReturn = CsvOpenAppendW(lpwCsvFilename, pdwCsvHeaderCount, &ppwCsvHeaderValues, pCsvHandle);

    if (ppwCsvHeaderValues && pdwCsvHeaderCount && *pdwCsvHeaderCount) UtilsHeapAllocAStrArrAndConvertWStrArr(gs_pCsvHeap, ppwCsvHeaderValues, *pdwCsvHeaderCount, pplpCsvHeaderValues);
    UtilsHeapFreeAndNullHelper(gs_pCsvHeap, lpwCsvFilename);
    return bReturn;
}

BOOL CsvWriteNextRecordW(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const LPWSTR pptCsvRecordValues[],
    _Out_opt_ PDWORD pdwCsvRecordNumber
    ) {
    BOOL bResult = ERROR_VALUE;
    PCSV_OBJECT pCsv = NULL;
    DWORD i = 0;

    SET_PTRVAL_IF_NOT_NULL(pdwCsvRecordNumber, 0);

    bResult = CsviGetObjectByHandle(hCsvHandle, &pCsv);
    CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    if ((pCsv->file.eOperationType != CsvFileOperationWrite)
        && (pCsv->file.eOperationType != CsvFileOperationAppend)) {
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_INVALID_OPERATION); // TODO: WPP
    }

    for (i = 0; i < pCsv->header.dwNumberOfFields; i++) {
        bResult = CsviWriteField(pCsv, pptCsvRecordValues[i]);
        CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP
    }

    pCsv->infos.dwNumberOfRecords += 1;

    SET_PTRVAL_IF_NOT_NULL(pdwCsvRecordNumber, pCsv->infos.dwNumberOfRecords);

    CSV_API_RETURN_SUCCESS(pCsv);
}

BOOL CsvWriteNextRecordA(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const LPSTR pptCsvRecordValues[],
    _Out_opt_ PDWORD pdwCsvRecordNumber
    ) {
    BOOL bReturn = FALSE;
	BOOL bResult = FALSE;
    LPWSTR *ppwCsvRecordValues = NULL;
	PCSV_OBJECT pCsv = NULL;

	bResult = CsviGetObjectByHandle(hCsvHandle, &pCsv);
	CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    if (pCsv->header.dwNumberOfFields) UtilsHeapAllocWStrArrAndConvertAStrArr(gs_pCsvHeap, pptCsvRecordValues, pCsv->header.dwNumberOfFields, &ppwCsvRecordValues);

    bReturn = CsvWriteNextRecordW(hCsvHandle, ppwCsvRecordValues, pdwCsvRecordNumber);

    if (pCsv->header.dwNumberOfFields && ppwCsvRecordValues) UtilsHeapFreeArrayHelper(gs_pCsvHeap, ppwCsvRecordValues, pCsv->header.dwNumberOfFields);
    return bReturn;
}

BOOL CsvWriteNextRecordWithTypes(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const DWORD_PTR ppCsvRecordValues[],
    _In_ const CSV_FIELD_TYPE pCsvRecordTypes[],
    _Out_opt_ PDWORD pdwCsvRecordNumber
    ) {
    DBG_UNREFERENCED_PARAMETER(hCsvHandle);
    DBG_UNREFERENCED_PARAMETER(ppCsvRecordValues);
    DBG_UNREFERENCED_PARAMETER(pCsvRecordTypes);
    DBG_UNREFERENCED_PARAMETER(pdwCsvRecordNumber);

    SET_PTRVAL_IF_NOT_NULL(pdwCsvRecordNumber, 0);

    //TODO!: stopped here
    return SUCCESS_VALUE;
}


BOOL CsvClose(
    _Inout_ PCSV_HANDLE phCsvHandle
    ) {
    BOOL bResult = ERROR_VALUE;
    PCSV_OBJECT pCsv = NULL;

    bResult = CsviGetObjectByHandle((*phCsvHandle), &pCsv);
    CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    if ((pCsv->file.eOperationType == CsvFileOperationWrite) || (pCsv->file.eOperationType == CsvFileOperationAppend)) {
        if (pCsv->file.read.pvFileMappedView) {
            bResult = UnmapViewOfFile(pCsv->file.read.pvFileMappedView);
            CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv));
        }
        if (pCsv->file.read.hFileMapping) {
            bResult = CloseHandle(pCsv->file.read.hFileMapping);
            CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, CSV_API_SAME_ERROR(pCsv));
        }
    }

    bResult = CloseHandle(pCsv->file.hFileHandle);
    CSV_API_RETURN_ERROR_IF_FAILED(pCsv, bResult, GLE()); // TODO: WPP
        
    bResult = CsviReleaseObject((*phCsvHandle));
    if (API_FAILED(bResult)) {
       return ERROR_VALUE; // TODO: WPP
    }

    (*phCsvHandle) = CSV_INVALID_HANDLE_VALUE;

    return SUCCESS_VALUE;
}


BOOL CsvResetFile(
    _In_ const CSV_HANDLE hCsvHandle
    ) {
    BOOL bResult = FALSE;
    PCSV_OBJECT pCsv = NULL;
    
    bResult = CsviGetObjectByHandle(hCsvHandle, &pCsv);
    CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    if (pCsv->file.eOperationType != CsvFileOperationRead) {
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_INVALID_OPERATION); // TODO: WPP
    }

    pCsv->file.dwCurrentField = 0;
    pCsv->file.read.llPosition = pCsv->header.llPositionAfterHeader;
    pCsv->file.read.bEofReached = FALSE;

    CSV_API_RETURN_SUCCESS(pCsv);
}

BOOL CsvSetOption(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const CSV_OPTION eCsvOption,
    _In_ const DWORD dwCsvOptionValue
    ) {
    BOOL bResult = ERROR_VALUE;
    PCSV_OBJECT pCsv = NULL;
    
    bResult = CsviGetObjectByHandle(hCsvHandle, &pCsv);
    CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    if (pCsv->infos.dwNumberOfRecords > 0) {
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_ALREADY_STARTED);
    }

    switch (eCsvOption) {
    case CsvOptionCustomFieldSeparator: bResult = CsviOptionSetFieldSeparator(pCsv, (WCHAR)dwCsvOptionValue); break;
    case CsvOptionAlwaysEnquoteFields: /*Nothing to do*/; bResult = TRUE; break;
    case CsvOptionBufferedOperations: CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_NOT_IMPLEMENTED); break; // TODO
    case CsvOptionThreadedOperations: CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_NOT_IMPLEMENTED); break; // TODO
    default:
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_UNKNOW_OPTION); // TODO: WPP
    }

    CSV_SET_OPTION(pCsv, eCsvOption);
    CSV_API_RETURN_SUCCESS(pCsv);
}


BOOL CsvUnsetOption(
    _In_ const CSV_HANDLE hCsvHandle,
    _In_ const CSV_OPTION eCsvOption
    ) {
    BOOL bResult = ERROR_VALUE;
    PCSV_OBJECT pCsv = NULL;

    bResult = CsviGetObjectByHandle(hCsvHandle, &pCsv);
    CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

    if (pCsv->infos.dwNumberOfRecords > 0) {
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_ALREADY_STARTED);
    }

    switch (eCsvOption) {
    case CsvOptionCustomFieldSeparator: bResult = CsviOptionSetFieldSeparator(pCsv, CSV_DEFAULT_SEPARATOR); break;
    case CsvOptionAlwaysEnquoteFields: /*Nothing to do*/ bResult = TRUE; break;
    case CsvOptionBufferedOperations: CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_NOT_IMPLEMENTED); break; // TODO
    case CsvOptionThreadedOperations: CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_NOT_IMPLEMENTED); break; // TODO
    default:
        CSV_API_RETURN_ERROR(pCsv, CSV_ERROR_UNKNOW_OPTION); // TODO: WPP
    }

    CSV_UNSET_OPTION(pCsv, eCsvOption);
    CSV_API_RETURN_SUCCESS(pCsv);
}


DWORD CsvGetLastError(
   _In_ const CSV_HANDLE hCsvHandle
   ) {
   BOOL bResult = ERROR_VALUE;
   PCSV_OBJECT pCsv = NULL;

   bResult = CsviGetObjectByHandle(hCsvHandle, &pCsv);
   CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

   return pCsv->infos.dwLastError;
}

BOOL WINAPI DllMain(
    _In_  HINSTANCE hinstDLL,
    _In_  DWORD fdwReason,
    _In_  LPVOID lpvReserved
    ) {
    BOOL bResult = FALSE;

    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: bResult = CsviLibInit(); break;
    case DLL_PROCESS_DETACH: bResult = CsviLibCleanup(); break;
    default: bResult = TRUE; break;
    }

    return bResult;
}

BOOL CsvGetHeaderNumberOfFields(
   _In_ const CSV_HANDLE hCsvHandle,
   _Inout_ PDWORD pdwHeaderNumberOfFields
   ) {
   BOOL bResult = ERROR_VALUE;
   PCSV_OBJECT pCsv = NULL;

   bResult = CsviGetObjectByHandle(hCsvHandle, &pCsv);
   CSV_API_RETURN_ERROR_IF_FAILED_GETTING_CSV(bResult, CSV_API_SAME_ERROR(pCsv)); // TODO: WPP

   (*pdwHeaderNumberOfFields) = pCsv->header.dwNumberOfFields;

   CSV_API_RETURN_SUCCESS(pCsv);
}

VOID CsvHeapFree(
	_In_ PVOID pMem
) {
	UtilsHeapFree(0, gs_pCsvHeap, pMem);
}

VOID CsvRecordArrayHeapFree(
	_In_ PVOID *ppMemArr,
	_In_ DWORD dwCount
) {
	UtilsHeapFreeArrayHelper(gs_pCsvHeap, ppMemArr, dwCount);
}