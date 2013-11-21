To print a report in MSACCESS for each item in the highlight list use a graphics
function command similar to the following:

Print Reports for Highlighted Items ^P|
|[%C]=$DDE(INITIATE,MSAccess,System,MSACCESS,C:\ACCESS\MSACCESS.EXE,MIN);
|[%C]=$DDE(EXECUTE,MSACCESS,@[OpenDatabase [%DL]access\avon.mdb]);
|[%C]=$OPEN(HLTLIST);WHILE($FETCH(HLTLIST)){
|[%C]=$DDE(EXECUTE,MSACCESS,@[Openreport report1,0,,GIS1 = '[STR]' AND GIS2 = '[PID]']);}
|[%C]=$CLOSE(HLTLIST);
|[%C]=$DDE(TERMINATE,MSACCESS)


To change old symbol names to new names create a file named SYMCONV.TXT in the [%DL]
directory. Put one line in this file for each symbol you wish to convert. Col 1-8 are
the old symbol name (uppercase) and col 10-42 are the new name (32 char max).

To load data in the ESRI GEN format use the Load GEN Data option in the Graphics Import Functions
utilities.

To load the wetland shape files use the following graphics function command:

Import Wetland SHP Files|[%IDM]=F;[%C]=$OPEN(FILELIST=i:\wetshape\filelist.txt,);WHILE($FETCH(FILELIST)){
|[%WT]=[SHPNAME];[%C]=$IMPORT(SHP,wetareas.tl2);}
|[%WT]=Done;[%C]=$CLOSE(FILELIST);[%IDM]=T

The file WETAREAS.TL2 is attached and should be placed in the working directory. Unzip the
files you wish to load into a directory and then call me and I will step you through it.
