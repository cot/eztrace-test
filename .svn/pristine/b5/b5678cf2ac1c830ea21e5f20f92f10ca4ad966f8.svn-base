// Ripped from /src/core/eztrace_dyninst_core.h
// Common part of both dyninst instrumentation test
// October, 3rd - 2011 - Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
#ifndef __DYNINST_COMMON_H
#define __DYNINST_COMMON_H

#include <stdio.h>
#include <stdint.h>
#include "BPatch.h"
#include "BPatch_function.h"
#include "BPatch_basicBlock.h"

// list of registered functions
int __intercept_function(BPatch_addressSpace *app,
				 const char* function_name,
				 uint32_t code_entry,
				 uint32_t code_exit);

// instrument all the registered functions
int dummy_dyninst_instrument(BPatch_addressSpace *app) {
  int ret = 0;

  ret += __intercept_function(app, "foo", 1, 2);
  ret += __intercept_function(app, "bar", 1, 2);
  return ret;
}

// Instrument a function: eztrace_code0(code_entry) is called at the
// beginning of the function and eztrace_code0(code_entry) is called
// at the end of the function.
// If code_entry or code_exit is null, the corresponding call to
// eztrace_code0 is skipped
int __intercept_function(BPatch_addressSpace *app,
			const char* function_name,
			uint32_t code_entry,
			uint32_t code_exit)
{
  BPatch_image *appImage;
  BPatch_Vector<BPatch_point*> *points;
  BPatch_Vector<BPatch_function *> functions;

  appImage = app->getImage();
  void *ret = appImage->findFunction(function_name, functions, false);

  if(ret==NULL) {
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

    // get the function parameters
    BPatch_function *f = functions[0];
    BPatch_Vector<BPatch_localVar*> *args = f->getParams();

    int i;
    BPatch_constExpr expr_nb_params(args->size());
    int nb_args = args->size();
    BPatch_paramExpr* local_args[6];

    // retrieve the parameters
    switch (nb_args) {
    case 4: local_args[3] = new BPatch_paramExpr(3);
    case 3: local_args[2] = new BPatch_paramExpr(2);
    case 2: local_args[1] = new BPatch_paramExpr(1);
    case 1: local_args[0] = new BPatch_paramExpr(0);
      break;
    default:
      // more than 5 args
      local_args[4] = new BPatch_paramExpr(4);
    }

    dummyArgs.push_back(&expr_nb_params);
    for(i=0;i<nb_args;i++)
      dummyArgs.push_back(local_args[i]);

    // Create the function call
    BPatch_Vector<BPatch_function *> funcs;
    appImage->findFunction("eztrace_generic", funcs);
    BPatch_function *dummyFunc = funcs[0];

    BPatch_funcCallExpr dummyCall(*dummyFunc, dummyArgs);

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
    BPatch_Vector<BPatch_function *> funcs;
    appImage->findFunction("eztrace_code0", funcs);
    BPatch_function *dummyFunc = funcs[0];
    BPatch_funcCallExpr dummyCall(*dummyFunc, dummyArgs);

    //Insert the function call at the point
    app->insertSnippet(dummyCall, *points);
  }
  return 1;
}

#endif // __DYNINST_COMMON_H

