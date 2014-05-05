/*--------------------------------------------------------------------
(C) J C Hill 2006
--------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include "pi.h"
#include "manglefn.h"
#include "defaults.h"

/* number of extra caps to allocate to polygon, to allow for expansion */
#define DNP             4

/* getopt options */
const char *optstr = "B:dqm:a:b:t:y:s:e:v:p:i:o:HT";

/* allocate polygons as a global array */
polygon *polys_global[NPOLYSMAX];
long long raster_ids[NPOLYSMAX];

/* local functions */
void     usage(void);
#ifdef  GCC
int     rasterize(int nhealpix_poly, int npoly, polygon *[npoly], int npolys, polygon *[npolys], int nweights, long long rastid_min, long double [nweights], long long raster_ids[npolys]);
#else
int     rasterize(int nhealpix_poly, int npoly, polygon *[/*npoly*/], int npolys, polygon *[/*npolys*/], int nweights, long long rastid_min, long double [/*nweights*/], long long raster_ids[/*npolys*/]);
#endif

/*--------------------------------------------------------------------
  Main program.
*/
int main(int argc, char *argv[])
{
  int ifile, nfiles, npoly, npolys, nhealpix_poly, nhealpix_polys, j, k, nweights, nweight,npolyw;
  long long rastid_min, rastid_max;
  long double *weights;
  char *filename;
  char subfilename[1000];
  int nchars;
  char *stringbegin;
  char *stringend;

  polygon **polys;
  polys=polys_global;

  /* default output format */
  //fmt.out = keywords[HEALPIX_WEIGHT];
  fmt.out = keywords[POLYGON];
  /* default is to renumber output polygons with old id numbers */
  fmt.newid = 'o';

  /* parse arguments */
  parse_args(argc, argv);

  /* at least two input and one output filenames required as arguments */
  if (argc - optind < 3) {
      if (optind > 1 || argc - optind == 1 || argc - optind == 2) {
         fprintf(stderr, "%s requires at least 3 arguments: polygon_infile1, polygon_infile2, and polygon_outfile\n", argv[0]);
         usage();
         exit(1);
     } else {
         usage();
         exit(0);
     }
  }

  msg("---------------- rasterize ----------------\n");

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

  if (strcmp(fmt.out, "healpix_weight") == 0 && sliceordice){
    fprintf(stderr, "rasterize: ERROR:  option to write out healpix weights (-H) and option to\n");
    fprintf(stderr, "output mask polygons sliced by the rasterizer polygons (-T) are incompatible.\n");
    exit(1);
  }

  /* read polygons from polygon_infile1 (healpix pixels, or some other 'rasterizer' pixels) */
  /* the id numbers of these polygons should match the pixel numbers of this pixelization scheme;
     for example, if you are using HEALPix, the id numbers should match the HEALPix pixel numbers
     in the NESTED scheme */
  nhealpix_poly = 0;
  ifile = optind;
  nhealpix_polys = rdmask(argv[ifile], &fmt, NPOLYSMAX - nhealpix_poly, &polys[nhealpix_poly], 1);
  if (nhealpix_polys == -1) exit(1);
  nhealpix_poly += nhealpix_polys;

  if (nhealpix_poly == 0) {
     msg("STOP\n");
     exit(0);
  }

  /* Input rasterizer polygons need not be balkanized if they are non-overlapping by construction,
     which is the case for the HEALPix polygons.  This is a special case - all other mangle functions
     that require balkanization require all input files to be balkanized.  To avoid getting an error
     here, increment the 'balkanized' counter here if the rasterizer polygons are not balkanized. */
  if (balkanized == 0) {
    balkanized++;
  }

  /* find maximum id number in rasterizer file*/
  rastid_max = 0;
  for (k = 0; k < nhealpix_poly; k++) {
    if (polys[k]->id >= rastid_max) rastid_max = polys[k]->id;
  }

  /* find minimum id number in rasterizer file*/
  rastid_min=rastid_max;
  for (k = 0; k < nhealpix_poly; k++) {
    if (polys[k]->id <= rastid_min) rastid_min = polys[k]->id;
  }

  /* set nweights equal to max id in rasterizer file - min id in rasterizer file plus 1*/
  nweights=rastid_max-rastid_min+1;
  
  /* read polygons from polygon_infile2, polygon_infile3, etc. */
  npoly = nhealpix_poly;
  nfiles = argc - 2 - optind;
  for (ifile = optind + 1; ifile < optind + 1 + nfiles; ifile++) {
      npolys = rdmask(argv[ifile], &fmt, NPOLYSMAX - npoly, &polys[npoly], 1);
      if (npolys == -1) exit(1);
      npoly += npolys;      
  }
  if (nfiles >= 2) {
    msg("total of %d polygons read from mask files\n", npoly-nhealpix_poly);
  }
  if (npoly-nhealpix_poly == 0) {
    msg("STOP\n");
    exit(0);
  }

  /*only check for snapped and balkanized if averaging within rasterizer polygons - if slicing input polygons
    into the rasterizer polygons (sliceordice=1) it doesn't matter if input is snapped or balkanized.*/ 
  if(!sliceordice){
    if (snapped==0 || balkanized==0) {
      fprintf(stderr, "Error: input polygons must be snapped and balkanized before rasterization.\n");
      fprintf(stderr, "If your polygons are already snapped and balkanized, add the 'snapped' and\n'balkanized' keywords at the beginning of each of your input polygon files.\n");
      exit(1);
    }
  }

  /* allocate memory for weights array */
  weights = (long double *) malloc(sizeof(long double) * (nweights));
  if (!weights) {
     fprintf(stderr, "rasterize: failed to allocate memory for %d long doubles\n", nweights);
     exit(1);
  }

  /* initialize weights array to 0 */
  for (k = 0; k < nweights; k++) weights[k] = 0.;

  /* rasterize */
  npolys = rasterize(nhealpix_poly, npoly, polys, NPOLYSMAX - npoly, &polys[npoly], nweights, rastid_min, weights,raster_ids);
  if (npolys == -1) exit(1);

  if(!sliceordice){
    /* copy new weights to original rasterizer polygons */
    for (k = 0; k < nhealpix_poly; k++) {
      for (j = 0; j < nweights; j++) {
	if (polys[k]->id == (long long)j+rastid_min) {
	  polys[k]->weight = weights[j];
	  break;
	}
      }
    }
  }

  ifile = argc - 1;
  if (strcmp(fmt.out, "healpix_weight") == 0) {
    npolys = wr_healpix_weight(argv[ifile], &fmt, nweights, weights);
    if (npolys == -1) exit(1);
  }
  else if (strcmp(fmt.out, "dpolygon") == 0) {
    npolyw = discard_poly(npolys, &polys[npoly]);
    npolys = wr_dpoly(argv[ifile], &fmt, npolys, &polys[npoly],npolyw,raster_ids);
    if (npolys == -1) exit(1);

    filename=argv[ifile];
    
    // strip off any leading directories and extensions to get the base filename to use for the individual subfiles
    stringbegin=strrchr(filename,'/');
    if(!stringbegin){
      stringbegin=&filename[0];
    } else {
      stringbegin++;
    }  
    stringend=strrchr(filename,'.');
    if(!stringend){
      stringend=&filename[0]+strlen(filename);
    } 
    nchars=(int)(stringend-stringbegin);
    sprintf(subfilename, "%s/%.*s_index.pol",filename,nchars,stringbegin);

    //write rasterizer polygons as index file
    fmt.out="polygon";
    npolys = wrmask(subfilename, &fmt, nhealpix_poly, polys);
    if (npolys == -1) exit(1);
  }
  else {
    if(sliceordice){
      npolys = wrmask(argv[ifile], &fmt, npolys, &polys[npoly]);
      if (npolys == -1) exit(1);
    }
    else{
      npolys = wrmask(argv[ifile], &fmt, npolys, polys);
      if (npolys == -1) exit(1);
    }
  }
  
  /* free array */
  for(k = 0; k < npoly; k++){
    free_poly(polys[k]);
  }
  free(weights);
  return(0);
}

/*-------------------------------------------------------------------------
*/
void usage(void)
{
     printf("usage:\n");
     printf("rasterize [-d] [-q] [-m<a>[u]] [-s<n>] [-a<a>[u]] [-b<a>[u]] [-t<a>[u]] [-y<r>] [-e<n>] [-vo|-vn] [-p[+|-][<n>]] [-i<f>[<n>][u]] [-o<f>[u]] [-H] [-T] polygon_infile1 polygon_infile2 [polygon_infile3 ...] polygon_outfile\n");
#include "usage.h"
}

/*-------------------------------------------------------------------------
*/
#include "parse_args.c"

/*-------------------------------------------------------------------------
  Rasterize a mask of input polygons against a mask of rasterizer polygons.

  Input: nhealpix_poly = number of rasterizer polygons.
         npoly = total number of polygons in input array.
	 poly = array of pointers to polygons.
	 nweights = number of weights in output array.
  Output: weights = array of rasterizer weights.
  Return value: number of weights in array,
                or -1 if error occurred.
*/

int rasterize(int nhealpix_poly, int npoly, polygon *poly[/*npoly*/], int npolys, polygon *polys[/*npolys*/], int nweights, long long rastid_min, long double weights[/*nweights*/],long long raster_ids[/*npolys*/])
{
#define WARNMAX                 0

  int min_pixel, max_pixel, ier, ier_h, ier_i, i, j,k, ipix, ipoly, begin_r, end_r, begin_m, end_m, verb, np, iprune,n,selfsnap,nadj;
  int *start_r, *start_m, *total_r, *total_m;
  long double *areas, area_h, area_i, tol;
  polygon *rasterizer_and_poly[2];
  char snapped_polys[2];
  static polygon *polyint = 0x0;
  
  if(!sliceordice){
    /* make sure weights are all zero for rasterizer pixels */
    for (i = 0; i < nhealpix_poly; i++) {
      poly[i]->weight = 0.;
    }
  }
  
  /* allocate memory for rasterizer areas array */
  areas = (long double *) malloc(sizeof(long double) * (nweights));
  if (!areas) {
    fprintf(stderr, "rasterize: failed to allocate memory for %d long doubles\n", nweights);
    exit(1);
  }
  
  /* initialize rasterizer areas array to 0 */
  for (i = 0; i < nweights; i++) areas[i] = 0.;

  /* initialize rasterizer ids array to 0 */
  for (i = 0; i < npolys; i++) raster_ids[i] = 0;

  /* allow error messages from garea */
  verb = 1;
  
  if(!sliceordice){ 
    /* find areas of rasterizer pixels for later use */
    for (i = 0; i < nweights; i++) {
      for (j = 0; j < nhealpix_poly; j++) {
	if (poly[j]->id == (long long)i+rastid_min) {
	  tol = mtol;
	  ier_h = garea(poly[j], &tol, verb, &area_h);
	  if (ier_h == 1) {
	    fprintf(stderr, "fatal error in garea\n");
	    exit(1);
	  }
	  if (ier_h == -1) {
	    fprintf(stderr, "failed to allocate memory in garea\n");
	    exit(1);
	  }
	  areas[i] += area_h;
	}
      }
    }
  }

  /* sort arrays by pixel number */
  poly_sort(nhealpix_poly, poly, 'p');
  poly_sort(npoly-nhealpix_poly, &(poly[nhealpix_poly]), 'p');

  /* allocate memory for pixel info arrays start_r, start_m, total_r, and total_m */
  min_pixel = poly[0]->pixel;
  max_pixel = (poly[nhealpix_poly-1]->pixel+1>poly[npoly-1]->pixel+1)?(poly[nhealpix_poly-1]->pixel+1):(poly[npoly-1]->pixel+1);
  start_r = (int *) malloc(sizeof(int) * max_pixel);
  if (!start_r) {
    fprintf(stderr, "rasterize: failed to allocate memory for %d integers\n", max_pixel);
    return(-1);
  }
  start_m = (int *) malloc(sizeof(int) * max_pixel);
  if (!start_m) {
    fprintf(stderr, "rasterize: failed to allocate memory for %d integers\n", max_pixel);
    return(-1);
  }
  total_r = (int *) malloc(sizeof(int) * max_pixel);
  if (!total_r) {
    fprintf(stderr, "rasterize: failed to allocate memory for %d integers\n", max_pixel);
    return(-1);
  }
  total_m = (int *) malloc(sizeof(int) * max_pixel);
  if (!total_m) {
    fprintf(stderr, "rasterize: failed to allocate memory for %d integers\n", max_pixel);
    return(-1);
  }

  /* build lists of starting indices of each pixel and total number of polygons in each pixel */
  ier = pixel_list(nhealpix_poly, poly, max_pixel, start_r, total_r);
  if (ier == -1) {
    fprintf(stderr, "rasterize: error building pixel index lists for rasterizer polygons\n");
    return(-1);
  }

  ier = pixel_list(npoly-nhealpix_poly, &(poly[nhealpix_poly]), max_pixel, start_m, total_m);
  if (ier == -1) {
    fprintf(stderr, "rasterize: error building pixel index lists for input mask polygons\n");
    return(-1);
  }

  /* correction due to the start_m array's offset */
  for (i = min_pixel; i < max_pixel; i++) {
    start_m[i] += nhealpix_poly;
  }

  j=0;

  /* compute intersection of each input mask polygon with each rasterizer polygon */
  for (ipix = min_pixel; ipix < max_pixel; ipix++) {
    begin_r = start_r[ipix];
    end_r = start_r[ipix] + total_r[ipix];
    begin_m = start_m[ipix];
    end_m = start_m[ipix] + total_m[ipix];

    for (ipoly = begin_m; ipoly < end_m; ipoly++) {
      /* disregard any null polygons */
      if (!poly[ipoly]) continue;

      for (i = begin_r; i < end_r; i++) {

	/* make sure polyint contains enough space for intersection */
	np = poly[ipoly]->np + poly[i]->np;
	ier = room_poly(&polyint, np, DNP, 0);
	if (ier == -1) goto out_of_memory;

	    //snap edges of mask polygon to rasterizer
	rasterizer_and_poly[0]=poly[i];
	rasterizer_and_poly[1]=poly[ipoly];
	selfsnap = 0;
	nadj = snap_polys(&fmt, 2, rasterizer_and_poly, selfsnap, axtol, btol, thtol, ytol, mtol, WARNMAX, snapped_polys);
	if(nadj==-1){
	  msg("rasterize: error snapping mask and rasterizer polygons together\n");
	  return(-1);
	}

	poly_poly(poly[ipoly], poly[i], polyint);

	/* suppress coincident boundaries, to make garea happy */
	iprune = trim_poly(polyint);

	/* intersection of poly[ipoly] and poly[i] is null polygon */
	if (iprune >= 2) area_i = 0.;

	else {
	  tol = mtol;
	  ier_i = garea(polyint, &tol, verb, &area_i);
	  if (ier_i == 1) {
	    fprintf(stderr, "fatal error in garea\n");
	    return(-1);
	  }
	  if (ier_i == -1) {
	    fprintf(stderr, "failed to allocate memory in garea\n");
	    return(-1);
	  }
	}

	/*if the "slicing" option is selected, write the intersection polygon into the output array */
	if(area_i!=0 && sliceordice){
	  tol = mtol;
	  iprune = prune_poly(polyint, tol);
	  if (iprune == -1) {
	    fprintf(stderr, "rasterize: failed to prune polygon %lld; continuing ...\n", (fmt.newid == 'o')? polys[i]->id : (long long)j+fmt.idstart);
	    /* return(-1); */
	  }
	  if (iprune >= 2) {
	    fprintf(stderr, "rasterize: polygon %lld is NULL; continuing ...\n", (fmt.newid == 'o')? polys[i]->id : (long long)j+fmt.idstart);
	  } 
	  else {
	    /* make sure output polygon contains enough space */
	    np = polyint->np;
	    ier = room_poly(&polys[j], np, DNP, 0);
	    if (ier == -1) goto out_of_memory;
	  
	    /* copy intersection into poly1 */
	    copy_poly(polyint, polys[j]);
	    /* set raster_id for new polygon equal to id of current rasterizer polygon */ 
	    raster_ids[j]=poly[i]->id;
	  /* if output id number option = p, set id number equal to id number of rasterizer polygon*/
	    if (fmt.newid == 'p') {
	      polys[j]->id = poly[i]->id;
	    }	  
	    /* set weight according to balkanization scheme: */
	    if(bmethod=='l'){
	      //do nothing - this is the default behavior
	    }
	    else if(bmethod=='a'){
	      polys[j]->weight=polys[j]->weight + poly[i]->weight;
	    }
	    else if(bmethod=='n'){
	      polys[j]->weight=(polys[j]->weight > poly[i]->weight)? poly[i]->weight : polys[j]->weight ;
	    }
	    else if(bmethod=='x'){
	      polys[j]->weight=(polys[j]->weight > poly[i]->weight)? polys[j]->weight : poly[i]->weight ;
	    }
	    else{
	      fprintf(stderr, "error in fragment_poly: balkanize method %c not recognized.\n", bmethod);
	      return(-1);
	    }
	    j++;
	  }
	}
	if(!sliceordice){
	  k=(int)((poly[i]->id)-rastid_min);
	  weights[k] += (area_i)*(poly[ipoly]->weight);
	}
      }
    }
  }

  if(!sliceordice){
    for (i=0; i<nweights; i++) {
      if(areas[i]!=0){
	weights[i] = weights[i]/areas[i];
      }
      else{
	weights[i]=0;
	if (strcmp(fmt.out, "healpix_weight") == 0) {
	  fprintf(stderr,"WARNING: rasterize: area of rasterizer polygon %d is zero.  Assigning zero weight.\n",i);
	}
      }
    }
  }

  n=sliceordice ? j : nhealpix_poly;

  /* assign new polygon id numbers in place of inherited ids */
  if (fmt.newid == 'n') {
    for (i = 0; i < n; i++) {
      if(sliceordice) polys[i]->id = (long long)i+fmt.idstart; else poly[i]->id = (long long)i+fmt.idstart;
    }
  }
  
  if (fmt.newid == 'p' && !sliceordice) {
    for (i = 0; i < n; i++) {
      poly[i]->id =(long long)poly[i]->pixel;
    }
  }
  

  free(start_r);
  free(start_m); 
  free(total_r);
  free(total_m);
  free(areas);
 
  return(n);


  /* ----- error return ----- */
  out_of_memory:
  fprintf(stderr, "rasterize: failed to allocate memory for polygon of %d caps\n", np + DNP);
  return(-1);

}
