/*
 * UCSD p-System virtual machine
 * Copyright (C) 2000 Mario Klebsch
 * Copyright (C) 2006, 2010 Peter Miller
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Core.h>

#include <lib/progname.h>
#include <lib/version.h>


#define XSIZE 280
#define YSIZE 192

#define PENCOLOR_NONE           0
#define PENCOLOR_WHITE          1
#define PENCOLOR_BLACK          2
#define PENCOLOR_REVERSE        3
#define PENCOLOR_RADAR          4
#define PENCOLOR_BLACK1         5
#define PENCOLOR_GREEN          6
#define PENCOLOR_VIOLET         7
#define PENCOLOR_WHITE1         8
#define PENCOLOR_BLACK2         9
#define PENCOLOR_ORANGE         10
#define PENCOLOR_BLUE           11
#define PENCOLOR_WHITE2         12

#define NUMBER(a)       (sizeof(a)/sizeof(a[0]))

static const char       *ColorNames[]={NULL, "white", "black", NULL,  NULL,
                                       "black", "green",  "violet", "white",
                                       "black", "orange", "blue",   "white"};

static GC               ColorGCs[NUMBER(ColorNames)];

static struct
{
  int   xMin;
  int   xMax;
  int   yMin;
  int   yMax;
} CurrentViewPort;
static int      CurrentPenColor;
static int      CurrentTurtleX;
static int      CurrentTurtleY;
static int      CurrentTurtleAng;
static int      CurrentCharType;

static unsigned char    *Buffer  = NULL;

XtAppContext    App;
Widget          Top;
Widget          w;

static void UpdatePixel(int x, int y)
{
  unsigned pixel = Buffer[y * 280 / 2 + x / 2];
  if (x&1)
    pixel >>= 4;
  else
    pixel &= 0x0f;

  if (pixel < NUMBER(ColorGCs) && ColorGCs[pixel])
  {
    XFillRectangle(XtDisplay(w), XtWindow(w), ColorGCs[pixel], x*2, (191-y)*2,
      2, 2);
  }
}

static void UpdatePlane(int xmin, int xmax, int ymin, int ymax)
{
  int x, y;

  for (y=ymin; y<ymax; y++)
    for (x=xmin; x<xmax; x++)
      UpdatePixel(x, y);
}

static int GetPixel(int x, int y)
{
  char ch=Buffer[y*280/2+x/2];

  if ((x&1))
    return ((ch&0xf0)>>4);
  else
    return (ch&0x0f);
}

static void SetPixel(int x, int y, int Color)
{
  unsigned char         *p = &Buffer[y * 280 /2 + x / 2];
  static const char     Inverse[NUMBER(ColorNames)] =
                                {0, 2, 1, 0, 0, 8, 7, 6, 5, 12, 11, 10, 9};

  if ( (x<CurrentViewPort.xMin) || (x>CurrentViewPort.xMax) ||
       (y<CurrentViewPort.yMin) || (y>CurrentViewPort.yMax) )
    return;

  if (Color == PENCOLOR_NONE)
    return;
  else if (Color == PENCOLOR_REVERSE)
    Color=Inverse[GetPixel(x,y)];

  if ((x&1))
    *p=(*p&0x0f)|(Color<<4);
  else
    *p=(*p&0xf0)|Color;
  UpdatePixel(x, y);
}

static void Line(int len, int ticks, int *LenVec, int *TickVec)
{
  int   i=len/2;
  int   j;

  for (j=0;j<len; j++)
    {
      CurrentTurtleX+=LenVec[0];
      CurrentTurtleY+=LenVec[1];
      i+=ticks;
      if (i>=len)
        {
          i-=len;
          CurrentTurtleX+=TickVec[0];
          CurrentTurtleY+=TickVec[1];
        }
      SetPixel(CurrentTurtleX, CurrentTurtleY, CurrentPenColor);
    }
}

static void MoveTo(int x, int y)
{
  static int right[] ={ 1,  0};
  static int left[]  ={-1,  0};
  static int up[]    ={ 0,  1};
  static int down[]  ={ 0, -1};

  if (XtDisplay(w) && (CurrentPenColor!=PENCOLOR_NONE) )
    {
      int dx=x-CurrentTurtleX;
      int dy=y-CurrentTurtleY;
      if ( (dx>0) && (abs(dy)<=dx) )
        Line(dx, abs(dy), right, (dy>0)?up:down);
      else if ( (dx<0) && (abs(dy)<=-dx) )
        Line(-dx, abs(dy), left, (dy>0)?up:down);
      else if ( (dy>0) && (abs(dx<=dy)) )
        Line(dy, abs(dx), up, (dx>0)?right:left);
      else
        Line(-dy, abs(dx), down, (dx>0)?right:left);
    }
  CurrentTurtleX=x;
  CurrentTurtleY=y;
}

void TurnTo(int Angle)
{
  CurrentTurtleAng=Angle;
  while (CurrentTurtleAng<0)
    CurrentTurtleAng+=360;
  while (CurrentTurtleAng>=360)
    CurrentTurtleAng-=360;
}

static void FillScreen(int Color)
{
  int   x, y;

  for (y=CurrentViewPort.yMin; y<=CurrentViewPort.yMax; y++)
    for (x=CurrentViewPort.xMin; x<=CurrentViewPort.xMax; x++)
      SetPixel(x, y, Color);
}

static void DrawByte(char Data, int XSkip, int Width,
                     int XScreen, int YScreen, int Mode)
{
  static const int      ModeTable[16][2]={
    {PENCOLOR_BLACK, PENCOLOR_BLACK},   {PENCOLOR_REVERSE, PENCOLOR_BLACK},
    {PENCOLOR_BLACK, PENCOLOR_REVERSE}, {PENCOLOR_REVERSE, PENCOLOR_REVERSE},
    {PENCOLOR_NONE,  PENCOLOR_BLACK},   {PENCOLOR_WHITE,   PENCOLOR_BLACK},
    {PENCOLOR_NONE,  PENCOLOR_REVERSE}, {PENCOLOR_WHITE,   PENCOLOR_REVERSE},
    {PENCOLOR_BLACK, PENCOLOR_NONE},    {PENCOLOR_REVERSE, PENCOLOR_NONE},
    {PENCOLOR_BLACK, PENCOLOR_WHITE},   {PENCOLOR_REVERSE, PENCOLOR_WHITE},
    {PENCOLOR_NONE,  PENCOLOR_NONE},    {PENCOLOR_WHITE,   PENCOLOR_NONE},
    {PENCOLOR_NONE,  PENCOLOR_WHITE},   {PENCOLOR_WHITE,   PENCOLOR_WHITE}   };
  int i;

  for (i=0;i<Width; i++)
    {
      int pixel;
      if ( (i+XSkip) > 7 )
        break;
      pixel = Data & (1 << (i+XSkip) ) ? 1:0;
      SetPixel(XScreen+i, YScreen, ModeTable[Mode][pixel]);
    }
}

static void WChar(unsigned char ch)
{
  int   y;
  static const char     SystemCharset[256*8]={
#include <ucsdpsys_xturtleserver/system.charset.h>
  };

  for (y=0; y<8; y++)
    DrawByte(SystemCharset[ch*8+y], 0, 7,
             CurrentTurtleX, CurrentTurtleY+y, CurrentCharType);
  CurrentTurtleX += 7;
}

static void TurtleExposeHandler(Widget wp, XtPointer Closure,
                                XEvent *Event, Boolean *Continue)
{
  (void)wp;
  (void)Closure;
  (void)Event;
  (void)Continue;
  UpdatePlane(0, 279,0,191);
}

static void InitTurtle(int Width, int Height)
{
  Arg                   args[5];
  Cardinal              n;

  XGCValues             GcValues;
  size_t                i;

  if (!w)
    {
      Buffer = malloc(Width * Height / 2);
      memset(Buffer, PENCOLOR_BLACK*0x11, Width*Height/2);

      n=0;
      XtSetArg(args[n], XtNwidth, 2*Width); n++;
      XtSetArg(args[n], XtNheight, 2*Height); n++;


      w=XtCreateManagedWidget( "turtlegraphics", coreWidgetClass, Top,
                               args, n);

      XtAddEventHandler(w, ExposureMask, TRUE, TurtleExposeHandler, NULL);
      XtRealizeWidget(Top);

      for (i=0; i<NUMBER(ColorNames); i++)
        if (ColorNames[i])
          {
            XColor      color,color1;
            if (!XAllocNamedColor(XtDisplay(w),
                                  DefaultColormap(XtDisplay(w),
                                                  DefaultScreen(XtDisplay(w))),
                                  ColorNames[i], &color, &color1))
              {
                fprintf(stderr,"Unable to allocate color %s\n",
                        ColorNames[i]);
                if (strcmp(ColorNames[i],"black")==0)
                  GcValues.foreground=BlackPixel(XtDisplay(w),
                                                 DefaultScreen(XtDisplay(w)));
                else
                  GcValues.foreground=WhitePixel(XtDisplay(w),
                                                 DefaultScreen(XtDisplay(w)));
              }
            else
              GcValues.foreground = color.pixel;
            ColorGCs[i] =
              XCreateGC( XtDisplay(w), XtWindow(w), GCForeground , &GcValues );
          }
        else
      ColorGCs[i]=0;

    }
#ifdef XXX
  {
  XSizeHints the_size_hints;
  long win_attr_mask;
  XSetWindowAttributes set_win_attr;
  int screen;

  screen = DefaultScreen( XtDisplay(w) );

      /* set windows attributes */
      set_win_attr.border_pixel = WhitePixel( XtDisplay(w), screen );
      set_win_attr.background_pixel = BlackPixel( XtDisplay(w), screen );
      set_win_attr.override_redirect = 0;
      set_win_attr.backing_store = WhenMapped;
      win_attr_mask = CWBorderPixel | CWBackPixel |
        CWOverrideRedirect | CWBackingStore;

      XtWindow(w) = XCreateXtWindow(W)( XtDisplay(w),
                              RootXtWindow(W)( XtDisplay(w), screen),
                              0, 0, Width*2, Height*2,
                              0, DefaultDepth( XtDisplay(w), screen ),
                              InputOutput,
                              DefaultVisual( XtDisplay(w), screen ),
                              win_attr_mask,
                              &set_win_attr );

      /*
       * Set size hints & Properties
       */
      the_size_hints.width = Width*2;
      the_size_hints.height = Height*2;
      the_size_hints.max_width = Width*2;
      the_size_hints.max_height = Height*2;
      the_size_hints.min_width = Width*2;
      the_size_hints.min_height = Height*2;
      the_size_hints.flags = PSize | PMaxSize | PMinSize;
      XSetStandardProperties( XtDisplay(w), XtWindow(w),
                              "Turtlegraphics", "Turtlegraphics",
                              None, 0, 0,  &the_size_hints );

      /*
       * SelectInput(XtDisplay(w), window, ExposureMask | StructureNotifyMask);
       */
    }
#endif

  CurrentViewPort.xMin=0;
  CurrentViewPort.xMax=Width-1;
  CurrentViewPort.yMin=0;
  CurrentViewPort.yMax=Height-1;
  FillScreen(PENCOLOR_BLACK);
  CurrentTurtleX=CurrentViewPort.xMax/2;
  CurrentTurtleY=CurrentViewPort.yMax/2;
  CurrentTurtleAng=0;
  CurrentCharType=10;
  CurrentPenColor=PENCOLOR_NONE;
}

static char *BufEnd=NULL;
static char *RdPtr=NULL;

int tgetchar(void)
{
  static char buffer[4096];
  if (RdPtr >= BufEnd)
    {
      int Len=read(0, buffer, sizeof(buffer));
      if ( Len < 1 )
        return(EOF);
      // FIXME: what about 0 (remote end closed connection)?
      BufEnd = buffer + Len;
      RdPtr = buffer;
    }
  return *RdPtr++;
}

void TurtleCommandHandler(XtPointer Closure, int *Source, XtInputId *Id)
{
  int   i;
  int   ch;
  char  buffer[80];
  char  *p;

  (void)Closure;
  (void)Source;
  (void)Id;
  do
    {
      for (i=0,p=buffer; i<79; i++,p++)
        {
          ch=tgetchar();
          if (ch == EOF)
            exit(0);
          if (ch == '\n')
            break;
          *p=ch;
        }
      *p='\0';

      p = strchr(buffer, ' ');
      if (p)
        *p++='\0';

      if (strcmp(buffer, "INITTURTLE")==0)
        InitTurtle(280,192);
      else if (strcmp(buffer, "TURN")==0)
        {
          if (sscanf( p, "%d", &i)==1)
            TurnTo(CurrentTurtleAng+i);
        }
      else if (strcmp(buffer, "TURNTO")==0)
        {
          if (sscanf( p, "%d", &i)==1)
            TurnTo(i);
        }
      else if (strcmp(buffer, "MOVE")==0)
        {
          if (sscanf( p, "%d", &i)==1)
            MoveTo(CurrentTurtleX+round(cos(CurrentTurtleAng*3.14/180)*i),
                   CurrentTurtleY+round(sin(CurrentTurtleAng*3.14/180)*i));
        }
      else if (strcmp(buffer, "MOVETO")==0)
        {
          int y;
          int x;
          if (sscanf( p, "%d %d", &x, &y)==2)
            MoveTo(x, y);
        }
      else if (strcmp(buffer, "PENCOLOR")==0)
        {
          if (sscanf( p, "%d", &i)==1)
            CurrentPenColor=i;
        }
      else if (strcmp(buffer, "TEXTMODE")==0)
        {
          if (w)
            XtMapWidget(w);
        }
      else if (strcmp(buffer, "GRAFMODE")==0)
        {
          if (w)
            XtUnmapWidget(w);
        }
      else if (strcmp(buffer, "FILLSCREEN")==0)
        {
          if (sscanf( p, "%d", &i)==1)
            if (XtDisplay(w))
              FillScreen(i);
        }
      else if (strcmp(buffer, "VIEWPORT")==0)
        {
          int yMax;
          int yMin;
          int xMax;
          int xMin;
          if (sscanf(p, "%d %d %d %d", &xMin, &xMax, &yMin, &yMax)==4)
            {
              CurrentViewPort.yMax=yMax;
              CurrentViewPort.yMin=yMin;
              CurrentViewPort.xMax=xMax;
              CurrentViewPort.xMin=xMin;
            }
        }
      else if (strcmp(buffer, "TURTLEX")==0)
        {
          printf("%d\n",CurrentTurtleX);
          fflush(stdout);
        }
      else if (strcmp(buffer, "TURTLEY")==0)
        {
          printf("%d\n",CurrentTurtleY);
          fflush(stdout);
        }
      else if (strcmp(buffer, "TURTLEANG")==0)
        {
          printf("%d\n",CurrentTurtleAng);
          fflush(stdout);
        }
      else if (strcmp(buffer, "SCREENBIT")==0)
        {
          int y;
          int x;
          if (sscanf(p, "%d %d",&x, &y)==2)
          {
            printf("%c\n",
              XtDisplay(w) && (GetPixel(x,y)!=PENCOLOR_BLACK) ? '1' : '0');
           }
           fflush(stdout);
        }
      else if (strcmp(buffer, "DRAWBLOCK")==0)
        {
          int x;
          int y;
          int Mode;
          int YScreen;
          int XScreen;
          int Height;
          int Width;
          int YSkip;
          int XSkip;
          int RowSize;

          if (sscanf( p, "%d %d %d %d %d %d %d %d",
                      &RowSize, &XSkip, &YSkip,
                      &Width, &Height, &XScreen, &YScreen, &Mode)==8)
            {
              for (y=0; y<Height; y++)
                {
                  int  Skip=XSkip&7;
                  for (x=0;x<Width;)
                    {
                      int  W=Width-x;
                      char Data=tgetchar();

                      if ( W > 8-Skip)
                        W = 8-Skip;

                      if (XtDisplay(w))
                        DrawByte(Data, Skip, W, XScreen+x, YScreen+y, Mode);
                      x+=W;
                      Skip=0;
                    }
                }
            }
        }
      else if (strcmp(buffer, "WCHAR")==0)
        {
          if (sscanf(p, "%d",&i)==1)
            if (XtDisplay(w))
              WChar(i&0xff);
        }
      else if (strcmp(buffer, "WSTRING")==0)
        {
          int  Len;
          int  j;
          if (sscanf(p, "%d",&Len)==1)
            if (XtDisplay(w))
              for (j = 0; j < Len; j++)
                WChar(tgetchar());
        }
      else if (strcmp(buffer, "CHARTYPE")==0)
        {
          if (sscanf(p, "%d", &i)==1)
            CurrentCharType=i&0x0f;
        }
    }  while (RdPtr < BufEnd);
}

static const struct option options[] =
{
    { "version", 0, 0, 'V' },
    { 0, 0, 0, 0 }
};

static void
usage(void)
{
    const char *prog;

    prog = progname_get();
    fprintf(stderr, "Usage: %s\n", prog);
    fprintf(stderr, "       %s --version\n", prog);
    exit(1);
}

int
main(int argc, char **argv)
{
  progname_set(argv[0]);
  for (;;)
  {
    int             c;

    c = getopt_long(argc, argv, "V", options, 0);
    if (c < 0)
      break;
    switch (c)
    {
    case 'V':
      version_print();
      return 0;

    default:
      usage();
    }
  }
  if (optind < argc)
    usage();

  Top=XtAppInitialize(&App, "XTurtlegraphicsServer", NULL, 0,
                      &argc, argv, NULL, NULL, 0);
  XtAppAddInput(App, fileno(stdin), (XtPointer)XtInputReadMask,
                TurtleCommandHandler, NULL);
  XtAppMainLoop(App);
  return 0;
}
