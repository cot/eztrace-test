#define eztrace_extra "OMP_NUM_THREADS=2 "
#define nproc 1
#define nameprog "test/openmp/openmp_nowait_c"
#define NBMAXEVENT 55
#define NBMAXREALEVENT 85
#define module "omp"
#define path "/src/modules/omp/.libs"

#include "testcommon.h"

/* Function  use to init the event structure */
void init_tabevent() {
	int j;
	nbdiffevent=0;
		for (j=0; j<nproc; j++) {
			listevent[j][0].code="10001";
			listevent[j][0].nbeventexpected=1;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][1].code="10002";
			listevent[j][1].nbeventexpected=2;
	
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][2].code="10003";
			listevent[j][2].nbeventexpected=2;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][3].code="10004";
			listevent[j][3].nbeventexpected=1;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][4].code="10005";
			listevent[j][4].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][5].code="10006";
			listevent[j][5].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][6].code="10007";
			listevent[j][6].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][7].code="1ffff";
			listevent[j][7].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][8].code="10010";
			listevent[j][8].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][9].code="10011";
			listevent[j][9].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][10].code="10020";
			listevent[j][10].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][11].code="10021";
			listevent[j][11].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][12].code="10030";
			listevent[j][12].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][13].code="10031";
			listevent[j][13].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][14].code="10040";
			listevent[j][14].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][15].code="10041";
			listevent[j][15].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][16].code="10042";
			listevent[j][16].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][17].code="10043";
			listevent[j][17].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][18].code="10050";
			listevent[j][18].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][19].code="10051";
			listevent[j][19].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][20].code="10060";
			listevent[j][20].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][21].code="10061";
			listevent[j][21].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][22].code="10070";
			listevent[j][22].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][23].code="10071";
			listevent[j][23].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][24].code="10072";
			listevent[j][24].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][25].code="10073";
			listevent[j][25].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][26].code="10080";
			listevent[j][26].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][27].code="10081";
			listevent[j][27].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][28].code="10082";
			listevent[j][28].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][29].code="10083";
			listevent[j][29].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][30].code="10090";
			listevent[j][30].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][31].code="10091";
			listevent[j][31].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][32].code="10092";
			listevent[j][32].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][33].code="10093";
			listevent[j][33].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][34].code="100a0";
			listevent[j][34].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][35].code="100a1";
			listevent[j][35].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][36].code="100b0";
			listevent[j][36].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][37].code="100b1";
			listevent[j][37].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][38].code="100b2";
			listevent[j][38].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][39].code="100b3";
			listevent[j][39].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][40].code="100b4";
			listevent[j][40].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][41].code="100b5";
			listevent[j][41].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][42].code="100b6";
			listevent[j][42].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][43].code="100b7";
			listevent[j][43].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][44].code="100b8";
			listevent[j][44].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][45].code="100b9";
			listevent[j][45].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][46].code="100ba";
			listevent[j][46].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][47].code="10101";
			listevent[j][47].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][48].code="10102";
			listevent[j][48].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][49].code="10103";
			listevent[j][49].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][50].code="10104";
			listevent[j][50].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][51].code="10111";
			listevent[j][51].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][52].code="10112";
			listevent[j][52].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][53].code="10113";
			listevent[j][53].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][54].code="10114";
			listevent[j][54].nbeventexpected=0;
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
