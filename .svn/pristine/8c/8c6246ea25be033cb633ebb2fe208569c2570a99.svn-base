
if LATEX
if EPSTOPDF
dist_doc_DATA = manuel.pdf

docdir = $(datadir)/doc/@PACKAGE@

LOCAL_TEXSOURCES=manuel.tex paje.eps FxT.bib

doc: manuel.pdf

# See
# http://www.gnu.org/software/automake/manual/html_node/distcleancheck.html#distcleancheck
# for explications
manuel.pdf: $(LOCAL_TEXSOURCES)
	$(MAKE) $(AM_MAKEFLAGS) paje.pdf
	for f in $(LOCAL_TEXSOURCES) ; do \
		test -f $$f || $(LN_S) $(srcdir)/$$f $$f ; \
	done
	-MAKEFILES=LaTeX.mk $(MAKE) -f /dev/null MAKE="$(MAKE) -f /dev/null" $@
	touch $@

paje.pdf: paje.eps
	epstopdf --outfile=$@ $<

CLEANFILES=paje.pdf
DISTCLEANFILES=$(dist_doc_DATA)

clean-local: clean-latex
distclean distclean-generic: distclean-latex

clean-latex distclean-latex:: %-latex:
	-MAKEFILES=LaTeX.mk $(MAKE) -f /dev/null MAKE="$(MAKE) -f /dev/null" $*
	
distclean-latex::
	for f in $(LOCAL_TEXSOURCES) ; do \
		if test -h "$$f" ; then $(RM) "$$f" ; fi ; \
	done
	
EXTRA_DIST = manuel.tex paje.eps FxT.bib

endif
endif
