BINARY=md_open_mp
DIRS=dyninst eztrace_plugin pin
TARGETS=$(CURDIR)/$(BINARY)
NBRUNS=50

all:	$(BINARY) recursive-all

run:	all simple-run recursive-run

simple-run:
	@echo ================================================================================
	@echo Running $(BINARY) without instrumentation
	@echo ================================================================================
	@for i in {1..$(NBRUNS)}; do \
		$(TARGETS) | grep seconds | xargs echo | cut -f 1 -d " " | xargs echo -n; \
		echo -n " "; \
	done
	@echo
	
recursive-%:
	@for i in $(DIRS); do \
		make -C $$i $(subst recursive-,,$@) TARGETS=$(TARGETS) NBRUNS=$(NBRUNS); \
	done;
		
md_open_mp: md_open_mp.f
	gfortran -o md_open_mp md_open_mp.f -fopenmp

clean:	recursive-clean
	rm -f $(BINARY)