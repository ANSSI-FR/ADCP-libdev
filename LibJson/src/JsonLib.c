/******************************************************************************\
Author : L. Delsalle
Date   : 26/01/2015
Descr. : Lightweight JSON Library written in C Windows
         based on jsmn project (cf. http://zserge.com/jsmn.html).
         For now, this library only support READ operations.
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#define LIB_ERROR_VAL gs_dwJsonLastError
#include "JsonInternals.h"

#ifdef _WIN32
    #include "JsonWpp.h"
    #include "JsonLib.tmh"
#endif

/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
DWORD gs_dwJsonLastError = NO_ERROR;
PUTILS_HEAP gs_pJsonHeap = NULL;
JSON_HANDLE_TABLE gs_sJsonHandleTable = { .dwTableSize = 0, .dwEntryCount = 0, .pEntries = NULL };

/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
BOOL
JsonLibInit (
)
{
   BOOL bResult = FALSE;

   bResult = UtilsHeapCreate(&gs_pJsonHeap, JSON_HEAP_NAME, NULL);
   API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

   API_RETURN_SUCCESS();
}

BOOL
JsonLibCleanup (
)
{
   BOOL bResult = FALSE;

   bResult = UtilsHeapDestroy(&gs_pJsonHeap);
   API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

   API_RETURN_SUCCESS();
}

BOOL JsonOpenFileReadW(
    _In_ const LPWSTR                                   lpwJsonFilename,
    _Out_ PJSON_OBJECT                                  *ppJsonObj
    )
{
    BOOL bResult = ERROR_VALUE;
    PJSON_INTERNAL_OBJECT pJson = NULL;
    JSON_HANDLE hJson = JSON_INVALID_HANDLE_VALUE;

    SET_PTRVAL_IF_NOT_NULL(ppJsonObj, 0);

    bResult = JsoniNewInternalObject(&pJson, &hJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP

    bResult = JsoniOpenFile(lpwJsonFilename, JsonFileOperationRead, pJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP

    bResult = JsoniCreateFileMapping(pJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP

    bResult = JsoniParseJS(pJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

    // Check that JSON file start with an array or an object, otherwise it's invalid
    if ((pJson->jsmn.pAllocatedTokens[0].type != JSMN_OBJECT)
        && ((pJson->jsmn.pAllocatedTokens[0].type != JSMN_ARRAY)))
    {
        API_RETURN_ERROR(JSON_ERROR_INVALID_JSON_FILE)
    }

    bResult = JsoniAllocAndFillJsonObj(NULL, &pJson->jsmn.pAllocatedTokens[0], pJson, ppJsonObj);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

    API_RETURN_SUCCESS();
}

BOOL JsonOpenFileReadA(
    _In_ const LPSTR                                    lpJsonFilename,
    _Out_ PJSON_OBJECT                                  *ppJsonObj
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwJsonFilename = NULL;

    UtilsHeapAllocWStrAndConvertAStr(gs_pJsonHeap, lpJsonFilename, &lpwJsonFilename);

    bReturn = JsonOpenFileReadW(lpwJsonFilename, ppJsonObj);

    UtilsHeapFreeAndNullHelper(gs_pJsonHeap, lpwJsonFilename);
    return bReturn;
}

BOOL JsonParseStringReadW(
    _In_ const LPWSTR                                   lpwJsonString,
    _Out_ PJSON_OBJECT                                  *ppJsonObj
    )
{
    BOOL bResult = ERROR_VALUE;
    LPSTR lpJsonString = NULL;
    PJSON_INTERNAL_OBJECT pJson = NULL;
    JSON_HANDLE hJson = JSON_INVALID_HANDLE_VALUE;

    SET_PTRVAL_IF_NOT_NULL(ppJsonObj, 0);

    bResult = JsoniNewInternalObject(&pJson, &hJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP

    UtilsHeapAllocAStrAndConvertWStr(gs_pJsonHeap, lpwJsonString, &lpJsonString);
    pJson->eJsonOperation = JsonFileOperationRead;
    pJson->data.pvData = lpJsonString;
    pJson->data.llDataSize = (strlen(lpJsonString) + 1) * sizeof(BYTE);

    bResult = JsoniParseJS(pJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

    // Check that JSON file start with an array or an object, otherwise it's invalid
    if ((pJson->jsmn.pAllocatedTokens[0].type != JSMN_OBJECT)
        && ((pJson->jsmn.pAllocatedTokens[0].type != JSMN_ARRAY)))
    {
        API_RETURN_ERROR(JSON_ERROR_INVALID_JSON_FILE)
    }

    bResult = JsoniAllocAndFillJsonObj(NULL, &pJson->jsmn.pAllocatedTokens[0], pJson, ppJsonObj);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

    API_RETURN_SUCCESS();
}

BOOL JsonParseStringReadA(
    _In_ const LPSTR                                    lpJsonString,
    _Out_ PJSON_OBJECT                                  *ppJsonObj
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwJsonString = NULL;

    UtilsHeapAllocWStrAndConvertAStr(gs_pJsonHeap, lpJsonString, &lpwJsonString);

    bReturn = JsonParseStringReadW(lpwJsonString, ppJsonObj);

    UtilsHeapFreeAndNullHelper(gs_pJsonHeap, lpwJsonString);

    return bReturn;
}

BOOL JsonGetNextObjectW(
    _In_ const PJSON_OBJECT                             pJsonObj,
    _In_opt_ LPWSTR                                     lppwObjectNames[],
    _In_opt_ DWORD                                      dwObjectNamesCount,
    _Inout_ PJSON_OBJECT                                *ppJsonResultObj
    )
{
    BOOL bResult = ERROR_VALUE;
    PJSON_INTERNAL_OBJECT pJson = NULL;
    LPSTR lpExtractedTokenKey = NULL;
    pjsmntok_t pChildToken = NULL;

    SET_PTRVAL_IF_NOT_NULL(ppJsonResultObj, 0);
    if (pJsonObj == NULL)
        API_RETURN_ERROR(JSON_ERROR_INVALID_PARAMETER);

    // Get the json object
    bResult = JsoniGetObjectByHandle(pJsonObj->privatestruct.hJson, &pJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP

    // Verify we are processing read Json object
    if (pJson->eJsonOperation != JsonFileOperationRead)
    {
        API_RETURN_ERROR(JSON_ERROR_INVALID_OPERATION);
    }

    // Verify we are trying to parse a JSON object or array
    if ((pJson->eObjectType != JsonResultTypeObject)
        && (pJson->eObjectType != JsonResultTypeArray))
    {
        API_RETURN_ERROR(JSON_ERROR_INVALID_JSON_FILE);
    }

    // Parse JSON content if needed
    if (pJson->parsing_state.eCurrentParseState == NOT_STARTED)
    {
        bResult = JsoniParseJS(pJson);
        API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());
    }

    // Verify we do not reach the end of the file (in that case, return success with a NULL object)
    if (pJson->parsing_state.dwRemainingTokens == 0)
    {
        API_RETURN_SUCCESS();
    }

    // Launch token browsing to find requested token
    bResult = JsoniBrowseJsonFile(pJson, lppwObjectNames, dwObjectNamesCount, &pChildToken, &lpExtractedTokenKey);
    if (bResult == SUCCESS_VALUE)
    {
        bResult = JsoniAllocAndFillJsonObj(lpExtractedTokenKey, pChildToken, pJson, ppJsonResultObj);
        API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());
    }
    // Fail if we have a different error than TOKEN_NOT_FOUND (which is a success call )
    else if (JsonGetLastError() != JSON_ERROR_TOKEN_NOT_FOUND)
    {
        API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());
    }

    API_RETURN_SUCCESS();
}

BOOL JsonGetNextObjectA(
    _In_ const PJSON_OBJECT                             pJsonObj,
    _In_opt_ LPSTR                                      lppObjectNames[],
    _In_opt_ DWORD                                      dwObjectNamesCount,
    _Inout_ PJSON_OBJECT                                *ppJsonResultObj
    )
{
    BOOL bReturn = FALSE;
    LPWSTR *lppwObjectNames = NULL;

    UtilsHeapAllocWStrArrAndConvertAStrArr(gs_pJsonHeap, lppObjectNames, dwObjectNamesCount, &lppwObjectNames);

    bReturn = JsonGetNextObjectW(pJsonObj, lppwObjectNames, dwObjectNamesCount, ppJsonResultObj);

    if (lppwObjectNames) UtilsHeapFreeAndNullHelper(gs_pJsonHeap, lppwObjectNames);

    return bReturn;
}

BOOL JsonRestartParsing(
    _In_ const PJSON_OBJECT                             pJsonObj
    )
{
    BOOL bResult = ERROR_VALUE;
    PJSON_INTERNAL_OBJECT pJson = NULL;

    // Get the csv object
    bResult = JsoniGetObjectByHandle(pJsonObj->privatestruct.hJson, &pJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP

    // Verify we are trying to reset a JSON object or array
    if ((pJson->eObjectType != JsonResultTypeObject)
        && (pJson->eObjectType != JsonResultTypeArray))
    {
        API_RETURN_ERROR(JSON_ERROR_INVALID_JSON_FILE);
    }

    // Reset parser pointers
    pJson->parsing_state.dwRemainingTokens = 1;
    pJson->parsing_state.dwTokensArrayIndex = 0;
    pJson->parsing_state.eCurrentParseState = START;

    API_RETURN_SUCCESS();
}

BOOL JsonObjectForeachRequestedElement(
    _In_ const PJSON_OBJECT                             pJsonObj,
    _In_ const BOOL                                     bAllowUnknownElements,
    _In_ const JSON_REQUESTED_ELEMENT                   pRequestedElements[],
    _In_ const DWORD                                    dwRequestedElementsCount,
    _In_opt_ const PVOID                                pvContext,
    _Out_opt_ PDWORD                                    pdwElmtCount
    )
{
    BOOL bNoMoreTokens = FALSE;
    BOOL bResult = FALSE;
    PJSON_OBJECT pJsonElement = NULL;
    PTCHAR *pptReqElmtKeys = NULL;
    BOOL *pbtReqElmtFound = NULL;
    DWORD i = 0;
    DWORD dwElmtIdx = 0;
    DWORD dwError = NO_ERROR;

    SET_PTRVAL_IF_NOT_NULL(pdwElmtCount, 0);

    if (pJsonObj->eObjectType != JsonResultTypeObject)
    {
        API_RETURN_ERROR(JSON_ERROR_INVALID_PARAMETER);
    }

    pptReqElmtKeys = UtilsHeapAllocArrayHelper(gs_pJsonHeap, PTCHAR, dwRequestedElementsCount);
    pbtReqElmtFound = UtilsHeapAllocArrayHelper(gs_pJsonHeap, BOOL, dwRequestedElementsCount);
    for (i = 0; i < dwRequestedElementsCount; i++) {
        pptReqElmtKeys[i] = pRequestedElements[i].ptKey;
        pbtReqElmtFound[i] = FALSE;
    }

    while (bNoMoreTokens == FALSE)
    {
        bResult = JsonGetNextObject(pJsonObj, NULL, 0, &pJsonElement);
        GOTO_FAIL_IF(API_FAILED(bResult), dwError = SAME_ERROR());

        if (pJsonElement == NULL)
        {
            bNoMoreTokens = TRUE;
        }
        else
        {
            bResult = IsInSetOfStrings(pJsonElement->ptKey, pptReqElmtKeys, dwRequestedElementsCount, &dwElmtIdx);
            if (bResult == FALSE)
            {
                GOTO_FAIL_IF(bAllowUnknownElements == FALSE, dwError = JSON_ERROR_INVALID_TOKEN);
            }
            else
            {
                GOTO_FAIL_IF(pRequestedElements[dwElmtIdx].eExpectedType != pJsonElement->eObjectType, dwError = JSON_ERROR_INVALID_OBJECT_TYPE);
                bResult = pRequestedElements[dwElmtIdx].pfnCallback(pJsonElement, pvContext);
                GOTO_FAIL_IF(bResult == FALSE, dwError = JSON_ERR_UNKNOWN_TODO);
                pbtReqElmtFound[dwElmtIdx] = TRUE;
                SET_PTRVAL_IF_NOT_NULL(pdwElmtCount, (*pdwElmtCount)+1);
                JsonReleaseObject(&pJsonElement);
            }
        }
    }

    for (i = 0; i < dwRequestedElementsCount; i++)
    {
        GOTO_FAIL_IF(pRequestedElements[i].bMustBePresent == TRUE && pbtReqElmtFound[i] == FALSE, dwError = JSON_ERROR_TOKEN_NOT_FOUND);
    }

    UtilsHeapFreeAndNullHelper(gs_pJsonHeap, pptReqElmtKeys);
    UtilsHeapFreeAndNullHelper(gs_pJsonHeap, pbtReqElmtFound);

    API_RETURN_SUCCESS();

LABEL_FAIL:
    UtilsHeapFreeAndNullHelper(gs_pJsonHeap, pptReqElmtKeys);
    UtilsHeapFreeAndNullHelper(gs_pJsonHeap, pbtReqElmtFound);
    API_RETURN_ERROR(dwError);
}

BOOL JsonForeachElement(
    _In_ const PJSON_OBJECT                             pJsonObj,
    _In_ const PFN_JSON_ELEMENT_CALLBACK                pfnCallback,
    _In_opt_ const PVOID                                pvContext,
    _Out_opt_ PDWORD                                    pdwElmtCount
    ) {
    BOOL bNoMoreEntries = FALSE;
    BOOL bResult = FALSE;
    PJSON_OBJECT pJsonElement = NULL;

    SET_PTRVAL_IF_NOT_NULL(pdwElmtCount, 0);

    if (pJsonObj->eObjectType != JsonResultTypeObject && pJsonObj->eObjectType != JsonResultTypeArray)
    {
        API_RETURN_ERROR(JSON_ERROR_INVALID_PARAMETER);
    }

    while (bNoMoreEntries == FALSE) {
        bResult = JsonGetNextObject(pJsonObj, NULL, 0, &pJsonElement);
        API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

        if (pJsonElement == NULL)
        {
            bNoMoreEntries = TRUE;
        }
        else
        {
            bResult = pfnCallback(pJsonElement, pvContext);
            JsonReleaseObject(&pJsonElement);
            API_RETURN_ERROR_IF_FAILED(bResult, JSON_ERR_UNKNOWN_TODO);
            SET_PTRVAL_IF_NOT_NULL(pdwElmtCount, (*pdwElmtCount) + 1);
        }
    }

    API_RETURN_SUCCESS();
}

BOOL JsonReleaseObject(
    _Inout_ PJSON_OBJECT                                *ppJsonObj
    )
{
    BOOL bResult = ERROR_VALUE;
    PJSON_INTERNAL_OBJECT pJson = NULL;

    // Get the json object
    bResult = JsoniGetObjectByHandle((*ppJsonObj)->privatestruct.hJson, &pJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP

    // Release internal object
    if (pJson->file.hFileHandle != NULL)
    {
        // If the json object came from a mapped file
        if (pJson->data.pvData) {
            bResult = UnmapViewOfFile(pJson->data.pvData);
            API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());
        }
        if (pJson->file.hFileMapping) {
            bResult = CloseHandle(pJson->file.hFileMapping);
            API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());
        }

        if (pJson->file.ptFileName) {
            UtilsHeapFreeAndNullHelper(gs_pJsonHeap, pJson->file.ptFileName);
        }

        bResult = CloseHandle(pJson->file.hFileHandle);
        API_RETURN_ERROR_IF_FAILED(bResult, GLE());
    }
    else
    {
        // If the json object came from memory data
        if ((pJson->data.pvData) && (pJson->eObjectType == JsonResultTypeString)) {
            UtilsHeapFreeAndNullHelper(gs_pJsonHeap, pJson->data.pvData);
        }
    }
    // Release public object
    if ((*ppJsonObj)->ptKey) {
        UtilsHeapFreeAndNullHelper(gs_pJsonHeap, (*ppJsonObj)->ptKey);
    }
    if (((*ppJsonObj)->value.lpObject) && (pJson->eObjectType == JsonResultTypeString)){
        UtilsHeapFreeAndNullHelper(gs_pJsonHeap, (*ppJsonObj)->value.lpObject);
    }

    // Release private object
    bResult = JsoniReleaseInternalObject((*ppJsonObj)->privatestruct.hJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP

    SET_PTRVAL_IF_NOT_NULL(ppJsonObj, 0);
    API_RETURN_SUCCESS();
}

DWORD JsonGetLastError(
    )
{
    return gs_dwJsonLastError;
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
    case DLL_PROCESS_ATTACH: bResult = JsonLibInit(); break;
    case DLL_PROCESS_DETACH: bResult = JsonLibCleanup(); break;
    default: bResult = TRUE; break;
    }

    return bResult;
}
#endif