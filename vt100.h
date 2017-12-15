/*From: brumleve@iboga (Dan Brumleve)*/

#ifndef VT100_H
#define VT100_h

#define VT_INITSEQ    "\033[1;24r"          /* fixes up margins */
#define VT_CURSPOS    "\033[%d;%dH"         /* respositions cursor */
#define VT_CURSRIG    "\033[%dC"            /* moves cursor right */
#define VT_CURSLEF    "\033[%dD"            /* moves cursor left */
#define VT_CLENSEQ    "\033[r\033[2J"       /* clears and resets screen */

#define VT_INVERTT    "\033[0;1;7m"         /* inverted text */
#define VT_BOLDTEX    "\033[0;1m"           /* bold text */
#define VT_NORMALT    "\033[0m"             /* normal text */
#define VT_MARGSET    "\033[%d;%dr"         /* sets margins */
#define VT_CURSAVE    "\0337"               /* saves cursor position */
#define VT_CURREST    "\0338"               /* restores cursor position */
#define ANSI_BLINK    "\033[5m"             /* blinking text */
#define ANSI_CYAN     "\033[36m"
#define ANSI_ORANGE   "\033[33m"
#define ANSI_RED1     "\033[4m"
#define ANSI_RED2     "\033[3m"             /* pukey shade of pastel red */
#define ANSI_BLUE     "\033[34m"
#define ANSI_PURPLE   "\033[35m"
#define ANSI_WHITE    "\033[37m"
#define ANSI_BK_ON_BK "\033[8m"             /* black on black text */
#define ANSI_BK_ON_WH "\033[7m"             /* black on white text */
#define ANSI_WH_ON_BL  "\033[44"            /* white on blue text */
#define ANSI_WH_ON_CY "\033[46m"            /* white on cyan text */
#define ANSI_WH_ON_GR "\033[42m"            /* white on green text */
#define ANSI_WH_ON_OR "\033[43m"            /* white on orange text */
#define ANSI_WH_ON_PR "\033[45m"            /* white on purple text */
#define ANSI_WH_ON_RD "\033[41m"            /* white on red text */

#endif /* !defined(VT100_H) */
