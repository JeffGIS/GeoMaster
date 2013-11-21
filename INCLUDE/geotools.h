//----------------------------------------------------------------------------
//
//	GEOTOOLS.H		GEOTOOLS.DLL API
//
//	Version 1.43
//
//	Copyright (c) 1997,1998,1999 GEOSPAN Corporation, All Rights Reserved.
//
//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

#ifndef	GTSCOPE
#define	GTSCOPE	dllimport
#endif

//----------------------------------------------------------------------------
//
//	Constants used in API args:
//
//----------------------------------------------------------------------------

// OpenPicFileEx() lPFIMode Arguments.

// The default OpenPicFile()/OpenPicFileEx() behavior is to use the global
// path set by SetPicPath() and SetPFIPath() if present, otherwise use the
// path in the OpenPicFile() file name.
// One, both or none of the constants below can be used:
#define	PICPATH_LOCAL	0x100	// Path in file name overrides SetPicPath()
#define	PFIPATH_LOCAL	0x200	// Path in file name overrides SetPFIPath()

// Just one of the following constants can be used in OpenPicFileEx().
// It can be combined with one or both of the xxxPATH_LOCAL constants above.
#define	PFI_RO		0	// normal default, PFI must exist and is read only
#define	PFI_IGNORE	1	// ignore any existing PFI and synthesize a new one
#define	PFI_UPDATE	2	// open existing PFI if found, else synthesize new
#define	PFI_NOPIC	3	// open only the PFI and not the pic file
#define	PFI_NEWPIC	4	// create a new pic file, write only mode
#define	PFI_RENAME	5	// alter path/name of already-open PFI for SavePFI()

// SetSys(3,N,0) BITMAP: N=Decode mode for reading frame data.
// The bits can be combined, but not all combos are legal.
#define	DECODE_NONE	0x0000	// read the raw frame data (for copying)
#define	DECODE_VFW	0x0001	// decode to DIB via installed VfW CODEC
#define	DECODE_HUFF	0x0002	// create a JPG by inserting a Huffman table
#define	DECODE_DIB	0x0004	// internal (non-VfW-CODEC) JPEG to DIB
#define	DECODE_FIX	0x0100	// fix selected JPEG problems

#define	DESC_MAX_LENGTH	127	// maximum length of a GetDesc() string

//----------------------------------------------------------------------------
//
//	Error codes returned by GEOTOOLS.
//
//	See GTERROR.TXT for more details and additional error codes.
//
//----------------------------------------------------------------------------

#define	GT_SUCCESS			  1	// all ok
#define	ERR_MEDIA_OPEN		  2	// cannot open media file (e.g. AVI)
#define	ERR_PFI_OPEN		  3	// cannot open PFI
#define	ERR_GEO_OPEN		  4	// cannot open GEO (*)
#define	ERR_GEO_HDR			  5	// cannot read GEO header (*)
#define	ERR_MANY_POINTS		  6	// too many keypoints
#define	ERR_GEO_REC			  7	// cannot read GEO record (*)
#define	ERR_PARAM_OOR		  8	// parameter out of range
#define	ERR_AREA_OOR		  9	// area (run) number out of range
#define	ERR_NOTHING_NEAR	 10	// nothing found near selected point
#define	ERR_NO_AREA			 11	// no area (run) selected
#define	ERR_TRACK_BEGIN		 12	// attempted move to before BOF
#define	ERR_TRACK_END		 13	// move beyond EOF, or no points exist
#define	ERR_TRACK_OP		 14	// bad type or position parameter
#define	ERR_BRT_OPEN		 15	// cannot open BRT (*)
#define	ERR_BRT_READ		 16	// cannot read BRT (*)
#define	ERR_NOT_BRT			 17	// not a valid BRT (*)
#define	ERR_NOT_IN_MEDIA	 18	// requested picture not on-line
#define	ERR_PIC_PHANTOM		 19	// requested picture not available
#define	ERR_TRUNCATED		 20	// too long for user-supplied buffer
#define	ERR_UNAVAILABLE		 21	// capability not available
#define	ERR_AVI_SGFO		 22	// AVIStreamGetFrameOpen error (*)
#define	ERR_GPSTREAM_USED	 23	// GPICSTREAM already in use
#define	ERR_GPSTREAM_OOR	 24	// GPICSTREAM number out of range
#define	ERR_PIC_NOTKEY		 25	// not a key/complete picture
#define	ERR_PIC_DECODE		 26	// error decoding the picture
#define	ERR_PIC_INVALID		 27	// picture is not valid
#define ERR_JPEG			 28	// JPEG error
#define ERR_PIC_OOR			 29	// picture number out of range
#define ERR_HDC_INVALID		 30	// invalid HDC/HWND passed
#define	ERR_GDI				 31	// Windows GDI error (*)
#define	ERR_GPSTREAM_NULL	 32	// GPICSTREAM not open
#define	ERR_PFI_MODE		 33	// illegal PFI mode specified
#define	ERR_LIST_FULL		 34	// could not add to list
#define	ERR_PFI_CREATE		 35	// cannot create PFI
#define	ERR_PFI_READ		 36	// PFI read error (*)
#define	ERR_NOT_PFI			 37	// Not a PFI file (*)
#define	ERR_PFI_NOMEM		 38	// PFI memory not allocated (*)
#define	ERR_PFI_LARGE		 39	// PFI too large (*)
#define	ERR_PFI_MEM			 40	// insufficient memory for PFI data
#define	ERR_PFI_WRITE		 41	// PFI write error (*)
#define	ERR_PFI_DIFFTYPE	 42	// unexpected type
#define	ERR_PFI_TYPE		 43	// unrecognized type
#define	ERR_GPFILE_USED		 44	// GPICFILE already in use
#define	ERR_GPFILE_OOR		 45	// GPICFILE number out of range
#define	ERR_GPFILE_NULL		 46	// GPICFILE not open
#define	ERR_PFI_LIST		 47	// PFI edit list full (*)
#define	ERR_PFI_REC			 48	// PFI rec not found (*)
#define	ERR_PFI_EDIT_OOR	 49	// edit record out of range
#define	ERR_PFI_VIEW_OOR	 50	// view record out of range
#define	ERR_PFI_VIEW		 51	// view not legal or not defined in PFI
#define	ERR_PFI_SEEK		 52	// seek reached BOF/EOF
#define	ERR_MEMDRAW			 53	// MemDraw failure (*)
#define	ERR_SCANFILE_CMD	 54	// unrecognized ScanFile command
#define	ERR_INDEX_OOR		 55	// index out of range
#define	ERR_IMAGE			 56	// image parameter error (*)
#define	ERR_EDIT_ACTION		 57	// edit record action conflict
#define	ERR_GEO_MEM			 58	// insufficient memory for GEO data
#define	ERR_RUNREC_OOR		 59	// run record number out of range
#define	ERR_INTERNAL		 60	// general "can't happen" trap
#define	ERR_AVI_UNSUPPORTED	 61 // (*)
#define	ERR_AVI_BADFORMAT	 62 // (*)
#define	ERR_AVI_MEMORY		 63 // (*)
#define	ERR_AVI_INTERNAL	 64 // (*)
#define	ERR_AVI_BADFLAGS	 65 // (*)
#define	ERR_AVI_BADPARAM	 66 // (*)
#define	ERR_AVI_BADSIZE		 67 // (*)
#define	ERR_AVI_BADHANDLE	 68 // (*)
#define	ERR_AVI_FILEREAD	 69 // (*)
#define	ERR_AVI_WRITE		 70	// AVI write error
#define	ERR_AVI_COMPRESSOR	 71 // (*)
#define	ERR_AVI_NOCOMPRESSOR 72 // (*)
#define	ERR_AVI_READONLY	 73 // (*)
#define	ERR_AVI_NODATA		 74 // (*)
#define	ERR_AVI_BUFFER		 75 // (*)
#define	ERR_AVI_CANTCOMPRESS 76 // (*)
#define	ERR_AVI_USERABORT	 77 // (*)
#define	ERR_AVI_ERROR		 78 // (*)
#define	ERR_AVI_UNKNOWN		 79	// Unknown AVI error (*)
#define	ERR_SKIP			 80	// point returned is a "skip" (new segment)
#define	ERR_HWND_INVALID	 81	// couldn't get HDC using this HWND
#define	ERR_SURVEY_RMS5P	 82	// (*)
#define	ERR_SURVEY_RMS1M	 83	// (*)
#define	ERR_SURVEY_NULL		 84	// (*)
#define	ERR_SURVEY_OUT		 85	// (*)
#define	ERR_SURVEY_SINGLE	 86	// (*)
#define	ERR_SURVEY_NOPROC	 87	// (*)
#define	ERR_SURVEY_FRAMES	 88	// (*)
#define	ERR_SURVEY_NOPOINT	 89	// (*)
#define	ERR_SURVEY_POINTS	 90	// (*)
#define	ERR_SURVEY_UNKNOWN	 91	// (*)
#define	ERR_NO_CAMERA		 92	// no preferred camera near selected point
#define	ERR_FORMAT_MEM		 93	// insufficient memory for image format data
#define	ERR_IMAGE_MEM		 94	// insufficient memory for image data
#define	ERR_NEED_NEW		 95	// copy destination must be new file
#define	ERR_PIC_NOT_READY	 96	// requested pic is not accessible yet
#define	ERR_NO_CENTER		 97	// center of map area not defined
#define	ERR_GEO_VER			 98	// GEO file version too old
#define	ERR_SCALE			 99	// illegal scale factor
#define	ERR_DATA_NOT_READY	100	// requested data is not accessible yet
#define	ERR_NO_MAP			101	// local map is not defined
#define	ERR_BAD_STRING		102	// empty or invalid file name or string
#define	ERR_BMP_SAVE		103	// error saving BMP image
#define	ERR_ORIGIN			104	// origin point not set
#define	ERR_CAM_CFG			105	// CAMERA.CFG not found
#define	ERR_CAM_OOR			106	// camera number out of range
#define	ERR_GEO_WRITE		107	// cannot write GEO record
#define	ERR_JPEG_CREATE		108	// cannot create JPG file
#define	ERR_JPEG_WRITE		109	// cannot write to JPG file
#define	ERR_SURV_REC		110	// unrecognized survey record format
#define	ERR_CAM_REC			111	// no camera record for this picture
#define	ERR_PIC_CAL			112	// camera not calibrated for this picture
#define	ERR_GPSTREAM2_OOR	113	// secondary picture stream out of range
#define	ERR_GPSTREAM2_NULL	114	// secondary picture stream not open
#define	ERR_AREA2_OOR		115	// secondary run number out of range
#define	ERR_PARALLEL		116	// ray is parallel to the surface
#define	ERR_DISTANCE		117	// intersection with surface too distant

//----------------------------------------------------------------------------
//
// Survey structures: DLL/SurvXfr2 I/O interface
// CAUTION--the survey interface is provisional and subject to change.
//
//----------------------------------------------------------------------------

#define	LOCATOR_VIEWS 10 // number of images per survey call

// Overview
// --------
// Both the FRAMEX and SURVXFR2 structures are for input and output.
// To survey, pick the same point in multiple images.
// Set up the FRAMEX structure for each possible image (even if not used).
// Set the other SURVXFR2 input members vars.
// Call Survey().
// Check the SURVXFR2.lDone member var for completion status.
// Display or save the results.
//
// Details
// -------
// FRAMEX: the I/O structure for each image used to survey.
// Set FRAMEX.lRun to the run number (all images should use the same
//	run number in this implementation, but that will change).
// Set FRAMEX.lIdFrame to the picture number of each image.
// Set FRAMEX.fPixyIn[] to the x [0] and y [1] coordinates of the
//	point picked in each image, scaled to the range 0.0 to 1.0, with
//	point 0,0 being the upper left of the image.  If the image is not
//	used in the survey, or there is no image, set to (-1,-1).
// FRAMEX.fPixyFML[][][] will be set after the Survey call.
//
// How to plot the result vectors in FRAMEX after the Survey() call.
// [SHB+JMS discussions 1997.12.19 & 23]
// fPixyFML = pixel values as (x,y) in First,Middle,Last:
//    First is where ray enters picture.
//    Middle is the point of interest in the picture, e.g. point being
//        surveyed if it is in the picture, or point of nearest approach
//        to it if it is not in the picture.
//    Last is where it exits the picture.
//    [3]=First,Middle,Last (1st index in DLL code, 3rd index in VB)
//    [10]=points from all other "cameras" as they appear in this frame;
//        note that the word "camera" used here means NOT a physical camera
//        but rather the "viewability of points picked in other frames in
//        this frame."  All frames could in fact be from the same physical
//        camera but at different times, or with different calibrations.
//    [2]=x,y pixel values (3rd index in DLL code, 1st index in VB)
// Should only draw vectors in frames that submitted points for survey.
// (...or...points that are more negative than -0.25 can be ignored...
// some pixel values CAN be slightly negative due to camera distortion.
// (we actually set unsurveyed pixels to -1.0. in sx2out().)
// If a vector was not generated for a camera, the 3 points will
// come out as 0,0 for all 3 FML points.
// In the index for "your" camera, the M point is actually the object
// being surveyed in "your" field of view; it will be -1,-1 if it didn't
// show up.  Example: in Frame 2, the Camera 2 "M" point is "your" point,
// i.e. the visibility of the surveyed point via this camera in this frame.
// Plotting strategy: first put a cross on the image at the survey point
// and show dPxyz[84].  If lDone is 4,5,or 6 (for sure): loop thru lFramesT
// images and look at fPixyFML where the 2nd index is the camera index and
// the 1st index is 1 (M image) and look at values for x,y there.  If either
// one of those is -1, the point is out of view, there is no cross to plot
// in that picture.  Else plot the pixel as the center of a cross on that
// image, plus show coordinate results for each.
// For plotting other rays there are 2 segments: from camera to closest
// approach to point (green), and from closest approach to infinity (gray).
// js #pragma pack (push, 1)
typedef struct {
	long    lStrucLen;		// input: size of this structure in bytes
	long	lStream;		// input: GPICSTREAM number
	long	lRun;			// input: GPICSTREAM run number
	long    lIdFrame;		// input: picture number
	// fPixyIn[] (0,0) = upper left of frame, (-1,-1) = do not survey.
	float   fPixyIn[2];		// input: pixel coordinate in range [0.0,1.0]
	float   fDist;			// out: distance to the point
	// output: view of other vectors & pt.
	float   fPixyFML[3][LOCATOR_VIEWS][2];
} FRAMEX;

// SURVXFR2: the I/O structure for one survey call.
// Set up each of the LOCATOR_VIEWS FRAMEX records as described above.
// Set SURVXFR2.lFramesT to LOCATOR_VIEWS.
// Set SURVXFR2.lOrigin to 0 to survey one point, or to 2 to calculate
//	the distance from the previous point in North East Down coords.
// If SURVXFR2.lOrigin is 2, set SURVXFR2.dOrigin to the previous survey
//	result point to measure from; otherwise, .dOrigin doesn't matter.
// If SURVXFR2.lOrigin is 3, set SURVXFR2.fDegNtoU; otherwise, doesn't
//	matter (contact JMS for more info on this option).
// After the Survey call, check SURVXFR2.lDone.  If =6, the result is valid.
//	The location is in SURVXFR2.dPxyz84 and SURVXFR2.dPllh84.
//	See the FRAMEX discussion above for how to plot the result vectors.
typedef struct {
	// lFramesT is the total number of FRAMEX recs passed, not the number of
	// FRAMEX recs containing pictures to be surveyed.
	long    lStrucLen;		// input: size of this structure in bytes
	long    lFramesT;		// input: number of FRAMEX recs passed
	long    lDone;			// output: Done Flag (if < 6: error)
	double  dPxyz84[3];		// output: Point-Coords-(X,Y,Z) in WGS84 (meters)
	double  dPllh84[3];		// output: (Lat,Lon,Ht) in WGS84 (deg/m)
	float   fAbs1sig;		// output: dist.errors-Abs 1 sigma-rss (meters)
	float   frmsDCalc;		// output: Calcs-rms
	float   frmsPix[2];		// output: pixels-rms (pixels)
	FRAMEX  F[LOCATOR_VIEWS];	// input images and output vectors (see above)
	// if lOrigin is 0, ignore the origin vars
	// if lOrigin is 1, fxyzRel is set to a relative offset
	//		(origin subtracted off)
	// if lOrigin is 2: fxyzRel is in ned coords
	// if lOrigin is 3: fxyzRel is rotated according to fDegNtoU
	//		(user coordinate frame)
	double  dOrigin[3];		// input: Origin-(X,Y,Z) in WGS84 coords (meters)
	long    lOrigin;		// input: flag(1=-O,2=-O*ned,3:-O*NtoU)
	float   fDegNtoU[3];	// input: rpy from NED-to User coords (deg)
	float   fRelDist;		// output: length of "fxyzRel"
	float   fxyzRel[3];		// output: Rel(X,Y,Z) per"lOrigin"12 (meters)
} SURVXFR2;
// js #pragma pack (pop)

//----------------------------------------------------------------------------
//
// Prototypes for API functions.
//
//----------------------------------------------------------------------------

__declspec(GTSCOPE) DWORD WINAPI AddEdit(long lFile, long lFrame, long lAction);
__declspec(GTSCOPE) long WINAPI AddRun(long lStream, LPCSTR lpszProj, LPCSTR lpszRun);
__declspec(GTSCOPE) DWORD WINAPI ClosePicFile(long lFile);
__declspec(GTSCOPE) DWORD WINAPI ClosePicStream(long lStream);
__declspec(GTSCOPE) long WINAPI ConvertPicToRaw(long lFile, long lPic);
__declspec(GTSCOPE) DWORD WINAPI ConvertPoint(double *pdOut, long lStream,
												double *pdIn, long lType);
__declspec(GTSCOPE) long WINAPI ConvertRawToPic(long lFile, long lFrame);
__declspec(GTSCOPE) DWORD WINAPI CopyPic(long lFileDest, long lFileSrc,
										   long lPic, long lField, long lType);
__declspec(GTSCOPE) DWORD WINAPI DeleteEdit(long lFile, long lFrame);
__declspec(GTSCOPE) long WINAPI FindRuns(long lStream, long lType,
												double dX1, double dY1,
												double dX2, double dY2);
__declspec(GTSCOPE) DWORD WINAPI GTTest(long lFile, long lData);
__declspec(GTSCOPE) DWORD WINAPI GetBufferCRC(LPVOID lpBuf, DWORD dwLen);
__declspec(GTSCOPE) long WINAPI GetCamCount(long lFile);
__declspec(GTSCOPE) long WINAPI GetCamOfView(long lFile, long lView);
__declspec(GTSCOPE) DWORD WINAPI GetDesc(long lNum, LPSTR lpszOut);
__declspec(GTSCOPE) long WINAPI GetDescLength(long lNum);
__declspec(GTSCOPE) long WINAPI GetEditCount(long lFile);
__declspec(GTSCOPE) DWORD WINAPI GetEditData(long lFile, long lRec);
__declspec(GTSCOPE) long WINAPI GetFileFirst(long lFile);
__declspec(GTSCOPE) DWORD WINAPI GetFileID(long lFile, LPSTR lpszOut);
__declspec(GTSCOPE) long WINAPI GetFileIDLength(long lFile);
__declspec(GTSCOPE) long WINAPI GetEditLabel(long lFile, long lRec);
__declspec(GTSCOPE) long WINAPI GetFileLong(long lFile);
__declspec(GTSCOPE) long WINAPI GetFilePicCount(long lFile);
__declspec(GTSCOPE) long WINAPI GetFilePicNum(long lFile);
__declspec(GTSCOPE) long WINAPI GetFilePicView(long lFile, long lPic);
__declspec(GTSCOPE) long WINAPI GetFileView(long lFile);
__declspec(GTSCOPE) long WINAPI GetFirst(long lStream);
__declspec(GTSCOPE) DWORD WINAPI GetGTVersion();
__declspec(GTSCOPE) double WINAPI GetGeoData(long lStream, long lRun, long lType);
__declspec(GTSCOPE) long WINAPI GetLast(long lStream);
__declspec(GTSCOPE) DWORD WINAPI GetLastFileData(long lFile);
__declspec(GTSCOPE) DWORD WINAPI GetLastFileError(long lFile);
__declspec(GTSCOPE) DWORD WINAPI GetLastStreamData(long lStream);
__declspec(GTSCOPE) DWORD WINAPI GetLastStreamError(long lStream);
__declspec(GTSCOPE) double WINAPI GetLoc(long lStream, long lType);
__declspec(GTSCOPE) DWORD WINAPI GetPFIChanged(long lFile);
__declspec(GTSCOPE) long WINAPI GetPicCount(long lStream);
__declspec(GTSCOPE) double WINAPI GetPicData(long lStream, long lRun,
											   long lPic, long lType);
__declspec(GTSCOPE) long WINAPI GetPicHeight(long lFile);
__declspec(GTSCOPE) long WINAPI GetPicNum(long lFile);
__declspec(GTSCOPE) long WINAPI GetPicView(long lFile, long lPic);
__declspec(GTSCOPE) long WINAPI GetPicWidth(long lFile);
__declspec(GTSCOPE) long WINAPI GetRawPicCount(long lFile);
__declspec(GTSCOPE) long WINAPI GetRawPicNum(long lFile);
__declspec(GTSCOPE) long WINAPI GetRunCount(long lStream);
__declspec(GTSCOPE) double WINAPI GetRunData(long lStream, long lType);
__declspec(GTSCOPE) DWORD WINAPI GetRunText(long lStream, long lType, LPSTR lpszOut);
__declspec(GTSCOPE) DWORD WINAPI GetSys(long lParam1, long lParam2);
__declspec(GTSCOPE) double WINAPI GetTrack(long lStream, long lType, long lPos);
__declspec(GTSCOPE) long WINAPI GetView(long lStream);
__declspec(GTSCOPE) long WINAPI GetViewCount(long lFile);
__declspec(GTSCOPE) long WINAPI GetViewData(long lFile, long lRec);
__declspec(GTSCOPE) long WINAPI GetViewLabel(long lFile);
__declspec(GTSCOPE) DWORD WINAPI GetViewName(long lFile, LPSTR lpszOut, long lView, long lParam);
__declspec(GTSCOPE) long WINAPI GetViewNameLength(long lFile, long lView, long lParam);
__declspec(GTSCOPE) long WINAPI GetViewOfCam(long lFile, long lCam);
__declspec(GTSCOPE) DWORD WINAPI LoadPic(long lStream, long lPic);
__declspec(GTSCOPE) DWORD WINAPI LoadRun(long lStream, long lRun);
__declspec(GTSCOPE) DWORD WINAPI OpenPicFile(long lFile, LPCSTR lpszIn);
__declspec(GTSCOPE) DWORD WINAPI OpenPicFileEx(long lFile, LPCSTR lpszIn, long lPFIMode);
__declspec(GTSCOPE) DWORD WINAPI OpenPicStream(long lStream);
__declspec(GTSCOPE) DWORD WINAPI ReadGeoRec(LPSTR pGRec, long lStream,
											  long lRun, long lRec);
__declspec(GTSCOPE) DWORD WINAPI SaveImage(LPCSTR lpszOut, LPCSTR lpBuf,
												 long lType, long lParam);
__declspec(GTSCOPE) DWORD WINAPI SaveOnePicRaw(LPCSTR lpszOut, LPCSTR lpszIn, long lFrame);
__declspec(GTSCOPE) DWORD WINAPI SavePFI(long lFile);
__declspec(GTSCOPE) DWORD WINAPI SaveTextPic(LPCSTR lpszOut, LPCSTR lpszText,
											   long lTextHeight, long lFile, long lPic);
__declspec(GTSCOPE) long WINAPI ScanFile(long lFile, long lCmd, long lData);
__declspec(GTSCOPE) DWORD WINAPI SeekFileView(long lFile, long lPic, long lIncr);
__declspec(GTSCOPE) long WINAPI SeekLoc(long lStream, double d1, double d2,
										   double d3);
__declspec(GTSCOPE) long WINAPI SeekPixel(long lStream, long lX, long lY);
__declspec(GTSCOPE) DWORD WINAPI SeekRunRec(long lStream, long lMode, long lIncr);
__declspec(GTSCOPE) DWORD WINAPI SeekView(long lStream, long lPic, long lIncr);
__declspec(GTSCOPE) DWORD WINAPI SelectProject(long lStream, LPCSTR lpszIn);
__declspec(GTSCOPE) DWORD WINAPI SelectRun(long lStream, long lRun);
__declspec(GTSCOPE) DWORD WINAPI SetCfgPath(LPCSTR lpszIn);
__declspec(GTSCOPE) DWORD WINAPI SetFileFirst(long lFile, long lPic);
__declspec(GTSCOPE) DWORD WINAPI SetFileID(long lFile, LPCSTR lpszIn);
__declspec(GTSCOPE) DWORD WINAPI SetFileLong(long lFile, long lValue);
__declspec(GTSCOPE) DWORD WINAPI SetFileView(long lFile, long lView);
__declspec(GTSCOPE) DWORD WINAPI SetGeoPath(LPCSTR lpszIn);
__declspec(GTSCOPE) DWORD WINAPI SetJPG(long lFile, long lWidth, long lHeight, long lQual);
__declspec(GTSCOPE) DWORD WINAPI SetLimit(long lStream, long lType,
											double dL1, double dL2);
__declspec(GTSCOPE) DWORD WINAPI SetMap(long lStream, double dLat, double dLon,
										  double dHeight, long lWidth, long lHeight,
										  double dXS, double dYS);
__declspec(GTSCOPE) DWORD WINAPI SetPFIPath(LPCSTR lpszIn);
__declspec(GTSCOPE) DWORD WINAPI SetPicPath(LPCSTR lpszIn);
__declspec(GTSCOPE) DWORD WINAPI SetSys(long lParam1, long lParam2, long lValue);
__declspec(GTSCOPE) DWORD WINAPI SetView(long lStream, long lView);
__declspec(GTSCOPE) DWORD WINAPI SetViewCount(long lFile, long lValue);
__declspec(GTSCOPE) DWORD WINAPI SetViewData(long lFile, long lRec, long lValue);
__declspec(GTSCOPE) DWORD WINAPI SetViewLabel(long lFile, long lValue);
__declspec(GTSCOPE) DWORD WINAPI ShowFilePic(HDC hDC,
				long lLeft, long lTop, long lWidth, long lHeight,
				long lFile, long lPic);
__declspec(GTSCOPE) DWORD WINAPI ShowOnePicRaw(HDC hDC,
				long lLeft, long lTop, long lWidth, long lHeight,
				LPCSTR lpszIn, long lFrame);
__declspec(GTSCOPE) DWORD WINAPI ShowPic(HDC hDC,
				long lLeft, long lTop, long lWidth, long lHeight,
				long lStream, long lPic);
__declspec(GTSCOPE) DWORD WINAPI ShowPicRaw(HDC hDC,
				long lLeft, long lTop, long lWidth, long lHeight,
				long lFile, long lFrame);
__declspec(GTSCOPE) DWORD WINAPI Survey(long lStream, LPSTR psx);
__declspec(GTSCOPE) DWORD WINAPI ViewPoint(long lStream, LPSTR psx, long lMode,
				double *Pxyz84);
 
//----------------------------------------------------------------------------

#ifdef __cplusplus
}	// extern "C" {
#endif

