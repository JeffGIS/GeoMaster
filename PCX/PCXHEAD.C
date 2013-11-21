typedef struct	{
	char	manufacturer;		/* always 0xa0 */
	char	version;		/* version number */
	char	encoding;		/* always 1 */
	char	bits_per_pixel;		/* color bits */
	int	xmin,ymin;		/* image origin     */
	int	xmax,ymax;		/* image dimensions */
	int	hres;			/* resolution values */
	int	vres;
	char	palette[48];		/* color palette */
        char	reserved;
	char	colour_planes;		/* color planes */
	int	bytes_per_line;		/* line buffer size */
	int	palette_type;		/* grey or color palette */
	char	filler[58];
} PCXHEAD;
