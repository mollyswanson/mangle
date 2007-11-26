/*------------------------------------------------------------------------------
© M E C Swanson 2005
------------------------------------------------------------------------------*/
#include <math.h>
#include "pi.h"
#include "manglefn.h"


/* Function get_pixel takes a pixel number and returns a pointer to a polygon 
   containing that pixel.
   inputs:
   pix: pixel number
   scheme: pixelization scheme
   returns pointer to polygon containing pixel, or 0x0 if an error occurs
*/

polygon *get_pixel(int pix, char scheme){
  int m,n,res,base_pix;
  double azmax, azmin, elmax, elmin;
  double angle[4];
  polygon *pixel;
  
  if(pix<0){
    fprintf(stderr, "error in get_pixel: %d not a valid pixel number.\n", pix);
    return(0x0);
  }

  res=get_res(pix,scheme);
  if(res==-1) return (0x0);
  
  if(scheme=='s'){
    // this scheme divides up the sphere by rectangles in az and el, and is numbered 
    // such that the resolution is encoded in each pixel number.  The whole sky is pixel 0,
    // pixels 1, 2, 3, and 4 are each 1/4 of the sky (resolution 1), pixels 5-20 are each 
    // 1/16 of the sky (resolution 2), etc.
  
    base_pix=pix-pixel_start(res,scheme);
   
    m=base_pix % (int)(pow(2,res));
    n=(base_pix-m)/pow(2,res);
    azmin=TWOPI/pow(2,res)*m;
    azmax=TWOPI/pow(2,res)*(m+1);
    elmin=asin(1-2.0/pow(2,res)*(n+1));
    elmax=asin(1-2.0/pow(2,res)*n);
    
    angle[0]=azmin;
    angle[1]=azmax;
    angle[2]=elmin;
    angle[3]=elmax;

    pixel=new_poly(4);
    if (!pixel) {
      fprintf(stderr, "error in get_pixel: failed to allocate memory for polygon of 4 caps\n");
      return(0x0);
    }
    
    //    printf("az range: %f - %f, el range: %f - %f\n", azmin, azmax, elmin,elmax);
    rect_to_poly(angle,pixel);
    pixel->pixel=pix;
 
    if(!pixel){
      fprintf(stderr, "error in get_pixel: polygon is NULL.\n");
    }
    
    return(pixel);
    
  }
  else if(scheme=='h'){
    fprintf(stderr, "error in get_pixel: pixel scheme %c not yet implemented\n", scheme);
    return(0x0);  
  }
  else{
    fprintf(stderr, "error in get_pixel: pixel scheme %c not recognized.\n", scheme);
    return(0x0);   
  }
}

/* Function get_child_pixels takes a pixel number and calculates the numbers of the 
   child pixels of that pixel
   inputs: 
   pix_p: parent pixel number
   scheme: pixelization scheme
   outputs:
   pix_c[4]: array containing the pixel numbers of the 4 child pixels
   returns 0 on success, 1 if an error occurs
*/

int get_child_pixels(int pix_p, int pix_c[4], char scheme){
  int mp,np,res,base_pix;

  if(pix_p<0){
    fprintf(stderr, "error in get_child_pixels: %d is not a valid pixel number\n",pix_p);
    return(1);
  }

  res=get_res(pix_p, scheme);
  if(res==-1) return (1);

  if(scheme=='s'){
    // this scheme divides up the sphere by rectangles in az and el, and is numbered 
    // such that the resolution is encoded in each pixel number.  The whole sky is pixel 0,
    // pixels 1, 2, 3, and 4 are each 1/4 of the sky (resolution 1), pixels 5-20 are each 
    // 1/16 of the sky (resolution 2), etc.
    
    base_pix=pix_p-pixel_start(res,scheme);
    mp=base_pix % (int)(pow(2,res));
    np=(base_pix-mp)/pow(2,res);
    
    //child pixels will have nc=2*np or 2*np+1, mc=2*mp or 2*mp+1, res_c=res+1
    //for first child pixel (nc=2*np, mc=2*mp), the base pixel number is given by
    //base_pix_c = 2^res_c*nc+mc = 2^(res+1)*2*np+2*mp=2^res*4*np+2*mp
    //combine this with base_pix_p=2^res*np+mp and extra resolution term 4^res 
    //to get formula for the number for the first child pixel number below
    
    pix_c[0]=pix_p+pow(4,res)+pow(2,res)*3*np+mp;
    pix_c[1]=pix_c[0]+1;
    pix_c[2]=pix_c[0]+pow(2,res+1);
    pix_c[3]=pix_c[2]+1;
    return(0);
  }
  else if(scheme=='h'){
    fprintf(stderr, "error in get_child_pixels: pixel scheme %c not yet implemented\n", scheme);
    return(1);  
  }
  else{
    fprintf(stderr, "error in get_child_pixels: pixel scheme %c not recognized.\n", scheme);
    return(1);   
  }

}

/* Function get_res takes a pixel number and returns the resolution 
   implied by that pixel number.
   inputs:
   pix: pixel number
   scheme: pixelization scheme
   returns the resolution of the pixel, or -1 if an error occurs
*/

int get_res(int pix, char scheme){
  int res;
  
  if(pix<0){
    fprintf(stderr, "error in get_res: %d not a valid pixel number.\n", pix);
    return(-1);
  }
  
  if(scheme=='s'){
    for(res=0;pix>=pow(4,res);res++){
      pix-=(int)pow(4,res);
    }
    return(res);
  }
  else if(scheme=='h'){
    fprintf(stderr, "error in get_res: pixel scheme %c not yet implemented\n", scheme);
    return(-1);  
  }
  else{
    fprintf(stderr, "error in get_res: pixel scheme %c not recognized.\n", scheme);
    return(-1);  
  }
  
}

