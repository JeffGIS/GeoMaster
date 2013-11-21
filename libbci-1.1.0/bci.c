/*
libbci is an interpolation library based on the bicubic interpolation method
Copyright (C) 2004 Andrew E. Shevtsov

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "graphint.h"

#include"bci.h"

_inline double td_func16(doublexyz xyz, const double *c);
_inline double td_firstxderfunc16(doublexyz xyz, const double *c);
_inline double td_firstyderfunc16(doublexyz xyz, const double *c);
_inline double td_mixedderfunc16(doublexyz xyz, const double *c);

void copyxyz(long width, const doublexyz *xyzfrom, doublexyz *xyz, long cols,
             long i, long j);
void copy4xyzwder(doublexyzwder *xyzwder, const doublexyz *xyz,
                  const doubleder *der, long cols, long x, long y);

_inline void countderivs(doubleder *der, const doublexyz *xyz, const double *c,
                 long cols, long x, long y);
void td_fillgrid(const doublexyz *xyz, long cols, long rows, doublexyz *fxyz,
                 long fcols, long frows);
void td_fillgrid1(const doublexyz *xyz, long cols, long rows, doublexyz *fxyz,
                 long fcols, long frows);
void td_fillgrid_wo_nulls(const doublexyz *xyz, long cols, long rows,
                          doublexyz *fxyz, long fcols, long frows);

#define cell(a,b,c) a[(b)*cols + c] /* a[i][j] = a[i*cols + j] */
/*
 copies (width)x(width) matrix xyz values from xyzfrom from [i][j] element
 */
void copyxyz(long width, const doublexyz *xyzfrom, doublexyz *xyz, long cols,
             long i, long j)
{
    long x, y;
    for(x = 0; x < width; x++)
        for(y = 0; y < width; y++) {
            xyz[width*x + y].x = cell(xyzfrom, x+i, y+j).x;
            xyz[width*x + y].y = cell(xyzfrom, x+i, y+j).y;
            xyz[width*x + y].z = cell(xyzfrom, x+i, y+j).z;
        }
}
#undef cell

/*
 f(x,y) = c0 + c1*y + c2*y^2 + c3*y^3 + c4*x + c5*x*y + c6*x*y^2 + c7*x*y^3
 + c8*x^2 + c9*x^2*y + c10*x^2*y^2 + c11*x^2*y^3 + c12*x^3 + c13*x^3*y
 + c14*x^3*y^2 + c15*x^3*y^3
 */   
int __cdecl printfx(const char *mess, ...)
{
	GSSiMsgBox (0,(LPSTR)mess,NULL,MB_ICONEXCLAMATION,0);
	return 0;
}
 
void bzero (HPSTR str,long n)
{
	while (n--)
		*str = 0;
	return;
}
_inline double td_func16(doublexyz xyz, const double *c)
{
    return c[0] + c[1]*xyz.y + c[2]*pow_2(xyz.y) + c[3]*pow_3(xyz.y) +
        c[4]*xyz.x + c[5]*xyz.x*xyz.y + c[6]*xyz.x*pow_2(xyz.y) +
        c[7]*xyz.x*pow_3(xyz.y) + c[8]*pow_2(xyz.x) +
        c[9]*pow_2(xyz.x)*xyz.y + c[10]*pow_2(xyz.x)*pow_2(xyz.y) +
        c[11]*pow_2(xyz.x)*pow_3(xyz.y) + c[12]*pow_3(xyz.x) +
        c[13]*pow_3(xyz.x)*xyz.y +
        c[14]*pow_3(xyz.x)*pow_2(xyz.y) +
        c[15]*pow_3(xyz.x)*pow_3(xyz.y);
}

/*
 first x derivative for func16
 */
_inline double td_firstxderfunc16(doublexyz xyz, const double *c)
{
    return c[4] + c[5]*xyz.y + c[6]*pow_2(xyz.y) + c[7]*pow_3(xyz.y) +
        c[8]*2.*xyz.x + c[9]*2.*xyz.x*xyz.y + c[10]*2.*xyz.x*pow_2(xyz.y) +
        c[11]*2.*xyz.x*pow_3(xyz.y) + c[12]*3.*pow_2(xyz.x) +
        c[13]*3.*pow_2(xyz.x)*xyz.y +
        c[14]*3.*pow_2(xyz.x)*pow_2(xyz.y) +
        c[15]*3.*pow_2(xyz.x)*pow_3(xyz.y);
}

/*
 first y derivative for func16
 */
_inline double td_firstyderfunc16(doublexyz xyz, const double *c)
{
    return c[1] + c[2]*2.*xyz.y + c[3]*3.*pow_2(xyz.y) + c[5]*xyz.x +
        c[6]*xyz.x*2.*xyz.y + c[7]*xyz.x*3.*pow_2(xyz.y) +
        c[9]*pow_2(xyz.x) + c[10]*pow_2(xyz.x)*2.*xyz.y +
        c[11]*pow_2(xyz.x)*3.*pow_2(xyz.y) +
        c[13]*pow_3(xyz.x) + c[14]*pow_3(xyz.x)*2.*xyz.y +
        c[15]*pow_3(xyz.x)*3.*pow_2(xyz.y);
}

/*
 mixed derivative for func16
 */
_inline double td_mixedderfunc16(doublexyz xyz, const double *c)
{
    return c[5] + c[6]*2.*xyz.y + c[7]*3.*pow_2(xyz.y) + c[9]*2.*xyz.x +
        c[10]*4.*xyz.x*xyz.y + c[11]*6.*xyz.x*pow_2(xyz.y) +
        c[13]*3.*pow_2(xyz.x) + c[14]*6.*pow_2(xyz.x)*xyz.y +
        c[15]*9.*pow_2(xyz.x)*pow_2(xyz.y);
}

#define xyz_def(a,b) xyz[(a)*cols + b]
#define der_def(a,b) der[(a)*cols + b]

void copy4xyzwder(doublexyzwder *xyzwder, const doublexyz *xyz,
                  const doubleder *der, long cols, long x, long y)
{
    /* (0,0) */
    xyzwder[0].x = xyz_def(x,y).x;
    xyzwder[0].y = xyz_def(x,y).y;
    xyzwder[0].z = xyz_def(x,y).z;
    xyzwder[0].dx = der_def(x,y).dx;
    xyzwder[0].dy = der_def(x,y).dy;
    xyzwder[0].dxdy = der_def(x,y).dxdy;
    /* (0,1) */
    xyzwder[1].x = xyz_def(x + 1,y).x;
    xyzwder[1].y = xyz_def(x + 1,y).y;
    xyzwder[1].z = xyz_def(x + 1,y).z;
    xyzwder[1].dx = der_def(x + 1,y).dx;
    xyzwder[1].dy = der_def(x + 1,y).dy;
    xyzwder[1].dxdy = der_def(x + 1,y).dxdy;
    /* (1,1) */
    xyzwder[2].x = xyz_def(x + 1,y + 1).x;
    xyzwder[2].y = xyz_def(x + 1,y + 1).y;
    xyzwder[2].z = xyz_def(x + 1,y + 1).z;
    xyzwder[2].dx = der_def(x + 1,y + 1).dx;
    xyzwder[2].dy = der_def(x + 1,y + 1).dy;
    xyzwder[2].dxdy = der_def(x + 1,y + 1).dxdy;
    /* (1,0) */
    xyzwder[3].x = xyz_def(x,y + 1).x;
    xyzwder[3].y = xyz_def(x,y + 1).y;
    xyzwder[3].z = xyz_def(x,y + 1).z;
    xyzwder[3].dx = der_def(x,y + 1).dx;
    xyzwder[3].dy = der_def(x,y + 1).dy;
    xyzwder[3].dxdy = der_def(x,y + 1).dxdy;
}

_inline void countderivs(doubleder *der, const doublexyz *xyz, const double *c,
                        long cols, long x, long y)
{
    der_def(x,y).dx = td_firstxderfunc16(xyz_def(x,y), c);
    der_def(x,y).dy = td_firstyderfunc16(xyz_def(x,y), c);
    der_def(x,y).dxdy = td_mixedderfunc16(xyz_def(x,y), c);
}

void td_fillgrid(const doublexyz *xyz, long cols, long rows, doublexyz *fxyz,
                 long fcols, long frows)
{
    long i, j, x, y, ff_cols, ff_rows;
    double xstep, ystep;
    double splc[16], c[16];
    doubleder *der; /* rows*cols*(first x + first y + mixed) */
    doublexyz splxyz[16];
    doublexyzwder xyzwder[4];
/* checks for minimum needed data */
    if(cols < 4 || rows < 4) {
        printf("Needs minimum data 4x4 points\n");
        exit(0);
    }
/* temporary check for square matrix */
    if(cols != rows) {
        printf("Works only with square data matrixes so far (rows = columns).\n");
        exit(0);
    }

    ff_cols = max (1,fcols/cols);
    ff_rows = max (1,frows/rows);

    bzero((HPSTR)splxyz, 16*sizeof(doublexyz));
    der = (doubleder *)calloc((int)(rows*cols), sizeof(doubleder));

/* counts derivatives */
    for(x = 0; x < cols - 3; x++)
        for(y = 0; y < rows - 3; y++) {
            copyxyz(4, xyz, splxyz, cols, x, y);
            td_solve4by4(c, splxyz);
            for(i = 0; i < 4; i++)
                for(j = 0; j < 4; j++)
                    countderivs(der, xyz, c, cols, x + i, y + j);
        }
#undef der_def
#undef xyz_def

/* sequently count each square */
    for(x = 0; x < rows - 1; x++) {
        for(y = 0; y < cols - 1; y++) {
            copy4xyzwder(xyzwder, xyz, der, cols, x, y);
            td_solveonespline4(c, xyzwder);
            xstep = (xyzwder[2].x - xyzwder[0].x)/ff_cols;
            ystep = (xyzwder[2].y - xyzwder[0].y)/ff_rows;
            for(i = 0; i < ff_cols; i++)
                for(j = 0; j < ff_rows; j++) {
                    if(!((i == 0 && j == 0) ||
                       (i == 0 && j == ff_rows) ||
                       (i == ff_cols && j == 0))) {
                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].x =
                            i*xstep;
                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].y =
                            j*ystep;

                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].z =
                            td_func16(fxyz[fcols*(x*ff_cols + i) +
                                           y*ff_rows + j], c);

                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].x +=
                            xyzwder[0].x;
                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].y +=
                            xyzwder[0].y;
                    }
                }
        }
    }
    for(x = 0; x < rows; x++)
        for(y = 0; y < cols; y++) {
            fxyz[fcols*x*ff_cols + y*ff_rows].x = xyz[cols*x + y].x;
            fxyz[fcols*x*ff_cols + y*ff_rows].y = xyz[cols*x + y].y;
            fxyz[fcols*x*ff_cols + y*ff_rows].z = xyz[cols*x + y].z;
        }

    free(der);
}

void td_fillgrid1(const doublexyz *xyz, long cols, long rows, doublexyz *fxyz,
                 long fcols, long frows)
{
    long i, j, x, y, ff_cols, ff_rows;
    double xstep, ystep;
    double splc[16], c[16];
    doubleder *der; /* rows*cols*(first x + first y + mixed) */
    doublexyz splxyz[16];
    doublexyzwder xyzwder[4];
/* checks for minimum needed data */
    if(cols < 4 || rows < 4) {
        printf("Needs minimum data 4x4 points\n");
        exit(0);
    }
/* temporary check for square matrix */
    if(cols != rows) {
        printf("Works only with square data matrixes so far (rows = columns).\n");
        exit(0);
    }

    ff_cols = max (1,fcols/cols);
    ff_rows = max (1,frows/rows);

    bzero((HPSTR)splxyz, 16*sizeof(doublexyz));
    der = (doubleder *)calloc((int)(rows*cols), sizeof(doubleder));

/* counts derivatives */
    for(x = 0; x < cols - 3; x++)
        for(y = 0; y < rows - 3; y++) {
            copyxyz(4, xyz, splxyz, cols, x, y);
            td_solve4by4(c, splxyz);
            for(i = 0; i < 4; i++)
                for(j = 0; j < 4; j++)
                    countderivs(der, xyz, c, cols, x + i, y + j);
        }
#undef der_def
#undef xyz_def

/* sequently count each square */
    for(x = 3; x < 4; x++) {
        for(y = 3; y <4; y++) {
            copy4xyzwder(xyzwder, xyz, der, cols, x, y);
            td_solveonespline4(c, xyzwder);
            xstep = (xyzwder[2].x - xyzwder[0].x)/ff_cols;
            ystep = (xyzwder[2].y - xyzwder[0].y)/ff_rows;
            for(i = 0; i < ff_cols; i++)
                for(j = 0; j < ff_rows; j++) {
                    if(!((i == 0 && j == 0) ||
                       (i == 0 && j == ff_rows) ||
                       (i == ff_cols && j == 0))) {
                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].x =
                            i*xstep;
                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].y =
                            j*ystep;

                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].z =
                            td_func16(fxyz[fcols*(x*ff_cols + i) +
                                           y*ff_rows + j], c);

                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].x +=
                            xyzwder[0].x;
                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].y +=
                            xyzwder[0].y;
                    }
                }
        }
    }
    for(x = 0; x < rows; x++)
        for(y = 0; y < cols; y++) {
            fxyz[fcols*x*ff_cols + y*ff_rows].x = xyz[cols*x + y].x;
            fxyz[fcols*x*ff_cols + y*ff_rows].y = xyz[cols*x + y].y;
            fxyz[fcols*x*ff_cols + y*ff_rows].z = xyz[cols*x + y].z;
        }

    free(der);
}

void td_fillgrid_wo_nulls(const doublexyz *xyz, long cols, long rows,
                          doublexyz *fxyz, long fcols, long frows)
{
    long i, j, x, y, ff_cols, ff_rows;

    td_fillgrid(xyz, cols, rows, fxyz, fcols, frows);

    ff_cols = fcols/cols;
    ff_rows = frows/rows;

    for(x = 0; x < rows; x++)
        for(y = 0; y < cols; y++) {
            /* clearing in-grid points */
            if(x < rows - 1 && y < cols - 1)
                if(xyz[cols*x + y].z == 0. ||
                   xyz[cols*(x + 1) + y].z == 0. ||
                   xyz[cols*x + y + 1].z == 0. ||
                   xyz[cols*(x + 1) + y + 1].z == 0.)
                    for(i = 1; i < ff_cols; i++)
                        for(j = 1; j < ff_rows; j++)
                            fxyz[fcols*(x*ff_cols + i) + y*ff_rows + j].z = 0.;
            /* clearing on-grid points */
            if(x < rows - 1)
                if(xyz[cols*x + y].z == 0. || xyz[cols*(x + 1) + y].z == 0.)
                    for(i = 1; i < ff_cols; i++)
                        fxyz[fcols*(x*ff_cols + i) + y*ff_rows].z = 0.;

            if(y < cols - 1)
                if(xyz[cols*x + y].z == 0. || xyz[cols*x + y + 1].z == 0.)
                    for(j = 1; j < ff_rows; j++)
                        fxyz[fcols*x*ff_cols + y*ff_rows + j].z = 0.;
        }
}
