/******************************************************************************\
Author : Lucas. BOUILLOT
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/


#ifndef __LOG_LIB_H__
#define __LOG_LIB_H__


/* --- INCLUDES ------------------------------------------------------------- */
#include "..\..\LibUtils\src\UtilsLib.h"

#ifdef VLD_DBG
    #define VLD_FORCE_ENABLE
    #include <vld.h>
    #pragma comment(lib,"vld.lib")
#endif

#undef DLL_FCT
#ifdef __LOG_INT_H__
    #define DLL_FCT __declspec(dllexport)
#else
    #define DLL_FCT __declspec(dllimport)
#endif


/* --- DEFINES -------------------------------------------------------------- */
#define LOG_LIBERR                  (0xCCCC)
#define LOG_ERR_UNKNOWN_TODO        ERROR_CODE(LOG_LIBERR, 1)

#define LOG_STRLVL_ALL  _T("ALL")
#define LOG_STRLVL_DBG  _T("DBG")
#define LOG_STRLVL_INFO _T("INFO")
#define LOG_STRLVL_WARN _T("WARN")
#define LOG_STRLVL_ERR  _T("ERR")
#define LOG_STRLVL_SUCC _T("SUCC")
#define LOG_STRLVL_NONE _T("NONE")

#define LOG_CHR_All     " "
#define LOG_CHR_Dbg     "?"
#define LOG_CHR_Info    "."
#define LOG_CHR_Warn    "!"
#define LOG_CHR_Err     "-"
#define LOG_CHR_Succ    "+"
#define LOG_CHR_Bypass  " "


#define LOG(lvl, frmt, ...)         LOG_NO_NL(lvl, NONE(frmt) ## _T("\r\n"), __VA_ARGS__);
#define LOG_NO_NL(lvl, frmt, ...)   Log(lvl, LOG_CHR(lvl) ## frmt, __VA_ARGS__);
#define LOG_CHR(lvl)                _T("[") ## _T(LOG_CHR_ ## lvl) ## _T("] ")
#define FATAL(frmt, ...)            MULTI_LINE_MACRO_BEGIN              \
                                        LOG(Err, frmt, __VA_ARGS__);    \
                                        ExitProcess(EXIT_FAILURE);      \
                                    MULTI_LINE_MACRO_END
#define FATAL_IF(cond, frmt, ...)   MULTI_LINE_MACRO_BEGIN              \
                                        if (cond) {                     \
                                            FATAL(frmt, __VA_ARGS__);   \
                                        }                               \
                                    MULTI_LINE_MACRO_END
#define FCT_FRMT(frmt)              _T(" --[") ## _T(__FUNCTION__) ## _T("] ") ## frmt
#define SUB_LOG(frmt)               _T(" -- ") ## frmt
#define LOG_FCT(lvl, frmt, ...)     LOG(lvl, FCT_FRMT(frmt), __VA_ARGS__);
#define FATAL_FCT(frmt, ...)        FATAL(FCT_FRMT(frmt), __VA_ARGS__);
#define FATAL_DBG()                 FATAL(_T("[DBG_BREAK] %s: %s: %u"), __FILE__, __FUNCTION__, __LINE__)
#define WARN_UNTESTED(s)            LOG(Warn, _T("WARNING: Using untested functionnality <") ## s ## _T(">"));

#define LOG_DEFAULT_CONSOLE_LVL     (Warn)
#define LOG_DEFAULT_LOGFILE_LVL     (Info)
#define LOG_ALL_TYPES               (0)

/* --- TYPES ---------------------------------------------------------------- */
typedef enum _LOG_LEVEL {
    All = 0,        // print all possible logs
    Dbg = 1,        // print debug information
    Info = 2,       // print info 
    Warn = 3,       // print warnings and more critical
    Err = 4,        // print errors and success
    Succ = 5,       // print success only
    None = 1337,    // leets dont need logs
    Bypass = 9001,  // IT'S OVER 9000 (print this whatever the loglevel is)
} LOG_LEVEL, *PLOG_LEVEL;

typedef enum _LOG_TYPE {
    LogTypeConsole = BIT(0),
    LogTypeLogfile = BIT(1),
} LOG_TYPE, *PLOG_TYPE;


/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */
DLL_FCT BOOL LogSetLogFile(
    _In_ const PTCHAR ptFileName
    );

DLL_FCT BOOL LogCloseLogFile(
    );

DLL_FCT BOOL LogSetLogLevel(
    _In_ const DWORD dwLogTypes,
    _In_ const PTCHAR ptLogLevel
    );

DLL_FCT void Log(
    _In_ const LOG_LEVEL eLogLvl,
    _In_ const PTCHAR ptFrmt,
    _In_ ...
    );

DLL_FCT DWORD LogGetLastError(
    );

#endif // __LOG_LIB_H__
