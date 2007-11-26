/*------------------------------------------------------------------------------
© A J S Hamilton 2001
------------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "manglefn.h"
#include "defaults.h"

/* getopt options */
const char *optstr = "dqSa:b:t:y:m:s:e:v:p:i:o:H";

/* local functions */
void	usage(void);

/*------------------------------------------------------------------------------
  Main program.
*/
int main(int argc, char *argv[])
{
    int ifile, nadj, nfiles, npoly, npolys, i, flag;
    polygon *polys[NPOLYSMAX];

    /* default output format */
    fmt.out = keywords[POLYGON];
    /* default is to renumber output polygons with new id numbers */
    fmt.newid = 'n';

    /* parse arguments */
    parse_args(argc, argv);

    /* at least one input and output filename required as arguments */
    if (argc - optind < 2) {
	if (optind > 1 || argc - optind == 1) {
	    fprintf(stderr, "%s requires at least 2 arguments: polygon_infile and polygon_outfile\n", argv[0]);
	    usage();
	    exit(1);
	} else {
	    usage();
	    exit(0);
	}
    }

    msg("---------------- snap ----------------\n");

    /* snap angles */
    scale(&axtol, axunit, 's');
    scale(&btol, bunit, 's');
    scale(&thtol, thunit, 's');
    axunit = 's';
    bunit = 's';
    thunit = 's';
    msg("snap angles: axis %Lg%c latitude %Lg%c edge %Lg%c\n", axtol, axunit, btol, bunit, thtol, thunit);
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
	msg("multiple intersections closer than %Lg%c will be treated as coincident\n", mtol, munit);
	scale(&mtol, munit, 'r');
	munit = 'r';
    }

    /* advise data format */
    advise_fmt(&fmt);

    /* read polygons */
    npoly = 0;
    nfiles = argc - 1 - optind;
    for (ifile = optind; ifile < optind + nfiles; ifile++) {
	npolys = rdmask(argv[ifile], &fmt, NPOLYSMAX - npoly, &polys[npoly]);
	if (npolys == -1) exit(1);
	npoly += npolys;
    }
    if (nfiles >= 2) {
	msg("total of %d polygons read\n", npoly);
    }
    if (npoly == 0) {
	msg("STOP\n");
	exit(0);
    }

    if (fmt.healpix_out == 1) {
      if (fmt.nweights == 0) fmt.nweights = npoly/nfiles;
      /* check whether fmt.nweights is a valid number based on the HEALPix pixelization scheme */
      for (i = 1; i <= 8192; i = 2*i) {
	if (12*powl(i,2) == fmt.nweights) {
	  flag = 1;
	  break;
	}
	else {
	  flag = 0;
	  continue;
	}
      }
      if (flag == 0) {
	fprintf(stderr, "error: %d is not a valid number of HEALPix polygons\n", fmt.nweights);
	fprintf(stderr, "see the HEALPix website at http://healpix.jpl.nasa.gov for more information\n");
	fprintf(stderr, "the -H argument will thus be ignored\n");
	fmt.healpix_out = 0;
      }
      else msg("polygon_outfile will be formatted for subsequent use as polygon_infile1 in rasterize, with nweights = %d\n", fmt.nweights);
    }

    /* adjust boundaries of polygons */
    nadj = snap(npoly, polys, mtol, &fmt, axtol, btol, thtol, ytol, selfsnap);
    if(nadj==-1) exit(1);

    /* write polygons */
    ifile = argc - 1;
    npoly = wrmask(argv[ifile], &fmt, npoly, polys);
    if (npoly == -1) exit(1);

    return(0);
}

/*------------------------------------------------------------------------------
*/
void usage(void)
{
    printf("usage:\n");
    printf("snap [-d] [-q] [-S] [-a<a>[u]] [-b<a>[u]] [-t<a>[u]] [-y<r>] [-m<a>[u]] [-s<n>] [-e<n>] [-vo|-vn] [-p[+|-][<n>]] [-i<f>[<n>][u]] [-o<f>[u]] [-H] polygon_infile1 [polygon_infile2 ...] polygon_outfile\n");
#include "usage.h"
}

/*------------------------------------------------------------------------------
*/
#include "parse_args.c"
