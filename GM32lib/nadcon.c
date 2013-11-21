#define PJ_LIB__

#include <projects.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#define  RADDEG    1.74532925199433e-2
typedef struct {double x,y;} DPOINT;
#define DWORD	unsigned int
typedef DPOINT	*LPDPOINT;
struct CTABLE	ct;

DWORD	GM32NADCON (LPDPOINT In,LPDPOINT Out,DWORD dir)
{
	int	inverse=dir;
	LP	in,out;

	in.lam = In->x * RADDEG;
	in.phi = In->y * RADDEG;
	out = nad_cvt(in, inverse, &ct); 
	if (out.lam == HUGE_VAL)
	{
		*Out = *In;
		return 0;
	}
	Out->x = out.lam / RADDEG;
	Out->y = out.phi / RADDEG;
	return 1;
}

DWORD GM32NADCONINIT (char	*File)
{
	struct	CTABLE	*pct;

	pct = nad_init (File);
	if (!pct)
		return 0;
	ct = *pct;
	return 1;
}
