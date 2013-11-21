#define PJ_LIB__

#include <projects.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#define  RADDEG    1.74532925199433e-2
int __stdcall testnad (int i)
{
	int	inverse=0;
	struct CTABLE	ct;
	LP	in,out;
	double	x,y;

	in.lam = -84.942180368340 * RADDEG;
	in.phi = 43.141126550104 * RADDEG;
	ct = *nad_init ("C:\\Gssi\\prog\\Proj-4\\win32\\proj\\nad\\conus.bin");
	out = nad_cvt(in, inverse, &ct); 
	x = out.lam / RADDEG;
	y = out.phi / RADDEG;
	return 1;
}
