typedef long            GInt32;
typedef unsigned long   GUInt32;
typedef short           GInt16;
typedef unsigned short  GUInt16;
typedef unsigned char   GByte;
typedef long            GBool;

typedef struct {
    long         offset;
    long         size;

    long         element_id;     /*!< Element number (zero based) */
    long         stype;          /*!< Structure type: (DGNST_*) */
    long         level;          /*!< Element Level: 0-63 */
    long         type;           /*!< Element type (DGNT_) */
    long         complex;        /*!< Is element complex? */
    long         deleted;        /*!< Is element deleted? */

    long         graphic_group;  /*!< Graphic group number */
    long         properties;     /*!< Properties: ORing of DGNPF_ flags */
    long         color;          /*!< Color index (0-255) */
    long         weight;         /*!< Line Weight (0-31) */
    long         style;          /*!< Line Style: One of DGNS_* values */

    long         attr_bytes;     /*!< Bytes of attribute data, usually zero. */
    unsigned char *attr_data;   /*!< Raw attribute data */

    long         raw_bytes;      /*!< Bytes of raw data, usually zero. */
    unsigned char *raw_data;    /*!< All raw element data including header. */
} DGNElemCore;
typedef DGNElemCore	_huge	*LPDGNElementCore;

#define CPLE_DGN_ERROR_BASE
#define CPLE_ElementTooBig              CPLE_DGN_ERROR_BASE+1

/**
 * \file dgnlib.h
 *
 * Definitions of public structures and API of DGN Library.
 */

/**
 * DGN Point structure.
 *
 * Note that the DGNReadElement() function transforms points into "master"
 * coordinate system space when they are in the file in UOR (units of
 * resolution) coordinates. 
 */

typedef struct {
    double x;   /*!< X (normally eastwards) coordinate. */
    double y;   /*!< y (normally northwards) coordinate. */
    double z;   /*!< z, up coordinate.  Zero for 2D objects. */
} DGNPoint; 
typedef DGNPoint	_huge	*HPDGNPoint;

/**
 * Element summary information.
 *
 * Minimal information kept about each element if an element summary
 * index is built for a file by DGNGetElementIndex().
 */
typedef struct {
    unsigned char       level;   /*!< Element Level: 0-63 */
    unsigned char       type;    /*!< Element type (DGNT_*) */
    unsigned char       stype;   /*!< Structure type (DGNST_*) */
    unsigned char       flags;   /*!< Other flags */
    long                offset;  /*!< Offset within file (private) */
} DGNElementInfo;  

/** 
 * Multipoint element 
 *
 * The core.stype code is DGNST_MULTIPOINT.
 *
 * Used for: DGNT_LINE(3), DGNT_LINE_STRING(4), DGNT_SHAPE(6), DGNT_CURVE(11),
 * DGNT_BSPLINE(21)
 */

typedef struct {
  DGNElemCore   core;

  long           num_vertices;  /*!< Number of vertices in "vertices" */
  DGNPoint      vertices[2];   /*!< Array of two or more vertices */

} DGNElemMultiPoint;    

/** 
 * Ellipse element 
 *
 * The core.stype code is DGNST_ARC.
 *
 * Used for: DGNT_ELLIPSE(15), DGNT_ARC(16)
 */

typedef struct {
  DGNElemCore   core;

  DGNPoint      origin;         /*!< Origin of ellipse */

  double        primary_axis;   /*!< Primary axis length */
  double        secondary_axis; /*!< Secondary axis length */

  double        rotation;       /*!< Counterclockwise rotation in degrees */
  long          quat[4];

  double        startang;       /*!< Start angle (degrees counterclockwise of primary axis) */
  double        sweepang;       /*!< Sweep angle (degrees) */

} DGNElemArc;

/** 
 * Text element 
 *
 * The core.stype code is DGNST_TEXT.
 *
 * NOTE: Currently we are not capturing the "editable fields" information.
 *
 * Used for: DGNT_TEXT(17).
 */

typedef struct {
    DGNElemCore core;
    
    long        font_id;       /*!< Microstation font id, no list available*/
    long        justification; /*!< Justification, see DGNJ_* */
    double      length_mult;   /*!< Char width in master (if square) */
    double      height_mult;   /*!< Char height in master units */
    double      rotation;      /*!< Counterclockwise rotation in degrees */
    DGNPoint    origin;        /*!< Bottom left corner of text. */
    char        string[1];     /*!< Actual text (length varies, \0 terminated*/
} DGNElemText;

/** 
 * Complex header element 
 *
 * The core.stype code is DGNST_COMPLEX_HEADER.
 *
 * Used for: DGNT_COMPLEX_CHAIN_HEADER(12), DGNT_COMPLEX_SHAPE_HEADER(14),
 *   DGNT_3DSURFACE_HEADER(18) and DGNT_3DSOLID_HEADER(19).
 */

typedef struct {
    DGNElemCore core;
    
    long         totlength;     /*!< Total length of surface */
    long         numelems;      /*!< # of elements in surface */
    long         surftype;      /*!< surface/solid type (only used for 3D surface/solid). One of  DGNSUT_* or DGNSOT_*. */
} DGNElemComplexHeader;

/** 
 * Color table.
 *
 * The core.stype code is DGNST_COLORTABLE.
 *
 * Returned for DGNT_GROUP_DATA(5) elements, with a level number of 
 * DGN_GDL_COLOR_TABLE(1).
 */
typedef unsigned char	GByte;
typedef struct {
  DGNElemCore   core;

  long          screen_flag;
  GByte         color_info[256][3]; /*!< Color table, 256 colors by red (0), green(1) and blue(2) component. */
} DGNElemColorTable;

typedef struct {
    long          flags;
    unsigned char levels[8];
    DGNPoint      origin;
    DGNPoint      delta;
    double        transmatrx[9];
    double        conversion;
    unsigned long activez;
} DGNViewInfo;

/** 
 * Terminal Control Block (header).
 *
 * The core.stype code is DGNST_TCB.
 *
 * Returned for DGNT_TCB(9).
 *
 * The first TCB in the file is used to determine the dimension (2D vs. 3D),
 * and transformation from UOR (units of resolution) to subunits, and subunits
 * to master units.  This is handled transparently within DGNReadElement(), so
 * it is not normally necessary to handle this element type at the application
 * level, though it can be useful to get the sub_units, and master_units names.
 */

typedef struct {
    DGNElemCore core;

    long        dimension;         /*!< Dimension (2 or 3) */

    double      origin_x;       /*!< X origin of UOR space in master units(?)*/
    double      origin_y;       /*!< Y origin of UOR space in master units(?)*/
    double      origin_z;       /*!< Z origin of UOR space in master units(?)*/
    
    long        uor_per_subunit;   /*!< UOR per subunit. */
    char        sub_units[3];      /*!< User name for subunits (2 chars)*/
    long        subunits_per_master; /*!< Subunits per master unit. */
    char        master_units[3];   /*!< User name for master units (2 chars)*/

    DGNViewInfo views[8];

} DGNElemTCB;

/** 
 * Cell Header.
 *
 * The core.stype code is DGNST_CELL_HEADER.
 *
 * Returned for DGNT_CELL_HEADER(2).
 */

typedef struct {
    DGNElemCore core;

    long        totlength;         /*!< Total length of cell */
    char        name[7];           /*!< Cell name */
 unsigned short cclass;            /*!< Class bitmap */
 unsigned short levels[4];         /*!< Levels used in cell */
    
    DGNPoint    rnglow;            /*!< X/Y/Z minimums for cell */
    DGNPoint    rnghigh;           /*!< X/Y/Z minimums for cell */
    
    double      trans[9];          /*!< 2D/3D Transformation Matrix */
    DGNPoint    origin;            /*!< Cell Origin */

    double      xscale;
    double      yscale;
    double      rotation;

} DGNElemCellHeader;

/** 
 * Cell Library.
 *
 * The core.stype code is DGNST_CELL_LIBRARY.
 *
 * Returned for DGNT_CELL_LIBRARY(1).
 */

typedef struct {
    DGNElemCore core;

    short       celltype;          /*!< Cell type. */
    short       attindx;           /*!< Attribute linkage. */
    char        name[7];           /*!< Cell name */

    long        numwords;          /*!< Number of words in cell definition */

    short       dispsymb;          /*!< Display symbol */
 unsigned short cclass;            /*!< Class bitmap */
 unsigned short levels[4];         /*!< Levels used in cell */

    char        description[28];   /*!< Description */
    
} DGNElemCellLibrary;

typedef union { char *string; GInt32 integer; double real; } tagValueUnion;

/** 
 * Tag Value.
 *
 * The core.stype code is DGNST_TAG_VALUE.
 *
 * Returned for DGNT_TAG_VALUE(37).
 */

typedef struct {
    DGNElemCore core;

    long         tagType;           /*!< Tag type indicator, DGNTT_* */
    long         tagSet;            /*!< Which tag set does this relate to? */
    long         tagIndex;          /*!< Tag index within tag set. */
    long         tagLength;         /*!< Length of tag information (text) */
    tagValueUnion tagValue;        /*!< Textual value of tag */

} DGNElemTagValue;

/**
 * Tag definition.
 *
 * Structure holding definition of one tag within a DGNTagSet.
 */
typedef struct _DGNTagDef {
    char        *name;      /*!< Name of this tag. */
    long         id;         /*!< Tag index/identifier. */
    char        *prompt;    /*!< User prompt when requesting value. */
    long         type;       /*!< Tag type (one of DGNTT_STRING(1), DGNTT_INTEGER(3) or DGNTT_FLOAT(4). */
    tagValueUnion defaultValue; /*!< Default tag value */
} DGNTagDef;

#define DGNTT_STRING      1
#define DGNTT_INTEGER     3
#define DGNTT_FLOAT       4

/** 
 * Tag Set.
 *
 * The core.stype code is DGNST_TAG_SET.
 *
 * Returned for DGNT_APPLICATION_ELEM(66), Level: 24.
 */

typedef struct {
    DGNElemCore core;

    long        tagCount;    /*!< Number of tags in tagList. */
    long        tagSet;      /*!< Tag set index. */
    long        flags;       /*!< Tag flags - not too much known. */
    char       *tagSetName; /*!< Tag set name. */

    DGNTagDef  *tagList;    /*!< List of tag definitions in this set. */

} DGNElemTagSet;

/** 
 * Cone element 
 *
 * The core.stype code is DGNST_CONE.
 *
 * Used for: DGNT_CONE(23)
 */
typedef struct {
  DGNElemCore core;

  short unknown;     /*!< Unknown data */
  long quat[4];      /*!< Orientation quaternion */
  DGNPoint center_1; /*!< center of first circle */
  double radius_1;   /*!< radius of first circle */
  DGNPoint center_2; /*!< center of second circle */
  double radius_2;   /*!< radius of second circle */

} DGNElemCone;

/* -------------------------------------------------------------------- */
/*      Structure types                                                 */
/* -------------------------------------------------------------------- */

/** DGNElemCore style: Element uses DGNElemCore structure */
#define DGNST_CORE                 1 

/** DGNElemCore style: Element uses DGNElemMultiPoint structure */
#define DGNST_MULTIPOINT           2 

/** DGNElemCore style: Element uses DGNElemColorTable structure */
#define DGNST_COLORTABLE           3 

/** DGNElemCore style: Element uses DGNElemTCB structure */
#define DGNST_TCB                  4 

/** DGNElemCore style: Element uses DGNElemArc structure */
#define DGNST_ARC                  5 

/** DGNElemCore style: Element uses DGNElemText structure */
#define DGNST_TEXT                 6 

/** DGNElemCore style: Element uses DGNElemComplexHeader structure */
#define DGNST_COMPLEX_HEADER       7

/** DGNElemCore style: Element uses DGNElemCellHeader structure */
#define DGNST_CELL_HEADER          8

/** DGNElemCore style: Element uses DGNElemTagValue structure */
#define DGNST_TAG_VALUE            9

/** DGNElemCore style: Element uses DGNElemTagSet structure */
#define DGNST_TAG_SET             10

/** DGNElemCore style: Element uses DGNElemCellLibrary structure */
#define DGNST_CELL_LIBRARY        11

/** DGNElemCore style: Element uses DGNElemCone structure */
#define DGNST_CONE                12

/* -------------------------------------------------------------------- */
/*      Element types                                                   */
/* -------------------------------------------------------------------- */
#define DGNT_CELL_LIBRARY          1
#define DGNT_CELL_HEADER           2
#define DGNT_LINE                  3
#define DGNT_LINE_STRING           4
#define DGNT_GROUP_DATA            5
#define DGNT_SHAPE                 6
#define DGNT_TEXT_NODE             7
#define DGNT_DIGITIZER_SETUP       8
#define DGNT_TCB                   9
#define DGNT_LEVEL_SYMBOLOGY      10
#define DGNT_CURVE                11
#define DGNT_COMPLEX_CHAIN_HEADER 12
#define DGNT_COMPLEX_SHAPE_HEADER 14
#define DGNT_ELLIPSE              15
#define DGNT_ARC                  16
#define DGNT_TEXT                 17
#define DGNT_3DSURFACE_HEADER     18
#define DGNT_3DSOLID_HEADER       19
#define DGNT_BSPLINE              21
#define DGNT_CONE                 23
#define DGNT_SHARED_CELL_DEFN     34
#define DGNT_SHARED_CELL_ELEM     35
#define DGNT_TAG_VALUE            37
#define DGNT_APPLICATION_ELEM     66

/* -------------------------------------------------------------------- */
/*      Line Styles                                                     */
/* -------------------------------------------------------------------- */
#define DGNS_SOLID              0
#define DGNS_DOTTED             1
#define DGNS_MEDIUM_DASH        2
#define DGNS_LONG_DASH          3
#define DGNS_DOT_DASH           4
#define DGNS_SHORT_DASH         5
#define DGNS_DASH_DOUBLE_DOT    6
#define DGNS_LONG_DASH_SHORT_DASH 7

/* -------------------------------------------------------------------- */
/*      3D Surface Types                                                */
/* -------------------------------------------------------------------- */
#define DGNSUT_SOLID                    0
#define DGNSUT_BOUNDED_PLANE            1
#define DGNSUT_BOUNDED_PLANE2           2
#define DGNSUT_RIGHT_CIRCULAR_CYLINDER  3
#define DGNSUT_RIGHT_CIRCULAR_CONE      4
#define DGNSUT_TABULATED_CYLINDER       5
#define DGNSUT_TABULATED_CONE           6
#define DGNSUT_CONVOLUTE                7
#define DGNSUT_SURFACE_OF_REVOLUTION    8
#define DGNSUT_WARPED_SURFACE           9

/* -------------------------------------------------------------------- */
/*      3D Solid Types                                                  */
/* -------------------------------------------------------------------- */
#define DGNSOT_VOLUME_OF_PROJECTION     0
#define DGNSOT_VOLUME_OF_REVOLUTION     1
#define DGNSOT_BOUNDED_VOLUME           2


/* -------------------------------------------------------------------- */
/*      Class                                                           */
/* -------------------------------------------------------------------- */
#define DGNC_PRIMARY                    0
#define DGNC_PATTERN_COMPONENT          1
#define DGNC_CONSTRUCTION_ELEMENT       2
#define DGNC_DIMENSION_ELEMENT          3
#define DGNC_PRIMARY_RULE_ELEMENT       4
#define DGNC_LINEAR_PATTERNED_ELEMENT   5
#define DGNC_CONSTRUCTION_RULE_ELEMENT  6

/* -------------------------------------------------------------------- */
/*      Group Data level numbers.                                       */
/*                                                                      */
/*      These are symbolic values for the typ 5 (DGNT_GROUP_DATA)       */
/*      level values that have special meanings.                        */
/* -------------------------------------------------------------------- */
#define DGN_GDL_COLOR_TABLE     1
#define DGN_GDL_NAMED_VIEW      3
#define DGN_GDL_REF_FILE        9

/* -------------------------------------------------------------------- */
/*      Word 17 property flags.                                         */
/* -------------------------------------------------------------------- */
#define DGNPF_HOLE         0x8000
#define DGNPF_SNAPPABLE    0x4000
#define DGNPF_PLANAR       0x2000
#define DGNPF_ORIENTATION  0x1000
#define DGNPF_ATTRIBUTES   0x0800
#define DGNPF_MODIFIED     0x0400
#define DGNPF_NEW          0x0200
#define DGNPF_LOCKED       0x0100
#define DGNPF_CLASS        0x000f

/* -------------------------------------------------------------------- */
/*      DGNElementInfo flag values.                                     */
/* -------------------------------------------------------------------- */
#define DGNEIF_DELETED     0x01
#define DGNEIF_COMPLEX     0x02

/* -------------------------------------------------------------------- */
/*      Justifications                                                  */
/* -------------------------------------------------------------------- */
#define DGNJ_LEFT_TOP           0
#define DGNJ_LEFT_CENTER        1
#define DGNJ_LEFT_BOTTOM        2
#define DGNJ_LEFTMARGIN_TOP     3    /* text node header only */
#define DGNJ_LEFTMARGIN_CENTER  4    /* text node header only */
#define DGNJ_LEFTMARGIN_BOTTOM  5    /* text node header only */
#define DGNJ_CENTER_TOP         6
#define DGNJ_CENTER_CENTER      7
#define DGNJ_CENTER_BOTTOM      8
#define DGNJ_RIGHTMARGIN_TOP    9    /* text node header only */
#define DGNJ_RIGHTMARGIN_CENTER 10   /* text node header only */
#define DGNJ_RIGHTMARGIN_BOTTOM 11   /* text node header only */
#define DGNJ_RIGHT_TOP          12
#define DGNJ_RIGHT_CENTER       13
#define DGNJ_RIGHT_BOTTOM       14

/* -------------------------------------------------------------------- */
/*      DGN file reading options.                                       */
/* -------------------------------------------------------------------- */
#define DGNO_CAPTURE_RAW_DATA   0x01

/* -------------------------------------------------------------------- */
/*      Known attribute linkage types, including my synthetic ones.     */
/* -------------------------------------------------------------------- */
#define DGNLT_DMRS              0x0000
#define DGNLT_INFORMIX          0x3848
#define DGNLT_ODBC              0x5e62
#define DGNLT_ORACLE            0x6091
#define DGNLT_RIS               0x71FB
#define DGNLT_SYBASE            0x4f58
#define DGNLT_XBASE             0x1971
#define DGNLT_SHAPE_FILL        0x0041
#define DGNLT_ASSOC_ID          0x7D2F

/* -------------------------------------------------------------------- */
/*      File creation options.                                          */
/* -------------------------------------------------------------------- */

#define DGNCF_USE_SEED_UNITS              0x01
#define DGNCF_USE_SEED_ORIGIN             0x02
#define DGNCF_COPY_SEED_FILE_COLOR_TABLE  0x04
#define DGNCF_COPY_WHOLE_SEED_FILE        0x08


DWORD DGNLibReadElement (HANDLE hDGN,LPDGNElementCore pElement,DWORD MaxDGNElementSize,LPDOUBLE pBaseDistToWinDist,
								  LPLONG pFillColor,LPLONG pNumAttributes,LPLONG Attributes);

