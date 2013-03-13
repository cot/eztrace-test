#include <sys/time.h>
#include <stdio.h>

#define NB_ITER 1000000000
#define TIME_DIFF(t1, t2) ((t2.tv_sec-t1.tv_sec)*1e6+(t2.tv_usec-t1.tv_usec))

int foo_local(){
  return 42;
}


int main() {
  int i, j;
  unsigned res = 0;
  struct timeval t1, t2;

  for(j=100; j<NB_ITER; j*=10) {
    fprintf(stderr, "START %d iter!\n", j);
    if(gettimeofday(&t1, NULL)) {
      perror("gettimeofday");
    }

    for(i=0; i<j; i++) {
      res += foo();
      res += foo_local();
    }
    /* fin de la mesure */
    if(gettimeofday(&t2, NULL)) {
      perror("gettimeofday");
    }
    fprintf(stderr, "STOP !\n");

    /* affichage du resultat */
    fprintf(stderr, "res = %d\n", res);

    printf("%d\t%lf\n",
	   j, TIME_DIFF(t1, t2)/j);
  }

  return 0;
}
