//define lpldatablehome
//define lpPolyHome
#include "graphint.h"     
#include "hash.h"

#include "gmextern.h"

#define lpPolyHome    
#include "offsetmn.h"
#include "polycom.h"  
#include "offdefs.h"
#include "polybt.h"
#undef lpPolyHome    


  DPoint       DP;
  lpDPoint     lpDP = (lpDPoint) &DP;
  lpDPoint     lpDPBase = (lpDPoint) &DP;
  FILE         *dbfile; 
  unsigned int ldatot;
  lpLine       lpL;  
  lpLine       lpLineBase;
  lpArea       lpAreaBase; 
  Links      Lk;
  lpLinks    lpLk = &Lk;
  
  lpTA       lpNew;
  lpTA       lpNewBase;
  
  lpWS       lpWork;
  lpWS       lpWorkBase;
  
  lpAS       lpAllS;
  lpAS       lpAllSBase;
  
  lpNA       lpNodeArrBase;
  lpNA       lpNodeArr;
  
  lpPolyCom1 lpPoly1Base;
  lpPolyCom1 lpPoly1;
  
  lpPolyCom2 lpPoly2Base;
  lpPolyCom2 lpPoly2;
  
  lpGenInfo  lpGInfo;
  lpGenInfo  lpGInfoBase;

static int MaxNumAreas;
static int MaxNewBase;
static int MaxNodeArrBase;
static int MaxWorkBase;
static int MaxNumAreas; 
static int MaxAllSides; 
static int MaxLineBase; 
static int MaxPoly2Base; 

extern	char space[52]; 
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
static  short far LowestSrnum; 
static  short far Here[4];
static   double far xxx[64]; 
static   short far  NodeIds[64], LdaLocs[64];

//static	BOOL RIGHT_SIDE_ONLY, LEFT_SIDE_ONLY;
//static	lpPolyCom1 lpPoly1Base;
//static	lpPolyCom2 lpPoly2Base;
typedef	struct
    {
      LPSTR mem_ptr; 
      HGLOBAL heap_ptr;
    }  lda4;

static lda4 file[12]; 
static	BOOL RIGHT_SIDE_ONLY = FALSE, LEFT_SIDE_ONLY = FALSE, 
     ROUND_ALIGNMENT_ENDS = FALSE, DEBUGGING = FALSE;    
static	double   IN_PTOL;

void   Get3PtsCurve(lpWS in,LPDOUBLE lpPC,LPDOUBLE lpPOC,LPDOUBLE lpPT);

void PolyTieBreaker (double *azk, double *azl,long K,long L)
{
//c******* specifications ***********************************************
//c*                                                                    *
//c*      program summary                                               *
//c*      ------- -------                                               *
//c*      This routine, part of the polygon analysis system,            *
//c*      performs an exacting determination to validate lines with     *
//c*      almost the same initial azimuths.                             *
//c*                                                                    *
//c*      If one or more of the lines are curves, this routine finds    *
//c*      the shortest radius and creates an azimuth to a point that    *
//c*      distance down each curve.  The change in azimuths encountered *
//c*      help determine the direction each lines is really proceeding. *
//c*                                                                    *
//c*      Argument Description                                          *
//c*      -------- -----------                                          *
//c*      azk        r*8     (input/output) Line K's azimuth            *
//c*      azl        r*8     (input/output) Line L's azimuth            *
//c*                                                                    *
//c*      K          i*4     (input)  The element number in the original*
//c*                                  sgr record array of the first     *
//c*                                  azl (ldarec(n))                   *
//c*                                                                    *
//c*      L          i*4     (input)  The element number of the current *
//c*                                  next-possible azl                 *
//c*                                                                    *
//c*      When this routine determines which line to take, it modifies  *
//c*      either the azk or azl to cause the                            *
//c*      subsequent test in poly_areas to work properly.               *
//c*                                                                    *
//c*      Author][  Larry Anderson  Jul 88                               *
//c*                                                                    *
//c**********************************************************************
//c*
    lpWS line1;
    lpWS line2;


//c*         degree_of_curve = 5729.58 / ldarad[k]
//c*         the line I am currently on has an azimuth  =  curr_azm
         lpPoly1->dis21 = *azl;
         lpPoly1->dis22 = *azk;
         *azl = 0e0;
         *azk = 0e0;
         *srnum = lpPoly1->poss_orig[L];
         *d2b = 0;
          lpPoly1->loc = PolyBTGetFirst(lpPoly1->bid3,space) ; //{the base record
//c*       after this  ldaloc hold the element number
//c*       for this particular
         line1 = lpWorkBase + *ldaloc;
         *srnum = lpPoly1->poss_orig[K];
         *d2b = 0;
         lpPoly1->loc = PolyBTGetFirst(lpPoly1->bid3,space) ; //{the base record
         line2 = lpWorkBase + *ldaloc; 
         if(line1->type == 2 && line2->type == 2)return; //nothing i can do
         if(line1->type == 3 && line2->type == 3)
         {
//c*          {both lines are curves and I find the shortest radius
            lpPoly1->dis11 = __min(line2->ldarad,line1->ldarad);
//c*                {dis11 = the shorter of the 2 rads
         } 
         else 
         { 
           if(line2->type == 3)lpPoly1->dis11 = line2->ldarad;
           if(line1->type == 3) lpPoly1->dis11 = line1->ldarad;
         } 

//c*       the first thing I do is to insure the matching azimuths
//c*       are within aztol of one another (they won't be when
//c*       one azimuth is just less than pii and the other is
//c*       just greater than zero.
//c*          {adjustments are necessary.
            if(lpPoly1->dis21 > 5.5 || lpPoly1->dis22 > 5.5)
            { 
               lpPoly1->dis21 = LTWOPI(lpPoly1->dis21 - PY);
               lpPoly1->dis22 = LTWOPI(lpPoly1->dis22 - PY);
            } 
            else 
            { 
              if(lpPoly1->dis21 <7e-1 || lpPoly1->dis22 <7e-1)
              { 
                lpPoly1->dis21 = LTWOPI(lpPoly1->dis21 + PY);
                lpPoly1->dis22 = LTWOPI(lpPoly1->dis22 + PY);
              } 
            }  
//d           print*,'Adjusting the azimuths to ',dis21,
//d    +      ' and ',dis22

//d        print*,'min dis11 = ',dis11
//c*       First I compute an azimuth from the intersection point
//c*       to a point on line K, dis11 dis11ance from the intx point
         if(line2->type == 3)
         { //  ; //{it's a curve
            if(line2->ldarad < 1e-4) goto S10 ; //{very short radi
            if(lpPoly1->with_the_grain[K])
            { 
               *azk = DSIGN(lpPoly1->dis11/(2e0*line2->ldarad),
                     -1e0*line2->ldalngth);
            } 
            else 
            { //{going against the grain
               *azk = DSIGN(lpPoly1->dis11/(2e0*line2->ldarad),line2->ldalngth);
            }  
//d           print*,'working with sgr line ',poss_orig[k]
//d           print*,'ldarad = ',ldarad(line2),
//d    +             ' grain = ',with_the_grain[k]
//d           print*,'At 1 radian distance from intx on sgr line ',
//d    +              poss_orig[k],' delta_azimuth = ', azk
         }  
S10:      *azk += lpPoly1->dis22;

//c*       Next I compute an azimuth to a point on line L
//c*       that is dist distance from the intersection point.
         if(line1->type == 3)
         {  //{it's a curve
           if(line1->ldarad < 1e-4) goto S20 ; //{very short radi
           if(lpPoly1->with_the_grain[L])
           { 
               *azl = DSIGN(lpPoly1->dis11/(2e0*line1->ldarad),
                     -1e0*line1->ldalngth);
            } 
            else 
            {  //{going against the grain
               *azl = DSIGN(lpPoly1->dis11/(2e0*line1->ldarad),line1->ldalngth);
            }  
//d          print*,'working with sgr line ', poss_orig(L)
//d          print*,'ldarad = ',ldarad(line1),
//d    +            ' grain = ',with_the_grain(l)
//d          print*,'At 1 radian distance from intx on sgr line ',
//d    +             poss_orig(L),' delta_azimuth = ', azl
         }  
S20:      *azl += lpPoly1->dis21;

//d        print*,'line ',poss_orig(L),' adjusted direction = ',azL
//d        print*,'line ',poss_orig[k],' adjusted direction = ',azk
         return;       
         }
         
         
   BOOL poly_load_lda_coords(void)
   {
//c*                                                                     *
//c*       B_tree BlpPoly1->id3 is designed to contain information about each     *
//c*       line's intersection.                                          *
//c*       |  I*2 | R*8 |   I*2   |  I*2  |RECN = sgr file record numbeR *
//c*       | RECN | O D | NODE_ID |LDA_LOC|O D  = lpPoly1->offset dist from BP    *
//c*       |------------|---------|-------|         origin               *
//c*       |--- KEY  ---|- TEXT        ---|NODE_ID = node identification *
//c*                                                                     *
//c*       The above search points to the next array                     *
//c*                                                                     *
//c*       Hash ID2, used to quickly location a nodes' coordinates       *
//c*       |  I*2   | R*8 | R*8 |                                        *
//c*       | NODE_ID|  X  |  Y  |                                        *
//c*       |--------|-----|-----|                                        *
//c*       |- KEY  -|-- TEXT  --|                                        *
//c*                                                                     *
//c*       Which in-turn points to this array.                           *
//c*                                                                     *
//c*       B_TREE ID1 is design to contained detailed information about  *
//c*                the lines intersecting at each node. Its' structure: *
//c*       | R*8 | R*8 |  I*2  |  I*2  | R*8 | I*2 | I*2 |  I*2  | I*4  |*
//c*       |  X  |  Y  |blk/occ| Numem | AZM | GRN | RECN|NODE_ID|XY_ARY|*
//c*       |------------------------------------------------------------|*
//c*       |-----   Key   -----|-----       Text                 -------|*
//c*                                                                     *
short    CurrSrnum, CurrNumem, CurrD2b, CurrLdaloc,CurrNodeNum;


         *d2b = 0e0;
         *node_num = 0;
         lpPoly1->line_num = 1;
         lpPoly1->old_line = -5;
         lpPoly1->i = 0;
         lpPoly1->num_lines = 0;
         lpPoly1->ifirst = 0;
         lpPoly1->ns = 1;
         *srnum = 1;
         *d2b = 0e0;
         *node_num = 0;
//c*       {Here I get the first node on the first line
S5:      lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3,space);
         lpPoly1->ld2b = *d2b;
         CurrSrnum = *srnum;
         CurrD2b = *d2b;
         CurrLdaloc = *ldaloc;
         CurrNodeNum = *node_num;
//c*       {*** srnum {the lrec1 record number of the line
         while (lpPoly1->st == 0)
         {
//d          print*,' '
//d          print*,'found sgr rec ',srnum,' lpPoly1->offset dis = ',d2b
//d          print*,' node num ',node_num
            lpPoly1->lda_loc = *ldaloc;  //save cuz it value gets trampled
            HASHF((int)lpPoly1->id2,&space[10],&lpPoly1->loc);
            if(lpPoly1->loc ==0)
            {  
              GSSiMsgBox(0,"Error in PolyLoad Coord",
                           "PolyAreas Utility",MB_ICONSTOP,0);
//d               Print*,'Error Attempting to find record ',srnum
               *srnum++;
               goto S5;
            }
             HASHG((int)lpPoly1->id2, &space[10], lpPoly1->loc);
//d          print*,'node''s coords are][ x = ',sx,' y = ',sy
            *occ = 0;
            lpPoly1->loc= PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //{the base record
//d          print*,'with ',numem,' lines intersecting there'
            lpPoly1->num_here = *numem;
            if(lpPoly1->old_line != *srnum)
            { // ; //{now on a different line
//d             print*,'found new sgr rec ',srnum
               lpPoly1->old_line = *srnum;
               lpPoly1->num_lines++;
               lpPoly1->ld2b = *d2b;
               lpPoly2 = lpPoly2Base + lpPoly1->ns;
               lpPoly2->from_x_array = *sx;
               lpPoly2->from_y_array = *sy; 
               lpPoly2->valid = TRUE;
               lpPoly2->line_desc[0] = *srnum;  
               lpPoly2->line_desc[1] = 0;
               lpPoly2->line_desc[2] = 0; 
               
               for(lpPoly1->j2 = 1;lpPoly1->j2 <= lpPoly1->num_here; lpPoly1->j2++)
               { 
                 *occ = lpPoly1->j2;
//c*             {find the first occurrence of the line leading out
//c*             {    srnum ==the_rec && the_way ==1
                 lpPoly1->loc= PolyBTGetFirst(lpPoly1->id1,&space[12]);
                 if(lpPoly1->loc == 0)
                 { 
                   if(*the_rec == *srnum && *the_way == 1)
                   { 
                      *x_y_ary_loc = lpPoly1->ns;
                       PolyBTPut (lpPoly1->id1, &space[12]);;
                      goto S20;
                   } 
                 } 
               } //was enddo
//d              print*,


//        Added for Debug Purposes
                GSSiMsgBox(0,"****** Big Time Trouble ******",
                "Poly Load Coords",MB_ICONSTOP,0);
               *occ = 0;
               *node_num = CurrNodeNum ;
                HASHF((int)lpPoly1->id2,&space[10],&lpPoly1->loc);
                HASHG((int)lpPoly1->id2, &space[10], lpPoly1->loc);
                 
               for(lpPoly1->j2 = 1;lpPoly1->j2 <= lpPoly1->num_here; lpPoly1->j2++)
               { 
                 *occ = lpPoly1->j2;
//c*             {find the first occurrence of the line leading out
//c*             {    srnum ==the_rec && the_way ==1
                 lpPoly1->loc= PolyBTGetFirst(lpPoly1->id1,&space[12]);
                 if(lpPoly1->loc == 0)
                 { 
                   if(*the_rec == *srnum && *the_way == 1)
                   { 
                      *x_y_ary_loc = lpPoly1->ns;
                       PolyBTPut (lpPoly1->id1, &space[12]);;
                      goto S20;
                   } 
                 } 
               } //was enddo
                GSSiMsgBox(0,"****** Big Time Trouble ******",
                "Poly Load Coords",MB_ICONSTOP,0);
//     End of debug additions 
             return FALSE;

              goto S20 ; //{get the next line node
            } 
            else 
            {  //{still on the same base line
               for(lpPoly1->j2 = 1;lpPoly1->j2 <= lpPoly1->num_here;lpPoly1->j2++)
               {  
                  *occ = lpPoly1->j2;
//c*                {I find the first occurrence where
//c*                {    srnum ==the_rec and the_way = 0 {leading in
                  lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
                  if(lpPoly1->loc == 0)
                  { 
                     if(*the_rec == *srnum && *the_way == 0)
                     { 
                       *x_y_ary_loc = lpPoly1->ns;
                        lpPoly1->st = PolyBTPut (lpPoly1->id1, &space[12]);
                       goto S10;
                     } 
                  }
               } // end of the while loop
               GSSiMsgBox(0,"*** Unable to find a Continuing Record ",
               "Poly Load Coord Error",MB_ICONSTOP,0); 
               *occ = 0;
               *node_num = CurrNodeNum ;
               HASHF((int)lpPoly1->id2,&space[10],&lpPoly1->loc);
               HASHG((int)lpPoly1->id2, &space[10], lpPoly1->loc);
               for(lpPoly1->j2 = 1;lpPoly1->j2 <= lpPoly1->num_here;lpPoly1->j2++)
               {  
                  *occ = lpPoly1->j2;
//c*                {I find the first occurrence where
//c*                {    srnum ==the_rec and the_way = 0 {leading in
                  lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
                  if(lpPoly1->loc == 0)
                  { 
                     if(*the_rec == *srnum && *the_way == 0)
                     { 
                       *x_y_ary_loc = lpPoly1->ns;
                        lpPoly1->st = PolyBTPut (lpPoly1->id1, &space[12]);
                       goto S10;
                     } 
                  }
               } // end of the while loop
               
              return FALSE; 
               
               
               
               
               
S10:           lpPoly2 = lpPoly2Base + lpPoly1->ns;
               lpPoly2->to_x_array = *sx;
               lpPoly2->to_y_array = *sy;
               lpPoly2->line_desc[0] = *srnum;  
//d              print*,'loaded x_y_ary_loc(',ns,') with'
//d              print*,'    fx = ',from_x_array(ns),
//d    +                '    fy = ',from_y_array(ns)
//d              print*,'    tx = ',to_x_array(ns),
//d    +                '    ty = ',to_y_array(ns)
               lpWork = lpWorkBase + lpPoly1->lda_loc;
               if(lpWork->type == 2)//{straight line
                  lpPoly2->iazm = 0e0;
               else 
               { //{it's a curve
//cd                 print*,'loading a curve delta azimuth'
//cd                 az2 = ltwopi(az8 + py)
//cd                 print*,'curve length = ',d2b - ld2b
//cd                 print*,'curve radius = ',ldarad(lda_loc)
                  lpPoly2->iazm = ((DSIGN(1e0, lpWork->ldalngth) *
                  ( *d2b - lpPoly1->ld2b))  / (lpWork->ldarad));
//cd                 print*,'storing azm change = ',iazm(ns)
               }
               lpPoly1->ns++;
               lpPoly2 = lpPoly2Base + lpPoly1->ns;
               lpPoly2->from_x_array = *sx;
               lpPoly2->from_y_array = *sy;
               lpPoly1->ld2b = *d2b;
 
//c*             {now I start the beginning of the next segment
//c*             {of the current line
              *occ = 0;
//c             {I again find the base record I was just working with
              lpPoly1->loc= PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //{the base record
//d              print*,'with ',numem,' lines intersecting there'
               lpPoly1->num_here = *numem;
//d               print*,'   looking for ',old_line,' and the_way = 1'
               for(lpPoly1->j2 = 1;lpPoly1->j2 <= lpPoly1->num_here;lpPoly1->j2++)
               { 
                  *occ = lpPoly1->j2;
//c*                {I find the first occurrence where
                  lpPoly1->loc= PolyBTGetFirst(lpPoly1->id1,&space[12]);
                  if(lpPoly1->loc==0)
                  { 
                     if(*the_rec == lpPoly1->old_line && *the_way == 1)
                     {
                       *x_y_ary_loc = lpPoly1->ns;
//d                       print*,'*** Success *** Found rec ',the_rec,
//d    +                 ' the way ',the_way
//d                       print*,'it becomes the beginning',
//d    +                        ' of x_y_ary_loc ',ns
                        lpPoly1->st = PolyBTPut (lpPoly1->id1, &space[12]);
                      goto S20;
                     }
                  } 
               } //was enddo
//c              Done with this node. Move on to the next one
            } // 
S20:       lpPoly1->st= PolyBTGetNext (lpPoly1->bid3,space) ; //{get a starting node
//d           print*,'next working with sgr rec ',srnum,' lpPoly1->offset = ',
//d    +             d2b
         }

//c        here I load the endpoint of the very last line
         lpPoly1->num_sides = lpPoly1->ns - 1 ; //{the new number of segments in the coord arrays
//d        print*,' '
//d        print*,' Found ',lpPoly1->num_sides,' sides in this mess'
//d        print*,' '
         for(lpPoly1->i=0;lpPoly1->i <= lpPoly1->num_sides ;lpPoly1->i++)
         { //{for the number of sides
             lpPoly2 = lpPoly2Base + lpPoly1->i;
             lpPoly2->valid = 1;
//d            print*,' fx = ',from_x_array[i],
//d    +       ' fy = ',from_y_array[i],' tx = ',to_x_array[i],
//d    +       ' ty = ',to_y_array[i],' delta_azm = ',iazm[i]
         }
         return TRUE;
}


int  poly_load_lda_line (double x1,double y1,lpWS L1,lpWS L2,
     short itype)
{     

         int    irc ;
         long base_rec;                  
         double  dis11,   dis21,    mx;

         
        irc = 0;
        lpPoly1->rx = x1 ;
        lpPoly1->ry = y1 ;
        *sx = x1 - P_TOL;
        mx = x1 + P_TOL;
        *sy = y1 ;
        lpPoly1->ineed1 = 0;
        lpPoly1->ineed2 = 0;
        *occ = 0; 
        if(mx == -1)
        {
           *srnum = 0;
           *d2b = 0e0; 
           lpPoly1->loc =  PolyBTGetFirst(lpPoly1->bid3,space) ; //{line intersection order B_tree
           while (lpPoly1->loc ==0)
           {  
              lpPoly1->loc = PolyBTGetNext(lpPoly1->bid3,space);
           }
           *sx = 0;
           *sy = 0;
           *occ = 0;
           lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
           while (lpPoly1->loc ==0)
           {  
              lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]);
           }
        }                      
        lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
        if(lpPoly1->loc == BT_NOT_FOUND) goto S1;
        lpPoly1->r8off = LDIST(x1 , y1 , *sx, *sy);
        while (lpPoly1->r8off > P_TOL &&
                  *sx <= mx && lpPoly1->loc == 0)
        { 
           dis11 = *sx;
           dis21 = *sy;         
           lpPoly1->r8off = LDIST(x1 , y1 , *sx, *sy);
           lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]);
        }


S1:     if(lpPoly1->loc == BT_NOT_FOUND ||
          lpPoly1->r8off > P_TOL ) //{need a node 
        {  
          lpPoly1->num_this_node = 0;
          lpPoly1->num_nodes++; 
          *node_num = lpPoly1->num_nodes;
          *this_node = *node_num;
          *sx = x1;
          *sy = y1;  //wipes out *ldaloc
//cc          print*,' '
//cc         print*,'Starting another node ', node_num,' x = ',sx,
//cc     +           ' y = ',sy
           HASHP((int)lpPoly1->id2,&space[10],&lpPoly1->loc);  //{load the node locator hash
       } 
       else 
       {
//c         I work with something close enough to
//c         an existing node to be considered the same location.
//c         SX and SY now contain the values found in the b_tree
//d         print*,'May already have everything at x = ',sx,' y = ',sy
//c         {I might already have these lines in id1 but
//c         {I better make absolutely sure because if][
//c         {line 1 is the base and this node was created when
//c         {it intersected with line 2, now line 2 is intersecting
//c         {line 3 within P_TOL dist of the 1 - 2 intersection
          *occ = 0; 
          lpPoly1->ineed1 = 0;
          lpPoly1->ineed2 = 0;
          lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);

          lpPoly1->num_here = *numem;
          lpPoly1->num_this_node = *numem  ; //{the number of occurrences so far
          *node_num = *this_node  ; //{the original node number
          base_rec = *the_rec;
          for(lpPoly1->i=1; lpPoly1->i<= lpPoly1->num_here;lpPoly1->i++)
          {
             lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]);
             if(*the_rec == L1->ldarec) lpPoly1->ineed1 = 1 ; //{found it
             if(*the_rec ==L2->ldarec)   lpPoly1->ineed2 = 1 ; //{found it
          }
          if(lpPoly1->ineed1 != 0 && lpPoly1->ineed2 != 0)return 0; //{already have all intersections
//cc         print*,'adding another line to node ',this_node,
//ccc     +      ' at x = ', sx, ' sy = ', sy 
                                                          //to insure base_rec is kept current
          if(lpPoly1->ineed1 == 0)goto S10;
          goto S20;
       } // 

//c      {here, I calculate some stats about the line.
S10:   if(L1->type  == 2)
       { 
          dis11   = LDIST(L1->ldax1, L1->lday1, x1 , y1 );
          lpPoly1->dazm[3] = L1->ldaazm ; //{the azimuth of the line at its' beginning and end
       } 
       else 
       {  //{it's a curve
          lpPoly1->dazm[2] = LTWOPI(LGETAZ(L1->ldax2,L1->lday2,x1 ,y1));
//c*                  {dazm(2) = azimuth of line from the RAD at POC
          lpPoly1->sign = DSIGN(1e0, L1->ldalngth); //{direction of curve
//c*        {here I find the difference between the beginning azimuth from
//c*        {the radius to the PC and the one to the intersection point
          lpPoly1->dazm[1] = AZDF(L1->ldaazm,lpPoly1->dazm[2],L1->ldalngth); //{length of curve in radians
          dis11   = fabs(L1->ldarad * lpPoly1->dazm[1] ); //{length along curve to ix point
          if(dis11 > fabs(L1->ldalngth +
                        DSIGN(P_TOL,L1->ldalngth)))
          { 
             dis11 = 0;
             lpPoly1->dazm[3] = LTWOPI(L1->ldaazm - (lpPoly1->sign * HALFPI));
          } 
          else 
             lpPoly1->dazm[3] = LTWOPI(lpPoly1->dazm[2] - ( lpPoly1->sign * HALFPI)) ; //{azm of line at ix point
      }
         
      if(dis11 < P_TOL)
            dis11 = 0;
      else 
      { 
      
       if(fabs(dis11 - fabs(L1->ldalngth)) < P_TOL) 
               dis11 = fabs(L1->ldalngth);
//c       print*,'Distance from beginning of ',i,' is ',dis11
//c*     Here I store some information in the blpPoly1->id3 b_tree 
      }
       *srnum =(short) L1->ldarec;
       *the_rec = (short) L1->ldarec  ; //{places the original sgr rec number
       *ldaloc = (short) L1->ldarec ; //{the location of the sgr record in ldarec[lpPoly1->num_sides]
       *d2b = dis11;

       PolyBTPut(lpPoly1->bid3,space) ; //{line intersection order B_tree
       *sx = x1;
//c                           {into id1_key(31][32)

//c***   {Next I store a line info record going away from the node
//c      {If I'm not already an the end of the line
       if(dis11 !=fabs(L1->ldalngth))
       { 
          lpPoly1->num_this_node++;
//c          print*,'Adding line ',i,' leading OUT-> azm = ', dazm(3)
          *occ = lpPoly1->num_this_node;
          *az8 = lpPoly1->dazm[3];
          *the_way = 1  ; //lda c change{with the grain, line pointing away from intx 
          *x_y_ary_loc = L1->ldarec;
           PolyBTPut(lpPoly1->id1,&space[12]);
       }  


//c      Here I store the line that leads into the node, If I'm not
//c      at the beginning of the line.
       if(dis11 !=0e0)
       {  //{I store a line going toward the node
          lpPoly1->num_this_node++;
          *az8 = LTWOPI(lpPoly1->dazm[3] + PY) ; //{point azm away from node
          *occ = lpPoly1->num_this_node;
//c          print*,'Adding line ',i,' leading ->IN azm = ', az8
          *the_way = 0  ; //lda c change{against the grain, line pointing toward intx
           PolyBTPut(lpPoly1->id1,&space[12]);
       } 



//c*   {********* Now I work with the intersection on the second line
S20:   if(lpPoly1->ineed1 == 0 && lpPoly1->ineed2 == 1) goto S100;
       if(lpPoly1->ineed2 == 1) return 0;
       if(L2->type == 2)
       { 
          dis21   = LDIST(L2->ldax1,L2->lday1, x1 , y1 );
          lpPoly1->dazm[3] = L2->ldaazm;
       } 
       else 
       {  //{it's a curve
//c*        {I find the azimuth from the rad to the intx point
          lpPoly1->dazm[2] = LTWOPI(LGETAZ(L2->ldax2,L2->lday2,x1 ,y1 ));
//c*       {dazm(2) = azimuth of line from the RAD to POC
          lpPoly1->sign    = DSIGN(1e0, L2->ldalngth) ; //{direction of curve
//c*        {here I find the difference between the beginning azimuth from
//c*        {the radius to the PC and from the radius to the intersection point
          lpPoly1->dazm[1] = AZDF(L2->ldaazm,lpPoly1->dazm[2],L2->ldalngth)  ; //{length of curve in radians
          dis21   = fabs(L2->ldarad* lpPoly1->dazm[1] ) ; //{length along curve to ix point
          if(dis21 > fabs(L2->ldalngth + DSIGN(P_TOL, L2->ldalngth)))
          { 
//c*           The intersection point is within the intersecting
//c*           routines parameters, however the intersection point
//c*           is not on the line.  I must find which end of the line
//c*           it really means.  A delta azimuth approaching 2pii
//c*           means the point is near the beginning of the line.
//c             print*,' '
//c             print*,'*** Caution *** Adjusting location of intx'
//c             print*,'on line ',j,' to the beginning of the line'
//c             print*,'computer distance = ',dis21
//c             print*,'length of ',j,' is ',ldalngth(j)

             dis21 = 0 ; //{didn't work with very small overruns
             lpPoly1->dazm[3] = LTWOPI(L2->ldaazm - (lpPoly1->sign *HALFPI));
          } 
          else 
          {
             lpPoly1->dazm[3] = LTWOPI(lpPoly1->dazm[2] -
                                   ( lpPoly1->sign * HALFPI)) ; //{azm of line at ix point
          }                         
       } 
       if(dis21 < P_TOL)
         dis21 = 0;
       else 
       { 
        if(fabs(dis21 - fabs(L2->ldalngth)) < P_TOL)
                        dis21 = fabs(L2->ldalngth);
//c      print*,' '
//c      print*,'Second line intersection information'
//c      print*,'dis from PC to INTX = ',dis21
       }
       *srnum = (short) L2->ldarec;
       *ldaloc = L2->ldarec; //{the location of the sgr record in ldarec[lpPoly1->num_sides]
       *the_rec = (short) L2->ldarec  ; //{ the lrec1 of the sgr file
       *x_y_ary_loc = L2->ldarec;
       *d2b = dis21;

        PolyBTPut(lpPoly1->bid3,space) ; //{line intersection order B_tree
        *sx = x1;


//c      Next I store a line info record going away from the node


//c      If I'm not already an the end of the line
       if(dis21 != fabs(L2->ldalngth))
       {
//c         {I store the line going OUT-> from the node
          lpPoly1->num_this_node++;
          *occ = lpPoly1->num_this_node;
          *az8 = LTWOPI(lpPoly1->dazm[3]);
          *the_way = 1 ; //lda c change {leading out, line pointing away from intx   

//c          print*,'Adding line ',j,' leading OUT-> azm = ',
//c     +    dazm(3)
           PolyBTPut(lpPoly1->id1,&space[12]);
       }  


//c      Here I store the line that leads into the node, If I'm not
//c      at the beginning of the line.
       if(dis21 !=0e0)
       { // ; //{I store a line going toward the node
          lpPoly1->num_this_node++;
          *occ = lpPoly1->num_this_node;
          *az8 = LTWOPI(lpPoly1->dazm[3] + PY) ; //{point azm away from node
          *the_way = 0  ; //lda c change{going against the grain, line pointing toward intx
//c         print*,'Adding line ',j,' leading ->IN azm = ',
//c     +    az8
           PolyBTPut(lpPoly1->id1,&space[12]);
       } // 

S100:  lpNodeArr = lpNodeArrBase + *this_node; 
       lpNodeArr->node_array = lpPoly1->num_this_node;
       *occ = 0;
       *numem = lpPoly1->num_this_node;
       *the_rec = L2->ldarec;
     //  *the_rec = (short) base_rec ; //{****ldarec[i] {the_base record identifier
        PolyBTPut(lpPoly1->id1,&space[12]) ; //{update the node base record
//c       print*,'Node ',node_num,' now contains '
//c     +                   ,num_this_node,' occurrences '
      return 0;
   }




  int PolyFindNextNode(double *xcor,double *ycor,char dir)
  {
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*                                                                    *
//c*       This routine finds the next intersection point using the     *
//c*       current hit_line and the_grain (The direction of travel on   *
//c*       the hit_line). If something is found, lpPoly1->loc  is set 
//        to the  node's base record in the hash ID1 array.             *
//c*                                                                    *
//c*                                                                    *
//c*       author][  Larry Anderson  Dec 87                              *
//c*                                                                    *
//c**********************************************************************
//c*


         *sx = *xcor;
         *sy = *ycor;
         *occ = 0;
         lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
         if(*sx !=*xcor && *sy !=*ycor)       goto S499;
        
         lpPoly1->num_here = *numem;
         if(dir =='T')
         {  //{coming into this node with the grain
            lpPoly1->l = 1  ; //{looking for line coming into intx
         } 
         else 
         {
             lpPoly1->l = 0  ; //{look for line going away from the intx
         } // 
         for (lpPoly1->k = 1;lpPoly1->k <= lpPoly1->num_here;lpPoly1->k++)
         {
             lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]);
            if(lpPoly1->hit_line == *x_y_ary_loc)// && lpPoly1->l == *the_way)
            {
               lpPoly1->curr_azm = *az8  ; //{found the right line
               return 0;
            } // 
         }
S499:     return  -1;
//d         print*,'Searched ID1 and didn''t find ',hit_line,
//d     +   ' the direction = ',idir
}


      
  void PolyFindOcean(long *island_ref,long *ocean_ref)
  {
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//C*       This routine was part of the polygon analysis system.        *
//C*       By the time 'it' gets here, 'it' found a line_desc element   *
//C*       that was not properly tracked. This routine attempts to find *
//C*       the ocean containing this island.                            *
//C*                                                                    *
//C*       If successful, the routine returns the element number of the *
//C*       line on the ocean shore.                                     *
//C*                                                                    *
//c*       if the area bounding the island is found, an exclusion       *
//c*       is added to the ocean and the island's shoreline is given    *
//c*       the ocean's decription.                                      *
//c*                                                                    *
//c*       Modified Dec 93 to look for areas withing areas of the same  *
//c*       type.  Positive areas within positive areas are, by          *
//c*       definition, invalid.  The same goes for negative areas within*
//c*       negative areas. E.G., an island on an island, an exclusion in*
//c*       an exclusion... neither need be.  LDA                           *
//C*                                                                    *
//C*       ARGUMENT DESCRIPTION                                         *
//C*       -------- -----------                                         *
//C*      island_ref i*4     (input/output) the element number of a line*
//C*                         on an island shore                         *
//C*      ocean_ref  I*4     (output)the element number of a line       *
//C*                         on an ocean beach                          *
//C*                                                                    *
//C*       AUTHOR][  LARRY ANDERSON  Dec 87                              *
//C*                                                                    *
//C**********************************************************************
//C*        

       long  possibly = 0;
       char ocean_type, island_type; 
       
       lpPoly1->real_x = 9e9  ;
       if (island_ref <= 0) return;
       for (lpPoly1->l = 1;lpPoly1->l <= lpPoly1->tru_num_areas  ;lpPoly1->l++)
       { //{for each defined area 
          lpGInfo = lpGInfoBase + lpPoly1->l;
          lpAllS = lpAllSBase + lpGInfo->j; //first record of the area in all_sides
          if(lpPoly1->l == *island_ref)
             island_type = lpAllS->all_dir;
          else 
          { 
            if(lpPoly1->hit_x >= lpGInfo->min_x &&
               lpPoly1->hit_x <= lpGInfo->max_x &&
               lpPoly1->hit_y >= lpGInfo->min_y && 
               lpPoly1->hit_y <= lpGInfo->max_y)
             { 
//c         {this area could possible contain the island
             lpPoly1->igood = 0;
             lpPoly1->dis22 = 9e9 ;
             lpPoly1->delta_x = lpPoly1->hit_x + 9e6;
             for (lpPoly1->l = lpGInfo->j;lpPoly1->l < lpGInfo->i_count;lpPoly1->l++)
             { 
               lpAllS = lpAllSBase + lpPoly1->l;
               lpPoly2 = lpPoly2Base + lpAllS->all_sides;
//c*              {for each side that makes up the ocean boundary
               XLL(&lpPoly1->hit_x,          &lpPoly1->hit_y, 
                   &lpPoly1->delta_x,        &lpPoly1->hit_y,
                   &lpPoly2->from_x_array,   &lpPoly2->from_y_array,
                   &lpPoly2->to_x_array,     &lpPoly2->to_y_array,
                   &lpPoly1->x1[0],          &lpPoly1->y1[0], 
                   &lpPoly1->x1[1],          &lpPoly1->y1[1], 
                   &lpPoly1->x1[2],          &lpPoly1->y1[2],
                   &lpPoly1->ineed1,         &lpPoly1->ineed2, 
                   &lpPoly1->ry,             &lpPoly1->j2);
                switch (lpPoly1->j2)
                {
                  case 3:
                  case 5:
                  case 6:
                  {
//c*                 {*** found an intersection ***}
                    lpPoly1->dis21 =  lpPoly1->x1[2] - lpPoly1->hit_x;
                    if(lpPoly1->dis21 == 0)goto S300 ; // adjacent area
                    if(lpPoly1->dis21 < lpPoly1->dis22)
                    { 
                      lpPoly1->dis22 = lpPoly1->dis21;
                      lpPoly1->hit_line = lpPoly1->l;
                    } 
                    lpPoly1->igood++;
                  } 
               } //end of the switch   
             } //enddo;
//c            {all done with this area
             if(fmod(lpPoly1->igood,2) !=0)
             { //{i'm in it
                if(lpPoly1->real_x > lpPoly1->dis22)
                {  //{want the closest
                   lpPoly1->real_x = lpPoly1->dis22; //{containing area
                   possibly = lpGInfo->AreaDesc ;
                   lpAllS = lpAllSBase + lpGInfo->j;
                   ocean_type =  lpAllS->all_dir;;
//d                  print*,'found possible ocean area ',i
//d                  print*,'distance is ', lpPoly1->dis22
                }  
             }  
          }  
S300:   ;
       }//enddo;
       if(possibly != 0 )
       { 
          *ocean_ref = possibly;
//c         {found the ocean the island is in 
          lpGInfo = lpGInfoBase + possibly;
          lpGInfo->num_excl++; //gen_info(ocean_ref,8) = gen_info(ocean_ref,8) + 1;
          _fmemmove(lpPoly1->keyword,ocean_ref,4);
          _fmemmove(&lpPoly1->keyword[5],&lpGInfo->num_excl,2);
          _fmemmove(&lpPoly1->keyword[7],island_ref,4);
           HASHP((int)lpPoly1->id3,lpPoly1->keyword,&lpPoly1->loc);
//d         print*,'adding another exclusion to area ',ocean_ref,' for ',
//d    +    'island ',island_ref
//c         next I change the line_desc of the out_side of the island
//c         to match the ocean shore line description   
          lpGInfo = lpGInfoBase + *island_ref;
          for (lpPoly1->j = lpGInfo->j;lpPoly1->j < lpGInfo->j + lpGInfo->i_count;lpPoly1->j++)
          {//  do j = gen_info(island_ref,7), gen_info(island_ref,7) + gen_info(island_ref,2) - 1;
            lpAllS = lpAllSBase + lpPoly1->j;
            lpPoly2 = lpPoly2Base + lpAllS->all_sides;
            if(lpPoly2->line_desc[1] == -2)
            { 
               lpPoly2->line_desc[1] =(short) *ocean_ref;
            } 
            else 
            {
               lpPoly2->line_desc[2] = (short)*ocean_ref;
            }   
          }//enddo;
          if(ocean_type == island_type)
          { //{found matching type areas
//c*        {the area, island_ref, is not a valid area... but I don't
//c*        {know what to do about it right now!  What I will do is
//c*        {turn the direction of the first line around and let
//c*        {offset_validate_area discover a multi direction area... 
//c*        {letting it handle the situation.
            lpAllS = lpAllSBase + lpGInfo->j;
            if( island_type =='T' )
            { 
               lpAllS->all_dir = 'F'; //all_dir(gen_info(island_ref,7)) = 'F';
            } 
            else 
            {
               lpAllS->all_dir = 'T';// all_dir(gen_info(island_ref,7)) = 'T';
            } 
            lpGInfo->CenY = 0 ; //{I think this keys something  
          }  
       } 
       else 
       { //{didn't find an ocean, perhaps this is another
//c*          {* continent.
          for (lpPoly1->j = lpGInfo->j;lpPoly1->j < lpGInfo->j + lpGInfo->i_count;lpPoly1->j++)
          {// do j = gen_info(island_ref,7), gen_info(island_ref,7) + gen_info(island_ref,2) - 1;
            lpAllS = lpAllSBase + lpPoly1->j;
            lpPoly2 = lpPoly2Base + lpAllS->all_sides;
            if(lpPoly2->line_desc[1] == -2)
            {
               lpPoly2->line_desc[1] = -1;
            } 
            else 
            { 
              if(lpPoly2->line_desc[2] == -2)
              { 
                 lpPoly2->line_desc[2] = -1;
              }
            }    
          }
       } 
       return;
}

}
    int PolyFindStart(BOOL *true,double *xcor,double *ycor)
{    
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       this routine was part of dime_unload_area until i broke it   *
//c*       into more manageable parts. this portion find the hit_line   *
//c*       of an area without using a centroid to begin from.           *
//c*                                                                    *
//c*       The secret to this process is to find a line that is tracked *
//c*       less then twice, then goto Sthe ends of the line and find    *
//c*       the rightest line. If the rightest line is untracked, that's *
//c*       the correct end to proceed to generate an area.              *

     
        lpPoly1->hit_line = 0;
         if(lpPoly1->min_valid_line != 0 && lpPoly1->first)
         { 
            lpPoly2 = lpPoly2Base + lpPoly1->min_valid_line;
            if(lpPoly2->valid)
            { 
                lpPoly1->min_start =lpPoly1->min_valid_line;
                lpPoly1->first = FALSE;
                goto S10;
            }  
         }
S1:      lpPoly1->min_start = 1;
         while (lpPoly1->min_start <= lpPoly1->num_sides)
         { 
           lpPoly2 = lpPoly2Base + lpPoly1->min_start; 
           if((lpPoly2->line_desc[1] ==0 ||
               lpPoly2->line_desc[2] ==0) &&
               lpPoly2->valid) goto S10;
           lpPoly1->min_start++;
         }
         goto S499;
S10:     lpPoly1->hit_line = lpPoly1->min_start;
         lpPoly1->i_start = lpPoly1->hit_line;
         if(lpPoly2->line_desc[2] == 0)
         { //{go off the to end
//c*            {will be working the right side of this line
              lpPoly1->real_x = lpPoly2->from_x_array;
              lpPoly1->real_y = lpPoly2->from_y_array;
              lpPoly1->k  = 1 ; //{going into the next node against the grain
              lpPoly1->first_grain = TRUE;
              lpPoly1->the_grain   = TRUE;
         } 
         else 
         { //{start out from the from end of the line}
//c*            {will be working the left side of the line
              lpPoly1->real_x = lpPoly2->to_x_array;
              lpPoly1->real_y = lpPoly2->to_y_array;
              lpPoly1->k   = 0  ; //{going into the next node with the grain
              lpPoly1->first_grain = FALSE;
              lpPoly1->the_grain   = FALSE;
         } 
//c        Now that I have the coordinates of the endpoint of the
//c        line I want to start with, I find its' statistics in
//c        b_tree ID1
         *occ = 0;
         *sx = lpPoly1->real_x;
         *sy = lpPoly1->real_y;
         lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
         if(*sx != lpPoly1->real_x && *sy != lpPoly1->real_y)
         { 
//d            print*,'unable to locate x = ',sx,' y = ',sy 
            GSSiMsgBox(0,"Unable to find Proper Coordinates in\
            to start the polygon analysis","PolyAreas Error",MB_ICONSTOP,0);
            lpPoly2->valid = FALSE;
            goto S1;
         }  
         lpPoly1->num_here = *numem;
         for (lpPoly1->j = 1;lpPoly1->j <= lpPoly1->num_here;lpPoly1->j++)
         {//   do jump = 1 , num_here ; //{for each line intersecting at this point
             lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]);
            if(*x_y_ary_loc == lpPoly1->hit_line)// && lpPoly1->k  == *the_way)
            {  
               lpPoly1->curr_azm = *az8;
               lpNew = lpNewBase + 1; // the first line
               lpNew->Poly1Loc = *the_rec ; //{the original sgr record
//d              print*,'Found start line ',hit_line
//d              print*,'Its'' original sgr line is ',poly_origs(1)
//d              if(the_grain)then
//d                 print*,'Going with the grain'
//d              else
//d                 print*,'Going against the grain'
//d              endif
               *xcor = *sx;
               *ycor = *sy;
               return 0;
            }  
          }
S499:    *true = FALSE;
         return -1;
 }
double CALCL_TOL(const double *temp_pcx,const double *temp_pcy,
                        const double *ldax2,   const double *lday2,
                        const double *temp_ptx,const double *temp_pty,
                        const double *newdir);
     

int PolyEliminateCoincidence(lpWS L1,lpWS L2,double **x1,double **y1,int otype)
{
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       The lines I and J (ltable elements numbers) were found       *
//c*       to be coincident.  This routine eliminates this coincidence. *
//c*       The elimination of the coincidence is accomplishment by][     *
//c*                                                                    *
//c*        - Determining what should be eliminated                     *
//c*            -- a line totally encompassed by the other              *
//c*            -- that portion of line J encompassed by line I         *
//c*            -- line J if identical to line I                        *
//c*                                                                    *
//c*        - Eliminations are accomplished by adjusting the line       *
//c*          definitons in ldatables.                                  *
//c*                                                                    *
//c*        - If another line (Other than I or J) also intersected      *
//c*          that portion of a line being deleted, the intersection    *
//c*          information placed in the three databases used to track   *
//c*          such things is modified to remove the occurrence of       *
//c*          the intersection.                                         *
//c*                                                                    *

//c*       Arguments                                                    *
//c*       ---------                                                    *
//c*       i     i*4     (input) The Ldatable element of the first line *
//c*       j     i*4     (input) The Ldatable element of the second line*
//c*       x1    r*8(2)  (input) The x coordinate of the intersection   *
//c*       y1    r*8(2)  (input) the y coordinate of the intersection   *
//c*       otype i*4     (input) The type of overlap occurring:         *
//c*                              1 = line 1 coincident for its' entire *
//c*                                  length with line 2.               *
//c*                              2 = line 2 coincident for its' entire *
//c*                                  length with line 1.               *
//c*                              3 = portions of both lines are        *
//c*                                  coincident.                       *
//c*                              4 = line 1 is identical to line 2.    *
//c*                                                                    *
//c*       irc i*4   (output)    -If returncode = 1 adjusted line 2    *
//c*                              so now there is only one hit.         *
//c*                                                                    *
//c*                             -If returncode = 2 using routine      *
//c*                              should stop intersect attempts with   *
//c*                              line J.  Line J invalidated.          *
//c*                                                                    *
//c*                             -If returncode = 3 using routine      *
//c*                              should start all over again.          *
//c*                              Line I invalidated.                   *
//c*                                                                    *
//c*                                                                    *
//c*       Author: Larry Anderson Jan 88                                *
//c*                                                                    *
//c**********************************************************************
//c*

         double temp_pc[2], temp_poc[2], temp_pt[2];
         double *lpPC = temp_pc, *lpPOC = temp_poc, *lpPT = temp_pt;
         
 

      
       int jbeg = 0, iboth;
       *d2b = 0;
 //      goto S(100, 200, 300, 200) otype;
 //      return;

//c*     The routine arrives here if line 1 is totally encompassed in 2
//c*     The remedy is to remove line 1 from further intersecting tests and
//c*     remove any information regarding any intersections that exist.
  switch (otype)
  {
    case 1:
   
       *srnum = L1->ldarec ; //{the line to delete
       *d2b = 0e0;
       L1->ldadesc = 0 ; //{invalidates it from the intersection process
       lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space);
//d      print*,'Eliminating ',ldarec[i]    
       if (lpPoly1->st !=0 || *srnum != L1->ldarec)
       { //{ line 1 is contained inside line 2 {so skip line 1
          return -1; //{not in databases
       }  
//d      print*,'and starting the intersection process over'
        //{no hits found between 1 and 3  {start all over again
       return 3;
    case 2:
    case 4:

//c*     The routine arrives here if line 2 is totally encompassed in 1
//c*     The remedy is to remove line 2 from further intersecting tests and
//c*     remove any information regarding intersections existing with it.

//       irc = 2 ; //{skip over intersect attempts with line J
       *srnum = L2->ldarec ; //{the line to delete
       *d2b = 0;
       L2->ldadesc = 0 ; //{invalidates it from the intersection process
       lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space);
//d      print*,'eliminating ',ldarec(j),' as a coincident line '
       if(lpPoly1->st !=0 || *srnum != L2->ldarec) return 2; //{not in database
       lpPoly1->dis21 = fabs(L2->ldalngth);
       goto S500 ; //{deletes stuff from the databases referring to line J
       break;

//c*     The routine arrives here if a portion of line 2 overlaps
//c*     a portion of line 1.  The remedy removes the overlap portion of
//c*     line 2 from further intersecting tests.  The modification
//c*     consists of changing the definition of line 2 to match the
//c*     endpoint of line 1.
    case 3:
       *srnum = L2->ldarec ; //{the line to modify definition of... and hits on
//d      print*,' Eliminating a portion of ',srnum
//c*     Here I find which end of line J must be redefined.
//c*                  {from bp of line J to first intx point
       lpPoly1->dis11 = LDIST(L2->ldax1, L2->lday1, *x1[0], *y1[0]);
//c*                  {from bp of line J to second intx point
       lpPoly1->dis12 = LDIST(L2->ldax1, L2->lday1, *x1[1], *y1[1]);

       if(lpPoly1->dis11 <=P_TOL)
       {  jbeg = 1  ;
//c*      {the bp of line J is at (x1(1),y1(1))
          //{second intx is new beginning point of line J
          goto S14;
       }   
       if(lpPoly1->dis12 <=P_TOL)
       {  jbeg = 0  ;
//c*        {the bp of line J is at (x1(2),y1(2))
          //{first intx is new beginning point of line J
          goto S14;
       }   
//c*        {I don't have a value for jbeg yet it means the endpoint
//c*        {of the line must be one of the intersect points 
           lpPoly1->sign = fabs(L2->ldalngth); 
           LOL8(&L2->ldax1, &L2->lday1, &L2->ldax2, &L2->lday2, &L2->ldalngth,
                &lpPoly1->sign, &lpPoly1->hit_x, &lpPoly1->hit_y, L2->type);
//c*        {I just got the endpoint coordinates of line J.  I then find
//c*        {which intx point its closest to.
          lpPoly1->dis11 = LDIST (lpPoly1->hit_x, lpPoly1->hit_y, *x1[0], *y1[0]);
          lpPoly1->dis12 = LDIST (lpPoly1->hit_x, lpPoly1->hit_y, *x1[1], *y1[1]);
          if(lpPoly1->dis11 <= lpPoly1->dis12)
             jbeg = 3   ; //{means ep of 2 is closest to first intx point, at x1(1), y1(1)
          else 
             jbeg = 2   ; //{means ep of 2 is closest to second intx point at x1(2), y1(2)
       }  

//c*     Here I determine the chunk of line 2 to be eliminated
S14:   if(jbeg < 2)
       { //{delete portion of the beginning of the line
//d        print*,'Eliminating the beginning of ',srnum
         if(L2->type == 2)
         { 
            L2->ldax1 = *x1[jbeg]   ; //{move the bp of line J to x1(1), y1(1)
            L2->lday1 = *y1[jbeg];
            lpPoly1->drotlen = LDIST(L2->ldax1, L2->lday1, L2->ldax2, L2->lday2);
            lpPoly1->dis21 = L2->ldalngth - lpPoly1->drotlen ;
            L2->ldalngth = lpPoly1->drotlen;//{reduce the length of the line
         } 
         else 
         {  //{it's a curve
            //{GET 3 PT CURVE DEFINITION
            Get3PtsCurve(L2,lpPC,lpPOC,lpPT)  ;
            temp_pc[0] = *x1[jbeg];
            temp_pc[1] = *y1[jbeg];
            if (L2->ldalngth >= 0 )
               { lpPoly1->newdir = 1.0;}
            else 
               { lpPoly1->newdir = -1.0;}
            lpPoly1->drotlen = CALCL_TOL(&temp_pc[0],&temp_pc[1],&L2->ldax2,
                      &L2->lday2,&temp_pt[0],&temp_pt[1],&lpPoly1->newdir);
            L2->ldax1 = *x1[jbeg];
            L2->lday1 = *y1[jbeg]   ;
            lpPoly1->dis21 = L2->ldalngth - lpPoly1->drotlen;
            L2->ldalngth = lpPoly1->drotlen;
//c           {next I calc the azimuth of the line from the rad to the PC
            L2->ldaazm = LTWOPI(LGETAZ(L2->ldax2,L2->lday2,L2->ldax1,L2->lday1));
            lpPoly1->sign = DSIGN(1e0, L2->ldalngth) ; //{direction of curve
         }  
         if(jbeg == 1)
         { 
            *x1[0] = *x1[1];
            *y1[0] = *y1[1];
         }  
         *d2b = 0e0  ; //{start deletion at beginning and go dis21 along line
       } 
       else // j >= 2
       {  //{just gotta shorten the line
//d        print*,'Eliminating the end of ',srnum
         jbeg =+ 2;
         if(L2->type  == 2)
         {  //got a straight line
            L2->ldax2 = *x1[jbeg]   ; //{move the bp of line J to x1(jbeg), y1(jbeg)
            L2->lday2 = *y1[jbeg];
            *x1[0] = L2->ldax2;
            *y1[0] = L2->lday2;
            lpPoly1->drotlen = LDIST(L2->ldax1, L2->lday1, L2->ldax2, L2->lday2);
            lpPoly1->dis21 = L2->ldalngth - lpPoly1->drotlen ;
            L2->ldalngth = lpPoly1->drotlen;
            *d2b = lpPoly1->dis21  ; //{look from this point to the endpoint of line J
         } 
         else 
         { //{it's a curve
            Get3PtsCurve(L2,lpPC,lpPOC,lpPT);
            temp_pt[0] = *x1[jbeg];
            temp_pt[1] = *y1[jbeg];
            if (L2->ldalngth >= 0)
            { lpPoly1->newdir = 1.0;} 
            else 
            { lpPoly1->newdir = -1.0;} 
            L2->ldalngth = CALCL_TOL(&temp_pc[0],&temp_pc[1],&L2->ldax2,
                         &L2->lday2,&temp_pt[0],&temp_pt[1],&lpPoly1->newdir);
            lpPoly1->dis21 = L2->ldalngth ;
            lpPoly1->sign = DSIGN(1e0, L2->ldalngth) ; //{direction of curve
            if(jbeg ==2 )
            {  //{using the second intx point
               *x1[0] = *x1[1];
               *y1[0] = *y1[1];
            }  
         } 
//c*       {Next I reduce the length of the line
         jbeg =+ 2;
       }  


//c*     {here I see if any other lines hit the line segment
//C*     {I just deleted.  If I find an intersection point on
//c*     {the deleted segment of the line with the adjusted definition
//c*     {I find the intx details in the id1 b_tree and delete
//c*     {the appropriate reference(s) to the line segment being deleted.

S500:  lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space);
S501:  if(lpPoly1->st != 0 )return 0;
       lpPoly1->nldaloc = *ldaloc;
       if(*srnum != L2->ldarec)return 0; //{line not in the databases.

       if(otype == 2 || otype == 4)
       { //{delete the whole line
           iboth = 2;
       } 
       else 
       { 
        if(jbeg <=2)
        {  //{working with the beginning of the line
           if(*d2b-P_TOL > lpPoly1->dis21) goto S600 ; //{beyond deleted part
           if(*d2b+P_TOL > lpPoly1->dis21)
             { iboth = 1 ;} //{delete only incoming line
           else 
             { iboth = 2 ;} //{delete both segment of the line
        } 
        else 
        {  //{working with the end of the line
           if(*d2b-P_TOL <= lpPoly1->dis21) return 0; //{not in deleted part
           if(*d2b+P_TOL <lpPoly1->dis21)
             { iboth = 0 ;} //{delete only outgoing line
           else 
             { iboth = 2 ;} //{delete both segment of the line
        } 
       }
//d      print*,'Found another intersection on line ',srnum,
//d    +        ' that must be removed from bid3'
       lpPoly1->last_node = *node_num;
       lpPoly1->last_num = *srnum;
        HASHF((int)lpPoly1->id2,&space[10],&lpPoly1->loc) ; //{find the node in the locator hash
       if(lpPoly1->loc == 0)
       { 
          GSSiMsgBox(0,"*** Error *** Unable to locate Hash item",
                     "Poly Eliminate Coincidence", MB_ICONSTOP,0);
          goto S1000;
       } 
        HASHG((int)lpPoly1->id2,&space[10],lpPoly1->loc) ; //{gets the nodes' coordinates

       *occ = 0;
       lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
       lpPoly1->num_here = *numem;
       lpPoly1->base_rec = *the_rec  ; //{the first sgr record number found here
       lpPoly1->shift = FALSE;
       lpPoly1->igood = 0;
       lpPoly1->iskip = 0;
       *occ = 1;
       while (*occ <=lpPoly1->num_here)
       {
S540:     lpPoly1->st = PolyBTGetFirst (lpPoly1->id1, &space[12]);
          if(*srnum == *the_rec && (iboth == 2 || iboth == *the_way))
          { 
             lpPoly1->shift = TRUE;
             lpPoly1->iskip++  ; //{skip over this one
             lpPoly1->inext = *occ + 1;
             if(lpPoly1->inext > lpPoly1->num_here) goto S560;
             *occ = lpPoly1->inext;
             goto S540;
          }  
          if(lpPoly1->shift)
          { 
//d            print*,'Moving occ ',occ ,' into occ ',igood + 1
             *occ = lpPoly1->igood + 1;
             PolyBTPut(lpPoly1->id1,&space[12]) ; //{i now inplace of i + 1
          }  
          lpPoly1->igood++;
          *occ = lpPoly1->igood + lpPoly1->iskip + 1;
       }

S560:   for(lpPoly1->l = lpPoly1->num_here; lpPoly1->l >= lpPoly1->igood + 1;lpPoly1->l--)
        {  
          *occ = (short)lpPoly1->l;
        lpPoly1->st = BT_DELETE(lpPoly1->id1,&space[12],&space[30],FALSE);
        }
       *occ = 0;
       lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //{get the base rec
       if(lpPoly1->loc !=0)goto S1000;
       if(*numem == lpPoly1->igood)
       {  //{didn't find what i was looking for
//c*         print*,'Trouble in poly_eliminate_coincidence'
//c*         print*,'Unable to locate sgr record ',srnum,' at X = ',sx,
//c*     +          ' Y = ',sy
         return 0;
       } 
       *numem = lpPoly1->igood  ; //{update the occurrence of this node
       lpNodeArr = lpNodeArrBase + *this_node;
       lpNodeArr->node_array = *numem;
//d      print*,'reducing node ',this_node,' to ',numem,' occurrences'

       *the_rec = lpPoly1->base_rec ; //{the_base record identifier
        PolyBTPut(lpPoly1->id1,&space[12]) ; //{update the node base record

//c*****  gotta update the bid3 b_tree table too
       if(iboth ==2 || //{both lines segments deleted here
         (*d2b <lpPoly1->dis21 && jbeg <=2) ||
         (*d2b <L2->ldalngth && jbeg > 2))
         lpPoly1->st = BT_DELETE(lpPoly1->bid3,space,&space[10],FALSE);

       if(iboth <2)
       { 
//c*        {not deleting the whole line and only one segment
//c*        {of it here at this node
//c*        {must update the record by adjusting the offset distance
          *ldaloc = lpPoly1->nldaloc;
          *d2b = 0e0 ; //{ d2b - dis21
           PolyBTPut(lpPoly1->bid3,space);
       }  

//c*     {gotta look for more hits in the deleted line segment
S570:  if(otype ==2 || otype ==4)
       { 
//c         {working on deleting the whole line
          *d2b = 0e0;
          lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space) ; //{may have to use get_ first
       } 
       else 
       {
           lpPoly1->st = PolyBTGetNext (lpPoly1->bid3,space) ; //{may have to use get_ first
       }  
       goto S501;


//c*    {When deleting the beginning of a line is complete I must
//c*    {then adjust the offset distances for all other intersections
//c*    {with the line stored in the bid3 b_tree.  I subtract the
//c*    {value currently held by dis21... delete the old item...
//c*    {subtract the offset distance change... and add a new item
//c*    {to bid3.
S600: lpPoly1->st= BT_DELETE(lpPoly1->bid3,space,&space[12],FALSE);
      *d2b = *d2b - lpPoly1->dis21;
      *ldaloc = lpPoly1->nldaloc;
      if(otype !=2 && otype !=4)  PolyBTPut(lpPoly1->bid3,space);
      goto S570;


S1000: ;

      return 0;
  }

int PolyEliminateDeadends(void)
{
  BOOL the_end;
  long Repetitions;
  int iway, examining, last_loc, line_num, i, j, rc;
  double tf ,tff, TheSign, ChoppedBeginning; 
  


//beginning of debug stuff
FILE *f1;
char mess[64], *cptr, *rptr;
int dp,Sign, debug = 0; 
  if(debug == 1)
  {
      f1 = fopen("c:\\debug.txt","wt");  
      *srnum = 0;
      *d2b = 0e0 ;
       lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space); 
       fwrite("Node d2b srnum\r\n",1,16,f1);
        while (lpPoly1->st == 0)
        { 
           cptr = mess;
           itoa(*node_num,cptr,10);
           cptr += _fstrlen(cptr);
           *cptr++ = ' ';
           *cptr++ = ' ';
           rptr = fcvt(*d2b,7,&dp,&Sign);
           if(dp < 0)
           {
             _fstrncpy(cptr,"0.",2);
             cptr += 2;
             _fstrncpy(cptr,rptr,7+dp);
             cptr += 7+dp;
           }
           else
           {
             _fstrncpy(cptr,rptr,dp);
             cptr += dp;
             *cptr++ = '.';
             _fstrncpy(cptr,rptr+dp,2);
             cptr += 2;
           }  
           *cptr++ = ' ';
           *cptr++ = ' ';
           itoa(*srnum,cptr,10);
           cptr += _fstrlen(cptr);
           *cptr++ = ' ';
           *cptr++ = '\r';
           *cptr++ = '\n';
           *cptr = '\0';
           cptr = mess;
           fwrite(cptr,1,_fstrlen(mess),f1);
           lpPoly1->st = PolyBTGetNext (lpPoly1->bid3, space);
           lpPoly1->i_count++;
        }       
      fclose(f1);
      *sx = 0.0;
      *sy = 0.0;
      *occ = 0;
   lpPoly1->st = PolyBTGetFirst (lpPoly1->id1, &space[12]);
   while (lpPoly1->st == 0)
      {
         lpPoly1->st = PolyBTGetNext (lpPoly1->id1, &space[12]);
      }
  }
//  end of the debug stuff
                           
      Repetitions = 0;
      *srnum = 0;
      *d2b = 0e0 ;
      the_end = FALSE;
//cc    Here I count the hits on this particular line
//c     {for all lines in bid3 via the intersection routines  
S1:  lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space);
    rc =  LowestSrnum = *srnum + 1;
S2:   while (lpPoly1->st == 0)
      {
        lpPoly1->last_num = *srnum ;
        tf = *d2b; 
        
        if(Repetitions++ >= 20000)return 1;
   /*     if(*srnum == 20)
        {
          i = 0;
        }   */
        lpNodeArr = lpNodeArrBase + *node_num;
S3:     if(lpNodeArr->node_array == 1)
        {         
//c        a deadend node  //0 = inbound, 1 = outbound

S4:        iway = 1; //{outbound 
           if(*node_num >= 160)
           {
             iway = 1;
           }  
           last_loc = *srnum;
           tf = *d2b; 
           lpPoly1->ineed1 = DeleteNode(iway); 
           if(LowestSrnum < rc)rc  = LowestSrnum;
           iway = 0; //{outbound 
           *srnum = last_loc;
           *d2b = tf;
           lpPoly1->ineed2 = DeleteNode(iway); 
           if(LowestSrnum < rc)rc  = LowestSrnum; 
           if(lpPoly1->ineed1 == -1 && lpPoly1->ineed2 == -1)
           { //Didn't find the node
              *srnum = last_loc;
              *d2b = tf;
              lpPoly1->st=BT_DELETE(lpPoly1->bid3,space,&space[10],FALSE);
              // in desperation I force a delete
           }
       //    *srnum = last_loc;
        ///   *d2b = tf;
       //    lpPoly1->st=BT_DELETE(lpPoly1->bid3,space,&space[10],FALSE);
           *srnum = rc;
           *d2b = 0e0 ; //{the beginning of the line 
           *srnum = rc - 1;
           goto S1 ; //{done with this record
        }
           
           
   //   *srnum = 0;
   //   *d2b = 0e0 ;
      the_end = FALSE;
//cc    Here I count the hits on this particular line
//c     {for all lines in bid3 via the intersection routines  
           
           
           
           
//      now I count how many times this line was intersected          
        lpPoly1->i_count  = -1;
        while (lpPoly1->st == 0 && lpPoly1->last_num == *srnum)
        {  
           tf = *d2b;
           lpPoly1->st = PolyBTGetNext (lpPoly1->bid3, space);
           lpPoly1->i_count++;
        }
        //back to the beginning of this line
        if(lpPoly1->st != 0)the_end = TRUE;
        *srnum = lpPoly1->last_num;
        *d2b = 0e0 ;
        lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space);
        last_loc = *srnum;

        if(lpPoly1->i_count == 0)
        { 
//c         this line was intersected only once... it therefore is not
//c         part of any area making perimeter and should be deleted.
//c         Next I retrieve the bid3 item. now I have it and the node_num
           lpWork = lpWorkBase + last_loc; 
           lpWork->valid = FALSE;
           goto S4;
        }  
        ChoppedBeginning = 0e0;
        if(*d2b > P_TOL)
        {  
//ccc       we get here if a line's first intersection point is;
//ccc       is not at the exact endpoint of the line;
            tf = *d2b;
            ChoppedBeginning = *d2b;
            iway = 0 ; //{inbound 
            last_loc = *srnum;
            j = *ldaloc;
            lpPoly1->st = DeleteNode(iway);
            if(LowestSrnum < rc)rc  = LowestSrnum;
            *ldaloc = j;
          switch (lpPoly1->st)
          {
             case 0:
               last_loc = *srnum;
               *d2b = 0e0;
               lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space);
               i = 0;
               while(last_loc == *srnum && lpPoly1->st == 0)
               {// I move everything up closer to the end
                 xxx[i] = *d2b -  tf; 
                 NodeIds[i] = *node_num;
                 LdaLocs[i++] = *ldaloc;
                 lpPoly1->st=BT_DELETE(lpPoly1->bid3,space,&space[10],FALSE);
                 lpPoly1->st = PolyBTGetNext (lpPoly1->bid3, space);
               }
               *srnum = last_loc;
               *d2b = 0e0;
               for(j = 0; j < i; j++)
               {// I move everything up closer to the end
                  *d2b = xxx[j];
                  tf = *d2b;
                  *node_num = NodeIds[j];
                  *ldaloc = LdaLocs[j] ;
                  PolyBTPut(lpPoly1->bid3,space) ; //{i now inplace of i + 1
               }
           }     //end of the switch   
        } 

//c*         ; //{Now I check out the "EP" end of the line
S30:       *d2b = tf ; 
           *srnum =lpPoly1->last_num  ;
           lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space);

         if(lpPoly1->st !=0)the_end = TRUE;

//c*      now we check the endpoint of the line
//c*
        lpNodeArr = lpNodeArrBase + *node_num;
        if(lpNodeArr->node_array == 1) goto S3;
        lpWork = lpWorkBase + *ldaloc;
        if( fabs(lpWork->ldalngth) > *d2b + ChoppedBeginning + P_TOL )
        {  
//ccc       we get here if a line's last intersection point is
//ccc       is not at the exact endpoint of the line 
           TheSign = (lpWork->ldalngth < 0e0 ?  -1.0 : 1.0);
           tf = fabs(lpWork->ldalngth) - *d2b;
           tff = lpWork->ldalngth - (TheSign * tf);
           iway = 1 ; //{outbound 
           last_loc = *srnum;
           tf = *d2b; 
           j = *ldaloc;
           lpPoly1->st = DeleteNode(iway) ;
           if(LowestSrnum < rc)rc  = LowestSrnum;
           *srnum = last_loc;
           *d2b = tf;
           lpPoly1->st=BT_DELETE(lpPoly1->bid3,space,&space[10],FALSE);
           *d2b = tff; 
           *ldaloc = j;
           *srnum = last_loc;
           PolyBTPut(lpPoly1->bid3,space) ; //{i now inplace of i + 1
           lpWork->ldalngth =   (TheSign * tff);   
         } 
         *d2b = 0e0;
         if(LowestSrnum < rc)
            *srnum = LowestSrnum;
         else   
            *srnum = rc; 
         if( the_end) goto S20 ;  
         goto S1;
//c*        recycle back and get the next line.
     
      }   

     
S20:;  *srnum = 0;
  if(debug == 1)
  {
      f1 = fopen("c:\\debug.txt","at");  
      *srnum = 0;
      *d2b = 0e0 ;
       lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space); 
       fwrite(" \r\n",1,3,f1);
       fwrite("  After\r\n",1,9,f1);
       fwrite("Node d2b srnum\r\n",1,16,f1);
        while (lpPoly1->st == 0)
        { 
           cptr = mess;
           itoa(*node_num,cptr,10);
           cptr += _fstrlen(cptr);
           *cptr++ = ' ';
           *cptr++ = ' ';
           rptr = fcvt(*d2b,2,&dp,&Sign);
           if(dp < 0)
           {
             _fstrncpy(cptr,"0.",2);
             cptr += 2;
             _fstrncpy(cptr,rptr,7+dp);
             cptr += 7+dp;
           }
           else
           {
             _fstrncpy(cptr,rptr,dp);
             cptr += dp;
             *cptr++ = '.';
             _fstrncpy(cptr,rptr+dp,2);
             cptr += 2;
           }  
           *cptr++ = ' ';
           *cptr++ = ' ';
           itoa(*srnum,cptr,10);
           cptr += _fstrlen(cptr);
           *cptr++ = ' ';
           *cptr++ = '\r';
           *cptr++ = '\n';
           *cptr = '\0';
           cptr = mess;
           fwrite(cptr,1,_fstrlen(mess),f1);
           lpPoly1->st = PolyBTGetNext (lpPoly1->bid3, space);
           lpPoly1->i_count++;
        }       
      fclose(f1);

    //  lpNodeArr = lpNodeArrBase;
   //   for (i = 0;i<lpPoly1->num_nodes;i++)
   //   {
   //      lpNodeArr = lpNodeArrBase + i;
   //   }
   //   *sx = 0.0;
   //   *sy = 0.0;
   //   *occ = 0;
   //   lpPoly1->st = PolyBTGetFirst (lpPoly1->id1, &space[12]);
   //   while (lpPoly1->st == 0)
   //   {
   //      lpPoly1->st = PolyBTGetNext (lpPoly1->id1, &space[12]);
   //   }    

  }     


   return 0;
                         
  }


//c***********************************************************
 int  DeleteNode(int iway)
 {  //all the information about the node to be deleted in in
    // char space[52], which I reference using the variable
    // assigned to each value needed
    int last_loc, StartedWith, NextSrnum, i, DeletingNode; 
    short  NumWhoHere, debug;
    char   Mess[8], Comment[64];
    FILE *f1; 
    double tf, NodeX, NodeY; 
           Here[0] = *srnum;
           Here[1] = 0;
           Here[2] = 0;
           
             
           debug = 0;   //change this to zero for production
           
           
           NextSrnum = *srnum;
           tf = *d2b; 
           i = *node_num; 
   if(*node_num >= 164)
   {  //Case of the disappearing node number 165
      // You'll get here twice for each node because I must
      // delete the inbound and outbound occurrence of the line
      // separately. The order of the nodes in id1 
      // is 165, 164, 61 cuz
      // 165 X = 651898.32
      // 165 Y = 231557.81
      
      // 164 X = 651899.06
      // 164 Y = 231559.29
      
      //  61 X = 651939.21
      //  61 Y = 234208.21  
      
           *node_num = 165;
           HASHF((int)lpPoly1->id2,&space[10],&lpPoly1->loc); //{gets the nodes coords   
           if(lpPoly1->loc ==0)return -1;
           HASHG((int)lpPoly1->id2,&space[10],lpPoly1->loc); //{gets the nodes coords 
           *occ = 0;    
           lpPoly1->st = PolyBTGetFirst (lpPoly1->id1, &space[12]);
     //You can see that its here before I delete node 164
           for(lpPoly1->l = 0; lpPoly1->l < 8;lpPoly1->l++)
           { //the key to id1 is *sx, *sy, *occ  in that order
             //this lets you look at the order of the next eight
             //records in the b tree
              lpPoly1->st = PolyBTGetNext (lpPoly1->id1, &space[12]);
           }
   }        
           *node_num = i;
           HASHF((int)lpPoly1->id2,&space[10],&lpPoly1->loc); //{gets the nodes coords   
           if(lpPoly1->loc ==0)return -1;
           HASHG((int)lpPoly1->id2,&space[10],lpPoly1->loc); //{gets the nodes coords 


           if(debug == 1)
           {
              itoa(*node_num, Mess, 10); 
              _fstrcpy(Comment," Deleting Node ");
              _fstrcat(Comment,Mess);
              itoa(*srnum,Mess,10);
              _fstrcat(Comment," With Srnum ");
              _fstrcat(Comment,Mess);
              itoa(iway,Mess,10);
              _fstrcat(Comment," Going ");
              _fstrcat(Comment,Mess);
              GSSiMsgBox(0,Comment,"Delete Node",MB_ICONINFORMATION,0);
           }      
           *occ = 0;
           lpPoly1->st = PolyBTGetFirst(lpPoly1->id1, &space[12]);
          if(*node_num !=*this_node || *occ != 0) 
          {
        //     MessageBox(0," *** Error *** Unable to find right node",
        //       "In PolyEliminateDeadends",MB_ICONINFORMATION)  ;
            return -1;
          }              
//c         ok, I'm at the intersection where I want to delete all
//c         references to srnum
          lpNodeArr = lpNodeArrBase + *node_num;
          DeletingNode = *node_num;
          NodeX = *sx;
          NodeY = *sy;
          NumWhoHere = 1;  
          lpPoly1->num_here = *numem;
      for( lpPoly1->l = 1; lpPoly1->l <= lpPoly1->num_here  ;lpPoly1->l++)
      {
             *occ = (short)lpPoly1->l;
             lpPoly1->st = PolyBTGetFirst (lpPoly1->id1, &space[12]);
             if(lpPoly1->st !=0)goto S100; 
             
                 if(NumWhoHere > 2)goto S22; 
                 for(i = 0; i < NumWhoHere; i++)
                 {
                   if(*the_rec == Here[i]) goto S22;
                 }
                 Here[NumWhoHere++] = *the_rec;

S22:      if(*srnum == *the_rec && iway == *the_way)
          {
               lpPoly1->i_count = *occ  ; //{ found the line I wish to delete
               while (lpPoly1->i_count < lpPoly1->num_here)
               {
//c*               for the other occurrences at this intersection
//c*               with occurrence numbers greater then the one
//c*               I want deleted, I adjust their occ numbers
                 *occ = (short) (lpPoly1->i_count + 1);
                 lpPoly1->st = PolyBTGetFirst(lpPoly1->id1,&space[12]);
                 if(lpPoly1->st !=0)goto S100 ;
                 if(NumWhoHere > 2)goto S23; 
                 for(i = 0; i < NumWhoHere; i++)
                 {
                   if(*the_rec == Here[i]) goto S23;
                 }
                 Here[NumWhoHere++] = *the_rec;

S23:             if(*occ > 0)*occ = *occ - 1;
                 PolyBTPut(lpPoly1->id1,&space[12]) ;
                 lpPoly1->i_count++;
               } 
               
               
               if(lpNodeArr->node_array > 0)lpNodeArr->node_array--;
               *occ = lpPoly1->num_here  ; //{the last occurrence of this node
               lpPoly1->st = PolyBTGetFirst(lpPoly1->id1,&space[12]);
               if(lpPoly1->st != 0)goto S100 ;
               if(NumWhoHere > 2)goto S24; 
               for(i = 0; i < NumWhoHere; i++)
               {
                   if(*the_rec == Here[i]) goto S24;
               }
               Here[NumWhoHere++] = *the_rec;
              //here I delete the highest *occ record
S24:           lpPoly1->st= BT_DELETE (lpPoly1->id1,&space[12],
                              &space[30],FALSE);
               if(Here[1] != 0 && Here[1] < Here[0])  LowestSrnum = Here[1];
               if(Here[1] == 0)LowestSrnum = Here[0];  
          switch (lpNodeArr->node_array)
            {  //based on the number of other lines at this node
               case 0:              
               //{delete the base record too
                  *occ = 0;
                   lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12],
                              &space[30],FALSE);
                   lpPoly1->st=BT_DELETE(lpPoly1->bid3,space,&space[10],FALSE);
                   break;
 
               case 1:
                  *d2b = tf;
                  *srnum = Here[0];
                  lpPoly1->st=BT_DELETE(lpPoly1->bid3,space,&space[10],FALSE);

               default: //two or more lines appear here
                *occ = 0;
                *sx = NodeX;
                *sy = NodeY;
                lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //{get the base rec
                if(*sx != NodeX || *sy != NodeY || *occ != 0)// we got trouble
                {
                    GSSiMsgBox(0,"Unable to find correct item in B Tree ID1",
                    "Delete Node",MB_ICONINFORMATION,0);
                }
                if(lpPoly1->loc !=0)goto S100;
                if(*numem > 0) *numem = *numem - 1  ; //{update the occurrence of this node
                PolyBTPut(lpPoly1->id1,&space[12]) ; //{update the base record
            }//end of the switch   
//c*                next I determine the need for updating the bid3 db
//c*               If no occurrence of srnum remain at this_node location
//c*               I delete the matching reference from the bid3 b_tree
                 lpPoly1->num_here = *numem;
                 for(lpPoly1->l = 1;lpPoly1->l <= lpPoly1->num_here;lpPoly1->l++)
                 {
                   lpPoly1->st = PolyBTGetNext (lpPoly1->id1,&space[12]);
                   if(lpPoly1->st != 0)goto S100;
                   if(*srnum == *the_rec)goto S20; //another occurrence of the line
                 }
                 lpWork = lpWorkBase + *srnum; 
                 lpWork->valid = FALSE;
                 lpPoly1->st= BT_DELETE(lpPoly1->bid3,space,&space[10],FALSE) ;
                 if(NumWhoHere == 2)// I delete the node
                 { // only one other line at this node
                   *sx = NodeX;
                   *sy = NodeY;
                   for(lpPoly1->l = 0;lpPoly1->l <= lpPoly1->num_here;lpPoly1->l++)
                   {
                     *occ = (short) lpPoly1->l;
                     lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12], &space[30],FALSE);
                   } 
                   *d2b = 0e0;
                   lpNodeArr->node_array = 0;
                   //now I eliminate the bid3 record for the other line
                   if(Here[0] >  Here[1] )LowestSrnum = Here[1];
                   *srnum = Here[1];  
                   //I search the other line at this node
                   // for the right intersection with it.
                   lpPoly1->st = PolyBTGetFirst (lpPoly1->bid3, space);
                   while(*srnum == Here[1] && lpPoly1->st == 0)
                   {
                     if(*node_num == DeletingNode)//found it
                     {
                        lpWork = lpWorkBase + *ldaloc; 
                        lpWork->valid = FALSE;
                        lpPoly1->st= BT_DELETE(lpPoly1->bid3,space,&space[10],FALSE) ;
                        break;
                     }
                     lpPoly1->st = PolyBTGetNext (lpPoly1->bid3, space);
                   }
                 } 
   if(*node_num >= 164)
   {  // By the time you get here node 164 should be gone from
      // id1.

           *node_num = 165;
           HASHF((int)lpPoly1->id2,&space[10],&lpPoly1->loc); //{gets the nodes coords   
           if(lpPoly1->loc ==0)return -1;
           HASHG((int)lpPoly1->id2,&space[10],lpPoly1->loc); //{gets the nodes coords 
           *occ = 0;    
           lpPoly1->st = PolyBTGetFirst (lpPoly1->id1, &space[12]);
       // as you can see, although I deleted node 164 out of id1
       // node 165 disappeared too.
           for(lpPoly1->l = 0; lpPoly1->l < 8;lpPoly1->l++)
           {
              lpPoly1->st = PolyBTGetNext (lpPoly1->id1, &space[12]);
           }
   }        
   
S20:      return 0;
          }
        }  
         
     
  
S100:      return -1;
  }
  
  
   int PolyPurge (void)
{   
//c***********************************************************************
//c*                                                                     *
//c*        Routine Summary                                              *
//c*        ------- -------                                              *
//c*                                                                     *
//c*        This routine, part of poly, finds all lines                  *
//c*        that are not connected to another line on both its ends.     *
//c*        These lines will be ignored by get_next_line                 *
//c*        to prevent trouble caused by tracking down                   *
//c*        dead end lines.                                              *
//c*                                                                     *
//c*        the matching 'valid' array element is set to 1               *
//c*        if the line is invalid, i.e., not connected to something     *
//c*        at both its' endpoints.                                      *
//c*                                                                     *
//c*        Author][ Larry Anderson Dec 87                                *
//c*                                                                     *
//c***********************************************************************
short	rtn=0;
      
        goto S20; 
        
S10:     PolyEliminator((short)lpPoly1->l)  ; //{found a node with one occurrence. 

S20:   for( lpPoly1->l = 1 ; lpPoly1->l<= lpPoly1->num_nodes ;lpPoly1->l++)
       { //{for each node, count the intersects there.
          lpNodeArr = lpNodeArrBase + lpPoly1->l;
          if(lpNodeArr->node_array == 1)
          { 
//d            print*,'Found a dead end. Node_number = ',j
             goto S10 ; //{found one with only one
          }  
       }
       for(lpPoly1->l = 1; lpPoly1->l <= lpPoly1->num_sides; lpPoly1->l++)
       { lpPoly2 = lpPoly2Base + lpPoly1->l;
         if(lpPoly2->valid)
         	return 1; //{found something still valid
       }
       return rtn;
  }
 int  PolyEliminator(int ibad)
{           
//c***********************************************************************
//c*                                                                     *
//c*     Called by poly_purge.ftn, part of /umsc/util/poly, which        *
//c*     strings sides together. This routine eliminates                 *
//c*     nodes, and lines, whenever just one line ends                   *
//c*     at a particular node.                                           *
//c*                                                                     *
//c*     To get here, poly_purge seached the node_array(1][num_nodes)     *
//c*     and found where a node had only one line ending at it.          *
//c*     This line, and any lines it subsequently deadends, when it is   *
//c*     eliminated, are removed from further consideration.             *
//c*                                                                     *
//c*                                                                     *
//c*         arguments                                                   *
//c*         ---------                                                   *
//c*         name     type               purpose                         *
//c*         ----     ----               -------                         *
//c*         ibad     i*4 (input)      the node number of the node       *
//c*                                    with only one set of coordinates.*
//c*                                                                     *
//c*        Written by Larry Anderson Dec 87                             *
//c***********************************************************************
lpPolyCom2 lpPoly4;
       int iway;
       *node_num = ibad;
S10:  
        *occ = 0;
        HASHF((int)lpPoly1->id2,&space[10],&lpPoly1->loc);
        if(lpPoly1->loc == 0)goto S100;
        HASHG((int)lpPoly1->id2,&space[10],lpPoly1->loc);
        lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
        if(lpPoly1->loc !=0)goto S100;
        lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]);//to get the first occurrance
        if(lpPoly1->loc !=0)goto S100;
        if (*x_y_ary_loc ==0) goto S50;
        lpPoly2 = lpPoly2Base + *x_y_ary_loc;
        if(*sx ==lpPoly2->from_x_array && *sy ==lpPoly2->from_y_array)
        { 
           lpPoly1->hit_x = lpPoly2->to_x_array ; //{Get set up to move to the
           lpPoly1->hit_y = lpPoly2->to_y_array ; //{other end of this line
        } 
        else
        {
           lpPoly1->hit_x = lpPoly2->from_x_array;
           lpPoly1->hit_y = lpPoly2->from_y_array;
        } 
        if(*the_way ==0)  //{currently going with the grain
           iway = 1   ;   //{will be going against it at the other end
       else               //{currently going against the grain
           iway = 0   ;   //{will be going with it at the other end
 
//c       I eliminate the dead end node 
        lpWork = lpWorkBase + *the_rec;
        lpWork->valid = FALSE ; //{says the line is now a dead end  
        lpPoly4 = lpPoly2Base + *x_y_ary_loc;
        lpPoly4->valid = FALSE; 
        lpPoly4->line_desc[1] = 0;
        lpPoly4->line_desc[2] = 0;
                    //beginning of debug stuff
        /*    _fstrcpy(lpPoly1->rec,"Poly Eliminator now on LINE    ");
            itoa(*x_y_ary_loc,&lpPoly1->rec[30],10);
            MessageBox(0,lpPoly1->rec,
                  "Offset Eliminator",MB_ICONINFORMATION);
            _fstrcpy(lpPoly1->rec,"Poly Eliminator now at NODE    ");
            itoa(*this_node,&lpPoly1->rec[30],10);
            MessageBox(0,lpPoly1->rec,
                  "Offset Eliminator",MB_ICONINFORMATION);    */
            //ending of debug stuff

        lpNodeArr = lpNodeArrBase + *node_num;
S50:    lpPoly1->i_meet = *the_rec ; //{the original sgr record number
        *occ = 0;
        lpPoly1->st = BT_DELETE (lpPoly1->id1,&space[12],&space[30],FALSE);
        *occ = 1;
        lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12],&space[30],FALSE);
        lpNodeArr->node_array = 0  ; //{reduce the occurrences to zero
//c       next i eliminate the appropriate occurrence
//c       of the other end of the line found to be a dead end

//c        next i move to the other end coords and check it for dead ending
         *occ = 0;
         *sx = lpPoly1->hit_x;
         *sy = lpPoly1->hit_y;
          lpPoly1->st = PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //{the base record
         if(lpPoly1->st !=0)goto S100;
         if(*sx != lpPoly1->hit_x || *sy != lpPoly1->hit_y)  goto S100; 
         lpNodeArr = lpNodeArrBase + *this_node;
         if(lpNodeArr->node_array == 0)return 0;
//c           i already have the valid array set to indicate
//c           this is a dead end line.

         //{I find the line to be eliminated and move everything
//c        {beyond it up one position in the hash array
          lpPoly1->i_count = 0;
          lpPoly1->num_here = *numem;
          for (lpPoly1->l = 1;lpPoly1->l < lpPoly1->num_here;lpPoly1->l++)
          {
            lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]) ; //{find the detailed record
            if(lpPoly1->loc != 0)goto S100;
            if(*the_rec == lpPoly1->i_meet && *the_way == iway) lpPoly1->i_count = 1 ; //{found the one to be eliminated
            if(lpPoly1->i_count == 1 && *occ < lpPoly1->num_here)
            { 
               *occ = *occ + 1;
//c***               _fmemmoveRE(int2(i+1),id1_key(17][),2) {the next one
               lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
               if(lpPoly1->loc !=0)goto S100;
               *occ = *occ - 1;
                PolyBTPut(lpPoly1->id1,&space[12]) ; //{i now inplace of i + 1
            } 
          } //end do;
          lpNodeArr = lpNodeArrBase + *this_node;
          if(lpNodeArr->node_array > 0)lpNodeArr->node_array--;
          *occ = lpPoly1->num_here  ; //{the last occurrence of this node
          lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12],&space[30],FALSE);
          if(lpNodeArr->node_array == 0)
          { // ; //{delete the base record too
             *occ = 0;
            lpPoly1->st=BT_DELETE(lpPoly1->id1,&space[12],&space[30],FALSE);
            lpPoly4->line_desc[2] = 0;
            //beginning of debug stuff
        /*    _fstrcpy(lpPoly1->rec,"Poly Eliminator now Deleting NODE    ");
            itoa(*this_node,&lpPoly1->rec[35],10);
            MessageBox(0,lpPoly1->rec,
                  "Offset Eliminator",MB_ICONINFORMATION); */
            //ending of debug stuff
          } 
          else 
          {
             *occ = 0;
             lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]) ; //{get the base rec
             if(lpPoly1->loc !=0)goto S100;
             if(*numem > 0) *numem = *numem - 1  ; //{update the occurrence of this node
            //beginning of debug stuff
       /*     _fstrcpy(lpPoly1->rec,"Poly Eliminator Reducing lines here to      ");
            itoa(*numem,&lpPoly1->rec[40],10);
            MessageBox(0,lpPoly1->rec,
                  "Offset Eliminator",MB_ICONINFORMATION);  */
            //ending of debug stuff
             PolyBTPut(lpPoly1->id1,&space[12]) ; //{update the base record
          } 
         if(lpNodeArr->node_array == 1)
         { //{must perform additional deletions 
           *node_num = *this_node; 
           goto S10  ; //{eliminate some more
         } 
         return 0;
S100:   

         lpNodeArr->node_array = 0;
         return -1;
    } 
    
    double  CALCL_TOL(const double *pcx,const double *pcy,const double *radx,
                      const double *rady,const double *ptx,const double *pty,
                      const double *len)
{
      lpPoly1->directions[0] = LGETAZ (*radx, *rady, *pcx, *pcy);
      lpPoly1->directions[1] = LGETAZ (*radx, *rady, *ptx, *pty);
      lpPoly1->directions[2] = AZDF(lpPoly1->directions[0],
                                    lpPoly1->directions[1],
                                    *len);

return lpPoly1->directions[2] * LDIST(*radx, *rady, *pcx, *pcy);
} 

/*         
void poly_display_deadends(ist);
; //{

//c***********************************************************************;
//c*                                                                     *;
//c*        Routine Summary                                              *;
//c*        ------- -------                                              *;
//c*                                                                     *;
//c*        This routine, part of poly, finds all nodes connected        *;
//c*        to a single line (dead end) and displays a cross at that     *;
//c*        location.                                                    *;
//c*                                                                     *;
//c***********************************************************************;


      double   sx, sy;
      longi, j, seg(1000);
      short node_num;
      character id2_key*18, string*3;

      equivalence (id2_key,node_num), 
                  (id2_key(3][),sx),
                  (id2_key(11][),sy);

S25:   format(i3);
      do i=1,nde;
          d} else {g (seg[i])
      enddo;
      nde = 0;
S20:   do j = 1 , num_nodes  ; //{for each node, check number of intersects
         occ = 0;
         if(node_array(j) ==1){ //
            node_num = j;
             hashf(id2,id2_key,loc_base);
            if (loc_base !=0) then;
                hashg(id2,id2_key,loc_base);
               nde = nde + 1;
               write(string(1][3),25) nde;
                point_marker(int4(nde),sx/1000d0,sy/1000d0,
                           string(][3),seg(nde));
            } // 
         } // 
      enddo;

      return;

//C==============================================================

      (void)poly_erase_deadends(ist);

      do i=1,nde;
         if (seg[i]>0)  d} else {g (seg[i])
      enddo;

      return;
      end;   */
      


double PolyCalcDeltaAzm(double *a1,BOOL d1,double *a2,BOOL d2,double *sc)
          {
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       This routine, part of the polygon system, generates the      *
//c*       delta_azm found between two intersection chords. The directio*
//c*       of travel on the way into the intersection point is A1,      *
//c*       The direction of travel on the outgoing line s A2.           *
//c*                                                                    *
//c*                                                                    *
//c*       arguments                                                    *
//c*       ---------                                                    *
//c*       a1        R*8   (input) the azimuth of the current chord     *
//c*       d1        L*4   (input) the direction on the first chord     *
//c*       a2        R*8   (input) the azimuth of the second chord      *
//c*       d2        L*4   (input) the direction on the second chord    *
//c*       sc        r*8   (input) the difference in azimuth between    *
//c*                               the two intersecting lines at the    *
//c*                               point of intersection.               *
//c*                                                                    *
//c*       Author][ Larry Anderson Jan 88                                *
//c*                                                                    *
//c*       Modified Mar 88 to work with smallest change and delta_azm's *
//c*       instead of the change in chord azimuth's.                    *
//c*                                                                    *
//c**********************************************************************
//c*
#include "cnstnt.h"
   

      if(d1) 
          return  ((PY - *sc) - *a1);
      else  
          return   (*a1 + (PY - *sc));
  }
// id2's x coordinate in unioned with id1's x coordinate to allow me
// to go from a set of coordinates to an intersection point.   
    


  int       poly_check_coincident(lpWS L1, lpWS L2,double **x1,double **y1)
  {
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       This routine is ed to check for coincident lines.        *
//c*                                                                    *
//c*       This routine, part of the polygon system, is used after      *
//c*       the ing routine determined the line in ltable[i]         *
//c*       intersects with ltable(J) twice.  The intersecting           *
//c*       coordinates, passed to this routine are used to determine    *
//c*       the azimuths of the two lines and the type of coincidence    *
//c*       (if any).                                                    *
//c*                                                                    *
//c*       Arguments                                                    *
//c*       ---------                                                    *
//c*       i     i*4     (input)  The Ltable element of the first line  *
//c*       j     i*4     (input)  The Ltable element of the second line *
//c*       x1    r*8(2)  (input)  The x coordinate of the intersections *
//c*       y1    r*8(2)  (input)  The y coordinate of the intersections *
//c*       coincid L*4   (output) Set to true if coincidence is present *
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
#define n3  3
          short i2s[5];
      
      if(L1->type != L2->type)return 0; //{coincidence not possible
     if(L1->type == 2)
      { 
//c*        {Already know we have two hits.  So... lines are
//c*        {parallel... All I gotta do is determine type of overlap.
       i2s[1] = INLNCK(&L1->ldax1,&L1->lday1,&L1->ldax2,&L1->lday2,&L2->ldax1,&L2->lday1);
       i2s[2] = INLNCK(&L1->ldax1,&L1->lday1,&L1->ldax2,&L1->lday2,&L2->ldax2,&L2->lday2);
       i2s[3] = INLNCK(&L2->ldax1,&L2->lday1,&L2->ldax2,&L2->lday2,&L1->ldax1,&L1->lday1);
       i2s[4] = INLNCK(&L2->ldax1,&L2->lday1,&L2->ldax2,&L2->lday2,&L1->ldax2,&L1->lday2);

          if(i2s[1]+i2s[2]+i2s[3]+i2s[4] == 0)   return 4 ; //{lines are identical (within p_tol standards).
          if(i2s[1] + i2s[2] == 0) return 2 ; //{line J within/on line I
          if(i2s[3] + i2s[4] == 0) return 1;
          return 3 ; //{lines overlap for only a portion of both
      } 
      else 
      {  //{they're both curves
//c*        {If the radi are within  p_tol of one another I will consider
//c*        {them to be coincident curves
//C*          rr=ldist(lx2[i],ly2[i],lx2(j),ly2(j)) {dist between radi
//C* this is not a valid check
//C*          if(rr > p_tol)  return{not coincident

//c*        {Here the coords of the first curve's PT are found 
           lpPoly1->sign = fabs(L1->ldalngth);
           LOL8(&L1->ldax1,&L1->lday1, &L1->ldax2, &L1->lday2, &L1->ldalngth,
                    &lpPoly1->sign, &lpPoly1->hit_x, &lpPoly1->hit_y, n3);

//c*        {Here the coords of the second curve's PT are found 
           lpPoly1->sign = fabs(L2->ldalngth);
           LOL8(&L2->ldax1, &L2->lday1, &L2->ldax2, &L2->lday2, &L2->ldalngth,
                    &lpPoly1->sign, &lpPoly1->rx, &lpPoly1->ry, n3);

//c*        {I check if the pc of curve J is in curve I
           i2s[1]=INCRVE(&L1->ldax1,&L1->lday1, &L1->ldax2, &L1->lday2, &L1->ldalngth,
                      &L2->ldax1, &L2->lday1);

//c*        {I check if the pc of curve I is in curve J
           i2s[2]=INCRVE(&L2->ldax1, &L2->lday1, &L2->ldax2, &L2->lday2, &L2->ldalngth,
                      &L1->ldax1,&L1->lday1);

//c*        {I check if the pt of curve J is in curve I
           i2s[3]=INCRVE(&L1->ldax1,&L1->lday1, &L1->ldax2, &L1->lday2, &L1->ldalngth,
                      &lpPoly1->rx, &lpPoly1->ry);

//c*        {I check if the pt of curve I is in curve J
           i2s[4]=INCRVE(&L2->ldax1, &L2->lday1, &L2->ldax2, &L2->lday2, &L2->ldalngth,
                      &lpPoly1->hit_x, &lpPoly1->hit_y);

        if(i2s[1] + i2s[2] + i2s[3] + i2s[4] == 0)
        {  
                lpPoly1->delta_x = LDIST(L1->ldax2, L1->lday2,L2->ldax2, L2->lday2)  ;
            //    center_dist = fabs(center_dist);
             if (lpPoly1->delta_x < P_TOL)
             {
                return  4 ; //{lines are identical (within p_tol standards).
             } 
             else 
             {
                return  0 ; //{foot ball shape or new moon shape
             } 
         } 
         else 
         { 
           if(i2s[1] + i2s[3] ==0) return 2 ; //{line J within/on line I
           if(i2s[2] + i2s[4] ==0) return 1 ; //{line I within/on line J
           return  3 ; //{lines overlap for only a portion of both
          } 
      }  
  
     }
      
    

  void      poly_check_for_islands()
    {
//C******* SPECIFICATIONS ***********************************************
//C*                                                                    *
//C*       PROGRAM SUMMARY                                              *
//C*       ------- -------                                              *
//C*       This routine was part of the polygon analysis system.        *
//C*       By the time 'it' gets here, right sides of all areas have    *
//C*       been tracked. This routine checks to ensure all elements     *
//C*       in the line_desc array have values > 0                    *
//C*                                                                    *
//C*       Should it find a line_desc element with a negative value     *
//C*       it assumes it found an island and proceeds to find out       *
//C*       what other area surrounds it.                                *
//C*                                                                    *
//C*                                                                    *
//C*                                                                    *
//C*       ARGUMENT DESCRIPTION                                         *
//C*       -------- -----------                                         *
//C*      st         I*4     (output) the returncode                   *
//C*                                                                    *
//C*       AUTHOR][  LARRY ANDERSON  Dec 87                              *
//C*                                                                    *
//C**********************************************************************
//C*

           long ocean_ref, island_ref;

        for(lpPoly1->k = 1;lpPoly1->k <= lpPoly1->num_sides;lpPoly1->k++)
       {
          lpPoly2 = lpPoly2Base + lpPoly1->k;
          if (lpPoly2->line_desc[1] == -2 && 
              lpPoly2->line_desc[2] == -2 &&
              lpPoly2->valid)goto S10; //'found a valid untracked line'
           if(lpPoly2->line_desc[1] == -2 && lpPoly2->valid)
            { 
//c             {the left side of the line remains untracked with
//c             {valid descriptions. the tru_area_number number of
//c             {island is in line_desc(i,2)
              island_ref = lpPoly2->line_desc[2];
              lpPoly1->hit_x = lpPoly2->to_x_array;
              lpPoly1->hit_y = lpPoly2->to_y_array;
              PolyFindOcean(&island_ref,&ocean_ref); 
              goto S10;
            }  
           if(lpPoly2->line_desc[2] == -2&& lpPoly2->valid)
            { 
//c             {the right side of the line remains untracked with
//c             {valid descriptions. the tru_area_number number of
//c             {island is in line_desc(i,1)
              island_ref = lpPoly2->line_desc[1];
              lpPoly1->hit_x = lpPoly2->from_x_array;
              lpPoly1->hit_y = lpPoly2->from_y_array;
              PolyFindOcean(&island_ref,&ocean_ref); 
            }   
         
S10:     ;      
       }
       return;
   }
       

int    PolyAreas(void)
{        
//c*      (void)poly_trouble_coords(trouble_x, trouble_y)
//C******* SPECIFICATIONS ***********************************************
//C*                                                                    *
//C*       PROGRAM SUMMARY                                              *
//C*       ------- -------                                              *
//C*       This routine was part of the polygon analysis system.        *
//C*       By the time 'it' gets here, the coordinates of all the       *
//C*       centroids are unknown. This routine uses the sides, without  *
//C*       centroids to determine the areas themselves                  *
//C*                                                                    *
//C*       ARGUMENT DESCRIPTION                                         *
//C*       -------- -----------                                         *
//C*      st         I*4     (output) the returncode                   *
//C*                                                                    *
//C*       AUTHOR][  LARRY ANDERSON  Dec 87                              *
//C*                                                                    *
//C**********************************************************************
//C*                           
//c*%include 'ltable.ftn' ; //{/umsc/include/ltable.ftn}
//%include 'ldatable.ftn' ; //{/umsc/include/ldatable.ftn}

  long jump;
static double aztol = 1e-6;
     double LastAzm, tester;    

         lpPoly1->shift = lpPoly1->debugging;
         lpPoly1->debugging = FALSE;
//c         do i = 1,num_sides+10
//c           tracker(i,1) = 0
//c           tracker(i,2) = 0
//c         enddo
         lpPoly1->tru_num_areas = 0;
         lpPoly1->max_num_sides_allowed = lpPoly1->num_orig_sides * 16;
         lpPoly1->min_start = 1;
         lpPoly1->num_lines = 0 ;
         lpPoly1->num_exclusions = 0;
         lpPoly1->first = TRUE;
         lpPoly1->got_min = FALSE;
         lpPoly1->good_one = TRUE;
//         if(j2 !=0)  writexy(j2,int2(50),int2(3),
//             'Now Creating Poly Areas                     ');
S10:     lpPoly1->loc = PolyFindStart(&lpPoly1->good_one,sx,sy);
         lpPoly1->i_count = 0;
         lpPoly1->taz = 0e0;

      if (lpPoly1->good_one)
      {   //{until all the lines have been tracked twice
          lpPoly1->next_orig = *the_rec  ;
          lpPoly2 = lpPoly2Base + lpPoly1->hit_line;
          LastAzm = *az8; 
          lpPoly1->curr_azm = *az8;
          lpPoly1->the_grain = *the_way;
          lpPoly1->got_min = FALSE;
          if(lpPoly1->min_valid_line == lpPoly1->hit_line)
                lpPoly1->got_min = TRUE;
          goto S399  ; //{and work off the other end of this line
      } 
      else 
      {
//c*        {all done with the subareas.
          goto S498;
      }   
//c     now i start circling the area, finding the next
//c     rightest line.

S360:     lpPoly1->i_meet = 0;
          *occ = 0;
          lpPoly1->loc = PolyBTGetFirst(lpPoly1->id1,&space[12]);
          lpPoly1->num_here = *numem;
          for (jump = 1; jump <= lpPoly1->num_here; jump++) //{for each line intersecting at this point
          {
            lpPoly1->loc = PolyBTGetNext(lpPoly1->id1,&space[12]);
            if(lpPoly1->loc != 0)goto S499 ; //{ FATAL ERROR in poly_areas *** ' 
            if(*x_y_ary_loc == 0)
            {
              GSSiMsgBox(0," Exiting PolyArea... Found an invalid array location",
                              "PolyAreas Error", MB_ICONSTOP,0);
              lpPoly1->debugging = lpPoly1->shift;
              return 32;
            }
 
            if(*x_y_ary_loc != lpPoly1->hit_line && lpPoly2->valid)
            {  
               lpPoly2 = lpPoly2Base + *x_y_ary_loc;
               lpPoly1->i_meet++;;
               lpPoly1->with_the_grain[lpPoly1->i_meet] = *the_way;
               lpPoly1->directions[lpPoly1->i_meet] = *az8;
               lpPoly1->possible_ways[lpPoly1->i_meet] = (short)*x_y_ary_loc ; //{the array element
               lpPoly2 = lpPoly2Base + *x_y_ary_loc ;
               lpPoly1->poss_orig[lpPoly1->i_meet] = *the_rec ; //{the original sgr record  
            }
            else
            {
               lpPoly1->dminx = *az8;//azimuth of the line currentlt on
            }  
          }

          if(lpPoly1->i_meet == 0)
          {//{this should not happen 
              GSSiMsgBox(0," *** ERROR *** A leak in one of the areas.","PolyAreas Error",
               MB_ICONSTOP,0);
              GSSiMsgBox(0,"The most probable cause of this type\
 of problem is overlapping lines.","PolyAreas Error",
               MB_ICONSTOP,0);
             lpPoly1->debugging = lpPoly1->shift;
             return 32;
          }  
//d         write(56,*)' ',' '
//c         {here, I find the rightest line
          lpPoly1->smallest_change = 9e9;
          lpPoly1->old_grain = !lpPoly1->the_grain;
          lpPoly1->az2 = lpPoly1->curr_azm;
          
          for(lpPoly1->l = 1; lpPoly1->l <= lpPoly1->i_meet ;lpPoly1->l++)
          { //{for each line at this intersection  
             lpPoly1->delta_azim = AZDF(lpPoly1->dminx, lpPoly1->directions[lpPoly1->l],-9e1);
             lpPoly1->close = FALSE;
             if(lpPoly1->i_meet == 1)goto S99;         
             if(fabs(lpPoly1->delta_azim) <=aztol ||
                fabs(lpPoly1->delta_azim - TWOPI) <= aztol)lpPoly1->close = TRUE;
             if(lpPoly1->close ||
                fabs(lpPoly1->az2 - lpPoly1->directions[lpPoly1->l]) <= aztol ||
                fabs(lpPoly1->az2 - (lpPoly1->directions[lpPoly1->l] - TWOPI)) <= aztol ||
                fabs((lpPoly1->az2 - TWOPI) - lpPoly1->directions[lpPoly1->l]) <= aztol)
             { 
//c*              {I load information about the current line into
//c*              {array element 63
                if(lpPoly1->close)
                { 
                   lpPoly1->k = 63;
                   lpPoly1->poss_orig[63] = lpPoly1->poss_orig[lpPoly1->next];
                   if(lpPoly1->with_the_grain[lpPoly1->next] == TRUE)
                   { 
                      lpPoly1->with_the_grain[63] = FALSE;
                   } 
                   else 
                   {
                      lpPoly1->with_the_grain[63] = TRUE;
                   }  
                   lpPoly1->directions[63] = lpPoly1->curr_azm;
                   lpPoly1->az3 = lpPoly1->curr_azm  ; //{current azimuth
                } 
                else 
                {
                   lpPoly1->az3 = lpPoly1->az2;
                   lpPoly1->k = lpPoly1->next;
                }  
//d               write(56,*)' Calling POLY_TIE_BREAKER with this ',az3
//d               write(56,*)'  and directions(',l,') = ',directions(L)
//d               write(56,*)' line 1 = ',poss_orig[k],' line 2 = ',
//d    +          poss_orig(L),' matching azm = ',directions(L)
                lpPoly1->az1 = lpPoly1->directions[lpPoly1->l];
                PolyTieBreaker(&lpPoly1->az3, &lpPoly1->az1, lpPoly1->k, lpPoly1->l);
                if(lpPoly1->az3 > lpPoly1->az1)
                {  //{changed it
                     lpPoly1->delta_azim = LTWOPI(lpPoly1->delta_azim - 1e-5);
               } 
               else 
               { 
                 if(lpPoly1->az1 > lpPoly1->az3)
                     lpPoly1->delta_azim = LTWOPI(lpPoly1->delta_azim + 1e-5);
               }
             }
               
S99:         if(lpPoly1->delta_azim < lpPoly1->smallest_change)
             {  //{found another line making a sharper turn to the right
                lpPoly1->k = (short) lpPoly1->l;
                lpPoly1->next = (short) lpPoly1->l;
                lpPoly1->az2 = lpPoly1->directions[lpPoly1->l];
                lpPoly1->next_line = lpPoly1->possible_ways[lpPoly1->l] ; //{refn of line}
                lpPoly1->the_grain = lpPoly1->with_the_grain[lpPoly1->l];
                lpPoly1->smallest_change = lpPoly1->delta_azim ; //{directions(L) - curr_azm
                lpPoly1->next_orig = lpPoly1->poss_orig[lpPoly1->l];
                
             } 
          } //enddo  ;

//c*        Ok, I've determined the next line to step onto

            lpPoly1->delta_azim = AZDF(LastAzm, lpPoly1->directions[lpPoly1->next],9e1);
             tester = LTWOPI(lpPoly1->delta_azim + 1e-6);
            if(tester < 1e-5) lpPoly1->delta_azim = 0e0;
            if(tester > PY) lpPoly1->delta_azim = tester - TWOPI;
            lpPoly1->taz += lpPoly1->delta_azim;
            lpPoly1->hit_line = lpPoly1->next_line ; 
            LastAzm = lpPoly1->az2;

          if(lpPoly1->min_valid_line == lpPoly1->hit_line)  lpPoly1->got_min = TRUE;

//c         next line now contains the refn of the line to follow
//c         next_azm contains it's azimuth.
//c         i track the current line and move on to the next line

          if(lpPoly1->i_start == lpPoly1->hit_line &&
              (lpPoly1->first_grain && lpPoly1->the_grain ||
              !lpPoly1->first_grain && !lpPoly1->the_grain))
          { //made it around to the start line going the same direction as when started
            if(lpPoly1->taz > 0 || !lpPoly1->good_one)
            { // ; //{tracked area clockwise
               lpPoly1->tru_num_areas++;
               if(lpPoly1->tru_num_areas >= MaxNumAreas)
               {
                 GSSiMsgBox(0,"Exceeded the MaxNumAreas limit",
                           "PolyAreas Error", MB_ICONSTOP,0);
                 return 32;
               }
               lpGInfo = lpGInfoBase + lpPoly1->tru_num_areas;
               lpGInfo->min_x = 999999999e0;
               lpGInfo->min_y = 999999999e0;
               lpGInfo->max_x = -999999999e0;
               lpGInfo->max_y = -999999999e0;
               for (lpPoly1->k = 1; lpPoly1->k <= lpPoly1->i_count  ;lpPoly1->k++)
               { //{for each line composing the area 
                  lpNew = lpNewBase + lpPoly1->k;
                  lpPoly1->num_lines++;
                  lpPoly2 = lpPoly2Base + lpNew->Poly2Loc;
                  
                  lpGInfo->min_x = __min(lpPoly2->from_x_array,
                          __min(lpPoly2->to_x_array,lpGInfo->min_x));
                          
                  lpGInfo->min_y = __min(lpPoly2->from_y_array,
                          __min(lpPoly2->to_y_array, lpGInfo->min_y));
                          
                  lpGInfo->max_x = __max(lpPoly2->from_x_array,
                          __max(lpPoly2->to_x_array,lpGInfo->max_x));
                          
                  lpGInfo->max_y = __max(lpPoly2->from_y_array,
                          __max(lpPoly2->to_y_array,lpGInfo->max_y));
                  
                  if(lpNew->direction == 'T')
                  { 
                     lpPoly2->line_desc[2] = lpPoly1->tru_num_areas; 
                  } 
                  else 
                  {
                     lpPoly2->line_desc[1] = lpPoly1->tru_num_areas;
                  }
                  lpGInfo->AreaDesc = lpPoly1->tru_num_areas;
 
                  if(lpPoly1->k == 1) lpGInfo->j = lpPoly1->num_lines  ; //{capture location of first record
                  if(lpPoly1->num_lines >= MaxAllSides)
                  {
                    GSSiMsgBox(0,"Exceeded MaxAllSides limit",
                                 "PolyAreas Error", MB_ICONSTOP,0);
                    return 32;
                  }
                  lpAllS = lpAllSBase + lpPoly1->num_lines;
                  lpAllS->all_sides =  lpNew->Poly2Loc;
                  lpAllS->all_dir =    lpNew->direction;
                  lpAllS->orig_lines = lpNew->Poly1Loc ; //{the original sgr rec
               }
               lpGInfo->i_count = (unsigned int) lpPoly1->i_count ; //{num of sides
               lpGInfo->num_excl = 0 ; //{the num_exclusions
//d              write(56,*)' ','stats for area ',tru_num_areas,' are]['
//d              write(56,*)' ',' tot_num_lines = ',i_count
//d              write(56,*)' ',' '
               if(!lpPoly1->good_one)
               { 
//c*                I must set the line_desc array for the untracked
//c*                side
                  lpPoly1->j2 = -2  ;
                  if(lpPoly1->got_min) lpPoly1->j2 = -1;
                  for(lpPoly1->k = 1;lpPoly1->k <= lpPoly1->i_count;lpPoly1->k++)
                  { 
                    lpNew = lpNewBase + lpPoly1->k;
                    lpPoly2 = lpPoly2Base + lpNew->Poly2Loc;
                    if(lpPoly2->line_desc[2] == 0)//{I was on the right side 
                       lpPoly2->line_desc[2] = lpPoly1->j2;
                    else 
                       lpPoly2->line_desc[1] = lpPoly1->j2;
                  }
                  goto S498 ; //{getting outa here
               }  
             
            } 
            else //tracked around backward (creating a negative area)
               // OR.... I just tracked the perimeter backward
            {  //{I must retrace this route and invalidate the line_desc
//ccc            if(got_min)'found and tracked the perimeter'
//ccd             'Untracking an area cause it''s negative'
                lpPoly1->j2 = -2  ;
                if(lpPoly1->got_min) lpPoly1->j2 = -1;
                for (lpPoly1->k = 1;lpPoly1->k <= lpPoly1->i_count;lpPoly1->k++)
                { 
                  lpNew = lpNewBase + lpPoly1->k;
                  lpPoly2 = lpPoly2Base + lpNew->Poly2Loc;
                  if(lpNew->direction=='T')
                     lpPoly2->line_desc[2] = lpPoly1->j2;
                  else 
                     lpPoly2->line_desc[1] = lpPoly1->j2;
               }
            } 
            goto S299;
          }  //{if got a complete area

//c         next i set the variables to go on to the next line
          lpPoly1->curr_line = lpPoly1->next_line;

S399:     lpPoly1->i_count++;
          lpNew = lpNewBase + lpPoly1->i_count;
          if(lpPoly1->i_count > lpPoly1->max_num_sides_allowed)
          {   
               GSSiMsgBox(0," Exiting PolyArea... Area exceeds number of sides limitation",
                          "PolyAreas Error", MB_ICONSTOP,0);
             lpPoly1->debugging = lpPoly1->shift;
             return 32;
          }                            
          lpPoly2 = lpPoly2Base + lpPoly1->hit_line;
          if (lpPoly1->the_grain)
          {
             lpPoly1->hit_x = lpPoly2->to_x_array ; //{set up for the next}
             lpPoly1->hit_y = lpPoly2->to_y_array ; //{match up at the +far}
             lpNew->direction = 'T';
          } 
          else 
          { //{end of the next line}
             lpPoly1->hit_x = lpPoly2->from_x_array;
             lpPoly1->hit_y = lpPoly2->from_y_array;
             lpNew->direction = 'F';
          }  
          lpNew->Poly1Loc = lpPoly1->next_orig;
          lpNew->Poly2Loc = (short) lpPoly1->hit_line;
          *sx = lpPoly1->hit_x;
          *sy = lpPoly1->hit_y;
          lpPoly1->st = PolyFindNextNode(sx,sy,lpNew->direction);
          if(lpPoly1->st != 0)
          { 
             GSSiMsgBox(0, "*** Fatal Error *** Unable to locate\
 proper coordinates", "PolyAreas Fatal Error", MB_ICONSTOP,0);
            lpPoly1->debugging = lpPoly1->shift;
            return 32;
          } 
         lpPoly1->the_grain = !lpPoly1->the_grain; 
         goto S360;
S299:   goto S10;

//c       finished with all the sub_areas
S498:    lpPoly1->debugging = lpPoly1->shift;
         return 0;
S499:    lpPoly1->debugging = lpPoly1->shift;
         return -1;

}         
         

time_t mytime;
       long  ntot, btst, ist;

       short      datlen,   lkey, i;
       lrec,      fldtyp[8], fldlen[8], id,       nkeyfld, ft[8],
       fl[8],     nidx;    
       
  HGLOBAL CurrGlobal;

           
int    PolyBTPut (const HGLOBAL bid5,LPSTR record)
{      
      if(bid5 == 0)return -1;
      if(CurrGlobal != bid5)
      {
        for(lkey = 0;lkey < 12; lkey++)
        {
          if(lpPoly1->hGlobals[lkey] == bid5) break;
        }
        CurrGlobal = lpPoly1->hGlobals[lkey];
      }  
      return (BT_PUT (bid5,record,&record[lpPoly1->datloc[lkey]]));
       
 }

HGLOBAL PolyBTInit (short lrec,short nkeyfld,short fldtyp[8],short fldlen[8])
{
  BTVARDESC BTDesc[8];
  LPBTVARDESC lpBTDesc = &BTDesc[0];
  HGLOBAL id;   
  
      ntot = 0;
      lkey = 0;
      nidx = nkeyfld;
      GSSiGetTempFileName (0,"gmp",0,(LPSTR)lpPoly1->file_name);

      for( i = 0; i < nidx;i++,lpBTDesc++)
      { 
        lpBTDesc = &BTDesc[i];
        lpBTDesc->BT_VARTYP = fldtyp[i];
        lpBTDesc->BT_VARLEN = fldlen[i];
        lpBTDesc->BT_VAROFF = lkey;
        lkey +=  fldlen[i];
      } //end of the for loop
      datlen = lrec - lkey;
      mytime = time(&mytime);
      lpBTDesc = &BTDesc[0];
      
/*      {
      	int Fid;
      	OFSTRUCT OFStruct;  
      	static	First=TRUE;
                           
        if (First)
        {
        	First=FALSE;
	  	Fid = GSSiOpenFile ("log",&OFStruct,OF_CREATE); 
	  	
	    BigWrite (Fid,(HPSTR)&datlen,2,-1); 
	    BigWrite (Fid,(HPSTR)&nidx,2,-1);
	    BigWrite (Fid,(HPSTR)lpBTDesc,nidx*sizeof(BTVARDESC),-1);
	    GSSiClose (Fid); 
	   } 
	  }*/
	  
      
      
      ist = BT_CREATE (lpPoly1->file_name, datlen, FALSE, nidx, 1, 
                 lpBTDesc, FALSE, 0, mytime, FALSE);

       if(!ist)
       {
          GSSiMsgBox(0,"Error... Unable to Initiate B Tree","B Tree Init Error",
                     MB_ICONSTOP,0);
          return 0;
       }  
       id = BT_OPEN (lpPoly1->file_name, mytime, 1, 0);
       i = 1;
       if(id)
       {
         while(lpPoly1->datloc[i] != 0)i++;
         lpPoly1->datloc[i] = lkey;
         lpPoly1->hGlobals[i] = id;
         CurrGlobal = id;
         lkey = i;
       } 
       return id;
}
//c     {***********************
int PolyBTGetFirst (const HGLOBAL bid1,LPSTR record)
{
//c      _fmemmoveRE(int2(0),record(lkey-1),2) 

      if(bid1 == 0)return -1;
      if(CurrGlobal != bid1)
      {
        for(lkey = 1; lkey < 12; lkey++)
        {
          if(lpPoly1->hGlobals[lkey] == bid1) break;
        }
        CurrGlobal = lpPoly1->hGlobals[lkey];
      } 
      return  BT_FIND (bid1,record,BT_FIRST,BT_GE,
                      &record[lpPoly1->datloc[lkey]]);
}
//c     {***********************
 int PolyBTGetThis (const HGLOBAL bid6,LPSTR record)
{    
      if(bid6 == 0)return -1;
      if(CurrGlobal != bid6)
      {
        for(lkey = 1;lkey < 12;lkey++)
        {
          if(lpPoly1->hGlobals[lkey] == bid6) break;
        }
        CurrGlobal = lpPoly1->hGlobals[lkey];
      }
      return BT_FIND (bid6,record,BT_FIRST,BT_EQ,
                       &record[lpPoly1->datloc[lkey]]);
       
}
//c     {***********************
int PolyBTGetNext (const HGLOBAL bid2,LPSTR record)
{     
	static BOOL	Back=FALSE;
      if(bid2 == 0)return -1;
      if(CurrGlobal != bid2)
      {
        for(lkey = 1; lkey < 12;lkey++)
        {
          if(lpPoly1->hGlobals[lkey] == bid2) break;
        }
         CurrGlobal = lpPoly1->hGlobals[lkey];
      } 
      if (!Back)
      return BT_FIND (bid2,record,BT_NEXT,BT_ANY,
                      &record[lpPoly1->datloc[lkey]]); 
      else
      return BT_FIND (bid2,record,BT_PRIOR,BT_ANY,
                      &record[lpPoly1->datloc[lkey]]);
      
     
 }
//c     {***********************
 int PolyBTClose (HGLOBAL *bid3)
 { 
   int btst;
      if(*bid3 == 0) return -1;
      return (BT_CLOSEANDDELETE (bid3));
 }
//c     {***********************
 int PolyBTClear (const HGLOBAL bid4)
 {     
      //return = BT_CLEAR (bid4); 
      return 0;
 }
      

  void OffsetQuicky(void);

const unsigned int RADICAL = 3201;
      
BOOL GetOffsetMem(lpArea lpAr);
 int  OffsetFindPerimeter(void);

	
               
   
//BOOL ROUND_ALIGNMENT_ENDS = TRUE, 
//     RIGHT_SIDE_ONLY = FALSE, 
//     one_side = FALSE,
 //    LEFT_SIDE_ONLY = FALSE,  
 //    DEBUGGING = FALSE;

  void OFFSET_MAIN(lpArea InArea, lpLArea *OutArea, long *ST)
{                               
/*C*       ENTRY OFFSET_CLOSE()*/
/*C******* SPECIFICATIONS ************************************************/
/*C*                                                                    **/
/*C*       PROGRAM SUMMARY                                              **/
/*C*       ------- -------                                              **/


       BOOL one_side;
       long IRC,  LENHAS, i;

        if(InArea->NumSides <= 0)
        { 
           *ST = -1;
           return;
        } 
        
//cccc        round_alignment_ends = FALSE;
       IN_PTOL = P_TOL; 
       for (i = 0;i < 12; i++)
       		file[i].heap_ptr=0; 

       if(InArea->fillet_desc <= 0)InArea->fillet_desc = 1317; 
       if(!GetOffsetMem(InArea)) return;

        lpPoly1->tol_fac = P_TOL;  
        lpPoly1->id2 = 0;
        lpPoly1->id3 = 0;
       // ldatot = 32000;
        lpPoly1->link_desc = InArea->LinkDesc;
        lpPoly1->good_one = TRUE; //used by poly_areas
        one_side = FALSE; //used by offset_these, offset_intx_all_lines
        if(RIGHT_SIDE_ONLY || LEFT_SIDE_ONLY)
        {
          one_side = TRUE;
          lpPoly1->good_one = FALSE;
        }
        lpPoly1->tru_num_areas = 0;
        lpPoly1->min_valid_line = 0;
        if(InArea->type == 1)  //user submitting an area
           lpPoly1->offset = InArea->offset_dist;
        else //user submitting an alignment
           lpPoly1->offset = DSIGN(InArea->offset_dist,-1e0) ; //always negative
        
        lpPoly1->j2 = 0;
        if (P_TOL == 0) P_TOL = 1e-4;

         DEBUGGING = FALSE ;
        if(DEBUGGING) dbfile = fopen("..\\results","rt"); 
        
        for(lpPoly1->l = 0; lpPoly1->l < 12; lpPoly1->l++) 
          { lpPoly1->datloc[lpPoly1->l] = 0;
            lpPoly1->hGlobals[lpPoly1->l] = 0;
          }
           
       //tol_fac =  P_TOL;
        *ST = 0;
        lpPoly1->num_sides = 0; 
        
        *ST = OffsetThese(InArea);
        if(*ST != 0)goto S200;
        if(lpPoly1->id2!=0)
        {
           HASHC((int)lpPoly1->id2);
           lpPoly1->id2 = 0;
        } 
        if(lpPoly1->id3!=0)
        {
           HASHC((int)lpPoly1->id3);
           lpPoly1->id3 = 0;
        } 
        LENHAS = lpPoly1->num_sides*6 ; //number of line intersection points
        HASHS ((LPINT)&lpPoly1->id2,18,2,LENHAS," ",0L,"new") ; //quick node location
        HASHS ((LPINT)&lpPoly1->id3,10,6,LENHAS," ",0L,"new") ; //hold area exclusions
//
//c*      //lines are now stored in ldatable,I intersect all lines with
//c*      //; //all other lines.

        offset_intx_all_lines (InArea, one_side, ST);
          
   /*       for(i=0;i <=lpPoly1->num_nodes;i++)
          {
             lpNodeArr = lpNodeArrBase + i; 
              lpNodeArr->node_array;          
          }  */
          

        if(*ST != 0) goto S300;        
        for(lpPoly1->l = 1;lpPoly1->l <= lpPoly1->num_sides;lpPoly1->l++)
        {
           lpWork = lpWorkBase + lpPoly1->l;
           if(lpWork->intxs != 2)goto KeepGoing;
        } 
//      if(I get here) all the lines intersect each other twice
//      one right after the other... so I just stringem together        
        OffsetQuicky();
        *ST = 0;
        goto S200;
        
        
KeepGoing: ;

         if(PolyEliminateDeadends()  != 0) goto S300; ;

         poly_load_lda_coords();

         if(!PolyPurge())goto S300; //eliminates deaded lines

         *ST = PolyAreas();

         if(*ST != 0) goto S300;

         poly_check_for_islands();

//c*      If the results of poly_areas includes more than one area;
//c*      I attempt to merge/melt/meld/exclude/disgard/ignore
//c*      the multiple areas into a single area record.;
        *ST = 0;
        if(lpPoly1->tru_num_areas > 1)
        {
          *ST = OffsetFindPerimeter();
        } 
        else 
        {
             offset_validate_area(ST);
        }

S200:  P_TOL = IN_PTOL;
       ROUND_ALIGNMENT_ENDS = TRUE ;
       RIGHT_SIDE_ONLY = FALSE;
       LEFT_SIDE_ONLY = FALSE;
       lpPoly1->good_one = FALSE;
       if(*ST == 0) OffsetGet( OutArea, ST) ;     

       if(DEBUGGING) fclose (dbfile);
        return;

S300:   *ST = -1;
       goto S200 ;
}
//c*    {**********************
      void set_alignment_ends_square()
      {

         ROUND_ALIGNMENT_ENDS = FALSE;
      return ;
      }
//c*    {*************************************************
      void set_left_side_only()
{
         RIGHT_SIDE_ONLY  = FALSE;
         LEFT_SIDE_ONLY = TRUE;

      return;
}
//c*    {**************************************************
    void set_right_side_only()
    {

         RIGHT_SIDE_ONLY = TRUE;
         LEFT_SIDE_ONLY = FALSE;

      return;
    }                                                   
//c*    {***********************************************************
  void OffsetQuicky(void)
  {
   int j; 
   WorkSpace Next;
   lpWS lpNext = &Next;
   lpPoly1->tru_num_areas = 1;
   lpGInfo = lpGInfoBase + 1;
   lpGInfo->i_count = lpPoly1->num_sides;
   lpGInfo->j = 1;
   lpGInfo->num_excl = 0;
   lpGInfo->AreaDesc = 1;
   lpGInfo->min_x = 9e9;
   lpGInfo->min_y = 9e9;
   
       for(j = lpGInfo->j; j <= lpGInfo->j + lpGInfo->i_count-1; j++)
       {   //for each side in this area
          lpAllS = lpAllSBase + j;
          lpAllS->all_sides = j; 
          lpAllS->orig_lines = j;
          lpWork = lpWorkBase + j; 
          lpPoly2 = lpPoly2Base + j;
          lpPoly2->from_x_array = lpWork->ldax1;
          lpPoly2->from_y_array = lpWork->lday1;
          if(j < lpGInfo->i_count)
          { 
            lpNext = lpWorkBase + (j+1);
            lpPoly2->to_x_array   = lpNext->ldax1;
            lpPoly2->to_y_array   = lpNext->lday1;
          }  
          lpPoly2->valid        = TRUE;
          lpPoly2->WorkLoc      = j;
          lpPoly2->line_desc[0]    = lpWork->ldadesc;
          lpGInfo->min_x = __min(lpWork->ldax1,__min(lpGInfo->min_x,lpWork->ldax2));
          lpGInfo->min_y = __min(lpWork->lday1,__min(lpGInfo->min_y,lpWork->lday2));
          lpGInfo->max_x = __max(lpWork->ldax1,__max(lpGInfo->max_x,lpWork->ldax2));
          lpGInfo->max_y = __max(lpWork->lday1,__max(lpGInfo->max_y,lpWork->lday2));
          
       }    
            lpNext = lpWorkBase + 1;
            lpPoly2->to_x_array   = lpNext->ldax1;
            lpPoly2->to_y_array   = lpNext->lday1;

  }     
     
     
//c*    //********************************************//
      void OffsetGet(lpLArea *Out, long *st) 
      
//    offset_get returns a lpLArea pointer to the structure containing
//    the derived area.      
      {
       lpLArea lpLA;
        short temp_refs = -3500;
        short j, k; 
        short nl;
        long  i, ipos_link, ilink;
        double   dazm, x,y;
       *st = 0;

    nl = 0; // number of lines loaded into the output structure
    if(lpPoly1->tru_num_areas <=0)
    {
       *Out = 0;
       *st = -1;
       return;
    }
    lpPoly1->i_count = 0;   
    for( i = 1; i <= lpPoly1->tru_num_areas; i++)  //for each area
    { 
       lpGInfo = lpGInfoBase + i;
       lpPoly1->i_count += lpGInfo->i_count;
    }  
      
    for( i = 1; i <= lpPoly1->tru_num_areas; i++)  //for each area
    { 
       lpGInfo = lpGInfoBase + i;
       
       if(lpGInfo->i_count == 0)
       {
          *st = -1;
          lpLA = 0;
          return;
       } 
       // now I create an area structure that emulates Ltable
          file[10].heap_ptr = GSSiGlobAlloc ( 283, GHND, sizeof (LArea) ); 
          file[10].mem_ptr  = GlobalLock(file[10].heap_ptr);
          lpLA  = (lpLArea) file[10].mem_ptr; 
          file[11].heap_ptr = GSSiGlobAlloc ( 284, GHND, sizeof (LINE)* (lpPoly1->i_count + 22) ); 
          lpLA->Lines  = (lpLine) GlobalLock(file[11].heap_ptr); 
          ipos_link = 0;
          lpLA->NumSides = 0; 
          lpLA->MinX = lpGInfo->min_x;
          lpLA->MinY = lpGInfo->min_y;
          lpLA->MaxX = lpGInfo->max_x;
          lpLA->MaxY = lpGInfo->max_y;

       //lpGInfo->j now point to the hitline of the area in the 
       //all_sides array.
       lpPoly2 =  lpPoly2Base + lpGInfo->j;
       if(i > 1 && lpPoly2->line_desc[1] < 0 ||  lpPoly2->line_desc[2] < 0)
       {
         //if the left side or the right side of the first line of
         //this area is < 0, it's an outside (external) area boundary
         //found another external positive area.  Must link;
         //it to the first positive area
//d        print*,' ';
//d         print*,' Adding a link to another positive area';
//d        print*,' '; 
          lpL = lpLA->Lines + 1;
          lpPoly1->real_x = lpL->x1;
          lpPoly1->real_y = lpL->y1;
          nl++; 
          lpL = lpLA->Lines + nl;
          lpL->x1 = lpPoly1->real_x;
          lpL->y1 = lpPoly1->real_y; 
          ipos_link = nl;
        //  ax2(nl) = from_x_array(all_sides(gen_info(i,7)))*mag
        //  ay2(nl) = from_y_array(all_sides(gen_info(i,7)))*mag 
          lpAllS =  lpAllSBase + lpGInfo->j;
          lpPoly2 = lpPoly2Base + lpAllS->all_sides;
          lpL->x2 = lpPoly2->from_x_array;
          lpL->y2 = lpPoly2->from_y_array;
//cd         print*,'fx = ',ax1[nl],' fy = ',ay1[nl]

//cd         print*,'tx = ',ax2[nl],' ty = ',ay2[nl]
//cd         print*,' '
          lpL->lngth = 0;
          lpL->azm   = 0;
          lpL->rad   = 0;
          lpL->ref   = ++temp_refs ; // lref(ldarec(orig_lines(j)))
          lpL->desc  = lpPoly1->link_desc ; // ldadesc(orig_lines(j))
          lpL->type  = 2 ; //a straight line
      } 
      else  
      {

       for(j = lpGInfo->j; j <= lpGInfo->j + lpGInfo->i_count-1; j++)
       {   //for each side in this area
//d         print*,all_sides(j),' ',all_dir(j),' orig line ',
//d    +    orig_lines[j]
//c*        here I check for and discard the  one_sided offset filler line 
          lpAllS = lpAllSBase + j;
          lpPoly2 = lpPoly2Base + lpAllS->all_sides; 
          lpWork = lpWorkBase + lpAllS->orig_lines;
          if(lpWork->line_desc == RADICAL+1)
          {
            // print*,'found and discarded one_side filler line';
             goto S900;
          }
          nl++;
          lpL = lpLA->Lines + nl;
          lpPoly2   = lpPoly2Base + lpAllS->all_sides;
          lpL->x1   = lpPoly2->from_x_array;
          lpL->y1   = lpPoly2->from_y_array; 
          lpL->type = lpWork->type;
          lpL->desc = lpWork->ldadesc;
          lpL->rec  = lpWork->ldarec;
          if(lpWork->type == 2)
          { //straight line
            lpL->x2    = lpPoly2->to_x_array;
            lpL->y2    = lpPoly2->to_y_array;
            lpL->lngth = lpWork->ldalngth;
            lpL->azm   = lpWork->ldaazm;
            lpL->rad   = lpWork->ldarad; 
          }
          else //its a curve
          {
            lpWork     = lpWorkBase + lpAllS->orig_lines;
            lpL->rx    = lpWork->ldax2; //the original rad
            lpL->ry    = lpWork->lday2; //coordinates  
            lpL->x2    = lpPoly2->to_x_array; //the endpoint x coord
            lpL->y2    = lpPoly2->to_y_array; //the endpoint x coord
            lpL->azm   = LGETAZ(lpL->rx, lpL->ry, lpL->x1, lpL->y1);
            if(fabs(lpL->azm) < 1e-7)lpL->azm = 0e0;
            lpL->eazm    = LGETAZ(lpL->rx,lpL->ry,
                                     lpPoly2->to_x_array,
                                     lpPoly2->to_y_array); 
            if(fabs(lpL->eazm) < 1e-7) lpL->eazm = 0e0;
            dazm       = AZDF(lpL->azm,lpL->eazm,lpWork->ldalngth);
            lpL->lngth = dazm * lpWork->ldarad*
                         DSIGN(1,lpWork->ldalngth);
            lpL->rad  = lpWork->ldarad;     
          }
            temp_refs++;
            lpL->ref = temp_refs; // lref(ldarec(orig_lines(j)))
S900:     ;
       } //end of the for loop;

       if(lpGInfo->num_excl > 0)
//          gen_info(tru_num_areas,8) = 0 {the num_exclusions
       {  // we have exclusions
          for( j = 1; j <= lpGInfo->num_excl; j++)
          {   //write out the islands
//          gen_info(tru_num_areas,1) =  line_desc {the hit_line
             _fmemmove(lpPoly1->keyword,  &lpGInfo->AreaDesc, 4);
             _fmemmove(&lpPoly1->keyword[4], &j, 2);
              HASHF((int)lpPoly1->id3,lpPoly1->keyword,&lpPoly1->loc);


             if(lpPoly1->loc==0)
             {
                // print*,'*** Error trying to find exclusion ***';
                 *st = -1;
                 return;
             }
             HASHG((int)lpPoly1->id3,lpPoly1->keyword,lpPoly1->loc);
             _fmemmove(&lpPoly1->island_ref,&lpPoly1->keyword[6],4);

//c            print*,' '
//c            print*,'excluding area number ',island_ref
//c            print*,' '
             lpPoly1->l = 0;  
             
//     set memory of gen_info to point to the island_ref area
       lpGInfo = lpGInfoBase + lpPoly1->island_ref;

       for(k = lpGInfo->j; k <= lpGInfo->j + lpGInfo->i_count; k++)
       {   //for each side in this area
            
          lpPoly2 = lpPoly2Base + k; 
          // i = lpPoly2->orig_lines;              
          lpAllS = lpAllSBase + k;
/*             do k = gen_info(island_ref,7) +
     +              gen_info(island_ref,2) -1,
     +              gen_info(island_ref,7), -1 */
          nl++; 
                 
                 
                 if(lpPoly1->l == 0)
                 { // //addlpL-> link tolpL->n exclusion island
//c                  // print*,' '
//c                  // print*,' Adding link to the exclusion'
//c                  // print*,' '
                   lpPoly1->l = 1;;
                   ilink = nl;
                   lpL = lpLineBase + nl;
                   lpL->x1 = lpLineBase->x1 ; //want to point to the beginning of 
                   lpL->y1 = lpLineBase->y1 ; //of the first line   
                   lpAllS= lpAllSBase + k;
                   lpPoly2 = lpPoly2Base + lpAllS->all_sides;
                   lpL->x2 = lpPoly2->from_x_array ;
                   lpL->y2 = lpPoly2->from_y_array ;
                   lpL->lngth = LDIST(lpL->x1, lpL->y1,lpL->x2,lpL->y2);
                   lpL->azm   = LGETAZ(lpL->x1, lpL->y1,lpL->x2,lpL->y2);
                   lpL->rad   = 0;
                   temp_refs++;
                   lpL->ref  = temp_refs;
                   lpL->desc  = lpPoly1->link_desc; // ldadesc(orig_lines(j))
                   lpL->type  = 2; //a straight line
                   nl++;
                 }
//c                 print*,all_sides[k],' ',all_dir[k],' orig line ',
//c     +                   orig_lines[k]  
            
                 lpL = lpLineBase + nl;
                 lpAllS= lpAllSBase + k;
                 lpPoly2 = lpPoly2Base + lpAllS->all_sides;
                 lpWork = lpWorkBase + lpAllS->orig_lines;
                 
                 
                 lpL->x1 = lpPoly2->from_x_array ;
                 lpL->y1 = lpPoly2->from_y_array  ;
                 
                 if(lpWork->type == 2)
                 { // //straight line
                    lpL->x2 = lpPoly2->to_x_array ;
                    lpL->y2 = lpPoly2->to_y_array ;
                    lpL->lngth = 0;
                    lpL->azm = 0;
                    lpL->rad = 0;
                 } 
                 else 
                 { //its a curve
                    lpL->x2 = lpWork->ldax2 ; //the original rad
                    lpL->y2 = lpWork->lday2 ; //coordinates 
                    lpL->azm = LGETAZ(lpL->x2, lpL->y2, lpL->x1, lpL->y1);
                    
                    lpL->eazm =  LGETAZ(lpL->x2,lpL->y2,
                                        lpPoly2->to_x_array ,
                                        lpPoly2->to_y_array);
                             
                    dazm = AZDF(lpL->azm, lpL->eazm, lpWork->ldalngth);
                    
                    lpL->lngth = dazm * lpWork->ldarad  *
                         DSIGN(1, lpWork->ldalngth);

                    lpL->rad = lpWork->ldarad ;
                 }
                 temp_refs++;
                 lpL->ref  = temp_refs;
                 lpL->desc = lpWork->ldadesc;
                 lpL->type = lpWork->type ;
//c                print*,'PC x = ',ax1(nl),' PC y = ',ay1(nl)
//c                print*,'rad_x = ',ax2(nl),' rad_y = ',ay2(nl)
//c                print*,'azm = ',aazm(nl),' len = ',alngth(nl),
//c     +            ' rad = ',arad(nl)
             } // end of the for loop
             
             
             nl++;  //now the link back to the ocean shore
//cd            print*,' '
//cd            print*,'Adding the link back from the exclusion'
//cd            print*,' ' 
             lpL = lpLineBase + ilink;
             x = lpL->x2;
             y = lpL->y2;
             lpL = lpLineBase + nl;
             lpL->x1 = x;
             lpL->y1 = y;
             lpL->x2 = lpLineBase->x1; // back to the begining of the first
             lpL->y2 = lpLineBase->y1; // line of this area
             lpL->lngth = LDIST(lpL->x1, lpL->y1,lpL->x2,lpL->y2);
             lpL->azm   = LGETAZ(lpL->x1, lpL->y1,lpL->x2,lpL->y2);
             lpL->rad   = 0;
             temp_refs++; 
             lpL->ref  = temp_refs;
             lpL->desc  = lpPoly1->link_desc; // ldadesc(orig_lines(j))
             lpL->type  = 2; //a straight line
          } // end of the for each area
       } 
       
       
       if(ipos_link !=0)
       { // //at the positive link back
          nl++;  //now the link back to the ocean shore
//cd         print*,' '
//cd         print*,'Adding the link back from the external ',
//cd    +           'positive area.'
//cd         print*,' ' 
          lpL = lpLineBase + ipos_link;
          x = lpL->x2;
          y = lpL->y2;
          lpL = lpLineBase + nl;
          lpL->x1 = x;
          lpL->y1 = y;
          lpL->x2 = lpLineBase->x1;
          lpL->y2 = lpLineBase->y1;
//cd         print*,'fx = ',ax1(nl),' fy = ',ay1(nl)
//cd         print*,'tx = ',ax2(nl),' ty = ',ay2(nl)
//cd         print*,
          lpL->lngth =  LDIST(lpL->x1, lpL->y1, lpL->x2, lpL->y2);
          lpL->azm   = LGETAZ(lpL->x1, lpL->y1, lpL->x2, lpL->y2);
          lpL->rad   = 0;
          temp_refs++; 
          lpL->ref  = temp_refs;
          lpL->desc  = lpPoly1->link_desc; // ldadesc(orig_lines(j))
          lpL->type  = 2; //a straight line
       }
       i++;
    } //for the do while (i <=tru_num_areas)

  
  }     
  lpLA->NumSides = nl;
  *Out = lpLA;  
       return;
}

//c      //*****************************
       void OffsetClose(void)
{       int i;

           PolyBTClose (&lpPoly1->id1);
           PolyBTClose (&lpPoly1->bid3);
   
         P_TOL = IN_PTOL;
         lpPoly1->id1 = 0;
         lpPoly1->bid3 = 0;
         HASHC((int)lpPoly1->id2);
         lpPoly1->id2 = 0;
         HASHC((int)lpPoly1->id3);
         lpPoly1->id3 = 0;
//         force_tol ( P_TOL );
             for (i = 0;i < 12; i++)
             {
                GSSiGlobUlFree (&file[i].heap_ptr); 
                file[i].mem_ptr = 0;  
             }  
 
         return;
}
void offset_merge_areas(BOOL *deletions,long *st)
{
//c******* specifications ***********************************************
//c*                                                                    *
//c*       Program Summary                                              *
//c*       ------- -------                                              *
//c*      This routine applies some rules in an attempt to deduce a     *
//c*      single area from a possible myraid of areas.  This routine    *
//c*      should only be ed after offset_find_perimeter.ftn had     *
//c*      a chance to find the primary positive area(s).                *
//c*                                                                    *
//c*      What this routine looks for and eliminates is][                *
//c*                                                                    *
//c*      - Positive areas within another positive area                 *
//c*                                                                    *
//c*      - Modification Aug 90 added the eliminates of negative        *
//c*        areas within internal positive areas.                       *
//c*                                                                    *
//c*      - Negative areas within another negative area.                *
//c*                                                                    *
//c*      Only when the excluded area is definded using lines           *
//c*      NOT identified as boundary lines, will this routine           *
//c*      delete an area.                                               *
//c*                                                                    *
//c*      Arguments                                                     *
//c*      ---------                                                     *
//c*      deletions  L*4     (output) Set to TRUE if any areas        *
//c*                                  deleted by this routine           *
//c*       st        i*4     (output) the returncode                   *
//c*                                                                    *
//c*       Author][  Larry Anderson  Nov 88                              *
//c*                                                                    *
//c**********************************************************************
//c*                                                                    *

//#include 'ardata.ftn' ////umsc/include/ardata.ftn};

       long  i,  loc, island_ref, iholder, ParentCenY;
       short j;
       BOOL good_one;
//      print*,' ';
//      print*,'Now in offset_merge_areas ';
//      print*,' ';
       *deletions = FALSE; 
       
       if(lpPoly1->tru_num_areas == 1)
       { //
          *st = 0;
          return;
       } 
       if(lpPoly1->tru_num_areas < 1)
       { 
          *st = -1;
          return;
       } 


//      print*,' ';
//      print*,'Now checking for BAD direction exclusions';
//      print*,' ';
//c*     //Here I eliminate invalid exclusions and identify
//c*     //the areas that are used as valid exclusions
       for (i = 0;i < lpPoly1->tru_num_areas;i++)
       { 
          lpGInfo = lpGInfoBase + i;
          if(lpGInfo->CenY !=0)
          { //found a valid parent
            ParentCenY = lpGInfo->CenY;
            if(lpGInfo->num_excl > 0)
            { 
              for(j = 0; j <= lpGInfo->num_excl; j++)
              {   //write out the islands 
                 lpGInfo = lpGInfoBase + j;
                 iholder = (long)lpGInfo->AreaDesc; 
                 _fmemmove(&lpPoly1->keyword,&iholder,4);
                 _fmemmove(&lpPoly1->keyword[4], &j, 2);
                 HASHF((int)lpPoly1->id3,lpPoly1->keyword,&loc);
                 if(loc == 0)
                 {                  
//                     print*,'*** Error trying to find exclusion ***';
                     *st = -1;
                     return;
                 } 
                 HASHG((int)lpPoly1->id3,lpPoly1->keyword,loc);
                 _fmemmove(&island_ref, &lpPoly1->keyword[6], 4);
//                 print*,'excluding area number ',island_ref; 
                 lpGInfo = lpGInfoBase + island_ref;
                 if(lpGInfo->CenY !=0 && lpGInfo->CenY == ParentCenY)
                 { 
//                 print*,'Found invalid exclusion';
    //temporary               offset_delete_area(island_ref);
                   *deletions = TRUE;
                 } 
                 else 
                 { 
                   if(lpGInfo->CenY !=0)
                   {  //Its a valid exclusion, I mark it as such
                      lpGInfo->CenRefn = 0;
                   } 
                 }  
              }// end of a for loop
            }  // //Area had exclusions
          }  // //I found a valid parent area
       } //end of a for loop

//c*     Here I check for areas that are not valid exclusions
//c*     (using CEN_REFN[i] ) that are NOT using perimeter lines
//    print*,'Now checking for deletable non primary positive areas';
       for ( i = 0; i <= lpPoly1->tru_num_areas;i++)
       {
          lpGInfo = lpGInfoBase + i;
          if(lpGInfo->CenRefn !=0 && lpGInfo->CenY == 1)
          {    //not used as a valid exclusion,
//c*           //yet its an internal positive area
             good_one = FALSE;
             for ( j = lpGInfo->j; j <= lpGInfo->j + lpGInfo->i_count -1;j++)
             { 
                lpAllS = lpAllSBase + j;
                lpPoly2= lpPoly2Base + lpAllS->all_sides;
                if(lpPoly2->line_desc[1] < 0 || 
                   lpPoly2->line_desc[2] < 0)
                    good_one = TRUE; //this is a boundary effected area
             }
             if(!good_one)
             { // print*,' ';
               //print*,'Found a non-boundary positive area requiring'//
               // ' deletions.';
               //print*,'It was composed of the following original '//
               //        'lines';
  //temporary              offset_delete_area(i);
                *deletions = TRUE;
             }  //
          }  //
       } // end of the for loop

       if(deletions) *st = -1;

       return;
  }

   void offset_validate_area(long *st)
{        
//c******* specifications ***********************************************
//c*                                                                    *
//c*       Program Summary                                              *
//c*       ------- -------                                              *
//c*      This routine verifies whether or not all the lines that make  *
//c*      up the area(s) are used to complete what could be considered  *
//c*      to be a single area.  The definition of a single area         *
//c*      includes][                                                     *
//c*                                                                    *
//c*       - one area, no exclusions                                    *
//c*                                                                    *
//c*       - one or more independent areas that may be linked together  *
//c*                                                                    *
//c*       - area(s) not forming independent areas are exclusions       *
//c*         and their boundary lines are included among those used     *
//c*         to define the primary area(s)                              *
//c*                                                                    *
//c*      The tests I use to determine if a verified area(s) is present *
//c*      include the following][                                        *
//c*                                                                    *
//c*       - if all sides defining the area(s) comprise separate        *
//c*         positive areas //They will have the -1 in line_desc(array)} *
//c*                                                                    *
//c*       - if all sides used to define multiple areas are present     *
//c*         in the description of the primary positive area(s)         *
//c*         i.e., they define exclusions. The primary positive         *
//c*         areas will include all valid lines used to define all      *
//c*         existing areas.                                            *
//c*                                                                    *
//c*         Note][ Watch for multiple primary areas.                    *
//c*                                                                    *
//c*     cenY ends up carrying the code indicator describing the       *
//c*     type of area that it is.                                       *
//c*     if cenY(area) = 1 //it's a positive area                       *
//c*     if cenY(area) = 0 //it's a mixed direction area                *
//c*     if cenY(area) = -1 //it's a negative area                      *
//c*     if cenY(area) = -2 If its a negative area on boundary         *

//c*     if cenX(area) = -1 The area used a boundary line
//c*     if cenX(area) = 0 The area contains no boundary lines
//c*                                                                    *
//c*      Arguments                                                     *
//c*      ---------                                                     *
//c*       st        i*4     (output) the returncode                   *
//c*                          st = 0  when all lines on boundary        *
//c*                                  and in right direction            *
//c*                          st = -1 when a boundary line used going   *
//c*                                  against the grain                 *
//c*                          st > 0  Indicating the number of mixed    *
//c*                                  direction areas present           *
//c*                                                                    *
//c*                                                                    *
//c*       Author][  Larry Anderson  Nov 88                              *
//c*                                                                    *
//c**********************************************************************
//c*                                                                    *

      BOOL internal;
       long  i; 
       unsigned short j;
       char first_dir;
//cd      print*,' '
//cd      print*,'Entering offset_validate_area'
       *st = 0;
       internal = FALSE;
       lpAllS = lpAllSBase + 1;
       if(lpPoly1->tru_num_areas == 1 && lpAllS->all_dir == 'T')
          return;
       if(lpPoly1->tru_num_areas < 1)
       { 
           *st = -1;
           return;
       } 

//cc*     Here I determine whether an area is a positive, negative,
//c*     or mixed area.  I set cen_y accordingly
//c*            cen_y(num_area) = 1  If its a positive area
//c*            cen_y(num_area) = 0  If its a mixed area
//c*            cen_y(num_area) = -1 If its a negative area
//c*            cen_y(num_area) = -2 If its a negative area on boundary
        *st = lpPoly1->tru_num_areas;
       for ( i = 1 ; i <= lpPoly1->tru_num_areas;i++)
       { //for each area
         lpGInfo = lpGInfoBase + i;
          if(lpGInfo->i_count == 0)
          { 
             *st = -1;  //area not found
             return;
          }  
//c          print*,' '
//c          print*,'Found area with ',gen_info(i,2),' outward sides'
          lpAllS = lpAllSBase + lpGInfo->j;
          first_dir = lpAllS->all_dir; //direction of first line
          lpGInfo->CenX = 0; //default = not a boundary area
          if(first_dir == 'T')
             lpGInfo->CenY = 1; //used as area type indicator (positive dir)
          else 
             lpGInfo->CenY = -1; //negative direction area type indicator

//c*        //Now for each side of this particular area
          for ( j = lpGInfo->j; j < lpGInfo->j + lpGInfo->i_count;j++)
          {
//c             print*,all_sides(j),' ',all_dir(j),' orig line ',
//c     +       orig_lines(j) 
              lpAllS = lpAllSBase + j;
              lpPoly2 = lpPoly2Base + lpAllS->all_sides;
              
             if(lpPoly2->line_desc[1] < 0 ||  //a perimeter line
                lpPoly2->line_desc[2] < 0) lpGInfo->CenX = -1; //used
             if((lpPoly2->line_desc[1] < 0 ||  //a perimeter line
                 lpPoly2->line_desc[2] < 0) &&  //a perimeter line
                 lpAllS->all_dir == 'F')
             { 
//c*              //found a negative direction line on the perimeter boundary
//c                print*,'*** Found NEGATIVE boundary area'
                lpGInfo->CenY = -2; //negative boundary area
//c                do k = j+1, gen_info(i,7) + gen_info(i,2) -1
//c                   print*,all_sides[k],' ',all_dir[k],' orig line ',
//c     +             orig_lines[k]
//c                enddo
                *st = 66;
                goto S10;
              }
              if(first_dir !=lpAllS->all_dir &&
                    lpPoly2->line_desc[1] != lpPoly2->line_desc[2])
              {   //trouble
//c               //Found a multi directional sided area 
//c*              //that is not a link line
                    lpGInfo->CenY = 0;
                    goto S10;
              }   
          } //end of a for loop
S10:       if(lpGInfo->CenY == 0)*st++; //found another mixed area

       }//end of a for loop

//c*     //The test begins, but ends if a line was not found in an
//c*     //area associated with a primary positive area.

       return;
 }     
       
//**************************************************************//       
BOOL GetOffsetMem(lpArea InArea)
{  int i;
       for (i = 0;i < 9; i++)
       {  //release any old grips on offset memory
           GSSiGlobUlFree (&file[i].heap_ptr); 
           file[i].mem_ptr = 0;  
       }  
          //for each incoming point I want space for 2 lines to compensate for any
          //fillet lines I add.
          MaxLineBase = InArea->NumPts * 2;
          file[1].heap_ptr = GSSiGlobAlloc ( 285, GHND, sizeof(LINE)*( MaxLineBase)); 
          file[1].mem_ptr  = GlobalLock(file[1].heap_ptr);
          lpL = (lpLine) file[1].mem_ptr; 
          lpLineBase = lpL; 
          
          //here I get memory for the variables used by offset main
          file[2].heap_ptr = GSSiGlobAlloc ( 286, GHND, sizeof (PolyCom1) ); 
          file[2].mem_ptr  = GlobalLock(file[2].heap_ptr); 
          lpPoly1 = (lpPolyCom1) file[2].mem_ptr;
          lpPoly1Base = lpPoly1; 
          
          // for each point in the incoming area, I want to double the number
          // lpPoly2 holds the line storage arrays generated by this process  
          MaxPoly2Base = 3 * InArea->NumPts;
          file[3].heap_ptr = GSSiGlobAlloc ( 287, GHND, (long)sizeof (PolyCom2) *(long)MaxPoly2Base ); 
          file[3].mem_ptr  = GlobalLock(file[3].heap_ptr); 
          lpPoly2 = (lpPolyCom2) file[3].mem_ptr;
          lpPoly2Base = lpPoly2;
          MaxWorkBase = InArea->NumPts*3;
          file[4].heap_ptr = GSSiGlobAlloc ( 288, GHND, (long)sizeof (WorkSpace)* (long)(MaxWorkBase) ); 
          file[4].mem_ptr  = GlobalLock(file[4].heap_ptr); 
          lpWork = (lpWS) file[4].mem_ptr; 
          lpWorkBase = lpWork;
          MaxNodeArrBase = InArea->NumPts * 3;
          file[5].heap_ptr = GSSiGlobAlloc ( 289, GHND, sizeof (NodeArray)* (MaxNodeArrBase) ); 
          file[5].mem_ptr  = GlobalLock(file[5].heap_ptr); 
          lpNodeArr = (lpNA) file[5].mem_ptr;
          lpNodeArrBase = lpNodeArr;
          MaxNumAreas = InArea->NumPts/10;
          if(MaxNumAreas < 128) MaxNumAreas = 128; //have enough room for at least fourteen areas
          file[6].heap_ptr = GSSiGlobAlloc ( 290, GHND, sizeof (gen_info)* MaxNumAreas ); 
          file[6].mem_ptr  = GlobalLock(file[6].heap_ptr); 
          lpGInfo = (lpGenInfo) file[6].mem_ptr;
          lpGInfoBase = lpGInfo;
          
          MaxAllSides = InArea->NumPts*6;
          file[7].heap_ptr = GSSiGlobAlloc ( 291, GHND, sizeof (AllSides)* (MaxAllSides) ); 
          file[7].mem_ptr  = GlobalLock(file[7].heap_ptr); 
          lpAllS = (lpAS) file[7].mem_ptr;
          lpAllSBase = lpAllS;  
          MaxNewBase = InArea->NumPts * 32;
          file[8].heap_ptr = GSSiGlobAlloc ( 292, GHND, sizeof (TempArea)* (MaxNewBase) ); 
          file[8].mem_ptr  = GlobalLock(file[8].heap_ptr); 
          lpNewBase = (lpTA) file[8].mem_ptr;
          lpNew = lpNewBase;
                                 
return TRUE;
}   
void  offset_find_link_partners(long *st)
{
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       this routine pairs up the first line before a link line with *
//c*       its corresponding last line after a link line to initiate    *
//c*       a one shot fillet attempt.  This can possibly close an area  *
//c*       linked to another.                                           *
//c*                                                                    *
//c*       I match endpoints of the link lines to find their partners.  *
//c*                                                                    *
//c*      argument description                                          *
//c*       -------- -----------                                         *
//c*                                                                    *
//c*      nl     i*4      (output) the number of links present          *
//c*                                                                    *
//c*      st     i*4      (output) If unable to locate partner for all  *
//c*                               links, this variable set <> 0        *
//c*                                                                    *
//c*      Author Larry Anderson Aug 89                                  *
//c*                                                                    *
//c**********************************************************************
//c*                        
         
       BOOL gotta_link = FALSE;
       lpWS lpNext,lpLink, lpLastNormalLine;
       long i,j;          
       lpLk->nPartners = 0 ; //number of link lines present among the area lines
       *st = 0;
       for(i = 0; i <= lpPoly1->num_sides;i++)
       {
         lpWork = lpWorkBase + i;
         if(lpWork->ldadesc == lpPoly1->link_desc)
         {  //found a link line
            if(gotta_link) goto S10;//Found adjacent link lines 
            gotta_link = TRUE;
            for( j = lpLk->nPartners; j >=0; j--)
            { //for each set of link endpoints already in existance
//c*            {I want to see if I already have this one.  If I do
//c*            {I want to id the first normal line after this link
              lpLink = lpLk->LBG[j]; //the line before stepping on the link
              if(lpLink->ldax2 == lpWork->ldax2 &&
                 lpLink->lday2 == lpWork->lday2)
              { // { I'm back to a point needing a partner
                if(i+1 <= lpPoly1->num_sides)
                {
                 lpNext = lpWorkBase + i+1;
                 if(lpNext->ldadesc != lpPoly1->link_desc &&
                    lpNext->ldadesc != 0 &&
                    lpNext->valid == 0)
                      lpLk->LACB[j] = lpNext; 
                 else
                      lpLk->LACB[j] = 0;
                }
                else
                      lpLk->LACB[j] = 0; //point back to the begining of
                      // the first line in the listing of lines    
                goto S10;
              }
            } //end of the for lop    
//c*        {get here if the link is initially discovered
            lpLk->nPartners++;
            if(lpLk->nPartners > 31)
            {
              GSSiMsgBox(0,"Error... Exceeded the Maximum Number\
Link Lines in an Area.","Offset 32 Links Maximum Error",MB_ICONSTOP,0);
              *st = -1;
              return;
            }  
            lpLk->LBG[lpLk->nPartners] = lpLastNormalLine; 
            lpLk->LACB[lpLk->nPartners] = 0;
            //now I walk the link line(s) till I get to the
            //next normal line
            //if I get here, I didn't find the next regular line
            //so I go back to the start of the first link partner
         }
         else
         { 
           if(lpWork->ldadesc !=0  && lpWork->valid == 0)
                lpLastNormalLine = lpWork;
           gotta_link = FALSE;
          //keep track of the last valid regular line 
          
         }  
         S10:;    
      }


      return;
}      


 int  OffsetFindPerimeter(void)
 {
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//C*       ------- -------                                              *
//c*       Working with the output from poly_areas, this routine        *
//c*       attempts to render multiple areas into a single area         *
//c*       record.                                                      *
//c*                                                                    *
//C*       This routine finds the perimeter of multiple areas,          *
//C*       when multiple areas are defined by poly_areas.  It does      *
//C*       this by locating all lines with a line_desc = -1 on one      *
//C*       of the lines sides.  (poly_areas gives continental boundaries*
//C*       a -1 line_desc on the side not defined as within an area.    *
//c*                                                                    *
//c*       After I find them all, I track around them finding the       *
//c*       leftest line as I generate a positive area.                  *
//c*                                                                    *
//c*       Any leftest line I track must travel the same direction      *
//c*       that I am going.  If I find a leftest line going against     *
//c*       the grain, I  poly_eliminator with it.  If I eliminate   *
//c*       any lines, I  poly_areas before returning.               *
//c*                                                                    *
//c*       Having found the perimeter while creating a positive area,   *
//c*       this routine returns.                                        *
//c*                                                                    *
//c*                                                                    *
//c*       author][  Larry Anderson  Nov 88                              *
//c*                                                                    *
//c**********************************************************************
//c*

      int i, j , passes = 1;
      long st; 
      BOOL      deletions = FALSE,  gotta_pos = FALSE,     
      gotta_neg = FALSE,  gotta_mixed = FALSE;

      lpPoly1->debugging = FALSE;
 
//c*    {If the lines form a single area, though more than one
//c*    {tru_num_areas are present, e.g., can happen if independent
//c*    {positive areas are present, and/or exclusions to an area
//c*    {are present.

S10:    offset_validate_area(&st);

      if(st ==0 )return 0; //everything forms a single area.

//c*    cen_y now contains a classification for each area
//c*    The order of battle is][
//c*    -- Look for and delete any negative area adjacent to the
//c*       perimeter.
//c*       If any are found and deleted... Call poly_areas again
//c*    -- Look for mixed areas.  If found, merge with all adjacent
//c*       valid positive areas, by eliminating a line common to
//c*       both areas.  If any mixed areas are present... and a common
//c*       line between them was eliminated...  poly_delete_deadends
//c*       and poly_areas.  Go back to the beginning of the routine.
//c*    -- If it doesn't find any negative areas adjacent to the
//c*       boundary and... it doesn't have any more mixed areas...
//c*       it should be done.


//c*    Here I look for and delete any negative areas adjacent to the
//c*    perimeter
      for (i = 1;i <= lpPoly1->tru_num_areas ;i++)
      { //for each area
         lpGInfo = lpGInfoBase + i;
         if(lpGInfo->i_count == 0) return -1;
         lpAllS = lpAllSBase + lpGInfo->j;
        switch (lpGInfo->CenY)
        {
         case -2: //  {its gotta be deleted
         {
            OffsetDeleteArea(i);
            deletions = TRUE; 
            break;
         } 
         case 1:
         {  //this is a positive area
          gotta_pos = TRUE;
          if(lpGInfo->CenX == 0)
          {  //its an internal positive area
//c*          {if it has an islands I want to delete them
            for( lpPoly1->j = 1; lpPoly1->j <= lpGInfo->num_excl; lpPoly1->j++)
            {   //for any islands
              _fmemmove(lpPoly1->keyword,  &lpGInfo->AreaDesc, 4);
              _fmemmove(&lpPoly1->keyword[4], &lpPoly1->j, 2);
              HASHF((int)lpPoly1->id3,lpPoly1->keyword,&lpPoly1->loc);
              if(lpPoly1->loc==0) return -1;
              HASHG((int)lpPoly1->id3,lpPoly1->keyword,lpPoly1->loc);
              _fmemmove(&lpPoly1->island_ref,&lpPoly1->keyword[6],4);
              OffsetDeleteArea((int) lpPoly1->island_ref);
              deletions = TRUE;
            } 
          } 
         } 
        } //end of the switch
      }//end of the for loop

      if(deletions) goto S300;
      
//c     here I eliminate lines between adjacent mixed areas
      for (i = 1;i <= lpPoly1->tru_num_areas ;i++)
      { //for each area
         lpGInfo = lpGInfoBase + i;
         if(lpGInfo->CenY >= 0 && lpGInfo->CenX ==0)
         { //it's internal mixed or positive area
            for(j = lpGInfo->j; j <= lpGInfo->j + lpGInfo->i_count; j++)
            {   //for each side in this area
              lpAllS = lpAllSBase + j;
              lpPoly2 = lpPoly2Base + lpAllS->all_sides; 
              if(lpPoly2->line_desc[1] == i)
                 lpPoly1->l = lpPoly2->line_desc[2] ; //the area outside the area
              else 
                 lpPoly1->l = lpPoly2->line_desc[1];
              //here I check on the area type on the other
              //side of the line   
              if(lpPoly1->l > 0 && 
                 lpPoly1->l <= lpPoly1->tru_num_areas)
              {   
                lpGInfo = lpGInfoBase + lpPoly1->l;
                if(lpPoly1->l > 0 && lpGInfo->CenY >= 0 )
                { //found a neighboring mixed or positive area
                  lpGInfo = lpGInfoBase + i;
                  *this_node = 0;
                  OffsetEliminator(0);
                  deletions = TRUE;
                }
                else
                {
                  lpGInfo = lpGInfoBase + i;
                }  
              }       
            }
         }  
      }
      
      if(deletions) goto S300;
      

//c*    well I tried everything in the book to make a valid area
//c*    here I bust open any internal mixed areas
      gotta_pos = FALSE;
      gotta_neg = FALSE;
      gotta_mixed = FALSE;
      for (i = 1;i <= lpPoly1->tru_num_areas ;i++)
      { //for each area
        lpGInfo = lpGInfoBase + i;
        if(lpGInfo->CenY == 1) gotta_pos = TRUE;
        if(lpGInfo->CenY == 0 && lpGInfo->CenX == 0) gotta_mixed = TRUE;
      } 
      
      if(gotta_pos && gotta_mixed)
      { 
         deletions = TRUE;
         for (i = 1;i <= lpPoly1->tru_num_areas ;i++)
         { //for each area
           lpGInfo = lpGInfoBase + i;
            if(lpGInfo->CenY == 0 && lpGInfo->CenX == 0)
            {
               OffsetDeleteArea(i);
            } 
         }
      } 
      if(deletions) goto S300; 
      
//if I get here, all normal conventions for popping areas open have failed.
// I now break open mixed areas contain boundary lines by finding 
// and eliminating a line within the area that isn't a boundary line .
// An area of this type is identified by CenY == 0 && CenX == -1       
      gotta_mixed = FALSE;
      for (i = 1;i <= lpPoly1->tru_num_areas ;i++)
      { //for each area
        lpGInfo = lpGInfoBase + i;
        if(lpGInfo->CenY == 0 && lpGInfo->CenX == -1) 
        {
           deletions = TRUE; 
           for (lpPoly1->l = lpGInfo->j;lpPoly1->l < lpGInfo->i_count + lpGInfo->j;lpPoly1->l++)
           { //Look for the first line that's not a boundary line
             lpAllS = lpAllSBase + lpPoly1->l;
             lpPoly2 = lpPoly2Base + lpAllS->all_sides; 
             if(lpPoly2->line_desc[1] > 0 && 
                lpPoly2->line_desc[2] > 0 && 
                lpPoly2->valid)
               {// found a line I can delete
                  OffsetEliminator(0);
                  goto S311;
               }
           } //end of the for loop 
         }  
S311:    ;        
      }//end of the for loop 
      
      if(deletions) goto S300; 

//c*    Here I look at all the lines that make up the areas
//c*    for a line with a negative value on its left side
//c*    and used with_the_grain to define a positive area
//c*    as indicated by a "T" in the all_dir(n) array.
//c*    line_desc(num_lines,1:2) array elements.

      for (i = 1;i <= lpPoly1->tru_num_areas ;i++)
      { //for each area
          lpGInfo = lpGInfoBase + i;
          lpAllS = lpAllSBase + lpGInfo->j;
          lpPoly2 = lpPoly2Base + lpAllS->all_sides; 
          if(lpPoly2->line_desc[1] < 0 && lpPoly2->valid)
          { 
//c*         {left side of a line is on the perimeter
           if(lpAllS->all_dir == 'T')
           {  //going with the grain
//c*            {the lines other side was tracked with_the_grain
//c*            {when creating a positive area.  I can use
//c*            {this line, going with_the grain to find the perimeter
              *sx = lpPoly2->from_x_array;
              *sy = lpPoly2->from_y_array; 
              *occ = 0;
              lpPoly1->hit_line = lpAllS->all_sides;
              lpPoly1->i_start = lpAllS->all_sides;
              lpPoly1->i_count = 1;
              lpNew = lpNewBase + 1;
              lpNew->Poly2Loc = lpAllS->all_sides;
              lpNew->direction = 'T';
//c              print*,'Found start line ',orig_lines[i],
//c     +              ' for perimeter tracking'
//c*            I want the curr_azm to have the lines'
//c*            azimuth as it points away from the to_node
              lpPoly1->curr_azm = LGETAZ
                 (lpPoly2->to_x_array,   lpPoly2->to_y_array,
                  lpPoly2->from_x_array, lpPoly2->from_y_array);
              goto S350;
           }  
        }  
     }

S350:    st = 0;
//c*      {When I get here, I made it around the perimeter of
//c*      {the area.  I  poly_areas with whats left to let
//c*      {it again find all the areas.

S300:    if(deletions)
         { 
           deletions = FALSE;
           for(i = 1;i <= lpPoly1->num_sides;i++)
           {
              lpPoly2 = lpPoly2Base + i;
              lpPoly2->line_desc[1] = 0  ; //this wipes out all previous work
              lpPoly2->line_desc[2] = 0  ; //identifying areas.  -perhaps unwise
           }
            if(PolyEliminateDeadends()  != 0) goto S499; ;
            if(!PolyPurge())goto S499; //eliminates deaded lines
            PolyMinValidLine();
            st = PolyAreas();
            poly_check_for_islands();
        } 
        if(st !=0 || lpPoly1->tru_num_areas == 1)return 0;

//c*      {if poly_areas found an area that can be ed a single
//c*      {area, this routine has done its' job.
//c*      {The tests I use to determine if the job is complete
//c*      {include the following two parts][
//c*      {   -if all sides defining the area(s) comprise separate
//c*      {    positive areas (They will have the -1 in line_desc(array)
//c*      {   -if all sides used to define multiple areas are present
//c*      {    in the description of the primary positive area(s)
//c*      {    i.e., they define exclusions. The primary positive
//c*      {    area number will be in line_desc(array) for all lines
//c*      {    Note][ Watch for multiple primary areas.

        passes++;
        if(passes > 5) goto S499;
        goto S10 ; //check for lpPoly2->validsingle area

S499:    if(lpPoly1->tru_num_areas > 1)
         { 
            lpPoly1->tru_num_areas = 1;
            return 0;
         } 
         return -1;
    }
         
 void    PolyMinValidLine(void)
 {
        lpPoly1->min_start = 1;
         while (lpPoly1->min_start <= lpPoly1->num_sides)
         { 
           lpPoly2 = lpPoly2Base + lpPoly1->min_start; 
           if((lpPoly2->line_desc[1] == -1 ||
               lpPoly2->line_desc[2] == -1) &&
               lpPoly2->valid) goto S10;
           lpPoly1->min_start++;
         }
        return;
S10:    lpPoly2 = lpPoly2Base + lpPoly1->min_start;
        lpPoly1->min_valid_line = lpPoly1->min_start ;
        lpPoly1->first = TRUE;
        
 
 return;
 }        
 int OffsetThese( lpArea lpA)
 {
//c******* specifications ***********************************************
//c*                                                                    *
//c*       program summary                                              *
//c*       ------- -------                                              *
//c*       this routine unloads ltable into ldatable for the $locate    *
//c*       when a user requests an offset area be created from an       *
//c*       existing area.  Each line's offset is first created. Lines   *
//c*       forming turns away from the offset have the gap between      *
//c*       their offsets filleted with a curve. The original area record*
//c*       is in ltable.ftn, as the offsets and fillets are             *
//c*       generated I place them in ldatable.ftn and when completed    *
//c*       the resulting offset area is placed in ATABLE.FTN            *
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
//c*      radical i*2 (input)  The symbol dictionary description        *
//c*                               number used for collapsed curves     *
//c*       whos_callen I*2      (input)  0 = rcdms, 1 = igs             *
//c*                                                                    *
//c*       type       i*4 (input)  User must indicate the type of       *
//c*                               object being offset                  *
//c*                                     0 = an alignment               *
//c*                                     1 = area                       *
//c*                                                                    *
//c*                                                                    *
//c*       author:  Larry Anderson  Dec 88                              *
//c*                                                                    *
//c**********************************************************************
//c*

//#include "cnstnt.h"   //umsc/include/cnstnt.ftn} 
         int st; 
         long   i,  irc, isave;
         unsigned short k, fd=0, whos_en=0;
         double  oldrad, sx, sy, r8len, roff,
          zx1[4], zy1[4], azm[4], gap, OldLen, 
          OldToX, OldToY, *lpX[4], *lpY[4];
         lpDLine  lpL; 
         lpWS lpLLast,lpLNext, lpWork, lpLFillet;
         DPOINT odp,dp;
         
         for(i=0;i<4;i++)
          {lpX[i] = &zx1[i];
           lpY[i] = &zy1[i];}
       lpPoly1->dminx = 9e9;
       i = 1 ;
       
       roff = lpA->offset_dist ;
       
       if(RIGHT_SIDE_ONLY)
       { 
          lpPoly1->num_sides = 0;
          st = OffsetAlignment(lpA);
          lpPoly1->num_orig_sides = lpPoly1->num_sides;
         return st;
       }  
       for( k = 0;k < lpA->NumSides;k++)
       { 
         lpL = lpA->lpDLineBase + k;
         if(k > 0)lpLLast = lpWorkBase + (i-1);
         lpLNext = lpWorkBase + (i+1);
         lpWork  = lpWorkBase + i; 
         lpWork->intxs = 0;
         lpPoly2 = lpPoly2Base + k;
         if(lpL->Desc == lpPoly1->link_desc)
         { 
            lpWork->ldadesc= lpL->Desc;
            lpWork->ldarec = (short)i;
            lpWork->ldaref= lpL->Refn;
            lpWork->ldax1 = lpL->F.x;
            lpWork->lday1 = lpL->F.y;
            lpWork->ldax2 = lpL->T.y;
            lpWork->lday2 = lpL->T.y;
            goto S99; //skip link lines
         }  

         lpWork->ldaref = k; //holds which line was used to create it
        // lpWork->valid = 0;
         switch (lpL->Type)
         {
           case 2: //a straight line 
           
            lpWork->ldaazm =  LGETAZ(lpL->F.x,  lpL->F.y,
                                     lpL->T.x,  lpL->T.y);
            odp.x = lpL->F.x;
            odp.y = lpL->F.y;
            azm[0] = LTWOPI(lpWork->ldaazm - HALFPI);
            dp = dnewpt(odp,azm[0],roff);
            lpWork->ldax1 = dp.x;
            lpWork->lday1 = dp.y;                   
            odp.x = lpL->T.x;
            odp.y = lpL->T.y;
            azm[0] = LTWOPI(lpWork->ldaazm - HALFPI);
            dp = dnewpt(odp,azm[0],roff);
            lpWork->ldax2 = dp.x;
            lpWork->lday2 = dp.y;              
                         
            lpWork->ldalngth = LDIST(lpWork->ldax1, lpWork->lday1,
                                     lpWork->ldax2, lpWork->lday2);
                             
            if(lpWork->ldalngth < lpPoly1->tol_fac)goto S100;
            lpWork->ldarad   = 0;
            lpWork->ldadesc  = lpL->Desc;
            lpWork->type  = lpL->Type;
            lpWork->ldarec   = (short)i;
//d           print*,' '
//d           print*,' line ',i,' offset description'
//d           print*,' from x = ',ldax1[i],' from y = ',lday1[i],
//d           print*,' to x = ',ldax2[i],' to y = ',lday2[i]
//d           print*,' line azm = ',ldaazm[i],
//d    +             ' length = ',ldalngth[i]
            break;
          case 3:   //a curve
//c*          //here I get the length of the radius  
            if(fabs(lpL->lngth) < lpPoly1->tol_fac)goto S100;
//c           //next I calc the azimuth of the line from the rad to the PC
            lpWork->type  = lpL->Type;
            lpWork->ldarec  = (short)i ;
            oldrad = LDIST(lpL->F.x, lpL->F.y, lpL->T.x, lpL->T.y);
            lpWork->ldarad = oldrad - (roff * DSIGN(1e0, lpL->lngth));
//d           print*, ' '
//d           print*,'The original radius = ',oldrad
//d           print*,'The offset radius = ',ldarad[i]
            if(lpWork->ldarad < 0)
            {
//c*             //the curve collapsed into a point on a line from
//c*             //the original curves PT through the RAD, the offset distance
//c*             //from the PT. The point is given the azimuth of the
//c*             //original curve at its PT.
//c*             //here I calculate the coords of the PT  
               r8len = lpL->lngth;
//d              print*,'original curve length = ',r8len

//c*             //Here I calculate the PT of the existing curve
//c*             //placing the coordinates in sx and sy 
                gap = fabs(r8len);
                LOL8(&lpL->F.x,  &lpL->F.y , &lpL->T.x , &lpL->T.y ,
                     &r8len,&gap, &sx, &sy, lpL->Type);

               lpWork->ldarad   = fabs(roff); //** set it to the radius **}  //
               lpWork->ldaazm = LGETAZ(sx,sy, lpL->T.x, lpL->T.y);
//c*                      //azm of line from old PT (new rad) to old rad
               lpWork->ldalngth = PY*DSIGN(lpWork->ldarad,0e0-lpL->lngth);
//c*                          //** it becomes a half circle
//c*             //here I calculate the coords of a new PC point 
                odp.x = sx;
                odp.y = sy;
                dp = dnewpt(odp,lpWork->ldaazm, fabs(roff));
                lpWork->ldax1 = dp.x;
                lpWork->lday1 = dp.y;
                          
               lpWork->ldax2    = sx; //the old PT x coord (now the new rad x
               lpWork->lday2    = sy; //the old PT y coord (now the new rad y
               lpWork->ldadesc  = radical;

            } 
            else 
            { //I redefine a curve using the existing radius
//c*             //Here I get the azm from the rad to the PC
               lpWork->ldaazm = LGETAZ(lpL->T.x, lpL->T.y,
                                              lpL->F.x, lpL->F.y);
//c*             //I calculate the new PC, put coords in ldax1[i] lday1[i]
                odp.x = lpL->T.x;
                odp.y = lpL->T.y;
                dp = dnewpt(odp,lpWork->ldaazm, lpWork->ldarad);
                lpWork->ldax1 = dp.x;
                lpWork->lday1 = dp.y;
              if(oldrad !=0)
                  lpWork->ldalngth = lpL->lngth * (lpWork->ldarad/oldrad);
               else 
                  lpWork->ldalngth = lpL->lngth;
               lpWork->ldax2   = lpL->T.x;  //radius coords
               lpWork->lday2   = lpL->T.y;  //stay the same
               lpWork->ldadesc = lpL->Desc;
            }   
            break;
          default:
        
              //invalid record type in the midst of the mess
              return -1;
            
            
          } //end of the switch
          //  lpGInfo->min_x = __min(lpGInfo->min_x,__min(lpWork->ldax1,lpWork->ldax2));
         //   lpGInfo->max_x = __max(lpGInfo->max_x,__max(lpWork->ldax1,lpWork->ldax2));
         //   lpGInfo->min_y = __min(lpGInfo->min_y,__min(lpWork->lday1,lpWork->lday2));
         //   lpGInfo->max_y = __max(lpGInfo->max_y,__max(lpWork->lday1,lpWork->lday2));



S111:  if(k > 0 && lpLLast->ldadesc != lpPoly1->link_desc)
       { // 
        //debug stuff
        
        //end of debug stuff
       //        if(lpLLast->ldarec >= 14)
       //        { 
        //          irc = 0;
       //        }   
           offset_intx_these_two(lpLLast, lpWork, lpX, lpY, &gap, &irc) ;
         switch (irc)
         {
           case 1:
           case 2:
//c*            I recalculate the line descriptions 
//c*            Now I end line I-1 at the same point
              if(lpLLast->type == 2)
              { 
                OldToX = lpLLast->ldax2;
                OldToY = lpLLast->lday2;
                OldLen = lpLLast->ldalngth;
                lpLLast->ldax2 = *lpX[0];
                lpLLast->lday2 = *lpY[0];
                lpLLast->ldalngth = LDIST(lpLLast->ldax1, lpLLast->lday1,
                                          lpLLast->ldax2, lpLLast->lday2);
               if(lpLLast->ldalngth == 0e0)
               { //Just created a zero length line
                 goto S98; 
               
               }
              }
              else 
              {
                OldLen = lpLLast->ldalngth;
                azm[1] = LGETAZ(lpLLast->ldax2,lpLLast->lday2,*lpX[0],*lpY[0]);
                azm[2] = AZDF(lpLLast->ldaazm,azm[1],lpLLast->ldalngth) ;
                if(fabs(azm[2]) > TWOPI-1e-3 &&
                   fabs(lpLLast->ldalngth) > 1e-2 &&
                   fabs(lpLLast->ldalngth) < lpLLast->ldarad) azm[2]= 0e0;
                lpPoly1->sign = DSIGN(1e0,lpLLast->ldalngth);
                lpLLast->ldalngth =  azm[2]*lpLLast->ldarad * lpPoly1->sign;
                if(lpLLast->ldalngth == 0)goto S98; //just created a zero length line

              }  // 
//c*            for line I I recalculate it's starting position
              if(lpWork->type == 2)
              { ////it's a straight line
                lpWork->ldax1 = *lpX[0];
                lpWork->lday1 = *lpY[0];
                lpWork->ldalngth = LDIST(lpWork->ldax1, lpWork->lday1,
                                    lpWork->ldax2, lpWork->lday2);
              } 
              else 
              {
                azm[1] = LGETAZ(lpWork->ldax2, lpWork->lday2, *lpX[0],*lpY[0]) ;
                azm[2] = AZDF(lpWork->ldaazm, azm[1], lpWork->ldalngth) ;
                lpPoly1->sign = DSIGN(1e0,lpWork->ldalngth);
                if(fabs(azm[2]) > TWOPI-1e-3 &&
                   fabs(lpWork->ldalngth) > 1e-2 &&
                   fabs(lpWork->ldalngth) <lpWork->ldarad)azm[2]= 0e0;
                lpWork->ldalngth = lpWork->ldalngth - (azm[2]*lpWork->ldarad*lpPoly1->sign);
                lpWork->ldax1 = *lpX[0];
                lpWork->lday1 = *lpY[0];
                lpWork->ldaazm = azm[1];
              }
              break;
           default:  //didn't get a hit  
             if (lpLLast->ldadesc !=radical)
             {// attempt a fillet     
               lpLFillet = lpWorkBase + (i+2);
                irc = OffsetCreateFillet (lpLLast, lpWork, lpLFillet,
                          whos_en, fd,  lpA->type);
               if(irc == 1)
               {                
//c*                gotta a hit with a fillet
//c*                I calculate the endpoint of line i-2
//c*                and the beginning of i-1
//c*                the fillet is now stored in the i'th element of the lda arrays
//c*                I move the line infor from I to I+1 
                  lpLNext->ldax1    = lpWork->ldax1;
                  lpLNext->lday1    = lpWork->lday1;
                  lpLNext->ldax2    = lpWork->ldax2 ;
                  lpLNext->lday2    = lpWork->lday2;
                  lpLNext->ldaazm   = lpWork->ldaazm;
                  lpLNext->ldarad   = lpWork->ldarad;
                  lpLNext->ldalngth = lpWork->ldalngth;
                  lpLNext->type     = lpWork->type; //a curved line
                  lpLNext->ldadesc  = lpWork->ldadesc;
                  lpLNext->ldarec   =(short) i+1;
                  lpLNext->valid    = 0;
                  lpLNext->ldaref   = lpWork->ldaref;
//c*                and I transfer the fillet record into position I
                  lpWork->ldax1    = lpLFillet->ldax1;
                  lpWork->lday1    = lpLFillet->lday1;
                  lpWork->ldax2    = lpLFillet->ldax2;
                  lpWork->lday2    = lpLFillet->lday2;
                  lpWork->ldaazm   = lpLFillet->ldaazm;
                  lpWork->ldarad   = lpLFillet->ldarad;
                  lpWork->ldalngth = lpLFillet->ldalngth;
                  lpWork->type     = lpLFillet->type; //a curved line
                  lpWork->ldadesc  = 250;
                  lpWork->ldarec   = (short) i ;
                  lpWork->ldaref   = lpLFillet->ldaref;
                  
                 // lpWork->valid = 0;
                  i++;
               }  // endif else { ; ////i-1 is a unhit radical  
             }  
             else //     I eliminate it from the line array
             {
S98:             lpLLast->ldax1    = lpWork->ldax1;
                 lpLLast->lday1    = lpWork->lday1;
                 lpLLast->ldax2    = lpWork->ldax2 ;
                 lpLLast->lday2    = lpWork->lday2;
                 lpLLast->ldaazm   = lpWork->ldaazm;
                 lpLLast->ldarad   = lpWork->ldarad;
                 lpLLast->ldalngth = lpWork->ldalngth;
                 lpLLast->type  = lpWork->type; 
                 lpLLast->ldadesc  = lpWork->ldadesc;
                 lpLLast->ldarec   = (short)i-1 ;
                 lpLLast->valid    = 0  ;
                 lpLLast->ldaref   = lpWork->ldaref;
                 if(i-2 > 0)
                 { 
                    lpLLast = lpWorkBase +(i-2);
                    lpLLast->ldalngth = OldLen;;
                    if(lpLLast->type == 2)
                    { 
                      lpLLast->ldax2 = OldToX;;
                      lpLLast->lday2 = OldToY;;
                    }   
                 }  

                 i--;
                 goto S111;
             }
          }      
       }  
S99:      i++;
S100:    continue;
       } 
       lpPoly1->num_sides = i -1  ; 
    //   for (i = 0;i <= lpPoly1->num_sides+1;i++)
    //   {
    //       lpWork = lpWorkBase + i;
    //   } 
       if(lpA->type == 0 && !LEFT_SIDE_ONLY)
       {
          st = OffsetAlignment( lpA) ; 
          lpPoly1->num_orig_sides = lpPoly1->num_sides;   //now using array
          return st;
       }   
       if (lpA->type == 1)
         { 
//c*       I make a one shot attempt of intersecting the last line
//c*       with the first
         isave = lpPoly1->num_sides ;
       
         if(lpWork->ldadesc != radical) //was ==
         {  
            lpLLast = lpWorkBase + lpPoly1->num_sides; //the last line in the array
            lpWork  = lpWorkBase + 1; //the first line  
            offset_intx_these_two(lpLLast,lpWork,lpX,lpY,&gap,&irc) ;
           if(irc > 0 && irc <3)
           { ////got a hit 
//c*            I recalculate the line descriptions 
//c*            for line I I recalculate it's starting position 
              zx1[1] = *lpX[0];
              zy1[1] = *lpY[0];
              if(lpWork->type == 2)
              { ////it's a straight line
                lpWork->ldax1 = zx1[1];
                lpWork->lday1 = zy1[1];
                lpWork->ldalngth = LDIST(lpWork->ldax1, lpWork->lday1,
                                         lpWork->ldax2, lpWork->lday2);
              } 
              else 
              {
                azm[1] = LGETAZ(lpWork->ldax2,lpWork->lday2,zx1[1],zy1[1]);
                azm[2] = AZDF(lpWork->ldaazm,azm[1],lpWork->ldalngth);
                if(fabs(azm[2]) > TWOPI-1e-6 &&
                   fabs(lpWork->ldalngth) > 1e-2)   azm[2]= 0E0;
                lpPoly1->sign = DSIGN(1e0,lpWork->ldalngth);
                lpWork->ldalngth = lpWork->ldalngth - 
                                     (azm[2]*lpWork->ldarad*lpPoly1->sign);
                lpWork->ldax1 = zx1[1];
                lpWork->lday1 = zy1[1];
                lpWork->ldaazm = azm[1];
              }   
//c*            Now I end line I-1 at the same point
              if(lpLLast->type == 2)
              { 
                lpLLast->ldax2 = zx1[1];
                lpLLast->lday2 = zy1[1];
                lpLLast->ldalngth = LDIST(lpLLast->ldax1, lpLLast->lday1,
                                          lpLLast->ldax2, lpLLast->lday2);
              } 
              else 
              {
                azm[1] = LGETAZ(lpLLast->ldax2,lpLLast->lday2,
                                lpWork->ldax1, lpWork->lday1);
                azm[2] = AZDF(lpLLast->ldaazm,azm[1],lpLLast->ldalngth) ;
                lpPoly1->sign = DSIGN(1e0,lpLLast->ldalngth);
                if(fabs(azm[2]) > TWOPI - 1e-6 && 
                   fabs(lpLLast->ldalngth) > 1e-2) azm[2]= 0e0;
                lpLLast->ldalngth =  azm[2]*lpLLast->ldarad*lpPoly1->sign;

              }  
             } 
             else 
             {  // attempt a fillet
               lpLFillet = lpLLast + 1;
               irc = OffsetCreateFillet
                  (lpLLast, lpWork, lpLFillet, whos_en, fd, lpA->type);
               if(irc == 1)
               { 
                  lpPoly1->num_sides++; 
                  lpLFillet->ldadesc  = 250;
                  lpLFillet->ldarec   = (short)lpPoly1->num_sides;
                  lpLFillet->valid = 0;
               }  
             }  
         
      } 
    }
       lpPoly1->num_orig_sides = lpPoly1->num_sides;   //now using array
  //     for (i = 14; i<=20; i++)
  //      { lpWork = lpWorkBase+i;}
  //     for (i = 0; i <= lpPoly1->num_sides;i++)
  //       {
  //         lpWork = lpWorkBase + i;
   //      }    
        return 0;  
}                                              
