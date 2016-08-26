/******************************************************************************\
Author : L. Delsalle
Date   : 26/01/2015
Descr. : Test case for LibJson
\******************************************************************************/

#ifndef __MAIN_H__
#define __MAIN_H__

/* --- INCLUDES ------------------------------------------------------------- */
#include <Windows.h>
#include "JsonLib.h"

/* --- DEFINES -------------------------------------------------------------- */
#define TEST_CASE_VERSION               _T("0.1")

/* --- TYPES ---------------------------------------------------------------- */
/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */
INT main(
    _In_ INT argc,
    _In_ PCHAR argv[]
    );

PTCHAR GetObjectTypeStr(
    _In_ JSON_OBJECT_TYPE eObjType
    );

VOID JsonDump(
    _In_ PJSON_OBJECT pJsonObj,
    _In_  DWORD dwIdentLvl
    );

#endif // __MAIN_H__