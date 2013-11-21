// GRMN/GRMN protocol display program
//
// This works under MSC v7.0.  good luck with anything else.
// Written by Tim Hogard  9/20/95
//    This code was developed based on info from:
//	Bernard Greening <bernard@igc.apc.org>  (rs-232 code)
//	William Soley <william.soley@sun.com>   (garmin protocol)
//	Other that wish to remain anonymous.
// This program is in the public domain.
// 	If you wish to help with this please let me know.
// Find a new version at http://www.inmind.com/~thogard or www.abnormal.com
// email:  thogard@inmind.com
// Ver: Wed Sep 20 09:39:59 EDT 1995
//
// To exit program, type Alt-F (Windows style for Alt-F,x)
//

#include <stdio.h>
#include <dos.h>

#ifdef BORLAND  // adjust a few things for Borloand...
#define _enable() enable()
#define _disable() disable()
#define _inp(x) inportb(x)
#define _outp(x,y) outportb(x,y)
#endif	// #def BORLAND

//----------------------------
/*      Offsets to various 8250 registers.  Taken from IBM Technical         */
/*      Reference Manual, p. 1-225                                           */

#define TXBUF   0                       /* Transmit buffer register */
#define RXBUF   0                       /* Receive buffer register */
#define DLLSB   0                       /* Divisor latch LS byte */
#define DLMSB   1                       /* Divisor latch MS byte */
#define IER     1                       /* Interrupt enable register */
#define IIR     2                       /* Interrupt ID register */
#define LCR     3                       /* Line control register */
#define MCR     4                       /* Modem control register */
#define LSR     5                       /* Line status register */
#define MSR     6                       /* Modem status register */

#define INT_CTRL 0x20                    /* 8259A Port address */
#define INT_MASK 0x21                    /* 8259A Port address */
#define EOI     0x20                    /* 8259A EOI command */
/// stuff above here should go in a .h file

// code below works on a standard com2 only.  Use dos
// mode com2: 9600 to set baud rate.
// INT is the dos interrupt vector number
#define INT	0xb

char *months[]={"no0","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug",
"Sep","Oct", "Nov","Dec"};

void (cdecl interrupt far *old_int[4])();
int count=0;
int auto_repeat=0;

//port settings:  com1: com2  com3  com4
int port_addr[4]={0x2f8,0x2f8,0x2f8,0x2f8};	// all com2 for now
int port_int[4]= {  0xb,  0xb,  0xb,  0xb};
int port_bit[4]= { 0xf7, 0xf7, 0xf7, 0xf7};
enum port_baud {b150=0x300,b300=0x1c0,b600=0xc0,b1200=0x60,b2400=0x30,
b4800=0x18,b9600=0xc,b192=0x7fe9};

char rx_buf[4][256],*rx_ptr[4];


// This is the basis for the ISR code.
#define ISR_PROTOTYPE(y)  { 						\
	int x=y; 						\
	int s;							\
	s=inp(port_addr[x]+IIR);					\
		*rx_ptr[x]++=inp(port_addr[x]+RXBUF);		\
	outp(INT_CTRL,EOI);					\
	if(rx_ptr[x]>rx_buf[x]+sizeof(rx_buf[x])-1)		\
		rx_ptr[x]=rx_buf[x];				\
	_chain_intr(old_int[x]);				\
}		

// We must have one ISR for each interrupt since there is no way
// to pass parameters into the ISR function.
// This should be 4 functions that call one common functions but
// this does not work reliably on all PCs with all compilers so
// its done the hard way.
void interrupt far isr_0() ISR_PROTOTYPE(0)
void interrupt far isr_1() ISR_PROTOTYPE(0)
void interrupt far isr_2() ISR_PROTOTYPE(0)
void interrupt far isr_3() ISR_PROTOTYPE(0)

// codes hiding in the arrays:
// EOM: end of message
// CHK_SUM: put check sum here
// START_CHK_SUM: start checksum here
// USE_ARGV: argv[1]
enum { EOM=-4, CHK_SUM, START_CHK_SUM, USE_ARGV };  // Must not be 0..255

int send_init[]={0x10,START_CHK_SUM,0xfe,0,CHK_SUM,0x10,3,EOM};
int send_init_spare[]={START_CHK_SUM,0x10,0xfe,0,2,0x10,3,EOM};
int send_alminac[]={0x10,START_CHK_SUM,0xa,2,1,0,CHK_SUM,0x10,3,EOM};
int send_begin_ack[]={0x10,START_CHK_SUM,0x6,2,0xfe,0,CHK_SUM,0x10,3,EOM};
int send_clock[]={0x10,START_CHK_SUM,0xa,2,5,0,CHK_SUM,0x10,3,EOM};// clock
int send_position[]={0x10,START_CHK_SUM,0xa,2,2,0,CHK_SUM,0x10,3,EOM};// clock
//int send_bad_position[]={0x10,START_CHK_SUM,0x11,0x20,0,0,0,0,0,0,0xf8,0x3f,
//	0,0,0,0,0,0,0xf8,0x3f, CHK_SUM,0x10,3,EOM1};// faked position
int send_request[]={0x10,START_CHK_SUM,0xa,2,USE_ARGV,0,CHK_SUM,0x10,3,EOM};// clock

char rxbuf[256]; int rxcount;
int *out;
#define debug 0

ser_open(int x)	{	// x will be 2 for com 2; for now ignore
	union REGS reg;
	_disable();
	old_int[0]=_dos_getvect(INT);
	_dos_setvect(INT,isr_0);
	reg.h.al=0xe3;	// 1110 0011  E3
	reg.h.ah=0;
	reg.x.dx=1;
int86(0x14, &reg, &reg); // this may help windows know were using the port
	outp(port_addr[0]+LCR,0x80);	// get at baud rate
	outp(port_addr[0]+DLLSB,b9600);
	outp(port_addr[0]+DLMSB,b9600>>8);
	outp(port_addr[0]+LCR,0x3);	// 8 bits, 1 st, np
	outp(port_addr[0]+IER,0x1);	// turn on Rx Interrupt
	outp(INT_MASK,inp(INT_MASK)&port_bit[0]);	// turn on 8259A irq
	outp(port_addr[0]+MCR,inp(port_addr[0]+MCR)|0x9);// Modem control register
	inp(port_addr[0]+RXBUF);	// flush buffer
	inp(port_addr[0]+RXBUF);	// flush buffer
	_enable();
}

bail() {
	_disable();
	outp(port_addr[0]+IER,0x0);	// turn off interrupts
	outp(INT_MASK,inp(INT_MASK)|~port_bit[0]);	// turn off 8259A irq
	_dos_setvect(INT,old_int[0]);
	_enable();
	exit(0);
}

main(int argc, char **argv) {
	int c;
	int last=0;
	int chksum=0;
	int rxcount=0;
	char *rptr;
printf("Opened Com2:\nHit Alt-F to exit\n");
	rptr=rx_ptr[0]=rx_buf[0];

	ser_open(2);	// init com2:
	//out=send_init;
	//out=send_alminac;
	out=send_clock;
//	out=send_position;
//	out=send_request;
	while(1) {
		if(*out!=EOM) {
			if(inp(port_addr[0]+LSR)&0x20) {
			//***** need check here to see if last Tx is done
			// Check bit 0x20 of port 0x2fd before Tx
			if(*out==USE_ARGV) {	// atoi number...
				if(debug) printf("argv(%x)\n",atoi(argv[1]));
				chksum+=atoi(argv[1]);
				outp(port_addr[0]+TXBUF,atoi(argv[1]));
			} else if(*out==CHK_SUM) {
				if(debug) printf("chk(%d)\n",256-(chksum&0xff));
				outp(port_addr[0]+TXBUF,256-(chksum&0xff));
			} else if(*out==START_CHK_SUM) {
				chksum=0;
			} else {
				chksum+=*out;
				outp(port_addr[0]+TXBUF,*out);
				if(debug) printf("%02x ",*out);
			}
			out++;
			}
		}
		while(kbhit()) {
			c=getch();
			if(c==0) {
				c=getch();
				switch(c) {
				case 33: 	// alt-F (File menu)
					bail(); break;
				case 134:	// F12 Toggle repeat
					auto_repeat=!auto_repeat; break;
				case 59:	// F1 - help
					help(); break;
				case 60:	// F2 - init
					out=send_init; break;
				case 61:	// F3 - init
					out=send_alminac; break;
				case 62:	// F4 - init
					out=send_begin_ack; break;
				case 63:	// F5 - init
					out=send_clock; break;
				case 64:	// F6 - init
					out=send_position; break;
				case 65:	// F7 - init
					out=send_request; break;
				default:
					printf("Key 0,%d pressed\n",c);
				}
			} else
				outp(port_addr[0]+TXBUF,c);
		}
		while(rptr!=rx_ptr[0]) {
			c=*rptr++;
			if(rptr>rx_buf[0]+sizeof(rx_buf[0])-1)
				rptr=rx_buf[0];
			if (debug) printf("%02x %c ",c,c>=' '?c:'?',count);
			//printf("%c",c);
			if(last==0x10) {
				if(c==0x3) {	//done
					done(rxbuf,rxcount);
					rxcount=255;
				} else if(c==0x10) {
					;//rxbuf[255&rxcount++]=0x10;
				} else {
					rxbuf[255&rxcount++]=0x10;
					//rxbuf[255&rxcount++]=c;
				}
				last=0;
			}
			if(c!=0x10)
				rxbuf[255&rxcount++]=c;
			else
				last=0x10;
		}
	}
}

// code to do real stuff goes here.  

// done gets called when the above crud finds what appears to be a complete
// grmn protocol sentence.
// below you can get at the data:
//	 r[0] is a 0x10,
//	 r[1] is the code,
//	 r[2] is the length,
//	 r[3] is the 1st data byte,
// use casting macros to get other stuff   short(r[5]) will make a 16 bit
// 	number out of r[5] and r[6];  its a shortcut for (*(int *)&r[5])
//
#define short(x) (*(int *)&x)
#define long(x) (*(long *)&x)
#define float(x) (*(float *)&x)
#define double(x) (*(double *)&x)
done(unsigned char *r,int rxcount) {
if(debug) printf("\n:%x c(%x) len(%x)\n",r[0],r[1],r[2]);
	switch(r[1]) {
	case 0x0:
		if(r[2]!=4) printf("len=%d ",r[2]);
		printf("Signal %04x %04x\n", short(r[3]),short(r[5]));
		break;
	case 0x3: printf("done\n");
		break;
	case 0x6:
		if(debug) printf("ack\n");
		break;
	case 0xd:
		if(r[2]!=4) printf("len=%d ",r[2]);
		printf("Event %02x %02x %02x %02x\n", r[3],r[4],r[5],r[6]);
		break;
	case 0xe:
		if(r[2]!=4) printf("len=%d ",r[2]);
		printf("Clock %s %02d %04d ", months[r[3]],r[4],short(r[5]));
		printf("%02d:%02d:%02d (+%d) ", r[7],r[9],r[10],r[8]);// guess about 8
		//printf(" %02d %02d %02d %02d\n", r[11],r[12],r[13],r[14]);
		printf(" %08lx (%20ld)\n", long(r[11]),long(r[11]));
		if(auto_repeat) out=send_clock; // try it again...
		break;
	case 0x11:
		if(r[2]!=4) printf("len=%d ",r[2]);
		printf("Position %15.15g %15.15g \n", double(r[3]),double(r[11]));
	if(auto_repeat) out=send_position;
		break;
	case 0x15:
		printf("nack %02x\n",r[3]);
		break;
	case 0x1f:	// Almanac Data
		if(r[2]!=4) printf("len=%d ",r[2]);
		printf("Almanac week:%02x %g %g %g %g %g %g %g %g %g %g\n",
		short(r[3]),
		float(r[5]),float(r[5+4]),	// time, Af0
		float(r[5+8]),float(r[5+12]),	// Af1,Eccentricity,
		float(r[5+12+4]),float(r[5+12+8]), // sqrt(a) m^1/2 mean anon
		float(r[5+12+12]),float(r[5+12+16]), //perigee, RA at TOA
		float(r[5+16+16]),float(r[5+32+4]));  // Rate of RA // Orbital Inclination
		out=send_begin_ack;	// generic ack
		break;
	case 0x1b:
		printf("Beginxfer %d\n",short(r[3]));
		out=send_begin_ack;	// should ack the request but it
			// isn't going to build the ack array yet.
		break;
	case 0x27:	// in test mode 
		if(r[2]!=22) printf("len=%d ",r[2]);
		printf("Unknown 27 %08x %08x %08x %08x\n", long(r[3]),long(r[7]),
long(r[7+4]),long(r[7+8]));
		out=send_begin_ack;	// should ack the request but it
		break;
	case 0x28:	// in test mode
		if(r[2]!=4) printf("len=%d ",r[2]);
		printf("Unknown 28 %08x %08x %08x %08x\n", long(r[3]),long(r[7]),
long(r[7+4]),long(r[7+8]));
		break;
	case 0xff:
		if(r[2]!=14) printf("len=%d ",r[2]);
		printf("version %02x %02x %d \"%18.18s\"\n",// 14.14 for GPS75
//			r[3],r[4],*(int *)&r[5],&r[7]);
			r[3],r[4],short(r[5]),&r[7]);
		break;
	}
}

help() {
	printf("Hit Alt-F to exit (from Alt-F X to exit win programs)\n\n");
	printf("F1-Help\n");
	printf("F2-Get ID\n");
	printf("F3-Send Alminac\n");
	printf("F4-Send ack\n");
	printf("F5-Send clock\n");
	printf("F6-Send position\n");
	printf("F7-Send command line request (Start w/ \"program 2\" to get pos\n");
	printf("F12-Auto repeat\n");
}
