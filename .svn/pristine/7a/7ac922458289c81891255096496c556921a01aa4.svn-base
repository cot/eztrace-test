#include "pin.H"
#include "hijacks.h"

ADDRINT fooReturnIp = 0;
int fooA = 0;
int fooB = 0;
ADDRINT barReturnIp = 0;
ADDRINT returnIp = 0;

int CheckReturn(ADDRINT sp, ADDRINT returnRegVal)
{  // is this a return from foo or bar
	ADDRINT retIp = *(reinterpret_cast<ADDRINT *>(sp));
	if((fooReturnIp == retIp) || (barReturnIp == retIp)) {
		returnIp = retIp;
		return 1;
	}
	return 0;
}

VOID ProcessReturnFromFoobar(ADDRINT ip, ADDRINT returnVal)
{
	if(returnIp == fooReturnIp) {
		hijack_foo_outro(fooA, fooB, returnVal);
		fooReturnIp = 0;
	} else if(returnIp == barReturnIp) {
		hijack_bar_outro(returnVal);
		barReturnIp = 0;
	} else {
		printf ("[fubar]   return from foo/bar at %p returns %p but it was not detected at either foo (%p) or bar (%p)\n", ip, fooReturnIp, barReturnIp, returnVal);
	}
	returnIp = 0;
}

void foo_hijack(ADDRINT returnIp, int a, int b)
{
	hijack_foo_intro(a,b);
	fooReturnIp = returnIp;
	fooA = a;
	fooB = b;
}

void bar_hijack(ADDRINT returnIp)
{
	hijack_bar_intro();
	barReturnIp = returnIp;
}

void ImageLoad(IMG img, void* v)
{
	PROTO proto;
	RTN rtn = RTN_FindByName(img, "foo");
	if (RTN_Valid(rtn)) {
		RTN_Open(rtn);
		RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(foo_hijack),
				IARG_RETURN_IP,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
				IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
				IARG_END);
		RTN_Close(rtn);
	}
	rtn = RTN_FindByName(img, "bar");
	if (RTN_Valid(rtn)) {
		RTN_Open(rtn);
		RTN_InsertCall(rtn, IPOINT_BEFORE, AFUNPTR(bar_hijack),
				IARG_RETURN_IP,
				IARG_END);
		RTN_Close(rtn);
	}
}


static void Instruction(INS ins, void *v)
{
	if( INS_IsRet(ins))
	{
		INS_InsertIfCall(ins, IPOINT_BEFORE,  (AFUNPTR)CheckReturn,
				IARG_REG_VALUE, REG_STACK_PTR,  IARG_END);
		INS_InsertThenCall(ins, IPOINT_BEFORE,  (AFUNPTR)ProcessReturnFromFoobar,
				IARG_INST_PTR, IARG_FUNCRET_EXITPOINT_VALUE,  IARG_END);
	}
}

int main(int argc, char **argv) {
	PIN_InitSymbols();
	PIN_Init(argc,argv);
	IMG_AddInstrumentFunction(ImageLoad, 0);
	INS_AddInstrumentFunction(Instruction, 0);
	PIN_StartProgram();
}
