// Ripped from /src/core/eztrace_dyninst_core.h
// October, 3rd - 2011 - Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>

#include "dyninst_common.h"

static BPatch bpatch;
static BPatch_process *app = NULL;
static BPatch_addressSpace *appImg = NULL;

int main(int argc, char**argv)
{
  if(argc < 2) {
    printf("usage: %s orig_prog [arg1 arg2 ... argn]\n", argv[0]);
    return 1;
  }

  // Correcting stupid dyninst behavior
  setenv("LD_LIBRARY_PATH", "", 0);

  char* file = argv[1];
  char* newFile = argv[2];
  bool ret;
  char **nargv = &(argv[1]);

  //Create the BPatch_addressSpace and BPatch_binaryEdit
  app = bpatch.processCreate(file, (const char**)nargv);
  if(!app) {
    fprintf(stderr, "Cannot open %s\n", file);
    return -1;
  }
  appImg = static_cast<BPatch_addressSpace *>(app);

  // Instrument all the specified functions
  int nb_inst = dummy_dyninst_instrument(appImg);
  printf("%d functions instrumented\n", nb_inst);
 
  // Run the child process
  app->continueExecution();
  while(!app->isTerminated()) {
	bpatch.waitForStatusChange();
  }
  return 0;
}

