#include <stdint.h>
#include <stdio.h>
#include "eztrace.h"
#include "instrument.h"

static int __foo_initialized = 0;
static int nb_events = 0;

int (*libcompute_) (int* np, int* nd, int* pos, int* vel, int* mass, int* f, int* pot, int* kin);
int (*libdist_) (int* nd, int* r1, int* r2, int* dr, int* d);
int (*libinitialize_) (int* np, int* nd, int* box, int* seed, int* pos, int* vel, int* acc);
int (*libtimestamp_) ();
int (*libupdate_) (int* np, int* nd, int* pos, int* vel, int* f, int* acc, int* mass, int* dt);

int compute_ (int* np, int* nd, int* pos, int* vel, int* mass, int* f, int* pot, int* kin) {
	record_event0(1);
	int ret = libcompute_ (np, nd, pos, vel, mass, f, pot, kin);
	record_event0(2);
	return ret;
}

int dist_ (int* nd, int* r1, int* r2, int* dr, int* d) {
	record_event0(1);
	int ret = libdist_ (nd, r1, r2, dr, d);
	record_event0(2);
	return ret;
}

int initialize_ (int* np, int* nd, int* box, int* seed, int* pos, int* vel, int* acc) {
	record_event0(1);
	int ret = libinitialize_ (np, nd, box, seed, pos, vel, acc);
	record_event0(2);
	return ret;
}

int timestamp_ () {
	record_event0(1);
	int ret = libtimestamp_ ();
	record_event0(2);
	return ret;
}

int update_ (int* np, int* nd, int* pos, int* vel, int* f, int* acc, int* mass, int* dt) {
	record_event0(1);
	int ret = libupdate_ (np, nd, pos, vel, f, acc, mass, dt);
	record_event0(2);
	return ret;
}


START_INTERCEPT
INTERCEPT2("update_", libupdate_)
INTERCEPT2("timestamp_", libtimestamp_)
INTERCEPT2("initialize_", libinitialize_)
INTERCEPT2("dist_", libdist_)
INTERCEPT2("compute_", libcompute_)
END_INTERCEPT

static void __foo_init (void) __attribute__ ((constructor));
static void
__foo_init (void)
{
  DYNAMIC_INTERCEPT_ALL();
}

