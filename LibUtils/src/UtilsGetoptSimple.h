/******************************************************************************\
\******************************************************************************/

#ifndef __GETOPT_SIMPLE_H__
#define __GETOPT_SIMPLE_H__


/* --- INCLUDES ------------------------------------------------------------- */
/* --- DEFINES -------------------------------------------------------------- */
#define getopt getopt_simple

/* --- TYPES ---------------------------------------------------------------- */
/* --- VARIABLES ------------------------------------------------------------ */
extern DLL_VAR int opterr;
extern DLL_VAR int optind;
extern DLL_VAR int optopt;
extern DLL_VAR int optreset;
extern DLL_VAR char *optarg;

/* --- PROTOTYPES ----------------------------------------------------------- */
DLL_FCT int getopt_simple(int nargc, char * const nargv[], const char *ostr);

#endif // __GETOPT_SIMPLE_H__

