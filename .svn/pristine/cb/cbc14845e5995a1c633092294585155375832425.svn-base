BIN = libexample.so main

all: $(BIN)

main: main.c example.h
	$(CC) -o main main.c -L. -lexample

libexample.so: example.o
	$(CC) --shared -o libexample.so example.o -ldl

example.o: example.c example.h
	$(CC) -o example.o -c example.c -fPIC


clean:
	rm -f $(BIN) *.o