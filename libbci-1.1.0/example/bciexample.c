#include<bci.h>

int main()
{
    int i, j;
    double x, y;
    doublexyz xyz[16], fxyz[256];

    for(i = 0; i < 4; i++)
        for(j = 0; j < 4; j++) {
            x = i + 2.;
            y = j + 2.;
            xyz[4*i+j].x = x;
            xyz[4*i+j].y = y;
            xyz[4*i+j].z = 2. + 2.*x*x + y*y*y + x*y;
            printf("%d: %f %f %f\n", 4*i + j,
                   xyz[4*i+j].x, xyz[4*i+j].y, xyz[4*i+j].z);
        }

    td_fillgrid(xyz, 4, 4, fxyz, 16, 16);

    for(i = 0; i < 16; i++) {
        for(j = 0; j < 16; j++)
            printf("%f ", fxyz[16*i + j].z);
        printf("\n");
    }
}
