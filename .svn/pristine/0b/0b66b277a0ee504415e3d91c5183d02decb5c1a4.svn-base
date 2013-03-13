#define nproc 1
#define nameprog "test/stdio/stdio"
#define NBMAXEVENT 50
#define NBMAXREALEVENT 50
#define module "stdio"
#define path "/src/modules/stdio/.libs"

#include "testcommon.h"

/* Function  use to init the event structure */
void init_tabevent() {
	int j;
	nbdiffevent=0;
		for (j=0; j<nproc; j++) {
			listevent[j][0].code="30110";
			listevent[j][0].nbeventexpected=20;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][1].code="30120";
			listevent[j][1].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][2].code="30130";
			listevent[j][2].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][3].code="30140";
			listevent[j][3].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][4].code="30210";
			listevent[j][4].nbeventexpected=21;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][5].code="30220";
			listevent[j][5].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][6].code="30230";
			listevent[j][6].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][7].code="30240";
			listevent[j][7].nbeventexpected= (debug > 0) ? 1 : 0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][8].code="30310";
			listevent[j][8].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][9].code="30320";
			listevent[j][9].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][10].code="30330";
			listevent[j][10].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][11].code="30340";
			listevent[j][11].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][12].code="30350";
			listevent[j][12].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][13].code="30111";
			listevent[j][13].nbeventexpected=20;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][14].code="30121";
			listevent[j][14].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][15].code="30131";
			listevent[j][15].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][16].code="30141";
			listevent[j][16].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][17].code="30211";
			listevent[j][17].nbeventexpected=21;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][18].code="30221";
			listevent[j][18].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][19].code="30231";
			listevent[j][19].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][20].code="30241";
			listevent[j][20].nbeventexpected=(debug > 0) ? 1 : 0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][21].code="30311";
			listevent[j][21].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][22].code="30321";
			listevent[j][22].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][23].code="30331";
			listevent[j][23].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][24].code="30341";
			listevent[j][24].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][25].code="30351";
			listevent[j][25].nbeventexpected=0;
		}
		nbdiffevent++;

}




/*Function use to make some test on stdio event */
void teststdio() {
	if (listevent[0][0].realnbofevent != listevent[0][13].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][0].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][1].realnbofevent != listevent[0][14].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][1].code); 
		exit(EXIT_FAILURE);
	}
	if (listevent[0][2].realnbofevent != listevent[0][15].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][2].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][3].realnbofevent != listevent[0][16].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][3].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][4].realnbofevent != listevent[0][17].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][4].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][5].realnbofevent != listevent[0][18].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][5].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][6].realnbofevent != listevent[0][19].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][7].realnbofevent != listevent[0][20].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][7].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][8].realnbofevent != listevent[0][21].realnbofevent)  {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][8].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][9].realnbofevent != listevent[0][22].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][9].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][10].realnbofevent != listevent[0][23].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][10].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][11].realnbofevent != listevent[0][24].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][11].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][12].realnbofevent != listevent[0][25].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][12].code);
		exit(EXIT_FAILURE);
	}
	if(debug >= DEBUG_INFO) fprintf(stderr, "test stdio finished\n");
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

	teststdio();
	// Check for eztrace_convert success
	run_eztrace_convert();

	clean();

	return EXIT_SUCCESS;
}


