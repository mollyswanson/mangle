#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "manglefn.h"
#include "defaults.h"

#define ARGLEN 10

int main(int argc, char *argv[])
{
  int ifile, ipoly, nfiles, npoly;
  polygon *polys[NPOLYSMAX];
   
  int pixel, res, n, m, pixel_num,i;
  double ra, dec;
  char scheme;
  int child_pix[4];
  int *parent_pix;

  /* default output format */
  fmt.out = keywords[POLYGON];
  
  
  /*  
  if(argc<5){
    msg("enter the arguments for which_pixel as command line arguments:\n ra, dec, resolution, and pixelization scheme.\n");
    exit(1);
  } 
  else{
    ra=atof(argv[1]);
    dec=atof(argv[2]);
    res=atoi(argv[3]);
    scheme=argv[4][0];
    
    scale(&ra, 'd', 'r');
    scale(&dec, 'd','r');
    
    pixel = which_pixel(ra,dec,res,scheme);
    printf("pixel=%i\n",pixel);  
    return(0);
  }
  */
  
  
  /*
  if(argc<3){
    msg("enter the arguments for get_child_pixels as command line arguments:\n pixel number and pixelization scheme.\n");
    exit(1);
  }
  else{
    pixel_num=atoi(argv[1]);
    scheme=argv[2][0];
    get_child_pixels(pixel_num,child_pix,scheme);
    printf("parent pixel = %d\nchild pixels = %d, %d, %d, and %d\n", 
	   pixel_num, child_pix[0],child_pix[1],child_pix[2],child_pix[3]);
    return(0);
  }
  */

  /*  
    if(argc<3){
      msg("enter the arguments for get_parent_pixels as command line arguments:\n pixel number and pixelization scheme.\n");
      exit(1);
    }
    else{
      pixel_num=atoi(argv[1]);
      scheme=argv[2][0];
      res=get_res(pixel_num, scheme);
      
      parent_pix = (int *) malloc(sizeof(int) * (res+1));
      if (!parent_pix) {
	fprintf(stderr, "test: failed to allocate memory for %d integers\n", res);
	exit(1);
      }
      get_parent_pixels(pixel_num,parent_pix,scheme);
      printf("child pixel = %d\nparent pixels =", pixel_num);
      for(i=0;i<res;i++){
	printf(" %d, ",parent_pix[i]);
      }
      printf("and %d\n",parent_pix[res]);
      free(parent_pix);
      return(0);
    }
    
  */

  
   
  if(argc<4){
    msg("enter as command line arguments:\n resolution, pixelization scheme, and name of output file\n");
    return(1);
  }
  else{
    res=atoi(argv[1]);
    scheme=argv[2][0];
    
    npoly=pow(4,res);
    
    for(ipoly=0;ipoly<npoly;ipoly++){
      pixel_num=ipoly+(int)((pow(4,res)+1)/3);
      polys[ipoly]=get_pixel(pixel_num,scheme);
      
      m=ipoly % (int)(pow(2,res));
      n=(ipoly-m)/pow(2,res);
      
      polys[ipoly]->weight=(n+m) % 2;
      polys[ipoly]->id=ipoly;
      //      polys[ipoly]->weight=(double)ipoly/(double)npoly;
    }
    
    
    ifile = argc - 1;
    
    advise_fmt(&fmt);
    
    npoly = wrmask(argv[ifile], &fmt, npoly, polys);
    
    if (npoly == -1) exit(1);

    for(ipoly=0;ipoly<npoly;ipoly++){
    free_poly(polys[ipoly]);
    }

      
      return(0);
  }

  

}
