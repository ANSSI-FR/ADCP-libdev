/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#include "LogInternals.h"
#include "LogLib.h"

//#include "LogWpp.h" //TODO: WPP


/* --- PRIVATE VARIABLES ---------------------------------------------------- */
static PUTILS_HEAP gs_pLogHeap = NULL;
static DWORD gs_dwLastError = NO_ERROR;
static LOG_LEVEL gs_eConsoleLogLevel = LOG_DEFAULT_CONSOLE_LVL;
static LOG_LEVEL gs_eLogFileLogLevel = LOG_DEFAULT_LOGFILE_LVL;
static HANDLE gs_hLogFile = INVALID_HANDLE_VALUE;
static HANDLE gs_hConsole = INVALID_HANDLE_VALUE;


/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
static BOOL LogiLibInit(
) {
	BOOL bResult = FALSE;

#ifdef UNICODE
	bResult = SetConsoleOutputCP(CP_UTF8);
	if (bResult == FALSE) {
		API_RETURN_ERROR(LOG_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
	}
#endif

	gs_hConsole = GetStdHandle(STD_ERROR_HANDLE);
	if (gs_hConsole == INVALID_HANDLE_VALUE) {
		API_RETURN_ERROR(LOG_ERR_UNKNOWN_TODO); // TODO: ERR+WPP
	}

	bResult = UtilsHeapCreate(&gs_pLogHeap, LOG_HEAP_NAME, NULL); // TODO!: callback
	API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());  // TODO: WPP

	API_RETURN_SUCCESS();
}

static BOOL LogiLibCleanup(
) {
	BOOL bResult = FALSE;

	bResult = LogCloseLogFile();
	API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());  // TODO: WPP

	bResult = UtilsHeapDestroy(&gs_pLogHeap);
	API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());  // TODO: WPP

	API_RETURN_SUCCESS();
}


BOOL LogiFormatBuffer(
	_Out_ PTCHAR *pptOutBuff,
	_Out_ PDWORD pdwOutLen,
	_In_ const PTCHAR ptFormat,
	_In_ const va_list vaArgs
) {
	SYSTEMTIME sTime = { 0 };
	PTCHAR ptTmpBuff = NULL;
	PTCHAR ptTmpBuffWithTime = NULL;
	DWORD dwCharLen = 0;
	int iSize = -1;

	(*pptOutBuff) = NULL;
	(*pdwOutLen) = 0;

	GetSystemTime(&sTime);

	dwCharLen = _vsctprintf(ptFormat, vaArgs);
	ptTmpBuff = UtilsHeapAllocOrReallocHelper(gs_pLogHeap, ptTmpBuff, SIZEOF_TSTR(dwCharLen));
	iSize = _vstprintf_s(ptTmpBuff, dwCharLen + 1, ptFormat, vaArgs);
	if (iSize == -1) {
		API_RETURN_ERROR(LOG_ERR_UNKNOWN_TODO); //TODO: ERR+WPP
	}

	dwCharLen += LOG_TIME_LEN;
	ptTmpBuffWithTime = UtilsHeapAllocStrHelper(gs_pLogHeap, dwCharLen);
	iSize = _stprintf_s(ptTmpBuffWithTime, dwCharLen + 1, LOG_TIME_FRMT, sTime.wHour, sTime.wMinute, sTime.wSecond, ptTmpBuff);
	if (iSize == -1) {
		API_RETURN_ERROR(LOG_ERR_UNKNOWN_TODO); //TODO: ERR+WPP
	}
	UtilsHeapFreeAndNullHelper(gs_pLogHeap, ptTmpBuff);

	(*pptOutBuff) = ptTmpBuffWithTime;
	(*pdwOutLen) = (DWORD)iSize;

	API_RETURN_SUCCESS();
}

BOOL LogiWriteLogBuffer(
	_In_ const HANDLE hOut,
	_In_ const PTCHAR ptBuffer,
	_In_ const DWORD dwLen
) {
	BOOL bResult = FALSE;
	DWORD dwWritten = 0;

	bResult = WriteFile(hOut, ptBuffer, SIZEOF_ARRAY(TCHAR, dwLen), &dwWritten, NULL);
	if (bResult == FALSE || dwWritten != SIZEOF_ARRAY(TCHAR, dwLen)) {
		API_RETURN_ERROR(LOG_ERR_UNKNOWN_TODO); // TODO: ERR GLE+WPP
	}

	API_RETURN_SUCCESS();
}

/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
BOOL LogSetLogFile(
	_In_ const PTCHAR ptFileName
) {
	BOOL bResult = FALSE;

	bResult = LogCloseLogFile();
	API_RETURN_ERROR_IF_FAILED(bResult, SAME_ERROR());  // TODO: ERR+WPP

	if (gs_eLogFileLogLevel < None) {
		gs_hLogFile = CreateFile(ptFileName, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (gs_hLogFile == INVALID_HANDLE_VALUE) {
			API_RETURN_ERROR(LOG_ERR_UNKNOWN_TODO); // TODO: ERR GLE+WPP
		}
	}

	API_RETURN_SUCCESS();
}

BOOL LogCloseLogFile(
) {
	BOOL bResult = FALSE;

	if (gs_hLogFile != INVALID_HANDLE_VALUE) {
		bResult = CloseHandle(gs_hLogFile);
		API_RETURN_ERROR_IF_FAILED(bResult, LOG_ERR_UNKNOWN_TODO);  // TODO: ERR GLE+WPP
		gs_hLogFile = INVALID_HANDLE_VALUE;
	}

	API_RETURN_SUCCESS();
}

BOOL LogSetLogLevel(
	_In_ const DWORD dwLogTypes,
	_In_ const PTCHAR ptLogLevel
) {
	static const PTCHAR gs_ptDebugLevels[] = { _T("ALL"), _T("DBG"), _T("INFO"), _T("WARN"), _T("ERR"), _T("SUCC"), _T("NONE") };
	DWORD dwLogIdx = 0;

	if (IsInSetOfStrings(ptLogLevel, gs_ptDebugLevels, _countof(gs_ptDebugLevels), &dwLogIdx)) {
		if (dwLogTypes == LOG_ALL_TYPES) {
			gs_eLogFileLogLevel = gs_eConsoleLogLevel = (LOG_LEVEL)dwLogIdx;
		}
		else {
			if ((dwLogTypes & LogTypeConsole) != 0) {
				gs_eConsoleLogLevel = (LOG_LEVEL)dwLogIdx;
			}
			if ((dwLogTypes & LogTypeLogfile) != 0) {
				gs_eLogFileLogLevel = (LOG_LEVEL)dwLogIdx;
			}
		}
		API_RETURN_SUCCESS();
	}

	API_RETURN_ERROR(LOG_ERR_UNKNOWN_TODO); //TODO: ERR+WPP
}

DWORD LogGetLastError(
) {
	return gs_dwLastError;
}

void Log(
	_In_ const LOG_LEVEL eLogLvl,
	_In_ const PTCHAR ptFrmt,
	_In_ ...
) {
	va_list vaArgs = { 0 };
	BOOL bResult = FALSE;
	PTCHAR ptOutBuff = NULL;
	DWORD dwOutLen = 0;
	BOOL bLogToConsole = (eLogLvl >= gs_eConsoleLogLevel);
	BOOL bLogToLogFile = (gs_hLogFile != INVALID_HANDLE_VALUE && eLogLvl >= gs_eLogFileLogLevel);

	if (bLogToConsole == TRUE || bLogToLogFile == TRUE) {
		va_start(vaArgs, ptFrmt);

		bResult = LogiFormatBuffer(&ptOutBuff, &dwOutLen, ptFrmt, vaArgs);
		FATAL_ERROR_IF(API_FAILED(bResult), _T("Failed to format log buffer <%s>: <err:%#08x>"), ptFrmt, LogGetLastError());

		if (bLogToLogFile == TRUE) {
			bResult = LogiWriteLogBuffer(gs_hLogFile, ptOutBuff, dwOutLen);
			FATAL_ERROR_IF(API_FAILED(bResult), _T("Failed to write log buffer to logfile: <%s> <err:%#08x>"), ptOutBuff, LogGetLastError());
		}

		if (bLogToConsole == TRUE) {
			/*
			This worked well with ansi strings, but prints messed up lines with unicode... CodeReuse-- :/
			bResult = LogiWriteLogBuffer(gs_hConsole, ptOutBuff, dwOutLen);
			FATAL_ERROR_IF(API_FAILED(bResult), _T("Failed to write log buffer to stderr: <%s> <err:%#08x>"), ptOutBuff, LogGetLastError());
			*/
			int written = _ftprintf(stderr, _T("%s"), ptOutBuff);
			FATAL_ERROR_IF((DWORD)written != dwOutLen, _T("Failed to write log buffer to stderr: <%s> <err:%#08x>"), ptOutBuff, LogGetLastError());
			fflush(stderr);
		}

		UtilsHeapFreeAndNullHelper(gs_pLogHeap, ptOutBuff);
		va_end(vaArgs);
	}
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
	case DLL_PROCESS_ATTACH: bResult = LogiLibInit(); break;
	case DLL_PROCESS_DETACH: bResult = LogiLibCleanup(); break;
	default: bResult = TRUE; break;
	}

	return bResult;
}
