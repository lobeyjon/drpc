PREFIX=output

CXX=g++
CXXFLAGS= -std=c++11
LDFLAGS= -lprotobuf -lpthread 

LIB=drpc.a
LIB_SRC=$(wildcard src/drpc/*.cc)
LIB_OBJ=$(patsubst %.cc, %.o, $(LIB_SRC))

PROTO=$(wildcard src/drpc/*.proto)
PROTO_SRC=$(patsubst %.proto, %.pb.cc, $(PROTO))
PROTO_HEADER=$(patsubst %.proto, %.pb.h, $(PROTO))
PROTO_OBJ=$(patsubst %.cc, %.o, $(PROTO_SRC))

PUB_INC=src/drpc/*.h $(PROTO)

all: build

clean:
		rm -f $(LIB) $(LIB_OBJ) $(PROTO_OBJ) $(PROTO_HEADER) $(PROTO_SRC)
		rm -rf $(PREFIX)

rebuild: clean all

$(PROTO_OBJ): $(PROTO_HEADER)

$(LIB_OBJ): $(PROTO_HEADER)

$(LIB): $(LIB_OBJ) $(PROTO_OBJ)
		ar crs $@ $(LIB_OBJ) $(PROTO_OBJ)

%.pb.cc %.pb.h: %.proto
		protoc -I=src/drpc/ --cpp_out=src/drpc $<

%.o: %.cc
		$(CXX) $(CXXFLAGS) -c $< -o $@

build: $(LIB)
		@echo
		@echo 'Build succeed, run "make install" to install drpc to "'$(PREFIX)'".'

install: $(LIB)
		mkdir -p $(PREFIX)/include/drpc
		cp -r $(PUB_INC) $(PREFIX)/include/drpc
		mkdir -p $(PREFIX)/lib
		cp $(LIB) $(PREFIX)/lib/
		@echo
		@echo 'Install succeed, target directory is "'$(PREFIX)'".'