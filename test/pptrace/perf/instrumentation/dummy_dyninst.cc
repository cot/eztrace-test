// Ripped from /src/core/eztrace_dyninst_core.h
// October, 3rd - 2011 - Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>

#include "dyninst_common.h"
static BPatch bpatch;
static BPatch_addressSpace *app = NULL;
static BPatch_binaryEdit *appBin = NULL;

int main(int argc, char**argv)
{
  if(argc != 3) {
    printf("usage: %s orig_prog new_prog\n", argv[0]);
    return 1;
  }

  // Correcting stupid dyninst behavior
  setenv("LD_LIBRARY_PATH", "", 0);

  char* file = argv[1];
  char* newFile = argv[2];
  bool ret;

  //Create the BPatch_addressSpace and BPatch_binaryEdit
  appBin = bpatch.openBinary(file, false);
  if(!appBin) {
    fprintf(stderr, "Cannot open %s\n", file);
    return -1;
  }
  app = static_cast<BPatch_addressSpace *>(appBin);

  // Instrument all the specified functions
  int nb_inst = dummy_dyninst_instrument(app);
  printf("%d functions instrumented\n", nb_inst);
  if(! nb_inst)
    return 1;
  if (appBin != NULL) {
    //Write a new instrumented executable
    appBin->writeFile(newFile);
  } else {
    fprintf(stderr, "cannot write %s\n", newFile);
    return -1;
  }
  return 0;
}
