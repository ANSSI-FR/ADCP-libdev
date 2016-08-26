/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#include "LdapInternals.h"
#include "LdapLib.h"
#include "LdapHelpers.h"

#include "LdapWpp.h"
#include "LdapLib.tmh"


/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
static BOOL LdapiLibInit(
    )
{
    LdapWppInit();
    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "LibLdap WPP initialized");
    API_RETURN_SUCCESS();
}

static BOOL LdapiLibCleanup(
    )
{
    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Cleanup LibLdap WPP");
    LdapWppClean();
    API_RETURN_SUCCESS();
}

static BOOL LdapExtractEntryDataFromLdapMsg(
    _In_ const PLDAP_CONNECT pConnection,
    _In_ const PLDAP_REQUEST pRequest,
    _In_ const PLDAPMessage pLdapMessage,
    _Out_ PLDAP_ENTRY *ppEntry
    )
{
    BOOL bResult = FALSE;
    BOOL bHasAttrList = (pRequest->dwRequestedAttrCount > 0);
    DWORD dwAttrIndex = 0;
    LPWSTR ptLdapDn = NULL;
    LPWSTR ptCurrAttrName = NULL;
    LPWSTR ptCurrAttrNameWithRange = NULL;
    BerElement* pBerElt = NULL;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting data extraction from LDAP message <%p> to LDAP entry", pLdapMessage);

    (*ppEntry) = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_ENTRY);
    ZeroMemory((*ppEntry), sizeof(LDAP_ENTRY)); // TODO: useless (warning C6001)

    
    ptLdapDn = ldap_get_dnW(pConnection->pLdapSession, pLdapMessage);
    if (ptLdapDn != NULL)
    {
        (*ppEntry)->ptDn = UtilsHeapStrDupHelper(pConnection->pConnectionHeap, ptLdapDn);
        ldap_memfree(ptLdapDn);
    }

    if (bHasAttrList == TRUE)
    {
        (*ppEntry)->ppAttributes = UtilsHeapAllocArrayHelper(pConnection->pConnectionHeap, PLDAP_ATTRIBUTE, pRequest->dwRequestedAttrCount);
        (*ppEntry)->dwAttributesCount = pRequest->dwRequestedAttrCount;
        for (DWORD i = 0; i < pRequest->dwRequestedAttrCount; i++)
        {
            (*ppEntry)->ppAttributes[i] = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_ATTRIBUTE);
            (*ppEntry)->ppAttributes[i]->ptName = UtilsHeapStrDupHelper(pConnection->pConnectionHeap, pRequest->pptAttributes[i]);
        }
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, _T("Processing entry <%ws> for LDAP request <%p>"), (*ppEntry)->ptDn, pRequest->pLDAPSearch);

    // Retrieve data for each attributes
    DWORD dwCurrIndex = 0;
    for (;;)
    {
        //
        // Extract attribute from retreived ldap message
        //

        if (dwCurrIndex == 0)
        {
            ptCurrAttrNameWithRange = ldap_first_attributeW(pConnection->pLdapSession, pLdapMessage, &pBerElt);
        }
        else
        {
            ptCurrAttrNameWithRange = ldap_next_attributeW(pConnection->pLdapSession, pLdapMessage, pBerElt);
        }

        //
        // Find or allocate ldap values in the entry attribute list
        //
        if (ptCurrAttrNameWithRange)
        {
            ptCurrAttrName = LdapExtractAttrName(pConnection, ptCurrAttrNameWithRange);

            if (bHasAttrList == TRUE)
            {
                bResult = LdapLocateNamedAttrW(pConnection, (*ppEntry), ptCurrAttrName, &dwAttrIndex);
                if (API_FAILED(bResult))
                {
                    // TODO: free stuff here
                    LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, _T("Unable to locate attribute <%ws> for entry <%ws> during LDAP request <%p>"), ptCurrAttrName, (*ppEntry)->ptDn, pRequest->pLDAPSearch);
                    API_RETURN_SAME_ERROR();
                }
            }
            else
            {
                (*ppEntry)->ppAttributes = UtilsHeapAllocOrReallocHelper(pConnection->pConnectionHeap, (*ppEntry)->ppAttributes, SIZEOF_ARRAY(PLDAP_ATTRIBUTE, ((*ppEntry)->dwAttributesCount + 1)));
                dwAttrIndex = (*ppEntry)->dwAttributesCount;
                (*ppEntry)->dwAttributesCount += 1;
            }

            // Select the right method to retrieve attributes (directly or by range)
            if (API_FAILED(IsRangedAtt(ptCurrAttrNameWithRange, bHasAttrList ? pRequest->pptAttributes[dwAttrIndex]: NULL)))
            {
                if (API_FAILED(LdapExtractAttributes(pConnection, pLdapMessage, ptCurrAttrNameWithRange, &(*ppEntry)->ppAttributes[dwAttrIndex]))) // TODO: changer pour ne pas se charger de l'alloc...
                {
                    ldap_unbind(pConnection->pLdapSession); // TODO: appeler les fonctions de destruction ici, plutot que de faire un unbind_directement
                    LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, _T("Unable to extract monovaluated attributes <%ws> for entry <%ws> during LDAP request <%p>"), ptCurrAttrNameWithRange, (*ppEntry)->ptDn, pRequest->pLDAPSearch);
                    API_RETURN_ERROR(LDAP_ERR_BAD_ATTR_DATA);
                }
                LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, _T("Successfully extracted monovaluated attribute <%ws> for entry <%ws> during LDAP request <%p>"), ptCurrAttrNameWithRange, (*ppEntry)->ptDn, pRequest->pLDAPSearch);
            }
            else
            {
				bResult = LdapExtractRangedAttributes(pConnection, pLdapMessage, ptCurrAttrName, ptCurrAttrNameWithRange, &(*ppEntry)->ppAttributes[dwAttrIndex]);
				if (gs_dwLastError == LDAP_NO_SUCH_OBJECT) {
					LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, _T("No such object (non-fatal) <%ws> for entry <%ws> during LDAP request <%p>"), ptCurrAttrNameWithRange, (*ppEntry)->ptDn, pRequest->pLDAPSearch);
				}
				else if (API_FAILED(bResult))
                {
                    ldap_unbind(pConnection->pLdapSession);  // TODO: appeler les fonctions de destruction ici, plutot que de faire un unbind_directement
                    LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, _T("Unable to extract multivaluated attributes <%ws> for entry <%ws> during LDAP request <%p>"), ptCurrAttrNameWithRange, (*ppEntry)->ptDn, pRequest->pLDAPSearch);
                    API_RETURN_ERROR(LDAP_ERR_BAD_ATTR_DATA);
                }
                LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, _T("Successfully extracted multivaluated attribute <%ws> for entry <%ws> during LDAP request <%p>"), ptCurrAttrNameWithRange, (*ppEntry)->ptDn, pRequest->pLDAPSearch);
            }

            LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, _T("Successfully stored extracted attribute <%ws> into result set for entry <%ws> during LDAP request <%p>"), ptCurrAttrNameWithRange, (*ppEntry)->ptDn, pRequest->pLDAPSearch);
            ldap_memfree(ptCurrAttrNameWithRange);
            ptCurrAttrNameWithRange = NULL;
            UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, ptCurrAttrName);
            dwCurrIndex++;
        }
        else
        {
            // No more attribute found by ldap_first/next_attribute
            break;
        }
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, _T("Successfully extracted data from LDAP message <%p> to LDAP entry <%p>"), pLdapMessage, (*ppEntry));
    API_RETURN_SUCCESS();
}

    static BOOL LdapBuildRequestObjectWithAttributes(
    _In_ const PLDAP_CONNECT pConnection,
    _In_opt_ const LPWSTR pptAttributes[],
    _Out_ PLDAP_REQUEST *ppRequest
    )
{
    DWORD dwIndex = 0;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting build of request object from attributes list");

    (*ppRequest) = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_REQUEST);

    // Copie the attribute parameters
    if (pptAttributes != NULL)
    {
        (*ppRequest)->pptAttributes = UtilsHeapAllocArrayHelper(pConnection->pConnectionHeap, LPWSTR, 1);
        (*ppRequest)->pptAttributes[0] = NULL;

        for (dwIndex = 0; pptAttributes[dwIndex] != NULL; /*dwIndex incrementation in loop body*/)
        {
            (*ppRequest)->pptAttributes[dwIndex] = UtilsHeapStrDupHelper(pConnection->pConnectionHeap, pptAttributes[dwIndex]);
            dwIndex++;

            (*ppRequest)->pptAttributes = UtilsHeapReallocArrayHelper(pConnection->pConnectionHeap, (*ppRequest)->pptAttributes, LPWSTR, (dwIndex + 1));
            (*ppRequest)->pptAttributes[dwIndex] = NULL;
        }

        (*ppRequest)->dwRequestedAttrCount = dwIndex;
    }
    else
    {
        LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "No attributes specified, requesting all attributes");
        (*ppRequest)->pptAttributes = NULL;
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully built request object from attributes list");
    API_RETURN_SUCCESS();
}

static BOOL LdapNonPagedRequest(
    _In_ PLDAP_CONNECT pConnection,
    _In_opt_ LPWSTR ptBase,
    _In_opt_ LPWSTR ptFilter,
    _In_ LDAP_REQ_SCOPE eScope,
    _In_opt_ LPWSTR pptAttributes[],
    _Out_ PLDAP_REQUEST *ppRequest,
    _Out_ PLDAPMessage *ppLdapMessage
    )
{
    BOOL bResult = FALSE;
    DWORD dwResult = LDAP_SUCCESS;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting non-paged request <%ws>", ptFilter);

    (*ppRequest) = NULL;
    (*ppLdapMessage) = NULL;

    bResult = LdapBuildRequestObjectWithAttributes(pConnection, pptAttributes, ppRequest);
    if (API_FAILED(bResult))
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Failed to build request object with attributes list: <err:%#08x>", LdapLastError());
        API_RETURN_SAME_ERROR();
    }

#pragma warning(suppress: 6387)
    dwResult = ldap_search_sW(pConnection->pLdapSession, ptBase, eScope, ptFilter, pptAttributes, FALSE, ppLdapMessage);
    if (dwResult != LDAP_SUCCESS)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Failed to execute non-paged LDAP request <%ws>: <err:%#08x>", ptFilter, dwResult);
        API_RETURN_ERROR(dwResult);
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully sent non-paged request <%ws>", ptFilter);
    API_RETURN_SUCCESS();
}

static BOOL LdapExtractRootDse(
    _In_ const PLDAP_CONNECT pConnection,
    _Out_ PLDAP_ROOT_DSE *ppRootDse
    )
{
    BOOL bResult = FALSE;
    DWORD dwResult = LDAP_SUCCESS;
    PLDAPMessage pLdapMsg = NULL;
    PLDAP_REQUEST pRequest = NULL;
    PLDAP_ENTRY pEntry = NULL;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting RootDSE attributes extraction");

    (*ppRootDse) = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_ROOT_DSE);

    //
    // Anonymous bind
    //
    dwResult = ldap_bind_sW(
        pConnection->pLdapSession,
        NULL,   // No binding DN
        NULL,   // No credentials
        LDAP_AUTH_SIMPLE
        );
    if (dwResult != LDAP_SUCCESS)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, CONNECTION, "Failed to bind anonymously to LDAP server: <%#x>", dwResult);
        API_RETURN_ERROR(dwResult);
    }
    LdapWppMessage(TRACE_LEVEL_INFORMATION, CONNECTION, "Successfully binded to LDAP server anonymously");

    //
    // Request all attributes of the RootDse (base scope)
    //

    // Here we cannot use LdapInitRequest/GetNextEntry, because they issue "paged requests"
    // that require a "session" to be established and thus cannot be done with an anonymous binding.
    // cf. http://msdn.microsoft.com/en-us/library/aa813628.aspx: LDAP Controls and Extended Operations supported by Active Directory
    //LDAP OID                  Name                            Description             Control type
    //1.2.840.113556.1.4.319    LDAP_PAGED_RESULT_OID_STRING    Paged search control    Session required
    bResult = LdapNonPagedRequest(
        pConnection,
        NULL,   // No base-DN
        NULL,   // No filter
        LdapScopeBase,
        NULL,   // All attributes
        &pRequest,
        &pLdapMsg
    );
    if (API_FAILED(bResult))
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Failed to request RootDSE attributes: <%#x>", dwResult);
        API_RETURN_SAME_ERROR();
    }
    LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Successfully initialized RootDSE request");

    //
    // Extract all attributes
    //
    bResult = LdapExtractEntryDataFromLdapMsg(pConnection, pRequest, pLdapMsg, &pEntry);
    if (API_FAILED(bResult))
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Error while getting LDAP entry for RootDSE request: <err:%#08x>", LdapLastError());
        API_RETURN_SAME_ERROR();
    }
    LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Successfully got LDAP entry for RootDSE request");

    bResult = TRUE;
    bResult &= LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_CONFIG_NAMING_CONTEXT, &(*ppRootDse)->extracted.ptConfigurationNamingContext, 1);
    bResult &= LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_CURRENT_TIME, &(*ppRootDse)->extracted.ptCurrentTime, 1);
    bResult &= LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_DEFAULT_NAMING_CONTEXT, &(*ppRootDse)->extracted.ptDefaultNamingContext, 1);
    bResult &= LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_DNS_HOST_NAME, &(*ppRootDse)->extracted.ptDnsHostName, 1);
    bResult &= LdapExtractNamedNumAttrW(pConnection, pEntry, LDAP_ROOTDSE_DC_FUNCTIONNALITY, (PDWORD)&(*ppRootDse)->extracted.eDomainControllerFunctionality, 1);
    bResult &= LdapExtractNamedNumAttrW(pConnection, pEntry, LDAP_ROOTDSE_DOMAIN_FUNCTIONNALITY, (PDWORD)&(*ppRootDse)->extracted.eDomainFunctionality, 1);
    bResult &= LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_DS_SERVICE_NAME, &(*ppRootDse)->extracted.ptDsServiceName, 1);
    bResult &= LdapExtractNamedNumAttrW(pConnection, pEntry, LDAP_ROOTDSE_FOREST_FUNCTIONNALITY, (PDWORD)&(*ppRootDse)->extracted.eForestFunctionality, 1);
    bResult &= LdapExtractNamedNumAttrW(pConnection, pEntry, LDAP_ROOTDSE_HIGHEST_COMMITTED_USN, &(*ppRootDse)->extracted.dwHighestCommittedUSN, 1);
    bResult &= LdapExtractNamedBoolAttrW(pConnection, pEntry, LDAP_ROOTDSE_IS_GC_READY, &(*ppRootDse)->extracted.bIsGlobalCatalogReady, 1);
    bResult &= LdapExtractNamedBoolAttrW(pConnection, pEntry, LDAP_ROOTDSE_IS_SYNCHRONIZED, &(*ppRootDse)->extracted.bIsSynchronized, 1);
    
    // TODO: special case, when remounting a ntds.dit file with dsamain, the 'ldapServiceName' attribute is not present...
    /*bResult &=*/ LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_LDAP_SERVICE_NAME, &(*ppRootDse)->extracted.ptLdapServiceName, 1);

    bResult &= LdapAllocAndExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_NAMING_CONTEXTS, &(*ppRootDse)->extracted.pptNamingContexts, &(*ppRootDse)->computed.count.dwNamingContextsCount);
    bResult &= LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_ROOT_DOMAIN_NAMING_CONTEXT, &(*ppRootDse)->extracted.ptRootDomainNamingContext, 1);
    bResult &= LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_SCHEMA_NAMING_CONTEXT, &(*ppRootDse)->extracted.ptSchemaNamingContext, 1);
    bResult &= LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_SERVER_NAME, &(*ppRootDse)->extracted.ptServerName, 1);
    bResult &= LdapExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_SUBSCHEMA_SUBENTRY, &(*ppRootDse)->extracted.ptSubschemaSubentry, 1);
    bResult &= LdapAllocAndExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_SUPPORTED_CAPABILITIES, &(*ppRootDse)->extracted.pptSupportedCapabilities, &(*ppRootDse)->computed.count.dwSupportedCapabilitiesCount);
    bResult &= LdapAllocAndExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_SUPPORTED_CONTROL, &(*ppRootDse)->extracted.pptSupportedControl, &(*ppRootDse)->computed.count.dwSupportedControlCount);

    // TODO: special case, when dumping a Samb4 directory, the 'supportedLDAPPolicies' attribute is not present...
    /*bResult &=*/ LdapAllocAndExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_SUPPORTED_LDAP_POLICIES, &(*ppRootDse)->extracted.pptSupportedLDAPPolicies, &(*ppRootDse)->computed.count.dwSupportedLDAPPoliciesCount);

    bResult &= LdapAllocAndExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_SUPPORTED_LDAP_VERSION, &(*ppRootDse)->extracted.pptSupportedLDAPVersion, &(*ppRootDse)->computed.count.dwSupportedLDAPVersionCount);
    bResult &= LdapAllocAndExtractNamedStrAttrW(pConnection, pEntry, LDAP_ROOTDSE_SUPPORTED_SASL_MECHANISMS, &(*ppRootDse)->extracted.pptSupportedSASLMechanisms, &(*ppRootDse)->computed.count.dwSupportedSASLMechanismsCount);

    if (API_FAILED(bResult))
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Failed to retreive all RootDSE attributes, some of them may be missing.");
        API_RETURN_ERROR(LDAP_NO_SUCH_ATTRIBUTE);
    }
    LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Successfully retreived all RootDSE attributes");

    //
    // Now fill 'computed' attributes
    //
    // TODO

    //
    // Cleanup
    //
    LdapReleaseRequest(pConnection, &pRequest);
    LdapReleaseEntry(pConnection, &pEntry);
    ldap_msgfree(pLdapMsg);
    pLdapMsg = NULL;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully ended RootDSE attributes extraction");
    API_RETURN_SUCCESS();
}

/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
BOOL LdapConnectW(
    _In_ const LPWSTR ptHost,
    _In_ const DWORD dwPort,
    _Out_ PLDAP_CONNECT *ppConnection,
    _Out_opt_ PLDAP_ROOT_DSE *ppRootDse
    )
{
    BOOL bResult = FALSE;
    DWORD dwResult = LDAP_SUCCESS;
    DWORD i = 0;
    ULONG ulVersion = LDAP_VERSION3;
    ULONG ulNoLimit = LDAP_NO_LIMIT;
    ULONG ulOptOn = 1; // LDAP_OPT_ON;
    PUTILS_HEAP pHeap = NULL;

    LDAP_OPTION sLdapOptions[4] = { 0 };
    LDAP_INIT_OPTION(&sLdapOptions[0], LDAP_OPT_PROTOCOL_VERSION, &ulVersion);
    LDAP_INIT_OPTION(&sLdapOptions[1], LDAP_OPT_SIZELIMIT, &ulNoLimit);
    LDAP_INIT_OPTION(&sLdapOptions[2], LDAP_OPT_TIMELIMIT, &ulNoLimit);
    LDAP_INIT_OPTION(&sLdapOptions[3], LDAP_OPT_AUTO_RECONNECT, &ulOptOn);

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting LDAP connection to host <%ws> on port <%u>", ptHost, dwPort);

    //
    // Initialization
    //
    (*ppConnection) = NULL;
    if (ppRootDse != NULL)
    {
        (*ppRootDse) = NULL;
    }
    bResult = UtilsHeapCreate(&pHeap, ptHost, NULL);
    if (pHeap == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, ALLOCATION, "Failed to create heap: <err:%#08x>", UtilsGetLastError());
        API_RETURN_SAME_ERROR();
    }
    LdapWppMessage(TRACE_LEVEL_INFORMATION, ALLOCATION, "Successfully created heap: <%p>", pHeap);

    (*ppConnection) = UtilsHeapAllocStructHelper(pHeap, LDAP_CONNECT);
    (*ppConnection)->pConnectionHeap = pHeap;
    (*ppConnection)->dwPort = dwPort;
    (*ppConnection)->ptHost = UtilsHeapStrDupHelper(pHeap, ptHost);
    (*ppConnection)->pLdapSession = ldap_initW(ptHost, dwPort);
    if ((*ppConnection)->pLdapSession == NULL)
    {
        dwResult = LdapGetLastError();
        LdapWppMessage(TRACE_LEVEL_ERROR, CONNECTION, "Failed to init connection to LDAP server: <err:%#08x>", dwResult);
        LdapCloseConnection(ppConnection, ppRootDse); // TODO: check return value ?
        API_RETURN_ERROR(dwResult);
    }
    LdapWppMessage(TRACE_LEVEL_INFORMATION, CONNECTION, "Successfully initialized connection to LDAP server");


    //
    // LDAP options
    //
    for (i = 0; i < _countof(sLdapOptions); i++)
    {
        bResult = LdapSetOption(*ppConnection, &sLdapOptions[i]);
        if (API_FAILED(bResult))
        {
            LdapWppMessage(TRACE_LEVEL_ERROR, CONFIGURATION, "Failed to set LDAP options");
            API_RETURN_SAME_ERROR();
        }
    }
    LdapWppMessage(TRACE_LEVEL_INFORMATION, CONFIGURATION, "Successfully set all LDAP options");


    //
    // Connection
    //
    dwResult = ldap_connect(
        (*ppConnection)->pLdapSession,
        NULL    // If NULL, the function uses a default timeout value 
        );
    if (dwResult != LDAP_SUCCESS)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, CONNECTION, "Failed to connect to LDAP server: <%#x>", dwResult);
        API_RETURN_ERROR(dwResult);
    }
    LdapWppMessage(TRACE_LEVEL_INFORMATION, CONNECTION, "Successfully connected to LDAP server");

    //
    // ROOT DSE exctraction
    //
    if (ppRootDse != NULL)
    {
        bResult = LdapExtractRootDse(*ppConnection, ppRootDse);
        if (API_FAILED(bResult))
        {
            LdapWppMessage(TRACE_LEVEL_ERROR, CONNECTION, "Failed to extract RootDSE attributes");
            API_RETURN_SAME_ERROR();
        }
        LdapWppMessage(TRACE_LEVEL_INFORMATION, CONNECTION, "Successfully extracted RootDSE attributes");
    }
    else
    {
        LdapWppMessage(TRACE_LEVEL_INFORMATION, CONNECTION, "Skipping RootDSE extraction");
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully established LDAP connection to host <%ws> on port <%u>", ptHost, dwPort);
    API_RETURN_SUCCESS();
}

#pragma warning(suppress: 6101)
BOOL LdapConnectA(
    _In_ const LPSTR ptHost,
    _In_ const DWORD dwPort,
    _Out_ PLDAP_CONNECT *ppConnection,
    _Out_opt_ PLDAP_ROOT_DSE *ppRootDse
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwHost = NULL;

    lpwHost = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (strlen(ptHost) + 1) * sizeof(LPWSTR));
    if (lpwHost == NULL) {
        LdapWppMessage(TRACE_LEVEL_ERROR, CONNECTION, "cannot allocate <%u> bytes", (DWORD)(strlen(ptHost) + 1) * sizeof(LPWSTR));
        API_RETURN_SAME_ERROR();
    }

    if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, ptHost, (DWORD)strlen(ptHost), lpwHost, (DWORD)strlen(ptHost))) {
        LdapWppMessage(TRACE_LEVEL_ERROR, CONNECTION, "Failed to convert ANSI string: <err:%#08x>", GetLastError());
        API_RETURN_SAME_ERROR();
    }

    bReturn = LdapConnectW(lpwHost, dwPort, ppConnection, ppRootDse);

    HeapFree(GetProcessHeap(), 0, lpwHost);
    return bReturn;
}

BOOL LdapBindExW(
    _Inout_ const PLDAP_CONNECT pConnection,
    _In_ const LPWSTR ptBindingDn,
    _In_ const LDAP_AUTH_METHOD eAuthMethod,
    _In_opt_ const PVOID pvAuthData
    )
{
    DWORD dwResult = LDAP_SUCCESS;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting binding to DN <%ws> using authentication method <%u>", ptBindingDn, eAuthMethod);

    dwResult = ldap_bind_sW(pConnection->pLdapSession, ptBindingDn, pvAuthData, eAuthMethod);
    if (dwResult != LDAP_SUCCESS)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, CONNECTION, "Failed to bind to DN <%ws> using authentication method <%u>", ptBindingDn, eAuthMethod);
        API_RETURN_ERROR(dwResult);
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully binded to DN <%ws> using authentication method <%u>", ptBindingDn, eAuthMethod);
    API_RETURN_SUCCESS();
}

BOOL LdapBindExA(
    _Inout_ const PLDAP_CONNECT pConnection,
    _In_ LPSTR ptBindingDn,
    _In_ const LDAP_AUTH_METHOD eAuthMethod,
    _In_opt_ const PVOID pvAuthData
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwBindingDn = NULL;

    UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptBindingDn, &lpwBindingDn);

    bReturn = LdapBindExW(pConnection, lpwBindingDn, eAuthMethod, pvAuthData);

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwBindingDn);
    return bReturn;
}

BOOL LdapBindW(
    _Inout_ const PLDAP_CONNECT pConnection,
    _In_ const LPWSTR ptBindingDn,
    _In_opt_ LPWSTR ptUsername,
    _In_opt_ LPWSTR ptPassword,
    _In_opt_ LPWSTR ptDomain
    )
{
    PVOID pvAuthData = NULL;
    SEC_WINNT_AUTH_IDENTITY sSecIdent = { 0 };
    ZeroMemory(&sSecIdent, sizeof(sSecIdent));

    if (ptUsername != NULL && ptPassword != NULL)
    {
#ifdef _UNICODE
        sSecIdent.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;
#else
        sSecIdent.Flags = SEC_WINNT_AUTH_IDENTITY_ANSI;
#endif

        LdapWppMessage(TRACE_LEVEL_INFORMATION, CONNECTION, "Binding using explicit authentication with username <%ws>", ptUsername);
        sSecIdent.User = (unsigned short*)ptUsername;
        sSecIdent.UserLength = (unsigned long)_tcslen(ptUsername);
        sSecIdent.Password = (unsigned short*)ptPassword;
        sSecIdent.PasswordLength = (unsigned long)_tcslen(ptPassword);
        sSecIdent.Domain = (unsigned short*)ptDomain;
        sSecIdent.DomainLength = ptDomain ? (unsigned long)_tcslen(ptDomain): 0;

        pvAuthData = (PVOID)&sSecIdent;
    }
    else
    {
        LdapWppMessage(TRACE_LEVEL_INFORMATION, CONNECTION, "Binding using implicit authentication");
    }

    return LdapBindEx(pConnection, ptBindingDn, LdapAuthNegotiate, pvAuthData);
}

BOOL LdapBindA(
    _Inout_ const PLDAP_CONNECT pConnection,
    _In_ LPSTR ptBindingDn,
    _In_opt_ LPSTR ptUsername,
    _In_opt_ LPSTR ptPassword,
    _In_opt_ LPSTR ptDnsDomainName
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwBindingDn = NULL;
    LPWSTR lpwUsername = NULL;
    LPWSTR lpwPassword = NULL;
    LPWSTR lpwDnsDomainNam = NULL;

    UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptBindingDn, &lpwBindingDn);
    if (ptUsername != NULL) UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptUsername, &lpwUsername);
    if (ptPassword != NULL) UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptPassword, &lpwPassword);
    if (ptDnsDomainName != NULL) UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptDnsDomainName, &lpwDnsDomainNam);

    bReturn = LdapBindW(pConnection, lpwBindingDn, lpwUsername, lpwPassword, lpwDnsDomainNam);

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwBindingDn);
    if (lpwUsername != NULL) UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwBindingDn);
    if (lpwPassword != NULL) UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwBindingDn);
    if (lpwDnsDomainNam != NULL) UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwBindingDn);
    return bReturn;
}

BOOL LdapCloseConnection(
    _Inout_ PLDAP_CONNECT *ppConnection,
    _Inout_opt_ PLDAP_ROOT_DSE *ppRootDse
    )
{
    BOOL bResult = FALSE;
    DWORD dwResult = LDAP_SUCCESS;
    DWORD i = 0;
    PUTILS_HEAP pConnHeap = NULL;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting closing LDAP connection");

    //
    // Disconnection
    //
    if ((*ppConnection)->pLdapSession != NULL)
    {
        dwResult = ldap_unbind((*ppConnection)->pLdapSession);
        if (dwResult != LDAP_SUCCESS)
        {
            LdapWppMessage(TRACE_LEVEL_ERROR, CONNECTION, "Failed to unbind from LDAP server: <%#x>", dwResult);
            API_RETURN_ERROR(dwResult);
        }
        (*ppConnection)->pLdapSession = NULL;
        LdapWppMessage(TRACE_LEVEL_INFORMATION, CONNECTION, "Successfully unbinded from LDAP server");
    }

    //
    // Resources liberation
    //
    pConnHeap = (*ppConnection)->pConnectionHeap;

    if (ppRootDse != NULL && (*ppRootDse) != NULL)
    {
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptConfigurationNamingContext);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptCurrentTime);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptDefaultNamingContext);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptDnsHostName);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptDsServiceName);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptLdapServiceName);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptRootDomainNamingContext);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptSchemaNamingContext);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptServerName);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->extracted.ptSubschemaSubentry);

        UtilsHeapFreeAndNullArrayHelper(pConnHeap, (*ppRootDse)->extracted.pptNamingContexts, (*ppRootDse)->computed.count.dwNamingContextsCount, i);
        UtilsHeapFreeAndNullArrayHelper(pConnHeap, (*ppRootDse)->extracted.pptSupportedCapabilities, (*ppRootDse)->computed.count.dwSupportedCapabilitiesCount, i);
        UtilsHeapFreeAndNullArrayHelper(pConnHeap, (*ppRootDse)->extracted.pptSupportedControl, (*ppRootDse)->computed.count.dwSupportedControlCount, i);
        UtilsHeapFreeAndNullArrayHelper(pConnHeap, (*ppRootDse)->extracted.pptSupportedLDAPPolicies, (*ppRootDse)->computed.count.dwSupportedLDAPPoliciesCount, i);
        UtilsHeapFreeAndNullArrayHelper(pConnHeap, (*ppRootDse)->extracted.pptSupportedLDAPVersion, (*ppRootDse)->computed.count.dwSupportedLDAPVersionCount, i);
        UtilsHeapFreeAndNullArrayHelper(pConnHeap, (*ppRootDse)->extracted.pptSupportedSASLMechanisms, (*ppRootDse)->computed.count.dwSupportedSASLMechanismsCount, i);
        
        // TODO: free computed attributes
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->computed.misc.ptDnsDomainName);
        UtilsHeapFreeAndNullHelper(pConnHeap, (*ppRootDse)->computed.misc.ptDomainSid);

        UtilsHeapFreeAndNullHelper(pConnHeap, *ppRootDse);
    }

    UtilsHeapFreeAndNullHelper(pConnHeap, (*ppConnection)->ptHost);
    UtilsHeapFreeAndNullHelper(pConnHeap, *ppConnection);

    bResult = UtilsHeapDestroy(&pConnHeap);
    API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully closed LDAP connection");
    API_RETURN_SUCCESS();
}

DWORD LdapLastError(
    )
{
    return gs_dwLastError;
}

BOOL LdapInitRequestExW(
    _In_ const PLDAP_CONNECT pConnection,
    _In_opt_ const LPWSTR ptBase,
    _In_opt_ const LPWSTR ptFilter,
    _In_ const LDAP_REQ_SCOPE eScope,
    _In_opt_ const LPWSTR pptAttributes[],
    _In_opt_ PLDAPControl pServerControls[],
    _In_opt_ PLDAPControl pClientControls[],
    _Out_ PLDAP_REQUEST *ppRequest
    )
{
    BOOL bResult = FALSE;
    DWORD dwResult = LDAP_SUCCESS;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting LDAP request to server <%ws> with filter <%ws>", pConnection->ptHost, ptFilter);

    dwResult = ldap_check_filter(pConnection->pLdapSession, ptFilter);
    if (dwResult != LDAP_SUCCESS)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Invalid LDAP filter <%ws>: <err:%#08x>", ptFilter, dwResult);
        API_RETURN_ERROR(dwResult);
    }

    bResult = LdapBuildRequestObjectWithAttributes(pConnection, pptAttributes,  ppRequest);
    if (API_FAILED(bResult))
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Failed to build request object from attributes list: <err:%#08x>", LdapLastError());
        API_RETURN_SAME_ERROR();
    }

    // Fire ldap request
#pragma warning(suppress: 6387)
    (*ppRequest)->pLDAPSearch = ldap_search_init_page(pConnection->pLdapSession, ptBase,
        eScope, ptFilter, (*ppRequest)->pptAttributes, FALSE, pServerControls, pClientControls, LDAP_NO_LIMIT, LDAP_NO_LIMIT, NULL);
    
    if ((*ppRequest)->pLDAPSearch == NULL)
    {
        dwResult = LdapGetLastError();
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Failed to initiate request <%ws> to LDAP server <%ws>: <err:%#08x>", ptFilter, pConnection->ptHost, dwResult);
        API_RETURN_ERROR(dwResult);
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully initialized request <%ws> for server <%ws>", ptFilter, pConnection->ptHost);
    API_RETURN_SUCCESS();
}

BOOL LdapInitRequestExA(
    _In_ const PLDAP_CONNECT pConnection,
    _In_opt_ const LPSTR ptBase,
    _In_opt_ const LPSTR ptFilter,
    _In_ const LDAP_REQ_SCOPE eScope,
    _In_opt_ const LPSTR pptAttributes[],
    _In_opt_ PLDAPControl pServerControls[],
    _In_opt_ PLDAPControl pClientControls[],
    _Out_ PLDAP_REQUEST *ppRequest
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwBase = NULL;
    LPWSTR lpwFilter = NULL;
    LPWSTR *lpwAttributes = NULL;
    DWORD dwAttrCount = 0;

    if (ptBase != NULL) UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptBase, &lpwBase);
    if (ptFilter != NULL) UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptFilter, &lpwFilter);

    if (pptAttributes) {
        for (dwAttrCount = 0; pptAttributes[dwAttrCount] != NULL; ++dwAttrCount) {}
        if (pptAttributes != NULL) UtilsHeapAllocWStrArrAndConvertAStrArr(pConnection->pConnectionHeap, pptAttributes, dwAttrCount + 1, &lpwAttributes);
    }
    bReturn = LdapInitRequestExW(pConnection, lpwBase, lpwFilter, eScope, lpwAttributes, pServerControls, pClientControls, ppRequest);

    if (lpwBase != NULL) UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwBase);
    if (lpwFilter != NULL) UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwFilter);
    if (lpwAttributes != NULL) UtilsHeapFreeArrayHelper(pConnection->pConnectionHeap, lpwAttributes, dwAttrCount + 1);
    return bReturn;
}

BOOL LdapInitRequestW(
    _In_ const PLDAP_CONNECT pConnection,
    _In_opt_ const LPWSTR ptBase,
    _In_opt_ const LPWSTR ptFilter,
    _In_ const LDAP_REQ_SCOPE eScope,
    _In_opt_ const LPWSTR pptAttributes[],
    _Out_ PLDAP_REQUEST *ppRequest
    )
{
    PVOID pvServerControls = NULL;
    PVOID pvClientControls = NULL;

    return LdapInitRequestExW(pConnection, ptBase, ptFilter, eScope, pptAttributes, pvServerControls, pvClientControls, ppRequest);
}

BOOL LdapInitRequestA(
    _In_ const PLDAP_CONNECT pConnection,
    _In_opt_ const LPSTR ptBase,
    _In_opt_ const LPSTR ptFilter,
    _In_ const LDAP_REQ_SCOPE eScope,
    _In_opt_ const LPSTR pptAttributes[],
    _Out_ PLDAP_REQUEST *ppRequest
    )
{
    PVOID pvServerControls = NULL;
    PVOID pvClientControls = NULL;

    return LdapInitRequestExA(pConnection, ptBase, ptFilter, eScope, pptAttributes, pvServerControls, pvClientControls, ppRequest);
}

BOOL LdapGetNextEntry(
    _In_ const PLDAP_CONNECT pConnection,
    _In_ const PLDAP_REQUEST pRequest,
    _Out_ PLDAP_ENTRY *ppEntry
    )
{
    DWORD dwResult = LDAP_SUCCESS, dwCount = 0;
    BOOL bResult = FALSE;
    PLDAPMessage pCurrentEntry = NULL;
    struct l_timeval timeout = { .tv_sec = 100, .tv_usec = 0 }; // TODO: hardcoded timeout

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Getting next entry of LDAP request <%p>", pRequest->pLDAPSearch);

    (*ppEntry) = NULL;

    // Retrieve the first page or the next page of the set if needed
    if ((!pRequest->pLDAPCurrentPage) || (pRequest->dwCurrentEntryIndex == pRequest->dwCurrentPageEntries))
    {
        if (pRequest->pLDAPCurrentPage != NULL)
        {
           ldap_msgfree(pRequest->pLDAPCurrentPage);
        }
        dwResult = ldap_get_next_page_s(pConnection->pLdapSession, pRequest->pLDAPSearch, &timeout, 1, &dwCount, &pRequest->pLDAPCurrentPage); // TODO: page size == 1 only ??? (perf--)
        if (dwResult != LDAP_SUCCESS)
        {
            // If search return empty, we reached the end of the result set
            if (dwResult == LDAP_NO_RESULTS_RETURNED)
            {
                // Resturn NULL as marker set
                (*ppEntry) = NULL;
                LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Reached the last entry in the current page for LDAP request <%p>", pRequest->pLDAPSearch);
                API_RETURN_SUCCESS();
            }

            // Internal NTDSA server error
            if (dwResult == LDAP_OPERATIONS_ERROR)
            {
               // Resturn NULL as marker set
               (*ppEntry) = NULL;
               LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "LDAP server encounter an DSID error for LDAP request <%p>.", pRequest->pLDAPSearch);
               API_RETURN_SUCCESS();
            }

            LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Unexpected error during LDAP request <%p>: <%#x>", pRequest->pLDAPSearch, dwResult);
            ldap_unbind(pConnection->pLdapSession); // TODO: appeler les fonctions de destruction ici, plutot que de faire un unbind_directement
            API_RETURN_ERROR(dwResult);
        }

        // Count page entries
        pRequest->dwCurrentPageEntries = ldap_count_entries(pConnection->pLdapSession, pRequest->pLDAPCurrentPage);
        pRequest->dwCurrentEntryIndex = 0;
        LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Retrieved <%u> entries in the current page for LDAP request <%p>", pRequest->dwCurrentPageEntries, pRequest->pLDAPSearch);
    }

    // if we reach the end of the set return marker
    if (pRequest->dwCurrentPageEntries == 0)
    {
        LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Reached the last entry in the current page  for LDAP request <%p>", pRequest->pLDAPSearch);
        (*ppEntry) = NULL;
        API_RETURN_SUCCESS();
    }

    // Retrieve the first entry if index == 0
    if (pRequest->dwCurrentEntryIndex == 0)
    {
        pCurrentEntry = ldap_first_entry(pConnection->pLdapSession, pRequest->pLDAPCurrentPage);
    }
    else
    {
        pCurrentEntry = ldap_next_entry(pConnection->pLdapSession, pRequest->pLDAPCurrentPage);
    }
    if (pCurrentEntry == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Unable to retrieve %s entry in the current result page for LDAP request <%p>", (pRequest->dwCurrentEntryIndex == 0 ? "first": "next"), pRequest->pLDAPSearch);
        ldap_unbind(pConnection->pLdapSession);  // TODO: appeler les fonctions de destruction ici, plutot que de faire un unbind_directement
        API_RETURN_ERROR(LDAP_ERR_BAD_PAGE_DATA);
    }
    pRequest->dwCurrentEntryIndex++;

    // Extract entry result
    bResult = LdapExtractEntryDataFromLdapMsg(pConnection, pRequest, pCurrentEntry, ppEntry);
    if (API_FAILED(bResult))
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Unable to extract entry data from the current result page for LDAP request <%p>: <err:%#08x>", pRequest->pLDAPSearch, LdapLastError());
        API_RETURN_SAME_ERROR();
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully requested next entry for LDAP request <%p> to server <%ws>", pRequest->pLDAPSearch, pConnection->ptHost);
    API_RETURN_SUCCESS();
}

BOOL LdapReleaseValue(
    _In_ const PLDAP_CONNECT pConnection,
    _Inout_ PLDAP_VALUE *ppValue
    )
{
    (*ppValue)->dwSize = 0;
    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppValue)->pbData);
    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppValue));

    API_RETURN_SUCCESS();
}

BOOL LdapReleaseAttribute(
    _In_ const PLDAP_CONNECT pConnection,
    _Inout_ PLDAP_ATTRIBUTE *ppAttribute
    )
{
    DWORD i = 0;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting release of LDAP attribute <%p> (connection <%p>)", (*ppAttribute), pConnection);

    for (i = 0; i < (*ppAttribute)->dwValuesCount; i++)
    {
        LdapReleaseValue(pConnection, &(*ppAttribute)->ppValues[i]);
    }

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppAttribute)->ppValues);
    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppAttribute)->ptName);
    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppAttribute));

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully released LDAP entry");
    API_RETURN_SUCCESS();
}

BOOL LdapReleaseEntry(
    _In_ const PLDAP_CONNECT pConnection,
    _Inout_ PLDAP_ENTRY *ppEntry
    )
{
    DWORD i = 0;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting release of LDAP entry <%p> (connection <%p>)", (*ppEntry), pConnection);

    for (i = 0; i < (*ppEntry)->dwAttributesCount; i++)
    {
        LdapReleaseAttribute(pConnection, &(*ppEntry)->ppAttributes[i]);
    }

    (*ppEntry)->dwAttributesCount = 0;
    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppEntry)->ppAttributes);
    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppEntry)->ptDn);
    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppEntry));

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully released LDAP entry");
    API_RETURN_SUCCESS();
}

BOOL LdapReleaseRequest(
    _In_ const PLDAP_CONNECT pConnection,
    _Inout_ PLDAP_REQUEST *ppRequest
    )
{
    DWORD i = 0;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting release of LDAP request <%p> (connection <%p>)", (*ppRequest), pConnection);

    if ((*ppRequest)->pptAttributes != NULL)
    {
        for (i = 0; (*ppRequest)->pptAttributes[i] != NULL; i++)
        {
            UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppRequest)->pptAttributes[i]);
        }
        UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppRequest)->pptAttributes);
    }

    // TODO luc

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, (*ppRequest));

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Successfully released LDAP request");
    API_RETURN_SUCCESS();
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
    case DLL_PROCESS_ATTACH: bResult = LdapiLibInit(); break;
    case DLL_PROCESS_DETACH: bResult = LdapiLibCleanup(); break;
    default: bResult = TRUE; break;
    }

    return bResult;
}
