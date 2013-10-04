# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build executable file 'mycc'
# 'make clean'  removes all .o and executable files
#

#CC = c89
#CXX = CC

### a MiniSat2-compatible solver is required. 
  # CryptoMiniSat seems to be the most alive - jjfiv f2013
CRYPTOMINISAT = 1 #use CryptoMiniSat 2.9.5 *instead* of MiniSat2

####### start of reduction-finding options #######
####### note that the choice of MiniSat2-compatible solver affects
####### reduction-finding

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
CC = gcc
CXX = g++
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
ifndef CRYPTOMINISAT
OCFLAGS += -DRF_minex=1 -DREDFIND_EXIMPROVE
endif
else
ifdef REDFIND_MAXEX
ifndef CRYPTOMINISAT
OCFLAGS += -DRF_maxex=1 -DREDFIND_EXIMPROVE
endif
else
ifdef REDFIND_ALTEX
ifndef CRYPTOMINISAT
OCFLAGS += -DRF_altex=1 -DREDFIND_EXIMPROVE
endif
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

#LFLAGS = -L../lib

ifdef CRYPTOMINISAT
LIBS = -L./cmsat/build/release/lib -lminisat-c ./cmsat/cmsat/cmsat/.libs/libcryptominisat.a -lz -fopenmp
else
LIBS = -L./minisat2/build/release/lib -lminisat-c -L./minisat2/minisat/build/release/lib/ -lminisat -lm #pow
endif
endif

# define the C source files

EXTERN_SRCS := extern/minisat/solver.c \
	             extern/limboole/limboole.c

CORE_SRCS := y.tab.c lex.yy.c 

SRCS = reduc/reduc.c cmd/cmd.c file/file.c hash/hash.c init/init.c parse/parse.c env/env.c help/help.c logic/eval.c test/main.c logic/interp.c logic/relation.c util/util.c logic/constant.c logic/tuple.c mace/usemace.c redfind/getex.c ${REDFIND_SOURCE} logic/check.c ${CORE_SRCS} ${EXTERN_SRCS}

OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = de 

ifdef GLUEMINISAT
SATBIND = ./glueminisat/build/release/lib/libminisat-c.a
SAT = ./glueminisat/minisat/build/release/lib/libglueminisat.a
else
ifdef CRYPTOMINISAT
SATBIND = ./cmsat/build/release/lib/libminisat-c.a 
SAT = ./cmsat/cmsat/cmsat/.libs/libcryptominisat.a 
else
SATBIND = ./extern/minisat2/build/release/lib/libminisat-c.a
SAT = ./extern/minisat2/minisat/build/release/lib/libminisat.a
endif
endif

.PHONY: depend clean

all:	${CUDD}  $(MAIN)

./cmsat/build/release/lib/libminisat-c.a:
	cd cmsat; ${MAKE} static; cd ..

./cmsat/cmsat/cmsat/.libs/libcryptominisat.a:
	cd cmsat/cmsat; ./configure; ${MAKE}; cd ../..

./glueminisat/build/release/lib/libminisat-c.a:
	cd glueminisat; ${MAKE} static; cd ..

./extern/minisat2/build/release/lib/libminisat-c.a:   
	cd minisat2; ${MAKE} static; cd ..

./extern/minisat2/minisat/build/release/lib/libminisat.a:
	cd minisat2/minisat; ${MAKE} lr; cd ../..

./glueminisat/minisat/build/release/lib/libglueminisat.a:
	cd glueminisat/minisat; ${MAKE} lr; cd ../..

./cudd/cudd/libcudd.a:
	cd cudd; ${MAKE}; cd ..

de:   $(CUDD) $(SAT) $(SATBIND) $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -o $(MAIN) ${OBJS} $(LFLAGS) $(LIBS) 

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
	$(RM) -r ${OBJS} $(MAIN) ./redfind/redfind*.o ./extern/minisat2/build ./extern/minisat2/minisat/build ./cmsat/build ./glueminisat/build ./glueminisat/minisat/build
	cd cudd; ${MAKE} distclean; cd ..
	cd glueminisat/minisat; ./clean.sh; cd ../..
	cd cmsat/cmsat; ${MAKE} distclean; cd ../..

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
