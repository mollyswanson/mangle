/*------------------------------------------------------------------------------
� A J S Hamilton 2001
------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include "manglefn.h"

/*------------------------------------------------------------------------------
  Called as fortran subroutine
      ikrand(ik,ikran)

  Pseudo-random unsigned long or unsigned long long associated with integer ik,
  returned as a double.
*/
void ikrand_(int *ik, double *ikran)
{
    unsigned long *likran;
    unsigned long long *llikran;

#pragma omp critical(ikrand)
    {

    /* seed random number generator with *ik */
    srandom((unsigned int) *ik);

    /* generate pseudo-random unsigned long */
    if (sizeof(long long) > sizeof(double)) {
	likran = (unsigned long *)ikran;
	*likran = random();
    /* generate pseudo-random unsigned long long */
    } else {
	llikran = (unsigned long long *)ikran;
	*llikran = (unsigned int)random();
	*llikran <<= (8 * sizeof(unsigned int));
	*llikran |= (unsigned int)random();
    }

    }

}

/*------------------------------------------------------------------------------
  Called as fortran subroutine
      ikrandp(ikchk,ikran)

  ikchk = ikchk + ikran
  passed as double's, but treated as unsigned long's or unsigned long long's.
*/
void ikrandp_(double *ikchk, double *ikran)
{
    unsigned long *likchk;
    unsigned long long *llikchk;

    if (sizeof(long long) > sizeof(double)) {
	likchk = (unsigned long *)ikchk;
	*likchk += *(unsigned long *)ikran;
    } else {
	llikchk = (unsigned long long *)ikchk;
	*llikchk += *(unsigned long long *)ikran;
    }
}

/*------------------------------------------------------------------------------
  Called as fortran subroutine
      ikrandm(ikchk,ikran)

  ikchk = ikchk - ikran
  passed as double's, but treated as unsigned long's or unsigned long long's.
*/
void ikrandm_(double *ikchk, double *ikran)
{
    unsigned long *likchk;
    unsigned long long *llikchk;

    if (sizeof(long long) > sizeof(double)) {
	likchk = (unsigned long *)ikchk;
	*likchk -= *(unsigned long *)ikran;
    } else {
	llikchk = (unsigned long long *)ikchk;
	*llikchk -= *(unsigned long long *)ikran;
    }
}
