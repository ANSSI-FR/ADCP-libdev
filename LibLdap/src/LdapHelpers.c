/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#include "UtilsLib.h"
#include "LdapInternals.h"
#include "LdapHelpers.h"

#ifdef _WIN32
    #include "LdapWpp.h"
    #include "LdapHelpers.tmh"
#endif

/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
BOOL LdapLocateNamedAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_ PDWORD pdwIndex
    )
{
    UNREFERENCED_PARAMETER(pConnection);
    DWORD i = 0;
    (*pdwIndex) = 0;

    for (i = 0; i < pEntry->dwAttributesCount; i++)
    {
        // Sometimes, the LDAP server return attributes names with different cases
        // that one we request.We must use case insensitive string comparison.
        if (STR_EQI(ptAttrName, pEntry->ppAttributes[i]->ptName))
        {
            (*pdwIndex) = i;
            API_RETURN_SUCCESS(); // TODO: WPP message ?
        }
    }

    LdapWppMessage(TRACE_LEVEL_VERBOSE, REQUEST, _T("Failed to locate named attribute <%ws>"), ptAttrName);
    API_RETURN_ERROR(LDAP_NO_SUCH_ATTRIBUTE);
}

BOOL LdapLocateNamedAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_ PDWORD pdwIndex
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwAttrName = NULL;

    UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptAttrName, &lpwAttrName);

    bReturn = LdapLocateNamedAttrW(pConnection, pEntry, lpwAttrName, pdwIndex);

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwAttrName);
    return bReturn;
}

BOOL LdapExtractStrAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pStrAttr,
    _Out_writes_(dwStrCount) LPWSTR pptStr[],
    _In_ DWORD dwStrCount
    )
{
    DWORD i = 0;

    for (i = 0; i < dwStrCount; i++)
    {
        pptStr[i] = NULL;
    }

    if (dwStrCount > pStrAttr->dwValuesCount)
    {
        LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Failed to extract string attribute, requesting <%u> values, has <%u>", dwStrCount, pStrAttr->dwValuesCount);
        API_RETURN_ERROR(LDAP_ERR_TOO_MANY_VAL);
    }

    for (i = 0; i < dwStrCount; i++)
    {
        // LDAP manipulate ANSI Str which should be convert as LPWSTR
        UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, (LPSTR)pStrAttr->ppValues[i]->pbData, &pptStr[i]);
    }

    API_RETURN_SUCCESS();// TODO: WPP message ?
}

BOOL LdapExtractStrAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pStrAttr,
    _Out_writes_(dwStrCount) LPSTR pptStr[],
    _In_ DWORD dwStrCount
    )
{
    BOOL bReturn = FALSE;
    LPWSTR *ppwStrArr = NULL;

#pragma warning(suppress: 6001)
#pragma warning(suppress: 6054)
    UtilsHeapAllocWStrArrAndConvertAStrArr(pConnection->pConnectionHeap, pptStr, dwStrCount, &ppwStrArr);

    bReturn = LdapExtractStrAttrW(pConnection, pStrAttr, ppwStrArr, dwStrCount);

    if (bReturn) UtilsHeapAllocAStrArrAndConvertWStrArr(pConnection->pConnectionHeap, ppwStrArr, dwStrCount, &pptStr);
    UtilsHeapFreeArrayHelper(pConnection->pConnectionHeap, ppwStrArr, dwStrCount);
    return bReturn;
}

BOOL LdapExtractNumAttr(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pNumAttr,
    _Out_writes_(dwNumCount) DWORD pdwNum[],
    _In_ DWORD dwNumCount
    )
{
    DWORD i = 0;

    UNREFERENCED_PARAMETER(pConnection);

    if (dwNumCount > pNumAttr->dwValuesCount)
    {
        LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Failed to extract numeric attribute, requesting <%u> values, has <%u>", dwNumCount, pNumAttr->dwValuesCount);
        API_RETURN_ERROR(LDAP_ERR_TOO_MANY_VAL);
    }

    for (i = 0; i < dwNumCount; i++)
    {
        pdwNum[i] = 0;
    }

    for (i = 0; i < dwNumCount && i < pNumAttr->dwValuesCount; i++)
    {
        pdwNum[i] = pNumAttr->ppValues[i]->dwSize > 0 ? _tstoi((LPCWSTR)pNumAttr->ppValues[i]->pbData): 0;
    }

    API_RETURN_SUCCESS(); // TODO: WPP message ?
}

BOOL LdapExtractBoolAttr(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pBoolAttr,
    _Out_writes_(dwBoolCount) BOOL pbBool[],
    _In_ DWORD dwBoolCount
    )
{
    DWORD i = 0;

    UNREFERENCED_PARAMETER(pConnection);

    if (dwBoolCount > pBoolAttr->dwValuesCount)
    {
        LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Failed to extract boolean attribute, requesting <%u> values, has <%u>", dwBoolCount, pBoolAttr->dwValuesCount);
        API_RETURN_ERROR(LDAP_ERR_TOO_MANY_VAL);
    }

    for (i = 0; i < dwBoolCount; i++)
    {
        pbBool[i] = FALSE;
    }

    for (i = 0; i < dwBoolCount && i < pBoolAttr->dwValuesCount; i++)
    {
        pbBool[i] = (BOOL)(pBoolAttr->ppValues[i]->dwSize > 0 && STR_EQI(_T("TRUE"), (LPCWSTR)pBoolAttr->ppValues[i]->pbData));
    }

    API_RETURN_SUCCESS(); // TODO: WPP message ?
}

BOOL LdapExtractNamedStrAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_writes_(dwStrCount) LPWSTR pptStr[],
    _In_ DWORD dwStrCount
    )
{
    DWORD dwIndex = 0;

    if (LdapLocateNamedAttrW(pConnection, pEntry, ptAttrName, &dwIndex))
    {
        if (LdapExtractStrAttr(pConnection, pEntry->ppAttributes[dwIndex], pptStr, dwStrCount))
        {
            API_RETURN_SUCCESS(); // TODO: WPP message ?
        }
    }

    LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Failed to extract string named attribute <%ws>", ptAttrName);
    API_RETURN_SAME_ERROR();
}

BOOL LdapExtractNamedStrAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_writes_(dwStrCount) LPSTR pptStr[],
    _In_ DWORD dwStrCount
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwAttrName = NULL;
    LPWSTR ppwStr[] = { 0 };

    UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptAttrName, &lpwAttrName);

    bReturn = LdapExtractNamedStrAttrW(pConnection, pEntry, lpwAttrName, ppwStr, dwStrCount);

#pragma warning(suppress: 6001)
    if (bReturn) UtilsHeapAllocAStrArrAndConvertWStrArr(pConnection->pConnectionHeap, ppwStr, dwStrCount, &pptStr);
    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwAttrName);
    return bReturn;
}

BOOL LdapAllocAndExtractNamedStrAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_ LPWSTR *ppptStr[],
    _Out_ PDWORD pdwStrCount
    )
{
    DWORD dwIndex = 0;

    (*ppptStr) = NULL;
    (*pdwStrCount) = 0;

    if (LdapLocateNamedAttrW(pConnection, pEntry, ptAttrName, &dwIndex))
    {
        (*pdwStrCount) = pEntry->ppAttributes[dwIndex]->dwValuesCount;
        (*ppptStr) = UtilsHeapAllocArrayHelper(pConnection->pConnectionHeap, PWCHAR, (*pdwStrCount));

        if (LdapExtractStrAttrW(pConnection, pEntry->ppAttributes[dwIndex], (*ppptStr), (*pdwStrCount)))
        {
            API_RETURN_SUCCESS(); // TODO: WPP message ?
        }
    }

    LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Failed to alloc and extract string named attribute <%ws>", ptAttrName);
    API_RETURN_SAME_ERROR();
}

BOOL LdapExtractNamedNumAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_writes_(dwNumCount) PDWORD pdwNum,
    _In_ DWORD dwNumCount
    )
{
    DWORD dwIndex = 0;

    if (LdapLocateNamedAttr(pConnection, pEntry, ptAttrName, &dwIndex))
    {
        if (LdapExtractNumAttr(pConnection, pEntry->ppAttributes[dwIndex], pdwNum, dwNumCount))
        {
            API_RETURN_SUCCESS(); // TODO: WPP message ?
        }
    }

    LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Failed to extract numeric named attribute <%ws>", ptAttrName);
    API_RETURN_SAME_ERROR();
}

BOOL LdapExtractNamedNumAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_writes_(dwNumCount) PDWORD pdwNum,
    _In_ DWORD dwNumCount
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwAttrName = NULL;

    UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptAttrName, &lpwAttrName);

    bReturn = LdapExtractNamedNumAttrW(pConnection, pEntry, lpwAttrName, pdwNum, dwNumCount);

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwAttrName);
    return bReturn;
}

BOOL LdapExtractNamedBoolAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_writes_(dwBoolCount) PBOOL pbBool,
    _In_ DWORD dwBoolCount
    )
{
    DWORD dwIndex = 0;

    if (LdapLocateNamedAttr(pConnection, pEntry, ptAttrName, &dwIndex))
    {
        if (LdapExtractBoolAttr(pConnection, pEntry->ppAttributes[dwIndex], pbBool, dwBoolCount))
        {
            API_RETURN_SUCCESS(); // TODO: WPP message ?
        }
    }

    LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Failed to extract boolean named attribute <%ws>", ptAttrName);
    API_RETURN_SAME_ERROR();
}

BOOL LdapExtractNamedBoolAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_writes_(dwBoolCount) PBOOL pbBool,
    _In_ DWORD dwBoolCount
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwAttrName = NULL;

    UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptAttrName, &lpwAttrName);

    bReturn = LdapExtractNamedBoolAttrW(pConnection, pEntry, lpwAttrName, pbBool, dwBoolCount);

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwAttrName);
    return bReturn;
}

BOOL LdapDupVal(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_VALUE pSourceVal,
    _Out_ PLDAP_VALUE *ppDestVal
    )
{
    DWORD dwSize = pSourceVal->dwSize;;

    (*ppDestVal) = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_VALUE);
    (*ppDestVal)->pbData = UtilsHeapAllocHelper(pConnection->pConnectionHeap, dwSize + 1);
    (*ppDestVal)->dwSize = dwSize;
    CopyMemory((*ppDestVal)->pbData, pSourceVal->pbData, dwSize);

    API_RETURN_SUCCESS(); // TODO: WPP message ?
}

BOOL LdapDupAttr(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pSourceAttr,
    _Out_ PLDAP_ATTRIBUTE *ppDestAttr
    )
{
    DWORD i = 0;
    BOOL bResult = FALSE;

    (*ppDestAttr) = NULL;

    (*ppDestAttr) = UtilsHeapAllocStructHelper(pConnection->pConnectionHeap, LDAP_ATTRIBUTE);
    (*ppDestAttr)->ptName = UtilsHeapStrDupHelper(pConnection->pConnectionHeap, pSourceAttr->ptName);
    (*ppDestAttr)->ppValues = UtilsHeapAllocArrayHelper(pConnection->pConnectionHeap, PLDAP_VALUE, pSourceAttr->dwValuesCount);
    (*ppDestAttr)->dwValuesCount = pSourceAttr->dwValuesCount;
    for (i = 0; i < pSourceAttr->dwValuesCount; i++)
    {
        bResult = LdapDupVal(pConnection, pSourceAttr->ppValues[i], &(*ppDestAttr)->ppValues[i]);
        GOTO_FAIL_IF(API_FAILED(bResult), (void)bResult/*NONE*/);
    }

    API_RETURN_SUCCESS(); // TODO: WPP message ?

LABEL_FAIL:
    LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Failed to duplicate attribute");
    if ((*ppDestAttr) != NULL)
    {
        LdapReleaseAttribute(pConnection, ppDestAttr);
    }
    API_RETURN_ERROR(LDAP_NO_MEMORY);
}

BOOL LdapDupNamedAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _Out_ PLDAP_ATTRIBUTE *ppAttr
    )
{
    DWORD dwIndex = 0;

    (*ppAttr) = NULL;

    if (LdapLocateNamedAttr(pConnection, pEntry, ptAttrName, &dwIndex))
    {
        if (LdapDupAttr(pConnection, pEntry->ppAttributes[dwIndex], ppAttr))
        {
            API_RETURN_SUCCESS(); // TODO: WPP message ?
        }
    }

    LdapWppMessage(TRACE_LEVEL_WARNING, REQUEST, "Failed to duplicate named attribute <%ws>", ptAttrName);
    API_RETURN_SAME_ERROR();
}

BOOL LdapDupNamedAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _Out_ PLDAP_ATTRIBUTE *ppAttr
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwAttrName = NULL;

    UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptAttrName, &lpwAttrName);

    bReturn = LdapDupNamedAttrW(pConnection, pEntry, lpwAttrName, ppAttr);

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwAttrName);
    return bReturn;
}

BOOL LdapGetValue(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ATTRIBUTE pAttribute,
    _In_ DWORD dwValueIndex,
    _Out_ PLDAP_VALUE *ppValue
    )
{
    (*ppValue) = NULL;

    if (pAttribute->dwValuesCount > dwValueIndex)
    {
        if (pAttribute->ppValues[dwValueIndex] != NULL && pAttribute->ppValues[dwValueIndex]->pbData != NULL)
        {
            return LdapDupVal(pConnection, pAttribute->ppValues[dwValueIndex], ppValue);
        }
    }

    API_RETURN_ERROR(LDAP_NO_SUCH_VALUE);
}

BOOL LdapGetValueOfNamedAttrW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptAttrName,
    _In_ DWORD dwValueIndex,
    _Out_ PLDAP_VALUE *ppValue
    )
{
    BOOL bResult = FALSE;
    DWORD dwAttrIdx = 0;

    (*ppValue) = NULL;
    bResult = LdapLocateNamedAttr(pConnection, pEntry, ptAttrName, &dwAttrIdx);
    if (API_SUCCEEDED(bResult))
    {
        bResult = LdapGetValue(pConnection, pEntry->ppAttributes[dwAttrIdx], dwValueIndex, ppValue);
    }
    API_RETURN_SAME_VALUE(bResult);
}

BOOL LdapGetValueOfNamedAttrA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptAttrName,
    _In_ DWORD dwValueIndex,
    _Out_ PLDAP_VALUE *ppValue
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwAttrName = NULL;

    UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptAttrName, &lpwAttrName);

    bReturn = LdapGetValueOfNamedAttrW(pConnection, pEntry, lpwAttrName, dwValueIndex, ppValue);

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwAttrName);
    return bReturn;
}

BOOL LdapEntryIsClassOfW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPCWSTR ptClass
    )
{
    BOOL bResult = FALSE;
    DWORD dwAttrIdx = 0;
    DWORD i = 0;
    PLDAP_ATTRIBUTE pAttr = NULL;

    UNREFERENCED_PARAMETER(pConnection);

    bResult = LdapLocateNamedAttr(pConnection, pEntry, LDAP_ATTR_OBJECT_CLASS, &dwAttrIdx);
    if (API_SUCCEEDED(bResult))
    {
        pAttr = pEntry->ppAttributes[dwAttrIdx];
        for (i = 0; i < pAttr->dwValuesCount; i++)
        {
            if (STR_EQ(ptClass, (LPCWSTR)pAttr->ppValues[i]->pbData) == TRUE)
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL LdapEntryIsClassOfA(
    _In_ PLDAP_CONNECT pConnection,
    _In_ PLDAP_ENTRY pEntry,
    _In_ LPSTR ptClass
    )
{
    BOOL bReturn = FALSE;
    LPWSTR lpwClass = NULL;

    UtilsHeapAllocWStrAndConvertAStr(pConnection->pConnectionHeap, ptClass, &lpwClass);

    bReturn = LdapEntryIsClassOfW(pConnection, pEntry, lpwClass);

    UtilsHeapFreeAndNullHelper(pConnection->pConnectionHeap, lpwClass);
    return bReturn;
}

/*
    TODO: not needed right now.

BOOL LdapEscapeW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCWSTR ptString
    )
{
    // TODO: http://msdn.microsoft.com/en-us/library/aa746475.aspx#Special_Characters
    // {'*', '(', ')', '\\', '\0', '/' } ==> {"\2a", "\28", "\29", "\5c", "\00", "\2f"}
}

BOOL LdapEscapeA(
_In_ PLDAP_CONNECT pConnection,
_In_ LPCSTR ptString
)
{
    // TODO: http://msdn.microsoft.com/en-us/library/aa746475.aspx#Special_Characters
    // {'*', '(', ')', '\\', '\0', '/' } ==> {"\2a", "\28", "\29", "\5c", "\00", "\2f"}
}

BOOL LdapConstructFilterW(
    _In_ PLDAP_CONNECT pConnection,
    _In_ LPCWSTR ptFilterFormat,
    _In_ LPCWSTR ptFilterValues[],
    _Out_ LPWSTR ptConstructedFilter
    )
{
    // TODO
}

BOOL LdapConstructFilterA(
_In_ PLDAP_CONNECT pConnection,
_In_ LPCSTR ptFilterFormat,
_In_ LPCSTR ptFilterValues[],
_Out_ LPSTR ptConstructedFilter
)
{
    // TODO
}
*/
