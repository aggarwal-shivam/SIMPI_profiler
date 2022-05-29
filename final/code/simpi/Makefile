CC      = mpicc
COPT    = -g -O3 -shared -fPIC -L $(HOME)/papi-install/lib -lpapi
CFLAGS  = -Wall -fPIC -I $(HOME)/papi-install/include

LD      = $(CC)
LDFLAGS = $(COPT)

MPI_DIR = $(HOME)/mpich/lib/

all: libsimpi.so

libsimpi.so: simpi.o
	$(CC) $(COPT) -o $@ $<

simpi.o: simpi.c
	$(CC) $(CFLAGS) -c $< -o $@

install: libsimpi.so
	cp libsimpi.so $(MPI_DIR)

.PHONY: all clean install

clean:
	rm -rf *.o *.a
