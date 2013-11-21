#include "graphint.h"      
#include "hash.h"  


#include "gmextern.h"

#include "offsetmn.h"
#include "polycom.h"  
#include "offdefs.h"
#include "polybt.h"

  extern lpDPoint     lpDP;
  extern lpDPoint     lpDPBase;
  extern FILE         *dbfile; 
  extern lpLine       lpL;  
  extern unsigned int ldatot;
  extern lpLine       lpLineBase; 
  extern lpArea       lpAreaBase; 
  extern lpLinks    lpLk;
  extern lpWS       lpWork;
  extern lpTA       lpNew;
  extern lpTA       lpNewBase;
  extern lpNA       lpNodeArr;
  extern lpAS       lpAllSBase;
  extern lpAS       lpAllS;
  extern lpNA       lpNodeArrBase;
  extern lpWS       lpWorkBase;
  extern lpLine     lpLineBase;
  extern lpPolyCom1 lpPoly1Base;
  extern lpPolyCom2 lpPoly2Base;
  extern lpPolyCom1 lpPoly1;
  extern lpPolyCom2 lpPoly2;
  extern lpGenInfo  lpGInfo;
  extern lpGenInfo  lpGInfoBase;

char space[52]; 
static  short far *srnum      = (short far *)  space;
static  double far *d2b       = (double far *) &space[2];
static  short far *node_num   = (short far *)  &space[10];
static  short far *ldaloc     = (short far *)  &space[12];
static  double far *sx        = (double far *) &space[12];
static  double far *sy        = (double far *) &space[20];
static  short far *occ        = (short far *)  &space[28];
static  short far *numem      = (short far *)  &space[30];
static  double far *az8       = (double far *) &space[32];
static  short far *the_way    = (short far *)  &space[40];
static  short far *the_rec    = (short far *)  &space[42];
static  short far *this_node  = (short far *)  &space[44];
static  long far *x_y_ary_loc = (long far *)   &space[46];
static	double	CTOL=0.002;

//************************************************************************
void Get3PtsCurve(lpWS L1,LPDOUBLE pc, LPDOUBLE poc, LPDOUBLE pt) 
{
      pc[0] = L1->ldax1;
      pc[1] = L1->lday1;
      PCURVE (&pc[0],&pc[1],&poc[0],&poc[1],&pt[0], &pt[1],
                  &L1->ldax2,&L1->lday2,&L1->ldalngth);
      return;
 }
//************************************************************************

int OffsetAlignment(lpArea lpA)
{
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       this routine unloads ltable into ldatable for the $locate    *
//c*       when a user requests an alignment offset. Offset_these.ftn   *
//c*       already created the offset to one side of the alignment,     *
//c*       this routine offsets the same original lines in the          *
//c*       other direction.  Note][ The order of these offset must       *
//c*       by reversed and the offset lines must now go against the     *
//c*       grain of the original line it is offsetting.  This is        *
//c*       necessitated by the follow on routines which use line order  *
//c*       and direction when intersecting the offset lines.            *
//c*                                                                    *
//c*                                                                    *
//c*                  |<- original line                                 *
//c*       |          |           |                                     *
//c*       | offset   |  offset   |                                     *
//c*       |   < 0    |    > 0    |                                     *
//c*       |          |           |                                     *
//c*                                                                    *
//c*       argument description                                         *
//c*       -------- -----------                                         *
//c*                                                                    *
//c*       radical i*2 (input)  The symbol dictionary description       *
//c*                               number used for collapsed curves     *
//c*       whos_en I*2      (input)  0 = rcdms, 1 = igs             *
//c*                                                                    *
//c*       aend        L*4      (input) if true, will create rounded    *
//c*                            ended for alignments, if false... square*
//c*                                                                    *
//c*       author][  Larry Anderson  Aug 89                              *
//c*                                                                    *
//c**********************************************************************
//c* 
//#include "cnstnt.h"
         long    i, j, m, irc;
         double  oldrad,  r8len,   roff, azm[4], zx1[3], zy1[3], gap; 
         double  *lpX[3], *lpY[3];
         unsigned short isave;
         short  num_ends_got, type = 3, fd=0, k ;
         BOOL aend=FALSE, one_side=FALSE ;
         lpDLine  lpL; 
         lpWS lpLLast,lpLNext, lpWork, lpLFillet=0;
         DPOINT dp, odp; 
         
          for(i=0;i<3;i++)
          {lpX[i] = &zx1[i];
           lpY[i] = &zy1[i];}


       num_ends_got = 0 ; //{used to determine need for end lines
       lpGInfo->min_x = 9e9;
       i = lpPoly1->num_sides + 1;
       roff = lpA->offset_dist;

       for( k = lpA->NumSides; k >=0;k--)
        {  //{for each item in ltable
//c*      I'm going to assume no one will create an alignment
//c*      using link lines.  LDA Mar 94  
         lpL = lpA->lpDLineBase + k;
         if(k > 0)lpLLast = lpWorkBase + (k-1);
         lpLNext = lpWorkBase + (k+1);
         lpWork  = lpWorkBase + i;
         lpPoly2 = lpPoly2Base + k;

        if(lpL->Desc == lpPoly1->link_desc) goto S100 ; //{skip link lines
         lpWork->valid        = 0;
         lpWork->type         = lpL->Type + 9;
         lpWork->ldarec       = (short) i;
         lpWork->ldaref       = k ; //{holds which line was used to create it
         lpWork->ldadesc      = lpL->Desc;
         if(lpL->Type ==2)
         {  //{a straight line
//c*          {Here I calc the azimuth of the line as it moves
//c*          {from the 'to' end to the 'from' end.
            lpWork->ldaazm =LGETAZ(lpL->T.x, lpL->T.y,lpL->F.x, lpL->F.y);
//c*          {Here I offset the 'to' end of the line and store the
//c*          {resulting coords in the 'from' end arrays.
            odp.x = lpL->T.x;
            odp.y = lpL->T.y;
            azm[0] = LTWOPI(lpWork->ldaazm - HALFPI);
            dp = dnewpt(odp, azm[0],roff);
            lpWork->ldax1 = dp.x;
            lpWork->lday1 = dp.y;  
//c*          {Here I offset the 'from' end of the line and store the
//c*          {resulting coords in the 'to' end arrays.
            odp.x = lpL->F.x;
            odp.y = lpL->F.y;
            azm[0] = LTWOPI(lpWork->ldaazm - HALFPI);
            dp = dnewpt(odp,azm[0],roff);
            lpWork->ldax2 = dp.x;
            lpWork->lday2 = dp.y;
            lpWork->ldalngth = LDIST(lpWork->ldax1, lpWork->lday1,
                                     lpWork->ldax2, lpWork->lday2);
            if(lpWork->ldalngth < lpPoly1->tol_fac)goto S100;
            lpWork->ldarad  = 0; 
            
            
            lpGInfo->min_x = __min(lpGInfo->min_x,__min(lpWork->ldax1,lpWork->ldax2));
            lpGInfo->max_x = __max(lpGInfo->max_x,__max(lpWork->ldax1,lpWork->ldax2));
            lpGInfo->min_y = __min(lpGInfo->min_y,__min(lpWork->lday1,lpWork->lday2));
            lpGInfo->max_y = __max(lpGInfo->max_y,__max(lpWork->lday1,lpWork->lday2));

//d           print*,' '
//d           print*,' line ',k,' offset description'
//d           print*,' Stored in array position ',i
//d           print*,' from x = ',lpWork->ldax1[i],' from y = ',lpWork->lday1 [i],
//d           print*,' to x = ',lpWork->ldax2[i],' to y = ',lpWork->lday2[i]
//d           print*,' azm = ',ldaazm[i],' len = ',lpwork->ldalngth [i]

        }
        else 
        { 
          if(lpL->Type ==3)
          {  //{a curve
//c*          {here I get the length of the radius
//c           {next I calc the azimuth of the line from the rad to the PC
            oldrad = LDIST(lpL->F.x, lpL->F.y, lpL->T.x, lpL->T.y);
            lpWork->ldarad = oldrad + (roff * DSIGN(1e0, lpL->lngth));

//d           print*, ' '
//d           print*,'The original curve radius = ',oldrad
//d           print*,'The offset radius = ',lpWork->ldarad[i]
            if(lpWork->ldarad < 0)
            {
//c*             {the curve collapsed into a point on a line from
//c*             {the original curves PC through the RAD, the offset distance
//c*             {from the PC.  The point is given PY + azimuth of the
//c*             {original curve at its PC.
               if(fabs(lpL->lngth) < lpPoly1->tol_fac)goto S100;
               lpWork->ldarad   = fabs(roff) ; //{** set it to the offset **}
//c              print*,'The new offset radius = ',lpWork->ldarad[i]  

               r8len = lpL->lngth;
               lpWork->ldaazm = LGETAZ(lpL->F.x,lpL->F.y,
                                              lpL->T.x,lpL->T.y);
               lpWork->ldalngth = PY*DSIGN(lpWork->ldarad,0e0-lpL->lngth);

//c       print*,'azm from old pc to new start PC = ',ldaazm[i]
//c*             {here I calculate the coords of a new PC point
                odp.x = lpL->F.x;
                odp.y = lpL->F.y;
                dp = dnewpt(dp, lpWork->ldaazm,fabs(roff));
                lpWork->ldax1 = dp.x;
                lpWork->lday1 = dp.y; 
//c* was this    lpwork->ldalngth(i) = PY * dsign(lpWork->ldarad(i),0E0-lpL->lngth(k))
               lpWork->ldalngth = (PY * lpWork->ldarad) ;
//c*                          {** it becomes a half circle
//c       print*,'New Length becomes ',lpwork->ldalngth(i)
               lpWork->ldax2  = lpL->F.x ; //{the old PC x coord (now the new rad x
               lpWork->lday2  = lpL->F.y ; //{the old PC y coord (now the new rad y
               lpWork->ldadesc  = radical;
//c       print*,'New RadX = ', lpWork->ldax2(i),' New RadY = ',lpWork->lday2(i) 
//c       print*,' ' 

            } 
            else 
            {  //{I redefine a reverse curve
               r8len = lpL->lngth; //{original curve length
//d              print*,'original curve length = ',r8len
               lpWork->ldax2 = lpL->T.x; //{old rad x is the same
               lpWork->lday2 = lpL->T.y; //{old rad y is the same

//c*             {Here I calculate the PT of the existing curve
//c*             {placing the coordinates in lpWork->ldax1(i) and lpWork->lday1 (i)
               gap = fabs(r8len);
               LOL8(&lpL->F.x, &lpL->F.y,
                    &lpWork->ldax2, &lpWork->lday2, &r8len,
                    &gap, &lpWork->ldax1, &lpWork->lday1, lpL->Type);

                //{azm of line from old RAD to NEW PC (old PT)
               lpWork->ldaazm = LGETAZ(lpWork->ldax2,lpWork->lday2,
                                       lpWork->ldax1,lpWork->lday1);

//c*             {here I calculate the coords of a new PC point
               odp.x = lpWork->ldax2;
               odp.y = lpWork->lday2;
               dp = dnewpt(odp,lpWork->ldaazm,lpWork->ldarad);
               lpWork->ldax1 = dp.x;
               lpWork->lday1 = dp.y;
                          

               if(oldrad != 0) // ; //{I reverse the direction
                  lpWork->ldalngth = -1e0 * lpL->lngth * (lpWork->ldarad/oldrad);
               else 
                  lpWork->ldalngth = -1e0 * lpL->lngth;
            }  //
            lpGInfo->min_x = __min(lpGInfo->min_x,__min(lpWork->ldax1,lpWork->ldax2));
            lpGInfo->max_x = __max(lpGInfo->max_x,__max(lpWork->ldax1,lpWork->ldax2));
            lpGInfo->min_y = __min(lpGInfo->min_y,__min(lpWork->lday1,lpWork->lday2));
            lpGInfo->max_y = __max(lpGInfo->max_y,__max(lpWork->lday1,lpWork->lday2));

//d           print*,' '
//d           print*,' curve ',i,' offset description'
//d           print*,' PC x = ',lpWork->ldax1(i),' PC y = ',lpWork->lday1 (i),
//d           print*,' RAD x = ',lpWork->ldax2(i),' RAD y = ',lpWork->lday2(i)
//d           print*,' RAD to PC azm = ',lpWork->ldaazm(i),
//d    +             ' length = ',lpwork->ldalngth(i)  

         } 
         else 
         {
            return -1  ; //{inlpWork->valid record type in the midst of the mess
         }   


         if(k == lpA->NumSides && !aend)
         {   
//c*           {user wants square ends of an alignment
             lpLNext->ldalngth  = lpWork->ldalngth;
             lpLNext->ldax2     = lpWork->ldax2;
             lpLNext->lday2     = lpWork->lday2;
             lpLNext->ldax1     = lpWork->ldax1;
             lpLNext->lday1     = lpWork->lday1;
             lpLNext->ldaazm    = lpWork->ldaazm;
             lpLNext->ldarad    = lpWork->ldarad;
             lpLNext->ldadesc   = lpWork->ldadesc;
             lpLNext->type      = lpWork->type;
             lpLNext->ldarec    = lpWork->ldarec + 1 ;
             lpLNext->valid     = 0;
             m = i-1;
             j = i+1;
             if(lpLLast->type == 12)
             { //{it's a curve 
//c*              I calculate it's endpoint   
//c*               {Here I calculate the PT of the existing curve
//c*               {placing the coordinates in lpWork->ldax1(i) and lpWork->lday1 (i)
                 gap = fabs(lpLLast->ldalngth);
                 LOL8(&lpLLast->ldax1, &lpLLast->lday1,
                    &lpLLast->ldax2, &lpLLast->lday2, &lpLLast->ldalngth,
                    &gap, &lpWork->ldax1, &lpWork->lday1, lpWork->type);
             } 
             else 
             {  //{it's a straight line
                    lpWork->ldax1 = lpLLast->ldax2;
                    lpWork->lday1 = lpLLast->lday2;
             }  //  
             lpWork->ldax2 = lpLNext->ldax1;
             lpWork->lday2 = lpLNext->lday1;
             
             lpWork->ldaazm = LGETAZ(lpWork->ldax1,lpWork->lday1,
                                     lpWork->ldax2,lpWork->lday2); 
                                 
             lpWork->ldalngth = LDIST(lpWork->ldax1,lpWork->lday1,
                                      lpWork->ldax2,lpWork->lday2) ;
             lpWork->type     = 11 ; //{a straight line
             lpWork->ldarec   = (short)i;
             lpWork->ldadesc  = 251;
             i++;
        }  
        else 
        { 
        if (i >  1)
        { // 
           offset_intx_these_two(lpLLast,lpWork, lpX, lpY, &gap, &irc) ;
           if(lpWork->ldadesc == radical)
           { //
              gap = LDIST(lpLLast->ldax1,lpLLast->lday1,zx1[1],zy1[1]);
              if(gap <  P_TOL *2 )irc = 0 ; //{radical hit the start of i-1          
           }  //
           if(irc >  0 && irc <  3)
           { // ; //{got a hit 
//c*            I recalculate the line descriptions 

//c*            for line i-1 I recalculate it's endpoint
              if(lpLLast->type == 11)
              { //
                lpLLast->ldax2 = zx1[1];
                lpLLast->lday2 = zy1[1];
                lpLLast->ldalngth = LDIST(lpLLast->ldax1, lpLLast->lday1,
                                          lpLLast->ldax2, lpLLast->lday2);
                if(lpWork->ldalngth == 0)goto S98 ; //{just created a zero length line
              } 
              else 
              {
                azm[1] = LGETAZ(lpLLast->ldax2,lpLLast->lday2,zx1[1],zy1[1]);
                azm[2] = AZDF(lpLLast->ldaazm,azm[1],lpLLast->ldalngth) ;
                if(fabs(azm[2]) >  TWOPI-1e-6 &&
                   fabs(lpLLast->ldalngth) >  1e-2)azm[2]= 0e0;
                lpPoly1->sign = DSIGN(1e0,lpLLast->ldalngth);
                lpLLast->ldalngth =  azm[2]*lpLLast->ldarad*lpPoly1->sign;
                if(lpLLast->ldalngth == 0)goto S98 ; //{just created a zero length line
              }  // 
//c*            for line I I recalculate it's starting position
              if(lpWork->type  == 11)
              { // ; //{it's a straight line
                lpWork->ldax1 = zx1[1];
                lpWork->lday1 = zy1[1];
                lpWork->ldalngth = LDIST(lpWork->ldax1, lpWork->lday1,
                                         lpWork->ldax2, lpWork->lday2);
              } 
              else 
              {
                azm[1] = LGETAZ(lpWork->ldax2,lpWork->lday2,zx1[1],zy1[1] );
                azm[2] = AZDF(lpWork->ldaazm,azm[1],lpWork->ldalngth) ;
                lpPoly1->sign = DSIGN(1e0,lpWork->ldalngth);
                if(fabs(azm[2]) >  TWOPI-1e-6 &&
                   fabs(lpWork->ldalngth) >  1e-2)azm[2]= 0e0;
                lpWork->ldalngth = lpWork->ldalngth - (azm[2]*lpWork->ldarad*lpPoly1->sign);
                lpWork->ldax1    = zx1[1];
                lpWork->lday1    = zy1[1];
                lpWork->ldaazm   = azm[1];
              }  // 
          }
          else
          {  
            if(lpLLast->ldadesc != radical)
            {  //{ attempt a fillet
              irc = OffsetCreateFillet
                    (lpLLast, lpWork,lpLFillet, lpA->whos_callen,lpA->fillet_desc, one_side);
               if(irc == 1)
               { 
//c*                gotta a hit with a fillet
//c*                I calculate the endpoint of line i-2
//c*                and the beginning of i-1
//c*                the fillet is now stored in the i'th element of the lda arrays
//c*                I move the line infor from I to I+1 
                  lpLNext->ldax1    = lpWork->ldax1;
                  lpLNext->lday1    = lpWork->lday1;
                  lpLNext->ldax2    = lpWork->ldax2;
                  lpLNext->lday2    = lpWork->lday2;
                  lpLNext->ldaazm   = lpWork->ldaazm;
                  lpLNext->ldarad   = lpWork->ldarad;
                  lpLNext->ldalngth = lpWork->ldalngth;
                  lpLNext->type     = lpWork->type; //{a curved line
                  lpLNext->ldadesc  = lpWork->ldadesc;
                  lpLNext->ldarec   = (short) (i+1) ;
                  lpLNext->valid    = 0  ;
                  lpLNext->ldaref   = lpWork->ldaref;
//c*                and I transfer the fillet record into position I
                  lpWork->ldax1    = lpLFillet->ldax1;
                  lpWork->lday1    = lpLFillet->lday1;
                  lpWork->ldax2    = lpWork->ldax2;
                  lpWork->lday2    = lpLFillet->lday2;
                  lpWork->ldaazm   = lpLFillet->ldaazm;
                  lpWork->ldarad   = lpLFillet->ldarad;
                  lpWork->ldalngth = lpLFillet->ldalngth;
                  lpWork->type     = lpLFillet->type; //{a curved line
                  lpWork->ldadesc  = 250;
                  lpWork->ldarec   = (short) i;
                  lpWork->ldaref   = lpWork->ldaref;
                  lpWork->valid    = 0;
                  i = i + 1;
               }  //  
           } 
           else 
           {    
//c                 I eliminate it from the line array
S98:              lpLLast->ldax1     = lpWork->ldax1;
                  lpLLast->lday1     = lpWork->lday1;
                  lpLLast->ldax2     = lpWork->ldax2;
                  lpLLast->lday2     = lpWork->lday2;
                  lpLLast->ldaazm    = lpWork->ldaazm;
                  lpLLast->ldarad    = lpWork->ldarad;
                  lpLLast->ldalngth  = lpWork->ldalngth;
                  lpLLast->type      = lpWork->type; //{a curved line
                  lpLLast->ldadesc   = lpWork->ldadesc;
                  lpLLast->ldarec    = (short)(i-1);
                  lpLLast->valid     = 0;
                  lpLLast->ldaref    = lpWork->ldaref;
                  i--;
            }  // 
        }   //{(i >  1){ // 

        i++;
S100:     continue ;
}

 
        if(!aend)
        { //
//cc*            {user wants square ends of an alignment
           lpLLast = lpWorkBase + (i-1);
           lpWork = lpWorkBase + i;
           lpLNext = lpWorkBase;
         //  m = i-1;
         //  j = 1;
           if(lpLLast->type == 3)
           { //{it's a curve 
//c*            I calculate it's endpoint   
//c*             {Here I calculate the PT of the existing curve
//c*             {placing the coordinates in lpWork->ldax1(m+1) and lpWork->lday1 (m+1)
               gap = fabs(lpLLast->ldalngth); 
               LOL8(&lpLLast->ldax1, &lpLLast->lday1,
                    &lpLLast->ldax2, &lpLLast->lday2, &lpLLast->ldalngth,
                    &gap, &lpWork->ldax1, &lpWork->lday1, 
                    lpLLast->type);
           } 
           else 
           { //{it's a straight line
                    lpWork->ldax1 = lpLLast->ldax2;
                    lpWork->lday1 = lpLLast->lday2;
           }  //  
           lpWork->ldax2    = lpLNext->ldax1;
           lpWork->lday2    = lpLNext->lday1;
           lpWork->ldaazm   = LGETAZ(lpWork->ldax1,lpWork->lday1,
                                     lpWork->ldax2,lpWork->lday2);
           lpWork->ldalngth = LDIST(lpWork->ldax1,lpWork->lday1,
                                    lpWork->ldax2,lpWork->lday2);
           lpWork->type     = 2 ; //{a straight line
           lpWork->ldarec   = (short)(m+1);
           lpWork->ldadesc  = 251;
           lpWork->valid    = 0;
           i++;
//cccc           endif {if(lpwork->ldadesc(i) == radical)
        } 
        else 
        { //{user wants rounded ends... I put one in  
            lpWork = lpWorkBase;
            lpLLast = lpWorkBase + (i-1);
            if(lpWork->ldadesc == radical)
               irc = 0;
            else 
              offset_intx_these_two(lpLLast,lpWork,lpX,lpY, &gap, &irc) ; 
               
            if(irc >  0 && irc <  3)
            { // ; //{got a hit 
//c*            I recalculate the line descriptions 
//c*            for line I I recalculate it's starting position
              if(lpWork->type == 11)
              { // ; //{it's a straight line
                lpWork->ldax1    = zx1[1];
                lpWork->lday1    = zy1[1];
                lpWork->ldalngth = LDIST(lpWork->ldax1, lpWork->lday1,
                                         lpWork->ldax2, lpWork->lday2);
              } 
              else 
              {
                azm[1] = LGETAZ(lpWork->ldax2,lpWork->lday2,zx1[1],zy1[1]);
                azm[2] = AZDF(lpWork->ldaazm,azm[1],lpWork->ldalngth) ;
                lpPoly1->sign = DSIGN(1e0,lpWork->ldalngth);
                if(fabs(azm[2]) >  TWOPI-1e-6 &&
                   fabs(lpWork->ldalngth) >  1e-2)azm[2]= 0e0;
                lpWork->ldalngth  = lpWork->ldalngth - (azm[2]*lpWork->ldarad*lpPoly1->sign);
                lpWork->ldax1     = zx1[1];
                lpWork->lday1     = zy1[1];
                lpWork->ldaazm    = azm[1];
              }  // 
//c*            Now I end line I-1 at the same point
              if(lpLLast->type == 11)
              { 
                lpLLast->ldax2    = zx1[1];
                lpLLast->lday2    = zy1[1];
                lpLLast->ldalngth = LDIST(lpLLast->ldax1, lpLLast->lday1,
                                          lpLLast->ldax2, lpLLast->lday2);
              } 
              else 
              {
                azm[1] = LGETAZ(lpLLast->ldax2,lpLLast->lday2,lpWork->ldax1,lpWork->lday1);
                azm[2] = AZDF(lpLLast->ldaazm,azm[1],lpLLast->ldalngth) ;
                if(fabs(azm[2] >  TWOPI-1e-6 &&
                   fabs(lpLLast->ldalngth)) >  1e-2)azm[2]= 0e0;
                lpPoly1->sign = DSIGN(1e0,lpLLast->ldalngth);
                lpLLast->ldalngth =  azm[2]*lpLLast->ldarad * lpPoly1->sign;

              }  // 
           } 
           else 
           {   //{ attempt a fillet
               lpPoly1->num_sides = i - 1  ;
               lpWork = lpWorkBase + (i-1);
               for( j = 0; j <= i-2 ;j++)
               {
                   lpLNext = lpWorkBase + j;
                   isave   = lpLNext->ldadesc;
                   lpLNext->ldadesc = radical;
                   irc = OffsetCreateFillet(lpWork, lpLNext,lpLFillet, 
                   lpA->whos_callen, lpA->fillet_desc, one_side);
                   lpLNext->ldadesc = isave;
                   if(irc == 1)
                   {  
                     lpLFillet->ldadesc  = 250;
                     lpLFillet->ldarec   = (short) lpPoly1->num_sides;
                     lpLFillet->valid    = 0;
                     i++;
                     goto S1000;
                   } 
               }//end of the for loop
           }   
        
        }            
 
S1000:  lpPoly1->num_sides = i - 1 ;
       for ( i = 0; i < lpPoly1->num_sides;i++)
       { 
         lpWork = lpWorkBase + i;
         if(lpWork->ldadesc == radical) lpWork->ldadesc = fd;
       }
   
       lpPoly1->num_orig_sides = lpPoly1->num_sides;
       return 0 ;
} }}
	return 0;                                                                         
}
  


int OffsetCreateFillet (lpWS lpFrom,lpWS lpTo,lpWS lpFillet, 
       int whos_callen, int fillet_desc,
       BOOL one_side)
{               
 //c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       This routine, part of the area offset system, creates        *
//c*       a possible fillet in ldatot.  If subsequent tests prove      *
//c*       the offset should be included in the offset area, a call     *
//c*       to offset_add_fillet.ftn places the fillet with the other    *
//c*       lines that make up the sides of the offset area.             *
//c*                                                                    *
//c*       The definition of appropriate includes the following         *
//c*       circumstances:                                               *
//c*       - The difference between the current line azimuth at its     *
//c*         PT and the next lines azimuth at its PC is <= 0            *
//c*       - The distance between the current line PT and the next      *
//c*         line PC is less than 2*radius                              *
//c*                                                                    *
//c*                                                                    *
//c*       A fillet is defined as a curve with a radius = fabs(offset)  *
//c*       its radius point is the endpoint of the current line;        *
//c*       its beginning azimuth is the ending azimuth of the current   *
//c*       line; its length is dsign(pii*radius,offset) initially.      *
//c*       If an actual hit with the next lines occurs with the fillet, *
//c*       the fillet's definition is changed to include only the       *
//c*       portion between the current line and the next line.          *
//c*                                                                    *
//c*                  |<- original line                                 *
//c*       |          |           |                                     *
//c*       | offset   |  offset   |                                     *
//c*       |   < 0    |    > 0    |                                     *
//c*       |          |           |                                     *
//c*                                                                    *
//c*       arguments                                                    *
//c*       ---------                                                    *
//c*       item      I*4   (input) LTABLE array element                 *
//c*       next      I*4   (input) LTABLE array element of second line  *
//c*       fillet_desc I*2 (input) the edsym line description for fillet*
//c*       wc        I*2   (input)  0 = rcdms, 1 = igs {whos_callen     *
//c*                                                                    *    
//c*       irc       I*4   (input) The type of intersection discovered  *
//c*                               by offset_intx_these_two:            *
//c*                               irc = 1 a single hit                 *
//c*                               irc = 2 means double hit             *
//c*       x1[3]     R*8   (input) X coord of the intersection          *
//c*       y1[3]     R*8   (input) Y coord of the intersection          *
//c*                                                                    *
//c*                                                                    *
//c*       Author: Larry Anderson Jan 88                                *
//c*                                                                    *
//c*       Modified Mar 88 to work with smallest change and delta_azm's *
//c*       instead of the change in chord azimuth's.                    *
//c*                                                                    *
//c**********************************************************************
//c*
//#include "cnstnt.h" //umsc/include/cnstnt.ftn};
       DPOINT dp,odp;
       double  pt_azm, pt_dis, delta_azim, sign, azm[3], mygap;
       short i2, myn1, myn2, i;

       double  myX1,myX2, myX3, myX4, myY1, myY2, myY3, myY4;
       
         if(lpFrom->type == 3 )
         { 
//c*          {the current line is a curve ... I calculate its PT
//c*          {which becomes the fillets PC 
            sign = fabs(lpFrom->ldalngth);
            LOL8(&lpFrom->ldax1, &lpFrom->lday1, &lpFrom->ldax2, &lpFrom->lday2,
                 &lpFrom->ldalngth,&sign ,
                 &lpFillet->ldax1, &lpFillet->lday1, (short)lpFrom->type);
//c*          {I calculate the azimuth of the line at its PT
            lpFillet->ldaazm = LGETAZ(lpFrom->ldax2, lpFrom->lday2,
                             lpFillet->ldax1, lpFillet->lday1)
                             - (HALFPI * DSIGN(1e0,lpFrom->ldalngth));
         } 
         else 
         { //its a straight line
            lpFillet->ldaazm = lpFrom->ldaazm;
            lpFillet->ldax1  = lpFrom->ldax2; //line endpoint is now the PC
            lpFillet->lday1  = lpFrom->lday2; //line endpoint is now the PC
         }  

       lpFillet->ldarad      = fabs(lpPoly1->offset);
       lpFillet->type        = 3 ; //a curved line
       lpFillet->ldalngth    =  -1e0 * PY * lpPoly1->offset ;

       if(!one_side && lpFrom->ldarec == lpPoly1->num_orig_sides && lpTo->ldarec == 0)
       {  
//c*       I just hook the end of the last line to the beginning
//c*       of the first line  
         myX1 = lpFillet->ldax1;
         myY1 = lpFillet->lday1; 
         lpPoly1->num_sides++;
         lpFillet = lpWorkBase + lpPoly1->num_sides;
         lpFillet->ldax1    =  myX1;
         lpFillet->lday1    =  myY1;
         lpFillet->ldax2    =  lpTo->ldax1;
         lpFillet->lday2    =  lpTo->lday1;
         lpFillet->ldaazm   =  LGETAZ(lpFillet->ldax2,lpFillet->lday2,
                                    lpFillet->ldax1,lpFillet->lday1);
         lpFillet->ldarad   = 0;
         lpFillet->ldalngth = LDIST(lpFillet->ldax2,lpFillet->lday2,
                                  lpFillet->ldax1,lpFillet->lday1 );

         lpFillet->type     =  2 ; //straight line
         lpFillet->ldadesc  = radical + 1;
         lpFillet->ldarec   = (short)lpPoly1->num_sides ;
         lpFillet->valid    = 0;
         return 1;
       }  // 


//c*     {I calculate the coordinates of the fillet radius 
          odp.x = lpFillet->ldax1;
          odp.y = lpFillet->lday1;
          azm[0] = LTWOPI(lpFillet->ldaazm + DSIGN(HALFPI,lpPoly1->offset));
          dp = dnewpt(odp,azm[0],fabs(lpPoly1->offset)); 
          lpFillet->ldax2 = dp.x;
          lpFillet->lday2 = dp.y;
          lpFillet->ldaazm = LGETAZ(lpFillet->ldax2, lpFillet->lday2,
                                    lpFillet->ldax1, lpFillet->lday1);
          i2 = -99; //makes it check for continuity
          XLC(&lpFillet->ldax1, &lpFillet->lday1, 
              &lpFillet->ldax2, &lpFillet->lday2, 
              &lpFillet->ldalngth,
              &lpTo->ldax1, &lpTo->lday1, 
              &lpTo->ldax2, &lpTo->lday2, 
              &myX1, &myY1, &myX2, &myY2, &myX3, &myY3,   
              &myn1, &myn2, &mygap, &i2);
          switch(i2)
          {
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case -3:
            {  //get azimuth from rad pt to intersection point
                 pt_azm = LGETAZ(lpFillet->ldax2, lpFillet->lday2,
                                 myX3, myY3); 
                                             
               //get the length of the curve in radians                                           
               delta_azim = AZDF(lpFillet->ldaazm,pt_azm,lpFillet->ldalngth);
               //get length of the curve in feet
               sign = DSIGN(1e0,lpFillet->ldalngth);
               lpFillet->ldalngth = delta_azim * fabs(lpPoly1->offset) * sign ; //shorten the fillet
               //adjust the beginning points of lpTo
               lpTo->ldax1 =  myX3;
               lpTo->lday1 = myY3; 
               return 1;
               break;
            }
            default:
            {
               delta_azim = PY + 1;
            }
          }//end of the switch
            
       if(lpTo->ldadesc == radical)
       { 
//c*        the next line is a collapsed curve
//c*        I must intx the fillet with the radical to get the coords
//c*        to reset line k's PC coords and length
         XCC(&lpTo->ldax1, &lpTo->lday1, &lpTo->ldax2, &lpTo->lday2, &lpTo->ldalngth,
             &lpFillet->ldax1, &lpFillet->lday1, 
             &lpFillet->ldax2, &lpFillet->lday2, &lpFillet->ldalngth,
             &myX1, &myY1, &myX2, &myY2, &myX3, &myY3,   
             &myn1, &myn2, &mygap, &i2);
          pt_dis = 0;
          if(i2 <= 2)
          { // ; //lines are continuous 
           switch(myn1)
           {
            case 2:
              myX1 = myX2;
              myY1 = myY2;
              break;
            case 3:  
              myX1 = myX3;
              myY1 = myY3;
              break;
           }   
//c* c*       lpFillet->ldaazm(r) already holds the azimuth to the fillet PC 
//c* c*       calc the azm from the fillet rad to the new ints pt
            azm[2] = LGETAZ(lpFillet->ldax2,lpFillet->lday2,myX1,myY1);
            pt_azm = AZDF(lpFillet->ldaazm,azm[2],lpFillet->ldalngth);
            if(fabs(pt_azm) >  TWOPI-1e-6 &&
                     fabs(lpTo->ldalngth) >  1e-2)pt_azm= 0e0;
//c            print*,'len of fillet to intx with next line '
//c            print*,'azm len = ',pt_azm,' len = ',pt_azm*fabs(roff)
//c*          i shorten the length of the fillet to pt_azm
            sign = DSIGN(1e0,lpFillet->ldalngth);
            lpFillet->ldalngth = pt_azm * fabs(lpPoly1->offset) * sign ; //to shorten it 

//c*          now I adjust the radical's dimensions
//c*          calc the azm from k's rad to it's pc
            azm[1] = LGETAZ(lpTo->ldax2,lpTo->lday2,lpTo->ldax1,lpTo->lday1);
//c*          calc the azm from k's rad to the new ints pt
            azm[2] = LGETAZ(lpTo->ldax2,lpTo->lday2,myX1,myY1);
            pt_azm = AZDF(azm[1],azm[2],lpTo->ldalngth) ;
           if(fabs(pt_azm) >  TWOPI-1e-6 &&
                     fabs(lpTo->ldalngth) >  1e-2)pt_azm= 0e0;
//c*          here I shorten the and length of the radical
            sign = DSIGN(1e0,lpTo->ldalngth);
            lpTo->ldalngth = lpTo->ldalngth - (pt_azm * lpTo->ldarad * sign) ; 
            lpTo->ldax1 = myX1 ; //new pcX for the next line
            lpTo->lday1 = myY1 ; //new pcY for the next line
            lpTo->ldaazm = azm[2];
          } 
          else 
            return -1;
       } 

        return -1  ;
}
  
 void  offset_intx_all_lines(lpArea lpa, BOOL one_side, long *st)
{                       
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       This routine, part of the offset area system, attempts to    *
//c*       intersect a line with the next line. Failing to find an      *
//c*       actual intersect, the routine determines if a fillet can     *
//c*       properly close a gap between the current line and the        *
//c*       next line.                                                   *
//c*                                                                    *
//c*       Originally a copy of poly_intx_all_lines.ftn, this routine   *
//c*       differs in that it makes special efforts to intersect        *
//c*       the current line with the very next line. If no actual       *
//c*       intersection is found, the routine may try a fillet to       *
//c*       close a gap - if appropriate.                                *
//c*       A fillet is defined as a curve with a radius = fabs(offset)  *
//c*       its radius point is the endpoint of the current line;        *
//c*       its beginning azimuth is the ending azimuth of the current   *
//c*       line; its length is dsign(pii*radius,offset)                 *
//c*                                                                    *
//c*       The definition of appropriate includes the following         *
//c*       circumstances:                                               *
//c*       - The difference between the current line azimuth at its     *
//c*         PT and the next lines azimuth at its PC is <= 0          *
//c*       - The distance between the current line PT and the next      *
//c*         line PC is less than 2*radius                              *
//c*                                                                    *
//c*    arguments                                                       *
//c*       wc        I*2   (input)  0 = rcdms, 1 = igs {whos_callen     *
//c*                                                                    *
//c*                                                                    *
//c*       The following steps are taken by, or directed by this        *
//c*       routine:                                                     *
//c*                                                                    *
//c*       A. Node_info                                                 *
//c*          Readies a hash array to contain all the intersection nodes*
//c*          Type: hash   Key:node_id, num_of_lines_sharing_node       *
//c*          Text: occurrence, line_id, x_y coords                     *
//c*          Size: 2048, key: 4, text: 20                              *
//c*                                                                    *
//c*       B. B_tree                                                    *
//c*          Readies a B_tree to contain the coordinates of the        *
//c*          intersection points.                                      *
//c*          Type: B_tree Key: x, y coord, line_id, beg_to_intx_dist   *
//c*          Size: 2048   Text: node_id, node_type, line_azm           *
//c*                                                                    *
//c*       C. Line_info                                                 *
//c*          Readies a hash array to contain the lines that intersect  *
//c*          it and the distance to the intersection point from the    *
//c*          lines origin.                                             *
//c*          Type: hash   Key:line_id,intersect_line_id                *
//c*          Text: occurrences_of_intersection with this line          *
//c*          Size: 2048, key: 8, text: 2                               *
//c*                                                                    *
//c*       Author: Larry Anderson Nov 88                                *
//c*                                                                    *
//c**********************************************************************
//c*
//#include "lparm$.h" //umsc/include/lparm$.ftn};

//#include "umdb_ins.h" //umsc/include/umdb.ins.ftn}; 

      lpWS lpFillet, lpNext;
      double  x1[4], y1[4], gap, roff;
      double far *lpX[4], *lpY[4];
      long    ireturn, irc, i, j ;
      short     nkeyfld = 3,              fillet_desc=0, 
                fldtyp[3] = {62,62,63},   fldlen[3]={8,8,2}, 
                b3fldtyp[2] = {63,62},    b3fldlen[2]={2,8}, 
                        wc=0,         one = 1;

          for(i=0;i<4;i++)
          {lpX[i] = &x1[i];
           lpY[i] = &y1[i];}
   //   equivalence (key(1:),i), (key(5:5),j);
      
//c      data  fldtyp/62,62,63/, fldlen/8,8,2/, nkeyfld/3/;
//c                   ^real ^integer
//      data  b3fldtyp/63,62/, b3fldlen/2,8/;
S1:   lpPoly1->num_orig_sides = lpPoly1->num_sides;
      lpPoly1->num_nodes = 0;
      if(lpPoly1->bid3 != 0)
      {
           PolyBTClose (&lpPoly1->id1);
           PolyBTClose (&lpPoly1->bid3);
      }
       
        lpPoly1->id1  =  PolyBTInit ((short) 38, nkeyfld, fldtyp, fldlen); 
        lpPoly1->bid3 =  PolyBTInit ((short) 14,(short) 2, b3fldtyp, b3fldlen);
      roff = lpPoly1->offset;
      *st = 0;
      offset_find_link_partners(st);
      if(*st != 0)return;
      for (i = 0;i < lpLk->nPartners;i++)
      { // I try to close each sub areas if fillets needed 
        lpWork = lpLk->LBG[i];
        lpNext = lpLk->LACB[i];
        offset_intx_these_two(lpWork, lpNext, lpX, lpY, &gap, &irc) ;
        if(irc <=  0 || irc >=  3)
        {
          lpFillet = lpWorkBase + lpPoly1->num_sides + 1;
          lpFillet->ldarec = (short)lpPoly1->num_sides + 1;
          irc = OffsetCreateFillet
                 (lpWork, lpNext, lpFillet, wc, fillet_desc, one_side);
          if(irc == 1 || irc == 2)lpPoly1->num_sides++;
        }  
      }
       
    for(i=1; i <= lpPoly1->num_sides; i++)
    {
       lpWork = lpWorkBase + i;
       if(lpWork->ldadesc == 0 ||
          lpWork->ldadesc == lpPoly1->link_desc ||
          lpWork->valid != 0) ;  // do nothing
    
       else
       {     
         j = i + 1; 
         
         
      //   if(i == 80 && j == 81)
      //   {
      //      irc  = i;
      //   }
         
         while (j <= lpPoly1->num_sides)  
         {  //intersect all other lines 
            lpNext = lpWorkBase + j;
            if(lpNext->ldadesc == 0 || 
               lpNext->ldadesc == lpPoly1->link_desc ||
               lpNext->valid != 0)goto S19;

            offset_intx_these_two(lpWork, lpNext, lpX, lpY, &gap, &irc) ;

             if(irc == 2)
             {  //2 hits   | ltable locations
               ireturn = offset_check_coincident(lpWork, lpNext);
               if (ireturn != 0)
               {  //found coincident lines
                   irc =   PolyEliminateCoincidence(lpWork, lpNext,lpX,lpY,(int)ireturn);
                   if(irc == 2) goto S19; //skip line J
                   if(irc == 3) goto S1; //start all over
               }  
             }  

            switch (irc)
            { 
              case 2:
                  lpPoly1->st = poly_load_lda_line
                                 (*lpX[1],*lpY[1],lpWork, lpNext,one);
                  lpWork->intxs++;
                  lpNext->intxs++; 
              case 1:
                  lpPoly1->st = poly_load_lda_line
                       (*lpX[0],*lpY[0],lpWork,lpNext,one); 
                  lpWork->intxs++;
                  lpNext->intxs++; 
            }  
S19:        j++;
         } // end of the while loop 
       }  
      }
  return;
}      
            

void offset_intx_these_two(lpWS lpOne,lpWS lpTwo,
               double far **x1,double far **y1,double *gap,long *irc)
{
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       This routine, part of the polygon system, generates the      *
//c*       line segments required by poly to generate the smallest      *
//c*       areas possible within a given geo_segment.                   *
//c*                                                                    *
//c*                                                                    *
//c*       arguments                                                    *
//c*       ---------                                                    *
//c*       i   I*4 (input) the position in ldatable of the first line   *
//c*       j   I*4 (input) the position in ldatable of the second line  *
//c*       x1  R*8(3) (output) the intersection point(s) x coord        *
//c*       y1  R*8(3) (output) the intersection point(s) y coord        *
//c*       gap R*8    (output) the minimum gap between two              *
//c*                           non_intersecting lines                   *
//c*                                                                    *
//c*       Original a copy of poly_intx_these_two.ftn modified to       *
//c*       work with elements of ldatable directly.                     *
//c*                                                                    *
//c*       Author][ Larry Anderson Nov 88                                *
//c*                                                                    *
//c**********************************************************************
//c*
//#include "umdb_ins.h" ///umsc/include/umdb.ins.ftn}

       double dis1, dis2, dis3, dis4, dsmall;
       short i2, n1, n2, nc, nl;
       
       
       *irc = 0;
       if(lpOne->type == 3)goto S20 ; //line 1 is a curve

       if(lpTwo->type == 3)goto S10 ; //line 2 is a curve

//c***    line 1 is straight line 2 is straight
          XLL(&lpOne->ldax1, &lpOne->lday1, &lpOne->ldax2, &lpOne->lday2,
              &lpTwo->ldax1, &lpTwo->lday1, &lpTwo->ldax2, &lpTwo->lday2,
              *x1,
              *y1,    
              *(x1+1),       
              *(y1+1),
              *(x1+2),       
              *(y1+2),      
              &n1,         &n2,       gap,   &i2);
//c*       xll  returncode meanings][  (Note][ xpt = intersection point)
//c*
//c*         No real intersections found
//c*         i2 = 0      2 line parallel *** No hits possible
//c*         i2 = 1      xpt inside of line 1; off line 2
//c*         i2 = 2      xpt inside of line 2; off line 1
//c*
//c*         A real intersection was found][
//c*         i2 = 3      xpt inside of both lines  {true intersection
//c*                     n1 = n2 = 3
//c*
//c*         intersections with the tolerance factor:
//c*         i2 = 4      xpt outside of both line,use two closest ep
//c*         i2 = 7      xpt outside of both line,use xpt point
//c*                     n1 = n2 = 3
//c*
//c*         If the lines meet at their endpoints:
//c*         i2 = 5      congruent at first lines' bp (bx1,by1)
//c*         i2 = 6      congruent at first lines' ep (ex1,ey1)
//c*
//c*         If the lines are parallel and overlap for all or a
//c*         portion of either line.
//c*         i2 = 8      lines overlap one another
//c*
//c*         How to use the nx variables:
//c*         n1=1 is (bx1,by1); =2 is (ex1,ey1); =3 is (x3,y3)
//c*         n2=1 is (bx2,by2); =2 is (ex2,ey2); =3 is (x3,y3)
        switch (i2)
        {
          case 3: //found intersect
           *x1[0] = *x1[2];
           *y1[0] = *y1[2];
           i2 = 1;
           break;
          case 2: //endpoints within p_tol distance
           dis1 = LDIST(lpOne->ldax1,lpOne->lday1,lpTwo->ldax1,lpTwo->lday1);
           dis2 = LDIST(lpOne->ldax1,lpOne->lday1,lpTwo->ldax2,lpTwo->lday2);
           dis3 = LDIST(lpOne->ldax2,lpOne->lday2,lpTwo->ldax1,lpTwo->lday1);
           dis4 = LDIST(lpOne->ldax2,lpOne->lday2,lpTwo->ldax2,lpTwo->lday2);
           dsmall = __min(dis1,__min(dis2,__min(dis3,dis4)));
           *gap = dsmall;
           if(dsmall > P_TOL)
           { 
               i2 = 0;
               break;
           } 
           else 
           { 
            if(dsmall == dis1 || dsmall == dis2)
            {
             *x1[0] = lpOne->ldax1;
             *y1[0] = lpOne->lday1;
            } 
            else 
            {
             *x1[0] = lpOne->ldax2;
             *y1[0] = lpOne->lday2;
            }  
            i2 = 1;
           }
           break;
         case 5: //congruent at line 1 bp 
         case 6:
            *x1[0] = *x1[+n1-1];
            *y1[0] = *y1[n1-1];
            i2 = 1;
            break;
         case 8:  //lines overlap one_another
            *x1[0] = *x1[1]   ; //use the midpoint of the overlap
            *y1[0] = *y1[1]   ; //portion and the intersection point
            i2 = 1; 
            break;
         default:
            i2 = 0;
        }  //end of the switch
//c*
        goto S40;

//c***    line 1 is straight line 2 is curve
S10:     i2 = -99;
         XLC(&lpTwo->ldax1, &lpTwo->lday1, &lpTwo->ldax2, &lpTwo->lday2, &lpTwo->ldalngth,
             &lpOne->ldax1, &lpOne->lday1, &lpOne->ldax2, &lpOne->lday2,  
             *x1,  
             *y1,  
             *(x1+1),  
             *(y1+1),   
             *(x1+2),     
             *(y1+2),
             &nc,    &nl,    gap,    &i2);

//c*                joint intersection codes
//c*                = 0   no intersection
//c*                = 1   1 tangent pt,on line & curve
//c*                =-1   1 tangent pt,on curve only POC @ EP meet
//c*                = 2   2 intersection pts,on line & curve
//c*                =-2   2 intersection pts,on curve only EP & BP @ POC'S
//c*                = 3   1st pt on both,2nd pt on curve only  INTX & EP @ POC
//c*                =-3   2nd pt on both,1st pt on curve only  BP @ POC & INTX
//c*                = 4   1st pt on both,2nd pt on ext. of both; one INTX
//c*                =-4   1st pt on curve only; BP @ POC
//c*                = 5   in continuity
//c*                = 6   both pts on ext. of both
//c*                = 7   both pts on ext. of curve,one of them on
//c*                = 8   both pts on line,both pts on ext of curve
S41: switch (i2)
     {
       case 0: //see previous section for possible returns
//c         {didn't find any hits
          break;
       case -1:
          if(*gap <= P_TOL)
          {
            *x1[0] = *x1[nl];
            *y1[0] = *y1[nl];
            i2 = 1;
          }
          break;  
       case -3: //second pt on both
          *x1[0] = *x1[1];
          *y1[0] = *y1[1];
          i2 = 1;        
          break;
       case -4:
       case -2: //no actual hits
          i2 = 0 ; //coords already in x1 y1 x2 y2
          break;
       case 1: //lines from single tangent point
          *x1[0] = *x1[nl];
          *y1[0] = *y1[nl];
          break;
       case 2: //two real intersects exist
//c         {1st coords already in x1, y1, 2nd coords in x2, y2
          break;
       case 3: //1st point on both
          *x1[0] = *x1[nc];
          *y1[0] = *y1[nc];
          i2 = 1 ; //tells ing routine of the one hit
          break;
       case 4: //1st point on both
          *x1[0] = *x1[nc];
          *y1[0] = *y1[nc];
          i2 = 1 ; //tells ing routine of the one hit
          break;
       case 5://lines are continuous
          *x1[0] = *x1[nl];
          *y1[0] = *y1[nl];
          i2 = 1 ; //tells ing routine of the one hit
          break;
        default:
          i2 = 0;
        }  // end of the switch
        goto S40;

S20:     if(lpTwo->type  == 3) goto S30; //line 2 is also a curve

//c***    line 1 is a curve line 2 is straight
        i2 = -99;
         XLC(&lpOne->ldax1, &lpOne->lday1, &lpOne->ldax2, &lpOne->lday2, &lpOne->ldalngth,
             &lpTwo->ldax1, &lpTwo->lday1, &lpTwo->ldax2, &lpTwo->lday2, 
             *x1,  
             *y1,   
             *(x1+1),   
             *(y1+1),   
             *(x1+2),  
             *(y1+2),
             &nc,    &nl,     gap,     &i2);
        goto S41; 


//c***   line 1 and line 2 are both curves

S30:     XCC(&lpOne->ldax1, &lpOne->lday1, &lpOne->ldax2, &lpOne->lday2, &lpOne->ldalngth,
             &lpTwo->ldax1, &lpTwo->lday1, &lpTwo->ldax2, &lpTwo->lday2, &lpTwo->ldalngth,
                *x1,    
                *y1,   
                *(x1+1),   
                *(y1+1),    
                *(x1+2),
                *(y1+2),    
                &n1,      &n2,     gap,      &i2);

     switch (i2)
       {
        case -1:
        case 0:
        case 1://    if(i2 <= 1)
           *x1[0] = *x1[n1-1];
           *y1[0] = *y1[n1-1];
           i2 = 1   ; //to indicate one intersect
           break;
        default: 
          ;  
       }  
S40:  *irc = i2;
      return;
 }


void  OffsetDeleteArea(int ithis)
{
//c******* specifications ***********************************************
//c*                                                                    *
//c*       This is a recursive routine...                               *
//c*                                                                    *
//c*       Program Summary                                              *
//c*       ------- -------                                              *
//c*      This routine deletes lines                                    *
//c*                                                                    *
//c*                                                                    *
//c*       ithis     i*4     (input) The byte position in the gen_info  *
//c*                                 array                              *
//c*                                                                    *
//c*       Author][  Larry Anderson  Nov 88                              *
//c*                                                                    *
//c**********************************************************************
//c*  
     short j;                                                                  
     if(ithis == 0)return;
     lpGInfo = lpGInfoBase + ithis;
     if(lpGInfo->i_count == 0) return;
     lpAllS = lpAllSBase + lpGInfo->j;
     for(j = lpGInfo->j; j <= lpGInfo->j + lpGInfo->i_count; j++)
     {   //for each side in this area
          lpAllS = lpAllSBase + j;
          lpPoly2 = lpPoly2Base + lpAllS->all_sides; 
          if(lpPoly2->valid)OffsetEliminator(0);
     }

          for( lpPoly1->j = 1; lpPoly1->j <= lpGInfo->num_excl; lpPoly1->j++)
          {   //for any islands
              _fmemmove(lpPoly1->keyword,  &lpGInfo->AreaDesc, 4);
              _fmemmove(&lpPoly1->keyword[4], &lpPoly1->j, 2);
              HASHF((int)lpPoly1->id3,lpPoly1->keyword,&lpPoly1->loc);
              if(lpPoly1->loc==0) return;
              HASHG((int)lpPoly1->id3,lpPoly1->keyword,lpPoly1->loc);
              _fmemmove(&lpPoly1->island_ref,&lpPoly1->keyword[6],4);
              OffsetDeleteArea((int)lpPoly1->island_ref);
              lpPoly1->deletions = TRUE;
          } 
          lpGInfo->i_count = 0;
          return;
}


     void OffsetEliminator(int inode)
{     
//c***********************************************************************
//c*                                                                     *
//c*    Called by offset_find_perimeter.ftn, part of /umsc/util/offset,  *
//c*    this routine eliminates lines from the numerous arrays used to   *
//c*    define valid area lines.  Originally a copy of poly_eliminator,  *
//c*    I modified this routine to allow the ing routine to specify  *
//c*    the line to be deleted at a specific node.                       *
//c*                                                                     *
//c*                                                                     *
//c*         arguments                                                   *
//c*         ---------                                                   *
//c*         name     type               purpose                         *
//c*         ----     ----               -------                         *
//c*         inode    i*2 (input)      the node number of the node       *
//c*                                   with only one set of coordinates. *
//c*         out_line i*2 (input)      The segment line number to be     *
//c*                                   deleted.                          *
//c*                                                                     *
//c*                                                                     *
//c*                                                                     *
//c*        Written by Larry Anderson Nov 88                             *
//c***********************************************************************
//c*                                                                     *
        short NodeNum, next_bad, iway, iline, st ;
        double xcor,ycor;
        lpPolyCom2 lpPoly3;
        if(!lpPoly2->valid)return; //already deleted the line
        
        if(inode == 0)
        {  //gotta get a node number
           *sx = lpPoly2->from_x_array ; //use the coordinates of
           *sy = lpPoly2->from_y_array ; //the lines from_node
           *occ = 0;
           lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
           if(*sx != lpPoly2->from_x_array || *sy != lpPoly2->from_y_array)
           { 
               GSSiMsgBox(NULL,"*** Error *** Unable to locate\
 correct coordinates","Offset Eliminator",MB_ICONINFORMATION,0);               
                return;
           }  
           NodeNum = *this_node;
           next_bad = NodeNum;
        } 
        else 
        {
           NodeNum = inode;
           next_bad = NodeNum;  
           HASHF((int)lpPoly1->id2,&space[10],&lpPoly1->loc);
           if(lpPoly1->loc == 0)goto S100;
           HASHG((int)lpPoly1->id2,&space[10],lpPoly1->loc);
           *occ = 0;
           *sy = *sy;// - 1e-9;
           lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
           if(lpPoly1->loc != 0)goto S100;
           if(next_bad != *this_node)return;
   /*        { 
               MessageBox(NULL,"Found the wrong node",
               "Offset Eliminator",MB_ICONINFORMATION);
      //debug loop
 
       *srnum = 0;
       *d2b = 0e0 ;
       lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space);
        while (lpPoly1->st == 0)
        {  
           lpPoly1->st = PolyBTGetNext (lpPoly1->bid3, space);
           lpPoly1->i_count++;
        }       
        // end of the debug loop
             goto S100;
           }    
          }  */
        } //ok, got the node number   

            //beginning of debug stuff
      /*      _fstrcpy(lpPoly1->rec,"Going After node   ");
            itoa(NodeNum,&lpPoly1->rec[18],10);
            MessageBox(NULL,lpPoly1->rec,
                  "Offset Eliminator",MB_ICONINFORMATION);  */
            //ending of debug stuff

        lpPoly1->num_here = *numem;
S10:    lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]);
        if(NodeNum != *this_node)return;
        if( lpPoly1->loc !=0)goto S100;
        lpPoly1->real_x = lpPoly2->from_x_array;
        lpPoly1->real_y = lpPoly2->to_x_array;
        lpPoly2 = lpPoly2Base + *x_y_ary_loc;
        if(lpPoly1->real_x == lpPoly2->from_x_array &&
           lpPoly1->real_y == lpPoly2->to_x_array || 
           lpPoly1->real_x == lpPoly2->to_x_array &&
           lpPoly1->real_y == lpPoly2->from_x_array)
        { 

           lpPoly2->valid = FALSE ; //says the line is now a dead end
           lpPoly2->line_desc[1] = 0;
           lpPoly2->line_desc[2] = 0;
           iline = *the_rec ; //the original sgr record number

            //beginning of debug stuff
       /*     _fstrcpy(lpPoly1->rec,"Where I found LINE    ");
            itoa(*x_y_ary_loc,&lpPoly1->rec[20],10);
            MessageBox(NULL,lpPoly1->rec,
                  "Offset Eliminator",MB_ICONINFORMATION);*/
            //ending of debug stuff

           if(*sx ==lpPoly2->from_x_array &&
              *sy ==lpPoly2->from_y_array)
           { 
              xcor = lpPoly2->to_x_array; //Get set up to move to the
              ycor = lpPoly2->to_y_array; //other end of this line
           } 
           else 
           {  
            if(*sx == lpPoly2->to_x_array &&
               *sy == lpPoly2->to_y_array)
              { 
                xcor = lpPoly2->from_x_array;
                ycor = lpPoly2->from_y_array;
              } 
              else 
              {
                 GSSiMsgBox(NULL,"Found the wrong Coordinates",
                  "Offset Eliminator",MB_ICONINFORMATION,0);
              }  
           }  
           if(*the_way == 0)
           {  //currently going with the grain
              iway = 1; //will be going against it at the other end
           } 
           else 
           { //currently going against the grain
              iway = 0; //will be going with it at the other end
           }  

//d          print*,'Invaliding valid array element ',x_y_ary_loc
//c*         {I delete it from the id1 b_tree
            lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12],
                              &space[30],FALSE);

           while(*occ <= lpPoly1->num_here - 1)
           {
//d             print*,'moving occ ',occ + 1,' into occ ',occ
              *occ = *occ + 1;
              lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
              if(lpPoly1->loc != 0)goto S100;
              *occ = *occ - 1;
              PolyBTPut(lpPoly1->id1,&space[12]) ; //i now inplace of i + 1
              *occ = *occ + 1;
           }
           lpNodeArr = lpNodeArrBase + *this_node;
           *occ = lpPoly1->num_here  ; //the last occurrence of this node
           lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12],
                              &space[30],FALSE);
           if(lpNodeArr->node_array > 0)lpNodeArr->node_array--;

            //beginning of debug stuff
       /*     _fstrcpy(lpPoly1->rec,"Reducing the lines here to    ");
            itoa(lpNodeArr->node_array,&lpPoly1->rec[27],10);
            MessageBox(NULL,lpPoly1->rec,
                  "Offset Eliminator",MB_ICONINFORMATION); */
            //ending of debug stuff

           if(lpNodeArr->node_array == 0)
           { 
//c*            {delete the base record too
              *occ = 0;
               lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12],
                              &space[30],FALSE);
           } 
           else 
           {
              *occ = 0;
               lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //get the base rec
              if(lpPoly1->loc != 0)goto S100;
              if(*numem > 0) *numem = *numem - 1  ; //update the occurrence of this node
               PolyBTPut(lpPoly1->id1,&space[12]) ; //update the base record
           } 
           if(lpNodeArr->node_array == 1)
           {  //must perform additional
             *occ = 1;
             lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //get the other rec 
             lpPoly3 = lpPoly2Base + *x_y_ary_loc ;
             lpPoly3->valid = FALSE;
             lpPoly3->line_desc[1] = 0;
             lpPoly3->line_desc[2] = 0;
             NodeNum = *this_node             ; //deletions
//d            print*,'Calling poly_eliminator with node ',node_num
             PolyEliminator( NodeNum);
           }  
        } 
        else 
        {
           if(next_bad != *this_node)return;
           goto S10;
        }  

//c        next I move to the other end coords and check it for dead ending
         *occ = 0;
         *sx = xcor;
         *sy = ycor;
         lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //the base record
         if(lpPoly1->loc !=0)goto S100;
         if(*sx !=xcor && *sy !=ycor)return;
       /*  {         
            MessageBox(NULL,"Found the wrong Coordinates",
                  "Offset Eliminator",MB_ICONINFORMATION);
            goto S100;
           } */ 

//d        print*,'where I found node num ',this_node
//d        print*,'with ',numem,' lines intersecting there'
           lpNodeArr = lpNodeArrBase + *this_node;
           
            //beginning of debug stuff
         /*   _fstrcpy(lpPoly1->rec,"Now I jump to node    ");
            itoa(*this_node,&lpPoly1->rec[20],10);
            MessageBox(NULL,lpPoly1->rec,
                  "Offset Eliminator",MB_ICONINFORMATION); */
            //ending of debug stuff

         if(lpNodeArr->node_array == 0)return;
//c           i already have the valid array set to indicate
//c           this is a dead end line.

//I find the line to be eliminated and move everything
//c        {beyond it up one position in the hash array
          lpPoly1->got_a_hit = 0;
          lpPoly1->num_here = *numem;
          for(lpPoly1->l = 1;lpPoly1->l < lpPoly1->num_here;lpPoly1->l++)
          {
            lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]) ; //find the detailed record
            if(lpPoly1->loc != 0)goto S100;
            if(*the_rec == iline && *the_way == iway)
            { 
               lpPoly1->got_a_hit = 1 ; //found the one to be eliminated
            }  
            if(lpPoly1->got_a_hit == 1 && *occ < lpPoly1->num_here)
            {            
               *occ = *occ + 1;
               lpPoly1->loc= PolyBTGetFirst(lpPoly1->id1,&space[12]);
               if(lpPoly1->loc != 0)goto S100;
               if(*occ > 0)*occ = *occ - 1;
               PolyBTPut(lpPoly1->id1,&space[12]) ; //i now inplace of i + 1
            }  
          }
          if(lpNodeArr->node_array > 0)lpNodeArr->node_array--;
            //beginning of debug stuff
     /*       _fstrcpy(lpPoly1->rec,"Reducing the lines here to    ");
            itoa(lpNodeArr->node_array,&lpPoly1->rec[27],10);
            MessageBox(NULL,lpPoly1->rec,
                  "Offset Eliminator",MB_ICONINFORMATION);  */
            //ending of debug stuff
          *occ = lpPoly1->num_here  ; //the last occurrence of this node
           lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12],
                              &space[30],FALSE);
          if(lpNodeArr->node_array == 0)
          {  //delete the base record too
             *occ = 0;
              lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12],
                              &space[30],FALSE);
          } 
          else 
          {
             *occ = 0;
             lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //get the base rec
             if(lpPoly1->loc != 0)goto S100;
             if(*numem > 0)*numem = *numem - 1; //update the occurrence of this node
              PolyBTPut(lpPoly1->id1,&space[12]) ; //update the base record
          }  
         lpNodeArr = lpNodeArrBase + *this_node;
         if(lpNodeArr->node_array == 1)
         {  //must perform additional
             *occ = 1;
             lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //get the other rec 
             lpPoly3 = lpPoly2Base + *x_y_ary_loc;
             lpPoly3->valid = FALSE;
             lpPoly3->line_desc[1] = 0;
             lpPoly3->line_desc[2] = 0;
             NodeNum = *this_node; //deletions
             PolyEliminator(NodeNum);
         }
          
         lpPoly2->line_desc[1] = 0;
         lpPoly2->line_desc[2] = 0;
         return;
S100:    lpNodeArr = lpNodeArrBase + NodeNum;
         lpNodeArr->node_array = 0;
         return;
}
    

 int  offset_check_coincident(lpWS L1, lpWS L2)
{         
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       This routine is ed to check for coincident lines.        *
//c*                                                                    *
//c*       This routine, part of the polygon system, is used after      *
//c*       the ing routine determined the line in ldatable[i]       *
//c*       intersects with ldatable(J) twice.  The intersecting         *
//c*       coordinates, passed to this routine are used to determine    *
//c*       the azimuths of the two lines and the type of coincidence    *
//c*       (if any).                                                    *
//c*                                                                    *
//c*       Arguments                                                    *
//c*       ---------                                                    *
//c*       i     i*4     (input)  The Ltable element of the first line  *
//c*       j     i*4     (input)  The Ltable element of the second line *
//c*       irc   i*4     (output) 0 = no coincidence.                   *
//c*                              1 = line I coincident for its' entire *
//c*                                  length with line J.               *
//c*                              2 = line J coincident for its' entire *
//c*                                  length with line I.               *
//c*                              3 = portions of both lines are        *
//c*                                  coincident.                       *
//c*                              4 = line I is identical to line J.    *
//c*                                                                    *
//c*       Author][ Larry Anderson May 89                                *
//c*                                                                    *
//c**********************************************************************
//c*

#define n3 3  

         short i2s[5];
     
      if(L1->type !=L2->type)return 0; //coincidence not possible
      if(L1->type == 2)
      {  
//c*        {Already know we have two hits.  So... lines are
//c*        {parallel... All I gotta do is determine type of overlap.
           i2s[1] = INLNCK(&L1->ldax1,&L1->lday1,&L1->ldax2,
                  &L1->lday2,&L2->ldax1,&L2->lday1);
           i2s[2] = INLNCK(&L1->ldax1,&L1->lday1,&L1->ldax2,&L1->lday2,
                  &L2->ldax2,&L2->lday2);
           i2s[3] = INLNCK(&L2->ldax1,&L2->lday1,&L2->ldax2,&L2->lday2,
                  &L1->ldax1,&L1->lday1);
           i2s[4] = INLNCK(&L2->ldax1,&L2->lday1,&L2->ldax2,&L2->lday2,
                  &L1->ldax2,&L1->lday2);

          if(i2s[1]+i2s[2]+i2s[3]+i2s[4] ==0)return 4;
          if(i2s[1] == 0 && i2s[2] == 0)return 2;
          if(i2s[3] == 0 && i2s[4] == 0)return 1;
          return  3 ; //lines overlap for only a portion of both
           
      } 
      else 
      {  //they're both curves
//c*        {If the radi are within  p_tol of one another I wildal consider
//c*        {them to be coincident curves
          lpPoly1->ry = LDIST(L1->ldax2,L1->lday2,L2->ldax2,L2->lday2) ; //dist between radi
          if(lpPoly1->ry > P_TOL)  return 0; //not coincident

//c*        {Here the coords of the first curve's PT are found 
           lpPoly1->sign = fabs(L1->ldalngth);
           LOL8(&L1->ldax1, &L1->lday1, &L1->ldax2, &L1->lday2,
                &L1->ldalngth, &lpPoly1->sign, &lpPoly1->hit_x, &lpPoly1->hit_y, n3);

//c*        {Here the coords of the second curve's PT are found
           lpPoly1->sign = fabs(L2->ldalngth);
           LOL8(&L2->ldax1, &L2->lday1, &L2->ldax2, &L2->lday2,
                &L2->ldalngth, &lpPoly1->sign, &lpPoly1->real_x, &lpPoly1->real_y, n3);

//c*        {I check if the pc of curve J is in curve I
          i2s[1]= INCRVE(&L1->ldax1, &L1->lday1, &L1->ldax2, &L1->lday2,
                  &L1->ldalngth, &L2->ldax1, &L2->lday1);

//c*        {I check if the pc of curve I is in curve J
          i2s[2]= INCRVE(&L2->ldax1, &L2->lday1, &L2->ldax2, &L2->lday2,
                  &L2->ldalngth, &L1->ldax1, &L1->lday1);

//c*        {I check if the pt of curve J is in curve I
          i2s[3]= INCRVE(&L1->ldax1, &L1->lday1, &L1->ldax2, &L1->lday2,
                  &L1->ldalngth, &lpPoly1->real_x, &lpPoly1->real_y);

//c*        {I check if the pt of curve I is in curve J
          i2s[4]= INCRVE(&L2->ldax1, &L2->lday1, &L2->ldax2, &L2->lday2,
                  &L2->ldalngth, &lpPoly1->hit_x, &lpPoly1->hit_y);

          if(i2s[1] + i2s[2] + i2s[3] +i2s[4] == 0) return 4;
 
          if(i2s[1] + i2s[3] == 0) return 2;
  
          if(i2s[2] + i2s[4] == 0) return 1;
          return  3 ; //lines overlap for onlday a portion of both
      } // if(ldatype[i] ==line$)
 }
      
BOOL OffsetHighlightedLines (int NumPoints,HANDLE hPoints, double Dist)
#if ENABLETRACE
{GSSiEnterProg (715);
#endif
{
 HDC hDC;
 char key;
 POINT	MousePoint, TestPoint;
 DPOINT	BasePoint;
 BOOL	Cancel;
 LPTHEME	pTheme;
 short  st; 
 long	np, i;
 LPLONG	npt; 
 UINT	n;
 long i4, NumCrvPts, loc; 
 long size;
 LPDPOINT	lpDpoint, lpDNext;
 HPPOINT	lpPoint, lpPoly;
 int     LastLine;
 char	cCor[32];
 HANDLE	hPoly; 
 LPSTR	lpColon;
 HPEN	hPen, OldPen;  
 HANDLE	hDPoints;
 HPDPOINT	lpDPoints;
 
  lpLArea OutArea;
  Area A;
  lpArea lpA = (lpArea) &A;
  lpDLine  lpLda;
  HGLOBAL heap_ptr; 
  lpA->type = 1;  
  lpA->whos_callen = 1;
  lpA->LinkDesc = 351;
  lpA->fillet_desc = 1321;  
  
  lpA->NumPts = (UINT)NumPoints * 2 - 2;
  size = (long)sizeof(DLine)*(long)(lpA->NumPts+1); 
  heap_ptr = GSSiGlobAlloc ( 326, GHND, size); 
  lpA->lpDLineBase = (lpDLine) GlobalLock(heap_ptr);
  lpLda = lpA->lpDLineBase; 
  lpDpoint = (HPDPOINT)GlobalLock (hPoints);
  lpDNext = lpDpoint + 1;  
  n = 1;
  for(i=0; i < NumPoints-1; i++,n++,lpDpoint++,lpDNext++,lpLda++)
  { 
    lpLda->F.x = lpDpoint->x;
    lpLda->F.y = lpDpoint->y;
    lpLda->T.x = lpDNext->x;
    lpLda->T.y = lpDNext->y;
    lpLda->Refn = i;
    lpLda->Desc = 21;
    lpLda->Type = 2;           
    lpLda->ID = 123+n;
  } 
  GlobalUnlock (hPoints);
  lpDpoint = (HPDPOINT)GlobalLock (hPoints); 
  lpDpoint += (NumPoints-1);
  i = NumPoints;
  lpDNext = lpDpoint - 1;  
  for(i=0; i < NumPoints-1; i++,n++,lpDpoint--,lpDNext--,lpLda++)
  { 
    lpLda->F.x = lpDpoint->x;
    lpLda->F.y = lpDpoint->y;
    if(n < lpA->NumPts)
    { 
      lpLda->T.x = lpDNext->x;
      lpLda->T.y = lpDNext->y;
    }  
    lpLda->Refn = i;
    lpLda->Desc = 21;
    lpLda->Type = 2;           
    lpLda->ID = 123+n;
  }
  GlobalUnlock (hPoints);
  lpDpoint = (LPDPOINT) GlobalLock (hPoints);  
  lpLda--;
  lpLda->T.x = lpDpoint->x;
  lpLda->T.y = lpDpoint->y;
  GlobalUnlock (hPoints);
  lpA->NumSides = lpA->NumPts;
  lpA->offset_dist = -Dist;
  OFFSET_MAIN(lpA, &OutArea, &i4);          
  if(i4 != 0)
  {   
  	 NumOffsetFailed++;
     GSSiMsgBox(NULL,"Sorry... Unable to offset this item",
               "Offset Error",MB_ICONINFORMATION,0);
          
  }
  GlobalUnlock(heap_ptr); 
  GSSiGlobUlFree (&heap_ptr);                           
  GSSiGlobUlFree (&hSavePoly);
  if(i4 != 0)
  {
        OffsetClose();
     	WaitCursor (-1);
{
#if ENABLETRACE
GSSiExitProg (715);
#endif
        return TRUE;
}
  }
  BasePoint.x = 0e0;
  lpLineBase = OutArea->Lines;
  for (i=1,OutArea->Lines++; i <= OutArea->NumSides; i++,OutArea->Lines++)
  {
     if(OutArea->Lines->type == 3)
     {
       BasePoint.x = BasePoint.x + fabs(OutArea->Lines->lngth);
     }
  }
  if(BasePoint.x > 0e0)//we found some curved lines
  { //gotta figure out how many intermediate points these curves
    //will be broken into. BasePoint.x has the length of the curves in feet.
    //First I need to convert the length from Base coordinates to Window
    //CurView->BaseUnitsPerPixel holds how many feet it takes per pixel
    //I want a point approximately every 50 pixels  
    if (CurView->BaseUnitsPerPixel==0)
    	NumCrvPts=12;
    else
    	NumCrvPts = 4 * BasePoint.x / (CurView->BaseUnitsPerPixel);
    //POINT BasePtToWinPt (DPOINT WPoint)
    //  SetCurvPltCtol (CurView->BaseUnitsPerPixel * 2.0);
    SetCurvPltCtol (0.05 * 2.0);
    //DPOINT WinPtToBasePt (POINT Point)
    //Second I need to figure out how many pixels I want to include in each
    //line segment.  Make a call to       void SetCurvPltCtol (double INCTOL);
    //so it's dividing the line properly
		  
  }
  else
  	NumCrvPts = 0;	
  size = (long)sizeof(POINT)*((long)OutArea->NumSides + NumCrvPts + 10);
  heap_ptr = NULL; 
  heap_ptr = GSSiGlobAlloc ( 327, GHND, size); 
  lpPoint = (HPPOINT) GlobalLock(heap_ptr);
  lpPoly = lpPoint; 
  
   OutArea->Lines = lpLineBase + 1;   
   for (np = 0,i=1; i <= OutArea->NumSides; i++,OutArea->Lines++, lpPoint++)
   { //put the first point of the line in 
        if(OutArea->Lines->type == 2)
        {
          BasePoint.x = OutArea->Lines->x1;
	   	  BasePoint.y = OutArea->Lines->y1;
		  *lpPoint = BasePtToWinPt(&BasePoint);
		 // lpPoint->y = lpPoint->y - CurView->DrawRect.top;
		  if(i == 1)TestPoint = *lpPoint; 
      /*    BasePoint.x = OutArea->Lines->x2;
	   	  BasePoint.y = OutArea->Lines->y2;
		  TestPoint = BasePtToWinPt(BasePoint); */
		  np++;
					  
		}  
        if(OutArea->Lines->type == 3)
        {
       /*   BasePoint.x = OutArea->Lines->x1;
	   	  BasePoint.y = OutArea->Lines->y1;
		  TestPoint = BasePtToWinPt(BasePoint); 
          BasePoint.x = OutArea->Lines->x2;
	   	  BasePoint.y = OutArea->Lines->y2;
		  TestPoint = BasePtToWinPt(BasePoint); 
          BasePoint.x = OutArea->Lines->rx;
	   	  BasePoint.y = OutArea->Lines->ry;
		  TestPoint = BasePtToWinPt(BasePoint);*/ 
		  if(i == 1)
		  { 
             BasePoint.x = OutArea->Lines->x1;
	   	     BasePoint.y = OutArea->Lines->y1;
		     TestPoint  = BasePtToWinPt(&BasePoint);
		  }
		  {
			  HANDLE hDP = GSSiGlobAlloc (0,GMEM_MOVEABLE,USHRT_MAX);
			  LPDPOINT	 pPointD = GlobalLock (hDP);
			  long	ndp=0;
              st =  CURVPLT(OutArea->Lines, &ndp,  &pPointD);
              while (ndp--)
			  {
				  *lpPoint++ = BasePtToWinPt (pPointD++);
				  np++;
			  }
			  GSSiGlobUlFree (&hDP);
		  }
        }
   }
   *lpPoint = TestPoint;
   lpPoint = lpPoly;
   np++;
                     
    hDPoints = GSSiGlobAlloc ( 328,GMEM_MOVEABLE,(long)np*sizeof(DPOINT));
    lpDPoints = (HPDPOINT)GlobalLock (hDPoints);
    for (i=0;i<np;i++,lpPoint++,lpDPoints++)
		*lpDPoints = WinPtToBasePt(*lpPoint);
	GlobalUnlock (hDPoints);
    lpDPoints = (HPDPOINT)GlobalLock (hDPoints);
	AddAreaToOffsetFile (0,3,np, lpDPoints,1,0,0);
	GSSiGlobUlFree (&hDPoints);
               
    DisplayPolyOff();  
    DisplayMaskArea();
                
    GSSiGlobUlFree (&heap_ptr); 
    OffsetClose();
	WaitCursor (-1);
{
#if ENABLETRACE
GSSiExitProg (715);
#endif
	return TRUE;
}
#if ENABLETRACE
}
#endif
} 

     int CURVPLT(lpLine L1,LPLONG np, HPDPOINT *lpPoint)
#if ENABLETRACE
{GSSiEnterProg (670);
#endif
{
//C******* SPECIFICATIONS **********************************************
//C*                                                                   *
//C*       PROGRAM SUMMARY                                             *
//C*       ------- -------                                             *
//C*       THIS ROUTINE PLOTS A CURVE OF LENGTH (L) WITH BEGINNING     *
//C*       POINT OF (X1,Y1) AND RADIUS POINT OF (X2,Y2).               *
//          
//         Arguments
//  input  lpLine L1 A structure containing information about a line
//  input  HGLOBAL *heap_ptr is a pointer to an area of memory supplied
//                   by the calling program, used to contain a series
//                   of coordinate set structures;
//  function retrn value: 0 if it added points to *heap_ptr
//                         -1 if it didn't do anything.

      double  DTH, A1, LenRad, LenCrv;
      DPOINT TestPoint, HoldPt;
      DPOINT  StartPoint, EndPoint, RadPt; 
      short size, Npts;
      DPOINT MyPt;
    //  XB=L1->x1; //X1;
    //  YB=L1->y1; //Y1;
//C******* COMPUTE RADIUS
     if( L1->rad <= 0)
{
#if ENABLETRACE
GSSiExitProg (670);
#endif
     	return -1;  
}
//C******* CALCULATE AZIMUTH OF BEGINNING & ENDING ARC RADII 

		    LenRad      = L1->rad; 
		    LenCrv      = L1->lngth;
            RadPt.x = L1->rx;
            RadPt.y = L1->ry;
            
//C******* CALCULATE AZIMUTH OF BEGINNING & ENDING ARC RADII
      A1 = L1->azm; //atan2(S,C);
             
      //A2 = L1->eazm;//A1 - L1->lngth / R;
      if(LenRad <= CTOL) goto S110; 
    //  if(L1->lngth <= CTOL) goto S100;
      DTH = acos ((LenRad - CTOL)/LenRad);
       if(DTH == 0) goto S110;
//C******* CALCULATE NUMBER OF VECTORS IN CURVE
      Npts = (short)(fabs(LenCrv) / (DTH * LenRad));
      Npts++; // to plot at least the beginning of the curve
      if(LenCrv < 0)DTH = -DTH;
      if(Npts < 2)goto S110;
//C******* PLOT REMAINING VECTORS
      for (lpPoly1->l = 2;lpPoly1->l <= Npts;lpPoly1->l++)
      {
		 MyPt = dnewpt (RadPt,A1,LenRad);
         **lpPoint = MyPt;
         *lpPoint = *lpPoint + 1;
         *np = *np + 1;
         A1 -=  DTH;
      }
//C******* FINISH CURVE
 S110:  *lpPoint = *lpPoint - 1;

{
#if ENABLETRACE
GSSiExitProg (670);
#endif
      return 0;
}
#if ENABLETRACE
}
#endif
}

void SetCurvPltCtol (double INCTOL)
#if ENABLETRACE
{GSSiEnterProg (671);
#endif
{
      CTOL = INCTOL;
{
#if ENABLETRACE
GSSiExitProg (671);
#endif
      return;
}
#if ENABLETRACE
}
#endif
}  

HANDLE OffsetPickedArea2 (int Item, double Dist,LPLONG pNumPoints)
#if ENABLETRACE
{GSSiEnterProg (714);
#endif
{
 HDC hDC;
 char key;
// POINT	MousePoint, TestPoint;
 DPOINT	BasePoint, TestPoint;
 BOOL	Cancel;
 LPTHEME	pTheme;
 int	st;      
 long	np, i;
 long i4, NumCrvPts, loc; 
 long size;
 HPDPOINT	lpDpoint, lpDNext;
// HPPOINT	lpPoint, lpPoly;
 int     LastLine;
 char	cCor[32];
 HANDLE	hPoly; 
 LPSTR	lpColon;
 HPEN	hPen, OldPen;  
 HANDLE	hDPoints=0;
 HPDPOINT	lpDPoints; 
 LPVIEWPORT	SaveVP=CurView;  
 BOOL	Reverse=FALSE; 
 MNMXCORD	PolyBounds;
 long	nPnts; 
 
  lpLArea OutArea;
  Area A;
  lpArea lpA = (lpArea) &A;
  lpDLine  lpLda;
  HGLOBAL heap_ptr; 
  lpA->type = 1; //indicates a standard area offset is requested
  lpA->whos_callen = 1;
  lpA->offset_dist = -50;
  lpA->LinkDesc = 351;
  lpA->fillet_desc = 1321; 
  if (PickList[Item].Type == 1 || PickList[Item].Type == 4)
  { 
  	long	nPnts;
  	 
  	hDPoints = CreateCirclePoly (PickList[Item].BeginPoint,Dist,&nPnts,0); 
  	*pNumPoints = nPnts;
  	return hDPoints;
  } 
  if (PickList[Item].Area < 0)
  	Reverse = TRUE;
  	
	if (GetPolyPoints ((LPPICKDATAHEADER)&PickList[Item],Reverse,&nPnts,&hPoly))
    	{
			double	MaxPixels, MaxDist, ThinDist;

			lpDpoint = (HPDPOINT)GlobalLock (hPoly);
	   		WaitCursor (1); 
	   		GetPolyBoundsD (hPoly,nPnts,&PolyBounds,PickList[Item].Type);
			MaxPixels = max (CurView->ScreenRect.right - CurView->ScreenRect.left,
							 CurView->ScreenRect.bottom - CurView->ScreenRect.top);
			MaxDist = max (PolyBounds.xmx - PolyBounds.xmn,PolyBounds.ymx - PolyBounds.ymn);
			ThinDist = 3*MaxDist/MaxPixels;
	   		ThinPoly (&nPnts,lpDpoint, ThinDist);
			GlobalUnlock (hPoly);
	   		BiasPoly (hPoly,nPnts,-PolyBounds.xmn,-PolyBounds.ymn);
            lpA->NumPts = nPnts ; 
 		  lpDpoint = (HPDPOINT)GlobalLock (hPoly);
          size = (long)sizeof(DLine)*(long)(lpA->NumPts); 
          heap_ptr = GSSiGlobAlloc ( 324, GMEM_MOVEABLE, size); 
          lpA->lpDLineBase = (lpDLine) GlobalLock(heap_ptr);
          lpLda = lpA->lpDLineBase; 
          lpDNext = lpDpoint + 1;
          for(i=0; i < lpA->NumPts; i++,lpDpoint++,lpDNext++)
          { 
            lpLda = lpA->lpDLineBase + i;
            lpLda->F.x = lpDpoint->x;
            lpLda->F.y = lpDpoint->y;
            if(i < lpA->NumPts - 1)
            { 
              lpLda->T.x = lpDNext->x;
              lpLda->T.y = lpDNext->y;
            }  
            lpLda->Refn = i;
            lpLda->Desc = 21;
            lpLda->Type = 2;           
            lpLda->ID = 123+i;
          }
          GlobalUnlock (hPoly);
          lpDpoint = (HPDPOINT)GlobalLock (hPoly);;
          lpLda->T.x = lpDpoint->x;
          lpLda->T.y = lpDpoint->y;
          lpA->NumSides = lpA->NumPts;
		  lpA->offset_dist = -Dist;
;
          OFFSET_MAIN(lpA, &OutArea, &i4);          
          if(i4 != 0)
          {  
          	 NumOffsetFailed++;
             GSSiMsgBox(NULL,"Sorry... Unable to offset this item",
                       "Offset Error",MB_ICONINFORMATION,0);
          
          }
          GSSiGlobUlFree (&heap_ptr); 
          GSSiGlobUlFree (&hPoly);
		  if(i4 != 0)
		  {
                OffsetClose();
		     	WaitCursor (-1);
{
#if ENABLETRACE
GSSiExitProg (714);
#endif
		        return 0;
}
		  }
		  BasePoint.x = 0e0;
		  lpLineBase = OutArea->Lines;
		  for (i=1,OutArea->Lines++; i <= OutArea->NumSides; i++,OutArea->Lines++)
		  {
		     if(OutArea->Lines->type == 3)
		     {
		       BasePoint.x = BasePoint.x + fabs(OutArea->Lines->lngth);
		     }
		  }
		  if(BasePoint.x > 0e0)//we found some curved lines
		  { //gotta figure out how many intermediate points these curves
		    //will be broken into. BasePoint.x has the length of the curves in feet.
		    //First I need to convert the length from Base coordinates to Window
		    //CurView->BaseUnitsPerPixel holds how many feet it takes per pixel
		    //I want a point approximately every 50 pixels  
		    if (CurView->BaseUnitsPerPixel==0)
		    	NumCrvPts=12;
		    else
		    	NumCrvPts = 4*BasePoint.x / (CurView->BaseUnitsPerPixel);
		    //POINT BasePtToWinPt (DPOINT WPoint)
          //  SetCurvPltCtol (CurView->BaseUnitsPerPixel * 2.0);
            SetCurvPltCtol (0.05 * 2.0);
            //DPOINT WinPtToBasePt (POINT Point)
		    //Second I need to figure out how many pixels I want to include in each
		    //line segment.  Make a call to       void SetCurvPltCtol (double INCTOL);
            //so it's dividing the line properly
		  
		  }
		  else
		  	NumCrvPts = 0;	
          hDPoints = GSSiGlobAlloc ( 325,GMEM_MOVEABLE,(long)USHRT_MAX*sizeof(DPOINT));
          lpDPoints = (HPDPOINT)GlobalLock (hDPoints);
          OutArea->Lines = lpLineBase + 1;   
          
          for (np = 0,i=1; i <= OutArea->NumSides; i++,OutArea->Lines++, lpDPoints++)
          { //put the first point of the line in 
          	if(OutArea->Lines->type == 2)
            {
				lpDPoints->x = OutArea->Lines->x1;
				lpDPoints->y = OutArea->Lines->y1;
				//*lpPoint = BasePtToWinPt(&BasePoint);
				// lpPoint->y = lpPoint->y - CurView->DrawRect.top;
				if(i == 1)
					TestPoint = *lpDPoints; 
				/*    BasePoint.x = OutArea->Lines->x2;
				BasePoint.y = OutArea->Lines->y2;
				TestPoint = BasePtToWinPt(BasePoint); */
				np++;
					  
			}  
            else if(OutArea->Lines->type == 3)
            {
	       /*   BasePoint.x = OutArea->Lines->x1;
		   	  BasePoint.y = OutArea->Lines->y1;
			  TestPoint = BasePtToWinPt(BasePoint); 
	          BasePoint.x = OutArea->Lines->x2;
		   	  BasePoint.y = OutArea->Lines->y2;
			  TestPoint = BasePtToWinPt(BasePoint); 
	          BasePoint.x = OutArea->Lines->rx;
		   	  BasePoint.y = OutArea->Lines->ry;
			  TestPoint = BasePtToWinPt(BasePoint);*/ 
			  if(i == 1)
			  { 
	             TestPoint.x = OutArea->Lines->x1;
		   	     TestPoint.y = OutArea->Lines->y1;
			  }
                 st =  CURVPLT(OutArea->Lines, &np,  &lpDPoints); 
                         
            }
          }
		  *lpDPoints = TestPoint;
		  np++;
                     
		  GlobalUnlock (hDPoints);
               
                
                /*
					SetDisplayMode (CurView->hDC, GF_TEXTMODE);
					if (!FileMode)
					{
					    CurView->hRgn = CreateVPRgn();
					    SelectClipRgn (CurView->hDC,CurView->hRgn);
					    DeleteObject(CurView->hRgn);  
					}
               		hPen = CreatePen (PS_SOLID,6,RGB(255,0,0));
                    OldPen = SelectObject (CurView->hDC,hPen);
                    i = Polyline (CurView->hDC,lpPoly,np);*/
         GSSiGlobUlFree (&heap_ptr); 
                   /*                          
                    SelectObject (CurView->hDC,OldPen);
                    DeleteObject (hPen); */
                    //GlobalUnlock (hPoly);
                    //GlobalFree (hPoly); 
                    //here I free up the memory used by the OutArea
         OffsetClose();
   		 WaitCursor (-1);
	    		
	} 
    *pNumPoints = np; 
	BiasPoly (hDPoints,np,PolyBounds.xmn,PolyBounds.ymn);

{
#if ENABLETRACE
GSSiExitProg (714);
#endif
        	return hDPoints;
}
#if ENABLETRACE
}
#endif
} 

    
      

