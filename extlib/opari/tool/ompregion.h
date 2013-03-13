/*************************************************************************/
/* OPARI Version 1.1                                                     */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef OMPREGION_H
#define OMPREGION_H

#include <iostream>
#include <string>
  using std::string;

class OMPRegion {
public:
  OMPRegion(const string& n, const string& file, int i, int bfl, int bll)
           : name(n), file_name(file), id(i),
             begin_first_line(bfl), begin_last_line(bll),
             end_first_line(0), end_last_line(0),
	     num_sections(0), noWaitAdded(false)
           {}

  static void generate_header(ostream& os) {
    os << "#include \"pomp_lib.h\"\n\n";
  }

  void generate_descr(ostream& os) {
    os << "struct ompregdescr omp_rd_" << id << " = {\n";
    os << "  \"" << name << "\", \"" << sub_name << "\", "
       << num_sections << ", \"" << file_name << "\", "
       << begin_first_line << ", " << begin_last_line << ", "
       << end_first_line << ", " << end_last_line << "\n};\n\n";
  }

  string name;
  string file_name;
  string sub_name;
  int    id;
  int    begin_first_line;
  int    begin_last_line;
  int    end_first_line;
  int    end_last_line;
  int    num_sections;
  bool   noWaitAdded;
};

#endif
