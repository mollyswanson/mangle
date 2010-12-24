/*------------------------------------------------------------------------------
© A J S Hamilton 2001
------------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "manglefn.h"
#include "defaults.h"

/* getopt options */
const char *optstr = "dqm:G:s:e:v:p:i:o:";

/* allocate polygons as a global array */
polygon *polys_global[NPOLYSMAX];

/* local functions */
void	usage(void);
#ifdef  GCC
int     grow(int npoly, polygon *[npoly], int npolys, polygon *[npolys], long double grow_angle);
#else
int     grow(int npoly, polygon *[/*npoly*/], int npolys, polygon *[/*npolys*/], long double grow_angle);
#endif


/*------------------------------------------------------------------------------
  Main program.
*/
int main(int argc, char *argv[])
{
    int ifile, nfiles, npoly, npolys,i;
    polygon **polys;
    polys=polys_global;

    /* default output format */
    fmt.out = keywords[POLYGON];
    /* default is to renumber output polygons with old id numbers */
    fmt.newid = 'o';

    /* parse arguments */
    parse_args(argc, argv);

    /* tolerance angle for multiple intersections */
    if (mtol != 0.) {
	scale(&mtol, munit, 's');
	munit = 's';
	msg("multiple intersections closer than %Lg%c will be treated as coincident\n", mtol, munit);
	scale(&mtol, munit, 'r');
	munit = 'r';
    }

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

    msg("---------------- grow ----------------\n");

    /*process grow angle */
    scale(&grow_angle, gunit, 's');
    gunit = 's';
    msg("Borders of %Lg%c will be grown around the input polygons.\n", grow_angle, gunit);
    scale(&grow_angle, gunit, 'r');
    gunit = 'r';
    
    /* check if input polygons are pixelized - for this routine we need them to *NOT* be pixelized */
    if (pixelized==1) {
      fprintf(stderr, "Error: input polygons are pixelized. The grow function can only be applied to non-pixelized polygons.");
      exit(1);
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

    /* grow polygons */
    npolys=grow(npoly, polys, NPOLYSMAX-npoly, &polys[npoly], grow_angle);
    if (npolys == -1) exit(1);

    ifile = argc - 1;
    npolys = wrmask(argv[ifile], &fmt, npolys, polys);
    if (npolys == -1) exit(1);
 
    for(i=0;i<npolys;i++){
      free_poly(polys[i]);
    }

    return(0);
}

/*------------------------------------------------------------------------------
*/
void usage(void)
{
    printf("usage:\n");
    printf("weight [-d] [-q] -z<survey> [-m<a>[u]] [-s<n>] [-e<n>] [-vo|-vn|-vp] [-p[+|-][<n>]] [-i<f>[<n>][u]] [-o<f>[u]] polygon_infile1 [polygon_infile2 ...] polygon_outfile\n");
#include "usage.h"
}

/*------------------------------------------------------------------------------
*/
#include "parse_args.c"

/*------------------------------------------------------------------------------
  Weight polygons.  ### NEED TO UPDATE DOCUMENTATION HERE

   Input: poly = array of pointers to polygons.
	  npoly = pointer to number of polygons.
	  survey = name of survey, or of filename containing list of weights.
  Output: polys = array of pointers to polygons;
  Return value: number of polygons weighted,
		or -1 if error occurred.
*/
int grow(int npoly, polygon *poly[/*npoly*/], int npolys, polygon *polys[/*npolys*/], long double grow_angle)
{
  int ipoly,iret,n,np;
  long double tol;
  
  n=npoly;
  for (ipoly = 0; ipoly < npoly; ipoly++) {
    tol=mtol;
    iret=grow_poly(&poly[ipoly], npolys-n, &polys[n], grow_angle, tol, &np);
    if(iret==-1) return(iret);
    n+=np;
  }

  for (ipoly = 0; ipoly < n; ipoly++) {
    /* assign new polygon id numbers in place of inherited ids */
    if (fmt.newid == 'n') {
      poly[ipoly]->id = ipoly;
    }      
    if (fmt.newid == 'p') {
      poly[ipoly]->id = poly[ipoly]->pixel;
    }
  }
  return(n);
}

int grow_poly(polygon **poly, int npolys, polygon *polys[/*npolys*/], long double grow_angle, long double mtol, int *np){
  int i, ip, jp, iret, ier, dn;
  long double s, cmi, cm_new,theta, theta_new, tol;
  polygon *poly1= 0x0;
  
/* part_poly should lasso all one-boundary polygons */
#define ALL_ONEBOUNDARY		2
  /* how part_poly should tighten lasso */
#define ADJUST_LASSO            2
  /* part_poly should force polygon to be split even if no part can be lassoed */
#define FORCE_SPLIT             1
  /* partition_poly should overwrite all original polygons */
#define OVERWRITE_ORIGINAL      2

  if((*poly)->pixel!=0){
    fprintf(stderr, "Error: input polygon is pixelized. The grow function can only be applied to non-pixelized polygons.");
    return(-1);
  }
  
  *np=0;
  // partition disconnected polygons
  tol = mtol;
  ier = partition_poly(poly, npolys, polys, tol, ALL_ONEBOUNDARY, ADJUST_LASSO, FORCE_SPLIT, OVERWRITE_ORIGINAL, &dn); 
  // error
  if (ier == -1) {
    fprintf(stderr, "grow: UHOH at polygon %d; continuing ...\n",polys[i]->id);
    // return(-1);
      // failed to partition polygon into desired number of parts
  } else if (ier == 1) {
    fprintf(stderr, "grow: failed to partition polygon %d fully; partitioned it into %d parts\n", (*poly)->id, dn + 1);
  }  
  *np+=dn;
  
  // check whether exceeded maximum number of polygons
  //printf("(2) n = %d\n", n);
  if (*np > npolys) {
    fprintf(stderr, "grow: total number of polygons exceeded maximum\n");
    fprintf(stderr, "if you need more space, enlarge NPOLYSMAX in defines.h, and recompile\n");
    return(-1);
  }
  
  for(i=-1; i<*np; i++){
    if(i=-1)
      poly1=(*poly);
    else
      poly1=polys[i];
    
    for (ip = 0; ip < poly1->np; ip++) { 
      cmi=poly1->cm[ip];
      //convert cm into an angle
      s = sqrtl(fabsl(cmi) / 2.);
      if (s > 1.) s = 1.;
      theta = 2. * asinl(s);
      theta=(cmi >= 0.)? theta : -theta;
      
      theta_new=theta+grow_angle;
      
      // if growing angle has caused cap to encompass whole sphere, set cm_new=2 to make it superfluous
      if(theta>=0 && theta_new>=PI)
	cm_new=2.;
      else if(theta<0 && theta_new>=0) 
	cm_new=2.;
      // if growing angle (with a negative value for grow_angle) has shrunk cap to less than nothing, make polygon null
      else if(theta>=0 && theta_new<0) 
	cm_new=0.;
      else if(theta<0 && theta_new<=-PI) 
	cm_new=0.;
      // otherwise convert normally
      else{
	/* 1 - cosl(radius) = 2 sin^2(radius/2) */
	s = sinl(theta_new / 2.);
	cm_new = s * s * 2.;
	cm_new = (theta_new >= 0.)? cm_new : -cm_new;
      }
      
      poly1->cm[ip]=cm_new;
    }
    tol=mtol;
    iret=prune_poly(poly1,tol);
  }
  return(iret);
}
  
