#define nproc 1
#define nameprog "test/memory/memory"
#define NBMAXEVENT 55
#define NBMAXREALEVENT 85
#define module "memory"
#define path "/src/modules/memory/.libs"

#include "testcommon.h"



/* Function  use to init the event structure */
void init_tabevent() {
	int j;
	nbdiffevent=0;
	for (j=0; j<nproc; j++) {
		listevent[j][0].code="50001";
		listevent[j][0].nbeventexpected=40;
	}
	nbdiffevent++;
	for (j=0; j<nproc; j++) {
		listevent[j][1].code="50002";
		listevent[j][1].nbeventexpected=0;

	}
	nbdiffevent++;
	for (j=0; j<nproc; j++) {
		listevent[j][2].code="50003";
		listevent[j][2].nbeventexpected=10;
	}
	nbdiffevent++;
	for (j=0; j<nproc; j++) {
		listevent[j][3].code="50010";
		listevent[j][3].nbeventexpected=40;
	}
	nbdiffevent++;
}





/* The main function */
int main() {
	parseenv();
	run_eztrace();
	fetch_traces();

	/* Before checking the bintrace we need to init the structur of event */
	init_tabevent();

	/*After that we retrieve the event of the trace */
	parseevent();

	// Check for eztrace_convert success
	run_eztrace_convert();

	clean();

	return EXIT_SUCCESS;
}

