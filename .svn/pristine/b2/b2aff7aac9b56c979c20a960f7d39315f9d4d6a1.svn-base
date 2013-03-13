
DIRS=programs/md programs/simple

all:	recursive-all

run:	all recursive-run

clean:	recursive-clean
	rm -f $(BINARY)
	
recursive-%:
	@for i in $(DIRS); do \
		make -C $$i $(subst recursive-,,$@); \
	done;
		
