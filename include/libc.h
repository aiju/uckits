#define	nelem(x)	(sizeof(x)/sizeof((x)[0]))
#define	offsetof(s, m)	(ulong)(&(((s*)0)->m))
#define	assert(x)	if(x){}else _assert("x")

/*
 * mem routines
 */
extern	void*	memccpy(void*, void*, int, ulong);
extern	void*	memset(void*, int, ulong);
extern	int	memcmp(void*, void*, ulong);
extern	void*	memcpy(void*, void*, ulong);
extern	void*	memmove(void*, void*, ulong);
extern	void*	memchr(void*, int, ulong);

/*
 * string routines
 */
extern	char*	strcat(char*, char*);
extern	char*	strchr(char*, int);
extern	int	strcmp(char*, char*);
extern	char*	strcpy(char*, char*);
extern	char*	strecpy(char*, char*, char*);
extern	char*	strdup(char*);
extern	char*	strncat(char*, char*, long);
extern	char*	strncpy(char*, char*, long);
extern	int	strncmp(char*, char*, long);
extern	char*	strpbrk(char*, char*);
extern	char*	strrchr(char*, int);
extern	char*	strtok(char*, char*);
extern	long	strlen(char*);
extern	long	strspn(char*, char*);
extern	long	strcspn(char*, char*);
extern	char*	strstr(char*, char*);
extern	int	cistrncmp(char*, char*, int);
extern	int	cistrcmp(char*, char*);
extern	char*	cistrstr(char*, char*);
extern	int	tokenize(char*, char**, int);

enum
{
	UTFmax		= 4,		/* maximum bytes per rune */
	Runesync	= 0x80,		/* cannot represent part of a UTF sequence (<) */
	Runeself	= 0x80,		/* rune and UTF sequences are the same (<) */
	Runeerror	= 0xFFFD,	/* decoding error in UTF */
	Runemax		= 0x10FFFF,	/* 21 bit rune */
	Runemask	= 0x1FFFFF,	/* bits used by runes (see grep) */
};

/*
 * rune routines
 */
extern	int	runetochar(char*, Rune*);
extern	int	chartorune(Rune*, char*);
extern	int	runelen(long);
extern	int	runenlen(Rune*, int);
extern	int	fullrune(char*, int);
extern	int	utflen(char*);
extern	int	utfnlen(char*, long);
extern	char*	utfrune(char*, long);
extern	char*	utfrrune(char*, long);
extern	char*	utfutf(char*, char*);
extern	char*	utfecpy(char*, char*, char*);

extern	Rune*	runestrcat(Rune*, Rune*);
extern	Rune*	runestrchr(Rune*, Rune);
extern	int	runestrcmp(Rune*, Rune*);
extern	Rune*	runestrcpy(Rune*, Rune*);
extern	Rune*	runestrncpy(Rune*, Rune*, long);
extern	Rune*	runestrecpy(Rune*, Rune*, Rune*);
extern	Rune*	runestrdup(Rune*);
extern	Rune*	runestrncat(Rune*, Rune*, long);
extern	int	runestrncmp(Rune*, Rune*, long);
extern	Rune*	runestrrchr(Rune*, Rune);
extern	long	runestrlen(Rune*);
extern	Rune*	runestrstr(Rune*, Rune*);

extern	Rune	tolowerrune(Rune);
extern	Rune	totitlerune(Rune);
extern	Rune	toupperrune(Rune);
extern	int	isalpharune(Rune);
extern	int	islowerrune(Rune);
extern	int	isspacerune(Rune);
extern	int	istitlerune(Rune);
extern	int	isupperrune(Rune);
extern	int	isdigitrune(Rune);

/*
 * malloc
 */
extern	void*	malloc(ulong);
extern	void*	mallocz(ulong, int);
extern	void	free(void*);
extern	ulong	msize(void*);
extern	void*	mallocalign(ulong, ulong, long, ulong);
extern	void*	calloc(ulong, ulong);
extern	void*	realloc(void*, ulong);
extern	void	setrealloctag(void*, uintptr);
extern	uintptr	getmalloctag(void*);
extern	void	setmalloctag(void*, uintptr);
extern	uintptr	getrealloctag(void*);
extern	void*	malloctopoolblock(void*);
extern	void*	sbrk(ulong);

/*
 * print routines
 */
typedef struct Fmt	Fmt;
struct Fmt{
	uchar	runes;			/* output buffer is runes or chars? */
	void	*start;			/* of buffer */
	void	*to;			/* current place in the buffer */
	void	*stop;			/* end of the buffer; overwritten if flush fails */
	int	(*flush)(Fmt *);	/* called when to == stop */
	void	*farg;			/* to make flush a closure */
	int	nfmt;			/* num chars formatted so far */
	va_list	args;			/* args passed to dofmt */
	int	r;			/* % format Rune */
	int	width;
	int	prec;
	ulong	flags;
};

enum{
	FmtWidth	= 1,
	FmtLeft		= FmtWidth << 1,
	FmtPrec		= FmtLeft << 1,
	FmtSharp	= FmtPrec << 1,
	FmtSpace	= FmtSharp << 1,
	FmtSign		= FmtSpace << 1,
	FmtZero		= FmtSign << 1,
	FmtUnsigned	= FmtZero << 1,
	FmtShort	= FmtUnsigned << 1,
	FmtLong		= FmtShort << 1,
	FmtVLong	= FmtLong << 1,
	FmtComma	= FmtVLong << 1,
	FmtByte		= FmtComma << 1,

	FmtFlag		= FmtByte << 1
};

extern	int	print(char*, ...);
extern	char*	seprint(char*, char*, char*, ...);
extern	char*	vseprint(char*, char*, char*, va_list);
extern	int	snprint(char*, int, char*, ...);
extern	int	vsnprint(char*, int, char*, va_list);
extern	char*	smprint(char*, ...);
extern	char*	vsmprint(char*, va_list);
extern	int	sprint(char*, char*, ...);
extern	int	fprint(int, char*, ...);
extern	int	vfprint(int, char*, va_list);

extern	int	runesprint(Rune*, char*, ...);
extern	int	runesnprint(Rune*, int, char*, ...);
extern	int	runevsnprint(Rune*, int, char*, va_list);
extern	Rune*	runeseprint(Rune*, Rune*, char*, ...);
extern	Rune*	runevseprint(Rune*, Rune*, char*, va_list);
extern	Rune*	runesmprint(char*, ...);
extern	Rune*	runevsmprint(char*, va_list);

extern	int	fmtfdinit(Fmt*, int, char*, int);
extern	int	fmtfdflush(Fmt*);
extern	int	fmtstrinit(Fmt*);
extern	char*	fmtstrflush(Fmt*);
extern	int	runefmtstrinit(Fmt*);
extern	Rune*	runefmtstrflush(Fmt*);

extern	int	fmtinstall(int, int (*)(Fmt*));
extern	int	dofmt(Fmt*, char*);
extern	int	dorfmt(Fmt*, Rune*);
extern	int	fmtprint(Fmt*, char*, ...);
extern	int	fmtvprint(Fmt*, char*, va_list);
extern	int	fmtrune(Fmt*, int);
extern	int	fmtstrcpy(Fmt*, char*);
extern	int	fmtrunestrcpy(Fmt*, Rune*);

extern	void	floatfmtinstall(void);

/*
 * error string for %r
 * supplied on per os basis, not part of fmt library
 */
extern	int	errfmt(Fmt *f);

/*
 * quoted strings
 */
extern	char	*unquotestrdup(char*);
extern	Rune	*unquoterunestrdup(Rune*);
extern	char	*quotestrdup(char*);
extern	Rune	*quoterunestrdup(Rune*);
extern	int	quotestrfmt(Fmt*);
extern	int	quoterunestrfmt(Fmt*);
extern	void	quotefmtinstall(void);
extern	int	(*doquote)(int);
extern	int	needsrcquote(int);

/*
 * random number
 */
extern	void	srand(long);
extern	int	rand(void);
extern	int	nrand(int);
extern	long	lrand(void);
extern	long	lnrand(long);
extern	double	frand(void);
extern	ulong	truerand(void);			/* uses /dev/random */
extern	ulong	ntruerand(ulong);		/* uses /dev/random */

/*
 * math
 */
extern	ulong	getfcr(void);
extern	void	setfsr(ulong);
extern	ulong	getfsr(void);
extern	void	setfcr(ulong);
extern	double	NaN(void);
extern	double	Inf(int);
extern	int	isNaN(double);
extern	int	isInf(double, int);
extern	ulong	umuldiv(ulong, ulong, ulong);
extern	long	muldiv(long, long, long);

extern	double	pow(double, double);
extern	double	atan2(double, double);
extern	double	fabs(double);
extern	double	atan(double);
extern	double	log(double);
extern	double	log10(double);
extern	double	exp(double);
extern	double	floor(double);
extern	double	ceil(double);
extern	double	hypot(double, double);
extern	double	sin(double);
extern	double	cos(double);
extern	double	tan(double);
extern	double	asin(double);
extern	double	acos(double);
extern	double	sinh(double);
extern	double	cosh(double);
extern	double	tanh(double);
extern	double	sqrt(double);
extern	double	fmod(double, double);

#define	HUGE	3.4028234e38
#define	PIO2	1.570796326794896619231e0
#define	PI	(PIO2+PIO2)

/*
 * Time-of-day
 */

typedef
struct Tm
{
	int	sec;
	int	min;
	int	hour;
	int	mday;
	int	mon;
	int	year;
	int	wday;
	int	yday;
	char	zone[4];
	int	tzoff;
} Tm;

extern	Tm*	gmtime(long);
extern	Tm*	localtime(long);
extern	char*	asctime(Tm*);
extern	char*	ctime(long);
extern	double	cputime(void);
extern	long	times(long*);
extern	long	tm2sec(Tm*);
extern	vlong	nsec(void);

extern	void	cycles(uvlong*);	/* 64-bit value of the cycle counter if there is one, 0 if there isn't */

/*
 * one-of-a-kind
 */
enum
{
	PNPROC		= 1,
	PNGROUP		= 2,
};

extern	void	_assert(char*);
extern	int	abs(int);
extern	int	atexit(void(*)(void));
extern	void	atexitdont(void(*)(void));
extern	int	atnotify(int(*)(void*, char*), int);
extern	double	atof(char*);
extern	int	atoi(char*);
extern	long	atol(char*);
extern	vlong	atoll(char*);
extern	double	charstod(int(*)(void*), void*);
extern	char*	cleanname(char*);
extern	int	decrypt(void*, void*, int);
extern	int	encrypt(void*, void*, int);
extern	int	dec64(uchar*, int, char*, int);
extern	int	enc64(char*, int, uchar*, int);
extern	int	dec32(uchar*, int, char*, int);
extern	int	enc32(char*, int, uchar*, int);
extern	int	dec16(uchar*, int, char*, int);
extern	int	enc16(char*, int, uchar*, int);
extern	int	encodefmt(Fmt*);
extern	void	exits(char*);
extern	double	frexp(double, int*);
extern	uintptr	getcallerpc(void*);
extern	char*	getenv(char*);
extern	int	getfields(char*, char**, int, int, char*);
extern	int	gettokens(char *, char **, int, char *);
extern	int	iounit(int);
extern	long	labs(long);
extern	double	ldexp(double, int);
extern	void	longjmp(jmp_buf, int);
extern	double	modf(double, double*);
extern	double	pow10(int);
extern	int	putenv(char*, char*);
extern	void	qsort(void*, long, long, int (*)(void*, void*));
extern	int	setjmp(jmp_buf);
extern	double	strtod(char*, char**);
extern	long	strtol(char*, char**, int);
extern	ulong	strtoul(char*, char**, int);
extern	vlong	strtoll(char*, char**, int);
extern	uvlong	strtoull(char*, char**, int);
extern	void	sysfatal(char*, ...);
extern	long	time(long*);
extern	int	tolower(int);
extern	int	toupper(int);

/*
 *  synchronization
 */
typedef
struct Lock {
	int	val;
} Lock;

extern int	_tas(int*);

extern	void	lock(Lock*);
extern	void	unlock(Lock*);
extern	int	canlock(Lock*);

typedef struct QLp QLp;
struct QLp
{
	int	inuse;
	QLp	*next;
	char	state;
};

typedef
struct QLock
{
	Lock	lock;
	int	locked;
	QLp	*head;
	QLp 	*tail;
} QLock;

extern	void	qlock(QLock*);
extern	void	qunlock(QLock*);
extern	int	canqlock(QLock*);
extern	void	_qlockinit(void* (*)(void*, void*));	/* called only by the thread library */

typedef
struct RWLock
{
	Lock	lock;
	int	readers;	/* number of readers */
	int	writer;		/* number of writers */
	QLp	*head;		/* list of waiting processes */
	QLp	*tail;
} RWLock;

extern	void	rlock(RWLock*);
extern	void	runlock(RWLock*);
extern	int	canrlock(RWLock*);
extern	void	wlock(RWLock*);
extern	void	wunlock(RWLock*);
extern	int	canwlock(RWLock*);

typedef
struct Rendez
{
	QLock	*l;
	QLp	*head;
	QLp	*tail;
} Rendez;

extern	void	rsleep(Rendez*);	/* unlocks r->l, sleeps, locks r->l again */
extern	int	rwakeup(Rendez*);
extern	int	rwakeupall(Rendez*);
extern	void**	privalloc(void);

#define	MORDER	0x0003	/* mask for bits defining order of mounting */
#define	MREPL	0x0000	/* mount replaces object */
#define	MBEFORE	0x0001	/* mount goes before others in union directory */
#define	MAFTER	0x0002	/* mount goes after others in union directory */
#define	MCREATE	0x0004	/* permit creation in mounted directory */
#define	MCACHE	0x0010	/* cache some data */
#define	MMASK	0x0017	/* all bits on */

#define	OREAD	0	/* open for read */
#define	OWRITE	1	/* write */
#define	ORDWR	2	/* read and write */
#define	OEXEC	3	/* execute, == read but check execute permission */
#define	OTRUNC	16	/* or'ed in (except for exec), truncate file first */
#define	OCEXEC	32	/* or'ed in, close on exec */
#define	ORCLOSE	64	/* or'ed in, remove on close */
#define	OEXCL	0x1000	/* or'ed in, exclusive use (create only) */

#define	AEXIST	0	/* accessible: exists */
#define	AEXEC	1	/* execute access */
#define	AWRITE	2	/* write access */
#define	AREAD	4	/* read access */

extern	void	abort(void);
extern	int	access(char*, int);
extern	int	close(int);
extern	int	create(char*, int, ulong);
extern	int	dup(int, int);
extern	int	errstr(char*, uint);
extern	int	open(char*, int);
extern	long	pread(int, void*, long, vlong);
extern	long	pwrite(int, void*, long, vlong);
extern	long	read(int, void*, long);
extern	long	readn(int, void*, long);
extern	int	remove(char*);
extern	vlong	seek(int, vlong, int);
extern	int	sleep(long);
extern	ssize_t	write(int, void*, size_t);

extern	void	rerrstr(char*, uint);
extern	void	werrstr(char*, ...);
extern	void	werrstr1(char*);

extern	long	ainc(long*);
extern	long	adec(long*);

extern char *argv0;
#define	ARGBEGIN	for((argv0||(argv0=*argv)),argv++,argc--;\
			    argv[0] && argv[0][0]=='-' && argv[0][1];\
			    argc--, argv++) {\
				char *_args, *_argt;\
				Rune _argc;\
				_args = &argv[0][1];\
				if(_args[0]=='-' && _args[1]==0){\
					argc--; argv++; break;\
				}\
				_argc = 0;\
				while(*_args && (_args += chartorune(&_argc, _args)))\
				switch(_argc)
#define	ARGEND		SET(_argt);USED(_argt,_argc,_args);}USED(argv, argc);
#define	ARGF()		(_argt=_args, _args="",\
				(*_argt? _argt: argv[1]? (argc--, *++argv): 0))
#define	EARGF(x)	(_argt=_args, _args="",\
				(*_argt? _argt: argv[1]? (argc--, *++argv): ((x), abort(), (char*)0)))

#define	ARGC()		_argc

#define ERRMAX 128

extern char _end[], _etext[], _data[], _edata[], _estack[];

extern char errbuf[];

extern char Enodev[];
extern char Ebadctl[];
extern char Eio[];

typedef struct CQueue CQueue;

struct CQueue {
	uchar *d;
	u16int wr, rd, sz;
	
	void (*fntxkick)(CQueue *);
	void (*fnrxkick)(CQueue *);
};

int cquwritenb(CQueue *, void *, uint);
int cqureadnb(CQueue *, void *, uint);
int cquwrite(CQueue *, void *, uint);
int cquread(CQueue *, void *, uint);
