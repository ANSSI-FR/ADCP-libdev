/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#include "UtilsInternals.h"
#include "UtilsLib.h"

/* --- PRIVATE VARIABLES ---------------------------------------------------- */
static DWORD gs_dwLibLastError = NO_ERROR;
static PUTILS_HEAP gs_pUtilsHeap = NULL;

/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
static BYTE ChartoInt(CHAR c) {
    DWORD dwRes = 0;
    if (c >= '0' && c <= '9') dwRes = c - '0';
    if (c >= 'A' && c <= 'F') dwRes = 10 + c - 'A';
    if (c >= 'a' && c <= 'f') dwRes = 10 + c - 'a';
    return (BYTE) dwRes;
}

/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
BOOL
UtilsLibInit (
)
{
   BOOL bResult = UtilsHeapCreate(&gs_pUtilsHeap, UTILS_HEAP_NAME, NULL); // TODO!: callback
   API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());  // TODO: WPP

   API_RETURN_SUCCESS();
}

BOOL
UtilsLibCleanup (
)
{
   BOOL bResult = UtilsHeapDestroy(&gs_pUtilsHeap);
   API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());  // TODO: WPP

   API_RETURN_SUCCESS();
}

BOOL UtilsHeapCreate(
    _Out_ PUTILS_HEAP *ppHeap,
    _In_ const PTCHAR ptName,
    _In_opt_ const PFN_UTILS_HEAP_FATAL_ERROR_HANDLER pfnFatalErrorHandler
    ) {
    HANDLE hHeap = NULL;

    (*ppHeap) = NULL;

    hHeap = HeapCreate(0, 0, 0);
    if (hHeap == NULL) {
        API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO); // TODO:WPP+ERR
    }

    (*ppHeap) = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, sizeof(UTILS_HEAP));
    if ((*ppHeap) == NULL) {
        HeapDestroy(hHeap);
        API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO); // TODO:WPP+ERR
    }

    (*ppHeap)->hHeap = hHeap;
    (*ppHeap)->pfnFatalErrorHandler = pfnFatalErrorHandler;
    (*ppHeap)->ptName = UtilsHeapStrDupHelper((*ppHeap), ptName);

    API_RETURN_SUCCESS();
}

BOOL UtilsHeapDestroy(
    _Inout_ PUTILS_HEAP *ppHeap
    ) {
    BOOL bResult = FALSE;
    HANDLE hHeap = (*ppHeap)->hHeap;

    UtilsHeapFreeAndNullHelper((*ppHeap), (*ppHeap)->ptName);
    UtilsHeapFreeAndNullHelper((*ppHeap), (*ppHeap));

    bResult = HeapDestroy(hHeap);
    API_RETURN_ERROR_IF_FAILED(bResult, GLE()); // TODO:WPP+ERR

    API_RETURN_SUCCESS();
}

_Ret_notnull_ PVOID UtilsHeapAlloc(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const DWORD dwSize
    ) {
    PVOID pvMem = HeapAlloc(pHeap->hHeap, HEAP_ZERO_MEMORY, dwSize);
    UTILS_HEAP_FATAL_IF(pvMem == NULL, pHeap, _T("cannot allocate <%u> bytes"), dwSize); // TODO:WPP+ERR
    return pvMem;
}

_Ret_notnull_ PVOID UtilsHeapRealloc(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PVOID pvMem,
    _In_ const DWORD dwNewSize
    ) {
    PVOID pvNewMem = HeapReAlloc(pHeap->hHeap, HEAP_ZERO_MEMORY, pvMem, dwNewSize);
    UTILS_HEAP_FATAL_IF(pvNewMem == NULL, pHeap, _T("cannot re-allocate memory <%p> with <%u> bytes"), pvMem, dwNewSize); // TODO:WPP+ERR
    return pvNewMem;
}

_Ret_notnull_ PVOID UtilsHeapAllocOrRealloc(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_opt_ const PVOID pvMem,
    _In_ const DWORD dwNewSize
    ) {
    if (pvMem == NULL) {
        return UtilsHeapAlloc(ptCaller, pHeap, dwNewSize);
    }
    else {
        return UtilsHeapRealloc(ptCaller, pHeap, pvMem, dwNewSize);
    }
}

_Ret_notnull_ PVOID UtilsHeapStrDup(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PTCHAR ptStr
    ) {
    return UtilsHeapMemDup(ptCaller, pHeap, ptStr, SIZEOF_TSTR((DWORD)_tcslen(ptStr)));
}

_Ret_notnull_ PVOID UtilsHeapMemDup(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PVOID pvMem,
    _In_ const DWORD dwSize
    ) {
    PVOID pvDupMem = UtilsHeapAlloc(ptCaller, pHeap, dwSize);
    CopyMemory(pvDupMem, pvMem, dwSize); // TODO:WPP+ERR

    return pvDupMem;
}

VOID UtilsHeapFree(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PVOID pMem
    ) {
    BOOL bResult = TRUE;
    if (pMem)
        bResult = HeapFree(pHeap->hHeap, 0, pMem);
    UTILS_HEAP_FATAL_IF(bResult == FALSE, pHeap, _T("cannot free memory <%p>"), pMem); // TODO:WPP+ERR
}

VOID UtilsHeapFreeArray(
    _In_ const PTCHAR ptCaller,
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PVOID *ppMemArr,
    _In_ const DWORD dwCount
    ) {
    UNREFERENCED_PARAMETER(ptCaller);

    if (ppMemArr == NULL) {
        return;
    }
    for (DWORD i = 0; i < dwCount; ++i) {
        UtilsHeapFreeHelper(pHeap, (ppMemArr[i]));
    }
    UtilsHeapFreeHelper(pHeap, ppMemArr);
}

void HexifyA(
    _In_ const LPSTR ptOutHexStr,
    _In_ const PBYTE pbInData,
    _In_ const DWORD dwLen // ptOutHexStr must be able to receive dwLen *2 chars + null
    ) {
    DWORD i = 0;
    const static CHAR acConv[] = "0123456789ABCDEF";
    for (i = 0; i < dwLen; i++) {
        ptOutHexStr[(2 * i) + 0] = acConv[((pbInData[i] & 0xF0) >> 4)];
        ptOutHexStr[(2 * i) + 1] = acConv[((pbInData[i] & 0x0F) >> 0)];
    }
    ptOutHexStr[2 * dwLen] = '\0';
}

void HexifyW(
    _In_ const LPWSTR ptOutHexStr,
    _In_ const PBYTE pbInData,
    _In_ const DWORD dwLen // ptOutHexStr must be able to receive dwLen *2 chars + null
) {
    DWORD i = 0;
    const static WCHAR acConv[] = L"0123456789ABCDEF";
    for (i = 0; i < dwLen; i++) {
        ptOutHexStr[(2 * i) + 0] = acConv[((pbInData[i] & 0xF0) >> 4)];
        ptOutHexStr[(2 * i) + 1] = acConv[((pbInData[i] & 0x0F) >> 0)];
    }
    ptOutHexStr[2 * dwLen] = L'\0';
}

void UnhexifyA(
    _In_ const PBYTE pbOutData,
    _In_ const LPSTR ptInStr // pbOutData must be able to receive strlen(ptInStr)/2 bytes
    ) {
    DWORD i = 0;
    size_t len = strlen(ptInStr) / 2;
    for (i = 0; i < len; i++) {
        pbOutData[i] = 16 * ChartoInt(ptInStr[(2 * i) + 0]) + ChartoInt(ptInStr[(2 * i) + 1]);
    }
}

void UnhexifyW(
    _In_ const PBYTE pbOutData,
    _In_ const LPWSTR ptInStr // pbOutData must be able to receive strlen(ptInStr)/2 bytes
    ) {
    LPSTR lpaInStr = NULL;
    UtilsHeapAllocAStrAndConvertWStr(gs_pUtilsHeap, ptInStr, &lpaInStr);
    UnhexifyA(pbOutData, lpaInStr);
    UtilsHeapFreeAndNullHelper(gs_pUtilsHeap, lpaInStr);
}

BOOL IsNumericW(
    _In_ const LPCWSTR ptStr
    ) {
    DWORD dwLen = (DWORD)wcslen(ptStr);

    if ((dwLen == 0) || (iswdigit(ptStr[0]) == 0 && ptStr[0] != L'+' && ptStr[0] != L'-')) {
        return FALSE;
    }

    for (DWORD i = 1; i < dwLen; i++) {
        if (iswdigit(ptStr[i]) == 0) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL IsNumericA(
    _In_ const LPCSTR ptStr
    ) {
    LPWSTR lpwStr = NULL;
    BOOL bResult = FALSE;

    UtilsHeapAllocWStrAndConvertAStr(gs_pUtilsHeap, ptStr, &lpwStr);
    bResult = IsNumericW(lpwStr);
    UtilsHeapFreeAndNullHelper(gs_pUtilsHeap, lpwStr);

    return bResult;
}

BOOL IsInSetOfStrings(
    _In_ const PTCHAR ptNeedle,
    _In_ const PTCHAR ptHaystack[],
    _In_ const DWORD dwSetSize,
    _Out_opt_ DWORD *pdwIndex
    ) {
    DWORD i = 0;

    SET_PTRVAL_IF_NOT_NULL(pdwIndex, 0);

    for (i = 0; i < dwSetSize; i++) {
        if (STR_EQ(ptNeedle, ptHaystack[i])) {
            SET_PTRVAL_IF_NOT_NULL(pdwIndex, i);
            return TRUE;
        }
    }

    return FALSE;
}

BOOL StrNextToken(
    _In_ const PTCHAR ptStr,
    _In_ const PTCHAR ptDelim,
    _Inout_ PTCHAR *pptCtx,
    _Out_ PTCHAR *pptTok
    ) {
    if ((*pptCtx) == NULL) {
        (*pptTok) = _tcstok_s(ptStr, ptDelim, pptCtx);
    }
    else {
        (*pptTok) = _tcstok_s(NULL, ptDelim, pptCtx);
    }
    return ((*pptTok) != NULL);
}

DWORD UtilsGetLastError(
    ) {
    return gs_dwLibLastError;
}

BOOL SetPrivilege(
    _In_ const HANDLE hToken,
    _In_ const PTCHAR ptPriv,
    _In_ const BOOL bEnablePrivilege
    ) {
    // From: http://msdn.microsoft.com/en-us/library/windows/desktop/aa446619.aspx

    TOKEN_PRIVILEGES sTp;
    LUID sLuid;

    if (!LookupPrivilegeValue(
        NULL,            // lookup privilege on local system
        ptPriv,   // privilege to lookup
        &sLuid))        // receives LUID of privilege
    {
        //LOG(Err, "LookupPrivilegeValue error: <gle:%#08x>", GLE()); // TODO!: nolog
        return FALSE;
    }

    sTp.PrivilegeCount = 1;
    sTp.Privileges[0].Luid = sLuid;
    if (bEnablePrivilege)
        sTp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    else
        sTp.Privileges[0].Attributes = 0;

    // Enable the privilege or disable all privileges.
    if (!AdjustTokenPrivileges(
        hToken,
        FALSE,
        &sTp,
        sizeof(TOKEN_PRIVILEGES),
        (PTOKEN_PRIVILEGES)NULL,
        (PDWORD)NULL))
    {
        //LOG(Err, "AdjustTokenPrivileges error: <gle:%#08x>", GLE());  // TODO!: nolog
        return FALSE;
    }

    return (GLE() == ERROR_SUCCESS);
}

BOOL EnablePrivForCurrentProcess(
    _In_ const PTCHAR ptPriv
    ) {
    BOOL bResult = FALSE;
    HANDLE hToken = 0;

    bResult = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken);
    if (!bResult) {
        // LOG(Err, "Cannot open process token: <gle:%#08x>", GLE()); // TODO!: nolog
        return FALSE;
    }

    bResult = SetPrivilege(hToken, ptPriv, TRUE);
    if (!bResult && GLE() == ERROR_NOT_ALL_ASSIGNED) {
        // LOG(Err, "Current process does not have the privilege <%s>", szPrivilege); // TODO!: nolog
        return FALSE;
    }

    return TRUE;
}

HANDLE FileOpenWithBackupPriv(
    _In_ PTCHAR ptPath,
    _In_ BOOL bUseBackupPriv
    ) {
    // Actually, http://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx:
    // FILE_FLAG_BACKUP_SEMANTICS: You must set this flag to obtain a handle to a directory
    // To open a directory using CreateFile, specify the FILE_FLAG_BACKUP_SEMANTICS flag as part of dwFlagsAndAttributes. Appropriate security checks still apply when this flag is used without SE_BACKUP_NAME and SE_RESTORE_NAME privileges.
    // => So we always use this flag.
    UNREFERENCED_PARAMETER(bUseBackupPriv);
    return CreateFile(ptPath, READ_CONTROL, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
}

BOOL IsDomainSid(
    _In_ const PSID pSid
    ) {
    static const SID_IDENTIFIER_AUTHORITY sSidIdAuthNtSecurity = SECURITY_NT_AUTHORITY;
    PUCHAR n = GetSidSubAuthorityCount(pSid);
    if ((*n) >= 1) {
        SID_IDENTIFIER_AUTHORITY *pIdAuth = GetSidIdentifierAuthority(pSid);
        if (memcmp(pIdAuth, &sSidIdAuthNtSecurity, sizeof(SID_IDENTIFIER_AUTHORITY)) == 0) {
            PDWORD pdwAuth = GetSidSubAuthority(pSid, 0);
            if (((*pdwAuth) == SECURITY_NT_NON_UNIQUE) || ((*pdwAuth) == SECURITY_BUILTIN_DOMAIN_RID)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL MemoryIsNull(
    _In_ const PBYTE pbMem,
    _In_ const DWORD dwLen
    ) {
    DWORD i = 0;
    for (i = 0; i < dwLen; i++) {
        if (pbMem[i] != 0) {
            return FALSE;
        }
    }
    return TRUE;
}

BOOL ConvertStrGuidToGuid(
    _In_ const PTCHAR ptGuid,
    _In_ GUID * pGuid
    ) {
    WCHAR awFormated[GUID_STR_SIZE_NULL] = { 0 };
    size_t size = 0;
    HRESULT hResult = NOERROR;

    RtlZeroMemory(pGuid, sizeof(GUID));

    size = swprintf(awFormated, _countof(awFormated), ptGuid[0] != '{' ? L"{%S}" : L"%S", ptGuid);
    if (size == -1) {
        // TODO: nolog FATAL(_T("Cannot format GUID <%s> for conversion"), ptGuid);
        API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
    }

    hResult = CLSIDFromString((LPCOLESTR)awFormated, pGuid);
    if (hResult == NOERROR) {
        API_RETURN_SUCCESS();
    }
    else {
        API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
    }
}

void AddStrPair(
    _In_ const PUTILS_HEAP pHeap,
    _Inout_ PSTR_PAIR_LIST *ppEnd,
    _In_ const PTCHAR ptName,
    _In_ const PTCHAR ptValue
    ) {
    PSTR_PAIR_LIST pair = UtilsHeapAllocStructHelper(pHeap, STR_PAIR_LIST);
    pair->next = NULL;
    pair->name = ptName;
    pair->val = ptValue;

    if ((*ppEnd) != NULL) {
        (*ppEnd)->next = pair;
    }
    *ppEnd = pair;
}

PTCHAR GetStrPair(
    _In_ const PSTR_PAIR_LIST pHead,
    _In_ PTCHAR ptName
    ) {
    PSTR_PAIR_LIST pCurr = pHead;

    while (pCurr != NULL) {
        if (STR_EQ(ptName, pCurr->name)) {
            return pCurr->val;
        }
        pCurr = pCurr->next;
    }

    return NULL;
}

void DestroyStrPairList(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const PSTR_PAIR_LIST pHead
    ) {
    PSTR_PAIR_LIST pCurr = pHead;
    PSTR_PAIR_LIST pTmp = NULL;

    while (pCurr != NULL) {
        pTmp = pCurr;
        pCurr = pCurr->next;
        UtilsHeapFreeAndNullHelper(pHeap, pTmp);
    }
}

#pragma optimize( "", off )
DWORD GenerateException(
    ) {
    DWORD x = 0;
    x = 1 / x;
    return x;
}
#pragma optimize( "", on )

BOOL ConvertAstrToWstr(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const LPCSTR pAnsiStr,
    _Inout_ LPWSTR *ppOutputWstr
    )
{
    DWORD dwLen = 0;
    // TODO : this function must not fail.

    if (ppOutputWstr == NULL) {
        API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO);
    }

    dwLen = (DWORD)strlen(pAnsiStr);
    *ppOutputWstr = UtilsHeapAllocStrHelper(pHeap, (dwLen + 1) * sizeof(WCHAR));

    if (dwLen > 0) {
        if (MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pAnsiStr, dwLen, *ppOutputWstr, dwLen) == 0)
        {
            *ppOutputWstr = NULL;
            API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO);
        }
    }

    API_RETURN_SUCCESS();
}

BOOL ConvertWstrToAstr(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const LPCWSTR lpwStr,
    _Inout_ LPSTR *ppOutputWstr
    )
{
    // TODO : respect coding conv.
    // TODO : this function must not fail.
    // TODO : handle len==0 strings
    if (ppOutputWstr == NULL)
        API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO);

    if (lpwStr == NULL)
        API_RETURN_SUCCESS();

    *ppOutputWstr = UtilsHeapAllocStrHelper(pHeap, (DWORD)(wcslen(lpwStr) + 1) * sizeof(CHAR));
    if (!WideCharToMultiByte(CP_ACP, 0, lpwStr, (INT)wcslen(lpwStr), *ppOutputWstr, (INT) (sizeof (CHAR)* (wcslen(lpwStr) + 1)), NULL, NULL))
    {
        *ppOutputWstr = NULL;
        API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO);
    }

    API_RETURN_SUCCESS();
}

BOOL ConvertAstrArrayToWstrArray(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const LPCSTR pAnsiStr[],
    _In_ const DWORD dwStrCount,
    _Inout_ LPWSTR *ppOutputWstr[]
    )
{
    // TODO : this function must not fail.
    if ((ppOutputWstr == NULL) || (pAnsiStr == NULL))
        API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO);

    *ppOutputWstr = UtilsHeapAllocArrayHelper(pHeap, LPWSTR, dwStrCount);
    for (DWORD i = 0; i < dwStrCount; ++i) {
        if (!pAnsiStr[i])
            continue;
        (*ppOutputWstr)[i] = UtilsHeapAllocStrHelper(pHeap, (DWORD)(strlen(pAnsiStr[i]) + 1) * sizeof(LPWSTR));

        if (strlen(pAnsiStr[i]) == 0) {
            (*ppOutputWstr)[i][0] = L'\x00';
            continue;
        }

        if (!MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pAnsiStr[i], (DWORD)strlen(pAnsiStr[i]), (*ppOutputWstr)[i], (DWORD)strlen(pAnsiStr[i])))
        {
            *ppOutputWstr = NULL;
            API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO);
        }
        (*ppOutputWstr)[i][strlen(pAnsiStr[i])] = L'\x00';
    }

    API_RETURN_SUCCESS();
}

#pragma warning(suppress: 6103)
BOOL ConvertWstrArrayToAstrArray(
    _In_ const PUTILS_HEAP pHeap,
    _In_ const LPCWSTR pWideStr[],
    _In_ const DWORD dwStrCount,
    _Inout_ LPSTR *ppOutputStr[]
    )
{
    // TODO : this function must not fail.
    if (ppOutputStr == NULL)
        API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO);

    *ppOutputStr = UtilsHeapAllocArrayHelper(pHeap, LPSTR, dwStrCount);
    for (DWORD i = 0; i < dwStrCount; ++i) {
        (*ppOutputStr)[i] = UtilsHeapAllocStrHelper(pHeap, (DWORD)(_tcslen(pWideStr[i]) + 1) * sizeof(LPSTR));
        if (!(*ppOutputStr)[i])
        {
            API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO);
        }
        if (!WideCharToMultiByte(CP_ACP, 0, pWideStr[i], (DWORD)_tcslen(pWideStr[i]) + 1, (*ppOutputStr)[i], (DWORD)_tcslen(pWideStr[i]) * sizeof(LPSTR), NULL, NULL))
        {
            *ppOutputStr = NULL;
            API_RETURN_ERROR(UTILS_ERR_UNKNOWN_TODO);
        }
        (*ppOutputStr)[i][_tcslen(pWideStr[i])] = '\x00';
    }
    API_RETURN_SUCCESS();
}

#ifdef DLL_MODE
BOOL WINAPI DllMain(
    _In_  HINSTANCE hinstDLL,
    _In_  DWORD fdwReason,
    _In_  LPVOID lpvReserved
    ) {
    BOOL bResult = FALSE;

    UNREFERENCED_PARAMETER(hinstDLL);
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH: bResult = UtilsLibInit(); break;
    case DLL_PROCESS_DETACH: bResult = UtilsLibCleanup(); break;
    default: bResult = TRUE; break;
    }

    return bResult;
}
#endif