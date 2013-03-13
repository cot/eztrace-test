#include "pin.H"
#include "hijacks.h"

int foo_hijack(CONTEXT * ctxt, AFUNPTR orig_foo, int a, int b)
{
    hijack_foo_intro(a,b);
    int r;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orig_foo,
    		PIN_PARG(int), &r, PIN_PARG(int), a, PIN_PARG(int), b, PIN_PARG_END());
    hijack_foo_outro(a,b,r);
    return r;
}

int bar_hijack(CONTEXT * ctxt, AFUNPTR orig_bar)
{
    hijack_bar_intro();
    int r;
    PIN_CallApplicationFunction(ctxt, PIN_ThreadId(), CALLINGSTD_DEFAULT, orig_bar,
    		PIN_PARG(int),  &r, PIN_PARG_END());
    hijack_bar_outro(r);
    return r;
}

void ImageLoad(IMG img, void* v)
{
	PROTO proto;
	RTN rtn = RTN_FindByName(img, "foo");
	if (RTN_Valid(rtn)) {
		proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "foo",
				PIN_PARG(int), PIN_PARG(int), PIN_PARG_END());
		RTN_ReplaceSignature(rtn, AFUNPTR(foo_hijack), IARG_PROTOTYPE, proto,
				IARG_CONST_CONTEXT,
				IARG_ORIG_FUNCPTR,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
				IARG_END);
	}
	rtn = RTN_FindByName(img, "bar");
	if (RTN_Valid(rtn)) {
		proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "bar", PIN_PARG_END());
		RTN_ReplaceSignature(rtn, AFUNPTR(bar_hijack), IARG_PROTOTYPE, proto,
				IARG_CONST_CONTEXT,
				IARG_ORIG_FUNCPTR,
				IARG_END);
	}
}

int main(int argc, char **argv) {
    PIN_InitSymbols();
    PIN_Init(argc,argv);
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_StartProgram();
}
