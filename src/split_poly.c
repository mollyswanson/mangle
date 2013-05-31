/*------------------------------------------------------------------------------
© A J S Hamilton 2001
------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <math.h>
#include "manglefn.h"

/* number of extra caps to allocate to polygon, to allow for expansion */
#define DNP		4

/*------------------------------------------------------------------------------
  If poly1 overlaps poly2, split poly1 into two parts.

  If poly3 is null on input, then the appropriate return value is returned,
  but poly1 is not actually split.

   Input: *poly1, poly2 are 2 polygons.
	  mtol = initial angular tolerance within which to merge multiple intersections.
  Output: If **poly3 is not null on input, then:
	      *poly1 and *poly3 are 2 split polygons of poly1, if poly1 is split,
		    with *poly1 the part outside poly2,
		    and *poly3 the part intersecting poly2;
	      *poly1 and *poly3 remain untouched if poly1 is not split.
	  If **poly3 is null on input, then *poly1 remains untouched.
  Return value: -1 = error occurred;
		0 = poly1 and poly2 have zero intersection;
		1 = poly2 fully encloses poly1;
		2 = poly2 splits poly1 into two.
*/
int split_poly(polygon **poly1, polygon *poly2, polygon **poly3, long double mtol)
{
    static polygon *poly = 0x0, *poly4 = 0x0;

    int ier, ip, iprune, np, np1, verb;
    long double area, area_tot, cm, tol,area1,area3,area4;

    /* poly2 is whole sphere, therefore contains poly1 */
    if (poly2->np == 0){
      return(1);
    }
    
    /* make sure poly contains enough space for intersection */
    np = (*poly1)->np + poly2->np;
    ier = room_poly(&poly, np, DNP, 0);
    if (ier == -1) goto out_of_memory;

    /* intersection of poly1 and poly2 */
    poly_poly(*poly1, poly2, poly);

    /* suppress coincident boundaries, to make garea happy */
    iprune = trim_poly(poly);

    /* intersection of poly1 and poly2 is null polygon */
    if (iprune >= 2) return(0);

    /* area of intersection */
    tol = mtol;
    verb = 1;
    ier = garea(poly, &tol, verb, &area_tot);
    if (ier) goto error;

    /* poly1 and poly2 have zero intersection */
    if (area_tot == 0.) return(0);

    /* area of poly1 */
    tol = mtol;
    verb = 1;
    ier = garea(*poly1, &tol, verb, &area1);
    if (ier) goto error;

    /* number of caps of poly1 */
    np1 = (*poly1)->np; 

    /* find boundary of poly2 which intersects poly1 */
    verb = 0;

    for (ip = 0; ip < poly2->np; ip++) {

	cm = poly->cm[np1 + ip];
	poly->cm[np1 + ip] = 2.;		/* suppress boundary to be tested */
	tol = mtol;
	ier = garea(poly, &tol, verb, &area);	/* area of intersection sans boundary */
	poly->cm[np1 + ip] = cm;		/* restore tested boundary */

	if (area > area_tot) {			/* boundary intersects poly1 */
	  /* poly2 splits poly1, but do not actually split */
	  if (!poly3) return(2);

	  /* number of caps of poly1 with extra boundary */
	  np = np1 + 1;
	  
	  /* make sure poly3 contains enough space */
	  ier = room_poly(poly3, np, DNP, 0);
	  
	  if (ier == -1) goto out_of_memory;	  
	  
	  /* poly3 is intersection of poly1 and ip'th cap of poly2 */
	  poly_polyn(*poly1, poly2, ip, 1, *poly3);
	  
	  /* prune poly3 */
	  iprune = prune_poly(*poly3, mtol);
	  if (iprune == -1) goto error;
	  /* poly3 may be null because of roundoff: skip to next cap */
	  if (iprune >= 2) continue;
	  
	  /* area of poly3 */
	  tol = mtol;
	  verb = 1;
	  ier = garea(*poly3, &tol, verb, &area3);
	  if (ier) goto error;

	  /* check to make sure area of poly3 is less than poly1; if not, skip to next cap*/
	  if(area3>=area1) continue;

	  /* make sure poly4 contains enough space */
	  ier = room_poly(&poly4, np, DNP, 1);
	  if (ier == -1) goto out_of_memory;
	  
	  /* poly4 is intersection of poly1 and complement of ip'th cap of poly2 */
	  poly_polyn(*poly1, poly2, ip, -1, poly4);
	  
	  /* prune poly4 */
	  iprune = prune_poly(poly4, mtol);
	  if (iprune == -1) goto error;
	  /* poly4 may be null because of roundoff: skip to next cap */
	  if (iprune >= 2) continue;
	  
	   /* area of poly4 */
	  tol = mtol;
	  verb = 1;
	  ier = garea(poly4, &tol, verb, &area4);
	  if (ier) goto error;

	  /* check to make sure area of poly3 is less than poly1; if not, skip to next cap*/
	  if(area4>=area1) continue;

	  /* make sure poly1 contains enough space */
	  np = poly4->np;
	  ier = room_poly(poly1, np, DNP, 0);
	  if (ier == -1) goto out_of_memory;
	  
	  /* copy poly4 into poly1 */
	  copy_poly(poly4, *poly1);
	  
	  /* poly1 successfully split into poly1 and poly3 */
	  return(2);

	} else if (area < area_tot) {
	    /* area should be >= area_tot because suppressing a boundary of poly should always increase its area;
	       but this can happen because of roundoff */
	    //fprintf(stderr, "split_poly: area %.16Lg of polygon with boundary %d suppressed should be >= area %.16Lg of polygon\n", area, ip, area_tot);
	}
    }
    
    /* poly2 contains poly1 */
     return(1);

    /* ---------------- error returns ---------------- */
    error:
    return(-1);

    out_of_memory:
    fprintf(stderr, "split_poly: failed to allocate memory for polygon of %d caps\n", np + DNP);
    return(-1);
}

/*------------------------------------------------------------------------------
  Fragment poly1 into several disjoint polygons,
  each of which is either wholly outside or wholly inside poly2.

   Input: *poly1, poly2 are 2 polygons.
	  discard = 0 to retains all parts of poly1;
	  	  = 1 to discard intersection of poly1 with poly2.
	  npolys = maximum number of polygons available in polys array.
	  mtol = initial angular tolerance within which to merge multiple intersections.
  Output: *poly1 and polys[i], i = 0 to npoly - 1,
		are disjoint polygons of poly1;
		all but the last polygon lie outside poly2;
		if discard = 0:
		    if poly1 intersects poly2, then the last polygon,
		    polys[npoly - 1] (or *poly1 if npoly = 0),
		    is the intersection of poly1 and poly2;
		if discard = 1:
		    if poly1 intersects poly2, then the last+1 polygon,
		    polys[npoly],
		    is the discarded intersection of poly1 and poly2;
		    if poly1 lies entirely inside poly2 (so npoly = 0),
		    then *poly1 is set to null.
  Return value: npoly = number of disjoint polygons, excluding poly1,
			or -1 if error occurred in split_poly().
*/
int fragment_poly(polygon **poly1, polygon *poly2, int discard, int npolys, polygon *polys[/*npolys*/], long double mtol, char bmethod)
{
    int npoly, nsplit;
    polygon **poly;

    /* iteratively subdivide polygons of poly1 */
    npoly = 0;
    poly = poly1;
    while (1) {
	/* check space is available */
	if (npoly >= npolys) return(npoly + 1);
	/* split */
	nsplit = split_poly(poly, poly2, &polys[npoly], mtol);
	/* error */
	if (nsplit == -1) return(-1);
	/* done */
	if (nsplit == 0 || nsplit == 1) {
	    if (nsplit == 1 && discard) {
		if (npoly == 0) {
		    free_poly(*poly);
		    *poly = 0x0;
		} else {
		    npoly--;
		}
	    }
	    if(discard==0 && nsplit==1){
	      if(npoly>0){
		/* set weight according to balkanization scheme: */
		if(bmethod=='l'){
		  //do nothing - this is the default behavior
		}
		else if(bmethod=='a'){
		  polys[npoly-1]->weight=(*poly1)->weight + poly2->weight;
		}
		else if(bmethod=='n'){
		  polys[npoly-1]->weight=((*poly1)->weight > poly2->weight)? poly2->weight : (*poly1)->weight ;
		}
		else if(bmethod=='x'){
		  polys[npoly-1]->weight=((*poly1)->weight > poly2->weight)? (*poly1)->weight : poly2->weight ;
		}
		else{
		  fprintf(stderr, "error in fragment_poly: balkanize method %c not recognized.\n", bmethod);
		  return(-1);
		}
	      }
	      else{
		/* set weight according to balkanization scheme: */
		if(bmethod=='l'){
		  //do nothing - this is the default behavior
		}
		else if(bmethod=='a'){
		  (*poly1)->weight=(*poly1)->weight + poly2->weight;
		}
		else if(bmethod=='n'){
		  (*poly1)->weight=((*poly1)->weight > poly2->weight)? poly2->weight : (*poly1)->weight ;
		}
		else if(bmethod=='x'){
		  (*poly1)->weight=((*poly1)->weight > poly2->weight)? (*poly1)->weight : poly2->weight ;
		}
		else{
		  fprintf(stderr, "error in fragment_poly: balkanize method %c not recognized.\n", bmethod);
		  return(-1);
		}
	      }
	    }
	    return(npoly);
	}
	poly = &polys[npoly++];
    }

}
