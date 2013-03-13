/*************************************************************************/
/* OPARI Version 1.1                                                     */
/* Copyright (C) 2001                                                    */
/* Forschungszentrum Juelich, Zentralinstitut fuer Angewandte Mathematik */
/*************************************************************************/

#ifndef OMPRAGMA_H
#define OMPRAGMA_H

#include <string>
  using std::string;
#include <vector>
  using std::vector;

class OMPragma {
public:
  OMPragma(const string& f, int l, int pl, int pp)
	  : filename(f), lineno(l), pline(pl), ppos(pp) {}
  void find_name();
  bool is_nowait();
  string find_sub_name();
  virtual void add_nowait() = 0;
  virtual OMPragma* split_combined() = 0;
  virtual ~OMPragma() {}

  string filename;
  int    lineno;
  unsigned          pline;               // current parsing line
  string::size_type ppos;                // current parsing position
  string name;
  vector<string> lines;

private:
  virtual string find_next_word() = 0;
  virtual bool find_word(const char* word, unsigned& line,
		         string::size_type& pos) = 0;
};

class OMPragmaF : public OMPragma {
public:
  OMPragmaF(const string& f, int l, int p, const string& line, int pomp)
	  : OMPragma(f, l, 0, p), slen(5+pomp) {
    lines.push_back(line);
    sentinel = pomp ? "$pomp" : "$omp";
  }
  virtual void add_nowait();
  virtual OMPragma* split_combined();

private:
  virtual string find_next_word();
  virtual bool find_word(const char* word, unsigned& line,
		         string::size_type& pos);
  void remove_empties();

  string sentinel;
  int slen;
};

class OMPragmaC : public OMPragma {
public:
  OMPragmaC(const string& f, int l, int pl, int pp, vector<string>& stmts)
	  : OMPragma(f, l, pl, pp) {
    lines.swap(stmts);
  }
  virtual void add_nowait();
  virtual OMPragma* split_combined();

private:
  virtual string find_next_word();
  virtual bool find_word(const char* word, unsigned& line,
		         string::size_type& pos);
  void remove_empties();
};

#endif
