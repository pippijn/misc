#include <stdio.h> 

int
main ()
{
  int i,j; short  B[50]; float n;
  
  while(true)
    {   
      for (i=0; i<50; i++) B[i] = 0; i = 0;
      printf("Enter decimal fraction starting with a decimal point: "); 
      scanf("%f",&n); 
      while (n != 0 && i < 50)
        { n = 2.0*n; 
          if (n < 1.0) 
            { B[i] = 0;}
          else
            { B[i] = 1; n = n - 1.0; }
          i = i + 1;
        }
      printf("Binary fraction is: "); 
      for (j=0; j<i; j++) printf("%d",B[j]);
      printf("\n");
      
    }  
}
