
#define LISTEN 0x20
#define TALK 0x40
#define SAD(x) ((((UCHAR)(x))-1 & 0xff)+1)
#define CMD(x) (0x200|(x))

typedef struct Tls {
	long ibsta;
	long iberr;
	long ibcnt;
} Tls;

enum {
	DESCVALID = 1, /* valid descriptor */
	DESCBOARD = 2, /* board (aka interface) descriptor */
	DESCNORMEND = 4, /* END bit is normal */
	DESCSENDEOI = 8, /* send EOI with last byte */

	/* currently ignored */
	DESCIST = 16, /* parallel poll status bit */
	DESCPP2 = 32, /* parallel poll mode */
	DESCLLO = 128, /* send LLO before addressing devices */
	DESCSRE = 256, /* assert REN when system controller */	
	DESCREADDR = 512, /* readdress devices for every operation */
	DESCUNADDR = 1024, /* send UNL/UNT after each read/write operation */
};
typedef struct Desc {
	int flags;
	UCHAR pad, sad;
	UCHAR tmo;
	USHORT eos;
	UCHAR ppc, rsv;
} Desc;
extern Desc fdtab[];

enum { NDESC = 64 };

extern int gibsta, giberr, gibcnt;

/* getdesc */
enum {
	DEVOK = 1,
	BRDOK = 2,
};

#define mypad (fdtab[0].pad)
#define mysad (fdtab[0].sad)
