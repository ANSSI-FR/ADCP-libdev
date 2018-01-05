/******************************************************************************\
Author : L. Delsalle
Date   : 26/01/2015
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#define LIB_ERROR_VAL gs_dwJsonLastError
#include "LibUtils\src\UtilsLib.h"
#include "JsonInternals.h"

#ifdef _WIN32
    #include "JsonWpp.h"
    #include "JsonInternals.tmh"
#endif

/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
static pjsmntok_t jsmn_alloc_token(
    _In_ pjsmn_parser                   parser,
    _Inout_ pjsmntok_t                  tokens,
    _Inout_ size_t                      num_tokens
    )
{
    UINT i = 0;

    for (i = parser->toknext; i < num_tokens; i++)
    {
        if (tokens[i].start == -1 && tokens[i].end == -1)
        {
            parser->toknext = i + 1;
            return &tokens[i];
        }
    }

    return NULL;
}

static void jsmn_fill_token(
    _Inout_ pjsmntok_t                  token,
    _In_ jsmntype_t                     type,
    _In_ INT                            start,
    _In_ INT                            end
    )
{
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

static int jsmn_parse_primitive(
    _In_ pjsmn_parser                   parser,
    _In_ const PCHAR                    js,
    _Inout_ pjsmntok_t                  tokens,
    _Inout_ size_t                      num_tokens
    )
{
    jsmntok_t *token;
    int start;

    start = parser->pos;

    for (; js[parser->pos] != '\0'; parser->pos++)
    {
        switch (js[parser->pos])
        {
#ifndef JSMN_STRICT
            /* In strict mode primitive must be followed by "," or "}" or "]" */
        case '\t': case '\r': case '\n': case ' ': case ':':
#endif
        case ',': case ']': case '}':
            goto found;
        }
        if (js[parser->pos] < 32 || js[parser->pos] >= 127)
        {
            parser->pos = start;
            return JSMN_ERROR_INVAL;
        }
    } // End of for loop
#ifdef JSMN_STRICT
    /* In strict mode primitive must be followed by a comma/object/array */
    parser->pos = start;
    return JSMN_ERROR_PART;
#endif

found:
    token = jsmn_alloc_token(parser, tokens, num_tokens);
    if (token == NULL)
    {
        parser->pos = start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
    parser->pos--;
    return JSMN_SUCCESS;
}

static int jsmn_parse_string(
    _In_ pjsmn_parser                   parser,
    _In_  const char                    *js,
    _Inout_ pjsmntok_t                  tokens,
    _Inout_ size_t                      num_tokens
    )
{
    jsmntok_t *token;

    INT start = parser->pos;

    parser->pos++;

    /* Skip starting quote */
    for (; js[parser->pos] != '\0'; parser->pos++)
    {
        CHAR c = js[parser->pos];

        /* Quote: end of string */
        if (c == '\"')
        {
            token = jsmn_alloc_token(parser, tokens, num_tokens);
            if (token == NULL)
            {
                parser->pos = start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_fill_token(token, JSMN_STRING, start + 1, parser->pos);
            return JSMN_SUCCESS;
        }

        /* Backslash: Quoted symbol expected */
        if (c == '\\')
        {
            parser->pos++;
            switch (js[parser->pos]) {
                /* Allowed escaped symbols */
            case '\"': case '/': case '\\': case 'b':
            case 'f': case 'r': case 'n': case 't':
                break;
                /* Allows escaped symbol \uXXXX */
            case 'u':
                /* TODO */
                break;
                /* Unexpected symbol */
            default:
                parser->pos = start;
                return JSMN_ERROR_INVAL;
            }
        }
    }
    parser->pos = start;
    return JSMN_ERROR_PART;
}

jsmnerr_t jsmn_parse(
    _In_ pjsmn_parser                   parser,
    _In_ const PCHAR                    js,
    _Inout_ pjsmntok_t                  tokens,
    _Inout_ INT                         num_tokens
    )
{
    INT r;
    INT i;
    pjsmntok_t token;

    /* initialize the rest of tokens (they could be reallocated) */
    for (i = parser->toknext; i < num_tokens; i++)
    {
        jsmn_fill_token(&tokens[i], JSMN_PRIMITIVE, -1, -1);
    }

    for (; js[parser->pos] != '\0'; parser->pos++)
    {
        CHAR c;
        jsmntype_t type;

        c = js[parser->pos];
        switch (c)
        {
            case '{': case '[':
                token = jsmn_alloc_token(parser, tokens, num_tokens);
                if (token == NULL)
                    return JSMN_ERROR_NOMEM;
                if (parser->toksuper != -1)
                    tokens[parser->toksuper].size++;
                token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
                token->start = parser->pos;
                parser->toksuper = parser->toknext - 1;
                break;
            case '}': case ']':
                type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
                for (i = parser->toknext - 1; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return JSMN_ERROR_INVAL;
                        }
                        parser->toksuper = -1;
                        token->end = parser->pos + 1;
                        break;
                    }
                }
                /* Error if unmatched closing bracket */
                if (i == -1)
                    return JSMN_ERROR_INVAL;
                for (; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        parser->toksuper = i;
                        break;
                    }
                }
                break;
            case '\"':
                r = jsmn_parse_string(parser, js, tokens, num_tokens);
                if (r < 0)
                    return r;
                if (parser->toksuper != -1)
                    tokens[parser->toksuper].size++;
                break;
            case '\t': case '\r': case '\n': case ':': case ',': case ' ':
                break;
#ifdef JSMN_STRICT
            /* In strict mode primitives are: numbers and booleans */
            case '-': case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
            case 't': case 'f': case 'n':
#else
                /* In non-strict mode every unquoted value is a primitive */
            default:
#endif
                r = jsmn_parse_primitive(parser, js, tokens, num_tokens);
                if (r < 0)
                    return r;
                if (parser->toksuper != -1)
                    tokens[parser->toksuper].size++;
                break;

#ifdef JSMN_STRICT
            /* Unexpected char in strict mode */
        default:
            return JSMN_ERROR_INVAL;
#endif

        } // End of switch
    }

    for (i = parser->toknext - 1; i >= 0; i--)
    {
        /* Unmatched opened object or array */
        if (tokens[i].start != -1 && tokens[i].end == -1) {
            return JSMN_ERROR_PART;
        }
    }

    return JSMN_SUCCESS;
}

VOID jsmn_init(
    _Inout_ pjsmn_parser                parser
    )
{
    parser->pos = 0;
    parser->toknext = 0;
    parser->toksuper = -1;
}

static BOOL JsoniExtendHandleTable(
    )
{
    PJSON_INTERNAL_OBJECT *pEntriesNew = NULL;
    DWORD dwTableSizeNew = 0;

    dwTableSizeNew = gs_sJsonHandleTable.dwTableSize + JSON_HANDLE_TABLE_DELTA;
    pEntriesNew = UtilsHeapAllocOrReallocHelper(gs_pJsonHeap, gs_sJsonHandleTable.pEntries, SIZEOF_ARRAY(PJSON_INTERNAL_OBJECT, dwTableSizeNew));

    gs_sJsonHandleTable.pEntries = pEntriesNew;
    gs_sJsonHandleTable.dwTableSize = dwTableSizeNew;

    API_RETURN_SUCCESS();
}

static BOOL SkipJsonTokenIfNeeded(
    _In_ const PJSON_INTERNAL_OBJECT    pJson,
    _In_ const pjsmntok_t               pToken,
    _Inout_ PDWORD                      pdwTokenBuffIdx,
    _Inout_ PDWORD                      pdwRemainingTokens
    )
{
    pjsmntok_t pTmp = NULL;

    // If we found an array or an object, we need to skip to attend next sibling element
    if ((pToken->type == JSMN_OBJECT) || (pToken->type == JSMN_ARRAY))
    {
        do
        {
            pTmp = &pJson->jsmn.pAllocatedTokens[++(*pdwTokenBuffIdx)];
            //printf("index:%#x start:%#x end:%#x\r\n", *pdwTokenBuffIdx, pTmp->start, pTmp->end);
            if (pTmp->start == -1)
                break;
        } while (pTmp->start < pToken->end);

        (*pdwRemainingTokens) -= pToken->size;
    }

    API_RETURN_SUCCESS();
}

/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
BOOL JsoniOpenFile(
    _In_ const LPWSTR                   lpwFilename,
    _In_ const JSON_OPERATION           eFileOperation,
    _Inout_ PJSON_INTERNAL_OBJECT       pJson
    )
{
    HANDLE hFileHandle = INVALID_HANDLE_VALUE;
    DWORD dwDesiredAccess = (eFileOperation == JsonFileOperationWrite) ? GENERIC_WRITE : ((eFileOperation == JsonFileOperationAppend) ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ);
    DWORD dwCreationDisposition = (eFileOperation == JsonFileOperationWrite) ? CREATE_NEW : ((eFileOperation == JsonFileOperationAppend) ? OPEN_EXISTING : OPEN_EXISTING);

    hFileHandle = CreateFileW(lpwFilename, dwDesiredAccess, FILE_SHARE_READ, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFileHandle == INVALID_HANDLE_VALUE)
    {
        DWORD dwErr = GetLastError();
        UNREFERENCED_PARAMETER(dwErr);
        API_RETURN_SAME_ERROR(); // TODO: WPP
    }

    pJson->eJsonOperation = eFileOperation;

    pJson->file.ptFileName = UtilsHeapStrDupHelper(gs_pJsonHeap, lpwFilename);
    pJson->file.hFileHandle = hFileHandle;
    pJson->file.hFileMapping = NULL;

    pJson->data.llDataSize = 0;
    pJson->data.bIsWCharData = FALSE;
    pJson->data.pvData = NULL;

    API_RETURN_SUCCESS();
}

BOOL JsoniCreateFileMapping(
    _Inout_ PJSON_INTERNAL_OBJECT       pJson
    )
{
    BOOL bResult = FALSE;
    LARGE_INTEGER liFileSize = { 0 };
    DWORD flProtect = (pJson->eJsonOperation == JsonFileOperationWrite) ? PAGE_READWRITE : ((pJson->eJsonOperation == JsonFileOperationAppend) ? PAGE_READWRITE : PAGE_READONLY);

    bResult = GetFileSizeEx(pJson->file.hFileHandle, &liFileSize);
    if (bResult == FALSE)
    {
        API_RETURN_ERROR(GLE()); // TODO: ERR+WPP
    }
    pJson->data.llDataSize = liFileSize.QuadPart;

    // TODO : it would be cleaner here to map a small portion of the file, and then map the next parts when needed, than map the entire file...
    // cf. LibCsv

    pJson->file.hFileMapping = CreateFileMapping(pJson->file.hFileHandle, NULL, flProtect, 0, 0, NULL);
    if (pJson->file.hFileMapping == NULL) {
        API_RETURN_ERROR(GLE()); // TODO: ERR+WPP
    }

    pJson->data.pvData = MapViewOfFile(pJson->file.hFileMapping, FILE_MAP_READ, 0, 0, 0);
    if (pJson->data.pvData == NULL) {
        API_RETURN_ERROR(GLE()); // TODO: ERR+WPP
    }

    API_RETURN_SUCCESS();
}

BOOL JsoniNewInternalObject(
    _Out_ PJSON_INTERNAL_OBJECT         *ppObject,
    _Out_ PJSON_HANDLE                  pJsonHandle
    )
{
    BOOL bResult = ERROR_VALUE;
    DWORD i = 0;

    (*pJsonHandle) = 0;
    (*ppObject) = NULL;

    if (gs_sJsonHandleTable.dwEntryCount == gs_sJsonHandleTable.dwTableSize)
    {
        bResult = JsoniExtendHandleTable();
        API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP
    }

    for (i = 0; i < gs_sJsonHandleTable.dwTableSize && gs_sJsonHandleTable.pEntries[i] != NULL; i++) {};

    gs_sJsonHandleTable.pEntries[i] = UtilsHeapAllocStructHelper(gs_pJsonHeap, JSON_INTERNAL_OBJECT);
    gs_sJsonHandleTable.dwEntryCount += 1;

    (*pJsonHandle) = i;
    (*ppObject) = gs_sJsonHandleTable.pEntries[i];
    (*ppObject)->eObjectType = JsonResultTypeUnknown;
    (*ppObject)->eJsonOperation = JsonFileOperationInvalid;

    (*ppObject)->file.hFileHandle = NULL;
    (*ppObject)->file.ptFileName = NULL;
    (*ppObject)->file.hFileMapping = INVALID_HANDLE_VALUE;

    (*ppObject)->data.bIsWCharData = FALSE;
    (*ppObject)->data.llDataSize = 0;
    (*ppObject)->data.pvData = NULL;

    jsmn_init(&((*ppObject)->jsmn.sParser));
    (*ppObject)->jsmn.dwAllocatedTokens = 0;
    (*ppObject)->jsmn.pAllocatedTokens = NULL;

    (*ppObject)->parsing_state.eCurrentParseState = NOT_STARTED;
    (*ppObject)->parsing_state.dwRemainingTokens = 0;
    (*ppObject)->parsing_state.dwTokensArrayIndex = 0;

    API_RETURN_SUCCESS();
}

BOOL JsoniGetObjectByHandle(
    _In_ const JSON_HANDLE              hHandle,
    _Out_ PJSON_INTERNAL_OBJECT         *ppObject
    )
{
    (*ppObject) = NULL;

    if (hHandle != JSON_INVALID_HANDLE_VALUE && hHandle <= gs_sJsonHandleTable.dwTableSize && gs_sJsonHandleTable.pEntries[hHandle] != NULL)
    {
        (*ppObject) = gs_sJsonHandleTable.pEntries[hHandle];
        API_RETURN_SUCCESS();
    }

    API_RETURN_ERROR(JSON_ERROR_INVALID_HANDLE);
}

BOOL JsoniParseJS(
    _In_ const PJSON_INTERNAL_OBJECT    pJson
    )
{
    jsmnerr_t dwErr = JSMN_ERROR_NOMEM;
    DWORD dwTokenCount = JSON_DEFAULT_TOKEN_COUNT;

    // Parse json file
    do
    {
        if (pJson->jsmn.pAllocatedTokens == NULL)
            pJson->jsmn.pAllocatedTokens = UtilsHeapAllocArrayHelper(gs_pJsonHeap, JSON_INTERNAL_OBJECT, JSON_DEFAULT_TOKEN_COUNT);
        else
            pJson->jsmn.pAllocatedTokens = UtilsHeapReallocArrayHelper(gs_pJsonHeap, pJson->jsmn.pAllocatedTokens, JSON_INTERNAL_OBJECT, dwTokenCount);
        dwTokenCount = 2 * dwTokenCount + 1;

        dwErr = jsmn_parse(&pJson->jsmn.sParser, pJson->data.pvData, pJson->jsmn.pAllocatedTokens, dwTokenCount);
    } while (dwErr == JSMN_ERROR_NOMEM);

    // Compute the exact number of token (TODO)
    pJson->jsmn.dwAllocatedTokens = dwTokenCount;

    // Setup parsing status
    pJson->parsing_state.dwRemainingTokens= 1;
    pJson->parsing_state.dwTokensArrayIndex = 0;
    pJson->parsing_state.eCurrentParseState = START;

    if (dwErr == JSMN_ERROR_INVAL)
        API_RETURN_ERROR(JSON_ERROR_INVALID_JSON_FILE)
    else if (dwErr == JSMN_ERROR_PART)
        API_RETURN_ERROR(JSON_ERROR_TRUNCATED_JSON_FILE)
    else
        API_RETURN_SUCCESS()
}

BOOL JsoniReleaseInternalObject(
    _In_ const JSON_HANDLE              hHandle
    )
{
    PJSON_INTERNAL_OBJECT pJson = gs_sJsonHandleTable.pEntries[hHandle];

    if (pJson->jsmn.pAllocatedTokens)
    {
        UtilsHeapFreeAndNullHelper(gs_pJsonHeap, pJson->jsmn.pAllocatedTokens);
    }

    UtilsHeapFreeAndNullHelper(gs_pJsonHeap, gs_sJsonHandleTable.pEntries[hHandle]);
    gs_sJsonHandleTable.dwEntryCount -= 1;

    API_RETURN_SUCCESS();
}

BOOL JsoniTokenStreq(
    _In_ const LPSTR                    lpJS,
    _In_ const pjsmntok_t               pJsmnToken,
    _In_ const LPWSTR                   lpwMatchingKey
    )
{
    LPSTR lpToken = lpJS + pJsmnToken->start;
    DWORD dwTokenlen = pJsmnToken->end - pJsmnToken->start;
    LPSTR lpMatchingKey = NULL;
    BOOL bRes = FALSE;

    UtilsHeapAllocAStrAndConvertWStr(gs_pJsonHeap, lpwMatchingKey, &lpMatchingKey);

    bRes =  (
        (strncmp(lpToken, lpMatchingKey, dwTokenlen) == 0)
        && (strlen(lpMatchingKey) == (size_t)(dwTokenlen))
        );

    UtilsHeapFreeAndNullHelper(gs_pJsonHeap, lpMatchingKey);
    return bRes;
}

BOOL JsoniTokenToStr(
    _In_ const LPSTR                    lpJS,
    _In_ const pjsmntok_t               pJsmnToken,
    _Out_ LPSTR                        *lppStr
    )
{
    LPSTR lpStr = NULL, lpStart = (lpJS + pJsmnToken->start);
    DWORD dwStrLen = (pJsmnToken->end - pJsmnToken->start);
    BOOL bRes = FALSE;

    lpStr = UtilsHeapAllocStrHelper(gs_pJsonHeap, dwStrLen + 1);

    bRes = memcpy_s(lpStr, (dwStrLen + 1) * sizeof(CHAR), lpStart, (dwStrLen)* sizeof(CHAR));
    if (bRes != 0)
        API_RETURN_ERROR(JSON_ERROR_MEMORY_OPERATION);

    *lppStr = lpStr;
    API_RETURN_SUCCESS();
}

BOOL JsoniAllocAndFillJsonObj(
    _In_opt_ LPSTR                      lpParentKey,
    _In_opt_ pjsmntok_t                 pChildToken,
    _In_ PJSON_INTERNAL_OBJECT          pJsonInternalObj,
    _In_opt_ PJSON_OBJECT               *ppJsonObj
    )
{
    BOOL bResult = FALSE;
    LPSTR lpChildJson = NULL;
    PJSON_INTERNAL_OBJECT pNewInternalObj = NULL;
    JSON_HANDLE hNewJsonObj = JSON_INVALID_HANDLE_VALUE;
    PJSON_OBJECT pNewJsonObj = NULL;

    // Extract the child token json
    bResult = JsoniTokenToStr(pJsonInternalObj->data.pvData, pChildToken, &lpChildJson);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

    // Allocate internal object
    bResult = JsoniNewInternalObject(&pNewInternalObj, &hNewJsonObj);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

    pNewInternalObj->eJsonOperation = pJsonInternalObj->eJsonOperation;
    pNewInternalObj->data.bIsWCharData = pJsonInternalObj->data.bIsWCharData;
    pNewInternalObj->data.llDataSize = (strlen(lpChildJson) + 1);
    pNewInternalObj->data.pvData = UtilsHeapMemDupHelper(gs_pJsonHeap, lpChildJson, (DWORD) (strlen(lpChildJson) + 1) * sizeof(BYTE));//UtilsHeapStrDupHelper(gs_pJsonHeap, lpChildJson);

    // Fill external object
    pNewJsonObj = UtilsHeapAllocStructHelper(gs_pJsonHeap, JSON_OBJECT);
    pNewJsonObj->privatestruct.hJson = hNewJsonObj;
    pNewJsonObj->eObjectType = JsonResultTypeUnknown;
    if (lpParentKey) UtilsHeapAllocWStrAndConvertAStr(gs_pJsonHeap, lpParentKey, &pNewJsonObj->ptKey);

    switch (pChildToken->type)
    {
    case JSMN_PRIMITIVE:
        // Determine if the primitive is a boolean
        if ((strcmp(lpChildJson, JSON_PRIMITIVE_BOOL_TRUE) == 0)
            || (strcmp(lpChildJson, JSON_PRIMITIVE_BOOL_FALSE) == 0))
        {
            pNewJsonObj->eObjectType = JsonResultTypeBoolean;
            pNewInternalObj->eObjectType = JsonResultTypeBoolean;
            pNewJsonObj->value.bBoolean = (strcmp(lpChildJson, JSON_PRIMITIVE_BOOL_TRUE) == 0);
        }
        // Otherwhise it should be a number
        else
        {
            pNewJsonObj->eObjectType = JsonResultTypeNumber;
            pNewInternalObj->eObjectType = JsonResultTypeNumber;
            pNewJsonObj->value.iNumber = atoi(lpChildJson);
        }
        break;
    case JSMN_OBJECT:
        pNewJsonObj->eObjectType = JsonResultTypeObject;
        pNewInternalObj->eObjectType = JsonResultTypeObject;
        break;
    case JSMN_ARRAY:
        pNewJsonObj->eObjectType = JsonResultTypeArray;
        pNewInternalObj->eObjectType = JsonResultTypeArray;
        break;
    case JSMN_STRING:
        UtilsHeapAllocWStrAndConvertAStr(gs_pJsonHeap, lpChildJson, &pNewJsonObj->value.ptStr);
        pNewJsonObj->eObjectType = JsonResultTypeString;
        pNewJsonObj->eObjectType = JsonResultTypeString;
        break;
    default:
        API_RETURN_ERROR(JSON_ERROR_INVALID_PARAMETER)
    }

    *ppJsonObj = pNewJsonObj;
    API_RETURN_SUCCESS();
}

BOOL JsoniBrowseJsonFile(
    _In_ const PJSON_INTERNAL_OBJECT    pJson,
    _In_opt_ const LPWSTR               lppwObjectNames[],
    _In_opt_ const DWORD                dwObjectNamesCount,
    _Inout_ pjsmntok_t                  *ppChildToken,
    _Inout_opt_ LPSTR                   *lppMatchedObjectName
    )
{
    BOOL bResult = ERROR_VALUE;
    LPSTR lpMatchedObjName = NULL;
    DWORD dwTokenBuffIdx = pJson->parsing_state.dwTokensArrayIndex;
    DWORD dwRemainingTokens = pJson->parsing_state.dwRemainingTokens;
    pjsmntok_t pChildToken = NULL;

    // Browse JSON structure until the end of the original token or the exhaustion of remaining token
    while (dwRemainingTokens > 0)
    {
        pjsmntok_t pCurrentToken = &pJson->jsmn.pAllocatedTokens[dwTokenBuffIdx];

        // When we find an object or an array, we need to increment remaining token to process
        if (pCurrentToken->type == JSMN_ARRAY || pCurrentToken->type == JSMN_OBJECT)
            dwRemainingTokens += pCurrentToken->size;

        switch (pJson->parsing_state.eCurrentParseState)
        {
        case START:
                // Verify file must have a root object
                if ((pCurrentToken->type != JSMN_OBJECT) && ((pCurrentToken->type != JSMN_ARRAY)))
                    API_RETURN_ERROR(JSON_ERROR_INVALID_JSON_FILE)

                // JSON file must have even number of children
                if ((pCurrentToken->type == JSMN_OBJECT) && (pCurrentToken->size % 2 != 0))
                    API_RETURN_ERROR(JSON_ERROR_INVALID_JSON_FILE)

                // Set the right next state
                if (pCurrentToken->size == 0)
                    pJson->parsing_state.eCurrentParseState = STOP;
                else if (pCurrentToken->type == JSMN_ARRAY)
                    pJson->parsing_state.eCurrentParseState = PRINT;
                else if (pCurrentToken->type == JSMN_OBJECT)
                    pJson->parsing_state.eCurrentParseState = KEY;
                else
                    API_RETURN_ERROR(JSON_ERROR_INVALID_JSON_FILE)

                dwRemainingTokens--;
                dwTokenBuffIdx++;
                break;
        case KEY:
                // Verify current key is a valid JSON key
                if (pCurrentToken->type != JSMN_STRING)
                    API_RETURN_ERROR(JSON_ERROR_INVALID_JSON_FILE)

                // By default, skip the token
                pJson->parsing_state.eCurrentParseState = SKIP;

                // If no object name provide, return first token encountered
                if (dwObjectNamesCount == 0)
                {
                    pJson->parsing_state.eCurrentParseState = PRINT;
                    bResult = JsoniTokenToStr(pJson->data.pvData, pCurrentToken, &lpMatchedObjName);
                    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

                }
                else // Compare current token with requested object names
                {
                    // Retrieve token only if it match demands
                    for (DWORD i = 0; i < dwObjectNamesCount; i++)
                    {
                        if (JsoniTokenStreq(pJson->data.pvData, pCurrentToken, lppwObjectNames[i]) == TRUE)
                        {
                            pJson->parsing_state.eCurrentParseState = PRINT;
                            bResult = JsoniTokenToStr(pJson->data.pvData, pCurrentToken, &lpMatchedObjName);
                            API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());
                        }
                    }
                }

                dwRemainingTokens--;
                dwTokenBuffIdx++;
                break;
        case SKIP:
                // Set the right next state
                if (pJson->parsing_state.dwRemainingTokens == 0)
                    pJson->parsing_state.eCurrentParseState = STOP;
                else if ((pJson->eObjectType == JsonResultTypeObject))
                    pJson->parsing_state.eCurrentParseState = KEY;
                else
                    pJson->parsing_state.eCurrentParseState = PRINT;

                dwRemainingTokens--;
                dwTokenBuffIdx++;

                // If we found a child array or a child object, we need to skip to attend next sibling element
                bResult = SkipJsonTokenIfNeeded(pJson, pCurrentToken, &dwTokenBuffIdx, &(dwRemainingTokens));
                API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP
                break;
        case PRINT:
                // Mark to token has found
                pChildToken = pCurrentToken;

                // Set the right next state
                if (pJson->parsing_state.dwRemainingTokens == 0)
                    pJson->parsing_state.eCurrentParseState = STOP;
                else if ((pJson->eObjectType == JsonResultTypeObject))
                    pJson->parsing_state.eCurrentParseState = KEY;
                else
                    pJson->parsing_state.eCurrentParseState = PRINT;

                dwRemainingTokens--;
                dwTokenBuffIdx++;

                // If we found a child array or a child object, we need to skip to attend next sibling element
                bResult = SkipJsonTokenIfNeeded(pJson, pChildToken, &dwTokenBuffIdx, &(dwRemainingTokens));
                API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR()); // TODO: WPP
                break;
        case STOP:
                // Just consume token
                dwRemainingTokens--;
                dwTokenBuffIdx++;
                break;
        case NOT_STARTED:
        default:
            API_RETURN_ERROR(JSON_ERROR_INVALID_PARSER_STATE)
        }

        // End the crawling loop when we have found a matching token
        if (pChildToken != NULL)
            break;
    }

    // If we do not have found the token, return corresponding error
    if (pChildToken == NULL)
        API_RETURN_ERROR(JSON_ERROR_TOKEN_NOT_FOUND)

    pJson->parsing_state.dwTokensArrayIndex = dwTokenBuffIdx;
    pJson->parsing_state.dwRemainingTokens = dwRemainingTokens;
    *ppChildToken = pChildToken;
    if (lppMatchedObjectName) *lppMatchedObjectName = lpMatchedObjName;
    API_RETURN_SUCCESS();
}

