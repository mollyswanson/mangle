/*--------------------------------------------------------------------
(C) James C Hill 2006
--------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include "pi.h"
#include "manglefn.h"
#include "defaults.h"

/* getopt options */
const char *optstr = "dqBa:b:t:y:m:s:e:p:i:";

/* local functions */
void     usage(void);

/*--------------------------------------------------------------------
  Main program.
*/
int main(int argc, char *argv[])
{
  const int per = 0;
  const int nve = 2;
  const int do_vcirc = 0;
  int ifile, ipoly, nfiles, npoly, npolys, nhealpix_poly, nhealpix_polys, nrast_poly, nrast_polys, i, j, verb, ier, imid, ivm, nev, nev0, nv, nvm, ier_r, ier_h, ipnest, nside;
  int *ipv, *gp, *ev;
  double *weights, *angle, area_r, area_h, tol;
  vec *ve, *vm;
  azel v;
  char *filename, key;
  FILE *file;
  
  polygon *polys[NPOLYSMAX], *healpix_polys[NPOLYSMAX], **rast_polys, **rasterized_polys, *healpix_poly_j;

  /* default output format */
  fmt.out = keywords[HEALPIX_WEIGHT];

  printf("NPOLYSMAX = %d\n", NPOLYSMAX);

  /* parse arguments */
  parse_args(argc, argv);

  /* at least two input and one output filenames required as arguments */
  if (argc - optind < 3) {
      if (optind > 1 || argc - optind == 1 || argc - optind == 2) {
         fprintf(stderr, "%s requires at least 3 arguments: polygon_infile1, polygon_infile2, and healpix_weight_outfile\n", argv[0]);
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

  /* advise data format */
  advise_fmt(&fmt);

  /* read polygons from polygon_infile2, polygon_infile3, etc. for first balkanization */
  npoly = 0;
  nfiles = argc - 2 - optind;
  for (ifile = optind + 1; ifile < optind + 1 + nfiles; ifile++) {
      npolys = rdmask(argv[ifile], &fmt, NPOLYSMAX - npoly, &polys[npoly]);
      if (npolys == -1) exit(1);
      npoly += npolys;
  }
  if (nfiles >= 2) {
    msg("total of %d polygons read from mask files\n", npoly);
  }
  if (npoly == 0) {
    msg("STOP\n");
    exit(0);
  }

  if (!pre_balk) {
     /* balkanize polygons */
     tol = mtol;
     npolys = balkanize(npoly, polys, NPOLYSMAX - npoly, &polys[npoly], tol, &fmt, axtol, btol, thtol, ytol);
     if (npolys == -1) exit(1);
  }

  else if (pre_balk) {
     msg("mask files (polygon_infile2, polygon_infile3,...) already balkanized... skipping this step\n");
  }

  /* read polygons from polygon_infile1 for rasterize balkanization */
  nhealpix_poly = 0;
  ifile = optind;
  nhealpix_polys = rdmask(argv[ifile], &fmt, NPOLYSMAX - nhealpix_poly, &healpix_polys[nhealpix_poly]);
  if (nhealpix_polys == -1) exit(1);
  nhealpix_poly += nhealpix_polys;

  if (nhealpix_poly == 0) {
     msg("STOP\n");
     exit(0);
  }

  //printf("fmt.nweights = %d\n", fmt.nweights);
  nside = get_nside(fmt.nweights);
  //printf("nside = %d\n", nside);
  
  /* make sure weights are all zero for healpix_polys[] */
  for (i = 0; i < nhealpix_poly; i++) {
      healpix_polys[i]->weight = 0.;
  }

  /* assign new id numbers to mask polygons for upcoming rasterize */
  if (!pre_balk) {
     for (i = npoly; i < npoly + npolys; i++) {
         polys[i]->id = nhealpix_poly + (i - npoly);
     } 
  }
  else if (pre_balk) {
     for (i = 0; i < npoly; i++) polys[i]->id = nhealpix_poly + i;
     }

  if (!pre_balk) nrast_poly = nhealpix_poly + npolys;
  else if (pre_balk) nrast_poly = nhealpix_poly + npoly;

  /* allocate memory for rast_polys and rasterized_polys arrays */
  rast_polys = (polygon **) malloc(sizeof(polygon) * nrast_poly);
  if (!rast_polys) {
     fprintf(stderr, "rasterize: failed to allocate memory for %d polygons\n", nrast_poly);
     exit(1);
  }

  rasterized_polys = (polygon **) malloc(sizeof(polygon) * (NPOLYSMAX - nrast_poly));
  if (!rasterized_polys) {
     fprintf(stderr, "rasterize: failed to allocate memory for %d polygons\n", NPOLYSMAX - nrast_poly);
     exit(1);
  }

  /* initialize rast_polys and rasterized_polys arrays */
  for (i = 0; i < nrast_poly; i++) rast_polys[i] = 0x0;
  for (i = 0; i < NPOLYSMAX - nrast_poly; i++) rasterized_polys[i] = 0x0;

  /* put polygons into one array for rasterizing */
  for (i = 0; i < nhealpix_poly; i++) {
      rast_polys[i] = healpix_polys[i];
      rast_polys[i]->np = healpix_polys[i]->np;
      rast_polys[i]->pixel = healpix_polys[i]->pixel;
      rast_polys[i]->id = healpix_polys[i]->id;
      rast_polys[i]->weight = healpix_polys[i]->weight;
  }
  if (!pre_balk) {
     j = 0;
     for (i = nhealpix_poly; i < nrast_poly; i++) {
	 rast_polys[i] = polys[npoly + j];
	 rast_polys[i]->np = polys[npoly + j]->np;
	 rast_polys[i]->pixel = polys[npoly + j]->pixel;
	 rast_polys[i]->id = polys[npoly + j]->id;
	 rast_polys[i]->weight = polys[npoly + j]->weight;
	 j++;
     }
  }
  else if (pre_balk) {
     j = 0;
     for (i = nhealpix_poly; i < nrast_poly; i++) {
	 rast_polys[i] = polys[j];
	 rast_polys[i]->np = polys[j]->np;
	 rast_polys[i]->pixel = polys[j]->pixel;
	 rast_polys[i]->id = polys[j]->id;
	 rast_polys[i]->weight = polys[j]->weight;
	 j++;
     }
  }

  /* sort array before balkanizing */
  key = 'p';
  poly_sort(nrast_poly, rast_polys, key);

  //for(ipoly=0;ipoly<nrast_poly;ipoly++) printf("rast_polys[%d]->id = %d, pixel = %d\n", ipoly, rast_polys[ipoly]->id, rast_polys[ipoly]->pixel);

  /* rasterize polygons (i.e., balkanize polys[] against healpix_polys[]) */
  tol = mtol;
  nrast_polys = balkanize(nrast_poly, rast_polys, NPOLYSMAX - nrast_poly, &rasterized_polys[0], tol, &fmt, axtol, btol, thtol, ytol);

  //for(ipoly=0;ipoly<nrast_polys;ipoly++) printf("rasterized_polys[%d]->id = %d, pixel = %d\n", ipoly, rasterized_polys[ipoly]->id, rasterized_polys[ipoly]->pixel);

  /* find HEALPix parent pixel of each balkanized polygon by finding a point within each polygon
     and then using the HEALPix utility ang2pix_nest */
  for (ipoly = 0; ipoly < nrast_polys; ipoly++) {
      /* discard null polygons */
      if (!rasterized_polys[ipoly]) continue;
  
      /* point somewhere in the middle of the polygon */
      tol = mtol;
      ier = gverts(rasterized_polys[ipoly], do_vcirc, &tol, per, nve, &nv, &ve, &angle, &ipv, &gp, &nev, &nev0, &ev);
      if (ier == -1) {
	 fprintf(stderr, "rasterize: failed to allocate memory in gverts\n");
	 exit(1);
      }
      tol = mtol;
      imid = vmid(rasterized_polys[ipoly], tol, nv, nve, ve, ipv, ev, &nvm, &vm);
      if (imid == -1) {
	 fprintf(stderr, "rasterize: failed to allocate memory in vmid\n");
	 exit(1);
      }
      /* check found a point inside the polygon */
      imid = 0;
      for (ivm = 0; ivm < nvm; ivm++) {
	  if (vm[ivm][0] != 0. || vm[ivm][1] != 0. || vm[ivm][2] != 0.) {
	     imid = 1.;
	     if (ivm > 0) for (i = 0; i < 3; i++) vm[0][i] = vm[ivm][i];
	     break;
	  }
      }
      /* found a point */
      if (imid == 1) {
	 rp_to_azel(vm[0], &v);
	 /* convert angles to HEALPix frame */
	 if (v.az < 0.) v.az += TWOPI;
	 v.el = PIBYTWO - v.el;
      }

      healpix_ang2pix_nest(nside, v.el, v.az, &ipnest);

      rasterized_polys[ipoly]->id = ipnest;
  }

  /* allocate memory for weights array */
  weights = (double *) malloc(sizeof(double) * (fmt.nweights));
  if (!weights) {
     fprintf(stderr, "rasterize: failed to allocate memory for %d doubles\n", fmt.nweights);
     exit(1);
  }

  /* initialize weights array to 0 */
  for (i = 0; i < fmt.nweights; i++) weights[i] = 0.;

  /* allow error messages from garea */
  verb = 1;

  //area_h = (4*PI)/(double)(fmt.nweights);

  /* compute weighted average (by area) of all weights in each HEALPix polygon */
  for (j = 0; j < fmt.nweights; j++) {
      for (ipoly = 0; ipoly < nrast_polys; ipoly++) {
	  if (rasterized_polys[ipoly]->id == j) {
	     tol = mtol;
	     ier_r = garea(rasterized_polys[ipoly], &tol, verb, &area_r);
	     if (ier_r == 1) {
	        fprintf(stderr, "fatal error in garea\n");
		exit(1);
	     }
	     if (ier_r == -1) {
	       fprintf(stderr, "failed to allocate memory in garea\n");
	       exit(1);
	     }
	     weights[j] += (rasterized_polys[ipoly]->weight)*(area_r);
	  }
      }
      healpix_poly_j = get_healpix_poly(nside, j);
      tol = mtol;
      ier_h = garea(healpix_poly_j, &tol, verb, &area_h);
      if (ier_h == 1) {
         fprintf(stderr, "fatal error in garea\n");
         exit(1);
      }
      if (ier_h == -1) {
         fprintf(stderr, "failed to allocate memory in garea\n");
         exit(1);
      }
      weights[j] = weights[j]/(area_h);
  }

  /* write weights */
  ifile = argc - 1;
  filename = argv[ifile];
  file = fopen(filename, "w");
  if (!file) {
     fprintf(stderr, "cannot open %s for writing\n", filename);
     return(-1);
  }
  
  fprintf(file, "healpix_weight %d\n", fmt.nweights);

  for (j = 0; j < fmt.nweights; j++) {
    fprintf(file, "%19.16f\n", weights[j]);
  }

  msg("%d weights written to %s\n", fmt.nweights, filename);

  fclose(file);

  /* free arrays */
  for (i = 0; i < nrast_polys; i++) free_poly(rasterized_polys[i]);
  for (i = 0; i < nrast_poly; i++) free_poly(rast_polys[i]);
  if (!pre_balk) for (i = 0; i < npoly; i++) free_poly(polys[i]);
  else ;

  return(0);
}

/*-------------------------------------------------------------------------
*/
void usage(void)
{
     printf("usage:\n");
     printf("rasterize [-d] [-q] [-B] [-a<a>[u]] [-b<a>[u]] [-t<a>[u]] [-y<r>] [-m<a>[u]] [-s<n>] [-e<n>] [-vo|-vn] [-p[+|-][<n>]] [-i<f>[<n>][u]] polygon_infile1 polygon_infile2 [polygon_infile3 ...] weight_outfile\n");
#include "usage.h"
}

/*-------------------------------------------------------------------------
*/
#include "parse_args.c"
