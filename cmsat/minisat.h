/***************************************************************************************[minisat.h]
Copyright (c) 2008-2011, Niklas Sorensson
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

/* chj - minimal version for CryptoMiniSat */
#ifndef Minisat_C_Bindings_h
#define Minisat_C_Bindings_h

/* SolverTypes:
*/
typedef struct minisat_solver_t minisat_solver;
#ifdef Minisat_Opaque
#define opaque(x) struct { x f; }
#else
#define opaque(x) x
#endif
typedef opaque(int) minisat_Var;
typedef opaque(int) minisat_Lit;
typedef opaque(int) minisat_lbool;
typedef opaque(int) minisat_bool; /* Only for clarity in the declarations below (this is just a plain c-bool). */
#undef opaque

/* Constants: (can these be made inline-able?)
*/

extern const minisat_lbool minisat_l_True;
extern const minisat_lbool minisat_l_False;
extern const minisat_lbool minisat_l_Undef;


minisat_solver* minisat_new             (void);
void            minisat_delete          (minisat_solver* s);
             
minisat_Var     minisat_newVar          (minisat_solver *s);
minisat_Lit     minisat_newLit          (minisat_solver *s);
             
minisat_Lit     minisat_mkLit           (minisat_Var x);
minisat_Lit     minisat_negate          (minisat_Lit p);
                                    
minisat_Var     minisat_var             (minisat_Lit p);
             
void            minisat_addClause_begin (minisat_solver *s);
void            minisat_addClause_addLit(minisat_solver *s, minisat_Lit p);
minisat_bool    minisat_addClause_commit(minisat_solver *s);
             
minisat_bool    minisat_solve           (minisat_solver *s, int len, minisat_Lit *ps);
             
minisat_lbool   minisat_modelValue_Lit  (minisat_solver *s, minisat_Lit p);

/* SimpSolver methods: */
void            minisat_setFrozen       (minisat_solver* s, minisat_Var v, minisat_bool b);

#endif
