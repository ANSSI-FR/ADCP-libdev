/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef UNICODE
#define UNICODE
#endif

#ifndef __LIB_LDAP_H__
#define __LIB_LDAP_H__

#ifdef __cplusplus
extern "C" {
#endif

/* --- INCLUDES ------------------------------------------------------------- */
#include "..\..\LibUtils\src\UtilsLib.h"
#include <Winldap.h>
#include <NtLdap.h>

#ifdef VLD_DBG
    #define VLD_FORCE_ENABLE
    #include <vld.h>
    #pragma comment(lib,"vld.lib")
#endif

#undef DLL_FCT
#ifdef DLL_MODE
    #ifdef __LDAP_INT_H__
        #define DLL_FCT __declspec(dllexport)
    #else
        #define DLL_FCT __declspec(dllimport)
    #endif
#else
    #define DLL_FCT
#endif

/* --- DEFINES -------------------------------------------------------------- */

//
// Custom LdapLib error codes
//
#define LDAP_LIBERR                 (0xAAAA)
// TODO: some api return win32 ldap codes, defined in winldap.h...
#define LDAP_ERR_UNKNOWN_TODO       ERROR_CODE(LDAP_LIBERR, 1)
#define LDAP_ERR_BAD_PAGE_DATA      ERROR_CODE(LDAP_LIBERR, 2)
#define LDAP_ERR_BAD_ENTRY_DATA     ERROR_CODE(LDAP_LIBERR, 3)
#define LDAP_ERR_BAD_ATTR_DATA      ERROR_CODE(LDAP_LIBERR, 4)
#define LDAP_ERR_TOO_MANY_VAL       ERROR_CODE(LDAP_LIBERR, 5)
#define LDAP_NO_SUCH_VALUE          ERROR_CODE(LDAP_LIBERR, 6)

//
// Attributes & Classes
//
#define LDAP_ATTR_DISTINGUISHED_NAME            _T("dn")
#define LDAP_ATTR_COMMON_NAME                   _T("cn")
#define LDAP_ATTR_GPLINK                        _T("gPLink")
#define LDAP_ATTR_OBJECT_CLASS                  _T("objectClass")
#define LDAP_ATTR_MEMBER                        _T("member")
#define LDAP_ATTR_NTSD                          _T("nTSecurityDescriptor")
#define LDAP_ATTR_OBJECT_SID                    _T("objectSid")
#define LDAP_ATTR_ADMIN_COUNT                   _T("adminCount")
#define LDAP_ATTR_SCHEMA_ID_GUID                _T("schemaIDGUID")
#define LDAP_ATTR_GOVERNS_ID                    _T("governsID")
#define LDAP_ATTR_DEFAULT_SD                    _T("defaultSecurityDescriptor")
#define LDAP_ATTR_PRIMARY_GROUP_ID              _T("primaryGroupID")
#define LDAP_ATTR_SID_HISTORY                   _T("sIDHistory")
#define LDAP_ATTR_USN_CREATED                   _T("uSNCreated")
#define LDAP_ATTR_UAC                           _T("userAccountControl")
#define LDAP_ATTR_MANAGED_BY                    _T("managedBy")
#define LDAP_ATTR_REVEAL_ON_DEMAND              _T("msDS-RevealOnDemandGroup")

#define LDAP_CLASS_GROUP                        _T("group")
#define LDAP_CLASS_USER                         _T("user")

#define LDAP_ROOTDSE_CONFIG_NAMING_CONTEXT      _T("configurationNamingContext")
#define LDAP_ROOTDSE_CURRENT_TIME               _T("currentTime")
#define LDAP_ROOTDSE_DEFAULT_NAMING_CONTEXT     _T("defaultNamingContext")
#define LDAP_ROOTDSE_DNS_HOST_NAME              _T("dnsHostName")
#define LDAP_ROOTDSE_DC_FUNCTIONNALITY          _T("domainControllerFunctionality")
#define LDAP_ROOTDSE_DOMAIN_FUNCTIONNALITY      _T("domainFunctionality")
#define LDAP_ROOTDSE_DS_SERVICE_NAME            _T("dsServiceName")
#define LDAP_ROOTDSE_FOREST_FUNCTIONNALITY      _T("forestFunctionality")
#define LDAP_ROOTDSE_HIGHEST_COMMITTED_USN      _T("highestCommittedUSN")
#define LDAP_ROOTDSE_IS_GC_READY                _T("isGlobalCatalogReady")
#define LDAP_ROOTDSE_IS_SYNCHRONIZED            _T("isSynchronized")
#define LDAP_ROOTDSE_LDAP_SERVICE_NAME          _T("ldapServiceName")
#define LDAP_ROOTDSE_NAMING_CONTEXTS            _T("namingContexts")
#define LDAP_ROOTDSE_ROOT_DOMAIN_NAMING_CONTEXT _T("rootDomainNamingContext")
#define LDAP_ROOTDSE_SCHEMA_NAMING_CONTEXT      _T("schemaNamingContext")
#define LDAP_ROOTDSE_SERVER_NAME                _T("serverName")
#define LDAP_ROOTDSE_SUBSCHEMA_SUBENTRY         _T("subschemaSubentry")
#define LDAP_ROOTDSE_SUPPORTED_CAPABILITIES     _T("supportedCapabilities")
#define LDAP_ROOTDSE_SUPPORTED_CONTROL          _T("supportedControl")
#define LDAP_ROOTDSE_SUPPORTED_LDAP_POLICIES    _T("supportedLDAPPolicies")
#define LDAP_ROOTDSE_SUPPORTED_LDAP_VERSION     _T("supportedLDAPVersion")
#define LDAP_ROOTDSE_SUPPORTED_SASL_MECHANISMS  _T("supportedSASLMechanisms")

//
// Misc.
//
#define LDAP_DEFAULT_PORT                       (389)

/* --- TYPES ---------------------------------------------------------------- */
typedef enum _LDAP_AUTH_METHOD {
    LdapAuthSimple = LDAP_AUTH_SIMPLE,
    LdapAuthDigest = LDAP_AUTH_DIGEST,
    LdapAuthDpa = LDAP_AUTH_DPA,
    LdapAuthMsn = LDAP_AUTH_MSN,
    LdapAuthNegotiate = LDAP_AUTH_NEGOTIATE,
    LdapAuthNtlm = LDAP_AUTH_NTLM,
    LdapAuthSicily = LDAP_AUTH_SICILY,
    LdapAuthSspi = LDAP_AUTH_SSPI,
} LDAP_AUTH_METHOD;

typedef enum _LDAP_REQ_SCOPE {
    LdapScopeBase = LDAP_SCOPE_BASE,
    LdapScopeOneLevel = LDAP_SCOPE_ONELEVEL,
    LdapScopeSubtree = LDAP_SCOPE_SUBTREE,
} LDAP_REQ_SCOPE;

typedef enum _AD_FUNC_LEVEL {
    AdFuncLvlWin2000 = 0,
    AdFuncLvlWin2003Interim = 1,
    AdFuncLvlWin2003 = 2,
    AdFuncLvlWin2008 = 3,
    AdFuncLvlWin2008R2 = 4,
    AdFuncLvlWin2012 = 5,
    AdFuncLvlWin2012R2 = 6,
} AD_FUNC_LEVEL;

typedef struct _LDAP_CONNECT {
    PUTILS_HEAP pConnectionHeap;
    PLDAP pLdapSession;
    PTCHAR ptHost;
    DWORD dwPort;
} LDAP_CONNECT, *PLDAP_CONNECT;

typedef struct _LDAP_REQUEST {
    PLDAPSearch pLDAPSearch;
    PTCHAR *pptAttributes;
    DWORD dwRequestedAttrCount;

    DWORD dwCurrentPageEntries;
    PLDAPMessage pLDAPCurrentPage;

    DWORD dwCurrentEntryIndex;
    PLDAPMessage pLDAPCurrentEntry;
} LDAP_REQUEST, *PLDAP_REQUEST;

typedef struct _LDAP_VALUE {
    DWORD dwSize;
    PBYTE pbData;
} LDAP_VALUE, *PLDAP_VALUE;

typedef struct _LDAP_ATTRIBUTE {
    DWORD dwValuesCount;
    PLDAP_VALUE *ppValues;
    PTCHAR ptName;
} LDAP_ATTRIBUTE, *PLDAP_ATTRIBUTE;

typedef struct _LDAP_ENTRY {
    PTCHAR ptDn;
    DWORD dwAttributesCount;
    PLDAP_ATTRIBUTE *ppAttributes;
} LDAP_ENTRY, *PLDAP_ENTRY;

typedef struct _LDAP_ROOT_DSE {
    //
    // 'Extracted' attributes (coming directly from LDAP data)
    // (cf. http://msdn.microsoft.com/en-us/library/windows/desktop/ms684291.aspx)
    //
    struct {
        PTCHAR ptConfigurationNamingContext;
        PTCHAR ptCurrentTime; // TODO: date format ? string / timestamp ?
        PTCHAR ptDefaultNamingContext;
        PTCHAR ptDnsHostName;
        AD_FUNC_LEVEL eDomainControllerFunctionality;
        AD_FUNC_LEVEL eDomainFunctionality;
        PTCHAR ptDsServiceName;
        AD_FUNC_LEVEL eForestFunctionality;
        DWORD dwHighestCommittedUSN;
        BOOL bIsGlobalCatalogReady;
        BOOL bIsSynchronized;
        PTCHAR ptLdapServiceName;
        PTCHAR *pptNamingContexts;
        PTCHAR ptRootDomainNamingContext;
        PTCHAR ptSchemaNamingContext;
        PTCHAR ptServerName;
        PTCHAR ptSubschemaSubentry;
        PTCHAR *pptSupportedCapabilities;
        PTCHAR *pptSupportedControl;
        PTCHAR *pptSupportedLDAPPolicies;
        PTCHAR *pptSupportedLDAPVersion;
        PTCHAR *pptSupportedSASLMechanisms;
    } extracted;

    //
    // 'Computed' attributes (because they can be useful)
    //
    struct {
        // TODO: others ?

        struct {
            PTCHAR ptDnsDomainName;
            PTCHAR ptDomainSid;
        } misc;

        struct {
            DWORD dwNamingContextsCount;
            DWORD dwSupportedCapabilitiesCount;
            DWORD dwSupportedControlCount;
            DWORD dwSupportedLDAPPoliciesCount;
            DWORD dwSupportedLDAPVersionCount;
            DWORD dwSupportedSASLMechanismsCount;
        } count;
    } computed;

} LDAP_ROOT_DSE, *PLDAP_ROOT_DSE;

/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */
//
// Init/Cleanup
//
DLL_FCT
BOOL
LdapLibInit(
);

DLL_FCT
BOOL
LdapLibCleanup(
);

DLL_FCT BOOL LdapConnectW(
    _In_ const LPWSTR ptHost,
    _In_ const DWORD dwPort,
    _Out_ PLDAP_CONNECT *ppConnection,
    _Out_opt_ PLDAP_ROOT_DSE *ppRootDse
    );

DLL_FCT BOOL LdapConnectA(
    _In_ const LPSTR ptHost,
    _In_ const DWORD dwPort,
    _Out_ PLDAP_CONNECT *ppConnection,
    _Out_opt_ PLDAP_ROOT_DSE *ppRootDse
    );

#ifdef UNICODE
#define LdapConnect  LdapConnectW
#else
#define LdapConnect  LdapConnectA
#endif // !UNICODE

DLL_FCT BOOL LdapBindW(
    _Inout_ const PLDAP_CONNECT pConnection,
    _In_ const LPWSTR ptBindingDn,
    _In_opt_ LPWSTR ptUsername,
    _In_opt_ LPWSTR ptPassword,
    _In_opt_ LPWSTR ptDnsDomainName
    );

DLL_FCT BOOL LdapBindA(
    _Inout_ const PLDAP_CONNECT pConnection,
    _In_ LPSTR ptBindingDn,
    _In_opt_ LPSTR ptUsername,
    _In_opt_ LPSTR ptPassword,
    _In_opt_ LPSTR ptDnsDomainName
    );

#ifdef UNICODE
#define LdapBind  LdapBindW
#else
#define LdapBind  LdapBindA
#endif // !UNICODE

DLL_FCT BOOL LdapBindExW(
    _Inout_ const PLDAP_CONNECT pConnection,
    _In_ const LPWSTR ptBindingDn,
    _In_ const LDAP_AUTH_METHOD eAuthMethod,
    _In_opt_ const PVOID pvAuthData
    );

DLL_FCT BOOL LdapBindExA(
    _Inout_ const PLDAP_CONNECT pConnection,
    _In_ LPSTR ptBindingDn,
    _In_ const LDAP_AUTH_METHOD eAuthMethod,
    _In_opt_ const PVOID pvAuthData
    );

#ifdef UNICODE
#define LdapBindEx  LdapBindExW
#else
#define LdapBindEx  LdapBindExA
#endif // !UNICODE

DLL_FCT BOOL LdapInitRequestW(
    _In_ const PLDAP_CONNECT pConnection,
    _In_opt_ const LPWSTR ptBase,
    _In_opt_ const LPWSTR ptFilter,
    _In_ const LDAP_REQ_SCOPE eScope,
    _In_opt_ const LPWSTR pptAttributes[],
    _Out_ PLDAP_REQUEST *ppRequest
    );

DLL_FCT BOOL LdapInitRequestA(
    _In_ const PLDAP_CONNECT pConnection,
    _In_opt_ const LPSTR ptBase,
    _In_opt_ const LPSTR ptFilter,
    _In_ const LDAP_REQ_SCOPE eScope,
    _In_opt_ const LPSTR pptAttributes[],
    _Out_ PLDAP_REQUEST *ppRequest
    );

#ifdef UNICODE
#define LdapInitRequest  LdapInitRequestW
#else
#define LdapInitRequest  LdapInitRequestA
#endif // !UNICODE

DLL_FCT BOOL LdapInitRequestExW(
    _In_ const PLDAP_CONNECT pConnection,
    _In_opt_ const LPWSTR ptBase,
    _In_opt_ const LPWSTR ptFilter,
    _In_ const LDAP_REQ_SCOPE eScope,
    _In_opt_ const LPWSTR pptAttributes[],
    _In_opt_ PLDAPControl pvServerControls[],
    _In_opt_ PLDAPControl pClientControls[],
    _Out_ PLDAP_REQUEST *ppRequest
    );

DLL_FCT BOOL LdapInitRequestExA(
    _In_ const PLDAP_CONNECT pConnection,
    _In_opt_ const LPSTR ptBase,
    _In_opt_ const LPSTR ptFilter,
    _In_ const LDAP_REQ_SCOPE eScope,
    _In_opt_ const LPSTR pptAttributes[],
    _In_opt_ PLDAPControl pServerControls[],
    _In_opt_ PLDAPControl pClientControls[],
    _Out_ PLDAP_REQUEST *ppRequest
    );

#ifdef UNICODE
#define LdapInitRequestEx  LdapInitRequestExW
#else
#define LdapInitRequestEx  LdapInitRequestExA
#endif // !UNICODE

DLL_FCT BOOL LdapGetNextEntry(
    _In_ const PLDAP_CONNECT pConnection,
    _In_ const PLDAP_REQUEST pRequest,
    _Out_ PLDAP_ENTRY *ppEntry
    );

DLL_FCT BOOL LdapReleaseAttribute(
    _In_ const PLDAP_CONNECT pConnection,
    _Inout_ PLDAP_ATTRIBUTE *ppAttribute
    );

DLL_FCT BOOL LdapReleaseValue(
    _In_ const PLDAP_CONNECT pConnection,
    _Inout_ PLDAP_VALUE *ppValue
    );

DLL_FCT BOOL LdapReleaseEntry(
    _In_ const PLDAP_CONNECT pConnection,
    _Inout_ PLDAP_ENTRY *ppEntry
    );

DLL_FCT BOOL LdapReleaseRequest(
    _In_ const PLDAP_CONNECT pConnection,
    _Inout_ PLDAP_REQUEST *ppRequest
    );

DLL_FCT BOOL LdapCloseConnection(
    _Inout_ PLDAP_CONNECT *ppConnection,
    _Inout_opt_ PLDAP_ROOT_DSE *ppRootDse
    );

DLL_FCT DWORD LdapLastError(
    );

#ifdef __cplusplus
}
#endif

#endif // __LIB_LDAP_H__
