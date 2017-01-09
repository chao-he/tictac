CXXFLAGS = -std=c++11 -g -O2 -I /home/hechao/local/include -I.
LDFLAGS = /usr/local/lib/libuv.a /usr/local/lib/libs2.a \
					-pthread -lrt -lprotobuf -lglog -lgflags -lleveldb \
					-lcrypto -lboost_program_options -ltcmalloc -lunwind

SRCS = $(wildcard example/*.proto) $(wildcard example/*.cc) $(wildcard core/*.cc) 
OBJS = $(subst .proto,.pb.o,$(subst .cc,.o,$(SRCS)))

EXTRA_SRCS = $(subst .rl,.rg.h,$(wildcard core/*.rl))

serv: $(EXTRA_SRCS) $(OBJS) 
	g++ -o $@ $(OBJS) $(LDFLAGS)

%.pb.cc: %.proto
	protoc --cpp_out=. $^

%.pb.o: %.pb.cc
	g++ $(CXXFLAGS)  -c -o $@ $^

%.rg.h: %.rl
	ragel -G2 $< -o $@

clean:
	@rm -f core/*.o core/*.pb.* core/*.rg.* example/*.o example/*.pb.* core/*.rg.*
