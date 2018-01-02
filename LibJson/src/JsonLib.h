/******************************************************************************\
Author : L. Delsalle
Date   : 26/01/2015
Descr. : Lightweight JSON Library written in C Windows
         based on jsmn project (cf. http://zserge.com/jsmn.html).
         For now, this library only support READ operations.
\******************************************************************************/

#ifndef __JSON_LIB_H__
#define __JSON_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- INCLUDES ------------------------------------------------------------- */
#include <assert.h>
#include "LibUtils\src\UtilsLib.h"

#ifdef VLD_DBG
    #define VLD_FORCE_ENABLE
    #include <vld.h>
    #pragma comment(lib,"vld.lib")
#endif

#undef DLL_FCT
#ifdef DLL_MODE
    #ifdef __JSON_INT_H__
        #define DLL_FCT __declspec(dllexport)
    #else
        #define DLL_FCT __declspec(dllimport)
    #endif
#else
    #define DLL_FCT
#endif

/* --- DEFINES -------------------------------------------------------------- */
//
// Custom LibJson error codes
//
#define JSON_LIBERR                                     (0xEEEE)
#define JSON_ERR_UNKNOWN_TODO                           ERROR_CODE(JSON_LIBERR, 1)
#define JSON_ERROR_INVALID_HANDLE                       ERROR_CODE(JSON_LIBERR, 2)
#define JSON_ERROR_INVALID_OPERATION                    ERROR_CODE(JSON_LIBERR, 3)
#define JSON_ERROR_INVALID_PARAMETER                    ERROR_CODE(JSON_LIBERR, 4)
#define JSON_ERROR_INVALID_JSON_FILE                    ERROR_CODE(JSON_LIBERR, 5)
#define JSON_ERROR_TRUNCATED_JSON_FILE                  ERROR_CODE(JSON_LIBERR, 6)
#define JSON_ERROR_INVALID_PARSER_STATE                 ERROR_CODE(JSON_LIBERR, 7)
#define JSON_ERROR_MEMORY_OPERATION                     ERROR_CODE(JSON_LIBERR, 8)
#define JSON_ERROR_END_OF_FILE                          ERROR_CODE(JSON_LIBERR, 9)
#define JSON_ERROR_TOKEN_NOT_FOUND                      ERROR_CODE(JSON_LIBERR, 10)
#define JSON_ERROR_TOKEN_NOT_AN_OBJECT                  ERROR_CODE(JSON_LIBERR, 11)
#define JSON_ERROR_TOKEN_NOT_AN_ARRAY                   ERROR_CODE(JSON_LIBERR, 12)
#define JSON_ERROR_INVALID_TOKEN                        ERROR_CODE(JSON_LIBERR, 13)
#define JSON_ERROR_INVALID_OBJECT_TYPE                  ERROR_CODE(JSON_LIBERR, 14)

#define JSON_ERROR_FEATURE_NOT_IMPLEMENTED              ERROR_CODE(JSON_LIBERR, 99)

//
// Data access
//
#define JSON_CHECK_TYPE_RETURN_VAL(j,type,val)          (assert((j)->eObjectType == type), (j)->value.val)
#define JSON_STRVAL(j)                                  JSON_CHECK_TYPE_RETURN_VAL(j, JsonResultTypeString, ptStr)
#define JSON_ARRVAL(j)                                  JSON_CHECK_TYPE_RETURN_VAL(j, JsonResultTypeArray, lpArray)
#define JSON_OBJVAL(j)                                  JSON_CHECK_TYPE_RETURN_VAL(j, JsonResultTypeObject, lpObject)
#define JSON_INTVAL(j)                                  JSON_CHECK_TYPE_RETURN_VAL(j, JsonResultTypeNumber, iNumber)
#define JSON_BOOLVAL(j)                                 JSON_CHECK_TYPE_RETURN_VAL(j, JsonResultTypeBoolean, bBoolean)

//
// Misc.
//
#define JSON_INVALID_HANDLE_VALUE                       ((JSON_HANDLE)-1)

/* --- TYPES ---------------------------------------------------------------- */
typedef DWORD JSON_HANDLE;
typedef JSON_HANDLE *PJSON_HANDLE;

typedef enum _JSON_OBJECT_TYPE {
                                                        JsonResultTypeUnknown   = 0,
                                                        JsonResultTypeNumber    = 1,
                                                        JsonResultTypeObject    = 2,
                                                        JsonResultTypeArray     = 3,
                                                        JsonResultTypeString    = 4,
                                                        JsonResultTypeBoolean   = 5
} JSON_OBJECT_TYPE, *PJSON_OBJECT_TYPE;

typedef struct _JSON_OBJECT {
    JSON_OBJECT_TYPE                                    eObjectType;

    PTCHAR                                              ptKey;

    union {
        PTCHAR                                          ptStr;
        LPVOID                                          lpArray;
        LPVOID                                          lpObject;
        INT                                             iNumber;
        BOOL                                            bBoolean;
    } value;

    struct {
        JSON_HANDLE                                     hJson;
    } privatestruct;

} JSON_OBJECT, *PJSON_OBJECT;

typedef BOOL (FN_JSON_ELEMENT_CALLBACK)(
    _In_ const PJSON_OBJECT                             pJsonElement,
    _In_opt_ const PVOID                                pvContext
    );
typedef FN_JSON_ELEMENT_CALLBACK *PFN_JSON_ELEMENT_CALLBACK;

typedef struct _JSON_REQUESTED_ELEMENT {
    PTCHAR                                              ptKey;
    JSON_OBJECT_TYPE                                    eExpectedType;
    PFN_JSON_ELEMENT_CALLBACK                           pfnCallback;
    BOOL                                                bMustBePresent;
} JSON_REQUESTED_ELEMENT, *PJSON_REQUESTED_ELEMENT;

/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */
//
// Init/Cleanup
//
DLL_FCT
BOOL
JsonLibInit(
);

DLL_FCT
BOOL
JsonLibCleanup(
);

//
// Open operations
//
DLL_FCT BOOL JsonOpenFileReadW(
    _In_ const LPWSTR                                   lpwJsonFilename,
    _Out_ PJSON_OBJECT                                  *ppJsonObj
    );

DLL_FCT BOOL JsonOpenFileReadA(
    _In_ const LPSTR                                    lpJsonFilename,
    _Out_ PJSON_OBJECT                                  *ppJsonObj
    );

#ifdef UNICODE
#define JsonOpenFileRead                                JsonOpenFileReadW
#else
#define JsonOpenFileRead                                JsonOpenFileReadA
#endif // !UNICODE

DLL_FCT BOOL JsonParseStringReadW(
    _In_ const LPWSTR                                   lpwJsonString,
    _Out_ PJSON_OBJECT                                  *ppJsonObj
    );

DLL_FCT BOOL JsonParseStringReadA(
    _In_ const LPSTR                                    lpJsonString,
    _Out_ PJSON_OBJECT                                  *ppJsonObj
    );

#ifdef UNICODE
#define JsonParseStringRead                             JsonParseStringReadW
#else
#define JsonParseStringRead                             JsonParseStringReadA
#endif // !UNICODE

//
// Read operations
//

DLL_FCT BOOL JsonGetNextObjectW(
    _In_ const PJSON_OBJECT                             pJsonObj,
    _In_opt_ LPWSTR                                     lppwObjectNames[],
    _In_opt_ DWORD                                      dwObjectNamesCount,
    _Inout_ PJSON_OBJECT                                *ppJsonResultObj
    );

DLL_FCT BOOL JsonGetNextObjectA(
    _In_ const PJSON_OBJECT                             pJsonObj,
    _In_opt_ LPSTR                                      lppObjectNames[],
    _In_opt_ DWORD                                      dwObjectNamesCount,
    _Inout_ PJSON_OBJECT                                *ppJsonResultObj
    );

#ifdef UNICODE
#define JsonGetNextObject                               JsonGetNextObjectW
#else
#define JsonGetNextObject                               JsonGetNextObjectA
#endif // !UNICODE

DLL_FCT BOOL JsonRestartParsing(
    _In_ const PJSON_OBJECT                             pJsonObj
    );

DLL_FCT BOOL JsonObjectForeachRequestedElement(
    _In_ const PJSON_OBJECT                             pJsonObj,
    _In_ const BOOL                                     bAllowUnknownElements,
    _In_ const JSON_REQUESTED_ELEMENT                   pRequestedElements[],
    _In_ const DWORD                                    dwRequestedElementsCount,
    _In_opt_ const PVOID                                pvContext,
    _Out_opt_ PDWORD                                    pdwElmtCount
    );

DLL_FCT BOOL JsonForeachElement(
    _In_ const PJSON_OBJECT                             pJsonObj,
    _In_ const PFN_JSON_ELEMENT_CALLBACK                pfnCallback,
    _In_opt_ const PVOID                                pvContext,
    _Out_opt_ PDWORD                                    pdwElmtCount
    );

//
// Both Read/Write operations
//
DLL_FCT BOOL JsonReleaseObject(
    _Inout_ PJSON_OBJECT                                *ppJsonObj
    );

//
// Other
//
DLL_FCT DWORD JsonGetLastError(
    );

#ifdef __cplusplus
}
#endif

#endif // __JSON_LIB_H__