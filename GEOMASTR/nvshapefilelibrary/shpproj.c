/******************************************************************************
 * Copyright (c) 1999, Carl Anderson
 *
 * This code is based in part on the earlier work of Frank Warmerdam
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * requires shapelib 1.2
 *   gcc shpproj ../shpopen.o ../dbfopen.o shpgeo.o -lm -lproj -o shpproj
 * 
 * this requires linking with the PROJ4.3 projection library available from
 *
 * ftp://kai.er.usgs.gov/ftp/PROJ.4
 *
 *
 * SHPGeo must be compiled with -DPROJ4 support
 *
 * $Log: shpproj.c,v $
 * Revision 1.10  2011-07-24 03:17:46  fwarmerdam
 * include string.h and stdlib.h where needed in contrib (#2146)
 *
 * Revision 1.9  2002-01-15 14:37:08  warmerda
 * upgrade to use proj_api.h
 *
 * Revision 1.8  2002/01/11 15:47:26  warmerda
 * several fixes
 *
 * Revision 1.7  2002/01/11 15:23:28  warmerda
 * use text mode reading and writing .prj files
 *
 * Revision 1.6  1999/05/26 02:56:31  candrsn
 * updates to shpdxf, dbfinfo, port from Shapelib 1.1.5 of dbfcat and shpinfo
 *
 * Revision 1.2  1999/05/13 19:30:52  warmerda
 * Removed libgen.h, added url for PROJ.4, and corrected unsafe return of
 * local variable in asFileName().
 *
 */


#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "shapefil.h"
#include "shpgeo.h"
#include "shr.h"
#include "gmextern.h"
#include "sqlite3.h"

LPSTR ShapeFileIndexName(LPSTR shapeFileName)
{
	static char indexFile[MAX_PATH];
	LPSTR indxName = indexFile;
	strcpy(indexFile, shapeFileName);
	LPSTR pDot = strrchr(indexFile, '.');
	if (pDot)
		strcpy(pDot, ".nvi");
	else
		*indxName = 0;
	return indxName;
}
BOOL CreateShapeFileIndexSLT(LPSTR shapeFileName)
{
	BOOL rtn = FALSE;
	int ii = 0;
	char cmd[1024];
	LPSTR indexName = ShapeFileIndexName(shapeFileName);
	sqlite3* database;
	SHPHandle	hSHP = SHPOpen(shapeFileName, "rb");
	SHPIndexType = SHP_INDEX_SLT;
	if (hSHP && GSSiLength(indexName) <= 0)
	{
		rtn = TRUE;
		BOOL isOpen = !SQLOK(sqlite3_open(indexName, &database), 0,"create shp index",0);
		FileAlreadyNotFound(indexName, 3, 0);
		SLT_StartTrans(database);

		strcpy (cmd,"CREATE VIRTUAL TABLE SHP_index USING rtree(id,minX, maxX, minY, maxY);CREATE TABLE SHP (RECNUM INTEGER PRIMARY KEY,symnum INT,offset INT);");
		SLT_Execute(cmd, database);
		CreateStatusWind(0, 1, "Create Shapefile Index");
			
		for (int irec = 0; irec < hSHP->nRecords; irec++)
		{
			if (!StatusWindowUpdate(0, "", hSHP->nRecords, irec))
			{
				rtn = FALSE;
				break;
			}

			int Offset = hSHP->panRecOffset[irec];
			MNMXCORD Bounds;
			SetSHPParms(irec);
			int SymNum = CurrentDesc;

			SHPObject *psCShape = SHPReadObject(hSHP, irec);
			if (psCShape)
			{
				if (psCShape->nVertices > 0)
				{
					MNMXCORD SHPBounds;
					SHPBounds.xmn = psCShape->dfXMin;
					SHPBounds.xmx = psCShape->dfXMax;
					SHPBounds.ymn = psCShape->dfYMin;
					SHPBounds.ymx = psCShape->dfYMax;
					sprintf(cmd, "INSERT INTO SHP_index VALUES(%i,%f,%f,%f,%f);INSERT INTO SHP VALUES(%i, %i, %i);", irec, SHPBounds.xmn, SHPBounds.xmx, SHPBounds.ymn, SHPBounds.ymx, irec, SymNum, Offset);
					SLT_Execute(cmd, database);
					SHPDestroyObject(psCShape);
				}
				else
					ii++;
			}
			else
				ii++;
		}

		if (rtn)
			SLT_EndTrans(database);
		SQLOK(sqlite3_close(database), 0, "create shp index", 0);
		DestroyStatusWindow(0);
	}

	if (hSHP)
		SHPClose(hSHP);
	return rtn;
}

int TransformSHP( int argc, char ** argv ,double * pOutFactor)
{
    SHPHandle	old_SHP, new_SHP;
    DBFHandle   old_DBF, new_DBF;
    int		nShapeType, nEntities, nVertices, nParts, *panParts, i, iPart, j;
    double	*padVertices, adBounds[4];
    const char 	*pszPlus;
    DBFFieldType  idfld_type;
    SHPObject	*psCShape;
    FILE	*ifp = NULL;
    int		idfld, nflds;
    char	kv[257] = "";
    char	idfldName[120] = "";
    char	fldName[120] = "";
    char	shpFileName[120] = "";
    char	dbfFileName[120] = "";
    char	prjFileName[120] = "";
    char	parg[80];
    double	apeture[4];
    int		inarg=3, outarg=4;
    char	*DBFRow = NULL;
	double	outFactor = 1.0;

	if (pOutFactor)
		outFactor = *pOutFactor;
/* for testing only 
    char	*in_args[] = { "init=nad83:1002", "units=us-ft" };
    char	*out_args[] = { "proj=utm", "zone=16", "units=m" };
*/

    char	*in_args[16];
    char	*out_args[16];
    int		in_argc = 0 , out_argc = 0, outf_arg;
    char	*arglst;
    projPJ	orig_prj, new_prj;
    va_list	myargs, moargs;
    char    *prjOrig =  argv[inarg] ;
    char    *pPrj = 0;

    if( argc < 4)
    {
	printf( "shpproj shp_file new_shp ( -i=in_proj_file | -i=\"in_params\" | -i=geographic ) ( -o=out_info_file | -o=\"out_params\" | -o=geographic ) \n" );
        return 0;
    }

    old_SHP = SHPOpen( argv[1], "rb" );
    if( old_SHP == NULL)
    {
        printf( "Unable to open old files:%s\n", argv[1] );
        return 0;
    }
    old_DBF = DBFOpen( argv[1], "rb" );
    if( old_DBF == NULL)
    {
        printf( "Unable to open old dbf files:%s\n", argv[1] );
        SHPClose(old_SHP);
        return 0;
    }
    outf_arg = 2;

// if shapefile has a nvp component then use that
    strcpy( prjFileName, argv[1] );
    ifp = fopen( asFileName ( prjFileName, "nvp" ),"rt");
    if ( ifp )
    {
        fseek (ifp,0,SEEK_END);
        long l = ftell (ifp);
        pPrj = malloc (l+4);
        fseek (ifp,0,SEEK_SET);
        fread (pPrj,1,l,ifp);
        pPrj[l] = 0;
        prjOrig = pPrj;
        fclose (ifp);
    }
    //orig_prj = SHPSetProjection ( in_argc, in_args );
    orig_prj = pj_init_plus(prjOrig);
    new_prj = pj_init_plus( argv[outarg] );
    if (pPrj)
        free (pPrj);
    //new_prj = SHPSetProjection ( out_argc, out_args );

    if ( !(( (!in_argc) || orig_prj) && ( (!out_argc) || new_prj) )) { 
      fprintf (stderr, "unable to process projection, exiting...\n");
        return 0;    }


    SHPGetInfo( old_SHP, &nEntities, &nShapeType, NULL, NULL);
    new_SHP = SHPCreate ( argv[outf_arg], nShapeType ); 
    
    new_DBF = DBFCloneEmpty (old_DBF, argv[outf_arg]);
    if( new_SHP == NULL || new_DBF == NULL )
    {
	printf( "Unable to create new files:%s\n", argv[outf_arg] );
        return 0;
    }

    DBFRow = (char *) malloc ( (old_DBF->nRecordLength) + 15 );
	CreateStatusWind(0, 1,"Transform Shapefile");
    for( i = 0; i < nEntities; i++ )
    {
        int		j;
		if (!StatusWindowUpdate(0, "", nEntities, i))
			break;
        psCShape = SHPReadObject ( old_SHP, i );
        
        SHPProject (psCShape, orig_prj, new_prj,&outFactor );
        
        SHPWriteObject ( new_SHP, -1, psCShape );
        SHPDestroyObject ( psCShape );
        
        memcpy ( DBFRow, DBFReadTuple ( old_DBF, i ), old_DBF->nRecordLength );
        DBFWriteTuple ( new_DBF, new_DBF->nRecords, DBFRow );
        
    }
	DestroyStatusWindow(0);
    SHPFreeProjection ( orig_prj );
    SHPFreeProjection ( new_prj );

    /* store projection params into prj file */
    ifp = fopen( asFileName ( argv[outf_arg], "prj" ),"wt");   
    if ( ifp ) {

       if ( out_argc == 0 ) 
        { fprintf( ifp, "proj=geographic\n" ); }
       else
        { for ( i = 0; i < out_argc; i++ )
           fprintf( ifp, "%s\n", out_args[i]);
        }
       fclose (ifp);
    }
    
    SHPClose( old_SHP );
    SHPClose( new_SHP );
    DBFClose( old_DBF );
    DBFClose( new_DBF );
    return 1;
}

//        pszAccess = "rb+"; 

