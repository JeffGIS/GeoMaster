/*C
C       ******************************
C        NGI AREA INTERSECTION COMMON - JS
C       ******************************
C******* CONTAINS THE INTERSECTION LOCATIONS OF EACH ROW OF NODES IN
C        A GEOSEGMENT WITH THE DESIGNATED AREA.
      INTEGER*4 BINTA
      COMMON BINTA(20,512)     */
struct js
   {
      long BINTA[21][513];
   } NAINTA;   