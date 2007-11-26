#include <math.h>
#include <stdlib.h>

int pixel_loop(int pixel, int n,double input[], double output[]);
const int nmax=1000;

int main(int argc, char *argv[]){

  double input[nmax];
  double output[nmax];
  int pixel,i,n,m;
  n=10;

  for(i=0; i<n; i++){
    //input[i]=(double)(rand())/RAND_MAX;
    input[i]=i;

  }

  for(i=0; i<n; i++){
    printf("input[%d]=%lf\n",i,input[i]);
  }

  pixel=0;
  i=0;
  
  m=pixel_loop(0,n,&input[i],&output[i]);
  
  printf("made %d polygons\n",m);
  
  for(i=0; i<m; i++){
    printf("output[%d]=%lf\n",i,output[i]);
  }
  
  return 0;

}
  
int pixel_loop(int pixel, int n,double input[], double output[]){
  double child[4];
  int i,j,k,m,out;
  double *poly;
  poly=(double *)malloc(n*sizeof(double));
  out=0;
  
  for(i=0;i<4;i++){    
    child[i]=pow(10,-pixel)*.2*(i+1);
    
    for(j=0;j<n;j++){
      poly[j]=0;
    }
    
    k=0;
    for(j=0;j<n;j++){
      
      if(input[j]<=2*i){
	poly[k]=input[j]+child[i];
	k++;
	printf("poly[%d]=%lf\n",k-1,poly[k-1]);
      }
    }
    m=k;
    if(2*m<n && pixel < 2){
      printf("calling pixel loop for pixel %d\n",pixel+1);
      out+=pixel_loop(pixel+1,m,poly,&output[out]);
    }
    else{
      m=0;
      printf("m=%d\n",m);
      for(k=0;k<m;k++){
	printf("m=%d\n",m);
	output[out]=poly[k];
	out++;     

      }
    }
  }
  free(poly);
  return out;
}
