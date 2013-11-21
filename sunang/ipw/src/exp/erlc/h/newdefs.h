/*
	SCCS version: @(#)   newdefs.h   2.1   4/13/90 
*/

/*
 *  {machine,system}-independent symbolic constants
 */

/*
 *  general-purpose
 */
#define DPROTECT        0755            /*  default mode for mkdir()    */
#define ERRSTRSIZE      80              /*  size of Qerrstr             */
#define FILENAMESIZE    64              /*  max # chars per path name   */
#define NBPW            sizeof(int)     /*  # bytes per "word"          */
#define NIL             ((char *)0)     /*  null pointer                */
#define NO              0               /*  logical "false"             */
#define NULLPTR         NIL             /*  (obsolete)                  */
#define NULLSTR         NIL             /*  (obsolete)                  */
#define READONLY        0               /*  read() mode                 */
#define READWRITE       2               /*  read() mode                 */
#define WRITEONLY       1               /*  read() mode                 */
#define PROTECT         0644            /*  default mode for creat()    */
#define TRACESIZE       128             /*  size of Qtrace              */
#define YES             1               /*  logical "true"              */

/*
 *  projections, default QDIPS ellipsoid is Clarke 1866
 */
#define ELLIPSE 2
