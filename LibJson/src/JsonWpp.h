/******************************************************************************\
Author : L. Delsalle
Date   : 26/01/2015
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __JSON_WPP_H__
#define __JSON_WPP_H__

/* --- INCLUDES ------------------------------------------------------------- */
/* --- DEFINES -------------------------------------------------------------- */
#define WPP_CONTROL_GUIDS                                                               \
    WPP_DEFINE_CONTROL_GUID(JsonLibGuid, (EE163B68, E5E0, 4A18, 93A5, 1EC31504BA23),    \
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
//      FUNC JsonWppMessage(LEVEL, FLAGS, MSG, ...);
//      USEPREFIX (CsvWppMessage, "%!STDPREFIX!");
// end_wpp

//
//

/* --- TYPES ---------------------------------------------------------------- */
/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */
#endif // __JSON_WPP_H__
