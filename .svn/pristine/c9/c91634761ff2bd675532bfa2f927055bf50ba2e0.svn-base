/*	fkt_select.c

	fkt_select --	program to graphically select a range is a trace
					numbers.

	Copyright (C) 2003, 2004 Samuel Thibault <samuel.thibault@ens-lyon.org>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


*/

#define __USE_ISOC99
#include <config.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

static struct page {
		unsigned long nb;
		unsigned long long cyc;
		unsigned long long time;
} *pages;
static unsigned long long maxtime=0,mintime=0xffffffffffffull;

static unsigned long nbpages=1<<10;

static Display			*display;
static Window			window;
static GC				gcblack,gcwhite,gcred,gcgray;
static int				screen;
static unsigned long	black,white;
static XSizeHints		hints;

/* current viewed bunch of pages */
static unsigned			pageBegin,pageEnd;
/* current window size */
#define	STATUSHEIGHT	18
static unsigned			curW=350,curH=250-STATUSHEIGHT;
/* current selected portion within the window */
static unsigned			selBegin,selEnd;

static struct col {
	unsigned long long firstValue;
	unsigned long long lastValue;
	unsigned long long minValue;
	unsigned long long maxValue;
	unsigned long minPage;
	unsigned long maxPage;
} *cols=NULL;

static void recompute( void )
	{
	struct col *col;
	struct page *page;
	unsigned value;
	unsigned long long cycwide=pages[pageEnd].cyc-pages[pageBegin].cyc;

	selBegin=curW;
	selEnd=0;
	cols=realloc(cols,curW*sizeof(*cols));
	
	page=pages+pageBegin;
	for(col=cols; col<cols+curW; col++)
		{
		col->minValue=0xffffffffffffffffULL;
		col->maxValue=0;
		col->minPage=page->nb;
		col->lastValue=col->firstValue=((page->time-mintime)*((long long)curH))/(maxtime-mintime);
		while(page->cyc-pages[pageBegin].cyc<((unsigned long long)(col+1-cols))*cycwide/curW)
			{
			col->lastValue=value=((page->time-mintime)*((long long)curH))/(maxtime-mintime);
			if( value > col->maxValue )
				col->maxValue = value;
			if( value < col->minValue )
				col->minValue = value;
			page++;
			}
		col->maxPage=(page-1)->nb;
		}
	}


static void redrawStatus( unsigned mousex, unsigned mousey )
	{
	static char buffer[128],*c;
	buffer[1]='\0';
	c=buffer;
	if( selBegin<=selEnd )
		{
		c+=snprintf(c,128-(c-buffer)," %lldMcy -> %lldMcy = %lldMcy",
						pages[cols[selBegin].minPage].cyc/1000000ULL,
						pages[cols[selEnd].maxPage].cyc/1000000ULL,
						(pages[cols[selEnd].maxPage].cyc-
							pages[cols[selBegin].minPage].cyc)/1000000ULL);
		}
	if( mousex >= 0 && mousex < curW && mousey >= 0 && mousey < curH )
		{
		c+=snprintf(c,128-(c-buffer)," %lldMcy",
						(pages[cols[mousex].minPage].cyc +
							pages[cols[mousex].maxPage].cyc)/2000000ULL);
		if( cols[mousex].minValue<=cols[mousex].maxValue )
			c+=snprintf(c,128-(c-buffer),"(%lldKcy)",
				(((cols[mousex].minValue+cols[mousex].maxValue)
				*(maxtime-mintime))/(2ULL*((long long)curH))+mintime)/1000ULL);
		c+=snprintf(c,128-(c-buffer)," %lldKcy",
						((((long long)mousey)*(maxtime-mintime))/
							((long long)curH)+mintime)/1000ULL);
		}
	XFillRectangle(display,window,gcwhite,0,curH,curW,STATUSHEIGHT);
	XDrawImageString(display,window,gcblack,2,curH+STATUSHEIGHT-2,buffer+1,strlen(buffer+1));
	}

static void redraw( void )
	{
	unsigned x;
	unsigned lastx=0,lasty=0;

	XClearWindow(display,window);
	if( selEnd>=selBegin && selEnd>selBegin+1 )
		XFillRectangle(display,window,gcgray,selBegin+1,0,selEnd-1-selBegin,curH);
	for( x = 0; x < curW; x++ )
		{
		if( cols[x].maxValue>=cols[x].minValue )
			{
			if( x )
				XDrawLine(display,window,gcred,lastx,lasty,x,cols[x].firstValue);
			XDrawLine(display,window,gcred,x,cols[x].maxValue,x,cols[x].minValue);
			lastx = x; lasty = cols[x].lastValue;
			}
		}
	if( selEnd>=selBegin )
		{
		XDrawLine(display,window,gcblack,selBegin,0,selBegin,curH-1);
		XDrawLine(display,window,gcblack,selEnd,0,selEnd,curH-1);
		}
	redrawStatus(-1,-1);
	}

void updatebutton( int button, int x )
	{
	if( x < 0 ) x = 0;
	if( x >= curW ) x = curW-1;
	switch( button )
		{
		case 1:
			selBegin = x;
			if( selEnd < selBegin )
				selEnd = x;
			break;
		case 2:
			selBegin = selEnd = x;
			break;
		case 3:
			selEnd = x;
			if( selEnd < selBegin )
				selBegin = x;
			break;
		}
	redraw();
	}

unsigned *stack, stacksize=16, stackptr;

void zoomin( void )
	{
	unsigned *newstack;
	if( selEnd <= selBegin ) return;
	if( stackptr == stacksize )
		{
		if( !(newstack = realloc(stack,(stacksize*2)*sizeof(stack[0]))) )
			{
			fprintf(stderr,"couldn't allocate bigger stack\n");
			return;
			}
		else
			{
			stacksize <<= 1;
			stack = newstack;
			}
		}
	stack[stackptr++] = pageBegin;
	stack[stackptr++] = pageEnd;
	pageBegin=cols[selBegin].minPage;
	pageEnd=cols[selEnd].maxPage;
	recompute();
	redraw();
	}


void zoomout( void )
	{
	unsigned *newstack;
	if( !stackptr ) return;
	pageEnd = stack[--stackptr];
	pageBegin = stack[--stackptr];
	recompute();
	redraw();
	if( stackptr < stacksize/2 )
		{
		if( (newstack = realloc(stack,(stacksize/2)*sizeof(stack[0]))) )
			{
			stack = newstack;
			stacksize >>= 1;
			}
		}
	}

static void usage( char *argv0 )
	{
	fprintf(stderr,"\
usage: %s file.timedat\n\
	where file.timedat was produced by fkt_timestats\n\
",argv0);
	exit(EXIT_FAILURE);
	}

int main( int argc, char *argv[] )
	{
	int				done,i;
	XEvent			event;
	char			keystr[10];
	KeySym			keysym;
	FILE*			fd;
	unsigned long	page;
	XColor			color,exact;
	Colormap		cmap;
	int				curbutton=0;

	if( argc < 2 )
		usage(argv[0]);

	if( !(fd=fopen(argv[1],"r")) )
		{
		fprintf(stderr,"couldn't open %s\n",argv[1]);
		perror("open");
		exit(EXIT_FAILURE);
		}

	if( !(pages=malloc(nbpages*sizeof(*pages))) )
		{
		perror("malloc");
		exit(EXIT_FAILURE);
		}

	if( !(stack=malloc(stacksize*sizeof(stack[0]))) )
		{
		perror("malloc");
		exit(EXIT_FAILURE);
		}

	page=0;
	while (fscanf(fd,"%lu %llu %llu\n",&pages[page].nb,&pages[page].cyc,&pages[page].time) == 3)
		{
		if( !(page%1000) )
			fprintf(stderr,"\r%lu",page);
		if( pages[page].nb!=page )
			{
			fprintf(stderr,"bad time file\n");
			exit(EXIT_FAILURE);
			}
		if( pages[page].time > maxtime )
			maxtime = pages[page].time;
		if( pages[page].time < mintime )
			mintime = pages[page].time;
		if( ++page == nbpages )
			{
			nbpages <<= 1;
			if( !(pages=realloc(pages,nbpages*sizeof(*pages))) )
				{
				perror("realloc");
				exit(EXIT_FAILURE);
				}
			}
		}
	fprintf(stderr,"\r%lu pages\ntype 'h' for help\n",page);
	fclose(fd);
	nbpages = page;
	pageBegin = 0;
	pageEnd = nbpages-1;
	if( !(pages=realloc(pages,nbpages*sizeof(*pages))) )
		{
		perror("realloc");
		exit(EXIT_FAILURE);
		}

	if( !(display=XOpenDisplay("")) )
		{
		exit(EXIT_FAILURE);
		}
	screen=DefaultScreen(display);
	white=WhitePixel(display,screen);
	black=BlackPixel(display,screen);

	hints.width=curW; hints.height=curH + STATUSHEIGHT;
	hints.flags=PSize;

	recompute();

	window=XCreateSimpleWindow(display, DefaultRootWindow(display),
					hints.x, hints.y, hints.width, hints.height, 5 /* border */,
					black, white);

	XSetStandardProperties(display, window, "fkt selection", "fkt selection",
					None, argv, argc, &hints);

	gcblack = XCreateGC(display,window,0,0);
	gcwhite = XCreateGC(display,window,0,0);
	gcred = XCreateGC(display,window,0,0);
	gcgray = XCreateGC(display,window,0,0);
	XSetBackground(display, gcblack, white);
	XSetBackground(display, gcwhite, white);
	XSetBackground(display, gcred, white);
	XSetBackground(display, gcgray, white);
	XSetForeground(display, gcblack, black);
	XSetForeground(display, gcwhite, white);

	cmap=XDefaultColormap(display,screen);
	if( !XAllocNamedColor(display,cmap,"Red",&exact,&color) )
		{
		fprintf(stderr,"couldn't get red color");
		exit(EXIT_FAILURE);
		}
	XSetForeground(display, gcred, color.pixel);

	if( !XAllocNamedColor(display,cmap,"Gray",&exact,&color) )
		{
		fprintf(stderr,"couldn't get red color");
		exit(EXIT_FAILURE);
		}
	XSetForeground(display, gcgray, color.pixel);

	XSelectInput(display, window, ButtonPressMask | ButtonReleaseMask
				| PointerMotionMask | KeyPressMask | ExposureMask
				| StructureNotifyMask );
	XMapRaised(display, window);

	done=0;
	while(!done)
		{
		XNextEvent(display,&event);
		switch(event.type)
			{
			case Expose:
				if( !event.xexpose.count )
					redraw();
				break;
			case ConfigureNotify:
				if( curW != event.xconfigure.width || curH != event.xconfigure.height )
					{
					curW = event.xconfigure.width;
					curH = event.xconfigure.height-STATUSHEIGHT;
					recompute();
					}
				redraw();
				break;
			case MappingNotify:
				XRefreshKeyboardMapping((XMappingEvent *)&event);
				break;
			case ButtonPress:
				curbutton = event.xbutton.button;
				updatebutton(curbutton,event.xbutton.x);
				break;
			case MotionNotify:
				if( curbutton )
					updatebutton(curbutton,event.xmotion.x);
				redrawStatus(event.xmotion.x, event.xmotion.y);
				break;
			case ButtonRelease:
				curbutton = 0;
				break;
			case KeyPress:
				i = XLookupString((XKeyEvent *)&event, keystr, 10, &keysym, 0);
				if( i == 1 ) /*len*/ 
				switch( tolower(keystr[0]) )
					{
					case 'q':
						done = 1;
						break;
					case 's':
						printf("selection: %lu %lu\n",cols[selBegin].minPage,
										cols[selEnd].maxPage);
						break;
					case 'z':
						zoomin();
						break;
					case 'u':
						zoomout();
						break;
					case 'h':
						fprintf(stderr,"\
keys:\n\
	q quits\n\
	s shows the selection page numbers, to give to the fkt_extract command\n\
	z zooms on the selected area, after pushing current view\n\
	u unzooms (unpops)\n\
	h you just pressed it\n\
\n\
mouse buttons:\n\
	left choose left boundary\n\
	right choose rigth boundary\n\
	middle choose left & right boundary (not really useful)\n\
");
					}
				break;
			}
		}
	XFreeGC(display,gcblack);
	XFreeGC(display,gcwhite);
	XFreeGC(display,gcred);
	XFreeGC(display,gcgray);
	XDestroyWindow(display,window);
	XCloseDisplay(display);
	exit(EXIT_SUCCESS);
	}

/* vim: ts=4
 */
