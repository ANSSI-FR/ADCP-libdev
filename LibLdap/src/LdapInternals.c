/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#define LIB_ERROR_VAL gs_dwLdapLastError
#include "LdapInternals.h"

#ifdef _WIN32
    #include "LdapWpp.h"
    #include "LdapInternals.tmh"
#endif

/* --- PRIVATE VARIABLES ---------------------------------------------------- */
DWORD gs_dwLdapLastError = NO_ERROR;

/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
/*
PVOID LdapHeapAllocZeroMem(
    _In_ HANDLE hHeap,
    _In_ SIZE_T dwBytes
    )
{
    PVOID pvMemory = NULL;

    pvMemory = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, dwBytes);
    if (pvMemory == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, ALLOCATION, "Failed to allocate <%#x> bytes of memory with heap <%p>", (unsigned int)dwBytes, hHeap);
        // TODO: FATAL error ? stop execution ? exception ?
    }
    else
    {
        LdapWppMessage(TRACE_LEVEL_VERBOSE, ALLOCATION, "Allocated <%#x> bytes of memory with heap <%p>: <%p>", (unsigned int)dwBytes, hHeap, pvMemory);
    }

    return pvMemory;
}

PVOID LdapHeapReAlloc(
    _In_ HANDLE hHeap,
    _In_ PVOID pvMemory,
    _In_ SIZE_T dwBytes
    )
{
    PVOID pvMemoryLoc = NULL;

    pvMemoryLoc = HeapReAlloc(hHeap, HEAP_ZERO_MEMORY, pvMemory, dwBytes);
    if (pvMemoryLoc == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, ALLOCATION, "Failed to re-allocate <%p> with <%#x> bytes of memory with heap <%p>", pvMemory, (unsigned int)dwBytes, hHeap);
        // TODO: FATAL error ? stop execution ? exception ?
    }
    else
    {
#pragma warning(suppress: 6001)
        LdapWppMessage(TRACE_LEVEL_VERBOSE, ALLOCATION, "Re-allocated <%p> with <%#x> bytes of memory with heap <%p>: <%p>", pvMemory, (unsigned int)dwBytes, hHeap, pvMemoryLoc);
    }

    return pvMemoryLoc;
}

LPWSTR LdapHeapStrDup(
    _In_ HANDLE hHeap,
    _In_ LPCWSTR ptStr
    )
{
    DWORD dwLen = 0;
    LPWSTR ptDupStr = NULL;

    dwLen = (DWORD)_tcslen(ptStr);
    ptDupStr = UtilsHeapAllocStrHelper(hHeap, dwLen);
    if (ptDupStr == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, ALLOCATION, "Failed to dup <%p:%s> with heap <%p>", ptStr, ptStr, hHeap);
    }
    else
    {
        CopyMemory(ptDupStr, ptStr, sizeof(TCHAR)*dwLen);
        LdapWppMessage(TRACE_LEVEL_VERBOSE, ALLOCATION, "Dupped <%p> to <%p> with heap <%p>: <%s>", ptStr, ptDupStr, hHeap, ptDupStr);
    }

    return ptDupStr;
}

void LdapHeapFree(
    _In_ HANDLE hHeap,
    _In_ PVOID pvMemory
    )
{
    BOOL bResult = FALSE;

    bResult = HeapFree(hHeap, 0, pvMemory);
    if (bResult == FALSE)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, ALLOCATION, "Failed to free <%p> with heap <%p>", pvMemory, hHeap);
        // TODO: FATAL error ? stop execution ? exception ?
    }
    else
    {
#pragma warning(suppress: 6001)
        LdapWppMessage(TRACE_LEVEL_VERBOSE, ALLOCATION, "Freed <%p> with heap <%p>", pvMemory, hHeap);
    }
}
*/

BOOL LdapSetOption(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_OPTION pOption
    )
{
    DWORD dwResult = LDAP_SUCCESS;

    dwResult = ldap_set_option(pConnection->pLdapSession, pOption->iOption, pOption->pvValue);
    if (dwResult != LDAP_SUCCESS)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, CONFIGURATION, "Failed to set LDAP option <%#x>", (unsigned int)pOption->iOption);
        API_RETURN_ERROR(dwResult);
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, CONFIGURATION, "Successfully set LDAP option <%#x>", (unsigned int)pOption->iOption);
    API_RETURN_SUCCESS();
}

BOOL LdapExtractAttributes(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAPMessage pCurrentEntry,
    _In_ LPWSTR ptAttributeName,
    _Inout_ PLDAP_ATTRIBUTE *ppNewAttribute
    )
{
    struct berval **ppBvals = NULL;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting attribute <%ws> extraction", ptAttributeName);

    if (pCurrentEntry == NULL || ptAttributeName == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Invalid input parameters for LdapExtractAttributes");
        API_RETURN_SAME_ERROR();
    }

    if ((*ppNewAttribute) == NULL)
    {
        (*ppNewAttribute) = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_ATTRIBUTE);
        (*ppNewAttribute)->ptName = UtilsHeapStrDupHelper(pConnection->pConnectionHeap, ptAttributeName);
    }

    ppBvals = ldap_get_values_len(pConnection->pLdapSession, pCurrentEntry, ptAttributeName);
    if (ppBvals == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Extracted empty value for attribute <%ws>", (*ppNewAttribute)->ptName);
        goto FreeAndContinue;
    }

    (*ppNewAttribute)->ppValues = UtilsHeapAllocArrayHelper(pConnection->pConnectionHeap, PLDAP_VALUE, 1);
    (*ppNewAttribute)->ppValues[0] = NULL;

    for (DWORD j = 0; ppBvals[j] != NULL; ++j)
    {
        PLDAP_VALUE pNewValue = NULL;
        DWORD dwResult = 0;

        pNewValue = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_VALUE);
        pNewValue->pbData = UtilsHeapAllocArrayHelper(pConnection->pConnectionHeap, BYTE, (ppBvals[j]->bv_len + 1));

        if (memcpy_s(pNewValue->pbData, sizeof(BYTE)* (ppBvals[j]->bv_len + 1), ppBvals[j]->bv_val, sizeof(BYTE)* (ppBvals[j]->bv_len)))
        {
            dwResult = GLE();
            LdapWppMessage(TRACE_LEVEL_ERROR, MEMORY_MANAGEMENT, "Unable to copy attribute value: <gle:%#08x>", dwResult);
            API_RETURN_SAME_ERROR();
        }

        pNewValue->dwSize = ppBvals[j]->bv_len;

        (*ppNewAttribute)->dwValuesCount += 1;
        (*ppNewAttribute)->ppValues[j] = pNewValue;

        // TODO: pas vraiment optimisé... on alloue toujours en avance N+1 elements pour ppValues, quand N sont utilisés
        (*ppNewAttribute)->ppValues = UtilsHeapReallocArrayHelper(pConnection->pConnectionHeap, (*ppNewAttribute)->ppValues, PLDAP_VALUE, ((j + 1) + 1));
        (*ppNewAttribute)->ppValues[j + 1] = NULL;
    }
    LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Extracted <%u> values for attribute <%ws>", (*ppNewAttribute)->dwValuesCount, (*ppNewAttribute)->ptName);

FreeAndContinue:
    if (ppBvals != NULL)
    {
        ldap_value_free_len(ppBvals);
        ppBvals = NULL;
    }
    API_RETURN_SUCCESS();
}

BOOL LdapExtractRangedAttributes(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAPMessage pCurrentEntry,
    _In_ LPWSTR ptOrigAttribute,
    _In_ LPWSTR ptAttribute,
    _Inout_ PLDAP_ATTRIBUTE *ppNewAttribute
    )
{
    DWORD dwRet = 0;
    DWORD dwStart = 0, dwEnd = 0;
    LPWSTR ptCurrentDN = ldap_get_dn(pConnection->pLdapSession, pCurrentEntry);
    LDAPMessage *pLDAPSearchResult, *pLDAPEntry;
    LPWSTR ptTmpNewAttribute = NULL;
    LPWSTR ptNewRangeAttributes[2] = { NULL };
    ptNewRangeAttributes[0] = ptAttribute;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting range attribute extraction");

    if (pCurrentEntry == NULL || ptAttribute == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Invalid input parameters for LdapExtractRangedAttributes");
        API_RETURN_SAME_ERROR();
    }

    if ((*ppNewAttribute) == NULL)
    {
        (*ppNewAttribute) = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_ATTRIBUTE);
        (*ppNewAttribute)->ptName = UtilsHeapStrDupHelper(pConnection->pConnectionHeap, ptOrigAttribute);
    }

    (*ppNewAttribute)->ppValues = UtilsHeapAllocArrayHelper(pConnection->pConnectionHeap, PLDAP_VALUE, 1);
    (*ppNewAttribute)->ppValues[0] = NULL;

    do
    {
        DWORD dwCurrAttributeCount = 0;
        PBYTE *ppbCurrData = NULL;
        PDWORD pdwCurrDataSize = NULL;

        dwRet = ldap_search_s(pConnection->pLdapSession, ptCurrentDN, LDAP_SCOPE_BASE, _T("(objectClass=*)"), ptNewRangeAttributes, FALSE, &pLDAPSearchResult);
        if (dwRet != LDAP_SUCCESS)
        {
            LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Unable to process inner LDAP request for multivaluate attribute <%ws>: <%#x>", ptCurrentDN, dwRet);
            API_RETURN_ERROR(dwRet);
        }

        pLDAPEntry = ldap_first_entry(pConnection->pLdapSession, pLDAPSearchResult);
        if (pLDAPEntry == NULL)
        {
            LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Unable to process inner LDAP request for multivaluate attribute <%ws>", ptCurrentDN);
            API_RETURN_ERROR(LDAP_ERR_BAD_ENTRY_DATA);
        }

        if (API_FAILED(GetRangeValues(pConnection, pLDAPEntry, ptOrigAttribute, &dwCurrAttributeCount, &ppbCurrData, &pdwCurrDataSize, &dwStart, &dwEnd)))
        {
            LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Unable to extract range values during inner LDAP request for multivaluate attribute <%ws>", ptCurrentDN);
            API_RETURN_SAME_ERROR();
        }

        (*ppNewAttribute)->ppValues = UtilsHeapReallocArrayHelper(pConnection->pConnectionHeap, (*ppNewAttribute)->ppValues, PLDAP_VALUE, ((*ppNewAttribute)->dwValuesCount + dwCurrAttributeCount));

        for (DWORD j = 0; j < dwCurrAttributeCount; ++j)
        {
            DWORD dwValIdx = (*ppNewAttribute)->dwValuesCount + j;

            (*ppNewAttribute)->ppValues[dwValIdx] = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_VALUE);
            (*ppNewAttribute)->ppValues[dwValIdx]->pbData = ppbCurrData[j]; // TODO: pas de alloc+copy ? vérifier GetRangeValues()
            (*ppNewAttribute)->ppValues[dwValIdx]->dwSize = pdwCurrDataSize[j];
        }
        (*ppNewAttribute)->dwValuesCount += dwCurrAttributeCount;

        UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, ppbCurrData);
        UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, pdwCurrDataSize);
        UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, ptTmpNewAttribute);

        if (dwEnd != -1)
        {
            if (API_FAILED(ConstructRangeAtt(pConnection, ptOrigAttribute, dwStart + dwCurrAttributeCount, -1, &ptTmpNewAttribute)))
            {
                LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Unable to build new attribute range values during inner LDAP request for multivaluate attribute <%ws>", ptCurrentDN);
                API_RETURN_SAME_ERROR();
            }
            ptNewRangeAttributes[0] = ptTmpNewAttribute;
        }
    } while (dwEnd != -1);

    LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Extracted <%u> values for ranged attribute <%ws>", (*ppNewAttribute)->dwValuesCount, (*ppNewAttribute)->ptName);
    API_RETURN_SUCCESS();
}

BOOL GetRangeValues(
    _In_ PLDAP_CONNECT pConnection,
    _Inout_ PLDAPMessage pEntry,
    _In_ LPWSTR ptOrigAttribute,
    _Out_ PDWORD pdwAttributeCount,
    _Out_ PBYTE **pppbData,
    _Out_ PDWORD *ppdwDataSize,
    _Out_ PDWORD pdwStart,
    _Out_ PDWORD pdwEnd
    )
{
    LPWSTR ptAttribute = NULL;
    BerElement *pBerElt = NULL;
    DWORD dwAttributeCount = 0;
    PBYTE *ppbData = NULL;
    PDWORD pdwDataSize = NULL;
    struct berval **ppBvals = NULL;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting range attribute values extraction");

    (*pdwAttributeCount) = 0;
    (*pppbData) = NULL;
    (*ppdwDataSize) = NULL;
    (*pdwStart) = 0;
    (*pdwEnd) = 0;

    ptAttribute = ldap_first_attribute(pConnection->pLdapSession, pEntry, &pBerElt);
    if (ptAttribute == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Unable to extract range values: there is no attributes for the current entry: <%p>", pEntry);
        API_RETURN_SAME_ERROR(); // TODO: mauvaise macro
    }

    if (API_FAILED(ParseRange(ptOrigAttribute, ptAttribute, pdwStart, pdwEnd)))
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Unable to parse attribute range values during inner LDAP request for multivaluate attribute <%ws>", ptAttribute);
        API_RETURN_SAME_ERROR();
    }

    ppBvals = ldap_get_values_len(pConnection->pLdapSession, pEntry, ptAttribute);
    if (ppBvals == NULL)
    {
        goto FreeAndContinue;
    }

    for (DWORD j = 0; ppBvals[j] != NULL; ++j)
    {
        PBYTE pbTmpData = NULL;

        ppbData = UtilsHeapAllocOrReallocHelper(pConnection->pConnectionHeap, ppbData, SIZEOF_ARRAY(PBYTE, (j + 1)));
        pdwDataSize = UtilsHeapAllocOrReallocHelper(pConnection->pConnectionHeap, pdwDataSize, SIZEOF_ARRAY(DWORD, (j + 1)));
        pbTmpData = UtilsHeapAllocArrayHelper(pConnection->pConnectionHeap, BYTE, (ppBvals[j]->bv_len + 1));

        if (memcpy_s(pbTmpData, SIZEOF_ARRAY(BYTE, (ppBvals[j]->bv_len + 1)), ppBvals[j]->bv_val, SIZEOF_ARRAY(BYTE, (ppBvals[j]->bv_len))))
        {
            LdapWppMessage(TRACE_LEVEL_ERROR, MEMORY_MANAGEMENT, "Unable to copy requested ranged attributes");
            API_RETURN_SAME_ERROR();
        }

        ppbData[j] = pbTmpData;
        pdwDataSize[j] = ppBvals[j]->bv_len;
        dwAttributeCount++;
    }

FreeAndContinue:
    if (ppBvals != NULL)
    {
        ldap_value_free_len(ppBvals);
        ppBvals = NULL;
    }
    if (ptAttribute != NULL)
    {
        ldap_memfree(ptAttribute);
        ptAttribute = NULL;
    }
    if (pBerElt != NULL)
    {
        ber_free(pBerElt, 0);
    }
    (*pdwAttributeCount) = dwAttributeCount;
    (*pppbData) = ppbData;
    (*ppdwDataSize) = pdwDataSize;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Extracted <%u> values for ranged attribute", dwAttributeCount);
    API_RETURN_SUCCESS();
}

BOOL ParseRange(
    _In_ LPWSTR ptAtttype,
    _In_ LPWSTR ptAttdescr,
    _Out_ PDWORD pdwStart,
    _Out_ PDWORD pdwEnd
    )
{
    LPWSTR ptRangeStr = LDAP_RANGE_MAGIC;
    LPWSTR ptStartstring = NULL, ptEndstring = NULL, ptOptionstart = NULL;
    INT iAtttypeLen = 0, iAttdescrLen = 0;
    DWORD dwRangestringlen = 0, dwCpt = 0;
    BOOL bRangefound = FALSE;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting range parsing <type:%ws> <desc:%ws>", ptAtttype, ptAttdescr);

    dwRangestringlen = (DWORD)_tcslen(ptRangeStr);

    if (ptAtttype == NULL || ptAttdescr == NULL)
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Invalid input parameters for ParseRange");
        API_RETURN_SAME_ERROR();
    }

    if (STR_EQI(ptAtttype, ptAttdescr))
    {
        // The attribute was returned without options.
        (*pdwStart) = 0;
        (*pdwEnd) = (DWORD)-1;
        LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Unable to parse range values: there is no range for the current attribute: <%ws>", ptAttdescr);
        API_RETURN_SUCCESS();
    }

    iAtttypeLen = (INT)_tcslen(ptAtttype);
    iAttdescrLen = (INT)_tcslen(ptAttdescr);

    if ((iAtttypeLen > iAttdescrLen) || (';' != ptAttdescr[iAtttypeLen]) || (_tcsnicmp(ptAtttype, ptAttdescr, iAtttypeLen)))
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, REQUEST, "Unable to parse range values: invalid range separator");
        API_RETURN_SAME_ERROR();
    }

    // It is the correct attribute type. Verify that if there is a range option.
    (*pdwStart) = 0;
    (*pdwEnd) = (DWORD)-1;
    ptOptionstart = ptAttdescr + iAtttypeLen + 1;
    do
    {
        if ((iAttdescrLen - (ptOptionstart - ptAttdescr)) < (INT)dwRangestringlen)
        {
            // No space left in the string for range option.
            ptOptionstart = ptAttdescr + iAttdescrLen;
        }
        else if (_tcsncicmp(ptOptionstart, ptRangeStr, dwRangestringlen) == 0)
        {
            // Found a range string. Ensure that it looks like what is expected and then parse it.
            ptStartstring = ptOptionstart + dwRangestringlen;
            for (dwCpt = 0; _istdigit((unsigned char)ptStartstring[dwCpt]); dwCpt++);

            if ((0 != dwCpt) && ('-' == ptStartstring[dwCpt]) && ((ptStartstring[dwCpt + 1] == '*') || _istdigit((unsigned char)ptStartstring[dwCpt + 1])))
            {
                // Acceptable. Finish parsing.
                ptEndstring = &ptStartstring[dwCpt + 1];
                (*pdwStart) = _tcstol(ptStartstring, NULL, 10);
                if (ptEndstring[0] == '*')
                    (*pdwEnd) = (DWORD)-1;
                else
                    (*pdwEnd) = _tcstol(ptEndstring, NULL, 10);
                bRangefound = TRUE;
            }
        }

        // If necessary, advance to the next option.
        if (bRangefound == FALSE)
        {
            while ((*ptOptionstart != '\0') && (*ptOptionstart != ';'))
                ptOptionstart++;

            // Skip the semicolon.
            ptOptionstart++;
        }
    } while (bRangefound == FALSE && (iAttdescrLen > (ptOptionstart - ptAttdescr)));
    LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Extracted new range for attribute <%ws>: <start:%u> <end:%u>", ptAtttype, *pdwStart, *pdwEnd);
    API_RETURN_SUCCESS();
}

BOOL ConstructRangeAtt(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCWSTR ptAttrType,
    _In_ DWORD dwStart,
    _In_ INT iEnd,
    _Inout_ LPWSTR* tOutputRangeAttr
    )
{
    TCHAR startstring[11], endstring[11];
    DWORD requiredlen = 0;
    LPWSTR ptOutbuff = NULL;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting build of an attribute range <type:%ws> <start:%u> <end:%d>", ptAttrType, dwStart, iEnd);

    startstring[10] = _T('\0');
    endstring[10] = _T('\0');

    // Calculate buffer space required.
    _sntprintf_s(startstring, 11, 10, _T("%u"), dwStart);
    if (iEnd == -1)
    {
        _tcscpy_s(endstring, 11, _T("*"));
    }
    else
    {
        _sntprintf_s(endstring, 11, 10, _T("%u"), (UINT)iEnd);
    }

    // Add in space for ';range=' and '-' and the terminating null.
    requiredlen = (DWORD)(_tcslen(ptAttrType) + _tcslen(startstring) + _tcslen(endstring));
    requiredlen += 9; // TODO: + x*sizeof(TCHAR)

    ptOutbuff = UtilsHeapAllocStrHelper(pConnection->pConnectionHeap, requiredlen);
    _sntprintf_s(ptOutbuff, (requiredlen + 1), requiredlen, _T("%s;range=%s-%s"), ptAttrType, startstring, endstring); // TODO: warning, %ws format specifier may not be portable in non-unicode env
    ptOutbuff[requiredlen] = _T('\0');

    (*tOutputRangeAttr) = ptOutbuff;
    LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Build new attribute range successfully: <%ws>", (*tOutputRangeAttr));
    API_RETURN_SUCCESS();
}

BOOL IsRangedAtt(
    _In_ LPCWSTR ptAttribute,
    _In_opt_ LPCWSTR ptOrigAttribute
    )
{
    // TODO: simplifier
    /*
    if (
        (ptOrigAttribute && !STR_EQI(ptOrigAttribute, ptAttribute)) || // If we have a reference that's easy
        (_tcsstr(ptAttribute, LDAP_RANGE_MAGIC)) // Otherwise, we need to test the string
    )
    {
        LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Attribute <%s> is a ranged one", ptAttribute);
        API_RETURN_SUCCESS();
    }
    else
    {
        LdapWppMessage(TRACE_LEVEL_INFORMATION, REQUEST, "Attribute <%s> is not a ranged one", ptAttribute);
        API_RETURN_SAME_ERROR(); // TODO: changer pour un "API_RETURN_ERROR()", car SAME_ERROR s'utilise apres un appel à une fct de la libLdap
    }
    */

    // If we have a reference that's easy
    if (ptOrigAttribute)
    {
        if (!_tcsicmp(ptOrigAttribute, ptAttribute))
        {
            API_RETURN_SAME_ERROR()
        }
        else
        {
            API_RETURN_SUCCESS();
        }
    }

    // Otherwise, we need to parse test the string
    if (_tcsstr(ptAttribute, LDAP_RANGE_MAGIC))
    {
        API_RETURN_SUCCESS()
    }
    else
    {
        API_RETURN_SAME_ERROR();
    }
}

BOOL ExtractOrigAttFromRangedAtt(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCWSTR tAttribute,
    _Out_ LPWSTR *pptOrigAttribute
    )
{
    DWORD dwStrLen = 0;
    LPWSTR ptPosition = NULL;

    LdapWppMessage(TRACE_LEVEL_INFORMATION, INITIALIZATION, "Starting original attribute extraction from ranged one");

    dwStrLen = (DWORD)_tcslen(tAttribute);
    ptPosition = _tcsrchr(tAttribute, ';');
    (*pptOrigAttribute) = NULL;

    if ((ptPosition == NULL) || (ptPosition >= (tAttribute + dwStrLen - 1)))
    {
        // TODO: WPP message ?
        API_RETURN_SAME_ERROR();
    }

    (*pptOrigAttribute) = UtilsHeapAllocStrHelper(pConnection->pConnectionHeap, (DWORD)(ptPosition - tAttribute));

    if (memcpy_s((*pptOrigAttribute), sizeof(TCHAR)* (ptPosition - tAttribute + 1), tAttribute, sizeof(TCHAR)* (ptPosition - tAttribute - 1)))
    {
        LdapWppMessage(TRACE_LEVEL_ERROR, MEMORY_MANAGEMENT, "Unable to copy original attribute name into result buffer");
        API_RETURN_SAME_ERROR();
    }

    LdapWppMessage(TRACE_LEVEL_INFORMATION, FINALIZATION, "Extracted original attribute from ranged one: <%ws>", (*pptOrigAttribute));
    API_RETURN_SUCCESS();
}

_Ret_notnull_ LPWSTR LdapExtractAttrName(
    _In_ const PLDAP_CONNECT pConnection,
    _In_ const LPWSTR pAttrNameWithRange
    )
{
    LPWSTR pName = UtilsHeapStrDupHelper(pConnection->pConnectionHeap, pAttrNameWithRange);
    LPWSTR pSemicolon = _tcschr(pName, _T(';'));

    if (pSemicolon != NULL)
    {
        (*pSemicolon) = _T('\0');
    }

    return pName;
}
