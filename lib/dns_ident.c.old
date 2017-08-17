/*-------------------------------------------------------*/
/* lib/dns_ident.c	( NTHU CS MapleBBS Ver 3.00 )	 */
/*-------------------------------------------------------*/
/* target : included C file for DNS routines		 */
/* create : 96/11/20					 */
/* update : 96/12/15					 */
/*-------------------------------------------------------*/


#include "dns.h"


/* ----------------------------------------------------- */
/* get remote host  / user name				 */
/* ----------------------------------------------------- */


/*
 * dns_ident() speaks a common subset of the RFC 931, AUTH, TAP, IDENT and
 * RFC 1413 protocols. It queries an RFC 931 etc. compatible daemon on a
 * remote host to look up the owner of a connection. The information should
 * not be used for authentication purposes.
 */


#define RFC931_PORT     113	/* Semi-well-known port */
#define ANY_PORT        0	/* Any old port will do */

  /* Thor.990325: 為了讓反查時能確定查出, 來自哪個interface就從那連回,
                  以防反查不到 */


void
dns_ident(sock, from, rhost, ruser)
  int sock;  /* Thor.990330: 負數保留給, 用getsock無法抓出正確port的時候.
                             代表 Port, 不過不太可能用到*/
  struct sockaddr_in *from;
  char *rhost;
  char *ruser;
{
  struct sockaddr_in rmt_sin;
  struct sockaddr_in our_sin;
  unsigned rmt_port, rmt_pt;
  unsigned our_port, our_pt;
  int s, cc;
  char buf[512];

  *ruser = '\0';

  /* get remote host name */

  if (dns_name((char *) &from->sin_addr, rhost))
    return;			/* 假設沒有 FQDN 就沒有跑 identd */

  /*
   * Use one unbuffered stdio stream for writing to and for reading from the
   * RFC931 etc. server. This is done because of a bug in the SunOS 4.1.x
   * stdio library. The bug may live in other stdio implementations, too.
   * When we use a single, buffered, bidirectional stdio stream ("r+" or "w+"
   * mode) we read our own output. Such behaviour would make sense with
   * resources that support random-access operations, but not with sockets.
   */
  /* Thor.990325: 為了讓反查時能確定查出, 來自哪個interface就從那連回,
                  以防反查不到  */
  /* Thor.990330:負數保留給,用getsock無法抓出正確port的時候.不太可能*/
  if(sock >= 0)
  {
    int s = sizeof(our_sin);

    if(getsockname(sock, (struct sockaddr *) &our_sin, &s) < 0)
      return;

    /* Thor.990325: 為了讓反查時能確定查出, 來自哪個interface就從那連回 */
    our_pt = ntohs(our_sin.sin_port);
    our_sin.sin_port = htons(ANY_PORT);
  }
  else
  {
    our_pt = -sock;
  }

  if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    return;

  /*
   * Bind the local and remote ends of the query socket to the same IP
   * addresses as the connection under investigation. We go through all this
   * trouble because the local or remote system might have more than one
   * network address. The RFC931 etc. client sends only port numbers; the
   * server takes the IP addresses from the query socket.
   */

  rmt_sin = *from;
  rmt_pt = ntohs(rmt_sin.sin_port);
  rmt_sin.sin_port = htons(RFC931_PORT);

  /* Thor.990325: 為了讓反查時能確定查出, 來自哪個interface就從那連回 */
  if ( (sock < 0 || !bind(s, (struct sockaddr *) & our_sin, sizeof(our_sin)))
    && !connect(s, (struct sockaddr *) & rmt_sin, sizeof(rmt_sin)))
  {
    /*
     * Send query to server. Neglect the risk that a 13-byte write would have
     * to be fragmented by the local system and cause trouble with buggy
     * System V stdio libraries.
     */

    sprintf(buf, "%u,%u\r\n", rmt_pt, our_pt);
    write(s, buf, strlen(buf));

    cc = read(s, buf, sizeof(buf));
    if (cc > 0)
    {
      buf[cc] = '\0';

      if (sscanf(buf, "%u , %u : USERID :%*[^:]:%79s",
	  &rmt_port, &our_port, ruser) == 3 &&
	rmt_pt == rmt_port && our_pt == our_port)
      {
	/*
	 * Strip trailing carriage return. It is part of the protocol, not
	 * part of the data.
	 */

	for (;;)
	{
	  cc = *ruser;

	  if (cc == '\r')
	  {
	    *ruser = '\0';
	    break;
	  }

	  if (cc == '\0')
	    break;

	  ruser++;
	}
      }
    }
  }
  close(s);
}
