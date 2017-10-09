
#include <stdlib.h>
#include <omp.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>

#ifndef MAX_THREADS
#define MAX_THREADS     2
#endif

#ifndef MAX_TASK_LEVEL
#define MAX_TASK_LEVEL  1
#endif

inline int psum(int *a, int *p, int n)
{
  printf("[%d]: %d\n", omp_get_thread_num(), n);
  int *e;
  for(e=a+n, *p=*a++; a < e; a++, p++) p[1] = p[0] + a[0];
}

int ppsum(int *a, int *p, int n, int l)
{
  if(l > MAX_TASK_LEVEL)
    return psum(a, p, n);

  int n1 = n/2;
  int s1;

#pragma omp task shared(s1)
  {
    ppsum(a, p, n1, l+1);
    s1 = p[n1-1];
  }

   #pragma omp task
  {   
    ppsum(a+n1, p+n1, n-n1, l+1);
  }   

   # pragma omp taskwait

   #pragma omp task
  for(int *pp = p+n1, *ep = p + n1 + n1/2; pp < ep; *pp++ += s1);

   #pragma omp task
  for(int *pp = p+n1+n1/2, *ep = p + n; pp < ep; *pp++ += s1);

    
   # pragma omp taskwait

}


int main(int argc, char *argv[])
{
  if(argc < 2) printf("Error. Provide size.\n");
  int n = atoi(argv[1]);
  int *A = (int *) malloc (n * sizeof(int));
  for(int i=0, j=0; i<n; i++,j++) A[i] = j&0xFF;
  int *p = (int *) malloc (n * sizeof(int));

  printf("%d threads\n", MAX_THREADS);
  printf("%d task levels\n", MAX_TASK_LEVEL);

  int nit = 1;

  double ctime1 = omp_get_wtime();

#pragma omp parallel num_threads(MAX_THREADS)
#pragma omp single
  while(nit--)
    ppsum(A, p, n, 1);

  double ctime2 = omp_get_wtime();
  printf("%f seconds elapsed.\n", ctime2 - ctime1);
  return 0;
}
