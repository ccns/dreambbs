/*-------------------------------------------------------*/
/* attr.h      ( NTHU CS MapleBBS Ver 3.10 )             */
/*-------------------------------------------------------*/
/* target : dynamic attribute database                   */
/* create : 99/03/11                                     */
/* update :   /  /                                       */
/*-------------------------------------------------------*/

#ifndef _ATTR_H_
#define _ATTR_H_

#if 0
  int key;
  key < 0 is reserved.
  key & 0xff == 0 is reserved.
  sizeof(attr): key & 0xff

  file: $userhome/.ATTR
#endif

#define ATTR_OTHELLO_TOTAL 	0x00001004
#define ATTR_FIVE_TOTAL 	0x00001104
#define ATTR_BLOCK_TOTAL 	0x00001204
#define ATTR_OTHELLO_WIN 	0x00001404
#define ATTR_FIVE_WIN 		0x00001504
#define ATTR_BLOCK_WIN 		0x00001604

/*-------------------------------------------------------*/
/* USER for WindTop					 */
/*-------------------------------------------------------*/

#define	ATTR_USER_KEY		0x000100FF
#define	ATTR_REG_KEY		0x00010104
#define	ATTR_CROSS_KEY		0x00010230

#define	REG_REQUEST		0x00000001
#define	REG_OPEN		0x00000002
#define	REG_FAULT		0x00000004
#define	REG_SENT		0x00000008
#define	REG_OK			0x00000010

#endif
