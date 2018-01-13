#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdint.h>

typedef struct Field Field;
typedef struct DescType DescType;
typedef struct Desc Desc;

struct Field {
	char *name;
	int size;
	enum { FMAND, FOPT, FLEN, FTLEN, FSTRING, FCONST, FBYTES } type;
	uint32_t val;
	char *data;
};

struct DescType {
	char *name;
	Field *d;
	int size;
};

struct Desc {
	char *name;
	uint16_t idx, len;
	Field *f;
	Desc *next;
};

static char *typeids[] = {
	NULL,
	"Device",
	"Configuration",
	"String",
	"Interface",
	"Endpoint",
	"Device_Qualifier",
};

static Field devfields[] = {
	{"bLength", 1, FLEN, 0},
	{"bDescriptorType", 1, FCONST, 1},
	{"bcdUSB", 2, FOPT, 0x0200},
	{"bDeviceClass", 1, FOPT, 0xff},
	{"bDeviceSubClass", 1, FOPT, 0xff},
	{"bDeviceProtocol", 1, FOPT, 0xff},
	{"bMaxPacketSize0", 1, FOPT, 64},
	{"idVendor", 2, FMAND, 0},
	{"idProduct", 2, FMAND, 0},
	{"bcdDevice", 2, FOPT, 0},
	{"iManufacturer", 1, FSTRING, 0},
	{"iProduct", 1, FSTRING, 0},
	{"iSerialNumber", 1, FSTRING, 0},
	{"bNumConfigurations", 1, FOPT, 1},
	{NULL}
};

static Field devqfields[] = {
	{"bLength", 1, FLEN, 0},
	{"bDescriptorType", 1, FCONST, 6},
	{"bcdUSB", 2, FOPT, 0x0200},
	{"bDeviceClass", 1, FOPT, 0xff},
	{"bDeviceSubClass", 1, FOPT, 0xff},
	{"bDeviceProtocol", 1, FOPT, 0xff},
	{"bMaxPacketSize0", 1, FOPT, 64},
	{"bNumConfigurations", 1, FOPT, 1},
	{"bReserved", 1, FCONST, 0},
	{NULL}
};

static Field conffields[] = {
	{"bLength", 1, FLEN, 0},
	{"bDescriptorType", 1, FCONST, 2},
	{"wTotalLength", 2, FTLEN, 0},
	{"bNumInterfaces", 1, FMAND, 0},
	{"bConfigurationValue", 1, FMAND, 0},
	{"iConfiguration", 1, FSTRING, 0},
	{"bmAttributes", 1, FOPT, 0x80},
	{"bMaxPower", 1, FOPT, 250},
	{NULL}
};

static Field intffields[] = {
	{"bLength", 1, FLEN, 0},
	{"bDescriptorType", 1, FCONST, 4},
	{"bInterfaceNumber", 1, FMAND, 0},
	{"bAlternateSetting", 1, FOPT, 0},
	{"bNumEndpoints", 1, FMAND, 0},
	{"bInterfaceClass", 1, FOPT, 0xff},
	{"bInterfaceSubClass", 1, FOPT, 0xff},
	{"bInterfaceProtocol", 1, FOPT, 0},
	{"iInterface", 1, FSTRING, 0},
	{NULL}
};

static Field epfields[] = {
	{"bLength", 1, FLEN, 0},
	{"bDescriptorType", 1, FCONST, 5},
	{"bEndpointAddress", 1, FMAND, 0},
	{"bmAttributes", 1, FMAND, 0},
	{"wMaxPacketSize", 2, FMAND, 0},
	{"bInterval", 1, FMAND, 0},
	{NULL}
};

static Field stringfields[] = {
	{"bLength", 1, FLEN, 0},
	{"bDescriptorType", 1, FCONST, 3},
	{"bString", 0, FBYTES, 0},
	{NULL}
};

static Field cdcheadfields[] = {
	{"bFunctionLength", 1, FLEN, 0},
	{"bDescriptorType", 1, FCONST, 0x24},
	{"bDescriptorSubtype", 1, FCONST, 0x00},
	{"bcdCDC", 2, FOPT, 0x0102},
	{NULL}
};

static Field cdcunionfields[] = {
	{"bFunctionLength", 1, FLEN, 0},
	{"bDescriptorType", 1, FCONST, 0x24},
	{"bDescriptorSubtype", 1, FCONST, 0x06},
	{"bControlInterface", 1, FMAND, 0},
	{"bSubordinateInterface", 0, FBYTES, 0},
	{NULL}
};

static Field cdcacmfields[] = {
	{"bFunctionLength", 1, FLEN, 0},
	{"bDescriptorType", 1, FCONST, 0x24},
	{"bDescriptorSubtype", 1, FCONST, 0x02},
	{"bmCapabilities", 1, FMAND, 0x00},
	{NULL}
};

static DescType types[] = {
	{"Device", devfields, sizeof(devfields)},
	{"Device_Qualifier", devqfields, sizeof(devqfields)},
	{"Configuration", conffields, sizeof(conffields)},
	{"Interface", intffields, sizeof(intffields)},
	{"Endpoint", epfields, sizeof(epfields)},
	{"String", stringfields, sizeof(stringfields)},
	{"CDC_Header", cdcheadfields, sizeof(cdcheadfields)},
	{"CDC_Union", cdcunionfields, sizeof(cdcunionfields)},
	{"CDC_ACM", cdcacmfields, sizeof(cdcacmfields)},
	{NULL},
};

typedef struct Node Node;

struct Node {
	enum {
		ASTINVAL,
		ASTSYM,
		ASTNUM,
		ASTSTRING,
		ASTCOMMA,
		ASTASS,
		ASTDESC,
	} t;
	Node *n1, *n2, *next;
	char *sym;
	int num;
	int lineno;
	Field *desc;
};

char *nodetypstr[] = {
	[ASTINVAL] "ASTINVAL",
	[ASTSYM] "ASTSYM",
	[ASTNUM] "ASTNUM",
	[ASTSTRING] "ASTSTRING",
	[ASTCOMMA] "ASTCOMMA",
	[ASTASS] "ASTASS",
	[ASTDESC] "ASTDESC"
};

FILE *fin;
char *filen;
int lineno;
Desc *descs;
Desc **lastdesc = &descs;
char *prefix = "usbdesc";

int peekchar = -1, peektok = -1;

static char *
nodetype(int t)
{
	static char buf[128];

	if((unsigned)t >= sizeof(nodetypstr)/sizeof(char*) || nodetypstr[t] == NULL){
		sprintf(buf, "%d", t);
		return buf;
	}
	return nodetypstr[t];
}

static void *
emalloc(size_t n)
{
	void *v;
	
	v = malloc(n);
	if(v == NULL){
		fprintf(stderr, "malloc: %s\n", strerror(errno));
		exit(1);
	}
	memset(v, 0, n);
	return v;
}

static void
report(int line, char *fmt, ...)
{
	va_list va;
	
	fprintf(stderr, "%s:%d: ", filen, line);
	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	fprintf(stderr, "\n");
	exit(1);
}
#define fatal report

static Node *
node(int t, ...)
{
	va_list va;
	Node *n;
	
	n = emalloc(sizeof(Node));
	va_start(va, t);
	n->t = t;
	n->lineno = lineno;
	switch(t){
	case ASTSYM:
	case ASTDESC:
	case ASTSTRING:
		n->sym = va_arg(va, char *);
		break;
	case ASTNUM:
		n->num = va_arg(va, int);
		break;
	case ASTCOMMA:
	case ASTASS:
		n->n1 = va_arg(va, Node *);
		n->n2 = va_arg(va, Node *);
		break;
	default:
		fatal(lineno, "node: unknown type %s", nodetype(t));
	}
	va_end(va);
	return n;
}

static void
astprint(Node *n, int env)
{
	Node *m;

	if(n == NULL){
		printf("NULL");
		return;
	}
	switch(n->t){
	case ASTSYM: printf("%s", n->sym); break;
	case ASTNUM: printf("%d", n->num); break;
	case ASTCOMMA:	
		if(env >= 20) printf("(");
		astprint(n->n1, 20);
		printf(", ");
		astprint(n->n2, 20);
		if(env >= 20) printf(")");
		break;
	case ASTASS:
		if(env >= 10) printf("(");
		astprint(n->n1, 10);
		printf(" = ");
		astprint(n->n2, 10);
		if(env >= 10) printf(")");
		break;
	case ASTDESC:
		printf("%s {", n->sym);
		for(m = n->n1; m != NULL; m = m->next){
			astprint(m, 0);
			printf("; ");
		}
		printf("}");
		break;
	default:
		printf("??? (%d)", n->t);
	}
}

static Field *
mkdesc(char *s)
{
	DescType *dt;
	Field *rv;
	
	for(dt = types; dt->name != NULL; dt++)
		if(strcmp(dt->name, s) == 0)
			break;
	if(dt->name == NULL)
		return NULL;
	rv = emalloc(dt->size);
	memcpy(rv, dt->d, dt->size);
	return rv;
}

static int
getch(void)
{
	int c;
	
	if(peekchar >= 0){
		c = peekchar;
		peekchar = -1;
		return c;
	}
	c = getc(fin);
	if(c == '\n')
		lineno++;
	return c;
}

static void
ungetch(int c)
{
	peekchar = c;
}

static int
peekch(void)
{
	return peekchar = getch();
}

enum {
	TSYM = 0x100,
	TNUM = 0x101,
	TSTRING = 0x102,
};
char lexbuf[1024];

static int
lex(void)
{
	int c;
	char *p;

	if(peektok >= 0){
		c = peektok;
		peektok = -1;
		return c;
	}
loop:
	while(c = getch(), c >= 0 && isspace(c))
		;
	if(c < 0) return -1;
	if(c == '/'){
		if(peekch() == '/'){
			while(c = getch(), c >= 0 && c != '\n')
				;
			goto loop;
		}
		if(peekch() == '*'){
			getch();
		c0:	c = getch();
			if(c < 0) report(lineno, "eof in comment");
			if(c != '*') goto c0;
		c1:	c = getch();
			if(c < 0) report(lineno, "eof in comment");
			switch(c){
			case '*': goto c1;
			case '/': goto loop;
			default: goto c0;
			}
		}
		return c;
	}
	if(isalpha(c) || c == '_' || c >= 0x80){
		p = lexbuf;
		*p++ = c;
		while(c = getch(), c >= 0 && (isalnum(c) || c == '_' || c >= 0x80))
			if(p < lexbuf + sizeof(lexbuf) - 1)
				*p++ = c;
		ungetch(c);
		*p = 0;
		return TSYM;
	}
	if(isdigit(c)){
		p = lexbuf;
		*p++ = c;
		while(c = getch(), c >= 0 && isalnum(c))
			if(p < lexbuf + sizeof(lexbuf) - 1)
				*p++ = c;
		ungetch(c);
		*p = 0;
		strtol(lexbuf, &p, 0);
		if(*p != 0) report(lineno, "invalid character %c in number", *p);
		return TNUM;
	}
	if(c == '"'){
		p = lexbuf;
		while(c = getch(), c >= 0 && c != '"'){
			if(c == '\\')
				switch(c = getch()){
				case 'n': c = '\n'; break;
				case 't': c = '\t'; break;
				case 'v': c = '\v'; break;
				case 'r': c = '\r'; break;
				default:
					if(c < 0)
						report(lineno, "eof in string");
				}
			if(c < 0) break;
			if(p < lexbuf + sizeof(lexbuf) - 1)
				*p++ = c;
		}
		if(c < 0)
			report(lineno, "eof in string");
		*p = 0;
		return TSTRING;
			
	}
	return c;
}

static int
peek(void)
{
	return peektok = lex();
}

static void
tokstr(int l, char *buf)
{
	if(l >= 32 && l < 127){
		buf[0] = l;
		buf[1] = 0;
		return;
	}
	if(l < 0){
		strcpy(buf, "eof");
		return;
	}
	switch(l){
	case TSYM: strcpy(buf, "symbol"); break;
	case TNUM: strcpy(buf, "number"); break;
	case TSTRING: strcpy(buf, "string"); break;
	default: sprintf(buf, "%d", l);
	}
}

static void
expect(int c)
{
	int l;
	char buf1[64], buf2[64];
	
	l = lex();
	if(l != c){
		tokstr(c, buf1);
		tokstr(l, buf2);
		report(lineno, "expected %s, got %s", buf1, buf2);
	}
}

static int
match(int c)
{
	if(peek() != c) return 0;
	lex();
	return 1;
}

static Node *topassign(void);

static Node *
topexpr(void)
{
	int c;
	Node *n, *m, **lp;
	char *s;

	char buf1[64];

	switch(c = lex()){
	case TSYM:
		s = strdup(lexbuf);
		if(match('{')){
			n = node(ASTDESC, s);
			lp = &n->n1;
			while(peek() != '}'){
				m = topassign();
				if(m != NULL){
					*lp = m;
					lp = &m->next;
				}
				expect(';');
			}
			expect('}');
			return n;
		}else
			return node(ASTSYM, s);
	case TNUM:
		return node(ASTNUM, strtol(lexbuf, NULL, 0));
	case TSTRING:
		return node(ASTSTRING, strdup(lexbuf));
	default:
		tokstr(c, buf1);
		report(lineno, "unexpected %s", buf1);
		return NULL;
	}
}

static Node *
topcommas(void)
{
	Node *n;

	n = topexpr();
	while(match(','))
		n = node(ASTCOMMA, n, topexpr());
	return n;
}

static Node *
topassign(void)
{
	Node *n;
	
	n = topcommas();
	if(match('='))
		return node(ASTASS, n, topassign());
	return n;
}

static void
gatherbytes(Node *n, int *np, char **pp)
{
	switch(n->t){
	case ASTNUM:
		if((*np & 63) == 0){
			*pp = realloc(*pp, *np + 64);
			if(*pp == NULL){
				fprintf(stderr, "realloc: %s\n", strerror(errno));
				exit(1);
			}
		}
		(*pp)[(*np)++] = n->num;
		break;
	case ASTCOMMA:
		gatherbytes(n->n1, np, pp);
		gatherbytes(n->n2, np, pp);
		break;
	default:
		report(n->lineno, "gatherbytes: invalid type %s", nodetype(n->t));
	}
}

static void
fromutf8(uint32_t *t, char *n)
{
	int rem;
	uint8_t v;
	uint32_t cp;
	
	rem = 0;
	for(;;){
		v = *n++;
		if((v & 0x80) == 0){
			if(rem > 0) *t++ = 0xfffd;
			*t++ = v;
			if(v == 0) break;
			rem = 0;
		}else if((v & 0xc0) == 0x80){
			if(rem == 0){
				*t++ = 0xfffd;
				continue;
			}
			cp = cp << 6 | v & 0x3f;
			if(--rem == 0)
				*t++ = cp;
		}else if((v & 0xe0) == 0xc0){
			if(rem > 0) *t++ = 0xfffd;
			cp = v & 0x1f;
			rem = 1;
		}else if((v & 0xf0) == 0xe0){
			if(rem > 0) *t++ = 0xfffd;
			cp = v & 0xf;
			rem = 2;
		}else if((v & 0xf8) == 0xf0){
			if(rem > 0) *t++ = 0xfffd;
			cp = v & 7;
			rem = 3;
		}else{
			*t++ = 0xfffd;
			rem = 0;
		}
	}
}

static int
toutf16(uint8_t *t, uint32_t *n)
{
	uint32_t cp;
	uint8_t *t0;
	
	t0 = t;
	while(cp = *n++, cp != 0){
		if(cp < 0x10000){
			*t++ = cp;
			*t++ = cp >> 8;
			continue;
		}
		cp -= 0x10000;
		*t++ = cp >> 10;
		*t++ = 0xd8 | cp >> 18 & 3;
		*t++ = cp;
		*t++ = 0xdc | cp >> 8 & 3;
	}
	t[0] = t[1] = 0;
	return t - t0;
}

static void outdesc(char *, Node *, int);

int stridx = 1;

static void
mklang(void)
{
	Desc *d;
	Node *no;

	if(stridx == 1)
		return;
	
	d = emalloc(sizeof(Desc));
	d->name = strdup("langtab");
	d->idx = 0x300;
	d->f = mkdesc("String");
	d->f[2].size = 2;
	d->f[2].data = strdup("\x09\x04");
	printf("uchar ROM %s_%s[] = {\n", prefix, d->name);
	no = node(ASTDESC, "String");
	no->desc = d->f;
	outdesc(d->name, no, 0);
	printf("};\n");
	*lastdesc = d;
	lastdesc = &d->next;
}

static int
mkstring(char *n8)
{
	int n;
	uint32_t *n32;
	uint8_t *n16;
	Desc *d;
	char buf[64];
	Node *no;
	
	n = strlen(n8) + 1;
	n32 = emalloc(n * 4);
	n16 = emalloc(n * 4);
	fromutf8(n32, n8);
	n = toutf16(n16, n32);
	free(n32);
	
	sprintf(buf, "str_%d", stridx);
	d = emalloc(sizeof(Desc));
	d->name = strdup(buf);
	d->idx = 0x300 | stridx;
	d->f = mkdesc("String");
	d->f[2].size = n;
	d->f[2].data = (char*) n16;
	printf("uchar ROM %s_%s[] = {\n", prefix, d->name);
	no = node(ASTDESC, "String");
	no->desc = d->f;
	outdesc(d->name, no, 0);
	printf("};\n");
	*lastdesc = d;
	lastdesc = &d->next;
	return stridx++;
}

static void
descset(Field *f, char *s, Node *v)
{
	for(; f->name != NULL; f++)
		if(strcmp(f->name, s) == 0)
			break;
	if(f->name == NULL){
		report(v->lineno, "no such field '%s'", s);
		return;
	}
	switch(f->type){
	case FMAND:
	case FOPT:
	case FLEN:
	case FTLEN:
		if(v->t != ASTNUM){
			report(v->lineno, "invalid rval");
			return;
		}
		f->val = v->num;
		f->type = FOPT;
		break;
	case FBYTES:
		gatherbytes(v, &f->size, &f->data);
		break;
	case FSTRING:
		if(v->t != ASTSTRING){
			report(v->lineno, "invalid rval");
			return;
		}
		f->val = mkstring(v->sym);
		f->type = FOPT;
		break;
	default:
		report(v->lineno, "field not assignable"); 
	}
}

static int
totlength(Node *m)
{
	Field *g;
	int len;

	switch(m->t){
	case ASTDESC:
		len = 0;
		for(g = m->desc; g->name != NULL; g++)
			len += g->size;
		return len;
	case ASTCOMMA:
		return totlength(m->n1) + totlength(m->n2);
	default:
		report(m->lineno, "totlength: unknown type %s", nodetype(m->t));
		return 0;
	}
}

static void
outdesc(char *n, Node *m, int tlen)
{
	uint32_t v;
	Field *f, *g;
	int i, len;

	switch(m->t){
	case ASTDESC:
		f = m->desc;
		break;
	case ASTCOMMA:
		outdesc(n, m->n1, tlen);
		outdesc(n, m->n2, tlen);
		return;
	default:
		report(m->lineno, "outdesc: unknown type %s", nodetype(m->t));
		return;
	}
	
	len = 0;
	for(g = f; g->name != NULL; g++)
		len += g->size;

	printf("\t/* %s */\n", m->sym);
	for(; f->name != NULL; f++){
		printf("\t");
		switch(f->type){
		case FMAND:
			report(0, "descriptor %s is missing mandatory field %s", n, f->name);
			break;
		case FTLEN:
			f->val = tlen;
			if(0){
		case FLEN:
			f->val = len;
			}
			/* wet floor */
		case FSTRING:
		case FOPT:
		case FCONST:
			v = f->val;
			for(i = 0; i < f->size; i++){
				printf("0x%.2x, ", v & 0xff);
				v >>= 8;
			}
			break;
		case FBYTES:
			for(i = 0; i < f->size; i++)
				printf("0x%.2x, ", (uint8_t) f->data[i]);
			break;
		default:
			report(0, "outdesc: unknown field type %d", f->type);
		}
		printf("/* %s */\n", f->name);
	}
}

static void
tabdesc(Node *l, Node *r)
{
	char **p;
	Desc *d;
	char buf[128];

	if(l->n1 == NULL || l->n1->t != ASTSYM){
		report(l->lineno, "invalid lval");
		return;
	}
	if(l->n2 == NULL || l->n2->t != ASTNUM || l->n2->num < 0 || l->n2->num > 255){
		report(r->lineno, "invalid lval");
		return;
	}
	for(p = typeids; p < typeids + sizeof(typeids)/sizeof(char *); p++)
		if(*p != NULL && strcmp(*p, l->n1->sym) == 0)
			break;
	if(p == typeids + sizeof(typeids)/sizeof(char *)){
		report(l->lineno, "unknown type %s", l->n1->sym);
		return;
	}
	d = emalloc(sizeof(Desc));
	snprintf(buf, sizeof(buf), "%s%d", *p, l->n2->num);
	d->name = strdup(buf);
	d->idx = p - typeids << 8 | l->n2->num;
	printf("uchar ROM %s_%s[] = {\n", prefix, d->name);
	outdesc(d->name, r, totlength(r));
	printf("};\n");
	d->f = r->desc;
	*lastdesc = d;
	lastdesc = &d->next;
}

static void
eval(Node *n, Field *ctxt)
{
	Node *m;

	if(n == NULL) return;
	switch(n->t){
	case ASTSYM:
	case ASTNUM:
	case ASTSTRING:
		return;
	case ASTASS:
		eval(n->n2, ctxt);
		eval(n->n1, ctxt);
		if(n->n2 == NULL){
			report(n->lineno, "NULL as rval");
			return;
		}
		if(ctxt == NULL && n->n1 != NULL && n->n1->t == ASTCOMMA){
			tabdesc(n->n1, n->n2);
			return;
		}
		if(n->n1 == NULL || n->n1->t != ASTSYM){
			report(n->lineno, "invalid lval");
			return;
		}

		if(ctxt != NULL)
			descset(ctxt, n->n1->sym, n->n2);
		else{
			printf("uchar ROM %s_%s[] = {\n", prefix, n->n1->sym);
			outdesc(n->n1->sym, n->n2, totlength(n->n2));
			printf("};\n");

		}
		break;
	case ASTCOMMA:
		eval(n->n1, ctxt);
		eval(n->n2, ctxt);
		break;
	case ASTDESC:
		n->desc = mkdesc(n->sym);
		if(n->desc == NULL){
			report(n->lineno, "unknown descriptor type '%s'", n->sym);
			return;
		}
		for(m = n->n1; m != NULL; m = m->next)
			eval(m, n->desc);
		break;
	default: report(n->lineno, "eval: unknown node type %s", nodetype(n->t));
	}
}

static void
desctable(void)
{
	Desc *d;
	
	printf("USBDesc ROM %s[] = {\n", prefix);
	for(d = descs; d != NULL; d = d->next)
		printf("\t{0x%.4x, sizeof(%s_%s), %s_%s},\n", d->idx, prefix, d->name, prefix, d->name);
	printf("\t{0, 0, nil},\n");
	printf("};\n");	
}

int
main(int argc, char **argv)
{
	switch(argc){
	case 1:
		fin = stdin;
		filen = "<stdin>";
		break;
	case 3:
		prefix = argv[2];
		/* wet floor */
	case 2:
		fin = fopen(argv[1], "r");
		if(fin == NULL){
			fprintf(stderr, "%s: %s: %s\n", argv[0], argv[1], strerror(errno));
			return -1;
		}
		filen = argv[1];
		break;
	default:
		fprintf(stderr, "usage: %s [ file [ prefix ] ]\n", argv[0]);
	}
	lineno = 1;
	printf("#include <u.h>\n");
	printf("#include <libc.h>\n");
	printf("#include <usb.h>\n");
	while(peek() >= 0){
		eval(topassign(), NULL);
		expect(';');
	}
	mklang();
	if(descs != NULL)
		desctable();
	return 0;
}
