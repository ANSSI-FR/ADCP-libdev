/******************************************************************************\
Author : X. XXXXXX
Date   : 01/01/1970
Descr. : XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
XXX XXX XXX XXX XXX XXX XXX XXX XXX
\******************************************************************************/

/* --- INCLUDES ------------------------------------------------------------- */
#include "LdapWpp.h"
#include "LdapWpp.tmh"

//
// Regenerate *.tmh files:
//      tracewpp.exe -cfgdir:"<WindowsKitDir>\bin\WppConfig\Rev1" -scan:"LdapWpp.h" *.c
//

/* --- PRIVATE VARIABLES ---------------------------------------------------- */
/* --- PUBLIC VARIABLES ----------------------------------------------------- */
/* --- PRIVATE FUNCTIONS ---------------------------------------------------- */
/* --- PUBLIC FUNCTIONS ----------------------------------------------------- */
// With static libs, the caller of WPP_INIT_TRACING must be the exe, not the lib

void LdapWppInit(
    ) {
    WPP_INIT_TRACING(LDAP_WPP_APP_NAME);
}

void LdapWppClean(
    ) {
    WPP_CLEANUP();
}
