/******************************************************************************
 * $Id: dgndump.c,v 1.5 2001/03/07 13:56:44 warmerda Exp $
 *
 * Project:  Microstation DGN Access Library
 * Purpose:  Temporary low level DGN dumper application.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2000, Avenza Systems Inc, http://www.avenza.com/
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
 * $Log: dgndump.c,v $
 * Revision 1.5  2001/03/07 13:56:44  warmerda
 * updated copyright to be held by Avenza Systems
 *
 * Revision 1.4  2001/03/07 13:49:37  warmerda
 * removed attribute dumping, handled by DGNDumpElement()
 *
 * Revision 1.3  2001/01/10 16:10:57  warmerda
 * Added extents reporting
 *
 * Revision 1.2  2000/12/28 21:27:38  warmerda
 * added summary report
 *
 * Revision 1.1  2000/12/14 17:11:18  warmerda
 * New
 *
 */
#include <windows.h>
#include "dgnlibp.h"
#define DGN_DESCRIPTION_NAME_LENGTH   34
#define DGN_DESCRIPTION_DESCRIPTION_LENGTH 68
 
/*typedef struct
{
 short   level;
 char   name        [ DGN_DESCRIPTION_NAME_LENGTH        ];
 char   description [ DGN_DESCRIPTION_DESCRIPTION_LENGTH ];
} DGN_Description;
 
typedef struct       // Type 66 Level 6
{
 Element_Header ehdr;
 Display_Header dhdr;
 
 char   vec1 [ 4 ];
 
 short   number_of_descriptions;
 DGN_Description description [ 1 ];
 
} DGN_Level_Names;*/

/************************************************************************/
/*                                main()                                */
/************************************************************************/

int main( int argc, char ** argv )

{
    DGNHandle   hDGN;
    DGNElemCore *psElement;
    const char	*pszFilename;
    int         bSummary = FALSE;
	FILE	*OutFile;
    double		adfExtents[6];

    if( argc < 2 )
    {
        MessageBox (GetFocus(), "Usage: dgndump [-s] filename.dgn\n" ,NULL,MB_ICONEXCLAMATION);
        exit( 1 );
    }
    
    if( strcmp(argv[1],"-s") == 0 )
    {
        bSummary = TRUE;
        pszFilename = argv[2];
    }
    else
        pszFilename = argv[1];

	OutFile = fopen (argv[2],"w+");
    hDGN = DGNOpen( pszFilename ,FALSE);
    if( hDGN == NULL )
        exit( 1 );

    DGNGetExtents( hDGN, adfExtents );
    fprintf(OutFile, "File Extents: %.2f %.2f %.2f %.2f\n", 
            adfExtents[0],adfExtents[1], adfExtents[3],  adfExtents[4]);
	if( !bSummary )
    {

        while( (psElement=DGNReadElement(hDGN)) != NULL )
        {
            DGNDumpElement( hDGN, psElement, OutFile );
            DGNFreeElement( hDGN, psElement );
        }
		fprintf(OutFile, "\n");
    }
    else
    {
        const DGNElementInfo 	*pasEI;
        int			nCount, i, nLevel, nType;
        int			anLevelTypeCount[128*64];
        int			anLevelCount[64];
        int			anTypeCount[128];


        pasEI = DGNGetElementIndex( hDGN, &nCount );

        fprintf(OutFile, "Total Elements: %d\n", nCount );
        
        memset( anLevelTypeCount, 0, 128*64*sizeof(int) );
        memset( anLevelCount, 0, 64*sizeof(int) );
        memset( anTypeCount, 0, 128*sizeof(int) );

        for( i = 0; i < nCount; i++ )
        {
            anLevelTypeCount[pasEI[i].level * 128 + pasEI[i].type]++;
            anLevelCount[pasEI[i].level]++;
            anTypeCount[pasEI[i].type]++;
        }

        fprintf(OutFile, "\n" );
        fprintf(OutFile, "Per Type Report\n" );
        fprintf(OutFile, "===============\n" );

        for( nType = 0; nType < 128; nType++ )
        {
            if( anTypeCount[nType] != 0 )
            {
                fprintf(OutFile, "Type %s: %d\n", 
                        DGNTypeToName( nType ), 
                        anTypeCount[nType] );
            }
        }

        fprintf(OutFile, "\n" );
        fprintf(OutFile, "Per Level Report\n" );
        fprintf(OutFile, "================\n" );

        for( nLevel = 0; nLevel < 64; nLevel++ )
        {
            if( anLevelCount[nLevel] == 0 )
                continue;

            fprintf(OutFile, "Level %d, %d elements:\n", 
                    nLevel, 
                    anLevelCount[nLevel] );

            for( nType = 0; nType < 128; nType++ )
            {
                if( anLevelTypeCount[nLevel * 128 + nType] != 0 )
                {
                    fprintf(OutFile, "  Type %s: %d\n", 
                            DGNTypeToName( nType ), 
                            anLevelTypeCount[nLevel*128 + nType] );
                }
            }

            fprintf(OutFile, "\n" );
        }
    }

    DGNClose( hDGN );
	fclose (OutFile);
    return 0;
}
