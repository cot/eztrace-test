// This segfault for some unknown reason at program startup. Impossible to debug it
#include "pin.H"
#include "hijacks.h"

typedef int (*foo_ptr)(int, int);
int foo_hijack(foo_ptr orig_foo, int a, int b)
{
    hijack_foo_intro(a,b);
    int r = orig_foo(a, b);
    hijack_foo_outro(a,b,r);
    return r;
}

typedef int (*bar_ptr)();
int bar_hijack(bar_ptr orig_bar)
{
    hijack_bar_intro();
    int r = orig_bar();
    hijack_bar_outro(r);
    return r;
}

void ImageLoad(IMG img, void* v)
{
	PROTO proto;
	RTN rtn = RTN_FindByName(img, "foo");
	if (RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn)) {
		printf("Instrumenting foo\n");
		proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "foo",
				PIN_PARG(int), PIN_PARG(int), PIN_PARG_END());
		RTN_ReplaceSignatureProbed (rtn, AFUNPTR(foo_hijack), IARG_PROTOTYPE, proto,
				IARG_ORIG_FUNCPTR,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
				IARG_END);
	}
	rtn = RTN_FindByName(img, "bar");
	if (RTN_Valid(rtn) && RTN_IsSafeForProbedReplacement(rtn)) {
		printf("Instrumenting bar\n");
		proto = PROTO_Allocate(PIN_PARG(int), CALLINGSTD_DEFAULT, "bar", PIN_PARG_END());
		RTN_ReplaceSignatureProbed (rtn, AFUNPTR(bar_hijack), IARG_PROTOTYPE, proto,
				IARG_ORIG_FUNCPTR,
				IARG_END);
	}
}

int main(int argc, char **argv) {
    PIN_InitSymbols();
    PIN_Init(argc,argv);
    IMG_AddInstrumentFunction(ImageLoad, 0);
    PIN_StartProgramProbed();
}
