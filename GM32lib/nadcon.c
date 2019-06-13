#define PJ_LIB__

#include <projects.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#define  DEGTORAD    1.74532925199433e-2
typedef struct {double x,y;} DPOINT;
#define DWORD	unsigned int
typedef DPOINT	*LPDPOINT;
static struct	CTABLE	*pct;

void GM32NADCONFREE(void)
{
	if (pct)
		nad_free(pct);
	pct = 0;
}

DWORD	GM32NADCON (LPDPOINT In,LPDPOINT Out,DWORD dir)
{
	int	inverse=dir;
	LP	in,out;

	in.lam = In->x * DEGTORAD;
	in.phi = In->y * DEGTORAD;
	out = nad_cvt(in, inverse, pct); 
	if (out.lam == HUGE_VAL)
	{
		*Out = *In;
		return 0;
	}
	Out->x = out.lam / DEGTORAD;
	Out->y = out.phi / DEGTORAD;
	return 1;
}

DWORD GM32NADCONINIT (char	*File)
{
	GM32NADCONFREE();

	pct = nad_init (File);
	if (!pct)
		return 0;
	return 1;
}


