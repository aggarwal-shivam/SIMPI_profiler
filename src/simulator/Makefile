ifeq ($(shell clang++ -v 2>&1 | grep -c "clang version"), 1)
	CXX := clang++
else
	CXX := g++
endif

NS3FLAGS = -DNS3_BUILD_PROFILE_DEBUG -DNS3_ASSERT_ENABLE -DNS3_LOG_ENABLE -DHAVE_SYS_IOCTL_H=1 -DHAVE_IF_NETS_H=1 -DHAVE_NET_ETHERNET_H=1 -DHAVE_PACKET_H=1 -DHAVE_SQLITE3=1 -DHAVE_IF_TUN_H=1 -DHAVE_GSL=1 -DHAVE_CRYPTO=1
NS3DIR = $(HOME)/ns3/include/ns3.29
NS3LIBDIR = $(HOME)/ns3/lib
NS3LIBS = $(shell ls $(NS3LIBDIR) | grep '\.so' | sed 's/lib/-l/' | sed 's/\.so//')
NS3LIBFLAGS = -L $(NS3LIBDIR) -Wl,-Bstatic -Wl,-Bdynamic -Wl,--no-as-needed -Wl,-rpath=$(NS3LIBDIR) $(NS3LIBS)

CXXOPT   = -pthread -I $(NS3DIR) $(NS3LIBFLAGS)
CXXFLAGS = -O3 -pthread -Wall -Werror -std=c++11 -fPIC -I $(NS3DIR) $(NS3FLAGS)

LD      = $(CXX)
LDFLAGS = $(CXXOPT)

OBJECTS = model/mpi-node.o helper/mpi-node-helper.o helper/topology-gen.o helper/parser.o model/simpi-event.o model/mpi-header.o model/address-map.o

all: simulator

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^ -o $@

model/mpi-node.o: model/mpi-node.cpp model/mpi-node.h model/address-map.h model/mpi-header.h model/simpi-event.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper/mpi-node-helper.o: helper/mpi-node-helper.cpp helper/mpi-node-helper.h model/mpi-node.h model/simpi-event.h model/address-map.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper/topology-gen.o: helper/topology-gen.cpp helper/topology-gen.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

helper/parser.o: helper/parser.cpp helper/parser.h model/simpi-event.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

model/simpi-event.o: model/simpi-event.cpp model/simpi-event.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

model/mpi-header.o: model/mpi-header.cpp model/mpi-header.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

model/address-map.o: model/address-map.cpp model/address-map.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

simulator.o: simulator.cpp model/mpi-node.h model/mpi-header.h helper/mpi-node-helper.h helper/parser.h helper/topology-gen.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

simulator: simulator.o $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

test-simulator.o: simulator.cpp model/mpi-node.h model/mpi-header.h helper/mpi-node-helper.h helper/parser.h helper/topology-gen.h
	$(CXX) $(CXXFLAGS) -DTEST_SIM -c $< -o $@

test-simulator: test-simulator.o $(OBJECTS)
	$(LD) $(LDFLAGS) $^ -o $@

.PHONY: all clean

clean:
	rm -f *.o $(OBJECTS) simulator test-simulator *.pcap *.tr animation.xml
