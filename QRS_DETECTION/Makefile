# Makefile for QRS_DETECTION
BIN_DIR=../bin
LIB_DIR=../bin
INCLUDE_DIR=../SEAL
SRCS=main.cpp
QRSRUN=$(BIN_DIR)/qrs_detection
CXX=g++
CXXFLAGS=-march=native -O3 -std=c++11
LDFLAGS=

.PHONY : all clean

all : $(QRSRUN)

$(QRSRUN) : $(SRCS)
	@-mkdir -p $(dir $@)
	$(CXX) $(SRCS) $(CXXFLAGS) $(LDFLAGS) $(addprefix -I,$(INCLUDE_DIR)) $(addprefix -L,$(LIB_DIR)) -lseal -o $@

clean :
	@-rm -f $(OBJS) $(QRSRUN)
