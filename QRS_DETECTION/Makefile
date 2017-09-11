# Makefile for QRS_DETECTION
LIB_DIR=../bin
INCLUDE_DIR=../SEAL
CXX=g++
CXXFLAGS=-march=native -O3 -std=c++11

.PHONY : all clean

dualslope : createdirs obj/he.o obj/dualslope.o obj/helper_functions.o obj/main.o
	$(info )
	$(info Compiling dualslope...)
	$(CXX) $(CXXFLAGS) obj/*.o $(addprefix -I,$(INCLUDE_DIR)) $(addprefix -L,$(LIB_DIR)) -lseal -o $@ 

createdirs :
	mkdir -p obj

obj/he.o : he.cpp he.h
	$(CXX) $(CXXFLAGS) -c he.cpp $(addprefix -I,$(INCLUDE_DIR)) -o obj/he.o

obj/dualslope.o : dualslope.cpp dualslope.h
	$(CXX) $(CXXFLAGS) -c dualslope.cpp $(addprefix -I,$(INCLUDE_DIR)) -o obj/dualslope.o

obj/helper_functions.o : helper_functions.cpp helper_functions.h
	$(CXX) $(CXXFLAGS) -c helper_functions.cpp $(addprefix -I,$(INCLUDE_DIR)) -o obj/helper_functions.o

obj/main.o : main.cpp
	$(CXX) $(CXXFLAGS) -c main.cpp $(addprefix -I, $(INCLUDE_DIR)) -o obj/main.o

dsr : dualslope
	./dualslope

clean :
	@-rm -rf obj *.exe *.o

help :
	@echo make dualslope - Compiles the project source code into an executable "dualslope"
	@echo make dsr - Compiles the project source code and runs it
	@echo make clean - Removes all executables .exe and objects .o 
