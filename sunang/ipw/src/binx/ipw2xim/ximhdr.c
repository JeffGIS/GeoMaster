#include "ipw.h"

#include "pgm.h"

#include "ImageHeader.h"

void
ximhdr()
{
	ImageHeader     h;
	int		level;
	int		level_scale;

 /*
  * initialize xim header
  */
	bzero(&h, sizeof(h));

	(void) sprintf(h.file_version, "%d", IMAGE_VERSION);
	(void) sprintf(h.header_size, "%d", sizeof(h));
	(void) sprintf(h.image_width, "%d", parm.nsamps);
	(void) sprintf(h.image_height, "%d", parm.nlines);
	(void) sprintf(h.num_colors, "%d", parm.nlevels);
	(void) sprintf(h.num_channels, "%d", parm.nbands);
	(void) sprintf(h.num_pictures, "%d", 1);
 /*
  * initialize colormap
  */
	level_scale = (OUT_NLEVELS + 1) / parm.nlevels;

	for (level = 0; level < parm.nlevels; ++level) {
		int	out_level = level * level_scale;

		h.c_map[level][0] = out_level;
		h.c_map[level][1] = out_level;
		h.c_map[level][2] = out_level;
	}
 /*
  * write xim header + colormap
  */
 /* NOSTRICT */
	if (uwrite(parm.o_fd, (addr_t)(&h), sizeof(h)) != sizeof(h)) {
		error("can't write xim header");
	}
}
