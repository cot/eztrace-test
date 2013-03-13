
prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

BIN = libeztrace-convert-herbert.so libeztrace-herbert.so libeztrace-autostart-herbert.so

ldflags=$(LDFLAGS)  -L${libdir}
cflags=$(CFLAGS)  -I/usr/local/include/gtg  -I/usr/local/include/fxt -DTID_RECORDING_ENABLED -I${includedir}

CONVERT_LDFLAGS=-lgtg -lfxt
RECORD_LDFLAGS=-lfxt

all: $(BIN)

libeztrace-convert-herbert.so: eztrace_convert_herbert.c herbert_ev_codes.h
	$(CC) --shared -o libeztrace-convert-herbert.so eztrace_convert_herbert.c -fPIC $(cflags) $(ldflags) $(CONVERT_LDFLAGS) -ldl

libeztrace-herbert.so: herbert.c herbert_ev_codes.h
	$(CC) --shared -o libeztrace-herbert.so herbert.c -fPIC $(cflags) $(ldflags) -Wl,-Bsymbolic $(RECORD_LDFLAGS) -ldl

libeztrace-autostart-herbert.so: herbert.c herbert_ev_codes.h
	$(CC) --shared -o libeztrace-autostart-herbert.so herbert.c -fPIC $(cflags) $(ldflags) -Wl,-Bsymbolic $(RECORD_LDFLAGS) -ldl -DEZTRACE_AUTOSTART

clean:
	rm -f $(BIN)
