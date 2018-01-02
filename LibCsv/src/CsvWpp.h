/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __CSV_WPP_H__
#define __CSV_WPP_H__

/* --- INCLUDES ------------------------------------------------------------- */
/* --- DEFINES -------------------------------------------------------------- */
#define WPP_CONTROL_GUIDS                                                               \
    WPP_DEFINE_CONTROL_GUID(CsvLibGuid, (514F0D70, 6383, 43E7, 81DA, 9BEFF6B9D095),     \
    WPP_DEFINE_BIT(INITIALIZATION)                                                      \
    WPP_DEFINE_BIT(FINALIZATION)                                                        \
    WPP_DEFINE_BIT(CONFIGURATION)                                                       \
    WPP_DEFINE_BIT(ALLOCATION)                                                          \
    WPP_DEFINE_BIT(MEMORY_MANAGEMENT)                                                   \
    WPP_DEFINE_BIT(READ_OPERATION)                                                      \
    WPP_DEFINE_BIT(WRITE_OPERATION)                                                     \
    WPP_DEFINE_BIT(OPEN_OPERATION)                                                      \
    WPP_DEFINE_BIT(CLOSE_OPERATION)                                                     \
    )

#define WPP_LEVEL_FLAGS_LOGGER(level, flags)                                            \
    WPP_LEVEL_LOGGER(flags)

#define WPP_LEVEL_FLAGS_ENABLED(level, flags)                                           \
    (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= level)

/* --- WPP CONFIG ----------------------------------------------------------- */
// begin_wpp config
//      FUNC CsvWppMessage(LEVEL, FLAGS, MSG, ...);
//      USEPREFIX (CsvWppMessage, "%!STDPREFIX!");
// end_wpp

//
//

/* --- TYPES ---------------------------------------------------------------- */
/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */

#endif // __CSV_WPP_H__
