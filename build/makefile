CPP=g++
OPTLEVEL=2
WARNFLAGS=-Wall -Wstrict-aliasing -Werror
INCLUDE=-I../include -I$(OPENCV)/include/opencv4
OPTFLAGS=-msse2 -mfpmath=sse
CONFIGFLAGS=-DUSE_OPENCV -DUSE_CDT #-DUSE_ZLIB #-D_DEBUG 

# -std=c++11 is to make sure building on macOS works
CPPFLAGS=-fpack-struct=8 -std=c++11

CPPFLAGS+=$(WARNFLAGS) $(INCLUDE) -O$(OPTLEVEL) $(OPTFLAGS) $(CONFIGFLAGS) # -g -pg 
SOURCES=$(wildcard ../src/core/*.cc) $(wildcard ../src/math/*.cc) $(wildcard ../src/image/*.cc) $(wildcard ../src/system/*.cc) $(wildcard ../src/other/*.cc) ../external/CDT/CDT.cc
INCLUDES=$(wildcard ../include/sbl/core/*.h) $(wildcard ../include/sbl/math/*.h) $(wildcard ../include/sbl/image/*.h) $(wildcard ../include/sbl/system/*.h) $(wildcard ../include/sbl/other/*.h) ../external/CDT/CDT.h
OBJECTS=$(SOURCES:.cc=.o)
LIBRARY=../lib/libsbl.a

.PHONY: depend clean valgrind

all: $(SOURCES) $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar rcs $(LIBRARY) $(OBJECTS)

.cc:
	$(CPP) $(CPPFLAGS) -c $@.cc -o $@.o

clean:
	rm -f ../src/*/*.o ../external/*/*.o $(LIBRARY)

depend: $(SOURCES)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
