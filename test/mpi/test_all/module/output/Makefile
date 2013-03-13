
prefix=/usr/local
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

BIN = libeztrace-convert-alltoall.so libeztrace-alltoall.so libeztrace-autostart-alltoall.so

ldflags=$(LDFLAGS)  -L${libdir}
cflags=$(CFLAGS)  -I/usr/local/include/gtg  -I/usr/local/include/fxt -DTID_RECORDING_ENABLED -I${includedir}

CONVERT_LDFLAGS=-lgtg -lfxt
RECORD_LDFLAGS=-lfxt

all: $(BIN)

libeztrace-convert-alltoall.so: eztrace_convert_alltoall.c alltoall_ev_codes.h
	$(CC) --shared -o libeztrace-convert-alltoall.so eztrace_convert_alltoall.c -fPIC $(cflags) $(ldflags) $(CONVERT_LDFLAGS) -ldl

libeztrace-alltoall.so: alltoall.c alltoall_ev_codes.h
	$(CC) --shared -o libeztrace-alltoall.so alltoall.c -fPIC $(cflags) $(ldflags) -Wl,-Bsymbolic $(RECORD_LDFLAGS) -ldl

libeztrace-autostart-alltoall.so: alltoall.c alltoall_ev_codes.h
	$(CC) --shared -o libeztrace-autostart-alltoall.so alltoall.c -fPIC $(cflags) $(ldflags) -Wl,-Bsymbolic $(RECORD_LDFLAGS) -ldl -DEZTRACE_AUTOSTART

clean:
	rm -f $(BIN)
