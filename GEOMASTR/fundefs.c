#include "graphint.h"

#include "gmextern.h"

int	GetFunctionID1 (LPSTR str)
{
			if (!_fstrnicmp (str,"D",1)) return 101;
			if (!_fstrnicmp (str,"L",1)) return 102;
			if (!_fstrnicmp (str,"X",1)) return 103;
			if (!_fstrnicmp (str,"Y",1)) return 104;
			if (!_fstrnicmp (str,"C",1)) return 105;
			if (!_fstrnicmp (str,"B",1)) return 106;
			if (!_fstrnicmp(str, "E", 1)) return 107;
			if (!_fstrnicmp(str, "M", 1)) return 108;
			return 0;
}
int	GetFunctionID2 (LPSTR str)
{
			if (!_fstrnicmp (str,"OS",2)) return 201;
			if (!_fstrnicmp (str,"LV",2)) return 202;
			if (!_fstrnicmp (str,"RV",2)) return 203;
			if (!_fstrnicmp (str,"VP",2)) return 204;
			return 0;
}
int	GetFunctionID3 (LPSTR str)
{
			if (!_fstrnicmp (str,"RND",3)) return 301;
			if (!_fstrnicmp (str,"CMA",3)) return 302;
			if (!_fstrnicmp (str,"OPV",3)) return 303;
			if (!_fstrnicmp (str,"RUN",3)) return 304;
			if (!_fstrnicmp (str,"CAL",3)) return 305;
			if (!_fstrnicmp (str,"HLT",3)) return 306;
			if (!_fstrnicmp (str,"ZMV",3)) return 307;
			if (!_fstrnicmp (str,"MID",3)) return 308;
			if (!_fstrnicmp (str,"RGB",3)) return 309;
			if (!_fstrnicmp (str,"TAB",3)) return 310;
			if (!_fstrnicmp (str,"CLK",3)) return 311;
			if (!_fstrnicmp (str,"CMD",3)) return 312;
			if (!_fstrnicmp (str,"STR",3)) return 313;
			if (!_fstrnicmp (str,"LWR",3)) return 314;
			if (!_fstrnicmp (str,"UPR",3)) return 315;
			if (!_fstrnicmp (str,"FLT",3)) return 316;
			if (!_fstrnicmp (str,"DDE",3)) return 317;
			if (!_fstrnicmp (str,"SET",3)) return 318;
			if (!_fstrnicmp (str,"RAW",3)) return 319;
			if (!_fstrnicmp (str,"WEB",3)) return 320;
			if (!_fstrnicmp (str,"VIS",3)) return 321;
			if (!_fstrnicmp (str,"GPS",3)) return 322;
			if (!_fstrnicmp (str,"DMS",3)) return 323;
			if (!_fstrnicmp (str,"CFG",3)) return 324;
			if (!_fstrnicmp (str,"LEN",3)) return 325;
			if (!_fstrnicmp (str,"HEX",3)) return 326;
			if (!_fstrnicmp (str,"PIK",3)) return 327;
			if (!_fstrnicmp (str,"MON",3)) return 328;
			if (!_fstrnicmp (str,"DOW",3)) return 329;
			if (!_fstrnicmp (str,"SQL",3)) return 330;
			if (!_fstrnicmp (str,"CHR",3)) return 331;
			if (!_fstrnicmp (str,"ELV",3)) return 332;
			if (!_fstrnicmp (str,"DTM",3)) return 333;
			if (!_fstrnicmp (str,"INT",3)) return 334;
			if (!_fstrnicmp (str,"INC",3)) return 335;
			if (!_fstrnicmp (str,"DEC",3)) return 336;
			if (!_fstrnicmp (str,"PAD",3)) return 337;
			if (!_fstrnicmp (str,"SUM",3)) return 338;
			if (!_fstrnicmp (str,"MIN",3)) return 339;
			if (!_fstrnicmp (str,"MAX",3)) return 340;
			if (!_fstrnicmp (str,"DOY",3)) return 341;
			if (!_fstrnicmp (str,"DOM",3)) return 342;
			if (!_fstrnicmp (str,"AZM",3)) return 343;
			if (!_fstrnicmp (str,"RDF",3)) return 344;
			if (!_fstrnicmp (str,"ABS",3)) return 345;
			if (!_fstrnicmp (str,"AVE",3)) return 346;
			if (!_fstrnicmp (str,"GMD",3)) return 347;
			if (!_fstrnicmp (str,"MOD",3)) return 348;
			if (!_fstrnicmp (str,"NUM",3)) return 349;
			if (!_fstrnicmp (str,"SUN",3)) return 350;
			if (!_fstrnicmp (str,"HSL",3)) return 351;
			//if (!_fstrnicmp (str,"STR",3)) return 352;
			if (!_fstrnicmp (str,"LIT",3)) return 353;
			if (!_fstrnicmp (str,"COG",3)) return 354;
			if (!_fstrnicmp (str,"FTP",3)) return 355;
			if (!_fstrnicmp (str,"DSN",3)) return 356;
			if (!_fstrnicmp(str, "VAR", 3)) return 357;
			if (!_fstrnicmp(str, "TIN", 3)) return 358;
			if (!_fstrnicmp(str, "TCP", 3)) return 359;
			if (!_fstrnicmp(str, "SHP", 3)) return 360;
			return 0;
}
int	GetFunctionID4 (LPSTR str)
{
			if (!_fstrnicmp (str,"ZOOM",4)) return 401; 
			if (!_fstrnicmp (str,"FCHR",4)) return 405; 
			if (!_fstrnicmp (str,"LCHR",4)) return 406; 
			if (!_fstrnicmp (str,"FILL",4)) return 407; 
			if (!_fstrnicmp (str,"OPEN",4)) return 408; 
			if (!_fstrnicmp (str,"EDIT",4)) return 409; 
			if (!_fstrnicmp (str,"BOOL",4)) return 410; 
			if (!_fstrnicmp (str,"INDX",4)) return 411; 
			if (!_fstrnicmp (str,"TEST",4)) return 412; 
			if (!_fstrnicmp (str,"MASK",4)) return 413; 
			if (!_fstrnicmp (str,"SAVE",4)) return 414; 
			if (!_fstrnicmp (str,"DIST",4)) return 415; 
			if (!_fstrnicmp (str,"MENU",4)) return 416; 
			if (!_fstrnicmp (str,"HOUR",4)) return 417; 
			if (!_fstrnicmp (str,"TRIM",4)) return 418; 
			if (!_fstrnicmp (str,"PICK",4)) return 419; 
			if (!_fstrnicmp (str,"NULL",4)) return 420; 
			if (!_fstrnicmp (str,"SNAP",4)) return 421; 
			if (!_fstrnicmp (str,"EXIT",4)) return 422; 
			if (!_fstrnicmp (str,"UNDO",4)) return 423; 
			if (!_fstrnicmp (str,"MISC",4)) return 424; 
			if (!_fstrnicmp (str,"HELP",4)) return 425; 
			if (!_fstrnicmp (str,"POLY",4)) return 426; 
			if (!_fstrnicmp (str,"REAL",4)) return 427; 
			if (!_fstrnicmp (str,"PING",4)) return 428; 
			if (!_fstrnicmp (str,"YEAR",4)) return 429; 
			if (!_fstrnicmp (str,"GRID",4)) return 430; 
			if (!_fstrnicmp (str,"DUMP", 4)) return 431;
			if (!_fstrnicmp(str, "RAND", 4)) return 432;
			if (!_fstrnicmp(str, "AREA", 4)) return 433;
			if (!_fstrnicmp(str, "TRAN", 4)) return 434;
			if (!_fstrnicmp(str, "WIFI", 4)) return 435;
			if (!_fstrnicmp(str, "FILE", 4)) return 436;
			if (!_fstrnicmp(str, "GDAL", 4)) return 437;
			if (!_fstrnicmp(str, "POST", 4)) return 438;
			return 0;
}
int	GetFunctionID5 (LPSTR str)
{
			if (!_fstrnicmp (str,"THEME",5)) return 501; 
			if (!_fstrnicmp (str,"WHILE",5)) return 502; 
			if (!_fstrnicmp (str,"FETCH",5)) return 503; 
			if (!_fstrnicmp (str,"TABLE",5)) return 504; 
			if (!_fstrnicmp (str,"TRNTO",5)) return 505; 
			if (!_fstrnicmp (str,"CVTTO",5)) return 506; 
			if (!_fstrnicmp (str,"CLOSE",5)) return 507; 
			if (!_fstrnicmp (str,"SLEEP",5)) return 508; 
			if (!_fstrnicmp (str,"RESET",5)) return 509; 
			if (!_fstrnicmp (str,"MACRO",5)) return 510; 
			if (!_fstrnicmp (str,"IMAGE",5)) return 511; 
			if (!_fstrnicmp (str,"AFTER",5)) return 512; 
			if (!_fstrnicmp (str,"PRINT",5)) return 513; 
			if (!_fstrnicmp (str,"ALERT",5)) return 514; 
			if (!_fstrnicmp (str,"GFKEY",5)) return 515; 
			if (!_fstrnicmp (str,"ONERR",5)) return 516; 
			if (!_fstrnicmp (str,"ABORT",5)) return 517; 
			if (!_fstrnicmp (str,"GFCMD",5)) return 518; 
			if (!_fstrnicmp (str,"UNHLT",5)) return 519; 
			if (!_fstrnicmp (str,"BTREE",5)) return 520; 
			if (!_fstrnicmp (str,"SUBDL",5)) return 521; 
			if (!_fstrnicmp (str,"STRIP",5)) return 522; 
			if (!_fstrnicmp (str,"ISHLT",5)) return 523; 
			if (!_fstrnicmp (str,"BASIC",5)) return 524;   
			if (!_fstrnicmp (str,"VOTER",5)) return 525; 
			if (!_fstrnicmp (str,"MUNIC",5)) return 526; 
			if (!_fstrnicmp (str,"LOGIN",5)) return 527; 
			if (!_fstrnicmp (str,"TIMER",5)) return 528; 
			if (!_fstrnicmp (str,"FENCE",5)) return 529; 
			if (!_fstrnicmp (str,"CLEAR",5)) return 530; 
			if (!_fstrnicmp (str,"REFNO",5)) return 531; 
			if (!_fstrnicmp (str,"POINT",5)) return 532; 
			if (!_fstrnicmp (str,"CRASH",5)) return 533; 
			if (!_fstrnicmp (str,"DEBUG",5)) return 534; 
			if (!_fstrnicmp (str,"CACHE",5)) return 535; 
			if (!_fstrnicmp (str,"ABEND",5)) return 536; 
			if (!_fstrnicmp (str,"UNLIT",5)) return 537; 
			if (!_fstrnicmp(str, "PLIST", 5)) return 538;

			return 0;		
}
int	GetFunctionID6 (LPSTR str)
{
			if (!_fstrnicmp (str,"TAGLOC",6)) return 601; 
			if (!_fstrnicmp (str,"LOADPM",6)) return 602; 
			if (!_fstrnicmp (str,"DIALOG",6)) return 603; 
			if (!_fstrnicmp (str,"REPORT",6)) return 604; 
			if (!_fstrnicmp (str,"APPEND",6)) return 605; 
			if (!_fstrnicmp (str,"VERIFY",6)) return 606; 
			if (!_fstrnicmp (str,"EXPORT",6)) return 607; 
			if (!_fstrnicmp (str,"IMPORT",6)) return 608; 
			if (!_fstrnicmp (str,"BROWSE",6)) return 609; 
			if (!_fstrnicmp (str,"INSERT",6)) return 610; 
			if (!_fstrnicmp (str,"BEFORE",6)) return 611; 
			if (!_fstrnicmp (str,"OFFSET",6)) return 612; 
			if (!_fstrnicmp (str,"RETURN",6)) return 613; 
			if (!_fstrnicmp (str,"SYMNUM",6)) return 614; 
			if (!_fstrnicmp (str,"BOUNDS",6)) return 615; 
			if (!_fstrnicmp (str,"DECODE",6)) return 616; 
			if (!_fstrnicmp (str,"TEXTAZ",6)) return 617; 
			if (!_fstrnicmp (str,"BACKUP",6)) return 618; 
			if (!_fstrnicmp (str,"EXPAND",6)) return 619; 
			if (!_fstrnicmp (str,"PARENT",6)) return 620; 
			if (!_fstrnicmp (str,"SIGNOF",6)) return 621; 
			if (!_fstrnicmp (str,"MEMMAP",6)) return 622; 
			if (!_fstrnicmp (str,"BMPDIM",6)) return 623; 
			if (!_fstrnicmp (str,"MAPSET",6)) return 624; 
			if (!_fstrnicmp (str,"SUBSET",6)) return 625; 
			if (!_fstrnicmp (str,"LENGTH",6)) return 626; 
			if (!_fstrnicmp (str,"RUNSQL",6)) return 627; 
			if (!_fstrnicmp (str,"SETELV",6)) return 628; 
			if (!_fstrnicmp (str,"MARKER",6)) return 629; 
			if (!_fstrnicmp (str,"RENAME",6)) return 630; 
			if (!_fstrnicmp (str,"STATUS",6)) return 631; 
			if (!_fstrnicmp (str,"ZOOMVP",6)) return 632; 
			if (!_fstrnicmp (str,"IDPOLY",6)) return 633; 
			if (!_fstrnicmp (str,"TOGGLE",6)) return 634; 
			if (!_fstrnicmp (str,"FIELDS",6)) return 635; 
			if (!_fstrnicmp (str,"CURSOR",6)) return 636; 
			if (!_fstrnicmp (str,"CANZIP",6)) return 637; 
			if (!_fstrnicmp (str,"WINDOW",6)) return 638; 
			if (!_fstrnicmp (str,"SCREEN",6)) return 639; 
			if (!_fstrnicmp (str,"THREAD",6)) return 640; 
			if (!_fstrnicmp (str,"GETVAL",6)) return 641; 
			if (!_fstrnicmp (str,"BUTTON",6)) return 642; 
			if (!_fstrnicmp (str,"BITMAP",6)) return 643; 
			if (!_fstrnicmp(str, "FORALL",6)) return 644;
			if (!_fstrnicmp(str, "GOOGLE",6)) return 645;
			if (!_fstrnicmp(str, "GMEDIT",6)) return 646;
			if (!_fstrnicmp(str, "FIXMAP",6)) return 647;
			if (!_fstrnicmp(str, "SQLITE", 6)) return 648;
			if (!_fstrnicmp(str, "INLIST", 6)) return 649;
			if (!_fstrnicmp(str, "LOWORD", 6)) return 650;
			if (!_fstrnicmp(str, "HIWORD", 6)) return 651;
			if (!_fstrnicmp(str, "NVCRIS", 6)) return 652;
			if (!_fstrnicmp(str, "LASZIP", 6)) return 653;
			if (!_fstrnicmp(str, "UNIQUE", 6)) return 654;
			if (!_fstrnicmp(str, "PROMPT", 6)) return 655;
			return 0;
}
int	GetFunctionID7 (LPSTR str)
{
			if (!_fstrnicmp (str,"LOADVIS",7)) return 701; 
			if (!_fstrnicmp (str,"LOADPIK",7)) return 702; 
			if (!_fstrnicmp (str,"MAKEMAP",7)) return 703; 
			if (!_fstrnicmp (str,"WHEREAT",7)) return 704; 
			if (!_fstrnicmp (str,"POINTER",7)) return 705; 
			if (!_fstrnicmp (str,"HLTAREA",7)) return 706; 
			if (!_fstrnicmp (str,"LOADRDF",7)) return 707; 
			if (!_fstrnicmp (str,"GETCVAL",7)) return 708; 
			if (!_fstrnicmp (str,"GETFVAL",7)) return 709; 
			if (!_fstrnicmp (str,"LINESYM",7)) return 710; 
			if (!_fstrnicmp (str,"LOADCFG",7)) return 711; 
			if (!_fstrnicmp (str,"TRNFROM",7)) return 720; 
			if (!_fstrnicmp (str,"CVTFROM",7)) return 721; 
			if (!_fstrnicmp (str,"AREASYM",7)) return 722; 
			if (!_fstrnicmp (str,"GETPATH",7)) return 723; 
			if (!_fstrnicmp (str,"NULLMAP",7)) return 724; 
			if (!_fstrnicmp (str,"MAKEDIR",7)) return 725; 
			if (!_fstrnicmp (str,"ENLARGE",7)) return 726; 
			if (!_fstrnicmp (str,"INFOBOX",7)) return 727; 
			if (!_fstrnicmp (str,"GETIVAL",7)) return 728; 
			if (!_fstrnicmp (str,"OPENAPP",7)) return 729; 
			if (!_fstrnicmp (str,"SYMNAME",7)) return 730; 
			if (!_fstrnicmp (str,"SYMDESC",7)) return 731; 
			if (!_fstrnicmp (str,"AUTOINC",7)) return 732; 
			if (!_fstrnicmp (str,"GLOBALS",7)) return 733; 
			if (!_fstrnicmp (str,"FILEFIT",7)) return 734; 
			if (!_fstrnicmp (str,"EPMACRO",7)) return 735; 
			if (!_fstrnicmp (str,"MESSAGE",7)) return 736; 
			if (!_fstrnicmp (str,"SAVEPIK",7)) return 737; 
			if (!_fstrnicmp (str,"SAVEVIS",7)) return 738; 
			if (!_fstrnicmp (str,"ATPRINT",7)) return 739; 
			if (!_fstrnicmp (str,"TCPOPEN",7)) return 740; 
			if (!_fstrnicmp (str,"TCPSEND",7)) return 741; 
			if (!_fstrnicmp (str,"GMDFIND",7)) return 742; 
			if (!_fstrnicmp (str,"UMREFNO",7)) return 743; 
			if (!_fstrnicmp (str,"CONVERT",7)) return 744; 
			if (!_fstrnicmp (str,"ADJTIME",7)) return 745; 
			if (!_fstrnicmp (str,"COPYMAP",7)) return 746; 
			if (!_fstrnicmp (str,"SYMCOPY",7)) return 747; 
			if (!_fstrnicmp (str,"EXECUTE",7)) return 748; 
			if (!_fstrnicmp (str,"CDUNITS",7)) return 749; 
			if (!_fstrnicmp (str,"LOADTIN",7)) return 750; 
			if (!_fstrnicmp (str,"SYMTYPE",7)) return 751; 
			if (!_fstrnicmp (str,"REPLACE",7)) return 752; 
			if (!_fstrnicmp (str,"SYMPLOT",7)) return 753; 
			if (!_fstrnicmp (str,"LOADDTM",7)) return 754; 
			if (!_fstrnicmp (str,"SYMDICT",7)) return 755; 
			if (!_fstrnicmp (str,"ARCDIST",7)) return 756; 
			if (!_fstrnicmp (str,"LOADBMP",7)) return 757; 
			if (!_fstrnicmp (str,"ADDAREA",7)) return 758; 
			if (!_fstrnicmp (str,"DOWNAME",7)) return 759; 
			if (!_fstrnicmp (str,"ADDLINE",7)) return 760; 
			if (!_fstrnicmp (str,"NUMROWS",7)) return 761; 
			if (!_fstrnicmp (str,"PICKCPT",7)) return 762; 
			if (!_fstrnicmp (str,"DISPLAY",7)) return 763; 
			if (!_fstrnicmp (str,"GMDCOPY",7)) return 764; 
			if (!_fstrnicmp (str,"VEHICLE",7)) return 765; 
			if (!_fstrnicmp (str,"ADDITEM",7)) return 766; 
			if (!_fstrnicmp (str,"TCPPARM",7)) return 767; 
			if (!_fstrnicmp (str,"DIRPATH",7)) return 768; 
			if (!_fstrnicmp (str,"NUMERIC",7)) return 769; 
			if (!_fstrnicmp (str,"FILEPOS",7)) return 770; 
			if (!_fstrnicmp (str,"FILELEN",7)) return 771; 
			if (!_fstrnicmp (str,"TCPFILE",7)) return 772; 
			if (!_fstrnicmp (str,"TOOLBAR",7)) return 773; 
			if (!_fstrnicmp (str,"NETWORK",7)) return 774; 
			if (!_fstrnicmp (str,"LOADMAP",7)) return 775; 
			if (!_fstrnicmp (str,"AZMDIFF",7)) return 776; 
			if (!_fstrnicmp (str,"PROCESS",7)) return 777; 
			if (!_fstrnicmp (str,"SESSION",7)) return 778; 
			if (!_fstrnicmp (str,"COMPOSE",7)) return 779; 
			if (!_fstrnicmp (str,"BATTERY",7)) return 780; 
			if (!_fstrnicmp(str, "GEOCODE", 7)) return 781;
			if (!_fstrnicmp(str, "ADDRESS", 7)) return 782;
			if (!_fstrnicmp(str, "TESTENV", 7)) return 783;
			if (!_fstrnicmp(str, "COPYDIR", 7)) return 784;
			if (!_fstrnicmp(str, "NVMETRO", 7)) return 785;
			if (!_fstrnicmp(str, "TAGDUMP", 7)) return 786;
			if (!_fstrnicmp(str, "DIMLINE", 7)) return 787;

			return 0;
}
int	GetFunctionID8 (LPSTR str)
{
			if (!_fstrnicmp (str,"POINTSYM",8)) return 801; 
			if (!_fstrnicmp (str,"COPYFILE",8)) return 802; 
			if (!_fstrnicmp (str,"EDITFILE",8)) return 803; 
			if (!_fstrnicmp (str,"CHANGEPT",8)) return 804; 
			if (!_fstrnicmp (str,"GOFILEPT",8)) return 805; 
			if (!_fstrnicmp (str,"COORDLOC",8)) return 806; 
			if (!_fstrnicmp (str,"IMAGELIM",8)) return 807; 
			if (!_fstrnicmp (str,"WMASETUP",8)) return 808; 
			if (!_fstrnicmp (str,"LOADTRAN",8)) return 809; 
			if (!_fstrnicmp (str,"GETCOLOR",8)) return 810; 
			if (!_fstrnicmp (str,"ADDPOINT",8)) return 811; 
			if (!_fstrnicmp (str,"LOADEDGE",8)) return 812; 
			if (!_fstrnicmp (str,"AREAGRID",8)) return 813; 
			if (!_fstrnicmp (str,"TCPCLOSE",8)) return 814; 
			if (!_fstrnicmp (str,"PARSECDT",8)) return 815; 
			if (!_fstrnicmp (str,"NUMLINES",8)) return 816; 
			if (!_fstrnicmp (str,"SCALERNG",8)) return 817; 
			if (!_fstrnicmp (str,"ZOOMLIST",8)) return 818; 
			if (!_fstrnicmp (str,"MIDPOINT",8)) return 819; 
			if (!_fstrnicmp (str,"NEWPOINT",8)) return 820; 
			if (!_fstrnicmp (str,"PATHTYPE",8)) return 821; 
			if (!_fstrnicmp (str,"GMDMERGE",8)) return 822; 
			if (!_fstrnicmp (str,"TRUNCATE",8)) return 823; 
			if (!_fstrnicmp (str,"FILETYPE",8)) return 824; 
			if (!_fstrnicmp (str,"REGISTER",8)) return 825; 
			if (!_fstrnicmp (str,"CONTINUE",8)) return 826; 
			if (!_fstrnicmp (str,"READLINE",8)) return 827; 
			if (!_fstrnicmp (str,"TABLEDEF",8)) return 828; 
			if (!_fstrnicmp (str,"OWNERLOC",8)) return 829; 
			if (!_fstrnicmp (str,"REORGMAP",8)) return 830; 
			if (!_fstrnicmp (str,"GETFOCUS",8)) return 831; 
			if (!_fstrnicmp (str,"SETFOCUS",8)) return 832; 
			if (!_fstrnicmp (str,"TRAVERSE",8)) return 833; 
			if (!_fstrnicmp (str,"CHECKTIF",8)) return 834; 
			if (!_fstrnicmp (str,"GMDREORG",8)) return 835; 
			if (!_fstrnicmp (str,"MAPINDEX",8)) return 836; 
			if (!_fstrnicmp (str,"TIMESPAN",8)) return 837; 
			if (!_fstrnicmp (str,"COPYTEXT",8)) return 838; 
			if (!_fstrnicmp (str,"FILECOPY",8)) return 839; 
			if (!_fstrnicmp (str,"COLORMAP",8)) return 840; 
			if (!_fstrnicmp (str,"MAPIMAGE",8)) return 841; 
			if (!_fstrnicmp (str,"XMLTOGMD",8)) return 842; 
			if (!_fstrnicmp (str,"PNETGRID",8)) return 843; 
			if (!_fstrnicmp (str,"CPTTOBPW",8)) return 844; 
			if (!_fstrnicmp (str,"FILELIST",8)) return 845; 
			if (!_fstrnicmp (str,"CHECKSUM",8)) return 846; 
			if (!_fstrnicmp (str,"MAXSLOPE",8)) return 847; 
			if (!_fstrnicmp (str,"FILETIME",8)) return 848; 
			if (!_fstrnicmp(str, "FUNCTION",8)) return 849;
			if (!_fstrnicmp(str, "FILEPART",8)) return 850;
			if (!_fstrnicmp(str, "MOVEFILE", 8)) return 851;
			if (!_fstrnicmp(str, "HEADTOAZ", 8)) return 852;
			if (!_fstrnicmp(str, "DATABASE", 8)) return 853;
			if (!_fstrnicmp(str, "GMMOBILE", 8)) return 854;
			if (!_fstrnicmp(str, "MAKELONG", 8)) return 855;
			if (!_fstrnicmp(str, "TEXTFILE", 8)) return 856;
			return 0;
}
int	GetFunctionID9 (LPSTR str)
{
			if (!_fstrnicmp (str,"ADDSEARCH",9)) return 901; 
			if (!_fstrnicmp (str,"REDISPLAY",9)) return 902; 
			if (!_fstrnicmp (str,"GMDUPDATE",9)) return 903; 
			if (!_fstrnicmp (str,"GMDDELETE",9)) return 906; 
			if (!_fstrnicmp (str,"GETMLCVAL",9)) return 904; 
			if (!_fstrnicmp (str,"DELETEREF",9)) return 905; 
			if (!_fstrnicmp (str,"MATCHTRAN",9)) return 907; 
			if (!_fstrnicmp (str,"CREATESYM",9)) return 908; 
			if (!_fstrnicmp (str,"GETINIVAL",9)) return 909; 
			if (!_fstrnicmp (str,"SETINIVAL",9)) return 910; 
			if (!_fstrnicmp (str,"POLYPOINT",9)) return 911; 
			if (!_fstrnicmp (str,"VEHUPDATE",9)) return 912; 
			if (!_fstrnicmp (str,"HLTOUTPUT",9)) return 913; 
			if (!_fstrnicmp (str,"INSTALLCD",9)) return 914; 
			if (!_fstrnicmp (str,"ORTHOSIZE",9)) return 915; 
			if (!_fstrnicmp (str,"GMDCREATE",9)) return 916; 
			if (!_fstrnicmp (str,"FINDFILES",9)) return 917; 
			if (!_fstrnicmp (str,"AZMTOBEAR",9)) return 918; 
			if (!_fstrnicmp (str,"SYMDELETE",9)) return 919; 
			if (!_fstrnicmp (str,"GMDIMPORT",9)) return 920; 
			if (!_fstrnicmp (str,"LAYERNAME",9)) return 921; 
			if (!_fstrnicmp (str,"ADDUNIQUE",9)) return 922; 
			if (!_fstrnicmp (str,"TRANIMAGE",9)) return 923; 
			if (!_fstrnicmp (str,"SYMPARENT",9)) return 924; 
			if (!_fstrnicmp (str,"SEWERDATA",9)) return 925; 
			if (!_fstrnicmp (str,"CURSORLOC",9)) return 926; 
			if (!_fstrnicmp (str,"CHECKTRAN",9)) return 927; 
			if (!_fstrnicmp (str,"CHANGETAG",9)) return 928; 
			if (!_fstrnicmp (str,"LAYERPATH",9)) return 929; 
			if (!_fstrnicmp (str,"FINDFIELD",9)) return 930; 
			if (!_fstrnicmp (str,"GETPICKED",9)) return 931; 
			if (!_fstrnicmp (str,"SENDEMAIL",9)) return 932; 
			if (!_fstrnicmp (str,"URLTOFILE",9)) return 933; 
			if (!_fstrnicmp (str,"STREETNUM",9)) return 934; 
			if (!_fstrnicmp (str,"FIELDDEFS",9)) return 935; 
			if (!_fstrnicmp (str,"NEWLATLON",9)) return 936; 
			if (!_fstrnicmp (str,"INTERSECT",9)) return 937; 
			if (!_fstrnicmp (str,"MOVEPOINT",9)) return 938; 
			if (!_fstrnicmp (str,"COMBOFILE",9)) return 939; 
			if (!_fstrnicmp (str,"TCPRETURN",9)) return 940; 
			if (!_fstrnicmp (str,"NEARPOINT",9)) return 941; 
			if (!_fstrnicmp (str,"POINTLIST",9)) return 942; 
			if (!_fstrnicmp (str,"POINTINVP",9)) return 943; 
			if (!_fstrnicmp (str,"BLOCKTEXT",9)) return 944; 
			if (!_fstrnicmp (str,"LINKLINES",9)) return 945; 
			if (!_fstrnicmp(str, "CLEARFILE",9)) return 946;
			if (!_fstrnicmp(str, "CLIPBOARD", 9)) return 947;
			if (!_fstrnicmp(str, "MAPSERVER", 9)) return 948;
			return 0;
}
int	GetFunctionID10 (LPSTR str)
{
			if (!_fstrnicmp (str,"DECOMPPOLY",10)) return 1001; 
			if (!_fstrnicmp (str,"DELETEFILE",10)) return 1002; 
			if (!_fstrnicmp (str,"STREETNAME",10)) return 1003; 
			if (!_fstrnicmp (str,"BOUNDPOINT",10)) return 1004; 
			if (!_fstrnicmp (str,"PRINTSETUP",10)) return 1005; 
			if (!_fstrnicmp (str,"SAVESTATUS",10)) return 1006; 
			if (!_fstrnicmp (str,"UPDATETIME",10)) return 1007; 
			if (!_fstrnicmp (str,"REGISTERCD",10)) return 1008; 
			if (!_fstrnicmp (str,"PRINTMERGE",10)) return 1009; 
			if (!_fstrnicmp (str,"VEHICLEDEF",10)) return 1010; 
			if (!_fstrnicmp (str,"SETSYMDESC",10)) return 1011; 
			if (!_fstrnicmp (str,"INSERTLINE",10)) return 1012; 
			if (!_fstrnicmp (str,"CROSSMATCH",10)) return 1013; 
			if (!_fstrnicmp (str,"AREAINAREA",10)) return 1014; 
			if (!_fstrnicmp (str,"CREATEFILE",10)) return 1015; 
			if (!_fstrnicmp (str,"WINTOWORLD",10)) return 1016; 
			if (!_fstrnicmp (str,"WORLDTOWIN",10)) return 1017; 
			if (!_fstrnicmp (str,"ADJUSTPOLY",10)) return 1018; 
			if (!_fstrnicmp (str,"TABTOCOMMA",10)) return 1019; 
			if (!_fstrnicmp (str,"SELECTICON",10)) return 1020; 
			if (!_fstrnicmp (str,"MOVECURSOR",10)) return 1021; 
			if (!_fstrnicmp (str,"CURSORINVP",10)) return 1022; 
			if (!_fstrnicmp (str,"IDPOLYGONS",10)) return 1023; 
			if (!_fstrnicmp (str,"FULLSCREEN",10)) return 1024; 
			if (!_fstrnicmp (str,"REFCONNECT",10)) return 1025; 
			if (!_fstrnicmp (str,"SCREENTOVP",10)) return 1026; 
			if (!_fstrnicmp (str,"AREACENTER",10)) return 1027; 
			if (!_fstrnicmp (str,"SERVERFILE",10)) return 1028; 
			if (!_fstrnicmp (str,"SPLITLINES",10)) return 1029; 
			if (!_fstrnicmp (str,"INVERTRECT",10)) return 1030; 
			if (!_fstrnicmp (str,"SETMAPTIME",10)) return 1031; 
			if (!_fstrnicmp (str,"GETMAPTIME",10)) return 1032; 
			if (!_fstrnicmp (str,"CHECKPOINT",10)) return 1033; 
			if (!_fstrnicmp (str,"PCTINAREAS",10)) return 1034; 
			if (!_fstrnicmp (str,"PROJECTION",10)) return 1035; 
			if (!_fstrnicmp(str, "BACKGROUND", 10)) return 1036;
			if (!_fstrnicmp(str, "WAITFORKEY", 10)) return 1037;
			if (!_fstrnicmp(str, "DIALOGITEM", 10)) return 1038;
			if (!_fstrnicmp(str, "SYMATTRKEY", 10)) return 1039;
			if (!_fstrnicmp(str, "PARCELTRAN", 10)) return 1040;
			if (!_fstrnicmp(str, "MAILLABELS", 10)) return 1041;
			if (!_fstrnicmp(str, "SAVESCREEN", 10)) return 1042;
			if (!_fstrnicmp(str, "GMDOCUMENT", 10)) return 1043;
			return 0;
}
int	GetFunctionID11 (LPSTR str)
{
			if (!_fstrnicmp (str,"DUMPGLOBALS",11)) return 1101; 
			if (!_fstrnicmp (str,"LAYERBOUNDS",11)) return 1102; 
			if (!_fstrnicmp (str,"FIXGMERRORS",11)) return 1103; 
			if (!_fstrnicmp (str,"ADDWAYPOINT",11)) return 1104; 
			if (!_fstrnicmp (str,"COMPILETRAN",11)) return 1105; 
			if (!_fstrnicmp (str,"GPSADDPOINT",11)) return 1106; 
			if (!_fstrnicmp (str,"UPDATEGRCMD",11)) return 1107; 
			if (!_fstrnicmp (str,"RUNTEXTFILE",11)) return 1108; 
			if (!_fstrnicmp (str,"GETTRANDATA",11)) return 1109; 
			if (!_fstrnicmp (str,"GETNEXTLINE",11)) return 1110; 
			if (!_fstrnicmp (str,"SELECTITEMS",11)) return 1111; 
			if (!_fstrnicmp (str,"GPSTRACKING",11)) return 1112; 
			if (!_fstrnicmp (str,"FONTDISPLAY",11)) return 1113; 
			if (!_fstrnicmp (str,"WETLANDDESC",11)) return 1114; 
			if (!_fstrnicmp (str,"GPSADDROUTE",11)) return 1115; 
			if (!_fstrnicmp (str,"BLOCKEDREFS",11)) return 1116; 
			if (!_fstrnicmp (str,"LOADNEWDATA",11)) return 1117; 
			if (!_fstrnicmp (str,"THEMEINAREA",11)) return 1118; 
			if (!_fstrnicmp (str,"SETREDEFINE",11)) return 1119; 
			if (!_fstrnicmp (str,"GROUPPOINTS",11)) return 1120; 
			if (!_fstrnicmp (str,"GETTEMPFILE",11)) return 1121; 
			if (!_fstrnicmp (str,"GMDADDINDEX",11)) return 1122; 
			if (!_fstrnicmp (str,"ADDFALSEINT",11)) return 1123; 
			if (!_fstrnicmp (str,"DUMPDGNSYMS",11)) return 1124; 
			if (!_fstrnicmp (str,"SCREENCACHE",11)) return 1125; 
			if (!_fstrnicmp (str,"VIRTUALPLOT",11)) return 1126; 
			if (!_fstrnicmp (str,"GMDADDFIELD",11)) return 1127; 
			if (!_fstrnicmp (str,"GMDCOPYFILE",11)) return 1128; 
			if (!_fstrnicmp (str,"TAGREFINDEX",11)) return 1129; 
			if (!_fstrnicmp (str,"INSERTLINES",11)) return 1130; 
			if (!_fstrnicmp (str,"ROTATEIMAGE",11)) return 1131; 
			if (!_fstrnicmp (str,"SYMATTRFILE",11)) return 1132; 
			if (!_fstrnicmp (str,"POINTINAREA",11)) return 1133; 
			if (!_fstrnicmp (str,"SCREENCOLOR",11)) return 1134; 
			if (!_fstrnicmp (str,"INTERTWINED",11)) return 1135; 
			if (!_fstrnicmp (str,"INTERLEAVED",11)) return 1135; 
			if (!_fstrnicmp(str, "STRINGTONUM", 11)) return 1136;
			if (!_fstrnicmp(str, "DATADISPLAY", 11)) return 1137;
			if (!_fstrnicmp(str, "ISLOCALFILE", 11)) return 1138;
			if (!_fstrnicmp(str, "FILEMANAGER", 11)) return 1139;

			return 0;
			
}
int	GetFunctionID12 (LPSTR str)
{
			if (!_fstrnicmp (str,"FINDWAYPOINT",12)) return 1201; 
			if (!_fstrnicmp (str,"SETDATERANGE",12)) return 1202; 
			if (!_fstrnicmp (str,"LOADUMSYMDEF",12)) return 1203; 
			if (!_fstrnicmp (str,"MAKEQUANFILE",12)) return 1204; 
			if (!_fstrnicmp (str,"USEDREFTABLE",12)) return 1205; 
			if (!_fstrnicmp (str,"ADDRECTANGLE",12)) return 1206; 
			if (!_fstrnicmp (str,"UPDATEGLOBAL",12)) return 1207; 
			if (!_fstrnicmp (str,"CHANGESYMBOL",12)) return 1208;
			if (!_fstrnicmp (str,"ORATABLENAME",12)) return 1209;
			if (!_fstrnicmp (str,"LONGPATHNAME",12)) return 1210;
			if (!_fstrnicmp (str,"HOTSPOTVALUE",12)) return 1211;
			if (!_fstrnicmp (str,"TRANSFERFILE",12)) return 1212;
			if (!_fstrnicmp (str,"NAMESPLITTER",12)) return 1213;
			if (!_fstrnicmp (str,"CONVERTIMAGE",12)) return 1214; 
			if (!_fstrnicmp (str,"POLYPROBLEMS",12)) return 1215; 
			if (!_fstrnicmp (str,"AREASTOLINES",12)) return 1216;   
			if (!_fstrnicmp (str,"PCTINHLTAREA",12)) return 1217;   
			if (!_fstrnicmp (str,"COMPAREFILES",12)) return 1218; 
			if (!_fstrnicmp (str,"SETSYMPARENT",12)) return 1219; 
			if (!_fstrnicmp (str,"SAVECONTOURS",12)) return 1220; 
			if (!_fstrnicmp (str,"THINCONTOURS",12)) return 1221; 
			if (!_fstrnicmp (str,"TRANSPARENCY",12)) return 1222; 
			if (!_fstrnicmp (str,"SQLFIELDTYPE",12)) return 1223; 
			if (!_fstrnicmp (str,"GMDFIELDTYPE",12)) return 1224; 
			if (!_fstrnicmp (str,"INSERTFORMAT",12)) return 1225; 
			
			return 0; 
			
}
int	GetFunctionID13 (LPSTR str)
{
			if (!_fstrnicmp (str,"DISPLAYCONFIG",13)) return 1301; 
			if (!_fstrnicmp (str,"NUMNONCONTROL",13)) return 1302; 
			if (!_fstrnicmp (str,"SPORTMAPORDER",13)) return 1303; 
			if (!_fstrnicmp (str,"ZOOMLISTBUILD",13)) return 1304; 
			if (!_fstrnicmp (str,"SPLITXFERFILE",13)) return 1305; 
			if (!_fstrnicmp (str,"SELECTDBITEMS",13)) return 1306;   
			if (!_fstrnicmp (str,"CREATESF3FILE",13)) return 1307;   
			if (!_fstrnicmp (str,"CREATESF1FILE",13)) return 1308;   
			if (!_fstrnicmp (str,"SHORTPATHNAME",13)) return 1309;   
			if (!_fstrnicmp (str,"PROCESSSTATUS",13)) return 1310;   
			if (!_fstrnicmp (str,"LINESTOPOINTS",13)) return 1311; 
			if (!_fstrnicmp(str, "PARENTSYMBOLS", 13)) return 1312;
			if (!_fstrnicmp(str, "ACCELEROMETER", 13)) return 1313;

			return 0; 
			
}
int	GetFunctionID14 (LPSTR str)
{
			if (!_fstrnicmp (str,"SETTEXTGLOBALS",14)) return 1401; 
			if (!_fstrnicmp (str,"LOADSSURGOCOMP",14)) return 1402; 
			if (!_fstrnicmp (str,"SETDESTINATION",14)) return 1403; 
			if (!_fstrnicmp (str,"SELECTFONTNAME",14)) return 1404; 
			if (!_fstrnicmp (str,"BOUNDSTOPOINTS",14)) return 1405; 
			if (!_fstrnicmp (str,"SPLITMRSIDFILE",14)) return 1406; 
			if (!_fstrnicmp (str,"FINDDUPLICATES",14)) return 1407; 
			if (!_fstrnicmp (str,"FILTERTEXTFILE",14)) return 1408; 
			if (!_fstrnicmp (str,"RECOVERPLTFILE",14)) return 1409; 
			if (!_fstrnicmp (str,"STRINGFROMFILE",14)) return 1410;
			if (!_fstrnicmp (str,"GLOBALFROMFILE",14)) return 1411;
			if (!_fstrnicmp (str,"COMPRESSEDFILE",14)) return 1412;
			
			return 0;
}
int	GetFunctionID15 (LPSTR str)
{
			if (!_fstrnicmp (str,"OPENVEHICLEFILE",15)) return 1501; 
			if (!_fstrnicmp (str,"REMOVELINKLINES",15)) return 1502;  
			if (!_fstrnicmp (str,"GETADDRESSCOORD",15)) return 1503;  
			if (!_fstrnicmp (str,"TEXTTOCLIPBOARD",15)) return 1504;  
			if (!_fstrnicmp (str,"ADDRESSLOCATION",15)) return 1505;  
			if (!_fstrnicmp (str,"CREATEWORDINDEX",15)) return 1506;  
			if (!_fstrnicmp (str,"GETMAXFILEREFNO",15)) return 1507;  
			if (!_fstrnicmp(str, "DELETEDIRECTORY", 15)) return 1508;
			if (!_fstrnicmp(str, "NETWORKANALYZER", 15)) return 1509;
			if (!_fstrnicmp(str, "COPYWITHREPLACE", 15)) return 1510;

			return 0;
}
int	GetFunctionID16 (LPSTR str)
{
			if (!_fstrnicmp (str,"CREATESPORTMAPCD",16)) return 1601; 
			if (!_fstrnicmp (str,"ADDISLANDSTOPOLY",16)) return 1602; 
			if (!_fstrnicmp (str,"POINTSBETWEENMPS",16)) return 1603; 
			if (!_fstrnicmp (str,"POINTSBETWEENPCT",16)) return 1604; 
			if (!_fstrnicmp (str,"FILLRECTWITHGRID",16)) return 1605; 
			if (!_fstrnicmp (str,"APPENDFILETOFILE",16)) return 1606;
			return 0; 
}			

int	GetFunctionID17 (LPSTR str)
{
			if (!_fstrnicmp (str,"RECOVERPLTFROMRIN",17)) return 1701; 
			if (!_fstrnicmp (str,"SCREENTOCLIPBOARD",17)) return 1702; 
			if (!_fstrnicmp (str,"PROFILEAREAPOINTS",17)) return 1703; 
			if (!_fstrnicmp (str,"SPLITCONTOURLINES",17)) return 1704; 
			if (!_fstrnicmp (str,"TEXTFROMCLIPBOARD",17)) return 1705; 
			
			return 0; 
}			

int	GetFunctionID18 (LPSTR str)
{
			if (!_fstrnicmp (str,"LOADBLOCKINGPOINTS",18)) return 1801; 
			if (!_fstrnicmp (str,"NEWELEMENTSETTINGS",18)) return 1802; 
			if (!_fstrnicmp (str,"ADJOININGAREASFILE",18)) return 1803; 
			if (!_fstrnicmp(str, "GETNEARESTINTCOORD", 18)) return 1804;
			if (!_fstrnicmp(str, "GETPERPPOINTONPOLY", 18)) return 1805;

			return 0; 
}			

int	GetFunctionID19 (LPSTR str)
{
			if (!_fstrnicmp (str,"CONVERTTEXTPOINTERS",19)) return 1901;
			return 0; 
}
int	GetFunctionID20 (LPSTR str)
{
			if (!_fstrnicmp (str,"GETINTERSECTIONCOORD",20)) return 2001;
			if (!_fstrnicmp (str,"ADDCONTOURSPLITLINES",20)) return 2002;
			return 0; 
}
int	GetFunctionID22 (LPSTR str)
{
			if (!_fstrnicmp (str,"JOINLINESBETWEENPOINTS",22)) return 2201;          
			return 0; 
}			
int	GetFunctionID26 (LPSTR str)
{
			if (!_fstrnicmp (str,"GETSTREETSEGSBETWEENPOINTS",26)) return 2601;          
			return 0; 
}			

LPSTR EndOfFunction(LPSTR str)
{
	LPSTR rtn = 0;
	if (*str == '$')
	{
		LPSTR ParenLoc = strchr(str, '(');
		if (ParenLoc)
		{
			int id = GetFunctionID(str, ParenLoc);
			if (id > 0)
			{
				rtn = MatchLev(ParenLoc+1, ')');
			}
		}
	}

	return rtn;
}

int	GetFunctionID (LPSTR str, LPSTR ParenLoc)
#if ENABLETRACE
{GSSiEnterProg (609);
#endif
{	
	int		l; 
	
	if (TraceOn)
	{
		*ParenLoc = 0;
		_fstrcpy (LastFunctionName,str);
		*ParenLoc = '(';
	} 
	str++;
	l = ParenLoc - str; 
	switch (l)
	{
		case 1: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID1 (str);
}
		case 2: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID2 (str);
}
		case 3: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID3 (str);
}
		case 4: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID4 (str);
}
		case 5: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID5 (str);
}
		case 6: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID6 (str);
}
		case 7: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID7 (str);
}
		case 8: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID8 (str);
}
		case 9: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID9 (str);
}
		case 10: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID10 (str);
}
		case 11: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID11 (str);
}
		case 12: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID12 (str);
}
		case 13: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID13 (str);
}
		case 14: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID14 (str);
}
		case 15: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID15 (str);
}
		case 16: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID16 (str);
}
		case 17: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID17 (str);
}
		case 18: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID18 (str);
}
		case 19: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID19 (str);
}
		case 20: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID20 (str);
}
		case 22: 
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID22 (str);
}
		case 26:
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return GetFunctionID26 (str);
}
		default:
{
#if ENABLETRACE
GSSiExitProg (609);
#endif
			return 0;
}
	}
#if ENABLETRACE
}
#endif
}

