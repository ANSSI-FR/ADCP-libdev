/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/


#ifndef __LDAP_INT_H__
#define __LDAP_INT_H__


/* --- INCLUDES ------------------------------------------------------------- */
#include <Windows.h>
#include <tchar.h>
#include <Winldap.h>
#include <Winber.h>
#include <stdio.h>

#include "LdapLib.h"


/* --- DEFINES -------------------------------------------------------------- */
#define LDAP_INIT_OPTION(pOpt, iOpt, pvVal) MULTI_LINE_MACRO_BEGIN          \
                                                (pOpt)->iOption = iOpt;     \
                                                (pOpt)->pvValue = pvVal;    \
                                            MULTI_LINE_MACRO_END

#define LDAP_RANGE_MAGIC                    _T("range=")


/* --- TYPES ---------------------------------------------------------------- */
typedef struct _LDAP_OPTION {
    int iOption;
    PVOID pvValue;
} LDAP_OPTION, *PLDAP_OPTION;


/* --- VARIABLES ------------------------------------------------------------ */
extern DWORD gs_dwLastError;

/* --- PROTOTYPES ----------------------------------------------------------- */

BOOL LdapSetOption(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_OPTION pOption
    );
    
BOOL LdapExtractAttributes(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAPMessage pCurrentEntry,
    _In_ LPWSTR ptAttribute,
    _Inout_ PLDAP_ATTRIBUTE *ppNewAttribute
    );

BOOL LdapExtractRangedAttributes(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAPMessage pCurrentEntry,
    _In_ LPWSTR ptOrigAttribute,
    _In_ LPWSTR ptAttribute,
    _Inout_ PLDAP_ATTRIBUTE *ppNewAttribute
    );

BOOL GetRangeValues(
    _In_ PLDAP_CONNECT pConnection,
    _Inout_ PLDAPMessage pEntry,
    _In_ LPWSTR ptOrigAttribute,
    _Out_ PDWORD pdwAttributeCount,
    _Out_ PBYTE **pppbData,
    _Out_ PDWORD *ppdwDataSize,
    _Out_ PDWORD pdwStart,
    _Out_ PDWORD pdwEnd
    );

BOOL ParseRange(
    _In_ LPWSTR ptAtttype,
    _In_ LPWSTR ptAttdescr,
    _Out_ PDWORD pdwStart,
    _Out_ PDWORD pdwEnd
    );

BOOL ConstructRangeAtt(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCWSTR ptAtttype,
    _In_ DWORD dwStart,
    _In_ INT iEnd,
    _Inout_ LPWSTR* pptOutputRangeAttr
    );

BOOL IsRangedAtt(
    _In_ LPCWSTR ptAttribute,
    _In_opt_ LPCWSTR ptOrigAttribute
    );

BOOL ExtractOrigAttFromRangedAtt(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCWSTR ptAttribute,
    _Out_ LPWSTR *pptOrigAttribute
    );

_Ret_notnull_ LPWSTR LdapExtractAttrName(
    _In_ const PLDAP_CONNECT pConnection,
    _In_ const LPWSTR pAttrNameWithRange
    );

#endif // __LDAP_INT_H__
