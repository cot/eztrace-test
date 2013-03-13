#include <stdarg.h>
#include "hijacks.h"

void eztrace_code0(int code)
{
	switch(code) {
		case 1: hijack_foo_intro(42,42);    return;
		case 2: hijack_foo_outro(42,42,84); return;
		case 3: hijack_bar_intro();         return;
		case 4: hijack_bar_outro(42);       return;
	}
}

void eztrace_generic(int code, int nb_args, ...)
{
	eztrace_code0(code);
}
