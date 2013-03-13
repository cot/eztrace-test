/*
 * errors.c -- test of errors function (errors.h)
 *
 *  Created on: 4 Aug. 2011
 *      Author: Damien Martin-Guillerez <damien.martin-guillerez@inria.fr>
 */
#include <errors.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int test_pptrace_error_reporting() {
	pptrace_clear_error();
	const char *err = pptrace_get_error();
	if(err != NULL)
		return -__LINE__;

	pptrace_report_error("ERROR");
	err = pptrace_get_error();
	if(err == NULL || strcmp(err, "ERROR"))
		return -__LINE__;

	pptrace_report_error("ERROR %d", 1);
	err = pptrace_get_error();
	if(err == NULL || strcmp(err, "ERROR 1"))
		return -__LINE__;

	pptrace_clear_error();
	malloc(-1); // Force error
	err = pptrace_get_error();
	if(errno) {
		return err != NULL && (strcmp(strerror(errno), err) == 0) ? 0 : -__LINE__;
	}
	return err == NULL ? 0 : -__LINE__;
}

int main() {
	return test_pptrace_error_reporting();
}
