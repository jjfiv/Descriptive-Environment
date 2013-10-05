Descriptive-Environment (DE)
===

This is the DescriptiveEnvironment (DE) described in Carmosino,
Immerman, Jordan, "Experimental Descriptive Complexity" (2012).
We encourage anyone to use and extend it, and send us comments,
examples, questions, improvements, etc.

To build the environment, try typing "make".  Various platforms
are supported in the Makefile.  We develop and test DE on POSIX
systems, but it is portable.

After building the system, type "./de" to run it.  You can then
use the examples from our paper, or the examples included in the
file "tests".  All commands end with a period and newline.

Currently, DE supports the following.

 - Defining vocabularies: id is new vocabulary{R1:2, R2:3, c1, c2}.
   This example declares a vocabulary called "id", that has a binary
   predicate symbol "R1", a ternary predicate symbol "R2", and 
   constant symbols "c1" and "c2"

 - Defining structures: id2 is new structure{id, 10, R1:2 is \phi,
   R2:3 is \psi, c1 is 0, c2 is 9}.
   This example defines "id2" to be a structure with vocabulary "id",
   and universe size 10 (the integers 0,1,...,9).  The relation R1
   is the set of tuples (x1,x2) such that \phi holds, R2 is the set
   of tuples (x1,x2,x3) such that \psi holds, c1 is 0 and c2 is 9.
   \phi and \psi are placeholders, please substitute formulas with
   free variables (x1,x2) and (x1,x2,x3).

 - Defining boolean queries on structures with a given vocabulary:
   id3 is new bquery{vocab, \phi}.

 - Defining k-ary reductions (or queries) from structures of vocab1 to 
   structures of vocab2: id is new reduction{vocab1, vocab2, 2, 
   of vocab2: id is new reduction{vocab1, vocab2, 2, R:4 is \phi, s is \psi...}.
   "query" is treated as a synonym for "reduction".  See the paper or
   references for details.

 - Evaluating these queries: sosat(form1).  id2 is red(id1). etc.

 - Formulas are written in ESO(TC) (existential second-order logic with
   reflexive commutative transitive closure).  LFP/IFP/PFP will be provided
   in the future.  \E is the existential quantifier, \A is universal.
   TC[x,y:\phi(x,y)](s,t) evaluates the TC of \phi with variables x and y on
   the pair (s,t).  Logical or (|), and (&), not (~), implies (->), 
   iff (<->) and xor (^) are implemented.  Ordering (<, <=), equality, and
   arithmetic (+, -, *) are available.  Precision is machine-dependent, but
   generally results are not truncated to fit in the structure's universe.
   Of course, R(max+1) and R(-1) are false for all relations.  \t is true
   and \f is false.
   There is a colon following the quantified variables, before the formula.
   The special constant "max" is the largest element of the universe.

 - Implicitly recursive definitions are evaluated in an undefined way and
   should not be used (i.e. E:2 is E(x1-2, x2+2))

 - Loading undirected graphs in DIMACS ASCII format (s is 0 and t is max):
   anna is load("/path/to/dimacs/anna.col").

 - Saving objects (only structures now) to files:
   save(structure,"/path/to/structure.str").

 - Finding models (for first-order formulas only: no TC or SO) using Mace4:
   id is mace(vocab,phi).
   Note: the user must obtain Mace4 and put the executable 'mace4' in
   their path.
   There is an optional third argument:
   id is mace(vocab, phi, 40).
   will ask mace to look for a model for at most 40 seconds.

 - Simple reduction finding, given Boolean queries p1 and p2:
   redfind(p1,p2,2,3,4).
   This searches for a quantifier-free reduction from p1 to p2 that obeys
   the following restrictions.
     1) k=2 (given as the first optional argument)
     2) all formulas in the reduction are in DNF, with c=3 clauses in each.
        (c is given as the second optional argument)
     3) the reduction is only required to be correct on structures of size
        n=4 (the last optional argument).
   Note that if any optional arguments are given, then all of them must be.
   redfind(p1,p2).
   Without optional arguments, uses k=1, c=1, n=3.
   Searching a range n1<=n<=n2 of counter-example sizes is also possible:
   redfind(p1,p2,2,3,4,5) uses k=2, c=3, n1=4, n2=5.
   There is a compile-time choice between using a MiniSat-2 compatible
   SAT solver (choosing from MiniSat-2, GlueMiniSat or CryptoMiniSat)
   or using BDDs via CUDD for reduction-finding.

The following builtins are available:
 - sat is a builtin vocabulary, {P:2(c,v), N:2(c,v)}.
 - graph is a builtin vocabulary, {E:2, s, t}.
 - minisat( ) is a boolean query on structures with vocabulary sat that
   uses the Minisat-C SAT solver.
 - threecolorwithsat is a boolean query on graphs using a reduction to sat
   and the Minisat-C SAT solver.
 - minisat2( ) and threecolorwithsat2( ) are equivalent, but use newer
   MiniSat-2 compatible solvers (MiniSat-2, GlueMiniSat and CryptoMiniSat
   are compile-time choices).
 - help is (an as of yet unhelpful) command (help.)
Examples are in the "tests" file.

I welcome questions and comments, skip@ist.hokudai.ac.jp
12/14/12