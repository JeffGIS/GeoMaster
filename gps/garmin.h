typedef unsigned long longword;
typedef unsigned char boolean;
typedef unsigned char byte;
typedef unsigned short word;
typedef short Symbol_Type;
typedef struct
{
long lat; /* latitude in semicircles */
long lon; /* longitude in semicircles */
} Semicircle_Type;  
typedef Semicircle_Type FAR	*LPSEMICIRCLETYPE;
/*The following formulas show how to convert between degrees and semicircles:
degrees = semicircles * ( 180 / 2 31 )
semicircles = degrees * ( 2 31 / 180 )
7.4.8. Radian_Type
The floating point Radian_Type is used to indicate latitude and longitude in radians, where  radians equals 180
degrees. North latitudes and East longitudes are indicated with positive numbers; South latitudes and West
longitudes are indicated with negative numbers.*/      
#define	SEMICIRCLETODEGREE	8.3819031715393e-008
typedef struct
{
double lat; /* latitude in radians */
double lon; /* longitude in radians */
} Radian_Type;
enum
{
Pid_Ack_Byte = 6,
Pid_Nak_Byte = 21,
Pid_Protocol_Array = 253, /* may not be implemented in all products */
Pid_Product_Rqst = 254,
Pid_Product_Data = 255
};

enum
{
Pid_Command_Data = 10,
Pid_Xfer_Cmplt = 12,
Pid_Date_Time_Data = 14,
Pid_Position_Data = 17,
Pid_Prx_Wpt_Data = 19,
Pid_Records = 27,
Pid_Rte_Hdr = 29,
Pid_Rte_Wpt_Data = 30,
Pid_Almanac_Data = 31,
Pid_Trk_Data = 34,
Pid_Wpt_Data = 35,
Pid_Pvt_Data = 51,
Pid_Rte_Link_Data = 98,
Pid_Trk_Hdr = 99
};    

enum
{
Tag_Phys_Prot_Id = 'P', /* tag for Physical protocol ID */
Tag_Link_Prot_Id = 'L', /* tag for Link protocol ID */
Tag_Appl_Prot_Id = 'A', /* tag for Application protocol ID */
Tag_Data_Type_Id = 'D' /* tag for Data Type ID */
};

enum
{
Cmnd_Abort_Transfer = 0, /* abort current transfer */
Cmnd_Transfer_Alm = 1, /* transfer almanac */
Cmnd_Transfer_Posn = 2, /* transfer position */
Cmnd_Transfer_Prx = 3, /* transfer proximity waypoints */
Cmnd_Transfer_Rte = 4, /* transfer routes */
Cmnd_Transfer_Time = 5, /* transfer time */
Cmnd_Transfer_Trk = 6, /* transfer track log */
Cmnd_Transfer_Wpt = 7, /* transfer waypoints */
Cmnd_Turn_Off_Pwr = 8, /* turn off power */
Cmnd_Start_Pvt_Data = 49, /* start transmitting PVT data */
Cmnd_Stop_Pvt_Data = 50 /* stop transmitting PVT data */
};

enum
{
/*---------------------------------------------------------------
Symbols for marine (group 0...0-8191...bits 15-13=000).
---------------------------------------------------------------*/
sym_anchor = 0, /* white anchor symbol */
sym_bell = 1, /* white bell symbol */
sym_diamond_grn = 2, /* green diamond symbol */
sym_diamond_red = 3, /* red diamond symbol */
sym_dive1 = 4, /* diver down flag 1 */
sym_dive2 = 5, /* diver down flag 2 */
sym_dollar = 6, /* white dollar symbol */
sym_fish = 7, /* white fish symbol */
sym_fuel = 8, /* white fuel symbol */
sym_horn = 9, /* white horn symbol */
sym_house = 10, /* white house symbol */
sym_knife = 11, /* white knife & fork symbol */
sym_light = 12, /* white light symbol */
sym_mug = 13, /* white mug symbol */
sym_skull = 14, /* white skull and crossbones symbol*/
sym_square_grn = 15, /* green square symbol */
sym_square_red = 16, /* red square symbol */
sym_wbuoy = 17, /* white buoy waypoint symbol */
sym_wpt_dot = 18, /* waypoint dot */
sym_wreck = 19, /* white wreck symbol */
sym_null = 20, /* null symbol (transparent) */
sym_mob = 21, /* man overboard symbol */
/*------------------------------------------------------
marine navaid symbols
------------------------------------------------------*/
sym_buoy_ambr = 22, /* amber map buoy symbol */
sym_buoy_blck = 23, /* black map buoy symbol */
sym_buoy_blue = 24, /* blue map buoy symbol */
sym_buoy_grn = 25, /* green map buoy symbol */
sym_buoy_grn_red = 26, /* green/red map buoy symbol */
sym_buoy_grn_wht = 27, /* green/white map buoy symbol */
sym_buoy_orng = 28, /* orange map buoy symbol */
sym_buoy_red = 29, /* red map buoy symbol */
sym_buoy_red_grn = 30, /* red/green map buoy symbol */
sym_buoy_red_wht = 31, /* red/white map buoy symbol */
sym_buoy_violet = 32, /* violet map buoy symbol */
sym_buoy_wht = 33, /* white map buoy symbol */
sym_buoy_wht_grn = 34, /* white/green map buoy symbol */
sym_buoy_wht_red = 35, /* white/red map buoy symbol */
sym_dot = 36, /* white dot symbol */
sym_rbcn = 37, /* radio beacon symbol */
/*------------------------------------------------------
leave space for more navaids (up to 128 total)
------------------------------------------------------*/
sym_boat_ramp = 150, /* boat ramp symbol */
sym_camp = 151, /* campground symbol */
sym_restrooms = 152, /* restrooms symbol */
sym_showers = 153, /* shower symbol */
sym_drinking_wtr = 154, /* drinking water symbol */
sym_phone = 155, /* telephone symbol */
sym_1st_aid = 156, /* first aid symbol */
sym_info = 157, /* information symbol */
sym_parking = 158, /* parking symbol */
sym_park = 159, /* park symbol */
sym_picnic = 160, /* picnic symbol */
sym_scenic = 161, /* scenic area symbol */
sym_skiing = 162, /* skiing symbol */
sym_swimming = 163, /* swimming symbol */
sym_dam = 164, /* dam symbol */
sym_controlled = 165, /* controlled area symbol */
sym_danger = 166, /* danger symbol */
sym_restricted = 167, /* restricted area symbol */
sym_null_2 = 168, /* null symbol */
sym_ball = 169, /* ball symbol */
sym_car = 170, /* car symbol */
sym_deer = 171, /* deer symbol */
sym_shpng_cart = 172, /* shopping cart symbol */
sym_lodging = 173, /* lodging symbol */
sym_mine = 174, /* mine symbol */
sym_trail_head = 175, /* trail head symbol */
sym_truck_stop = 176, /* truck stop symbol */
/*---------------------------------------------------------------
Symbols for land (group 1...8192-16383...bits 15-13=001).
---------------------------------------------------------------*/
sym_is_hwy = 8192, /* interstate hwy symbol */
sym_us_hwy = 8193, /* us hwy symbol */
sym_st_hwy = 8194, /* state hwy symbol */
sym_mi_mrkr = 8195, /* mile marker symbol */
sym_trcbck = 8196, /* TracBack (feet) symbol */
sym_golf = 8197, /* golf symbol */
sym_sml_cty = 8198, /* small city symbol */
sym_med_cty = 8199, /* medium city symbol */
sym_lrg_cty = 8200, /* large city symbol */
sym_freeway = 8201, /* intl freeway hwy symbol */
sym_ntl_hwy = 8202, /* intl national hwy symbol */
sym_cap_cty = 8203, /* capitol city symbol (star) */
sym_amuse_pk = 8204, /* amusement park symbol */
sym_bowling = 8205, /* bowling symbol */
sym_car_rental = 8206, /* car rental symbol */
sym_car_repair = 8207, /* car repair symbol */
sym_fastfood = 8208, /* fast food symbol */
sym_fitness = 8209, /* fitness symbol */
sym_movie = 8210, /* movie symbol */
sym_museum = 8211, /* museum symbol */
sym_pharmacy = 8212, /* pharmacy symbol */
sym_pizza = 8213, /* pizza symbol */
sym_post_ofc = 8214, /* post office symbol */
sym_rv_park = 8215, /* RV park symbol */
sym_school = 8216, /* school symbol */
sym_stadium = 8217, /* stadium symbol */
sym_store = 8218, /* dept. store symbol */
sym_zoo = 8219, /* zoo symbol */
sym_gas_plus = 8220, /* convenience store symbol */
sym_faces = 8221, /* live theater symbol */
sym_ramp_int = 8222, /* ramp intersection symbol */
sym_st_int = 8223, /* street intersection symbol */
sym_weigh_sttn = 8226, /* inspection/weigh station symbol */
sym_toll_booth = 8227, /* toll booth symbol */
sym_elev_pt = 8228, /* elevation point symbol */
sym_ex_no_srvc = 8229, /* exit without services symbol */
sym_geo_place_mm = 8230, /* Geographic place name, man-made */
sym_geo_place_wtr = 8231, /* Geographic place name, water */
sym_geo_place_lnd = 8232, /* Geographic place name, land */
/*---------------------------------------------------------------
Symbols for aviation (group 2...16383-24575...bits 15-13=010).
---------------------------------------------------------------*/
sym_airport = 16384, /* airport symbol */
sym_int = 16385, /* intersection symbol */
sym_ndb = 16386, /* non-directional beacon symbol */
sym_vor = 16387, /* VHF omni-range symbol */
sym_heliport = 16388, /* heliport symbol */
sym_private = 16389, /* private field symbol */
sym_soft_fld = 16390, /* soft field symbol */
sym_tall_tower = 16391, /* tall tower symbol */
sym_short_tower = 16392, /* short tower symbol */
sym_glider = 16393, /* glider symbol */
sym_ultralight = 16394, /* ultralight symbol */
sym_parachute = 16395, /* parachute symbol */
sym_vortac = 16396, /* VOR/TACAN symbol */
sym_vordme = 16397, /* VOR-DME symbol */
sym_faf = 16398, /* first approach fix */
sym_lom = 16399, /* localizer outer marker */
sym_map = 16400, /* missed approach point */
sym_tacan = 16401, /* TACAN symbol */
sym_seaplane = 16402, /* Seaplane Base */
}; 
/*
7.5.1. D100_Wpt_Type
Example products: GPS 38, GPS 40, GPS 45, GPS 75 and GPS II.  
*/
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
} D100_Wpt_Type;

/*7.5.2. D101_Wpt_Type
Example products: GPSMAP 210 and GPSMAP 220 (both prior to version 4.00).
*/
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
float dst; /* proximity distance (meters) */
byte smbl; /* symbol id */
} D101_Wpt_Type;

/*The enumerated values for the “smbl” member of the D101_Wpt_Type are the same as those for Symbol_Type (see
Section 7.4.9 on page 29). However, since the “smbl” member of the D101_Wpt_Type is only 8-bits (instead of 16-
bits), all Symbol_Type values whose upper byte is non-zero are unallowed in the D101_Wpt_Type.
The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
7.5.3. D102_Wpt_Type
Example products: GPSMAP 175, GPSMAP 210 and GPSMAP 220. 
*/
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
float dst; /* proximity distance (meters) */
Symbol_Type smbl; /* symbol id */
} D102_Wpt_Type;
/*
The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
7.5.4. D103_Wpt_Type
Example products: GPS 12, GPS 12 XL, GPS 48 and GPS II Plus. 
*/
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
byte smbl; /* symbol id */
byte dspl; /* display option */
} D103_Wpt_Type; 

/*The enumerated values for the “smbl” member of the D103_Wpt_Type are shown below:*/
enum
{
smbl_dot = 0, /* dot symbol */
smbl_house = 1, /* house symbol */
smbl_gas = 2, /* gas symbol */
smbl_car = 3, /* car symbol */
smbl_fish = 4, /* fish symbol */
smbl_boat = 5, /* boat symbol */
smbl_anchor = 6, /* anchor symbol */
smbl_wreck = 7, /* wreck symbol */
smbl_exit = 8, /* exit symbol */
smbl_skull = 9, /* skull symbol */
smbl_flag = 10, /* flag symbol */
smbl_camp = 11, /* camp symbol */
smbl_circle_x = 12, /* circle with x symbol */
smbl_deer = 13, /* deer symbol */
smbl_1st_aid = 14, /* first aid symbol */
smbl_back_track = 15 /* back track symbol */
};
/*The enumerated values for the “dspl” member of the D103_Wpt_Type are shown below:*/
enum
{
dspl_name = 0, /* Display symbol with waypoint name */
dspl_none = 1, /* Display symbol by itself */
dspl_cmnt = 2 /* Display symbol with comment */
};

/*7.5.5. D104_Wpt_Type
Example products: GPS III.*/
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
float dst; /* proximity distance (meters) */
Symbol_Type smbl; /* symbol id */
byte dspl; /* display option */
} D104_Wpt_Type;
//The enumerated values for the “dspl” member of the D104_Wpt_Type are shown below:
enum
{
dspl_smbl_only = 1, /* Display symbol by itself */
dspl_smbl_name = 3, /* Display symbol with waypoint name */
dspl_smbl_cmnt = 5, /* Display symbol with comment */
};

/*The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
7.5.6. D105_Wpt_Type
Example products: StreetPilot (user waypoints).*/
typedef struct
{
Semicircle_Type posn; /* position */
Symbol_Type smbl; /* symbol id */
/* char wpt_ident[]; null-terminated string */
} D105_Wpt_Type;

/*7.5.7. D106_Wpt_Type
Example products: StreetPilot (route waypoints). */
typedef struct
{
byte wpt_class; /* class */
byte subclass[13]; /* subclass */
Semicircle_Type posn; /* position */
Symbol_Type smbl; /* symbol id */
/* char wpt_ident[]; null-terminated string */
/* char lnk_ident[]; null-terminated string */
} D106_Wpt_Type; 
/*
The enumerated values for the “wpt_class” member of the D106_Wpt_Type are as follows:
Zero: indicates a user waypoint (“subclass” is ignored).
Non-zero: indicates a non-user waypoint (“subclass” must be valid). */
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
byte smbl; /* symbol id */
byte dspl; /* display option */
float dst; /* proximity distance (meters) */
byte color; /* waypoint color */
} D107_Wpt_Type; 
/*
The enumerated values for the “smbl” member of the D107_Wpt_Type are the same as the the “smbl” member of
the D103_Wpt_Type.
The enumerated values for the “dspl” member of the D107_Wpt_Type are shown below are the same as the the
“dspl” member of the D103_Wpt_Type.
The enumerated values for the “color” member of the D107_Wpt_Type are shown below: */
enum
{
clr_default = 0, /* Default waypoint color */
clr_red = 1, /* Red */
clr_green = 2, /* Green */
clr_blue = 3 /* Blue */
}; 
typedef struct /* size */
{
byte wpt_class; /* class (see below) 1 */
byte color; /* color (see below) 1 */
byte dspl; /* display options (see below) 1 */
byte attr; /* attributes (see below) 1 */
Symbol_Type smbl; /* waypoint symbol 2 */
byte subclass[18]; /* subclass 18 */
Semicircle_Type posn; /* 32 bit semicircle 8 */
float alt; /* altitude in meters 4 */
float dpth; /* depth in meters 4 */
float dist; /* proximity distance in meters 4 */
char state[2]; /* state 2 */
char cc[2]; /* country code 2 */
 char ident[266]; /*variable length string 1-51 */
/* char comment[]; waypoint user comment 1-51 */
/* char facility[]; facility name 1-31 */
/* char city[]; city name 1-25 */
/* char addr[]; address number 1-51 */
/* char cross_road[]; intersecting road label 1-51 */
} D108_Wpt_Type;
//The enumerated values for the “wpt_class” member of the D108_Wpt_Type are defined as follows:
enum
{
USER_WPT = 0x00, /* User waypoint */
AVTN_APT_WPT = 0x40, /* Aviation Airport waypoint */
AVTN_INT_WPT = 0x41, /* Aviation Intersection waypoint */
AVTN_NDB_WPT = 0x42, /* Aviation NDB waypoint */
AVTN_VOR_WPT = 0x43, /* Aviation VOR waypoint */
AVTN_ARWY_WPT = 0x44, /* Aviation Airport Runway waypoint */
AVTN_AINT_WPT = 0x45, /* Aviation Airport Intersection */
AVTN_ANDB_WPT = 0x46, /* Aviation Airport NDB waypoint */
MAP_PNT_WPT = 0x80, /* Map Point waypoint */
MAP_AREA_WPT = 0x81, /* Map Area waypoint */
MAP_INT_WPT = 0x82, /* Map Intersection waypoint */
MAP_ADRS_WPT = 0x83, /* Map Address waypoint */
MAP_LABEL_WPT = 0x84, /* Map Label Waypoint */
MAP_LINE_WPT = 0x85, /* Map Line Waypoint */
};
//The “color” member can be one of the following values:
enum { Black, Dark_Red, Dark_Green, Dark_Yellow,
Dark_Blue, Dark_Magenta, Dark_Cyan, Light_Gray,
Dark_Gray, Red, Green, Yellow,
Blue, Magenta, Cyan, White,
Default_Color = 0xFF };
//The enumerated values for the “dspl” member of the D108_Wpt_Type are the same as the the “dspl” member of the
//D103_Wpt_Type.
//The “attr” member should be set to a value of 0x60.
//The “subclass” member of the D108_Wpt_Type is used for map waypoints only, and should be set to 0x0000
//0x00000000 0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF for other classes of waypoints.
//The “alt” and “dpth” members may or may not be supported on a given unit. A value of 1.0e25 in either of these
//fields indicates that this parameter is not supported or is unknown for this waypoint.
//The “dist” member is used during the Proximity Waypoint Transfer Protocol only, and should be set to zero for
//other cases.
//The “comment” member of the D108_Wpt_Type is used for user waypoints only, and should be an empty string for
//other waypoint classes.
//The “facility” and “city” members are used only for aviation waypoints, and should be empty strings for other
//waypoint classes.
//The “addr” member is only valid for MAP_ADRS_WPT class waypoints and will be an empty string otherwise.
//The “cross_road” member is valid only for MAP_INT_WPT class waypoints, and will be an empty string otherwise.
/*7.5.9. D150_Wpt_Type
Example products: GPS 150, GPS 155, GNC 250 and GNC 300.*/

    typedef struct                      /*                                 size */
        {
        byte            dtyp;           /* data packet type (0x01 for D109)1    */
        byte            wpt_class;      /* class                           1    */
        byte            dspl_color;     /* display & color (see below)     1    */
        byte            attr;           /* attributes (0x70 for D109)      1    */
        Symbol_Type     smbl;           /* waypoint symbol                 2    */
        byte            subclass[18];   /* subclass                        18   */
        Semicircle_Type posn;           /* 32 bit semicircle               8    */
        float           alt;            /* altitude in meters              4    */
        float           dpth;           /* depth in meters                 4    */
        float           dist;           /* proximity distance in meters    4    */
        char            state[2];       /* state                           2    */
        char            cc[2];          /* country code                    2    */
        longword        ete;            /* outbound link ete in seconds    4    */
        char            ident[266];        /* variable length string          1-51 */
    /*  char            comment[];         waypoint user comment           1-51 */
    /*  char            facility[];        facility name                   1-31 */
    /*  char            city[];            city name                       1-25 */
    /*  char            addr[];            address number                  1-51 */
    /*  char            cross_road[];      intersecting road label         1-51 */
        } D109_Wpt_Type;
typedef D109_Wpt_Type	FAR	*LPD109_Wpt_Type;

/*All fields are defined the same as D108 except as noted below.

dtyp - Data packet type, must be 0x01 for D109.

dsp_color - The 'dspl_color' member contains three fields; bits 0-4 specify
the color, bits 5-6 specify the waypoint display attribute and bit 7 is unused
and must be 0. Color values are as specified for D108 except that the default
value is 0x1f. Display attribute values are as specified for D108.

attr - Attribute. Must be 0x70 for D109.

ete - Estimated time en route in seconds to next waypoint. Default value is
0xffffffff.*/

    typedef struct                      /*                                 size */
        {
        byte            dtyp;           /* data packet type (0x01 for D110)1    */
        byte            wpt_class;      /* class                           1    */
        byte            dspl_color;     /* display & color (see below)     1    */
        byte            attr;           /* attributes (0x80 for D110)      1    */
        Symbol_Type     smbl;           /* waypoint symbol                 2    */
        byte            subclass[18];   /* subclass                        18   */
        Semicircle_Type posn;           /* 32 bit semicircle               8    */
        float           alt;            /* altitude in meters              4    */
        float           dpth;           /* depth in meters                 4    */
        float           dist;           /* proximity distance in meters    4    */
        char            state[2];       /* state                           2    */
        char            cc[2];          /* country code                    2    */
        longword        ete;            /* outbound link ete in seconds    4    */
		float 			temp;			/* temperature 					   4	*/
		longword 		time; 			/* timestamp					   4	*/
		short 			wpt_cat;		/* category membership			   2	*/
        char            ident[266];        /* variable length string          1-51 */
    /*  char            comment[];         waypoint user comment           1-51 */
    /*  char            facility[];        facility name                   1-31 */
    /*  char            city[];            city name                       1-25 */
    /*  char            addr[];            address number                  1-51 */
    /*  char            cross_road[];      intersecting road label         1-51 */
        } D110_Wpt_Type; //sizeof(D110_Wpt_Type)
typedef D110_Wpt_Type	FAR	*LPD110_Wpt_Type;


typedef struct
{
char ident[6]; /* identifier */
char cc[2]; /* country code */
byte wpt_class; /* class */
Semicircle_Type posn; /* position */
int alt; /* altitude (meters) */
char city[24]; /* city */
char state[2]; /* state */
char name[30]; /* facility name */
char cmnt[40]; /* comment */
} D150_Wpt_Type;
//The enumerated values for the “wpt_class” member of the D150_Wpt_Type are shown below:
enum
{
apt_wpt_class_D150 = 0, /* airport waypoint class */
int_wpt_class_D150 = 1, /* intersection waypoint class */
ndb_wpt_class_D150 = 2, /* NDB waypoint class */
vor_wpt_class_D150 = 3, /* VOR waypoint class */
usr_wpt_class_D150 = 4, /* user defined waypoint class */
rwy_wpt_class_D150 = 5, /* airport runway threshold waypoint class */
aint_wpt_class_D150 = 6 /* airport intersection waypoint class */
};
/*7.5.10. D151_Wpt_Type
Example products: GPS 55 AVD, GPS 89. */
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
float dst; /* proximity distance (meters) */
char name[30]; /* facility name */
char city[24]; /* city */
char state[2]; /* state */
int alt; /* altitude (meters) */
char cc[2]; /* country code */
char unused2; /* should be set to zero */
byte wpt_class; /* class */
} D151_Wpt_Type;
//The enumerated values for the “wpt_class” member of the D151_Wpt_Type are shown below:
enum
{
apt_wpt_class_D151 = 0, /* airport waypoint class */
vor_wpt_class_D151 = 1, /* VOR waypoint class */
usr_wpt_class_D151 = 2 /* user defined waypoint class */
};
/*The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
The “city,” “state,” “name,” and “cc” members are invalid when the “wpt_class” member is equal to usr_wpt_class.
The “alt” member is valid only when the “wpt_class” member is equal to apt_wpt_class.
7.5.11. D152_Wpt_Type
Example products: GPS 90, GPS 95 AVD, GPS 95 XL and GPSCOM 190.  */
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
float dst; /* proximity distance (meters) */
char name[30]; /* facility name */
char city[24]; /* city */
char state[2]; /* state */
int alt; /* altitude (meters) */
char cc[2]; /* country code */
char unused2; /* should be set to zero */
byte wpt_class; /* class */
} D152_Wpt_Type;
//The enumerated values for the “wpt_class” member of the D152_Wpt_Type are shown below:
enum
{
apt_wpt_class = 0, /* airport waypoint class */
int_wpt_class = 1, /* intersection waypoint class */
ndb_wpt_class = 2, /* NDB waypoint class */
vor_wpt_class = 3, /* VOR waypoint class */
usr_wpt_class = 4 /* user defined waypoint class */
};
/*The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
The “city,” “state,” “name,” and “cc” members are invalid when the “wpt_class” member is equal to usr_wpt_class.
The “alt” member is valid only when the “wpt_class” member is equal to apt_wpt_class.
7.5.12. D154_Wpt_Type
Example products: GPSMAP 195.*/
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
float dst; /* proximity distance (meters) */
char name[30]; /* facility name */
char city[24]; /* city */
char state[2]; /* state */
int alt; /* altitude (meters) */
char cc[2]; /* country code */
char unused2; /* should be set to zero */
byte wpt_class; /* class */
Symbol_Type smbl; /* symbol id */
} D154_Wpt_Type;
//The enumerated values for the “wpt_class” member of the D154_Wpt_Type are shown below:
enum
{
apt_wpt_class_D154 = 0, /* airport waypoint class */
int_wpt_class_D154 = 1, /* intersection waypoint class */
ndb_wpt_class_D154 = 2, /* NDB waypoint class */
vor_wpt_class_D154 = 3, /* VOR waypoint class */
usr_wpt_class_D154 = 4, /* user defined waypoint class */
rwy_wpt_class_D154 = 5, /* airport runway threshold waypoint class */
aint_wpt_class_D154 = 6, /* airport intersection waypoint class */
andb_wpt_class_D154 = 7, /* airport NDB waypoint class */
sym_wpt_class_D154 = 8 /* user defined symbol-only waypoint class */
};
/*The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
The “city,” “state,” “name,” and “cc” members are invalid when the “wpt_class” member is equal to usr_wpt_class
or sym_wpt_class. The “alt” member is valid only when the “wpt_class” member is equal to apt_wpt_class.
7.5.13. D155_Wpt_Type
Example products: GPS III Pilot. */
typedef struct
{
char ident[6]; /* identifier */
Semicircle_Type posn; /* position */
longword unused; /* should be set to zero */
char cmnt[40]; /* comment */
float dst; /* proximity distance (meters) */
char name[30]; /* facility name */
char city[24]; /* city */
char state[2]; /* state */
int alt; /* altitude (meters) */
char cc[2]; /* country code */
char unused2; /* should be set to zero */
byte wpt_class; /* class */
Symbol_Type smbl; /* symbol id */
byte dspl; /* display option */
} D155_Wpt_Type;
//The enumerated values for the “dspl” member of the D155_Wpt_Type are shown below:
enum
{
dspl_smbl_only_D155 = 1, /* Display symbol by itself */
dspl_smbl_name_D155 = 3, /* Display symbol with waypoint name */
dspl_smbl_cmnt_D155 = 5, /* Display symbol with comment */
};
//The enumerated values for the “wpt_class” member of the D155_Wpt_Type are shown below:
enum
{
apt_wpt_class_D155 = 0, /* airport waypoint class */
int_wpt_class_D155 = 1, /* intersection waypoint class */
ndb_wpt_class_D155 = 2, /* NDB waypoint class */
vor_wpt_class_D155 = 3, /* VOR waypoint class */
usr_wpt_class_D155 = 4 /* user defined waypoint class */
};
/*The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
The “city,” “state,” “name,” and “cc” members are invalid when the “wpt_class” member is equal to usr_wpt_class.
The “alt” member is valid only when the “wpt_class” member is equal to apt_wpt_class.
7.5.14. D200_Rte_Hdr_Type
Example products: GPS 55 and GPS 55 AVD. */
typedef byte D200_Rte_Hdr_Type; /* route number */
/*7.5.15. D201_Rte_Hdr_Type
Example products: all products unless otherwise noted.  */
typedef struct
{
byte nmbr; /* route number */
char cmnt[20]; /* comment */
} D201_Rte_Hdr_Type;
/*7.5.16. D202_Rte_Hdr_Type
Example products: StreetPilot. */
typedef struct
{
 char rte_ident[]; /*null-terminated string */
} D202_Rte_Hdr_Type;
/*7.5.17. D300_Trk_Point_Type
Example products: all products unless otherwise noted.*/
typedef struct
{
Semicircle_Type posn; /* position */
longword time; /* time */
boolean new_trk; /* new track segment? */
} D300_Trk_Point_Type; 
typedef	D300_Trk_Point_Type FAR	*LPD300_Trk_Point_Type;
/*The “time” member provides a timestamp for the track log point. This time is expressed as the number of seconds
since 12:00 AM on December 31 st , 1989.
When true, the “new_trk” member indicates that the track log point marks the beginning of a new track log segment.
7.5.18. D400_Prx_Wpt_Type
Example products: GPS 55 and GPS 75. */
typedef struct
{
D100_Wpt_Type wpt; /* waypoint */
float dst; /* proximity distance (meters) */
} D400_Prx_Wpt_Type;
/*The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
7.5.19. D403_Prx_Wpt_Type
Example products: GPS 12, GPS 12 XL and GPS 48.*/
typedef struct
{
D103_Wpt_Type wpt; /* waypoint */
float dst; /* proximity distance (meters) */
} D403_Prx_Wpt_Type;
/*The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
7.5.20. D450_Prx_Wpt_Type
Example products: GPS 150, GPS 155, GNC 250 and GNC 300.*/
typedef struct
{
int idx; /* proximity index */
D150_Wpt_Type wpt; /* waypoint */
float dst; /* proximity distance (meters) */
} D450_Prx_Wpt_Type;
/*The “dst” member is valid only during the Proximity Waypoint Transfer Protocol.
7.5.21. D500_Almanac_Type
Example products: GPS 38, GPS 40, GPS 45, GPS 55, GPS 75, GPS 95 and GPS II.*/
typedef struct
{
int wn; /* week number (weeks) */
float toa; /* almanac data reference time (s) */
float af0; /* clock correction coefficient (s) */
float af1; /* clock correction coefficient (s/s) */
float e; /* eccentricity (-) */
float sqrta; /* square root of semi-major axis (a) (m**1/2) */
float m0; /* mean anomaly at reference time (r) */
float w; /* argument of perigee (r) */
float omg0; /* right ascension (r) */
float odot; /* rate of right ascension (r/s) */
float i; /* inclination angle (r) */
} D500_Almanac_Type;
/*7.5.22. D501_Almanac_Type
Example products: GPS 12, GPS 12 XL, GPS 48, GPS II Plus and GPS III.*/
typedef struct
{
int wn; /* week number (weeks) */
float toa; /* almanac data reference time (s) */
float af0; /* clock correction coefficient (s) */
float af1; /* clock correction coefficient (s/s) */
float e; /* eccentricity (-) */
float sqrta; /* square root of semi-major axis (a) (m**1/2) */
float m0; /* mean anomaly at reference time (r) */
float w; /* argument of perigee (r) */
float omg0; /* right ascension (r) */
float odot; /* rate of right ascension (r/s) */
float i; /* inclination angle (r) */
byte hlth; /* almanac health */
} D501_Almanac_Type;
/*7.5.23. D550_Almanac_Type
Example products: GPS 150, GPS 155, GNC 250 and GNC 300.8*/
typedef struct
{
char svid; /* satellite id */
int wn; /* week number (weeks) */
float toa; /* almanac data reference time (s) */
float af0; /* clock correction coefficient (s) */
float af1; /* clock correction coefficient (s/s) */
float e; /* eccentricity (-) */
float sqrta; /* square root of semi-major axis (a) (m**1/2) */
float m0; /* mean anomaly at reference time (r) */
float w; /* argument of perigee (r) */
float omg0; /* right ascension (r) */
float odot; /* rate of right ascension (r/s) */
float i; /* inclination angle (r) */
} D550_Almanac_Type;
/*The “svid” member identifies a satellite in the GPS constellation as follows: PRN-01 through PRN-32 are indicated
by “svid” equal to 0 through 31, respectively.
7.5.24. D551_Almanac_Type
Example products: GPS 150 XL, GPS 155 XL, GNC 250 XL and GNC 300 XL.*/
typedef struct
{
char svid; /* satellite id */
int wn; /* week number (weeks) */
float toa; /* almanac data reference time (s) */
float af0; /* clock correction coefficient (s) */
float af1; /* clock correction coefficient (s/s) */
float e; /* eccentricity (-) */
float sqrta; /* square root of semi-major axis (a) (m**1/2) */
float m0; /* mean anomaly at reference time (r) */
float w; /* argument of perigee (r) */
float omg0; /* right ascension (r) */
float odot; /* rate of right ascension (r/s) */
float i; /* inclination angle (r) */
byte hlth; /* almanac health bits 17:24 (coded) */
} D551_Almanac_Type;
/*The “svid” member identifies a satellite in the GPS constellation as follows: PRN-01 through PRN-32 are indicated
by “svid” equal to 0 through 31, respectively.
7.5.25. D600_Date_Time_Type
Example products: all products unless otherwise noted.*/
typedef struct
{
byte month; /* month (1-12) */
byte day; /* day (1-31) */
word year; /* year (1990 means 1990) */
int hour; /* hour (0-23) */
byte minute; /* minute (0-59) */
byte second; /* second (0-59) */
} D600_Date_Time_Type;
/*The D600_Date_Time_Type contains the UTC date and UTC time.
7.5.26. D700_Position_Type
Example products: all products unless otherwise noted.*/
typedef Radian_Type D700_Position_Type;
/*7.5.27. D800_Pvt_Data_Type
Example products: GPS III and StreetPilot.*/
typedef struct
{
float alt; /* altitude above WGS 84 ellipsoid (meters) */
float epe; /* estimated position error, 2 sigma (meters) */
float eph; /* epe, but horizontal only (meters) */
float epv; /* epe, but vertical only (meters) */
int fix; /* type of position fix */
double tow; /* time of week (seconds) */
Radian_Type posn; /* latitude and longitude (radians) */
float east; /* velocity east (meters/second) */
float north; /* velocity north (meters/second) */
float up; /* velocity up (meters/second) */
float msl_hght; /* height of WGS 84 ellipsoid above MSL (meters) */
int leap_scnds; /* difference between GPS and UTC (seconds) */
long wn_days; /* week number days */
} D800_Pvt_Data_Type;
/*
The “alt” parameter provides the altitude above the WGS 84 ellipsoid. To find the altitude above mean sea level, add
“msl_ hght” to “alt” (“msl_hght” gives the height of the WGS 84 ellipsoid above mean sea level at the current
position).
The “tow” parameter provides the number of seconds (excluding leap seconds) since the beginning of the current
week, which begins on Sunday at 12:00 AM (i.e., midnight Saturday night-Sunday morning). The “tow” parameter
is based on Universal Coordinated Time (UTC), except UTC is periodically corrected for leap seconds while “tow”
is not corrected for leap seconds. To find UTC, subtract “leap_scnds” from “tow.” Since this may cause a negative
result for the first few seconds of the week (i.e., when “tow” is less than “leap_scnds”), care must be taken to
properly translate this negative result to a positive time value in the previous day. Also, since “tow” is a floating
point number and may contain fractional seconds, care must be taken to properly round off when using “tow” in
integer conversions and calculations.
The “wn_days” parameter provides the number of days that have occurred from December 31 st , 1989 to the
beginning of the current week (thus, “wn_days” always represents a Sunday). To find the total number of days that
have occurred from December 31 st , 1989 to the current day, add “wn_days” to the number of days that have
occurred in the current week (as calculated from the “tow” parameter).
The enumerated values for the “fix” member of the D800_Pvt_Data_Type are shown below. It is important for the
Host to inspect this value to ensure that other data members in the D800_Pvt_Data_Type are valid. No indication is
given as to whether the GPS is in simulator mode versus having an actual position fix.
*/
enum
{
unusable = 0, /* failed integrity check */
invalid = 1, /* invalid or unavailable */
x2D = 2, /* two dimensional */
x3D = 3, /* three dimensional */
x2D_diff = 4, /* two dimensional differential */
x3D_diff = 5 /* three dimensional differential */
};

//7.5.18. D210_Rte_Link_Type
//Example products: GPSMAP 162/168, eMap, GPSMAP 295.
typedef struct
{
word class; /* link class; see below */
byte subclass[18]; /* sublcass */
/* char ident[]; variable length string */
}D210_Rte_Link_Type;
/*The “class” member can be one of the following values:
enum
{
line = 0,
link = 1,
net = 2,
direct = 3,
snap = 0xFF
};
The “ident” member has a maximum length of 51 characters, including the terminating NULL.
If “class” is set to “direct” or “snap”, subclass should be set to its default value of 0x0000 0x00000000
0xFFFFFFFF 0xFFFFFFFF 0xFFFFFFFF. */