/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/


#ifndef __LDAP_HELPERS_H__
#define __LDAP_HELPERS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- INCLUDES ------------------------------------------------------------- */
#include "LdapLib.h"


/* --- DEFINES -------------------------------------------------------------- */
/* --- TYPES ---------------------------------------------------------------- */
/* --- VARIABLES ------------------------------------------------------------ */
extern DWORD gs_dwLastError;

/* --- PROTOTYPES ----------------------------------------------------------- */
DLL_FCT BOOL LdapLocateNamedAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_ PDWORD pdwIndex
    );

DLL_FCT BOOL LdapLocateNamedAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_ PDWORD pdwIndex
    );

#ifdef UNICODE
#define LdapLocateNamedAttr  LdapLocateNamedAttrW
#else
#define LdapLocateNamedAttr  LdapLocateNamedAttrA
#endif // !UNICODE

DLL_FCT BOOL LdapExtractStrAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pStrAttr,
    _Out_writes_(dwStrCount) LPWSTR pptStr[],
    _In_ DWORD dwStrCount
    );

DLL_FCT BOOL LdapExtractStrAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pStrAttr,
    _Out_writes_(dwStrCount) LPSTR pptStr[],
    _In_ DWORD dwStrCount
    );

#ifdef UNICODE
#define LdapExtractStrAttr  LdapExtractStrAttrW
#else
#define LdapExtractStrAttr  LdapExtractStrAttrA
#endif // !UNICODE

DLL_FCT BOOL LdapExtractNumAttr(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pNumAttr,
    _Out_writes_(dwNumCount) PDWORD pdwNum,
    _In_ DWORD dwNumCount
    );

DLL_FCT BOOL LdapExtractBoolAttr(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pBoolAttr,
    _Out_writes_(dwBoolCount) PBOOL pbBool,
    _In_ DWORD dwBoolCount
    );

DLL_FCT BOOL LdapExtractNamedStrAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_writes_(dwStrCount) LPWSTR pptStr[],
    _In_ DWORD dwStrCount
    );

DLL_FCT BOOL LdapExtractNamedStrAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_writes_(dwStrCount) LPSTR pptStr[],
    _In_ DWORD dwStrCount
    );

#ifdef UNICODE
#define LdapExtractNamedStrAttr  LdapExtractNamedStrAttrW
#else
#define LdapExtractNamedStrAttr  LdapExtractNamedStrAttrA
#endif // !UNICODE

DLL_FCT BOOL LdapAllocAndExtractNamedStrAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_ LPWSTR *ppptStr[],
    _Out_ PDWORD pdwStrCount
    );

DLL_FCT BOOL LdapExtractNamedNumAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_writes_(dwNumCount) PDWORD pdwNum,
    _In_ DWORD dwNumCount
    );

DLL_FCT BOOL LdapExtractNamedNumAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_writes_(dwNumCount) PDWORD pdwNum,
    _In_ DWORD dwNumCount
    );

#ifdef UNICODE
#define LdapExtractNamedNumAttr  LdapExtractNamedNumAttrW
#else
#define LdapExtractNamedNumAttr  LdapExtractNamedNumAttrA
#endif // !UNICODE

DLL_FCT BOOL LdapExtractNamedBoolAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_writes_(dwBoolCount) PBOOL pbBool,
    _In_ DWORD dwBoolCount
    );

DLL_FCT BOOL LdapExtractNamedBoolAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_writes_(dwBoolCount) PBOOL pbBool,
    _In_ DWORD dwBoolCount
    );

#ifdef UNICODE
#define LdapExtractNamedBoolAttr  LdapExtractNamedBoolAttrW
#else
#define LdapExtractNamedBoolAttr  LdapExtractNamedBoolAttrA
#endif // !UNICODE

DLL_FCT BOOL LdapDupAttr(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pSourceAttr,
    _Out_ PLDAP_ATTRIBUTE *ppDestAttr
    );

DLL_FCT BOOL LdapDupNamedAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_ PLDAP_ATTRIBUTE *ppAttr
    );

DLL_FCT BOOL LdapDupNamedAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_ PLDAP_ATTRIBUTE *ppAttr
    );

#ifdef UNICODE
#define LdapDupNamedAttr  LdapDupNamedAttrW
#else
#define LdapDupNamedAttr  LdapDupNamedAttrA
#endif // !UNICODE

// TODO: helper for OID, dates, etc.

DLL_FCT BOOL LdapGetValue(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pAttribute,
    _In_ DWORD dwValueIndex,
    _Out_ PLDAP_VALUE *ppValue
    );

DLL_FCT BOOL LdapGetValueOfNamedAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _In_ DWORD dwValueIndex,
    _Out_ PLDAP_VALUE *ppValue
    );

DLL_FCT BOOL LdapGetValueOfNamedAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _In_ DWORD dwValueIndex,
    _Out_ PLDAP_VALUE *ppValue
    );

#ifdef UNICODE
#define LdapGetValueOfNamedAttr  LdapGetValueOfNamedAttrW
#else
#define LdapGetValueOfNamedAttr  LdapGetValueOfNamedAttrA
#endif // !UNICODE

DLL_FCT BOOL LdapEntryIsClassOfW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptClass
    );

DLL_FCT BOOL LdapEntryIsClassOfA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptClass
    );

#ifdef UNICODE
#define LdapEntryIsClassOf  LdapEntryIsClassOfW
#else
#define LdapEntryIsClassOf  LdapEntryIsClassOfA
#endif // !UNICODE

DLL_FCT BOOL LdapEscapeW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCWSTR ptString
    );

DLL_FCT BOOL LdapEscapeA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCSTR ptString
    );

#ifdef UNICODE
#define LdapEscape  LdapEscapeW
#else
#define LdapEscape  LdapEscapeA
#endif // !UNICODE

DLL_FCT BOOL LdapConstructFilterW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCWSTR ptFilterFormat,
    _In_ LPCWSTR ptFilterValues[],
    _Out_ LPWSTR ptConstructedFilter
    );

DLL_FCT BOOL LdapConstructFilterA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCSTR ptFilterFormat,
    _In_ LPCSTR ptFilterValues[],
    _Out_ LPSTR ptConstructedFilter
    );

#ifdef UNICODE
#define LdapConstructFilter  LdapConstructFilterW
#else
#define LdapConstructFilter  LdapConstructFilterA
#endif // !UNICODE

#ifdef __cplusplus
}
#endif

#endif // __LDAP_HELPERS_H__
