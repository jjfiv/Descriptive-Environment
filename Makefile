# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

SHELL:=/bin/bash
RM:=rm

####### start of reduction-finding options #######

#REDFIND_DEBUG = 1#comment out to exclude redfind debugging output
#REDFIND_DEBUG2 = 1#comment out to exclude extra redfind debugging output

#REDFIND_OLDRANGE=1 # when looking at a range of sizes, never reset to n1
#REDFIND_MINEX= 1  # comment out to disable greedily-minimizing examples
#REDFIND_MAXEX= 1  # comment out to disable greedily-maximizing examples
REDFIND_ALTEX= 1  # comment out to disable alternating minimizing,maximizing
		   # examples
#REDFIND_MINRED= 1 # comment out to disable greedily-minimizing hypotheses
#REDFIND_IGNTF= 1  # when minimizing hypotheses, it's okay to add \f to
		  # clauses (try to remove clauses without lengthening others)
#REDFIND_IGNALL= 1 # not recommended, too slow.
		  # different way to minimize hypotheses, okay to lengthen
		  # clauses.  start by forbidding succ, then others from
		  # one clause at a time. leave only 'essential'
#REDFIND_RANDEX=20 # when getting counter-examples, choose one of this many 
		  # almost-randomly. comment out to disable
  # enabling REDFIND_MINEX and REDFIND_RANDEX chooses the random example first,
  # then minimizes it -- not a random greedily-minimized example

### Sizes for hashtables used in reduction-finding.
  # increase one/both of these sizes (probably RED_MAXVARS) if you get 
  # "Assertion failed: (hash->nodecount < hash->maxcount)"
  # RED_MAXVARS is large because we use entries for each
  # bit of output structures, which tend to be large in the parameters

REDFIND_SUCC = 1 # enables successor (x=y+1) literals in reduction-finding
REDFIND_FEWVARS = 1 #disables literals "x=i"/"x=max" in reduction-finding
REDFIND_FV_0max = 1 #if REDFIND_FEWVARS is also set, re-enables literals "x=0" 
		    #and "x=max" (otherwise no effect).

####### end of reduction-finding options #######

#Linux, gcc
ifdef CLANG
CC := clang
CXX := clang++
else
CC := gcc
CXX := g++
endif

OCFLAGS:=-O3 -fomit-frame-pointer
#OCFLAGS = -O3 -g 
#OCFLAGS = -O0 -g

ifdef REDFIND_SUCC
OCFLAGS += -DREDFIND_SUCC
endif

ifdef REDFIND_DEBUG
OCFLAGS += -DREDFIND_DEBUG
endif
ifdef REDFIND_DEBUG2
OCFLAGS += -DREDFIND_DEBUG2
endif
ifdef REDFIND_FEWVARS
OCFLAGS += -DREDFIND_FEWVARS
endif
ifdef REDFIND_FV_0max
OCFLAGS += -DREDFIND_FV_0max
endif
ifdef REDFIND_OLDRANGE
OCFLAGS += -DREDFIND_OLDRANGE
endif

ifdef REDFIND_MINEX
OCFLAGS += -DRF_minex=1 -DREDFIND_EXIMPROVE
else
ifndef CRYPTOMINISAT
OCFLAGS += -DRF_maxex=1 -DREDFIND_EXIMPROVE
else
ifdef REDFIND_ALTEX
OCFLAGS += -DRF_altex=1 -DREDFIND_EXIMPROVE
endif
endif
endif

ifdef REDFIND_MINRED
#ifndef CRYPTOMINISAT
OCFLAGS += -DREDFIND_MINRED -DREDFIND_REDIMPROVE
#endif
endif

ifdef REDFIND_IGNALL
OCFLAGS += -DRED_IGNALL
else
ifdef REDFIND_IGNTF
OCFLAGS += -DRED_IGNTF
endif
endif
ifdef REDFIND_RANDEX
OCFLAGS += -DREDFIND_RANDEX=$(REDFIND_RANDEX)
endif


CFLAGS:=${OCFLAGS} ${WCFLAGS} -Wall -Wno-strict-aliasing -std=c11
CXXFLAGS:=${OCFLAGS} ${WCFLAGS} -Wall -Wno-strict-aliasing -std=c++11

MY_INCLUDES:=-Iinclude -Isrc

SAT_LIB:=./extern/minisat2/build/release/lib/libminisat.a
SATBIND_LIB:=./extern/minisat-c/build/release/lib/libminisat-c.a
SAT_HEADER:=./extern/minisat-c/minisat/simp/SimpSolver.h
LPATH:= -L$(dir $(SATBIND_LIB)) -L$(dir $(SAT_LIB))
LIBS:= -lminisat-c -lminisat -lm

# define the C source files

EXTERN_SRCS := extern/limboole/limboole.c extern/solver/solver.c extern/hash/hash.c 

LEX_SRCS := src/soe_parse.tab.c src/soe_lex.tab.c src/soe_parse.tab.h
REDFIND_SRCS := $(shell ls redfind/*.c)
LOGIC_SRCS := $(shell ls logic/*.c)

CORE_SRCS := src/cmd.o \
	src/env.o \
	src/file.o \
	src/help.o \
	src/parse.o \
	src/reduc.o \
	src/usemace.o \
	src/util.o

MAIN_O := src/main.o
SRCS := $(LOGIC_SRCS) ${REDFIND_SRCS} ${CORE_SRCS} ${EXTERN_SRCS} ${LEX_SRCS}
OBJS := $(SRCS:.c=.o)

TEST_SRCS := $(shell ls test/*.cc)
TEST_OBJS := $(TEST_SRCS:.cc=.o)


# define the executable file 
MAIN = de 
TEST = test/test_de

.PHONY: clean all depclean generate compile tags run test run_test
.DEFAULT: all

# generate files, and then run main
all: generate 
	@$(MAKE) --no-print-directory compile

test: generate
	@$(MAKE) --no-print-directory compile
	@$(MAKE) --no-print-directory run_test

generate: $(LEX_SRCS) $(SATBIND_LIB)
compile: $(MAIN)

# copy minisat2 headers to minisat-c
$(SAT_HEADER): $(SAT_LIB)
	cp -R ./extern/minisat2/minisat ./extern/minisat-c/minisat

$(SAT_LIB):
	$(MAKE) -C ./extern/minisat2

$(SATBIND_LIB): $(SAT_LIB) $(SAT_HEADER)
	$(MAKE) -C ./extern/minisat-c lr MINISAT_LIB=../../$(MINISAT_LIB)

$(MAIN): $(SATBIND_LIB) $(OBJS) ${MAIN_O}
	$(CXX) $(CXXFLAGS) $(MY_INCLUDES) -o $(MAIN) ${OBJS} ${MAIN_O} $(LPATH) $(LIBS) 

$(TEST): $(SATBIND_LIB) $(OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) $(MY_INCLUDES) -o $(TEST) ${TEST_OBJS} ${OBJS} $(LPATH) $(LIBS) 


# make C and C++ compiles depend on lex and yacc being run first
%.o:%.cc
	${CXX} -c ${CXXFLAGS} $(MY_INCLUDES) $< -o $@

%.o:%.c
	$(CC) -c $(CFLAGS) $(MY_INCLUDES) $< -o $@

%.tab.c:%.y
	bison -d $< -o $@

%.tab.h:%.y
	bison -d $< -o $@

%.tab.c:%.l
	flex -o $@ $<

mace4/p9m4-v05.tar.gz:
	wget -P mace4 http://www.cs.unm.edu/~mccune/prover9/gui/p9m4-v05.tar.gz
mace4/p9m4-v05/bin/mace4: mace4/p9m4-v05.tar.gz
	tar -C mace4 -xf $<

run: all mace4/p9m4-v05/bin/mace4
	@PATH=$(PATH):mace4/p9m4-v05/bin rlwrap ./$(MAIN)

run_test: $(TEST) mace4/p9m4-v05/bin/mace4
	diff test/expected.out <(PATH=$(PATH):mace4/p9m4-v05/bin ./$(TEST))
	

tags:
	ctags src/* include/* logic/* redfind/* extern/solver/* extern/hash/* extern/limboole/* test/*

depclean: clean
	$(RM) -rf ./extern/minisat2/build
	$(RM) -rf ./extern/minisat-c/build
	$(RM) -rf ./extern/minisat-c/minisat

clean:
	$(RM) -rf ${OBJS} $(MAIN)
	$(RM) -rf $(LEX_SRCS)

