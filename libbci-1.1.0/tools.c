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

#define cell(a,b,c) a[(b)*cols + c]

void bcopy (HPSTR in, HPSTR out, long len)
{
	hmemmove (out,in,len);
	return;
}
	
/*
 prints a matrix
 */
void printm(char *title, double *data, long cols, long rows)
{
    long i, j;

    printf("=> %s\n", title);
    for(i = 0; i < rows; i++) {
        printf("%2d> ", i + 1);

        for(j = 0; j < cols; j++)
            if(j == cols - 1)
                printf("%3g", data[i*cols + j]);
            else
                printf("%3g ", data[i*cols + j]);

        printf("\n");
    }
    printf("<= %s\n", title);
}

/*
 triangulizes equation system mtrx[cols][cols]*v[cols]=g[cols]
 */
void triangm(double *mtrx, long cols, double *g)
{
    long i, j, k, r;
    double alpha;
    double *tmprow, tmpg;

    for(k = 0; k < cols; k++) {
        for(i = k; i < cols - 1; i++) {
            /* if cell is null trying to swap rows */
            if(cell(mtrx,k,k) == 0.0) {
                for(r = k; r < cols; r++)
                    if(cell(mtrx,r,k) != 0.0) {
                        printf("triang: moving rows %d %d\n", r, k);
                        tmprow = (double *)malloc((int)(sizeof(double)*cols));
                        tmpg = g[k];
                        bcopy((HPSTR)&mtrx[cols*k], (HPSTR)tmprow, sizeof(double)*cols);
                        bcopy((HPSTR)&mtrx[cols*r], (HPSTR)&mtrx[cols*k], sizeof(double)*cols);
                        g[k] = g[r];
                        bcopy((HPSTR)tmprow, (HPSTR)&mtrx[cols*r], sizeof(double)*cols);
                        g[r] = tmpg;
                        free(tmprow);
                        break;
                    }
                /* untriangulizable system */
                if(r == cols) {
                    printf("triang: cell(%d,%d) is null\nNo rows to move\n", k, k);
                    printm("mtrx", mtrx, cols, cols);
                    exit(1);
                }
            }
            if(cell(mtrx,i+1,k) != 0.) {
            /* count */
                alpha = cell(mtrx,i+1,k)/cell(mtrx,k,k);

                for(j = k; j < cols; j++)
                    cell(mtrx,i+1,j) -= alpha*cell(mtrx,k,j);

                if( g != NULL)
                    g[i+1] -= alpha*g[k];
            }
        }
    }
}

/* thanx to GSL */
double pow_int(double x, long n)
{
  double value = 1.0;

  if(n < 0) {
    x = 1.0/x;
    n = -n;
  }

  /* repeated squaring method
   * returns 0.0^0 = 1.0, so continuous in x
   */
  do {
     if(n & 1) value *= x;  /* for n odd */
     n >>= 1;
     x *= x;
  } while (n);

  return value;
}

