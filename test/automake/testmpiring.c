#define nproc 3
#define nameprog "./test/mpi/mpi_ring"
#define NBMAXEVENT 85
#define NBMAXREALEVENT 50
#define module "mpi"
#define path "/src/modules/mpi/.libs"

#include "testcommon.h"

/* Function  use to init the event structure */
void init_tabevent() {
	int j;
	nbdiffevent=0;
		for (j=0; j<nproc; j++) {
			listevent[j][0].code="40010";
			listevent[j][0].nbeventexpected=1;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][1].code="40001";
			listevent[j][1].nbeventexpected=10;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][2].code="40002";
			listevent[j][2].nbeventexpected=10;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][3].code="40003";
			listevent[j][3].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][4].code="40004";
			listevent[j][4].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][5].code="40005";
			listevent[j][5].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][6].code="40006";
			listevent[j][6].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][7].code="40007";
			listevent[j][7].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][8].code="40008";
			listevent[j][8].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][9].code="4000a";
			listevent[j][9].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][10].code="4000b";
			listevent[j][10].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][11].code="4000c";
			listevent[j][11].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][12].code="4000d";
			listevent[j][12].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][13].code="40101";
			listevent[j][13].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][14].code="40103";
			listevent[j][14].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][15].code="40105";
			listevent[j][15].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][16].code="40107";
			listevent[j][16].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][17].code="40201";
			listevent[j][17].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][18].code="40202";
			listevent[j][18].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][19].code="40203";
			listevent[j][19].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][20].code="40204";
			listevent[j][20].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][21].code="41003";
			listevent[j][21].nbeventexpected=10;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][22].code="41004";
			listevent[j][22].nbeventexpected=10;
		}
		for (j=0; j<nproc; j++) {
			listevent[j][23].code="41005";
			listevent[j][23].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][24].code="42001";
			listevent[j][24].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][25].code="42002";
			listevent[j][25].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][26].code="42003";
			listevent[j][26].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][27].code="42004";
			listevent[j][27].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][28].code="42005";
			listevent[j][28].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][29].code="42006";
			listevent[j][29].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][30].code="42007";
			listevent[j][30].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][31].code="42008";
			listevent[j][31].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][32].code="42009";
			listevent[j][32].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][33].code="4200a";
			listevent[j][33].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][34].code="4200b";
			listevent[j][34].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][35].code="43101";
			listevent[j][35].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][36].code="43001";
			listevent[j][36].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][37].code="43002";
			listevent[j][37].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][38].code="43003";
			listevent[j][38].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][39].code="43004";
			listevent[j][39].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][40].code="43005";
			listevent[j][40].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][41].code="43006";
			listevent[j][41].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][42].code="43007";
			listevent[j][42].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][43].code="43008";
			listevent[j][43].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][44].code="43009";
			listevent[j][44].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][45].code="4300a";
			listevent[j][45].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][46].code="4300b";
			listevent[j][46].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][47].code="4300c";
			listevent[j][47].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][48].code="4300d";
			listevent[j][48].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][49].code="4300e";
			listevent[j][49].nbeventexpected=11;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][50].code="43011";
			listevent[j][50].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][51].code="43012";
			listevent[j][51].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][52].code="43013";
			listevent[j][52].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][53].code="43014";
			listevent[j][53].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][54].code="43015";
			listevent[j][54].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][55].code="43016";
			listevent[j][55].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][56].code="43017";
			listevent[j][56].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][57].code="43018";
			listevent[j][57].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][58].code="43019";
			listevent[j][58].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][59].code="4301a";
			listevent[j][59].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][60].code="4301b";
			listevent[j][60].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][61].code="4301c";
			listevent[j][61].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][62].code="4301d";
			listevent[j][62].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][63].code="4301e";
			listevent[j][63].nbeventexpected=11;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][64].code="44001";
			listevent[j][64].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][65].code="44002";
			listevent[j][65].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][66].code="45001";
			listevent[j][66].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][67].code="45002";
			listevent[j][67].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][68].code="45003";
			listevent[j][68].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][69].code="45004";
			listevent[j][69].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][70].code="45010";
			listevent[j][70].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][71].code="45100";
			listevent[j][71].nbeventexpected=0;
		}
		nbdiffevent++;
		for (j=0; j<nproc; j++) {
			listevent[j][72].code="49999";
			listevent[j][72].nbeventexpected=0;
		}
		nbdiffevent++;
		
		//init the event table;



}

/* function that test MPI communication */


void mpisendrecv() {

	int i;
	int index2;
	int proc=0;
	int index;
	long long int timecom;
	for (i=0; i<nproc; i++) {
		for (proc=0; proc<nproc; proc++) {
			index=0;
			index2=0;
			if (proc!=i) {
				if(debug >= DEBUG_VERBOSE) fprintf(stderr, "send/recv test from trace %d to trace %d\n", i, proc);
				while(index < listevent[i][1].realnbofevent) {
					if (listevent[i][1].tab_realevent[index].arg2==proc) {	
						if(debug >= DEBUG_VERBOSE) fprintf(stderr, "send found from trace %d to trace %d\n", i, proc);

						if (index2 == listevent[proc][22].realnbofevent) {
							fprintf(stderr, "Problem in logged communication between %d and %d, there is a send from %d but no recv\n", i, proc, i);
							exit(EXIT_FAILURE);
						}	
						while(index2 < listevent[proc][22].realnbofevent) {
							if (listevent[proc][22].tab_realevent[index2].arg2==i) {
								timecom=listevent[proc][22].tab_realevent[index2].time-listevent[i][1].tab_realevent[index].time;
								if (timecom<0) {
									fprintf(stderr, "problem during communication from %d to %d, negative time =%lli\n", i, proc, timecom);
									return	EXIT_FAILURE;
								}
								if(debug >= DEBUG_VERBOSE) {
									fprintf(stderr, "time of send = %llu\n", listevent[i][1].tab_realevent[index].time);
									fprintf(stderr, "time of recv = %llu\n", listevent[proc][22].tab_realevent[index2].time);
									fprintf(stderr, "MPI comm detected, msg send from %d to %d, time of the communication = %lli\n", i, proc, timecom);
								}
								index2++;
								break;
							}
							index2++;
							if(debug >= DEBUG_VERBOSE) fprintf(stderr, "%d & %d \n" , index2, listevent[proc][22].realnbofevent);
							if (index2 == listevent[proc][22].realnbofevent) {
								fprintf(stderr, "Problem in comunication between %d and %d, there is a send from %d but no recv\n", i, proc, i);
								exit(EXIT_FAILURE);
							}	
						}
						
					}
					index++;
				}
			}		
		}	
	}
	// we do the same for recv 
	for (i=0; i<nproc; i++) {
		for (proc=0; proc<nproc; proc++) {
			index=0;
			index2=0;
			if (proc!=i) {
				if(debug >= DEBUG_VERBOSE) fprintf(stderr, "recv/send test from trace %d to trace %d\n", i, proc);
				while(index < listevent[i][22].realnbofevent) {
					if (listevent[i][22].tab_realevent[index].arg2==proc) {	
						if(debug >= DEBUG_VERBOSE) fprintf(stderr, "recv found from trace %d to trace %d\n", i, proc);

						if (index2 == listevent[proc][1].realnbofevent) {
							fprintf(stderr, "Problem in communication between %d and %d, there is recv from %d but no send\n", i, proc, i);
							exit(EXIT_FAILURE);
						}	
						while(index2 < listevent[proc][1].realnbofevent) {
							if (listevent[proc][1].tab_realevent[index2].arg2==i) {
								timecom=listevent[i][22].tab_realevent[index].time-listevent[proc][1].tab_realevent[index2].time;
								if (timecom<0) {
									fprintf(stderr, "problem during communication from %d to %d, negative time =%lli\n", i, proc, timecom);
									return	EXIT_FAILURE;
								}
								if(debug >= DEBUG_VERBOSE) {
									fprintf(stderr, "time of send = %llu\n", listevent[i][22].tab_realevent[index].time);
									fprintf(stderr, "time of recv = %llu\n", listevent[proc][1].tab_realevent[index2].time);
									fprintf(stderr, "MPI comm detected, msg send from %d to %d, time of the communication = %lli\n", i, proc, timecom);
								}
								index2++;
								break;
							}
							index2++;
							fprintf(stderr, "%d et %d \n" , index2, listevent[proc][1].realnbofevent);
							if (index2 == listevent[proc][1].realnbofevent) {
								fprintf(stderr, "Problem in communication between %d and %d, there is a recv from %d but no send\n", i, proc, i);
								exit(EXIT_FAILURE);
							}	
						}
						
					}
					index++;
				}
			}		
		}	
	}
	if(debug >= DEBUG_INFO) fprintf(stderr, "mpi communication test finished\n");
}





/*Function use to make some test on mpi event */
int testmpi() {
	if (listevent[0][1].realnbofevent != listevent[0][2].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][0].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][3].realnbofevent != listevent[0][4].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][1].code); 
		exit(EXIT_FAILURE);
	}
	if (listevent[0][5].realnbofevent != listevent[0][6].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][2].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][7].realnbofevent != listevent[0][8].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][3].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][9].realnbofevent != listevent[0][10].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][4].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][11].realnbofevent != listevent[0][12].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][5].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][17].realnbofevent != listevent[0][18].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][19].realnbofevent != listevent[0][20].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][5].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][21].realnbofevent != listevent[0][22].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][24].realnbofevent != listevent[0][25].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][27].realnbofevent != listevent[0][28].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][31].realnbofevent != listevent[0][32].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][33].realnbofevent != listevent[0][34].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][36].realnbofevent != listevent[0][50].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][2].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][37].realnbofevent != listevent[0][51].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][3].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][38].realnbofevent != listevent[0][52].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][4].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][39].realnbofevent != listevent[0][53].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][5].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][40].realnbofevent != listevent[0][54].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][41].realnbofevent != listevent[0][55].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][5].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][42].realnbofevent != listevent[0][56].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][43].realnbofevent != listevent[0][57].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][44].realnbofevent != listevent[0][58].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][45].realnbofevent != listevent[0][59].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][46].realnbofevent != listevent[0][60].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][47].realnbofevent != listevent[0][61].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][48].realnbofevent != listevent[0][62].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if (listevent[0][49].realnbofevent != listevent[0][63].realnbofevent) {
		fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
		exit(EXIT_FAILURE);
	}
	if(debug >= DEBUG_INFO) fprintf(stderr, "test mpi finished\n");
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

	testmpi();
	
	// Check for eztrace_convert success
	run_eztrace_convert();
	
	if (nproc>1) { // special test for mpi communication
		mpisendrecv();
	}

	clean();

	return EXIT_SUCCESS;
}
