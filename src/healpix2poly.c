/*----------------------------------------------------------------------
(C) James C Hill 2006
----------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include "pi.h"
#include "manglefn.h"
#include "defaults.h"

/* getopt options */
const char *optstr = "dqj:i:o:";    // min/max weight and input format


/* local functions */
void    usage(void);

/*--------------------------------------------------------------------
  Main program.
*/
int main(int argc, char *argv[])
{
  int ifile, ipoly, nfiles, npoly, npolys, i, j, nweights;
  double *weights;

  polygon *polys[NPOLYSMAX], *healpix_polys[NPOLYSMAX];

  /* default output format */
  fmt.out = keywords[POLYGON];
  
  /* parse arguments */
  parse_args(argc, argv);

  /* only allowed input format is HEALPix weights (obviously); this is an error here because a switch other than -ih must have been entered */
  if (fmt.in != keywords[HEALPIX_WEIGHT] && fmt.in != NULL) {
     fprintf(stderr, "NOTE: only allowed input format is HEALPix weights!\n");
     usage();
     exit(1);
  }

  /* at least one input and output filename required as arguments */
  if (argc - optind < 2) {
    if (optind > 1 || argc - optind == 1) {
      fprintf(stderr, "%s requires at least 2 arguments: HEALPix_weights_infile and polygon_outfile\n", argv[0]);
      usage();
      exit(1);
    } else {
      usage();
      exit(0);
    }
  }

  msg("---------------- healpix2poly ----------------\n");

  /* snap angles */
  scale(&axtol, axunit, 's');
  scale(&btol, bunit, 's');
  scale(&thtol, thunit, 's');
  axunit = 's';
  bunit = 's';
  thunit = 's';
  msg("snap angles: axis %g%c latitude %g%c edge %g%c\n", axtol, axunit, btol, bunit, thtol, thunit);
  scale(&axtol, axunit, 'r');
  scale(&btol, bunit, 'r');
  scale(&thtol, thunit, 'r');
  axunit = 'r';
  bunit = 'r';
  thunit = 'r';

  /* tolerance angle for multiple intersections */
  if (mtol != 0.) {
     scale(&mtol, munit, 's');
     munit = 's';
     msg("multiple intersections closer than %g%c will be treated as coincident\n", mtol, munit);
     scale(&mtol, munit, 'r');
     munit = 'r';
  }

  /* weight limits */
  if (is_weight_min && is_weight_max) {
     /* min <= max */
     if (weight_min <= weight_max) {
        msg("will keep only weights with values inside [%g, %g]\n", weight_min, weight_max);
	msg("warning: discarded regions of your mask will now have 0 weight!\n");
     }
     /* min > max */
     else {
        msg("will keep only weights with values >= %g or <= %g\n", weight_min, weight_max);
        msg("           (only weights with values outside (%g, %g))\n", weight_max, weight_min);
	msg("warning: discarded regions of your mask will now have 0 weight!\n");
     }
  }
  else if (is_weight_min) {
     msg("will keep only weights with values >= %g\n", weight_min);
     msg("warning: discarded regions of your mask will now have 0 weight!\n");
  }
  else if (is_weight_max) {
     msg("will keep only weights with values <= %g\n", weight_max);
     msg("warning: discarded regions of your mask will now have 0 weight!\n");
  }

  /* advise data format */
  advise_fmt(&fmt);

  /* read weights */
  npoly = 0;
  nfiles = argc - 1 - optind;
  for (ifile = optind; ifile < optind + nfiles; ifile++) {
     npolys = rdmask(argv[ifile], &fmt, NPOLYSMAX - npoly, &polys[npoly]);
     if (npolys == -1) exit(1);
     /* increment number of polygons (i.e., weights) */
     else npoly += npolys;
  }

  if ((ifile - optind)*(npolys) != npoly) {
    fprintf(stderr, "the number of weights in each input file must be the same!\n");
    exit(1);
  }
  if (nfiles >= 2) {
     msg("total of %d weights read\n", npoly);
  }
  /* only allowed input format is HEALPix weights (obviously); this is a trick to catch the error of a wrongly
     formatted input file by seeing whether fmt.nweights has been changed by rdmask, which only occurs if the
     input file was in the healpix_weight format */
  if (fmt.nweights == 0) {
     fprintf(stderr, "ERROR: only allowed input format is HEALPix weights!\n");
     usage();
     exit(1);
  }

  /* only allowed input format is HEALPix weights (obviously); this is just a warning in case the -ih wasn't used */
  if (fmt.in != keywords[HEALPIX_WEIGHT]) {
     msg("NOTE: only allowed input format is HEALPix weights\n");
     msg("no -ih option was specified\n");
     msg("make sure your input file is formatted properly!\n");
  }
  if (npoly == 0) {
     msg("STOP\n");
     exit(0);
  }

  /* allocate memory for weights array */
  weights = (double *) malloc(sizeof(double) * npoly);
  if (!weights) {
    fprintf(stderr, "healpix2poly: failed to allocate memory for %d doubles\n", npoly);
    exit(1);
  }
  
  nweights = npolys;  //since each weights input file must contain the same number of weights (i.e., must be at the same resolution)

  /* check if fmt->nweights is the same as the actual number of weights in the file */
  if (nweights != fmt.nweights) {
    fprintf(stderr, "ERROR: the number of weights in the input file(s) is not equal to %d\n", fmt.nweights);
    exit(1);
  }
  
  /* assign the HEALPix polygons to healpix_polys */
  for(j=0; j<nweights; j++)  healpix_polys[j] = polys[j];

  /* assign the HEALPix polygons' weights to weights array */
  for(j=0; j<npoly; j++) weights[j] = polys[j]->weight;

  /* initialize healpix_polys weights to 0 */
  for(j=0; j<nweights; j++) (healpix_polys[j])->weight = 0.;

  /* add up the weight within each pixel */
  for(ifile=0; ifile<nfiles; ifile++) {
     for(j=0; j<nweights; j++) {
        (healpix_polys[j])->weight += weights[j+ifile*nweights];
     }
  }

  /* apply id numbers to output polygons */
  for (ipoly = 0; ipoly < nweights; ipoly++) {
       (healpix_polys[ipoly])->id = ipoly;
  }

  /* write polygons */
  ifile = argc - 1;
  npolys = wrmask(argv[ifile], &fmt, nweights, healpix_polys);
  if (npolys == -1) exit(1);

  /* free arrays (this frees weights[] as well since they point to
     the weights of the polygons in the polys[] array) */
  for(i=0;i<nweights;i++){
    free_poly(healpix_polys[i]);
  }
  for(i=0;i<npoly;i++){
    free_poly(polys[i]);
  }

  return(0);
}

/*--------------------------------------------------------------------
 */
void usage(void)
{
  printf("usage:\n");
  printf("healpix2poly [-d] [-q] [-j[<min>][,<max>]] [-i<f>[<n>][<u>]] [-o<f>[<u>]] HEALPix_weights_infile1 [HEALPix_weights_infile2 ...] polygon_outfile\n");
#include "usage.h"
}

/*--------------------------------------------------------------------
 */
#include "parse_args.c"
