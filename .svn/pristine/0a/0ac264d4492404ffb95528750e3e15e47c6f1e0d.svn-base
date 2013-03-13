/*BEGIN_LEGAL 
Intel Open Source License 

Copyright (c) 2002-2012 Intel Corporation. All rights reserved.
 
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.  Redistributions
in binary form must reproduce the above copyright notice, this list of
conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.  Neither the name of
the Intel Corporation nor the names of its contributors may be used to
endorse or promote products derived from this software without
specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE INTEL OR
ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
END_LEGAL */
//
// This tool counts the number of times a routine is executed and 
// the number of instructions executed in a routine
//

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string.h>
#include "pin.H"
#include "instrument.h"

 // record one event before function foo is called

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This Pintool counts the number of times a routine is executed" << endl;
    cerr << "and the number of instructions executed in a routine" << endl;
    cerr << endl << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

int instrument_function(IMG img, const char* function_name, AFUNPTR callback)
{
    __init();

    RTN rtn = RTN_FindByName(img, function_name);
    if(rtn == RTN_Invalid()) {
      //printf("RTN_FindByName failed\n");
      return -1;
    }

    RTN_Open(rtn);
    RTN_InsertCall(rtn, IPOINT_BEFORE, callback, IARG_UINT32, 1, IARG_END );
    RTN_InsertCall(rtn, IPOINT_AFTER, callback, IARG_UINT32, 2, IARG_END );
    RTN_Close(rtn);
    printf("callback inserted for function %s\n", function_name);
    return 0;
}


void my_function(int n)
{
  //printf("my_function was called with param %d\n", n);
  record_event0(1);
}

void my_fini(void* arg)
{
  __conclude();
}

VOID ImageLoad(IMG img, VOID *v) {
  printf("Loading %s, Image id = %d\n", IMG_Name(img).c_str(), IMG_Id(img));
  //instrument_function(img, "foo", AFUNPTR(record_event0));
  instrument_function(img, "compute_", AFUNPTR(my_function));
  instrument_function(img, "dist_", AFUNPTR(my_function));
  instrument_function(img, "initialize_", AFUNPTR(my_function));
  instrument_function(img, "timestamp_", AFUNPTR(my_function));
  instrument_function(img, "update_", AFUNPTR(my_function));
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int main(int argc, char * argv[])
{
    // Initialize symbol table code, needed for rtn instrumentation
    PIN_InitSymbols();

    // Initialize pin
    if (PIN_Init(argc, argv)) return Usage();

    IMG img = IMG_Open(argv[argc-1]);
    if(img == IMG_Invalid( ) ) {
      printf("Can't open file %s\n", argv[argc-1]);
      return 1;
    }


    PIN_AddFiniFunction(FINI_CALLBACK(my_fini), NULL);

    IMG_AddInstrumentFunction(ImageLoad, 0);

    // Start the program, never returns
    PIN_StartProgram();
    return 0;
}
