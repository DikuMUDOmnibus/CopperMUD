
/* ************************************************************************
*  file: id.h, definitions for ident query daemon         Part of Copper3 *
*  Usage : #include in your favorite source files                         *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
************************************************************************* */

#ident	"@(#) $Id:$\n"
/* $Log:$
 */

struct descriptor_data
{
    int descriptor;
    char name[20];
    int gamedesc; /* descriptor number in the games */
    unsigned long host_addr;
    struct descriptor_data *next;
};


