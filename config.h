/* * * * This is the prototype file - use the config.h in your $ARCH
 * * * * directory, or copy this one there and remove this header.
 * * * */

/* A configuration file for Copper III -
*
* By #define-ing the following as 1 or 0, the corresponding features will be
* either enabled or disabled:
*
* CONFIG_JAIL     - Jail system which protects humanoids in hometowns
* CONFIG_AUTORENT - Save players/items to disk after idle time (lag helper)
* CONFIG_...
*
*/

#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_JAIL 1
#define CONFIG_AUTORENT 1
#define CONFIG_IDQD 1

/* OS info */
#define CONFIG_BSD 1
#define CONFIG_SYSV 0
#define HAVE_SETDTABLESIZE 0

#define HAVE_MAXMIN 1

/* Choose only one of these, if possible */
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 0

#define HAVE_STRSTR 1
#define HAVE_STRDUP 1

#endif /*!defined(CONFIG_H)*/
