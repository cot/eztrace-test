/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1
 * See COPYING in top-level directory.
 */

#include <stdio.h>
#include <stdint.h>
#include "BPatch.h"
#include "BPatch_function.h"
#include "BPatch_basicBlock.h"

#include <stdio.h>
#include <stdint.h>
#include "BPatch.h"
#include "BPatch_callbacks.h"
#include "BPatch_function.h"

#define CREATE_BINARY 1

// describe a function to instrument
struct eztrace_dyninst_func {
  char* fname;
  uint32_t code_entry;
  uint32_t code_exit;
  struct eztrace_dyninst_func *next;
};
typedef struct eztrace_dyninst_func *eztrace_dyninst_func_t;

// list of registered functions
eztrace_dyninst_func_t func_list = NULL;

static int __intercept_function(BPatch_addressSpace *app,
				 const char* function_name,
				 uint32_t code_entry,
				 uint32_t code_exit);

static int nb_func=0;

// register a function
extern "C" void eztrace_dyninst_register(const char* fname,
					 uint32_t code_entry,
					 uint32_t code_exit) {
  struct eztrace_dyninst_func *new_func = (struct eztrace_dyninst_func *) malloc(sizeof(struct eztrace_dyninst_func));

  asprintf(&new_func->fname, "%s", fname);
  new_func->code_entry = code_entry;
  new_func->code_exit = code_exit;

  if(func_list)
    new_func->next = func_list;
  else
    new_func->next = NULL;
  func_list = new_func;
}

// instrument all the registered functions
extern "C" int eztrace_dyninst_instrument(BPatch_addressSpace *app) {
  int ret = 0;
  eztrace_dyninst_func_t func = func_list;

  ret = 0;
  while(func != NULL) {
    ret += __intercept_function(app,
			 func->fname,
			 func->code_entry,
			 func->code_exit);
    func = func->next;
  }
  return ret;
}

extern "C" int eztrace_dyninst_nb_function_to_register()
{
  eztrace_dyninst_func_t func = func_list;
  int ret = 0;

  while(func && func->next != func_list) {
    func = func->next;
    ret++;
  }
  return ret;
}

// Instrument a function: eztrace_code0(code_entry) is called at the
// beginning of the function and eztrace_code0(code_entry) is called
// at the end of the function.
// If code_entry or code_exit is null, the corresponding call to
// eztrace_code0 is skipped
static int __intercept_function(BPatch_addressSpace *app,
			const char* function_name,
			uint32_t code_entry,
			uint32_t code_exit)
{
  BPatch_image *appImage;
  BPatch_Vector<BPatch_point*> *points;
  BPatch_Vector<BPatch_function *> functions;

  BPatch_Vector<BPatch_function *> record_event0_ptr;

  appImage = app->getImage();

  // search for record_event0 function
  BPatch_Vector<BPatch_module*> *loaded_modules = appImage->getModules();
  printf("Threre are %d modules\n", loaded_modules->size());

  for(int i=0; i< loaded_modules->size(); i++) {
    BPatch_module* cur_mod = (*loaded_modules)[i];

    char mod_name[80];
    cur_mod->getName(mod_name, 80);

    cur_mod->findFunction("record_event0", record_event0_ptr, false);

    if(!record_event0_ptr.size()) {
      printf("\tfunction record_event0 not found in module %s\n", mod_name);
    } else {
      printf("Found ! in module %s\n", mod_name);
      break;
    }
  }

  if(!record_event0_ptr.size()) {
    printf("Cannot find record_event0 function\n");
    return -1;
  }

  printf("PLOP\n");

  for(int i=0; i< loaded_modules->size(); i++) {
    BPatch_module* cur_mod = (*loaded_modules)[i];

    char mod_name[80];
    cur_mod->getName(mod_name, 80);

    cur_mod->findFunction(function_name, functions, false);

    if(!functions.size()) {
      printf("\tfunction %s not found in module %s\n", function_name, mod_name);
    } else {
      printf("Found %s! \n", function_name );
      break;
    }
  }

 if(!functions.size()) {
    fprintf(stderr, "warning: cannot find function %s in executable\n", function_name);
    return 0;
  }

  // Instrument the entry of the function
  if(code_entry) {
    // We need to call eztrace_generic(code, nb_param, param1, param2, ...)
    points = functions[0]->findPoint(BPatch_entry);
    BPatch_Vector<BPatch_snippet*> dummyArgs;

    // Create the parameter (code_entry)
    BPatch_constExpr code(code_entry);
    dummyArgs.push_back(&code);

    // Create the function call
#if 0
    BPatch_Vector<BPatch_function *> funcs;
    appImage->findFunction("record_event0", funcs);
    BPatch_function *dummyFunc = funcs[0];

    BPatch_funcCallExpr dummyCall(*dummyFunc, dummyArgs);
#else
    BPatch_funcCallExpr dummyCall(*record_event0_ptr[0], dummyArgs);
#endif

    //Insert the function call at the point
    app->insertSnippet(dummyCall, *points);
  }

  // Instrument the exit of the function
  if(code_exit) {
    // the function parameters are not available here, so we have to
    // call eztrace_code0(code)

    points = functions[0]->findPoint(BPatch_exit);
    // Create the parameter (code_entry)
    BPatch_Vector<BPatch_snippet*> dummyArgs;
    BPatch_constExpr code(code_exit);
    dummyArgs.push_back(&code);

    // Create the function call
#if 0
    BPatch_Vector<BPatch_function *> funcs;
    appImage->findFunction("record_event0", funcs);
    BPatch_function *dummyFunc = funcs[0];

    BPatch_funcCallExpr dummyCall(*dummyFunc, dummyArgs);
#else
    BPatch_funcCallExpr dummyCall(*record_event0_ptr[0], dummyArgs);
#endif

    //Insert the function call at the point
    app->insertSnippet(dummyCall, *points);
  }
  return 1;
}

// instrument all the registered functions
extern "C" int eztrace_dyninst_instrument(BPatch_addressSpace *app);

static BPatch bpatch;
static BPatch_addressSpace *app = NULL;
static BPatch_binaryEdit *appBin = NULL;

int main(int argc, char**argv)
{
  if(argc != 3) {
    printf("usage: %s orig_prog new_prog\n", argv[0]);
    return 1;
  }

  char* file = argv[1];
  char* newFile = argv[2];
  bool ret;


  eztrace_dyninst_register ("compute_", 1, 2);
  eztrace_dyninst_register ("dist_", 3, 4);
  eztrace_dyninst_register ("initialize_", 5, 6);
  eztrace_dyninst_register ("timestamp_", 7, 8);
  eztrace_dyninst_register ("update_", 9, 10);


  if(!eztrace_dyninst_nb_function_to_register()) {
    printf("0 functions instrumented\n");
    return 1;
  }


#ifdef CREATE_BINARY
  //Create the BPatch_addressSpace and BPatch_binaryEdit
  appBin = bpatch.openBinary(file, true);
  if(!appBin) {
    fprintf(stderr, "Cannot open %s\n", file);
    return -1;
  }
  app = static_cast<BPatch_addressSpace *>(appBin);


  if(! app->loadLibrary(LIB_EZTRACE_SO)) {
    printf("Cannot load %s\n", LIB_EZTRACE_SO);
    return 1;
  }

#else
  // run the program
  BPatch_process *appProc = bpatch.processCreate(file, NULL);
  if(!appProc) {
    printf("Cannot load program %s\n", file);
  }

  if(! appProc->loadLibrary(LIB_EZTRACE_SO, true)) {
    printf("Cannot load %s\n", LIB_EZTRACE_SO);
    return 1;
  }

  app = static_cast<BPatch_addressSpace *>(appProc);

#endif


  // Instrument all the specified functions
  int nb_inst = eztrace_dyninst_instrument(app);
  printf("%d functions instrumented\n", nb_inst);
  if(! nb_inst)
    return 1;

#ifdef CREATE_BINARY
  if (appBin != NULL) {
    //Write a new instrumented executable
    appBin->writeFile(newFile);
  } else {
    fprintf(stderr, "cannot write %s\n", newFile);
    return -1;
  }
#else

  appProc->continueExecution();
  while(!appProc->isTerminated()) {
    bpatch.waitForStatusChange();
  }
#endif
  return 0;
}
