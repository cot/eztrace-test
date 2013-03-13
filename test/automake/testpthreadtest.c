#define nproc 1
#define nameprog "test/pthread/test_pthread"
#define NBMAXEVENT 55
#define NBMAXREALEVENT 85
#define module "pthread"
#define path "/src/modules/pthread/.libs"

#include "testcommon.h"


/* Function  use to init the event structure */
void init_tabevent() {
	int j;
	nbdiffevent=0;
		for (j=0; j<nproc; j++) {
			listevent[j][0].code="20010";
			listevent[j][0].nbeventexpected=23;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][1].code="20011";
			listevent[j][1].nbeventexpected=22;
	
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][2].code="20012";
			listevent[j][2].nbeventexpected=22;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][3].code="20020";
			listevent[j][3].nbeventexpected=40;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][4].code="20021";
			listevent[j][4].nbeventexpected=40;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][5].code="20022";
			listevent[j][5].nbeventexpected=40;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][6].code="20023";
			listevent[j][6].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][7].code="20030";
			listevent[j][7].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][8].code="20031";
			listevent[j][8].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][9].code="20033";
			listevent[j][9].nbeventexpected=20;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][10].code="20034";
			listevent[j][10].nbeventexpected=20;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][11].code="20035";
			listevent[j][11].nbeventexpected=20;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][12].code="20040";
			listevent[j][12].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][13].code="20041";
			listevent[j][13].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][14].code="20042";
			listevent[j][14].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][15].code="20043";
			listevent[j][15].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][16].code="20050";
			listevent[j][16].nbeventexpected=20;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][17].code="20051";
			listevent[j][17].nbeventexpected=20;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][18].code="20052";
			listevent[j][18].nbeventexpected=20;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][19].code="20053";
			listevent[j][19].nbeventexpected=20;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][20].code="20054";
			listevent[j][20].nbeventexpected=40;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][21].code="20060";
			listevent[j][21].nbeventexpected=4;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][22].code="20061";
			listevent[j][22].nbeventexpected=4;
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

	testpthread();
	// Check for eztrace_convert success
	run_eztrace_convert();

	clean();

	return EXIT_SUCCESS;
}

