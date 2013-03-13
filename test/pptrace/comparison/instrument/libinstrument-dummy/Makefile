LIBRARIES=libinstrument.a libinstrument.so
all:  $(LIBRARIES)

libinstrument.a: instrument.c
	gcc -c instrument.c -o instrument_static.o
	ar rcv libinstrument.a  instrument_static.o

libinstrument.so: instrument.c
	gcc -c instrument.c -o instrument_dynamic.o -fPIC
	gcc instrument_dynamic.o -shared -o libinstrument.so
	ar rcv libinstrument.a  instrument_static.o

run:

clean:
	rm -f *.o $(LIBRARIES)