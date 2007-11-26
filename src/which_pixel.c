/*------------------------------------------------------------------------------
© M E C Swanson 2005
------------------------------------------------------------------------------*/
#include <math.h>
#include "pi.h"
#include "manglefn.h"

/* Function which_pixel returns the pixel number for a given azimuth and 
   elevation angle.
   inputs:
   az: azimuth angle (in radians)
   el: elevation angle (in radians)
   res: desired resolution of the pixel to be returned
   scheme: pixelization scheme
   returns the number of the pixel containing the point, or -1 if error occurs
*/

int which_pixel(double az, double el, int res, char scheme)
{
  int n,m,pix,base_pix;

  if(az<0){
    az+=TWOPI;
  }
  
  if(az>TWOPI || az<0){
    fprintf(stderr, "error in which_pixel: az must lie beween 0 and 2*PI.\n");
    return(-1);
  }
  if(el>PIBYTWO || el<-PIBYTWO){
    fprintf(stderr, "error in which_pixel: el must lie beween -PI/2 and PI/2.\n");
    return(-1);
  }
  if(res<0){
    fprintf(stderr, "error in which_pixel: resolution must be an integer >=0.\n");
    return(-1);
  }
  

  if(scheme=='s'){
    // this scheme divides up the sphere by rectangles in az and el, and is numbered 
    // such that the resolution is encoded in each pixel number.  The whole sky is pixel 0,
    // pixels 1, 2, 3, and 4 are each 1/4 of the sky (resolution 1), pixels 5-20 are each 
    // 1/16 of the sky (resolution 2), etc.


    if(az==TWOPI) az=0;
    n=(sin(el)==1) ? 0 : ceil((1-sin(el))/2*pow(2,res))-1;
    
    m=floor(az/(TWOPI)*pow(2,res));
    base_pix=pow(2,res)*n+m;
    pix=pixel_start(res,scheme)+base_pix; 
    return(pix);
  }
  else if(scheme=='h'){
    fprintf(stderr, "error in which_pixel: pixel scheme %c not yet implemented\n", scheme);
    return(-1);  
  }
  else{
    fprintf(stderr, "error in which_pixel: pixel scheme %c not recognized.\n", scheme);
    return(-1);  
  }
}

/* Function get_parent_pixels generates a list of the parent pixels for a given child pixel.
   inputs:
   pix_c: number of child pixel 
   scheme: pixelization scheme
   output:
   pix_p[]: array containing parent pixels.  pix_p[r] is the parent pixel of resolution r.
   returns 0 on success, 1 on error
*/

int get_parent_pixels(int pix_c, int pix_p[], char scheme){
  int m,n,res,base_pix,i;

  if(pix_c<0){
    fprintf(stderr, "error in get_parent_pixels: %d is not a valid pixel number\n",pix_c);
    return(1);
  }

  res=get_res(pix_c, scheme);
  if(res==-1) return (1);

  if(scheme=='s'){
    // this scheme divides up the sphere by rectangles in az and el, and is numbered 
    // such that the resolution is encoded in each pixel number.  The whole sky is pixel 0,
    // pixels 1, 2, 3, and 4 are each 1/4 of the sky (resolution 1), pixels 5-20 are each 
    // 1/16 of the sky (resolution 2), etc.
    
    base_pix=pix_c-pixel_start(res,scheme);
    m=base_pix % (int)(pow(2,res));
    n=(base_pix-m)/pow(2,res);

    for(i=res;i>=0;i--){
      //put pixel number into array
      pix_p[i]=pixel_start(i,scheme)+(int)(pow(2,i))*n+m;
      //make child pixel into next parent pixel
      n=n/2;
      m=m/2;
    }
    return(0);
  }
  else if(scheme=='h'){
    fprintf(stderr, "error in get_parent_pixels: pixel scheme %c not yet implemented\n", scheme);
    return(1);  
  }
  else{
    fprintf(stderr, "error in get_parent_pixels: pixel scheme %c not recognized.\n", scheme);
    return(1);   
  }
}

/* Function pixel_start returns the starting pixel number for the pixels of a given resolution
   inputs:
   res: resolution 
   scheme: pixelization scheme
   returns the starting pixel number for the set of pixels of resolution res, 
   or -1 if error occurs
*/

int pixel_start(int res, char scheme){

  if(res<0){
    fprintf(stderr, "error in pixel_start: %d not a valid resolution.\n", res);
    return(-1);
  }
  
  if(scheme=='s'){
    return ((int)((pow(4,res)-1)/3));
  }
  else if(scheme=='h'){
    fprintf(stderr, "error in pixel_start: pixel scheme %c not yet implemented\n", scheme);
    return(-1);  
  }
  else{
    fprintf(stderr, "error in pixel_start: pixel scheme %c not recognized.\n", scheme);
    return(-1);  
  }
  
}  
