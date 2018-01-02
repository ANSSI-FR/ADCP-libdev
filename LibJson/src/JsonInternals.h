/******************************************************************************\
Author : L. Delsalle
Date   : 26/01/2015
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __JSON_INT_H__
#define __JSON_INT_H__

/* --- INCLUDES ------------------------------------------------------------- */
#include "JsonLib.h"

/* --- DEFINES -------------------------------------------------------------- */
//
// Misc
//
#define JSON_HANDLE_TABLE_DELTA         (0x10)
#define JSON_HEAP_NAME                  _T("CSVLIB")

//
// JSON primitive values
//
#define JSON_PRIMITIVE_BOOL_TRUE        "true"
#define JSON_PRIMITIVE_BOOL_FALSE       "false"

//
// Default number of json Token allocated (use for performance only)
//
#define JSON_DEFAULT_TOKEN_COUNT        128

/* --- JSMN TYPES ----------------------------------------------------------- */
/**
* JSON type identifier. Basic types are:
*  - Object
*  - Array
*  - String
*  - Other primitive: number, boolean (true/false) or null
*/
typedef enum _jsmntype_t {
                                        JSMN_PRIMITIVE = 0,
                                        JSMN_OBJECT = 1,
                                        JSMN_ARRAY = 2,
                                        JSMN_STRING = 3
} jsmntype_t, *pjsmntype_t;

typedef enum _parse_state {
                                        START = 0,
                                        KEY = 1,
                                        PRINT = 2,
                                        SKIP = 3,
                                        STOP = 4,
                                        NOT_STARTED = 5
} parse_state, *pparse_state;

typedef enum _jsmnerr_t {
                                        /* Not enough tokens were provided */
                                        JSMN_ERROR_NOMEM = -1,
                                        /* Invalid character inside JSON string */
                                        JSMN_ERROR_INVAL = -2,
                                        /* The string is not a full JSON packet, more bytes expected */
                                        JSMN_ERROR_PART = -3,
                                        /* Everything was fine */
                                        JSMN_SUCCESS = 0
} jsmnerr_t, *pjsmnerr_t;

/**
* JSON token description.
* @param    type     type (object, array, string etc.)
* @param    start    start position in JSON data string
* @param    end      end position in JSON data string
*/
typedef struct _jsmntok_t {
    jsmntype_t                          type;
    INT                                 start;
    INT                                 end;
    INT                                 size;
} jsmntok_t, *pjsmntok_t;

/**
* JSON parser. Contains an array of token blocks available. Also stores
* the string being parsed now and current position in that string
*/
typedef struct _jsmn_parser{
    UINT                                pos; /* offset in the JSON string */
    INT                                 toknext; /* next token to allocate */
    INT                                 toksuper; /* suporior token node, e.g parent object or array */
} jsmn_parser, *pjsmn_parser;

/* --- TYPES ---------------------------------------------------------------- */
typedef enum _JSON_OPERATION {
                                        JsonFileOperationInvalid,
                                        JsonFileOperationRead,
                                        JsonFileOperationWrite,
                                        JsonFileOperationAppend
} JSON_OPERATION, *PJSON_OPERATION;

typedef struct _JSON_INTERNAL_OBJECT {
    JSON_OBJECT_TYPE                    eObjectType;
    JSON_OPERATION                      eJsonOperation;

    struct {
        LPWSTR                          ptFileName;
        HANDLE                          hFileHandle;
        HANDLE                          hFileMapping;
    } file;

    struct {
        LONGLONG                        llDataSize;
        BOOL                            bIsWCharData;
        LPVOID                          pvData;
    } data;

    struct {
        jsmn_parser                     sParser;
        pjsmntok_t                      pAllocatedTokens;
        DWORD                           dwAllocatedTokens;
    } jsmn;

    struct {
        DWORD                           dwTokensArrayIndex;
        DWORD                           dwRemainingTokens;
        parse_state                     eCurrentParseState;
    } parsing_state;

} JSON_INTERNAL_OBJECT, *PJSON_INTERNAL_OBJECT;

typedef struct _JSON_HANDLE_TABLE {
    DWORD                               dwTableSize;
    DWORD                               dwEntryCount;
    PJSON_INTERNAL_OBJECT               *pEntries;
} JSON_HANDLE_TABLE, *P_JSON_HANDLE_TABLE;

/* --- VARIABLES ------------------------------------------------------------ */
// Global variable for error code
extern PUTILS_HEAP                      gs_pJsonHeap;
extern DWORD                            gs_dwJsonLastError;
extern JSON_HANDLE_TABLE                gs_sJsonHandleTable;

/* --- PRIVATE PROTOTYPES --------------------------------------------------- */
/**
* Create JSON parser over an array of tokens
*/
VOID jsmn_init(
    _Inout_ pjsmn_parser                parser
    );

/**
* Run JSON parser. It parses a JSON data string into and array of tokens, each describing
* a single JSON object.
*/
jsmnerr_t jsmn_parse(
    _In_ pjsmn_parser                   parser,
    _In_ const PCHAR                    js,
    _Inout_ pjsmntok_t                  tokens,
    _Inout_ INT                         num_tokens
    );

/* --- PROTOTYPES ----------------------------------------------------------- */
BOOL JsoniOpenFile(
    _In_ const LPWSTR                   lpwFilename,
    _In_ const JSON_OPERATION           eFileOperation,
    _Inout_ PJSON_INTERNAL_OBJECT       pJson
    );

BOOL JsoniCreateFileMapping(
    _Inout_ PJSON_INTERNAL_OBJECT       pJson
    );

BOOL JsoniNewInternalObject(
    _Out_ PJSON_INTERNAL_OBJECT         *ppObject,
    _Out_ PJSON_HANDLE                  pJsonHandle
    );

BOOL JsoniGetObjectByHandle(
    _In_ const JSON_HANDLE              hHandle,
    _Out_ PJSON_INTERNAL_OBJECT         *ppObject
    );

BOOL JsoniParseJS(
    _In_ const PJSON_INTERNAL_OBJECT    pJson
    );

BOOL JsoniReleaseInternalObject(
    _In_ const JSON_HANDLE              hHandle
    );

BOOL JsoniTokenStreq(
    _In_ const LPSTR                    lpJS,
    _In_ const pjsmntok_t               pJsmnToken,
    _In_ const LPWSTR                   lpwMatchingKey
    );

BOOL JsoniTokenToStr(
    _In_ const LPSTR                    lpJS,
    _In_ const pjsmntok_t               pJsmnToken,
    _Out_ LPSTR                        *lppStr
    );

BOOL JsoniAllocAndFillJsonObj(
    _In_opt_ LPSTR                      lpParentKey,
    _In_opt_ pjsmntok_t                 pChildToken,
    _In_ PJSON_INTERNAL_OBJECT          pJsonInternalObj,
    _In_opt_ PJSON_OBJECT               *ppJsonObj
    );

BOOL JsoniBrowseJsonFile(
    _In_ const PJSON_INTERNAL_OBJECT    pJson,
    _In_opt_ const LPWSTR               lppwObjectNames[],
    _In_opt_ const DWORD                dwObjectNamesCount,
    _Inout_ pjsmntok_t                  *ppChildToken,
    _Inout_opt_ LPSTR                   *lppMatchedObjectName
    );

#endif // __JSON_INT_H__