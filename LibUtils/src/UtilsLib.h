/******************************************************************************\
Author : Lucas. BOUILLOT
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __UTILS_LIB_H__
#define __UTILS_LIB_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- INCLUDES ------------------------------------------------------------- */
#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

/* --- DEFINES -------------------------------------------------------------- */
#ifdef VLD_DBG
    #define VLD_FORCE_ENABLE
    #include <vld.h>
    #pragma comment(lib,"vld.lib")
#endif

#undef DLL_FCT
#ifdef DLL_MODE
    #ifdef __UTILS_INT_H__
        #define DLL_FCT __declspec(dllexport)
    #else
        #define DLL_FCT __declspec(dllimport)
    #endif
#else
    #define DLL_FCT
#endif
#define DLL_VAR DLL_FCT

//
// There is not a single getopt supporting all of the following :
//      - a "long only" options
//      - a "optreset" var
//      - a TCHAR version
// Therefore, you can use the following #define to choose the getopt you need :
//      - UTILS_REQUIRE_GETOPT_SIMPLE   => simplest one, supporting optreset. Chosen by default.
//      - UTILS_REQUIRE_GETOPT_COMPLEX  => getopt using TCHAR and supporting a long only mode
//
#if defined(UTILS_REQUIRE_GETOPT_SIMPLE) && defined(UTILS_REQUIRE_GETOPT_COMPLEX)
    #error Only one option must be specified between UTILS_REQUIRE_GETOPT_SIMPLE and UTILS_REQUIRE_GETOPT_COMPLEX
#elif !defined(UTILS_REQUIRE_GETOPT_SIMPLE) && !defined(UTILS_REQUIRE_GETOPT_COMPLEX)
    #define UTILS_REQUIRE_GETOPT_SIMPLE
#endif
#if defined(UTILS_REQUIRE_GETOPT_SIMPLE)
    #include "UtilsGetoptSimple.h"
#elif defined(UTILS_REQUIRE_GETOPT_COMPLEX)
    #include "UtilsGetoptComplex.h"
#endif

//
// Custom 'LibUtils' error codes
//
#define UTILS_LIBERR                (0xBBBB)
#define UTILS_ERR_UNKNOWN_TODO      ERROR_CODE(UTILS_LIBERR, 1)

//
// 'Mutli-line' macro helpers
//
#define MULTI_LINE_MACRO_BEGIN              do {
#define MULTI_LINE_MACRO_END                                                \
                                            __pragma(warning(push))         \
                                            __pragma(warning(disable:4127)) \
                                            } while (0)                     \
                                            __pragma(warning(pop))

//
// 'Return' macros
//
#ifndef LIB_ERROR_VAL
#error LIB_ERROR_VAL must be defined
#endif
#define ERROR_CODE(lib, code)               (((DWORD)(lib) << 16) | (code)) // lib codes and error codes are one 16 bits max
#define SUCCESS_VALUE                       (TRUE)
#define ERROR_VALUE                         (FALSE)
#define API_SUCCEEDED(bReturn)              ((bReturn) == (SUCCESS_VALUE))
#define API_FAILED(bReturn)                 ((bReturn) != (SUCCESS_VALUE))
#define API_RETURN_SUCCESS()                return LIB_ERROR_VAL = (NO_ERROR), (SUCCESS_VALUE);
#define API_RETURN_ERROR(dwErrorCode)       return LIB_ERROR_VAL = (dwErrorCode), (ERROR_VALUE);
#define API_RETURN_SAME_ERROR()             return (ERROR_VALUE);
#define API_RETURN_SAME_VALUE(bValue)       return (bValue);
#define SAME_ERROR()                        (LIB_ERROR_VAL)
#define API_RETURN_ERROR_IF_FAILED(bValue, dwErr)   MULTI_LINE_MACRO_BEGIN      \
                                                if (API_FAILED(bValue)) {       \
                                                    API_RETURN_ERROR(dwErr);    \
                                            }                                   \
                                            MULTI_LINE_MACRO_END

// TODO!: would be better with a clean "fatal callback"
#define FATAL_ERROR(frmt, ...)              MULTI_LINE_MACRO_BEGIN                      \
                                                _ftprintf(stderr, frmt, __VA_ARGS__);   \
                                                int c = getchar(); (void)c;     \
                                                ExitProcess(EXIT_FAILURE);              \
                                            MULTI_LINE_MACRO_END
#define FATAL_ERROR_IF(cond, frmt, ...)     MULTI_LINE_MACRO_BEGIN                                  \
                                                if (cond) {                                         \
                                                    FATAL_ERROR(frmt ## _T("\r\n"), __VA_ARGS__);   \
                                                }                                                   \
                                            MULTI_LINE_MACRO_END
#define LABEL_FAIL                          label_fail
#define GOTO_FAIL(code)                     MULTI_LINE_MACRO_BEGIN  \
                                                code;               \
                                                goto LABEL_FAIL;    \
                                            MULTI_LINE_MACRO_END
#define GOTO_FAIL_IF(condition, code)       MULTI_LINE_MACRO_BEGIN      \
                                                if (condition) {        \
                                                    GOTO_FAIL(code);    \
                                                }                       \
                                            MULTI_LINE_MACRO_END

//
// 'Str' macros helpers
//
#define STR(x)                      _T(#x)
#define CHR(x)                      _T(#@x)
#define CONCAT(a,b)                 a ## b
#define NONE(x)                     x
#define NULL_CHAR                   _T('\0')
#define TAB_CHAR                    _T('\t')
#define EMPTY_STR                   _T("")
#define STR_EMPTY(s)                ((BOOL)((s) == NULL || (s)[0] == NULL_CHAR))
#define STRLEN_IFNOTNULL(a)            (a ? _tcslen(a) : (size_t)0)
#define STR_EQ(a,b)                 ((BOOL)(_tcscmp(a,b) == 0))
#define STR_EQI(a,b)                ((BOOL)(_tcsicmp(a,b) == 0))
#define STR_EQN(a,b,n)              ((BOOL)(_tcsncmp(a,b,n) == 0))
#define SEPARATOR_LINE              _T("------------------------------")

//
// 'Mem' macros helpers
//
#define BAD_POINTER                 ((PVOID)-1)
#define BAD(p)                      ((p) == BAD_POINTER)
#define NULL_IF_BAD(p)              (BAD(p) ? NULL: (p))

//
// 'Bitmap' macros
//
#define DECLARE_BITMAP(name, count) BYTE name[ ROUND_UP(count, BITS_IN_BYTE) / BITS_IN_BYTE ]
#define BITMAP_SET_BIT(bmp, idx)    (bmp)[ idx / BITS_IN_BYTE ] |= (1 << (idx % BITS_IN_BYTE))
#define BITMAP_GET_BIT(bmp, idx)    (((bmp)[ idx / BITS_IN_BYTE ] & (1 << (idx % BITS_IN_BYTE))) >> (idx % BITS_IN_BYTE))

//
// 'Bits' macros
//
#define BIT(i)                      (1 << (i))
#define SET_BIT(dw, i)              (dw) |= BIT(i)
#define CLR_BIT(dw, i)              (dw) &= ~(BIT(i))
#define HAS_BIT(dw, i)              (((dw) & BIT(i)) > 0)

//
// Guid & Sid & SD macros
//
#define GUID_EQ(pg1, pg2)           (memcmp((PVOID)pg1, (PVOID)pg2, sizeof(GUID)) == 0)
#define GUID_EMPTY(pg)              (MemoryIsNull((PBYTE)(pg), sizeof(GUID)))
#define SID_EMPTY(ps, len)          (MemoryIsNull((PBYTE)(ps), len ? len: GetLengthSid(ps)))

//
// Sizes
//
#define MAX_LINE                    (4096)
#define BITS_IN_BYTE                (8)
#define STR_GUID_LEN                (36)
#define GUID_STR_SIZE               (8 + 2*4 + 8*2 + 4*1 + 1 + 1)   // hexified: 1*dword + 2*short + 8*char + 4*'-' + '{' + '}'
#define GUID_STR_SIZE_NULL          (GUID_STR_SIZE + 1)             // + null
#define SIZEOF_ARRAY(s, n)          (sizeof(s)*(n))
#define SIZEOF_TSTR(len)            (SIZEOF_ARRAY(TCHAR, ((len) + 1)))
#define ARRAY_COUNT(x)              (sizeof(x) / sizeof((x)[0]))

//
// 'Misc' macros (TODO: organize!)
//
#define BIT(i)                          (1 << (i))
#define ROUND_UP(N, S)                  ((((N) + (S) - 1) / (S)) * (S))
#define PERCENT(count, total)           (total ? ( (((float)(count)) * 100.0) / total ): 0)
#define TIME_DIFF_SEC(start, stop)      (((stop) - (start)) / 1000.0)
#define GLE()                           GetLastError()
#define SET_PTRVAL_IF_NOT_NULL(p, v)    MULTI_LINE_MACRO_BEGIN  \
                                            if ((p) != NULL) {  \
                                                (*p) = v;       \
                                            }                   \
                                        MULTI_LINE_MACRO_END

//
// 'Alloc' helpers
//
#define UtilsHeapAllocHelper(h, n)              UtilsHeapAlloc(_T(__FUNCTION__), h, n)
#define UtilsHeapReallocHelper(h, p, n)         UtilsHeapRealloc(_T(__FUNCTION__), h, p, n)
#define UtilsHeapStrDupHelper(h, s)             UtilsHeapStrDup(_T(__FUNCTION__), h, s)
#define UtilsHeapMemDupHelper(h, m, n)          UtilsHeapMemDup(_T(__FUNCTION__), h, m, n)
#define UtilsHeapFreeHelper(h, p)               UtilsHeapFree(_T(__FUNCTION__), h, p)
#define UtilsHeapFreeArrayHelper(h, p, c)       UtilsHeapFreeArray(_T(__FUNCTION__), h, p, c)

#define UtilsHeapAllocStrHelper(h, c)           UtilsHeapAllocHelper(h, SIZEOF_TSTR(c))
#define UtilsHeapAllocStructHelper(h, s)        UtilsHeapAllocHelper(h, sizeof(s))
#define UtilsHeapAllocArrayHelper(h, s, n)      UtilsHeapAllocHelper(h, SIZEOF_ARRAY(s,n))
#define UtilsHeapStructDupHelper(h, m, s)       UtilsHeapMemDupHelper(h, m, sizeof(s))
#define UtilsHeapReallocArrayHelper(h, p, s, n) UtilsHeapReallocHelper(h, p, SIZEOF_ARRAY(s,n))
#define UtilsHeapAllocOrReallocHelper(h, p, n)  ((p) == NULL) ? UtilsHeapAllocHelper(h, n) : UtilsHeapReallocHelper(h, p, n);
#define UtilsHeapFreeAndNullHelper(h, p)        MULTI_LINE_MACRO_BEGIN              \
                                                    if ((p) != NULL) {              \
                                                        UtilsHeapFreeHelper(h, p);  \
                                                        (p) = NULL;                 \
                                                    }                               \
                                                MULTI_LINE_MACRO_END
#define UtilsHeapFreeAndNullArrayHelper(h, a, c, i) MULTI_LINE_MACRO_BEGIN                          \
                                                        for (i = 0; i < (c); i++) {                 \
                                                            UtilsHeapFreeAndNullHelper(h, a[i]);    \
                                                        }                                           \
                                                        UtilsHeapFreeAndNullHelper(h, a);           \
                                                        (c) = 0;                                    \
                                                    MULTI_LINE_MACRO_END
#define UtilsHeapAllocWStrAndConvertAStr(h, a, w) ConvertAstrToWstr(h, a, w)
#define UtilsHeapAllocAStrAndConvertWStr(h, w, a) ConvertWstrToAstr(h, w, a)
#define UtilsHeapAllocWStrArrAndConvertAStrArr(h, a, c, w) ConvertAstrArrayToWstrArray(h, a, c, w)
#define UtilsHeapAllocAStrArrAndConvertWStrArr(h, w, c, a) ConvertWstrArrayToAstrArray(h, w, c, a)

//
// Unicode
//
#ifdef UNICODE
    #define Hexify      HexifyW
    #define Unhexify    UnhexifyW
    #define IsNumeric   IsNumericW
#else
    #define Hexify      HexifyA
    #define Unhexify    UnhexifyA
    #define IsNumeric   IsNumericA
#endif

/* --- TYPES ---------------------------------------------------------------- */
typedef void (FN_UTILS_HEAP_FATAL_ERROR_HANDLER)(
    );
typedef FN_UTILS_HEAP_FATAL_ERROR_HANDLER *PFN_UTILS_HEAP_FATAL_ERROR_HANDLER;

typedef struct _UTILS_HEAP {
    HANDLE hHeap;
    PTCHAR ptName;
    PFN_UTILS_HEAP_FATAL_ERROR_HANDLER pfnFatalErrorHandler;
} UTILS_HEAP, *PUTILS_HEAP;

typedef struct _NUMERIC_CONSTANT {
    LPTSTR name;
    DWORD value;
} NUMERIC_CONSTANT, *PNUMERIC_CONSTANT;

typedef struct _STR_PAIR_LIST {
    struct _STR_PAIR_LIST * next;
    LPTSTR name;
    LPTSTR val;
} STR_PAIR_LIST, *PSTR_PAIR_LIST;

/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */
//
// Init/Cleanup
//
DLL_FCT
BOOL
UtilsLibInit(
);

DLL_FCT
BOOL
UtilsLibCleanup(
);

//
// Allocation
//
DLL_FCT BOOL UtilsHeapCreate(
    _Out_ PUTILS_HEAP *ppHeap,
    _In_ const PTCHAR ptName,
    _In_opt_ const PFN_UTILS_HEAP_FATAL_ERROR_HANDLER pfnFatalErrorHandler
    );

DLL_FCT BOOL UtilsHeapDestroy(
    _Inout_ PUTILS_HEAP *ppHeap
    );

DLL_FCT _Ret_notnull_ PVOID UtilsHeapAlloc(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const DWORD dwSize
    );

DLL_FCT _Ret_notnull_ PVOID UtilsHeapRealloc(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PVOID pvMem,
    _In_ const DWORD dwNewSize
    );

DLL_FCT _Ret_notnull_ PVOID UtilsHeapAllocOrRealloc(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_opt_ const PVOID pvMem,
    _In_ const DWORD dwNewSize
    );

DLL_FCT _Ret_notnull_ PVOID UtilsHeapStrDup(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PTCHAR ptStr
    );

DLL_FCT _Ret_notnull_ PVOID UtilsHeapMemDup(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PVOID pvStruct,
    _In_ const DWORD dwSize
    );

DLL_FCT void UtilsHeapFree(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PVOID pMem
    );

DLL_FCT void UtilsHeapFreeArray(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PVOID *ppMemArr,
    _In_ const DWORD dwCount
    );

//
// String
//
DLL_FCT void HexifyA(
    _In_ const LPSTR ptOutHexStr,
    _In_ const PBYTE pbInData,
    _In_ const DWORD dwLen // ptOutHexStr must be able to receive dwLen *2 chars + null
    );

DLL_FCT void HexifyW(
    _In_ const LPWSTR ptOutHexStr,
    _In_ const PBYTE pbInData,
    _In_ const DWORD dwLen // ptOutHexStr must be able to receive dwLen *2 chars + null
);

DLL_FCT void UnhexifyA(
    _In_ const PBYTE pbOutData,
    _In_ const LPSTR ptInStr // pbOutData must be able to receive strlen(ptInStr)/2 bytes
    );

DLL_FCT void UnhexifyW(
    _In_ const PBYTE pbOutData,
    _In_ const LPWSTR ptInStr // pbOutData must be able to receive strlen(ptInStr)/2 bytes
    );

DLL_FCT BOOL IsNumericA(
    _In_ const LPCSTR ptStr
    );

DLL_FCT BOOL IsNumericW(
    _In_ const LPCWSTR ptStr
    );

DLL_FCT BOOL IsInSetOfStrings(
    _In_ const PTCHAR ptNeedle,
    _In_ const PTCHAR ptHaystack[],
    _In_ const DWORD dwSetSize,
    _Out_opt_ DWORD *pdwIndex
    );

DLL_FCT BOOL StrNextToken(
    _In_ const PTCHAR ptStr,
    _In_ const PTCHAR ptDelim,
    _Inout_ PTCHAR *pptCtx,
    _Out_ PTCHAR *pptTok
    );

DLL_FCT BOOL ConvertAstrToWstr(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const LPCSTR pAnsiStr,
    _Inout_ LPWSTR *ppOutputWstr
    );

DLL_FCT BOOL ConvertWstrToAstr(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const LPCWSTR lpwStr,
    _Inout_ LPSTR *ppOutputAstr
    );

DLL_FCT BOOL ConvertAstrArrayToWstrArray(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const LPCSTR pAnsiStr[],
    _In_ const DWORD dwStrCount,
    _Inout_ LPWSTR *ppOutputWstr[]
    );

DLL_FCT BOOL ConvertWstrArrayToAstrArray(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const LPCWSTR pWideStr[],
    _In_ const DWORD dwStrCount,
    _Inout_ LPSTR *ppOutputStr[]
    );

//
// Privileges
//
DLL_FCT BOOL SetPrivilege(
    _In_ const HANDLE hToken,
    _In_ const PTCHAR ptPriv,
    _In_ const BOOL bEnablePrivilege
    );

DLL_FCT BOOL EnablePrivForCurrentProcess(
    _In_ const PTCHAR ptPriv
    );

DLL_FCT HANDLE FileOpenWithBackupPriv(
    _In_ PTCHAR ptPath,
    _In_ BOOL bUseBackupPriv
    );

//
// Memory
//
DLL_FCT BOOL MemoryIsNull(
    _In_ const PBYTE pbMem,
    _In_ const DWORD dwLen
    );

//
// StrPair
//
DLL_FCT void AddStrPair(
    _In_ const PUTILS_HEAP pHeap,
    _Inout_ PSTR_PAIR_LIST *ppEnd,
    _In_ const PTCHAR ptName,
    _In_ const PTCHAR ptValue
    );

DLL_FCT PTCHAR GetStrPair(
    _In_ const PSTR_PAIR_LIST pHead,
    _In_ PTCHAR ptName
    );

DLL_FCT void DestroyStrPairList(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PSTR_PAIR_LIST pHead
    );

//
// Misc/Others
//
DLL_FCT DWORD UtilsGetLastError(
    );

DLL_FCT BOOL IsDomainSid(
    _In_ const PSID pSid
    );

DLL_FCT BOOL ConvertStrGuidToGuid(
    _In_ const PTCHAR ptGuid,
    _In_ GUID * pGuid
    );

DLL_FCT DWORD GenerateException(
    );

#ifdef __cplusplus
}
#endif

#endif // __UTILS_LIB_H__
