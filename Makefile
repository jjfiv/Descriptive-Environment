# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

SHELL:=/bin/bash

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
RED_EXMAXVARS = 16394 #hashtable size for counter-example finding
RED_MAXVARS = 65536 #hashtable size for reduction-candidate finding

REDFIND_SUCC = 1 # enables successor (x=y+1) literals in reduction-finding
REDFIND_FEWVARS = 1 #disables literals "x=i"/"x=max" in reduction-finding
REDFIND_FV_0max = 1 #if REDFIND_FEWVARS is also set, re-enables literals "x=0" 
		    #and "x=max" (otherwise no effect).

####### end of reduction-finding options #######

#Linux, gcc
ifdef CLANG
CC = clang
CXX = clang++
else
CC = gcc
CXX = g++
endif
WCFLAGS = -Wextra -ansi -pedantic

OCFLAGS = -O3 -fomit-frame-pointer #-g #-pg#-s 
#OCFLAGS = -O3 -g 
#OCFLAGS = -O0 -g

OCFLAGS += -DRED_MAXVARS=$(RED_MAXVARS) -DRED_EXMAXVARS=$(RED_EXMAXVARS)

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

# byacc and flex's output doesn't look good with -Wall -ansi -pedantic :P
CFL = ${OCFLAGS}

CFLAGS = ${CFL} ${WCFLAGS} -Wall 

INCLUDES = -Iinclude  -I.

SAT_LIB:=./extern/minisat2/build/release/lib/libminisat.a
SATBIND_LIB:=./extern/minisat-c/build/release/lib/libminisat-c.a
SAT_HEADER:=./extern/minisat-c/minisat/simp/SimpSolver.h
LPATH= -L$(dir $(SATBIND_LIB)) -L$(dir $(SAT_LIB))
LIBS = -lminisat-c -lminisat -lm #pow

# define the C source files

EXTERN_SRCS := extern/limboole/limboole.c extern/solver/solver.c extern/hash/hash.c 

LEX_SRCS := y.tab.c lex.yy.c 
REDFIND_SRCS := $(shell ls redfind/*.c)
LOGIC_SRCS := $(shell ls logic/*.c)

CORE_SRCS := $(shell ls src/*.c)

SRCS := $(LOGIC_SRCS) ${REDFIND_SRCS} ${CORE_SRCS} ${EXTERN_SRCS} $(LEX_SRCS)

OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = de 

.PHONY: clean all
.DEFAULT: all

all:	$(MAIN)

# copy minisat2 headers to minisat-c
$(SAT_HEADER): $(SAT_LIB)
	cp -R ./extern/minisat2/minisat ./extern/minisat-c/minisat

$(SAT_LIB):
	$(MAKE) -C ./extern/minisat2

$(SATBIND_LIB): $(SAT_LIB) $(SAT_HEADER)
	$(MAKE) -C ./extern/minisat-c lr MINISAT_LIB=../../$(MINISAT_LIB)


$(MAIN): $(SAT) $(SATBIND_LIB) lex.yy.c $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(MAIN) ${OBJS} $(LPATH) $(LIBS) 

.cpp.o:
	${CXX} ${CFLAGS} $(INCLUDES) -o ${<:.cpp=.o} -c $<

y.tab.c:
	yacc -d soe.y

y.tab.h:
	yacc -d soe.y

lex.yy.c: y.tab.h
	lex soe.l

y.tab.o: y.tab.c
	${CC} ${CFL} ${INCLUDES} -c -o y.tab.o y.tab.c

lex.yy.o: lex.yy.c
	${CC} ${CFL} ${INCLUDES} -c -o lex.yy.o lex.yy.c

.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -o ${<:.c=.o} -c $<

clean:
	$(RM) -r ${OBJS} $(MAIN) ./redfind/redfind*.o
	$(RM) -rf ./extern/minisat2/build
	$(RM) -rf ./extern/minisat-c/build
	$(RM) -rf ./extern/minisat-c/minisat
	$(RM) -rf y.tab.c y.tab.h lex.yy.c

