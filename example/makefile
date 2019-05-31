PREFIX=../output
CXX=g++
INCPATH=-I. -I$(PREFIX)/include
CXXFLAGS= -std=c++11 $(INCPATH)
LDFLAGS= -lprotobuf -lpthread

PROTO=$(wildcard *.proto)
PROTO_SRC=$(patsubst %.proto, %.pb.cc, $(PROTO))
PROTO_HEADER=$(patsubst %.proto, %.pb.h, $(PROTO))
PROTO_OBJ=$(patsubst %.cc, %.o, $(PROTO_SRC))

LIB=$(PREFIX)/lib/drpc.a

BIN=testserver testclient

build: $(BIN)

clean:
		rm -f $(BIN) *.o *.pb.*

rebuild: clean all

$(PROTO_OBJ): $(PROTO_HEADER)

%.pb.cc %.pb.h: %.proto
		protoc -I=. --cpp_out=. $<

testserver: $(PROTO_OBJ) testserver.o
		$(CXX) $^ -o $@ $(LIB) $(LDFLAGS)

testclient: $(PROTO_OBJ) testclient.o
		$(CXX) $^ -o $@ $(LIB) $(LDFLAGS)

%.o: %.cc
		$(CXX) $(CXXFLAGS) -c $< -o $@

