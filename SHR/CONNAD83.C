


// this routine works with decimal degrees for Latitude and Longitude
// For Example 46 degrees, 46 minutes, and 51.6612 seconds are entered as
// 46.781017, 92 degrees, 7 minutes, and 4.7244 seconds are entered as
// 92.117979.  To State Plane Coordinates and County Coordinates
// are in feet. 

//  f1 = fopen("c:\\windows\\COUNTY.PAR","rb"); uses

#include "shr.h"
 int ConvertCtyCoord(long inCountyno,  
                  long Inputcoor,  long WhatPartOfCounty,
                  double *Lat, double *Long,
                  double *spX, double *spY, 
                  double *cpX, double *cpY);
void CountyChoice(void);
void DegToRad(void);
void RadToDeg(void);
void CompState(void);
void CompCounty(void);
void StateValues(void);
void CountyValues(void);
void Zoneconstant(void); 
void ldaDegToRad(void);
void Mercconstants(void);
void Merccoord(void);
void Mercgeo(void);
void Obliqueconstant(void);
void Obliquegeo(void);
void Obliquecoord(void);
void Geocalc(void);
void Coordcalc(void);
void ReInit(void);
void Arctan(void);

static	double  Beta,  Phirad, Drad, Lamdarad, Zeta,  Secs, Latdeg, Degr, Latmin, Mins,
        Latsec, Londeg, Lonmin, Lonsec, fraction, Southrad, Cmrad, Nsprad, Ssprad;
static  int  Oblique, Ch, N, Inc,  Obliqmerc,  Iterations ;
static	char   CountyZone[32], TCounty[32],Latitude[16], cZone[16];
static	double South, Meridian, Datum , Northsp, Southsp, Second, Decdeg;
static	int Zone,  Countyno;
static	double Degree, Minute;
static	double Scale, Kk, Spx, Spy, Thetarad, Spd, Spm, Sps, A, E0, N0, Daf, R, E1, N1,
       Cyx, Cyy, Cyd, Cym, Cys, Ngszn, Phib, Cmr, Phis, Phin, Flat, E, Term1, Term2,
       Qs, Ws, Qn, Wn, Qb, Sinphio, Qo, Wo, K, Rb, To, Kko, No, Eccsqd, Eta2,
       Rectradi, U0, U2, U4, U6, V0, V2, V4, V6, Sp0, Cp0, Omega0, S0, L, Sl,
       Cl, Omega, X, Y, Ro;
static	double S, A2, A4, T, A3, A5, C1, C3, Cosom, Phif, Rf, Tf, Etaf2, B2, B4, Q, B3, B5,
       D1, D3, A1, Epsqd, F0, F1, F2, F4, F6, Cpc2, Bb, Aa, Snpc, Qc, Temp, Ccc, Dd, Salpha0,
       Alphac, Landa0, Lamdac, Ff, Gg, Ii, Phic, Wc, Lamda0, J, Numerator, Denominator,
       Uu, Arctan2, Vv, Rr, Ss, Tt, Qq, Psi, Gamma, Xprime, Yprime, Sinphi,Atan22;       

//#define FNSinh(X)     0.5*(exp(X)-exp(-X))
//#define FNCosh(X)     0.5*(exp(X)+exp(-X))
//#define FNArccosh(X)  log(X+sqrt(pow(X,2)-1))
double FNSinh(double X){return    (0.5*(exp(X)-exp(-X)));}
double FNCosh(double X) {return  (0.5*(exp(X)+exp(-X)));}
double FNArccosh(double X){return  (log(X+sqrt(pow(X,2)-1)));}

void GPEnd(void)
{// ' Close input file, if open, and conclude printing
 // CloseFiles();
  return;
} 
//ConvertCoord arguments
//  input Countyno     the number of the county
//                       Anoka = 2
//                       Carver = 10
//                       Dakota = 19
//                       Hennepin = 27
//                       Ramsey = 62
//                       Scott = 70
//                       Washington = 82
//  input Accuracy       0 = nnnnnn.nnn
//                       1 = nnnnnn.nn
//                       2 = nnnnnn.n
//                       3 = nnnnnn
//                       4 = nnnnn0
//                       5 = nnnn00
//  input Inputcoor      0 = Lat Long  user providing Lat Long  *** Only One that works
//                       1 = State Plane user providing State Plane Coords
//                       2 = County Plane user provideing County Coords

//  int WhatPartOfCounty  The system divides some counties into parts.
//                        If the county of interest is in parts, specify
//                        the part you're interested in as follows:
//                          BELTRAMI North = 1 and South = 2
//                          CASS North = 1 and South = 2    
//                          COOK county is subdivided into three areas
//                            1 The area NORTH of 47 53'00s latitude north 
//                            2 The area SOUTH of 47 53'00s latitude north 
//                            3 The area along the North Shore of Lake Superior
//                          ITASCA North = 1 and South = 2
//                          LAKE county is subdivided into two areas 
//                            1 The main body of Lake county
//                            2 The area along the North Shore of Lake Superior
//                          LAKE OF THE WOODS North = 1 and South = 2
//                          ST. LOUIS county is subdivided into four areas
//                            1 The area NORTH of 47 50'00 latitude north 
//                            2 The area BETWEEN 47 15'00 and 47 50'00 latitude north 
//                            3 The area SOUTH of 47 15'00 latitude north 
//                            4 The city of Duluth and the area along the North Shore of Lake Superior 
//                          for all other counties this variable is not used                
// 
//     This routine uses the following file
//                 f1 = fopen("c:\\windows\\COUNTY.PAR","rb");

 int ConvertCtyCoord(long inCountyno,  
                  long Inputcoor,  long WhatPartOfCounty,
                  double *Lat, double *Long,
                  double *spX, double *spY, 
                  double *cpX, double *cpY)
{
Countyno = inCountyno;
switch (Inputcoor)
{
 case 1:
 {
    if (*Lat > 60.0000 || *Lat<30.0000)
    { 
//      MessageBox(NULL,"The entered latitude is nowhere near Minnesota.",
//                 "ConvertCoord Error",MB_ICONINFORMATION);
      return -6;
    }                         
    if(*Long > 120.0000 || *Long < 70.0000)
    {
//      MessageBox(NULL,"The entered longitude is nowhere near Minnesota.",
//                 "ConvertCoord Error",MB_ICONINFORMATION);
      return -7;
    }
    Beta = *Lat;
    DegToRad();
    Phirad = Drad;
    Beta = *Long;
    DegToRad();
    Lamdarad = Drad;
    Ch = WhatPartOfCounty;
    CountyChoice();
    CompState();
    CompCounty(); 
  Zeta = Phirad;
  RadToDeg();
  Zeta = Lamdarad;
  RadToDeg();
   *spX = Spx;
   *spY = Spy; 
   *cpX = Cyx;
   *cpY = Cyy;

 break;
}
case 2: //Convert State Plane to Geodetic and County Plane
{
  CountyChoice();
  StateValues();
  X = *spX;
  Y = *spY;
  // Compute Geod Pos and State Plane Theta
  StateValues();
  Zoneconstant();
  Coordcalc();
  Scale = Kk;
  Zeta = Thetarad;
  RadToDeg();
  Zeta = Phirad;
  RadToDeg();
  *Lat = Decdeg;
  Zeta = Lamdarad;
  RadToDeg();
  *Long = Decdeg;
  CompCounty();
  *cpX = Cyx;
  *cpY = Cyy;
 break;
}
case 3:
{
//CC: ' County to Geod Pos and State Plane
 CountyChoice();
 StateValues();

    Cyx = *cpX;
    Cyy = *cpY;
  // Compute County Theta and Geod Pos
  Obliqmerc = 1;
  Iterations = 0;
  //Top of County to Geodetic Position computations
  //These lines are executed twice for Oblique or Transverse Mercator systems
  while( Iterations < 2 && Obliqmerc == 1)
  {
    X = Cyx;
    Y = Cyy;
    CountyValues();
    N = Countyno;
    if( N == 1 || N == 14 || N == 15 || N == 16 || N == 29 || N == 38 || N == 48 || 
        N == 69 || N == 82 || N == 84)
    {
      if( N == 16 || N == 38 || N == 69)
      {
        if(Oblique == 1)
        {
          A = A - Datum; /*       ' Oblique*/
          E0 = 0.0;
          N0 = 0.0;
          Obliqueconstant();
          X = X - E1;
          Y = Y - N1;
          Daf = (Datum + R) / R;
          X = X / Daf;
          Y = Y / Daf;
          Obliquegeo();
        }  
        else
        {
          if(Countyno == 38)
          {
            A = A - Datum;     /* ' Lake county Transverse area*/
            Mercconstants();
            Daf = (Datum + R) / R;
            X = X / Daf;
            Y = Y / Daf;
            Mercgeo();
          }  
          else
          {
            Zoneconstant();
            Coordcalc();
            Obliqmerc = 0;
          }
        }
      }  
      else
      {
        A = A - Datum;  /*   ' Transverse Mercator counties*/
        Mercconstants();
        Daf = (Datum + R) / R;
        X = X / Daf;
        Y = Y / Daf;
        Mercgeo();
      }
    }  
    else
    {  
      Zoneconstant(); /* Lambert counties*/
      Coordcalc();
      Obliqmerc = 0;
    }
    Iterations++;
  } // end of the while loop
  
  Zeta = Phirad;
  RadToDeg();
 *Lat = Decdeg;
  Zeta = Lamdarad;
  RadToDeg();
  *Long = Decdeg;
  Zeta = Thetarad;
  RadToDeg();
  CompState();
  *spX = Spx;
  *spY = Spy;
}
} // end of the switch 
return 0;
}
/*
' Subroutines -- Select county, acquire parameters
'  */
void CountyChoice(void)
{//: ' Choose county
/*COLOR 14
CLS
PRINT "Choose county for which coordinates will be computed"
COLOR 12
PRINT
PRINT " 1 Aitkin         19 Dakota        37  Lac Qui Parle  54 Norman       71   Sherburne"
PRINT " 2 Anoka          20 Dodge         38  Lake           55 Olmsted      72   Sibley"
PRINT " 3 Becker         21 Douglas       39  Lake of Woods  56 Ottertail     73  Stearns"
PRINT " 4 Beltrami       22 Faribault     40  Le Sueur       57 Pennington    74  Steele"
PRINT " 5 Benton         23 Fillmore      41  Lincoln        58 Pine          75  Stevens"
PRINT " 6 Big Stone      24 Freeborn      42  Lyon           59 Pipestone     76  Swift"
PRINT " 7 Blue Earth     25 Goodhue       43  McLeod         60 Polk          77  Todd"
PRINT " 8 Brown          26 Grant         44  Mahnomen       61 Pope          78  Traverse"
PRINT " 9 Carlton        27 Hennepin      45  Marshall       62 Ramsey        79  Wabasha"
PRINT " 10 Carver        28  Houston      46   Martin        63  Red Lake     80   Wadena"
PRINT " 11 Cass          29  Hubbard      47   Meeker        64  Redwood      81   Waseca"
PRINT " 12 Chippewa      30  Isanti       48   Mille Lacs    65  Renville     82   Washington"
PRINT " 13 Chisago       31  Itasca       49   Morrison      66  Rice         83   Watonwan"
PRINT " 14 Clay          32  Jackson      50   Mower         67  Rock         84   Wilkin"
PRINT " 15 Clearwater    33  Kanabec      51   Murray        68  Roseau       85   Winona"
PRINT " 16 Cook          34  Kandiyohi    52   Nicollet      69  St. Louis    86   Wright"
PRINT " 17 Cottonwood    35  Kittson      53   Nobles        70  Scott        87   Yellow Medic."
PRINT " 18 Crow Wing     36  Koochiching"
PRINT                    
COLOR 14                 
LOCATE 23,1              
PRINT "Use arrow keys to choose county, then press [Enter].  Press [Esc] to quit."
Arrow$="=>"
LOCATE Row%,16*(Column%-1)+1    */
HFILE F1;
int result; 

typedef struct 
  {
    char   County[23];
    double Souths,
           Meridians,
           Datums,
           Northsps,
           Southsps;
    short  Zones;
  } ACounty;
typedef ACounty *lpACounty;
static	ACounty ACty; 
static	long	CurCountyNo=-1;
OFSTRUCTGM	OFStruct;
BOOL	DoRead = FALSE;
 
lpACounty lpCty = &ACty; 
  
  Oblique = 0;
  CountyZone[0] = '\0';
  if (CurCountyNo != Countyno)
  	DoRead = TRUE;
  CurCountyNo = Countyno; 
  if (DoRead)
  {
	  F1 = GSSiOpenFile ("[%DL]county.par",&OFStruct,OF_READ);
	  if (F1 == HFILE_ERROR) 
	  {
	      GSSiMessageBox (0,"Unable to open file COUNTY.PAR",
	                 "ConvertCoord Error",MB_ICONINFORMATION,0);  
	      return;
	  } 
	}
  
//  if(Countyno == 1)f2 = fopen("e:\\conad\\connad83\\county.txt","wt");
//OPEN "COUNTY.PAR" AS #1 LEN=65 //' Open COUNTY.PAR file
//FIELD #1, 23 AS County, 8 AS Souths, 8 AS Meridians, 8 AS Datums, 
//8 AS Northsps, 8 AS Southsps, 2 AS Zones

 /* for(Countyno = 1;Countyno <= 112;Countyno++)
   {
     result = fseek(f1,(Countyno-1)*65,0);
     result = BigRead(F1,&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = fseek(f1,((Countyno-1)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
   }
    return;  */
  
  if(DoRead && Countyno >= 1 && Countyno <= 3)
  {  
     result = GSSillseek(F1,(Countyno-1)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno-1)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
  }
  if(Countyno == 4)
  {
    _fstrcpy(TCounty,"BELTRAMI");
    _fstrcpy(Latitude,"48 01'12");
    if (DoRead)
    {
     result = GSSillseek(F1,((Countyno-2)+Ch)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,(((Countyno-2)+Ch)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42); 
    }
   // GET #1,Ch +3;
  }  
  if(DoRead && Countyno >= 5 && Countyno <= 10)
  {
     result = GSSillseek(F1,(Countyno)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
    //GET #1,Countyno +1
  }  
  if( Countyno == 11)
  {
    _fstrcpy(TCounty,"CASS"); 
    _fstrcpy(Latitude, "46 48'15");
    //TwoZones(); 
    if (DoRead)
    {
     result = GSSillseek(F1,((Countyno-1)+Ch)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno-1+Ch)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42); 
    }
    //GET #1,Ch + 11;
  }  
  if(DoRead &&  Countyno >= 12 && Countyno <=15)
  {
     result = GSSillseek(F1,(Countyno+1)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno+1)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
    //GET #1,Countyno + 2;
  }  
if( Countyno == 16)   //Cook county
{ 
	if (DoRead)
	{
     result = GSSillseek(F1,(Countyno+1+Ch)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno+1+Ch)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
    }
  if(Ch == 3)
  {
    _fstrcpy(CountyZone,"NORTH SHORE");
    Oblique = 1;
    Beta = 46.5000;
    DegToRad();
    Phic = Drad;
    Beta = 89.3277777777777778;
    DegToRad();
    Lamdac = Drad;
    Beta = 62.0000;
    DegToRad();
    Alphac = Drad ;
    E1 = -20300000.0;
    N1 = -11500000.0;
  } 
  if(Ch == 2) _fstrcpy(CountyZone,"SOUTH");
  if(Ch == 1) _fstrcpy(CountyZone,"NORTH");
}
  if(DoRead && Countyno >= 17 && Countyno <= 21)
  {
     result = GSSillseek(F1,(Countyno+4)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno+4)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
    //GET #1,Countyno +5;
  }  
  if(DoRead && Countyno >=22 && Countyno <=30)
  {
   // GET #1,Countyno +6;
     result = GSSillseek(F1,(Countyno+5)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno+5)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
  }  
  if(Countyno == 31)
  {
    _fstrcpy(TCounty,"ITASCA");
    _fstrcpy(Latitude,"47 30'00");
    //TwoZones();
    //GET #1,Ch +36;  
    if (DoRead)
    {
     result = GSSillseek(F1,(Ch+35)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Ch+35)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42); 
    }
  } 
  if(DoRead && Countyno >= 32 && Countyno <= 37)
  {
     result = GSSillseek(F1,(Countyno+6)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno+6)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
     //GET #1,Countyno +7;
  } 
  if(Countyno == 38)
  {  
  	if (DoRead)
  	{
     result = GSSillseek(F1,(Countyno+6+Ch)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno+6+Ch)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42); 
    }
    if(Ch == 2)
    {
      _fstrcpy(CountyZone,"NORTH SHORE");
      Oblique =1;
      Beta = 46.16666666666667;
      DegToRad();
      Phic = Drad ;
      Beta = 89.97611111111111;
      DegToRad();
      Lamdac=Drad;
      Beta = 46.0000;
      DegToRad();
      Alphac = Drad;
      E1 = -13800000.0;
      N1 = -14400000.0;
    }
}
if(Countyno == 39)
{
  _fstrcpy(TCounty,"LAKE OF THE WOODS");
  _fstrcpy(Latitude,"48 59'00");
  //TwoZones();
  //GET #1,Ch +47; 
  if (DoRead)
  {
     result = GSSillseek(F1,(Ch+46)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Ch+46)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42); 
  }
}  
if(DoRead && Countyno >= 40 && Countyno <= 60)
{
     result = GSSillseek(F1,(Countyno + 9)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno + 9)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
    //GET #1,Countyno +10;
} 
if(DoRead && Countyno >= 61 && Countyno <= 68)
{
     result = GSSillseek(F1,(Countyno + 10)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno + 10)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
     //GET #1,Countyno +11;
} 
if( Countyno == 69)
{
  if(Ch == 0)MessageBox(NULL," ST. LOUIS county is subdivided into four areas\n\
1) The area NORTH of 47 50'00 latitude north\n\
2) The area BETWEEN 47 15'00 and 47 50'00 latitude north\n\
3) The area SOUTH of 47 15'00 latitude north\n\
4) The city of Duluth and the area along the North Shore of Lake Superior",
"ConvertCoords",MB_ICONINFORMATION); 
if (DoRead)
{
     result = GSSillseek(F1,(78+Ch)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((78+Ch)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
} 
  //GET #1,79+Ch ;
  if(Ch == 4)
  {
    _fstrcpy(CountyZone,"NORTH SHORE");
    Oblique = 1;
    Beta = 45.7500;
    DegToRad();
    Phic = Drad;
    Beta = 90.6916666666666667;
    DegToRad();
    Lamdac = Drad;
    Beta = 45.0000;
    DegToRad();
    Alphac =Drad;
    E1 =-13500000.0;
    N1 =-14300000.0;
  }
  if(Ch == 3)_fstrcpy(CountyZone,"SOUTH");
  if(Ch == 2)_fstrcpy(CountyZone,"CENTRAL");
  if(Ch == 1)_fstrcpy(CountyZone,"NORTH");
}
if(DoRead && Countyno >= 70 && Countyno <= 82)
{
     result = GSSillseek(F1,(Countyno +13)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno +13)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
  //GET #1,Countyno +14;
}  
if( DoRead && Countyno >= 83 && Countyno <= 87)
{
     result = GSSillseek(F1,(Countyno +14)*65,0);
     result = BigRead(F1,(HPSTR)&lpCty->County,22);
     lpCty->County[22] = '\0';
     result = GSSillseek(F1,((Countyno +14)*65)+23,0);
     result = BigRead(F1,(char *)&lpCty->Souths,42);
 // GET #1,Countyno +15;
}
if(Countyno < 1 || Countyno >87)
 {
  GSSiMessageBox (0,"Invalid County number","ConvertCoord",MB_ICONSTOP,0);
  return;
 } 
 /* WHILE RIGHT$(County$,1)=" ";
    County$=LEFT$(County$,LEN(County$)-1);
  WEND; */
 /*   lpCty->County[22] = '\0'; 


     itoa(Countyno,mess,10);
     _fstrcat(mess,"  ");
  i =   fputs( mess,f2);
  i =   fputs(lpCty->County,f2);
  i =   fputs( "\n",f2);
       fclose (f1);
  
    return ;        */
    
    
    
 South    = lpCty->Souths;
 Meridian = lpCty->Meridians;
 Datum    = lpCty->Datums;
 Northsp  = lpCty->Northsps;
 Southsp  = lpCty->Southsps;
 Zone     = lpCty->Zones;
 if (DoRead)
 	GSSiClose (F1);
 Beta = South;
 ldaDegToRad();
 Southrad = Drad;
 Beta = Meridian;
 ldaDegToRad();
 Cmrad = Drad;
 Beta = Northsp;
 ldaDegToRad();
 Nsprad = Drad;
 Beta = Southsp ;
 ldaDegToRad();
 Ssprad = Drad;
//COLOR 7
//CLS
  return;
}  

void TwoZones(void)
{//: ' Choose between 2 subzones in a county
 char mess[200]; 
 _fstrcpy (mess,TCounty);
 _fstrcat (mess," is subdivided into two areas, one North and one South of ");
 _fstrcat (mess,Latitude);
 MessageBox(NULL,mess,"ConvertCoord",MB_ICONINFORMATION);
 if( Ch == 1)
   _fstrcpy(CountyZone,"NORTH");
 else
   _fstrcpy(CountyZone,"SOUTH");
 return;
} 



void DegToRad(void)
{ // ' Convert angles in DDDMMSS.ss to radians
  Decdeg = Beta; 
  Drad = Decdeg*3.14159265358979323846/180; // Convert decimal degrees to radians
  return;
}
void ldaDegToRad(void)
{ // ' Convert angles in DDDMMSS.ss to radians
  fraction = modf(Beta / 10000, &Degree) ; // First, convert to decimal degrees
  fraction = modf((Beta - Degree*10000)/100, &Minute);
  Second = Beta - (Degree * 10000 + Minute * 100);
  Decdeg = Degree+Minute/60+Second/3600;
  Drad = Decdeg*3.14159265358979323846/180; // Convert decimal degrees to radians
  return;
}
void RadToDeg(void)
{// Radians to deg, min, sec
  Decdeg = Zeta *180/3.14159265358979323846;
  return;
}  


void CompState(void)
{// Compute state plane coordinates and theta
  StateValues();
  Zoneconstant();
  Geocalc();
  Scale =Kk;            
  Spx =X;
  Spy =Y;
  Zeta = Thetarad;
  RadToDeg();
  return;
}

void CompCounty(void)
{// Compute county coordinates and theta
  CountyValues();
  N = Countyno;
if( N == 1 || N == 14 || N == 15 || N==16 || N==29 || N==38 || 
     N==48 || N==69 || N==82 || N==84)
{  
   if( N==16 || N==38 || N==69)
   {
    if(Oblique==1)
    {
      A -= Datum;//          ' Oblique counties
      E0=0.0;
      N0=0.0;
      Obliqueconstant();
      Obliquecoord();
      Daf=(Datum+R)/R;
      X=X*Daf+E1;
      Y=Y*Daf+N1;
    }  
    else
    {
      if(Countyno==38)
      {
        A-=Datum;//      ' Lake county Transverse area
        Mercconstants();
        Merccoord();
        Daf=(Datum+R)/R;
        X=X*Daf;
        Y=Y*Daf;
      }  
      else
      {
        Zoneconstant();// ' Lambert counties
        Geocalc();
      }
    }
  }
  else
  {
    A -= Datum;//           ' Transverse Mercator counties
    Mercconstants();
    Merccoord();
    Daf = (Datum +R)/R;
    X = X * Daf;
    Y = Y *Daf;
  }
}
else
{
  Zoneconstant();// ' Lambert counties
  Geocalc();
}
Cyx = X;
Cyy = Y;
Zeta = Thetarad;
RadToDeg();
//Cyd = Degr;
//Cym = Mins;
//Cys = Secs;
  return;
}

  
void StateValues(void)
{// Get state plane parameters
if(Zone==1)
{
  _fstrcpy(cZone,"NORTH");
  Ngszn = 1;
  Beta =46.50; //463000.0
  DegToRad();
  Phib = Drad;
  Beta = 93.10;  //930600.0
  DegToRad();
  Cmr = Drad;
  Beta = 47.03333333333333;   //470200.0
  DegToRad();
  Phis =Drad;
  Beta = 48.63333333333333;  //483800.0
  DegToRad();
  Phin = Drad;
}
if(Zone==2)
{
  _fstrcpy(cZone,"CENTRAL");
  Ngszn = 2;
  Beta = 45.0000;
  DegToRad();
  Phib = Drad;
  Beta = 94.2500;  //941500.0
  DegToRad();
  Cmr = Drad;
  Beta = 45.6166666666666667;   //453700.0
  DegToRad();
  Phis = Drad;
  Beta = 47.05;   //470300.
  DegToRad();
  Phin =Drad;
}
if(Zone==3)
{
  _fstrcpy(cZone,"SOUTH");
  Ngszn = 3;
  Beta = 43.00;
  DegToRad();
  Phib = Drad;
  Beta = 94.00;
  DegToRad();
  Cmr =Drad;
  Beta = 43.783333333333333;  //434700.0
  DegToRad();
  Phis = Drad;
  Beta = 45.2166666666666667;  //451300.0
  DegToRad();
  Phin = Drad;
}
 N0 = 100000.0*3937.0/1200.0;  //    ' Convert to feet
 E0 = 800000.0*3937.0/1200.0;
 A =  6378137.0*3937.0/1200.0;
return;
}

void CountyValues(void)
{//: ' Set county values
Phib = Southrad;
Phis = Ssprad;
Phin = Nsprad;
Cmr = Cmrad;
A = 6378137.0 * 3937.0 / 1200.0 + Datum;//    ' In effect, changes the shape of the spheriod to fit the county
E0 =500000.0;
N0 =100000.0;
return;
}

void Zoneconstant(void)
{//: ' Compute zone constants for direct and inverse equations in Geocalc and Coordcalc.
//IF Counter%=23 THEN GOSUB PrintHead
Flat = 1.0/298.25722210;//            ' Dimensionless constant (flattening)
E = sqrt(2.0*Flat - pow(Flat,2));
Term1 = log((1+sin(Phis))/(1-sin(Phis)));
Term2 = E*log((1.0+E*sin(Phis))/(1.0-E*sin(Phis)));
Qs = 0.5*(Term1-Term2);
Ws = sqrt(1.0-pow(E,2)*pow(sin(Phis),2));
Term1 = log((1.0+sin(Phin))/(1.0-sin(Phin)));
Term2=E * log((1+E*sin(Phin))/(1-E*sin(Phin)));
Qn = 0.5*(Term1-Term2);
Wn = sqrt(1.0-pow(E,2)*pow(sin(Phin),2));
Term1 = log((1.0+sin(Phib))/(1.0-sin(Phib)));
Term2 = E*log((1.0+E*sin(Phib))/(1.0-E*sin(Phib)));
Qb = 0.5*(Term1-Term2);
Sinphio = log((Wn*cos(Phis))/(Ws*cos(Phin)))/(Qn-Qs);
Term1 = log((1.0+Sinphio)/(1.0-Sinphio));
Term2 = E* log((1.0+E*Sinphio)/(1.0-E*Sinphio));
Qo = 0.5*(Term1-Term2);
Wo = sqrt(1.0-pow(E,2)*pow(Sinphio,2));
K = (A*cos(Phis) * exp(Qs*Sinphio))/(Ws*Sinphio);
Rb = K / exp(Qb * Sinphio);
Ro = K / exp(Qo * Sinphio);
Kko = Wo *tan(atan(Sinphio /sqrt(1.0-pow(Sinphio,2))))*Ro /A;//  ' ASN(x)=ATN(x/SQR(1-x^2))
No = Rb + N0 - Ro;
return;
}

void Mercconstants(void)
{//: ' Get Mercator constants
Flat = 1/298.25722210;
Eccsqd = 2*Flat- pow(Flat,2);
R = A/sqrt(1-Eccsqd*pow(sin(Phirad),2));
Eta2 = Eccsqd*pow(cos(Phirad),2)/(1-Eccsqd);
Rectradi = 6367449.14577*3937.0/1200.0;
U0 =-.005048250776;
U2 =.000021259204;
U4 =-.000000111423;
U6 =.000000000626;
V0 =.005022893948;
V2 =.000029370625;
V4 =.000000235059;
V6 =.000000002181;
Sp0 =sin(Phib);
Cp0 =cos(Phib);
Omega0 = Phib +Sp0 *Cp0 *(U0 +U2 * pow(Cp0,2) + U4 * pow(Cp0,4)+ U6 *pow(Cp0,6));
S0 = 1.00*Omega0 *Rectradi;
return;
}

void Merccoord(void)
{//: ' Perform forward computation based on Transverse Mercator Projection
//'   Equations used were compiled by T. Vincenty, NGS, 1984
L =(Lamdarad -Cmr)*cos(Phirad);
Sl = sin(Phirad);
Cl =cos(Phirad);
Omega = Phirad + Sl * Cl * (U0+U2*pow(Cl,2)+ U4 * pow(Cl,4)+U6*pow(Cl,6));
S = 1.00 * Omega * Rectradi;
R = 1.00 * R;
A2 = 0.5 * R *tan(Phirad);
A4 = (5.0-pow(tan(Phirad),2)+9.0*Eta2)/12.0;
Y = S - S0 + A2 * pow(L,2) * (1.0 + A4 * pow(L,2))+N0;
A1 = -R;
T = tan(Phirad);
A3 = (1-T*T + Eta2)/6.0;
A5 = (5-18.0*pow(T,2)+pow(T,4))/120.0;
X =E0 + A1* L * (1.0 + L * L * (A3 + A5 * L* L));
   //     ' Calculate convergence
C1 = -tan(Phirad);
C3 = (1+3*Eta2)/3.0;
Thetarad = C1 * L * (1 + C3* L * L);
return;
}

void Mercgeo(void)
{// ' Perform reverse computation on Transverse Mercator Projection
Omega = (Y-N0+S0)/(1.00*Rectradi);
Cosom = cos(Omega);
Phif = Omega+sin(Omega)*Cosom*(V0+V2*pow(Cosom,2)+V4*pow(Cosom,4)+V6*pow(Cosom,6));
Rf = 1.00*A/sqrt(1-Eccsqd*pow(sin(Phif),2));
Q = (X-E0)/Rf;
Tf=tan(Phif);
Etaf2 = Eccsqd*pow(cos(Phif),2)/(1-Eccsqd);
B2 = -0.5*Tf*(1+Etaf2);
B4 = -(5+3*pow(Tf,2) + Etaf2 * (1-9*pow(Tf,2)))/12.0;
Phirad = Phif+B2 *Q*Q*(1+B4*pow(Q,2));
B3 = -(1+2*pow(Tf,2) + Etaf2)/6.0;
B5=(5+28*pow(Tf,2)+24*pow(Tf,4))/120.0;
L = Q*(1+pow(Q,2)*(B3+B5*pow(Q,2)));
Lamdarad = Cmr-(L/cos(Phif));
  //    ' Compute convergence
D1 = Tf;
D3 = -(1+pow(Tf,2) - Etaf2)/3;
Thetarad = D1*Q*(1+D3*pow(Q,2));
return;
}


void Obliqueconstant(void)
{//: ' Get constants for Oblique Mercator Projection
Flat = 1/298.25722210;
Eccsqd = 2 * Flat - pow(Flat,2);
E = sqrt(Eccsqd);
Epsqd = Eccsqd/(1-Eccsqd);
R = A/sqrt(1-Eccsqd*pow(sin(Phirad),2));
F0 = 0.006686920927;
F2 = 0.000052014584;
F4 = 0.000000554430;
F6 = 0.000000006820;
Cpc2 = pow(cos(Phic),2);
Bb = sqrt(1 + Epsqd * pow(Cpc2,2));
Wc = sqrt(1-Eccsqd*pow(sin(Phic),2));
Aa = A* Bb * sqrt(1-Eccsqd)/pow(Wc,2);
Snpc = sin(Phic);
Term1 = log((1+Snpc)/(1-Snpc));
Term2 = E * log((1+E*Snpc)/(1-E*Snpc));
Qc = 0.5*(Term1-Term2);
Temp = Bb * sqrt(1-Eccsqd)/(Wc*cos(Phic));
Ccc = FNArccosh(Temp) - Bb * Qc;
Dd = 1.0 * Aa / Bb;
Salpha0 = A * sin(Alphac)* cos(Phic)/(Aa*Wc);
Temp = Salpha0*FNSinh(Bb*Qc+Ccc)/cos(atan(Salpha0/sqrt(1-pow(Salpha0,2))));// ' ASN(x)=ATN(X/sqr(1-X^2))
Lamda0 = Lamdac+atan(Temp/sqrt(1-pow(Temp,2)))/Bb;//  ' ASN(x)=ATN(x/SQR(1-x^2))
Ff = Salpha0;
Gg = cos(atan(Salpha0/sqrt(1-pow(Salpha0,2))));// ' ASN(x)=ATN(x/SQR(1-x^2))
Ii = 1.0 * Aa /A;
return;
}

void Obliquecoord(void)
{//: ' Perform forward computation based on Oblique Mercator Projection
L = (Lamdarad - Lamda0)*Bb;
Term1 = log((1+sin(Phirad))/(1-sin(Phirad)));
Term2 = E*log((1+E*sin(Phirad))/(1-E*sin(Phirad)));
Q = 0.5 * (Term1-Term2);
J = FNSinh(Bb*Q+Ccc);
K = FNCosh(Bb*Q+Ccc);
Numerator =J *Gg -Ff *sin(L);
Denominator = cos(L);
Arctan();// ' Function to determine Arctan2# based on Numerator# & Denominator#
Uu = Dd * Arctan2;
Vv = 0.5*Dd*log((K-Ff*J-Gg*sin(L))/(K+Ff*J+Gg*sin(L)));
Y = Uu*cos(Alphac)-Vv*sin(Alphac)+N0;
X = Uu*sin(Alphac)+Vv*cos(Alphac)+E0;
Numerator = Ff- J * Gg *sin(L);
Denominator = K * Gg * cos(L);
Arctan();// ' Function to determine Arctan2# from Numerator# & Denominator#
Thetarad = Arctan2 - Alphac;
return;
}

void Arctan(void)
{//: ' Replacement of FNArctan2(Numerator#,Denominator#)
if( Denominator == 0)
{
  if(Numerator >= 0) 
    Arctan2=3.141592653589793/2 ;
  else
    Arctan2=3*3.14159265358979323846/2;
}    
else
{
  if( Numerator == 0)
  {
    if(Denominator >= 0)
      Arctan2 = 0;
    else
      Arctan2 =3.141592653589793;
  }    
  else
  {
    Atan22 =atan(fabs(Numerator)/fabs(Denominator));
    if( Numerator > 0 && Denominator > 0) Arctan2=Atan22;
    if( Numerator > 0 && Denominator < 0) Arctan2 = 3.141592653589793 - Atan22;
    if( Numerator < 0 && Denominator < 0) Arctan2 = 3.141592653589793 + Atan22;
    if( Numerator < 0 && Denominator > 0)  Arctan2 = 2*3.141592653589793-Atan22;
  }
}

return;
}
void Obliquegeo(void)
{//: ' Perform reverse computation on Oblique Mercator Projection
Uu = (X-E0)*sin(Alphac)+(Y-N0)*cos(Alphac);
Vv = (X-E0)*cos(Alphac)-(Y-N0)*sin(Alphac);
Rr = FNSinh(Vv/Dd);
Ss = FNCosh(Vv/Dd);
Tt = sin(Uu/Dd);
Qq = (0.5*log((Ss-Rr*Ff+Gg*Tt)/(Ss+Rr*Ff-Gg*Tt))-Ccc)/Bb;
Numerator = exp(Qq)-1;
Denominator = exp(Qq)+1;
Arctan();// ' Function to determine Arctan2# based on Numerator# & Denominator#
Psi = 2*Arctan2;
Phirad = Psi+sin(Psi)*cos(Psi)*(F0+F2*pow(cos(Psi),2)+F4*pow(cos(Psi),4)+F6*pow(cos(Psi),6));
Numerator = Rr * Gg + Tt * Ff;
Denominator = cos(Uu/Dd);
Arctan();// ' Function to determine Arctan2# based on Numerator# & Denominator#
Lamdarad = Lamda0 - Arctan2/Bb;
Obliquecoord();// ' Use Obliquecoord subroutine to compute Thetarad#
return;
}

void Geocalc(void)
{//: ' Calculate X and Y coords given lat and long
Term1 = log((1+sin(Phirad))/(1-sin(Phirad)));
Term2 = E * log((1+E*sin(Phirad))/(1-E*sin(Phirad)));
Q = 0.5 * (Term1-Term2);
R = K / exp(Q * Sinphio);
Gamma = (Cmr - Lamdarad) *Sinphio;//         ' In radians
Y = Rb + N0 - R * cos(Gamma);
X = E0 + R * sin(Gamma);
Thetarad = Gamma;
Kk =sqrt(1- pow(E,2)*pow(sin(Phirad),2))*R*Sinphio/(A*cos(Phirad));
return;
}

void Coordcalc(void)
{//: ' Perform equations used in inverse calculations
Yprime = Rb - Y + N0;
Xprime = X - E0;
Gamma = atan(Xprime/Yprime);
Lamdarad = Cmr - (Gamma/Sinphio);
R = sqrt(pow(Yprime,2) + pow(Xprime,2));
Q = log(K/R)/Sinphio;
 // ' Computation of latitude is iterative.  Starting with the approximation
Sinphi = (exp(2*Q)-1)/(exp(2*Q)+1);
 // ' Iterate SIN(Phi) three times as follows
for( Inc = 1; Inc <= 3; Inc++)
{//FOR Inc%=1 TO 3
  Term1 = log((1+Sinphi)/(1-Sinphi));
  Term2 = E * log((1+E*Sinphi)/(1-E*Sinphi));
  F1 = 0.5 * (Term1 - Term2)- Q;
  Term1 = 1/(1-pow(Sinphi,2));
  Term2 = E * log((1+E * Sinphi)/(1-pow(E,2) * pow(Sinphi,2)));
  F2 = Term1 - Term2;
  Sinphi = Sinphi - (F1/F2);
}//NEXT Inc%
Phirad = atan(Sinphi/sqrt(1-pow(Sinphi,2)));// ' ASN(x)=ATN(x/SQR(1-x^2))
Kk = sqrt(1-pow(E,2)*pow(Sinphi,2))*R*Sinphio/(A*cos(Phirad));
Thetarad = Gamma;
return;
}

void ReInit(void)
{//: ' Re-initialize variables
/*Counter = Counter+1;
Pointno=Pointno+1;
Serno=Serno+1;
Seqno=Seqno+10;
Elev="       "
Ngseleva="      "
Horcode="  "
Uccmisc="  MONMNDT19CXXXX"
Ngsmisc="BMN23" */
return;                                
}
