/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

#ifndef __LDAP_WPP_H__
#define __LDAP_WPP_H__

/* --- INCLUDES ------------------------------------------------------------- */
/* --- DEFINES -------------------------------------------------------------- */
#define WPP_CONTROL_GUIDS                                                               \
    WPP_DEFINE_CONTROL_GUID(LdapLibGuid, (25FD4E5E, FE4C, 4CF2, BFA6, 90573C092B6D),    \
    WPP_DEFINE_BIT(INITIALIZATION)                                                      \
    WPP_DEFINE_BIT(FINALIZATION)                                                        \
    WPP_DEFINE_BIT(CONFIGURATION)                                                       \
    WPP_DEFINE_BIT(ALLOCATION)                                                          \
    WPP_DEFINE_BIT(MEMORY_MANAGEMENT)                                                   \
    WPP_DEFINE_BIT(CONNECTION)                                                          \
    WPP_DEFINE_BIT(REQUEST)                                                             \
    )

#define WPP_LEVEL_FLAGS_LOGGER(level, flags)                                            \
    WPP_LEVEL_LOGGER(flags)

#define WPP_LEVEL_FLAGS_ENABLED(level, flags)                                           \
    (WPP_LEVEL_ENABLED(flags) && WPP_CONTROL(WPP_BIT_ ## flags).Level >= level)

/* --- WPP CONFIG ----------------------------------------------------------- */
// begin_wpp config
//      FUNC LdapWppMessage(LEVEL, FLAGS, MSG, ...);
//      USEPREFIX (LdapWppMessage, "%!STDPREFIX!");
// end_wpp

//
// TODO: WPP does not handle TCHAR strings with a single '%s' format.
//        so we must explicitely specify '%S' format
//        => this is not portable and wont work if the 'character set' change...
//

/* --- TYPES ---------------------------------------------------------------- */
/* --- VARIABLES ------------------------------------------------------------ */
/* --- PROTOTYPES ----------------------------------------------------------- */
#endif // __LDAP_WPP_H__
