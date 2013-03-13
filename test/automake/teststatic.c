#define nproc 1
#define nameprog "test/static/static"
#define NBMAXEVENT 50
#define NBMAXREALEVENT 50
#define module "staticlib"
#define path "/test/static/.libs"

#include "testcommon.h"

/* Function  use to init the event structure */
void init_tabevent() {
	int j;
	nbdiffevent=0;
		for (j=0; j<nproc; j++) {
			listevent[j][0].code="990001";
			listevent[j][0].nbeventexpected=8;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][1].code="990002";
			listevent[j][1].nbeventexpected=8;
		}
		nbdiffevent++;
}



/*Function use to make some test on stdio event */
void teststatic() {
	if (listevent[0][0].realnbofevent != listevent[0][1].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][0].code);
		exit(EXIT_FAILURE);
	}
	if(debug >= DEBUG_INFO) fprintf(stderr, "test static finished\n");
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
	
	// checks
	teststatic();
	run_eztrace_convert();

	// clean-up
	clean();
	
	return EXIT_SUCCESS;
}

