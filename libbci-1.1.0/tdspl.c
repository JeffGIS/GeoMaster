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
#include<math.h>
#include<stdio.h>
#include"bci.h"

#define cell(a,b,c) a[(b)*cols + c]

static double wt[256] =
{
 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
 0., 0., 0., 0., 0., 0., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0.,
-3., 0., 0., 3., 0., 0., 0., 0.,-2., 0., 0.,-1., 0., 0., 0., 0.,
 2., 0., 0.,-2., 0., 0., 0., 0., 1., 0., 0., 1., 0., 0., 0., 0.,
 0., 0., 0., 0., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0., 1., 0., 0., 0.,
 0., 0., 0., 0.,-3., 0., 0., 3., 0., 0., 0., 0.,-2., 0., 0.,-1.,
 0., 0., 0., 0., 2., 0., 0.,-2., 0., 0., 0., 0., 1., 0., 0., 1.,
-3., 3., 0., 0.,-2.,-1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
 0., 0., 0., 0., 0., 0., 0., 0.,-3., 3., 0., 0.,-2.,-1., 0., 0.,
 9.,-9., 9.,-9., 6., 3.,-3.,-6., 6.,-6.,-3., 3., 4., 2., 1., 2.,
-6., 6.,-6., 6.,-4.,-2., 2., 4.,-3., 3., 3.,-3.,-2.,-1.,-1.,-2.,
 2.,-2., 0., 0., 1., 1., 0., 0., 0., 0., 0., 0., 0., 0., 0., 0.,
 0., 0., 0., 0., 0., 0., 0., 0., 2.,-2., 0., 0., 1., 1., 0., 0.,
-6., 6.,-6., 6.,-3.,-3., 3., 3.,-4., 4., 2.,-2.,-2.,-2.,-1.,-1.,
 4.,-4., 4.,-4., 2., 2.,-2.,-2., 2.,-2.,-2., 2., 1., 1., 1., 1.};

/*
 solves f(x,y) = Sum_{i=0}^{3}{ Sum_{j=0}^{3}{ c_{ij}*x^i*y^j } }
 on 4x4 points
 */
void td_solve4by4(double c[16], doublexyz xyz[16])
{
    short i, j, k, cols = 16; /* cols defined to use matrix cell() define */
    double yv[16];
    // M[16][16] = m_i_j
    double mtrx[256];
    // initialize M and y-vector
    for(k = 0; k < 16; k++) {
        for(i = 0; i < 4; i++)
            for(j = 0; j < 4; j++) {
                if(i == 0 && j == 0)
                    cell(mtrx, k, 0) = 1.;
                else if(i == 0 && j != 0)
                    cell(mtrx, k, j) = pow_int(xyz[k].y, j);
                else if(i != 0 && j == 0)
                    cell(mtrx, k, 4*i) = pow_int(xyz[k].x, i);
                else
                    cell(mtrx, k, 4*i + j) =
                        pow_int(xyz[k].x, i)*pow_int(xyz[k].y, j);
            }
        yv[k] = xyz[k].z;
    }

    // triangulize the matrix
    triangm(mtrx, 16, yv);

    // count c-vector items
    for(i = 15; i >= 0; i--) {
        c[i] = yv[i];
        for(j = i + 1; j < 16; j++)
            if(cell(mtrx, i, j) != 0. || c[j] != 0.)
                c[i] -= cell(mtrx, i, j)*c[j];
        c[i] /= cell(mtrx, i, i);
    }
}

/*
 finds c_i coefficients of function f(x,y) on 4 points, 4+4 first
 derivatives and 4 mixed derivatives
 f(x,y) = c0 + c1*y + c2*y^2 + c3*y^3 + c4*x + c5*x*y + c6*x*y^2 + c7*x*y^3
 + c8*x^2 + c9*x^2*y + c10*x^2*y^2 + c11*x^2*y^3 + c12*x^3 + c13*x^3*y
 + c14*x^3*y^2 + c15*x^3*y^3
 */
void td_solveonespline4(double c[16], const doublexyzwder xyzwder[4])
{
    long i, j;
    double x[16], dx, dy, dxdy;

    dx = xyzwder[2].x - xyzwder[0].x;
    dy = xyzwder[2].y - xyzwder[0].y;
    dxdy = dx*dy;

    for(i = 0; i < 4; i++) {
        x[     i] = xyzwder[i].z;
        x[ 4 + i] = xyzwder[i].dx*dx;
        x[ 8 + i] = xyzwder[i].dy*dy;
        x[12 + i] = xyzwder[i].dxdy*dxdy;
    }

    for(i = 0; i < 16; i++) {
        c[i] = 0.;
        for(j = 0; j < 16; j++)
            c[i] += wt[16*i + j]*x[j];
    }
}

