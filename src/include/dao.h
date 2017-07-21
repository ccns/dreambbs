#ifndef	_DAO_H_
#define	_DAO_H_


#include <stdio.h>
#include <sys/types.h>


#ifndef	NULL
#define	NULL	0			/* ((char *) 0) */
#endif


#ifndef	BLK_SIZ
#define BLK_SIZ         4096		/* disk I/O block size */
#endif


#ifndef	REC_SIZ
#define REC_SIZ         4096		/* disk I/O record size */
#endif


/* Thor.981206: lkchu patch */
extern char radix32[32];


#include "hdr.h"			/* prototype */
#include "dns.h"			/* dns type */
#include "splay.h"			/* splay type */
#include "../lib/dao.p"			/* prototype */


#endif	/* _DAO_H_ */
