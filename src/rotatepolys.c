/*------------------------------------------------------------------------------
© M E C Swanson 2012
------------------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "inputfile.h"
#include "manglefn.h"
#include "defaults.h"

/* getopt options */
const char *optstr = "dqf:s:e:v:p:i:o:";

/* allocate polygons as a global array */
polygon *polys_global[NPOLYSMAX];

/* local functions */
void	usage(void);
#ifdef	GCC
int	rotatepolys(int npoly, polygon *[npoly], long double, long double, long double);
#else
int	rotatepolys(int npoly, polygon *[/*npoly*/], long double, long double, long double);
#endif

/*------------------------------------------------------------------------------
  Main program.
*/
int main(int argc, char *argv[])
{
  int ifile, nfiles, npoly, npolys,i,itr;
    polygon **polys;
    polys=polys_global;

    /* default output format */
    fmt.out = keywords[POLYGON];

    /* parse arguments */
    parse_args(argc, argv);
 /* parse option <fopt> to -f<fopt> */
    if (fopt) itr = parse_fopt();

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

    msg("---------------- rotatepolys ----------------\n");

    /* advise data format */
    advise_fmt(&fmt);

    /* read polygons */
    npoly = 0;
    nfiles = argc - 1 - optind;
    for (ifile = optind; ifile < optind + nfiles; ifile++) {
        npolys = rdmask(argv[ifile], &fmt, NPOLYSMAX - npoly, &polys[npoly], 1);
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

    /* weight polygons */
    npoly = rotatepolys(npoly, polys, fmt.azn,fmt.eln,fmt.azp);
    pixelized=0;

    ifile = argc - 1;
    npoly = wrmask(argv[ifile], &fmt, npoly, polys);
    if (npoly == -1) exit(1);

    for(i=0;i<npoly;i++){
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
*/
#include "parse_fopt.c"

/*------------------------------------------------------------------------------

  NEED TO UPDATE DOCUMENTATION HERE
  Weight polygons.

   Input: poly = array of pointers to polygons.
	  npoly = pointer to number of polygons.
	  survey = name of survey, or of filename containing list of weights.
  Output: polys = array of pointers to polygons;
  Return value: number of polygons weighted,
		or -1 if error occurred.
*/
int rotatepolys(int npoly, polygon *poly[/*npoly*/], long double azn,long double eln, long double azp)
{
#define AZEL_STR_LEN	32

  int ipoly, ip,j;
  long double cpsi,spsi,ctheta,stheta,cphi,sphi,xold,yold,zold,r;
  //rows of rotation matrix
  vec row0,row1, row2;
  char az_str[AZEL_STR_LEN], el_str[AZEL_STR_LEN];
  azel vf;


  msg("Rotating polygons to new frame: azn=%Lf,eln=%Lf,azp=%Lf\n",azn,eln,azp);
  if(pixelized==1){
    msg("NOTE: rotating polygons removes pixelization info - to repixelize in new frame, run pixelize again.\n");
  }

  spsi=sinl((90.+azn)*DEGREE/RADIAN);
  cpsi=cosl((90.+azn)*DEGREE/RADIAN);
  stheta=sinl((90.-eln)*DEGREE/RADIAN);
  ctheta=cosl((90.-eln)*DEGREE/RADIAN);
  sphi=sinl((90.-azp)*DEGREE/RADIAN);
  cphi=cosl((90.-azp)*DEGREE/RADIAN);

  row0[0]=cphi*cpsi-sphi*ctheta*spsi;
  row0[1]=cphi*spsi+sphi*ctheta*cpsi;
  row0[2]=sphi*stheta;
  row1[0]=-sphi*cpsi-cphi*ctheta*spsi;
  row1[1]=-sphi*spsi+cphi*ctheta*cpsi;
  row1[2]=cphi*stheta;
  row2[0]=stheta*spsi;
  row2[1]=-stheta*cpsi;
  row2[2]=ctheta;
  
  for (ipoly = 0; ipoly < npoly; ipoly++) {
    poly[ipoly]->pixel=0;
    for (ip = 0; ip < poly[ipoly]->np; ip++) {
      /*
      rp_to_azel(poly[ipoly]->rp[ip],&vf);
      scale_azel(&vf,'r','d');
      wrangle(vf.az, 'd', -1, AZEL_STR_LEN, az_str);
      wrangle(vf.el, 'd', -1, AZEL_STR_LEN, el_str);
      printf("old: %s %s\n", az_str, el_str);
      */

      xold=poly[ipoly]->rp[ip][0];
      yold=poly[ipoly]->rp[ip][1];
      zold=poly[ipoly]->rp[ip][2];
      poly[ipoly]->rp[ip][0]=row0[0]*xold+row0[1]*yold+row0[2]*zold;
      poly[ipoly]->rp[ip][1]=row1[0]*xold+row1[1]*yold+row1[2]*zold;
      poly[ipoly]->rp[ip][2]=row2[0]*xold+row2[1]*yold+row2[2]*zold;

      //make sure x,y,z is still exactly a unit vector
      r = 0.;
      for (j = 0; j < 3; j++) r += poly[ipoly]->rp[ip][j] * poly[ipoly]->rp[ip][j];
      if (r != 1.) {
	r = sqrtl(r);
	for (j = 0; j < 3; j++) poly[ipoly]->rp[ip][j] /= r;
      }	

      /*
      rp_to_azel(poly[ipoly]->rp[ip],&vf);
      scale_azel(&vf,'r','d');
      wrangle(vf.az, 'd', -1, AZEL_STR_LEN, az_str);
      wrangle(vf.el, 'd', -1, AZEL_STR_LEN, el_str);
      printf("new: %s %s\n", az_str, el_str);
      */

    }
  }

  return(npoly);
}
