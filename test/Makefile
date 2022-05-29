CC      = mpicc
COPT    = -g -O3
CFLAGS  = -Wall

LD      = $(CC)
LDFLAGS = $(COPT)

TARGETS = simple-bcast/bcast.x simple-gather/gather.x simple-scatter/scatter.x simple-send-recv/send-recv.x

all: $(TARGETS)

%.x: %.o
	$(LD) $(LDFLAGS) $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all clean

clean:
	rm -f $(TARGETS)
