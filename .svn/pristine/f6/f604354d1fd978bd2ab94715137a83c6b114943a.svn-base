BIN = libeztrace-convert-example.so libeztrace-autostart-example.so libeztrace-example.so

# LDFLAGS=-LPATH_TO_EZTRACE/lib
# CFLAGS=-IPATH_TO_EZTRACE/include
all: $(BIN)

libeztrace-convert-example.so: eztrace_convert_example.c example_ev_codes.h
	$(CC) --shared -o libeztrace-convert-example.so eztrace_convert_example.c -fPIC $(CFLAGS) $(LDFLAGS) -ldl

libeztrace-example.so: example.c example_ev_codes.h
	$(CC) --shared -o libeztrace-example.so example.c -fPIC $(CFLAGS) $(LDFLAGS) -ldl

libeztrace-autostart-example.so: example.c example_ev_codes.h
	$(CC) --shared -o libeztrace-autostart-example.so example.c -fPIC $(CFLAGS) $(LDFLAGS) -ldl -DEZTRACE_AUTOSTART

clean:
	rm -f $(BIN)