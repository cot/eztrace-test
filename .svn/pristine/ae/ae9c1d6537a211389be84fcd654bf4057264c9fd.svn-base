/*
 * hijacks.h
 *
 *  Created on: 03 Oct. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <stdio.h>

static int foo_intro_count = 0;
static int foo_outro_count = 0;
static int bar_intro_count = 0;
static int bar_outro_count = 0;

void hijack_foo_intro(int a, int b)
{
	++foo_intro_count;
}

void hijack_foo_outro(int a, int b, int r)
{
	++foo_outro_count;
}

void hijack_bar_intro()
{
	++bar_intro_count;
}

void hijack_bar_outro(int r)
{
	++bar_outro_count;
}
