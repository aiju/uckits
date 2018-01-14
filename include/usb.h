typedef struct USBDesc USBDesc;
typedef struct USBReq USBReq;
typedef struct USBEp USBEp;

struct USBDesc {
	u16int idx;
	u16int len;
	uchar *data;
};

struct USBReq {
	u8int bmRequestType;
	u8int bRequest;
	u16int wValue;
	u16int wIndex;
	u16int wLength;
	
	u16int ptr, len;
	u8int *data;
};

struct USBEp {
	u8int addr, type, maxpkt, hwep;
	void (*act)(USBEp *);
	void *aux;
/* STM32 specific */
	volatile u16int *reg;
	volatile u16int *pmatab;
	volatile u16int *pmatx, *pmarx;
};

enum {
	RTTOHOST = 0x80,
	RTTYPE = 0x60,
	RTSTANDARD = 0,
	RTCLASS = 0x20,
	RTVENDOR = 0x40,
	RTRECIP = 0x1f,
	RTRDEVICE = 0,
	RTRIFACE = 1,
	RTRENDP = 2,
	RTROTHER = 3,
};

enum {
	/*type*/
	EPTYPE = 3,
	EPCTRL = 0,
	EPISO = 1,
	EPBULK = 2,
	EPINT = 3,
	
	EPACTIVE = 0x40,
	
	/*addr*/
	EPDIR = 0x80,
	EPOUT = 0,
	EPIN = 0x80,
};

void	usbinit(USBDesc *, USBEp *, int, int (*)(u8int));
void	usbirq(void);
void	usbepclear(void);
USBEp*	usbepcfg(u8int, u8int, int, void(*)(USBEp*), void *);
void	usbepdecfg(USBEp*);
int	usbepsendnb(USBEp*, void *, uint);
int	usbeprecvnb(USBEp*, void *, uint);

/* class definitions */
void	usbacminit(void);
extern CQueue	usbacmrxqu, usbacmtxqu;

/* user-provided */
void	usbdisconnect(int);

/* internal functions */
void	usbep0complete(USBReq *);
void	usbep0begin(USBReq *);
int	usbep0req(USBReq *);
void	usbep0reset(void);
void	usbphyinit(void);
void	usbphyreset(void);
void	usbreset(void);
void	usbphysetaddr(u8int);
int	usbphyepcfg(USBEp *);
void	usbphyepdecfg(USBEp *);

extern int	(*usbconfig)(u8int);
extern USBEp*	usbep;
extern int	usbmaxep;

#define usbdebug(...)
