#===============================================================================
# User Options
#===============================================================================

COMPILER    = gnu
MPI         = no
OPENMP      = yes
OPTIMIZE    = no
DEBUG       = yes
PROFILE     = no
PAPI        = no
BENCHMARK   = no

#===============================================================================
# Program name & source code list
#===============================================================================

program = parallel_hash_map

source = \
main.cpp 

obj = $(source:.cpp=.o)

#===============================================================================
# Sets Flags
#===============================================================================

# Regular gcc Compiler
ifeq ($(COMPILER),gnu)
  CC = g++
endif

# intel Compiler
ifeq ($(COMPILER),intel)
  CC = icc
endif

# IBM XL Compiler
ifeq ($(COMPILER),ibm)
  CC = mpicc
endif

# BG/Q gcc Cross-Compiler
ifeq ($(MACHINE),bluegene)
  CC = mpicc
endif

# Standard Flags
CFLAGS := -std=c++11

# Linker Flags
LDFLAGS = -lm

# Debug Flags
ifeq ($(DEBUG),yes)
  CFLAGS += -g
endif

# Profiling Flags
ifeq ($(PROFILE),yes)
  CFLAGS += -pg
endif

# Optimization Flags
ifeq ($(OPTIMIZE),yes)
ifeq ($(COMPILER),gnu)
  CFLAGS += -Ofast -ffast-math -ftree-vectorize -msse2
endif
ifeq ($(COMPILER),intel)
  CFLAGS += -O3 -xhost -ansi-alias -no-prec-div -DINTEL
endif
ifeq ($(COMPILER),ibm)
  CFLAGS += -O5 -qhot -qsimd=auto -qalias=ansi:allptrs -qarch=qp -DIBM
endif
endif

# PAPI source (you may need to provide -I and -L pointing
# to PAPI depending on your installation
ifeq ($(PAPI),yes)
  CFLAGS += -DPAPI
  LDFLAGS += -lpapi
  OPENMP = yes
endif

# MPI
ifeq ($(MPI),yes)
  CC = mpicc
  CFLAGS += -DMPI
endif

# OpenMP
ifeq ($(OPENMP),yes)
ifeq ($(COMPILER), gnu)
	CFLAGS += -fopenmp
endif
ifeq ($(COMPILER), intel)
	CFLAGS += -openmp
endif
ifeq ($(COMPILER), ibm)
	CFLAGS += -qsmp
endif
   	CFLAGS += -DOPENMP
endif


#===============================================================================
# Targets to Build
#===============================================================================

$(program): $(obj) parallel_hash_map.h
	$(CC) $(CFLAGS) $(obj) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(program) $(obj)

edit:
	vim -p $(source) parallel_hash_map.h

run:
	./$(program)
