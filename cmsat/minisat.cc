/**************************************************************************************[minisat.cc]
Copyright (c) 2008-2010, Niklas Sorensson
              2008, Koen Claessen

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#include <stdlib.h>
#include "cmsat/cmsat/Solver.h"

using namespace CMSat;

struct minisat_solver_t : public Solver { 
    vec<Lit> clause;
    vec<Lit> assumps;
};

extern "C" {

#include "minisat.h"

// This implementation of lbool may or not may be an exact mirror of the C++ implementation:
//
extern const minisat_lbool minisat_l_True  = 1;
extern const minisat_lbool minisat_l_False = 0;
extern const minisat_lbool minisat_l_Undef = -1;

static inline minisat_lbool toC(lbool a)
{
    return a == l_True  ? minisat_l_True
         : a == l_False ? minisat_l_False
         : minisat_l_Undef;
}

static inline lbool fromC(minisat_lbool a)
{
    return a == minisat_l_True  ? l_True
         : a == minisat_l_False ? l_False
         : l_Undef;
}


// Solver C-API wrapper functions:
//
minisat_solver* minisat_new             (void){ return new minisat_solver_t(); }
void          minisat_delete          (minisat_solver *s){ delete s; }
minisat_Var   minisat_newVar          (minisat_solver *s){ return s->newVar(); }
minisat_Lit   minisat_newLit          (minisat_solver *s){ return toInt(mkLit(s->newVar())); }
minisat_Lit   minisat_mkLit           (minisat_Var x){ return toInt(mkLit(x)); }
minisat_Lit   minisat_negate          (minisat_Lit p){ return toInt(~toLit(p)); }
minisat_Var   minisat_var             (minisat_Lit p){ return var(toLit(p)); }
void         minisat_addClause_begin (minisat_solver *s){ s->clause.clear(); }
void         minisat_addClause_addLit(minisat_solver *s, minisat_Lit p){ s->clause.push(toLit(p)); }
int          minisat_addClause_commit(minisat_solver *s){ return s->addClause(s->clause); }

// NOTE: Currently these run with default settings for implicitly calling preprocessing. Turn off
// before if you don't need it. This may change in the future.
minisat_lbool minisat_modelValue_Lit (minisat_solver *s, minisat_Lit p){ return toC(s->modelValue(toLit(p))); }

// SimpSolver methods:
void         minisat_setFrozen       (minisat_solver* s, minisat_Var v, minisat_bool b) { /* s->setFrozen(v, b);*/ }

// Convenience functions for actual c-programmers (not language interfacing people):
//
int  minisat_solve(minisat_solver *s, int len, minisat_Lit *ps)
{
    s->assumps.clear();
    for (int i = 0; i < len; i++)
        s->assumps.push(toLit(ps[i]));
    return s->solve(s->assumps)==l_True;
}

}
