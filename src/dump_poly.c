/*------------------------------------------------------------------------------
� A J S Hamilton 2001
------------------------------------------------------------------------------*/
#include "manglefn.h"

/*------------------------------------------------------------------------------
  Dump polygon.
*/
void dump_poly(int npoly, polygon *poly[/*npoly*/])
{
    char outfile[] = "jpoly";

    wrmask(outfile, 0x0, npoly, poly);
}
