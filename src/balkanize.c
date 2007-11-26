/*------------------------------------------------------------------------------
© A J S Hamilton 2001
------------------------------------------------------------------------------*/
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "manglefn.h"
#include "defaults.h"

/* define CARRY_ON_REGARDLESS if you want balkanize() to continue even when the number of polygons hits NPOLYSMAX;
   if CARRY_ON_REGARDLESS is defined, then balkanize() will create a possibly incomplete polygon file of polygons */
#undef	CARRY_ON_REGARDLESS
//#define	CARRY_ON_REGARDLESS

/* getopt options */
//const char *optstr = "dqa:b:t:y:m:s:e:v:p:i:o:";
const char *optstr = "dqm:s:e:v:p:i:o:";

/* local functions */
void	usage(void);

/*------------------------------------------------------------------------------
  Main program.
*/
int main(int argc, char *argv[])
{
    int ifile, nfiles, npoly, npolys;
    clock_t start, stop;
    polygon *polys[NPOLYSMAX];
    char key;

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

    msg("---------------- balkanize ----------------\n");
    
    // snap angles 
    scale(&axtol, axunit, 's');
    scale(&btol, bunit, 's');
    scale(&thtol, thunit, 's');
    axunit = 's';
    bunit = 's';
    thunit = 's';
    // msg("snap angles: axis %Lg%c latitude %Lg%c edge %Lg%c\n", axtol, axunit, btol, bunit, thtol, thunit);
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

    key = 'p';
    poly_sort(npoly, polys, key);

    start=clock();

    /* balkanize polygons */
    npolys = balkanize(npoly, polys, NPOLYSMAX - npoly, &polys[npoly], mtol, &fmt, axtol, btol, thtol, ytol);
    if (npolys == -1) exit(1);

    stop=clock();

     msg("balkanization took %Lg seconds.\n", (long double)(stop-start)/CLOCKS_PER_SEC);
    /* write polygons */
    ifile = argc - 1;
    npolys = wrmask(argv[ifile], &fmt, npolys, &polys[npoly]);
    if (npolys == -1) exit(1);
    /* memmsg(); */

    return(0);
}

/*------------------------------------------------------------------------------
*/
void usage(void)
{
    printf("usage:\n");
    printf("balkanize [-d] [-q] [-a<a>[u]] [-b<a>[u]] [-t<a>[u]] [-y<r>] [-m<a>[u]] [-s<n>] [-e<n>] [-vo|-vn] [-p[+|-][<n>]] [-i<f>[<n>][u]] [-o<f>[u]] polygon_infile1 [polygon_infile2 ...] polygon_outfile\n");
#include "usage.h"
}

/*------------------------------------------------------------------------------
*/
#include "parse_args.c"
