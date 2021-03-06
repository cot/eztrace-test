######################################################
#  Compiler                                          #
######################################################
CC=gcc -std=c99
FORTRAN=gfortran
OPENMP_C_FLAG=-fopenmp
OPENMP_F_FLAG=-fopenmp

######################################################
#  OPARI & DUMMY LIBRARY                             #
######################################################
POMP_INC=-I/home/rue/EZTRACE/eztrace/unused-git/extlib/opari2/build-frontend/../doc/example_user_instrumentation/../../include
POMPLIBDIR=/home/rue/EZTRACE/eztrace/unused-git/extlib/opari2/build-frontend/../doc/example_user_instrumentation/../../build-frontend/.libs/ 
POMPLIB=-L${POMPLIBDIR} -lpomp 

BINDIR=/usr/local/bin
OPARI=$(BINDIR)/opari2
OPARI2_CONFIG=$(BINDIR)/opari2-config

# We need to make sure that we use the right versions
# of nm, awk and grep. opari2-config returns the right
# commands with the necessary options.
NM=`$(OPARI2_CONFIG) --nm`
AWK=`$(OPARI2_CONFIG) --awk-cmd`
AWK_SCRIPT=`$(OPARI2_CONFIG) --awk-script`
GREP=`$(OPARI2_CONFIG) --egrep`

all: c fortran

######################################################
#  C EXAMPLE                                         #
######################################################

c: example_user_instrumentation_c example_user_instrumentation_c_inst

# Build without instrumentation
example_user_instrumentation_c: example_user_instrumentation.c
	$(CC) $(OPENMP_C_FLAG) $? -o $@

# Linking step
example_user_instrumentation_c_inst: example_user_instrumentation_c.mod.o pompregions_c.o
	$(CC) $(OPENMP_C_FLAG) $? $(POMPLIB) -o $@

pompregions_c.o : pompregions_c.c
	$(CC)  $(POMP_INC) -c $? -o $@

# Use nm to find the initialization functions in the 
# object file of the instrumented user code. Then the 
# awk script generates these functions.
pompregions_c.c : example_user_instrumentation_c.mod.o
	$(NM) example_user_instrumentation_c.mod.o | $(GREP) -i POMP2_Init_regions | $(GREP) " [TD] " | $(AWK) -f $(AWK_SCRIPT) > pompregions_c.c

example_user_instrumentation_c.mod.o: example_user_instrumentation.mod.c
	$(CC) $(OPENMP_C_FLAG)  $(POMP_INC) -c $? -o $@

example_user_instrumentation.mod.c: example_user_instrumentation.c
	$(OPARI) example_user_instrumentation.c

######################################################
#  FORTRAN EXAMPLE                                   #
######################################################

fortran: example_user_instrumentation_f example_user_instrumentation_f_inst

# Build without instrumentation
example_user_instrumentation_f: example_user_instrumentation.f
	$(FORTRAN) $(OPENMP_F_FLAG) $? -o $@

# Linking step
example_user_instrumentation_f_inst: example_user_instrumentation_f.mod.o pompregions_f.o
	$(FORTRAN) $(OPENMP_F_FLAG) $? $(POMPLIB) -o $@

pompregions_f.o : pompregions_f.c
	$(CC)  $(POMP_INC) -c $? -o $@

#  Use nm to find the initialization functions in the 
# object file of the instrumented user code. Then the 
# awk script generates these functions.
pompregions_f.c : example_user_instrumentation_f.mod.o
	$(NM) example_user_instrumentation_f.mod.o | $(GREP) -i POMP2_Init_regions | $(AWK) -f $(AWK_SCRIPT) > pompregions_f.c

example_user_instrumentation_f.mod.o : example_user_instrumentation.mod.F
	$(FORTRAN) $(OPENMP_F_FLAG) -c -o $@ $?

example_user_instrumentation.mod.F: example_user_instrumentation.f
	$(OPARI) example_user_instrumentation.f

######################################################

clean:
	rm -rf example_user_instrumentation_?
	rm -rf example_user_instrumentation_?_inst
	rm -rf example_user_instrumentation.mod.?
	rm -rf example_user_instrumentation.?.opari.inc
	rm -rf pompregions_?.c

	rm -rf *.o
