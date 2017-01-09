CXXFLAGS = -std=c++11 -fPIC -g -O2 -I /home/hechao/local/include
LDFLAGS = /usr/local/lib/libuv.a /usr/local/lib/libs2.a \
					-pthread -lrt -lprotobuf -lglog -lgflags -lleveldb \
					-lcrypto -lboost_program_options -ltcmalloc -lunwind

SRCS = $(wildcard *.proto) $(wildcard *.cc) 
OBJS = $(subst .proto,.pb.o,$(subst .cc,.o,$(SRCS)))

EXTRA_SRCS = $(subst .rl,.rg.h,$(wildcard *.rl))

libnet.a: $(EXTRA_SRCS) $(OBJS) 
	ar crvf $@ $^

%.pb.cc: %.proto
	protoc --cpp_out=. $^

%.pb.o: %.pb.cc
	g++ $(CXXFLAGS)  -c -o $@ $^

%.rg.h: %.rl
	ragel -G2 $< -o $@

clean:
	@rm -f *.o serv *.pb.* *.rg.*
