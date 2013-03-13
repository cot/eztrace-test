#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>

#define NB_ITER 1000000
#define SIZE 100

#define TIME_DIFF(t1, t2) ((t2.tv_sec-t1.tv_sec)*1e6+(t2.tv_usec-t1.tv_usec))


double foo_compute(int n, double* a, double*b, double*c) {
  double res = 0;
  int i;
  for(i=0; i<n; i++) {
    c[i]=a[i]+b[i];
    res+=c[i];
  }

  return res;
}

int main() {
  int i;
  double res = 0;
  struct timeval t1, t2;

  double *a, *b, *c;
  a = malloc(sizeof(double)*SIZE);
  b = malloc(sizeof(double)*SIZE);
  c = malloc(sizeof(double)*SIZE);
  for(i=0; i<SIZE; i++) {
    a[i] = i;
    b[i] = i;
  }

  printf("START !\n");
  if(gettimeofday(&t1, NULL)) {
    perror("gettimeofday");
  }

  for(i=0; i<NB_ITER; i++) {
    res += foo_compute(SIZE, a, b, c);
  }
  /* fin de la mesure */
  if(gettimeofday(&t2, NULL)) {
    perror("gettimeofday");
  }
  printf("STOP !\n");

  /* affichage du resultat */
  printf("res = %lf\n", res);

  printf("%d iterations in %lf s (%lf µs per iteration)\n",
	 NB_ITER, TIME_DIFF(t1, t2)/1e6, TIME_DIFF(t1, t2)/NB_ITER);

  return 0;
}
