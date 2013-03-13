LIBRARIES=libinstrument.a libinstrument.so

FXT_ROOT=$$EZTRACE_PREFIX
FXT_CFLAGS=-I$(FXT_ROOT)/include
FXT_LDFLAGS=-Wl,--rpath,$(FXT_ROOT)/lib -L$(FXT_ROOT)/lib -lfxt


all:  $(LIBRARIES)

libinstrument.a: instrument.c
	gcc -c instrument.c -o instrument_static.o $(FXT_CFLAGS)
	ar rcv libinstrument.a  instrument_static.o

libinstrument.so: instrument.c
	gcc -c instrument.c -o instrument_dynamic.o -fPIC $(FXT_CFLAGS)
	gcc instrument_dynamic.o -shared -o libinstrument.so $(FXT_LDFLAGS)

run:

clean:
	rm -f *.o $(LIBRARIES)