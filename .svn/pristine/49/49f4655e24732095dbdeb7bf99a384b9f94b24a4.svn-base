BINARIES=simple_static simple_dynamic
LIBRARIES=libsimple.a libsimple.so
PWD=`pwd`

all: $(BINARIES) $(LIBRARIES)

simple_static: simple.o libsimple.a
	$(CC) -o simple_static simple.o -L. -lsimple


simple_dynamic: simple.o libsimple.so
	$(CC) -rdynamic -o simple_dynamic simple.o -Wl,--rpath,$(PWD) -L. -lsimple


libsimple.a: foo.c
	gcc -c foo.c -o foo_static.o
	ar rcv libsimple.a  foo_static.o

libsimple.so: foo.c
	gcc -c foo.c -o foo_dynamic.o -fPIC
	gcc foo_dynamic.o -shared -o libsimple.so

clean:
	rm -f $(BINARIES) *.o $(LIBRARIES)
	
run:	all
	@echo ================================================================================
	@echo Running simple_static without instrumentation
	@echo ================================================================================
	@echo
	@./simple_static
	@echo ================================================================================
	@echo Running simple_dynamic without instrumentation
	@echo ================================================================================
	@echo
	@./simple_dynamic
	@make -C ../../instrument run TARGETS="$(CURDIR)/simple_static $(CURDIR)/simple_dynamic"
