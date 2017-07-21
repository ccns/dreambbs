/* dns.c */
void dns_init(void);
int dns_query(char *name, int qtype, querybuf *ans);
/* dns_addr.c */
unsigned long dns_addr(char *name);
/* dns_ident.c */
void dns_ident(int sock, struct sockaddr_in *from, char *rhost, char *ruser);
/* dns_name.c */
int dns_name(unsigned char *addr, char *name);
/* dns_open.c */
int dns_open(char *host, int port);
/* dns_smtp.c */
int dns_smtp(char *host);
