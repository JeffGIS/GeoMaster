/*
 * This function seems to have been left out of POSIX and the Standard C
 * library.  Since it takes one line in ctype.h, most vendors include it.
 * Just in case you don't have it, here it is.  A macro definition, perhaps
 * in macro.h, would be even better, but we can't guarantee the vendor didn't
 * make a special function that our macro would override.
 *
 * Dana Jacobsen, ERL-C.
 */

DEFUN( isascii, (c), int c)
{
   return ( ((c) & (1 << 7)) == 0 );
}
