BIN = libexample.so libstatic_example.a main main_mpi

all: $(BIN)

main_mpi: main.c example.h
	mpicc -o main_mpi main.c -L. -lexample

main: main.c example.h libexample.so libstatic_example.a
	$(CC) -o main main.c -L. -lexample libstatic_example.a -O0 -g

libexample.so: example.o
	$(CC) --shared -o libexample.so example.o -ldl

libstatic_example.a: static_example.o
	ar rcs libstatic_example.a static_example.o

static_example.o: static_example.c
	$(CC) -c static_example.c  -O0 -g

example.o: example.c example.h
	$(CC) -o example.o -c example.c -fPIC  -O0 -g


clean:
	rm -f $(BIN) *.o