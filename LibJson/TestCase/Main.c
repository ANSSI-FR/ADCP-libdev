/******************************************************************************\
Author : L. Delsalle
Date   : 26/01/2015
Descr. : Test case for LibJson
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#include "Main.h"

/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* Application heap */
HANDLE hHeap = NULL;

/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
INT main(
    _In_ INT argc,
    _In_ PCHAR argv[]
    )
{
    BOOL bRes = FALSE;
    PJSON_OBJECT pJsonObj = NULL;
    PTCHAR ptJsonFile = _T("C:\\Users\\Lde\\Desktop\\ADng\\test.json");
    PTCHAR ptJsonString = _T("{ \"sd\" : { \"descr\" : \"Dump AD security descriptors\" } }");

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    UNREFERENCED_PARAMETER(ptJsonFile);
    UNREFERENCED_PARAMETER(ptJsonString);

    printf("JsonLib test case v%ws- L. Delsalle - ANSSI/COSSI/DTO/BAI\r\n\r\n", TEST_CASE_VERSION);

    // Heap creation
    hHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
    if (!hHeap)
        return 1;

    bRes = JsonOpenFileRead(ptJsonFile, &pJsonObj);
    //bRes = JsonParseStringRead(ptJsonString, &pJsonObj);
    if (bRes == FALSE)
    {
        printf("Unable to call JsonOpenRead. Err: %#x", JsonGetLastError());
        return 0;
    }

    // TODO : While get sibling
    if (pJsonObj->eObjectType == JsonResultTypeObject)
        printf("JSON Object :\r\n");
    else
        printf("JSON Array :\r\n");

    JsonDump(pJsonObj, 0);

    return 0;
}

VOID JsonDump(
    _In_ PJSON_OBJECT pJsonObj,
    _In_  DWORD dwIdentLvl
    )
{
    PJSON_OBJECT pJsonObjOut = NULL;
    BOOL bRes = FALSE;

    do
    {
        PTCHAR ptTmp = L"descr";
        bRes = JsonGetNextObject(pJsonObj, &ptTmp, 0, &pJsonObjOut);
        if (bRes == FALSE) {
            printf("Unable to call JsonGetNextObject. Err: %#x", JsonGetLastError());
            return;
        }
        else if (pJsonObjOut == NULL) return;

        for (DWORD i = 0; i < dwIdentLvl; ++i) printf(" ");
        printf(" (%ws) ", GetObjectTypeStr(pJsonObjOut->eObjectType));

        if (pJsonObjOut->ptKey)
            printf(" %ws : ", pJsonObjOut->ptKey);

        if (pJsonObjOut->eObjectType == JsonResultTypeString)
            printf(" %ws\r\n", pJsonObjOut->value.ptStr);
        else if (pJsonObjOut->eObjectType == JsonResultTypeNumber)
            printf(" %d\r\n", pJsonObjOut->value.iNumber);
        else if (pJsonObjOut->eObjectType == JsonResultTypeBoolean)
            printf(" %ws\r\n", (pJsonObjOut->value.bBoolean == TRUE) ? _T("TRUE") : _T("FALSE"));
        else if ((pJsonObjOut->eObjectType == JsonResultTypeArray) || (pJsonObjOut->eObjectType == JsonResultTypeObject))
        {
            printf("\r\n");
            JsonDump(pJsonObjOut, dwIdentLvl + 3);
        }
    } while (pJsonObjOut != NULL);

    bRes = JsonReleaseObject(&pJsonObj);
    if (bRes == FALSE) {
        printf("Unable to call JsonReleaseObject. Err: %#x", JsonGetLastError());
        return;
    }

}

PTCHAR GetObjectTypeStr(
    _In_ JSON_OBJECT_TYPE eObjType
    )
{
    PTCHAR ptStr = NULL;

    ptStr = (eObjType == JsonResultTypeObject) ? _T("Json Object")
        : (eObjType == JsonResultTypeArray) ? _T("Array")
        : (eObjType == JsonResultTypeString) ? _T("String")
        : (eObjType == JsonResultTypeNumber) ? _T("Number")
        : (eObjType == JsonResultTypeBoolean) ? _T("Boolean") : _T("Unknown");

    return ptStr;
}