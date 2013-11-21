/* SurvXfr2.h 96.05.31.1400cst JMS  Desktop Surveying Software */
//                     Copyright 1995 GEOSPAN Corp.
//                    GEOSPAN Confidential Document.
//                           Internal Use Only.
//Type Definitions       Comments                                Bytes Units
typedef struct {	//Frame IDentification data		   264
   char    cIdRun[8];	//@	Run Identifier			     8 a8
   long    lIdFrame;    //@     Frame Identifier                     4 frame
   float   fPixyIn[2];  //@     Designated Pixel Coordinates         8 pix
   int	   iWHPix[2];	//@	Frame-Width & Height		     4 pix
   float    fPixyFML[3][10][2];//out: -view of other vectors & pt. 240 pix
   } FRAME;

//Type Definitions       Comments                                Bytes Units
typedef struct {	// Desktop Surveying I/O		  ____
   long    lFramesT	//@	Total number of frames in use	     4 none
   long    lDone;	//	Done Flag (if <6:error-read cOutMsg) 4 none
   double  dPxyz84[3];	//	Point-Coords-(X,Y,Z) in WGS84	    24 meters
   double  dPllh84[3];	//		    -(Lat,Lon,Ht) in WGS84  24 deg/m
   float   fAbs1sig;    //           -dist.errors-Abs 1 sigma-rss    4 meters
   float   frmsDCalc;   //                       -Calcs-rms          4
   float   frmsPix[2];  //                       -pixels-rms         8 pixels
   char    cOutMsg[40]; //      Message-Interprets"lDone"           40 a40
   FRAME   F[10];	//				   10*264 2640 struct

   double  dOrigin[3];	//@	Origin-(X,Y,Z) in WGS84 coords	    24 meters
   long    lOrigin;	//@	      -flag(1=-O,2=-O*ned,3:-O*NtoU) 4 none
   float   fDegNtoU[3]; //@           -rpy from NED-to User coords  12 deg
   float   fRelDist;	//		    -length of "fxyzRel"     4
   float   fxyzRel[3];  //                  -Rel(X,Y,Z) per"lOrigin"12 meters
   } SURVIN;


   "SURVIN.lDone" &  Corresponding "SURVIN.cOutMsg" Message
     Value
       6        "Survey Point and Vectors computed.
       5        "Surveying Error: RMS of Fit > 5 pixels.
       4        "Surveying Error: RMS of Fit > 1 meter.
       3        "Analytics Reject of Data:parallel/null.
       2        "Trigonometry Reject of Data: way out.
       1        "Single vector shown - as requested.
       0        "Data was read but not processed.
      -1        "Less than two frames in view.
      -2        "No point surveyed in any view.
      -3        "More points surveyed than views! ?

                                                    JMS
                                                  95.12.07
