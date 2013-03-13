/*************************************************************************/
/* OPARI Version 1.1                                                     */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef HANDLER_H
#define HANDLER_H

#include "opari.h"
#include "ompragma.h"

typedef void (* phandler_t)(OMPragma*, ostream&);

void init_handler(const char* infile, const char* rcfile,
		  Language l, bool genLineStmts);

void finalize_handler(const char* incfile, const char* tabfile);

void generateTableFile(const char* rcfile, const char* tabfile);

phandler_t find_handler(const string& pragma);

void extra_handler(int lineno, ostream& os);

bool set_disabled(const string& constructs);

bool instrument_locks();

extern bool do_transform;

#endif
