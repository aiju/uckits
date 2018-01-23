Tls*	tls();
long	error(int);
long	setibsta(void);

#define NOPE { error(ECAP); return tls()->ibsta; }
#define NOPEV { error(ECAP); }

int	sendraw(void *, int);
int	recvraw(void *, int, int);
int	sendesc(void *, int, int, int);
int	recvesc(void *, int, int, int, int *, int);
int	sendcmd(int, ...);
int	getstatus(short *);
int	checkaddr(int, int);
Desc*	getdesc(int, int);
