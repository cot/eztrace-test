// A refactoring of automake tests
#ifndef __TEST_COMMON_H
#define __TEST_COMMON_H

#ifndef eztrace_extra
#define eztrace_extra
#endif

#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define TOP_BUILDDIR "/home/rue/EZTRACE/eztrace/unused-git/"
#define EZTRACE "/home/rue/EZTRACE/eztrace/unused-git/src/core/eztrace"
#define EZTRACE_CONVERT "/home/rue/EZTRACE/eztrace/unused-git/src/core/eztrace_convert"
#define FXT_PRINT "/home/rue/EZTRACE/eztrace/unused-git/extlib/fxt/tools/fxt_print"

char pwd[512];
char user[32];
int debug = 3;

char trace[nproc][256];
pid_t pid;
int nbdiffevent;
char filecode[nproc][25];
char fileref[nproc][25];
char filetime[nproc][25];
char filearg1[nproc][25];
char filearg2[nproc][25];

/* struct used in order to record event */

struct realevent {
    int proc;
    unsigned long long int time;
    int arg1, arg2;
};

struct event {
    char* code;
    int nbeventexpected;
    int realnbofevent;
    struct realevent tab_realevent[NBMAXREALEVENT];
};

struct event listevent[nproc][NBMAXEVENT];

#define DEBUG_INFO    1
#define DEBUG_VERBOSE 2
#define DEBUG_ALL     3

/* function that delete all files created */
void clean() {
    int j;
    for (j = 0; j < nproc; j++) {
        unlink(trace[j]);
        unlink(filecode[j]);
        unlink(fileref[j]);
        unlink(filetime[j]);
        unlink(filearg1[j]);
        unlink(filearg2[j]);
        unlink("paje.trace");
    }
    if (debug >= DEBUG_VERBOSE)
        fprintf(stderr, "all files deleted\n");

}

/* function that return the number of the event choose for each proc, also complete the table of real event */
int getnbevent(int index, int numproc) {
    int i;
    FILE *fic, *fic2, *fic3, *fic4;
    int nb_event = 0;
    char ligne[128], ligne2[128], ligne3[128], ligne4[128];
    char buffertime[25];
    unsigned long long int time;

    // opening files
    fic = fopen(filecode[numproc], "r"); // ouvrir en lecture
    fic2 = fopen(filetime[numproc], "r");
    fic3 = fopen(filearg1[numproc], "r");
    fic4 = fopen(filearg2[numproc], "r");
    if (fic == NULL || fic2 == NULL || fic3 == NULL || fic4 == NULL) {
        fprintf(stderr, "can't open one file in function getnbevent()\n");
        return -1;
    }

    // count event and do the work
    nb_event = 0;
    while (fgets(ligne, 512, fic) != NULL) {
        assert(fgets(ligne2, 512, fic2) != NULL);
        assert(fgets(ligne3, 512, fic3) != NULL);
        assert(fgets(ligne4, 512, fic4) != NULL);
        if (strncmp(ligne, listevent[numproc][index].code, 5) == 0) {
            strncpy(buffertime, ligne2, 15);
            buffertime[15] = '\0';
            time = atol(buffertime);
            listevent[numproc][index].tab_realevent[nb_event].time = time;
            listevent[numproc][index].tab_realevent[nb_event].arg1 = atoi(ligne3);
            listevent[numproc][index].tab_realevent[nb_event].arg2 = atoi(ligne4);
            if (debug >= DEBUG_ALL)
                fprintf(stderr, "%d event %s found with time = %llu , arg1 = %d , arg2 = %d\n", nb_event + 1,
                        listevent[numproc][index].code, listevent[numproc][index].tab_realevent[nb_event].time,
                        listevent[numproc][index].tab_realevent[nb_event].arg1,
                        listevent[numproc][index].tab_realevent[nb_event].arg2);
            nb_event++;
        }
    }
    listevent[numproc][index].tab_realevent[nb_event].time = 0;
    // closing files
    if (fclose(fic) == EOF || fclose(fic2) == EOF || fclose(fic3) == EOF || fclose(fic4) == EOF) {
        fprintf(stderr, "Problem closing one file in function getnbevent()\n");
        return -1;
    }

    return nb_event;
}

void sysexec(char *cmd, ...) {
    // Variable length system method
    char command[1024];
    command[0] = 0;
    va_list ap;
    va_start(ap, cmd);
    while (cmd != NULL) {
        strncat(command, cmd, 1024);
        cmd = va_arg(ap, char*);
    }
    va_end(ap);
    command[1023] = 0;
    if (debug >= DEBUG_ALL)
        fprintf(stderr, "%s\n", command);
    if (system(command) == -1) {
        fprintf(stderr, "Command %s failed!\n", command);
        exit (EXIT_FAILURE);
    }
}

/* Function that parse all element of each trace */
void parseevent() {
    int i;
    int j;

    for (j = 0; j < nproc; j++) {
        snprintf(fileref[j], 25, "testlog%d.txt", j);
        if (debug >= DEBUG_ALL)
            fprintf(stderr, "fileref %d = %s\n", j, fileref[j]);
    }

    for (j = 0; j < nproc; j++) {
        snprintf(filecode[j], 25, "testlog%d.txt", j + 100);
        if (debug >= DEBUG_ALL)
            fprintf(stderr, "filecode %d = %s\n", j, filecode[j]);
        snprintf(filetime[j], 25, "testlog%d.txt", j + 200);
        if (debug >= DEBUG_ALL)
            fprintf(stderr, "filetime %d = %s\n", j, filetime[j]);
        snprintf(filearg1[j], 25, "testlog%d.txt", j + 300);
        if (debug >= DEBUG_ALL)
            fprintf(stderr, "filearg1 %d = %s\n", j, filearg1[j]);
        snprintf(filearg2[j], 25, "testlog%d.txt", j + 400);
        if (debug >= DEBUG_ALL)
            fprintf(stderr, "filearg2 %d = %s\n", j, filearg2[j]);
    }

    for (j = 0; j < nproc; j++) {
    	/* read the binary trace */
        sysexec(FXT_PRINT, " -f ", trace[j], " > ", fileref[j], NULL);

	    /* remove the header */
	    sysexec("sed -i 1,11d ", fileref[j], " >/dev/null", NULL);
    
	    /* retrieve the code, time and arguments of each event */
        sysexec("awk '{print $8}' ", fileref[j], " > ", filecode[j], NULL);
        sysexec("awk '{print $1}' ", fileref[j], " > ", filetime[j], NULL);
        sysexec("awk '{print $9}' ", fileref[j], " > ", filearg1[j], NULL);
        sysexec("awk '{print $10}' ", fileref[j], " > ", filearg2[j], NULL);
    }

    /* check if the number of events is the one that we expect */
    for (j = 0; j < nproc; j++) {
        for (i = 0; i < nbdiffevent; i++) {
            listevent[j][i].realnbofevent = getnbevent(i, j);
            if (listevent[j][i].realnbofevent != listevent[j][i].nbeventexpected) {
                fprintf(stderr, "Problem with code %s, expected number : %d, but there is %d event on the trace\n",
                    listevent[j][i].code, listevent[j][i].nbeventexpected, listevent[j][i].realnbofevent);
                exit (EXIT_FAILURE);
            }
            if (debug >= DEBUG_INFO)
                fprintf(stderr, "code %s have the correct number of events for trace %d\n", listevent[j][i].code, j);
        }
    }

    if (debug >= DEBUG_INFO)
        fprintf(stderr, "parsing of event finish\n");
}

// Test the existence and readability of file
void testfilereadable(char *file) {
    struct stat st;
    if (stat(file, &st) < 0) {
        fprintf(stderr, "Can't stat file %s: %s\n", file, strerror(errno));
        exit (EXIT_FAILURE);
    }
    if ((st.st_mode & S_IRUSR) == 0) {
        fprintf(stderr, "File %s is not readable\n", file);
        exit (EXIT_FAILURE);
    }
}

/* function that check the creation of all trace */
void testbintracecreation() {
    int j;
    for (j = 0; j < nproc; j++) {
        testfilereadable(trace[j]);
        if (debug >= DEBUG_INFO)
            fprintf(stderr, "trace[%d] correctly created\n", j);
    }

    if (debug >= DEBUG_INFO)
        fprintf(stderr, "bintrace correctly creates\n");
}

// Parse common information from the environment
inline void parseenv() {
    /* debug info */
    char *buffer = getenv("DEBUG_LEVEL");
    if (buffer != NULL)
        debug = atoi(buffer);

    if (debug <= 0)
        setenv("EZTRACE_DEBUG", "-1", 1);
    /* bug on hydra, don't know why...
     else if(debug > 2) {
     setenv("EZTRACE_DEBUG", "2", 1);
     } */

    /* we need some environment variable to retrieve the trace name */
    if (getenv("USER") == NULL)
        setenv("USER", "foo", 1);

    strcpy(user, getenv("USER"));

    // The path
    assert(getcwd(pwd, sizeof(pwd)) != NULL);
}

// Actually run eztrace
inline void run_eztrace() {
    if (debug >= DEBUG_INFO)
      fprintf(stderr, "libpath= %s%s\n", TOP_BUILDDIR, path);
    /* We execute the program with eztrace with the correct module*/
    char *devnull = (debug == 0) ? " 2>/dev/null" : "";
    if (nproc == 1)               // without MPI
      sysexec(eztrace_extra "EZTRACE_LIBRARY_PATH=", TOP_BUILDDIR, path, " EZTRACE_TRACE=", module, " ", EZTRACE, " -p ",
	      TOP_BUILDDIR, nameprog, devnull, NULL);
    else { // with MPI
        char numbuf[4];
        sprintf(numbuf, "%d", nproc);
        sysexec(eztrace_extra "EZTRACE_LIBRARY_PATH=", TOP_BUILDDIR, path, " EZTRACE_TRACE=", module, " mpirun -np ", numbuf,
		" ", EZTRACE, " -p ", TOP_BUILDDIR, nameprog, devnull, NULL);
    }
}

// Run eztrace_convert
inline void run_eztrace_convert() {
    char traces[2048];
    int i;
    traces[0] = 0;
    for (i = 0; i < nproc; i++) {
        strncat(traces, " ", 2048);
        strncat(traces, trace[i], 2048);
    }
    if (debug >= DEBUG_INFO)
        fprintf(stderr, "libpath=%s%s\n", TOP_BUILDDIR, path);
    /* We execute the program with eztrace_convert with the correct module*/
    char *devnull = (debug == 0) ? " 2>/dev/null >/dev/null" : "";
    sysexec("EZTRACE_LIBRARY_PATH=", TOP_BUILDDIR, path, " EZTRACE_TRACE=", module, " ", EZTRACE_CONVERT, traces, devnull,
        NULL);
    testfilereadable("paje.trace");
    if (debug >= DEBUG_INFO)
        fprintf(stderr, "eztrace_convert ran with success!\n");
}

// Fetch the traces
inline void fetch_traces() {
    char* trace_dir = "/tmp";
    char buffertrace[256];
    int i;

    if (getenv("EZTRACE_TRACE_DIR") != NULL)
        trace_dir = getenv("EZTRACE_TRACE_DIR");

    if (debug >= DEBUG_VERBOSE)
        fprintf(stderr, "trace_dir = %s\n", trace_dir);

    snprintf(buffertrace, 256, "%s/%s_eztrace_log_rank_", trace_dir, user);

    if (nproc == 1) {
        snprintf(trace[i], 256, "%s1", buffertrace);
        if (debug >= DEBUG_VERBOSE)
            fprintf(stderr, "trace = %s\n", trace[0]);
    } else {
        for (i = 0; i < nproc; i++) {
            snprintf(trace[i], 256, "%s%d", buffertrace, i);
            if (debug >= DEBUG_VERBOSE)
                fprintf(stderr, "trace[%d] = %s\n", i, trace[i]);
        }
    }
    /* We need to check if the traces are created */
    testbintracecreation();
}

// test for pthread module
/*Function use to make some test on pthread event */
void testpthread() {
    if (listevent[0][1].realnbofevent != listevent[0][2].realnbofevent) {
        fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][0].code);
        exit (EXIT_FAILURE);
    }
    if (listevent[0][3].realnbofevent != listevent[0][4].realnbofevent) {
        fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][1].code);
        exit (EXIT_FAILURE);
    }
    if (listevent[0][9].realnbofevent != listevent[0][10].realnbofevent) {
        fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][2].code);
        exit (EXIT_FAILURE);
    }
    if (listevent[0][14].realnbofevent != listevent[0][15].realnbofevent) {
        fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][3].code);
        exit (EXIT_FAILURE);
    }
    if (listevent[0][16].realnbofevent != listevent[0][17].realnbofevent) {
        fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][4].code);
        exit (EXIT_FAILURE);
    }
    if (listevent[0][18].realnbofevent != listevent[0][19].realnbofevent) {
        fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][5].code);
        exit (EXIT_FAILURE);
    }
    if (listevent[0][21].realnbofevent != listevent[0][22].realnbofevent) {
        fprintf(stderr, "Number of start/stop for event %s isn't the same", listevent[0][6].code);
        exit (EXIT_FAILURE);
    }
    if (debug >= DEBUG_INFO)
        fprintf(stderr, "test pthread finished\n");
}

#endif // defined(__TESTCOMMON_H)_
