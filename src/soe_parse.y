/* ISC LICENSE */
/* soe.y
 * Skip Jordan
 * Basic input to yacc for SO\E logic
 *
 * chj	10/19/06	Created.
 * chj	11/13/06	Support for first-order quantifiers, and fork and
 *			add support for assignment commands for DE.
 * chj	11/23/06	add transitive closure
 * chj  11/28/06	reductions and boolean queries
 * chj  11/1/11		a is load(filename)
 * chj  11/3/11		save(id,filename)
 * chj	11/11/11	IFP, and "query" as synonym for "reduction"
 * chj	12/1/11		a is mace(vocab, formula) to use Mace4 (model-finder)
 * chj	12/5/11		XOR,draw
 * chj	 4/3/12		Reduction finding
 * chj	11/12/12	support searching range of sizes in redfind
 * chj	 1/24/13	initial stub for formula distance
 * chj	 1/29/13	loadstring
 */

%{
  #include "parse.h"
  extern void yyerror(const char *msg);
  extern int yylex(void);
%}

%union {
  int integer;
  char *string;
  struct Node *n;
  }

%start ROOT

%token <string> CONSTANT
%token <string> VAR
%token <integer> NUMBER
%token MULT
%token PLUS
%token MINUS

%token AND
%token OR
%token NOT
%token IFF
%token IMPLIES
%token XOR
%token LT
%token LTE
%token EQUALS
%token NEQUALS
%token TRUE
%token FALSE
%token <string> PRED
%token FORALL
%token EXISTS
%token TC
%token IFP
%token SOE

%token LPAREN
%token RPAREN
%token LBRACK
%token RBRACK
%token LBRACE
%token RBRACE

%token COMMA
%token PERIOD
%token COLON

%token ASSIGN
%token NEW
%token REDFIND
%token FD
%token VOCAB
%token STRUC

%token QUOTE
%token REDUC
%token BQUERY
%token QUERY
%token LOAD
%token LOADSTRING
%token SAVE
%token MACE
%token DRAW
%token SIZE
%token CVRELARG
%token CVCONSARG
%token CSARGS
%token CSRELDEF
%token CSRELDEFI
%token CSCONSDEF
%token ID
%token FNODEIL
%token FNODEIR

%token NL

%token <string> FILENAME

%token ERROR

%token VARLIST VLR EXPREDALL TCARGS CRRELDEF SOVARLIST LFPARGS
%token RELARGS EXCONS EXPRED APPLY CRCONSDEF ABQUERY

%token RF_RANGE

%type <n> ROOT form impform andform notform qform atomic sterm q_form varlist
%type <n> restr cmd_start form_start aterm mterm cmd assign_cmd id cmdexpr
%type <n> cvexpr csexpr cvargs csargs cvconsargs csreldefs crreldefs bquery_cmd
%type <n> csconsdefs number relargs constant var pred cvarg1 cvconsarg1
%type <n> struc_ex_cmd even_args eargs crexpr applyexpr crconsdefs cbexpr
%type <n> draw_cmd sovarlist maceexpr loadexpr save_cmd lfpargs lfpargs2
%type <n> redfindexpr fdexpr

%%

ROOT:	cmd_start NL			{cmdtree=node(0,$1,0); $$=$1;}
    ;

/* begin logic section */
form_start:   form 			{parsetree=node(0,$1,0);
          $$=$1;}
  ;

form: impform			{$$=$1;}
    ;

impform: 
       andform			{$$=$1;}
      | andform IMPLIES impform 	{$$=node(IMPLIES,$1,$3);}
      | andform IFF impform	{$$=node(IFF,$1,$3);}
     ;

andform:
       notform			{$$=$1;}
  | andform AND notform	{$$=node(AND,$1,$3);}
  | andform OR notform	{$$=node(OR,$1,$3);}
  | andform XOR notform	{$$=node(XOR,$1,$3);}
  ;

notform:
       NOT notform	{$$=node(NOT,$2,0);}
  | qform		{$$=$1;}
  ;

qform:
     q_form {$$=$1;}
  |  
  atomic	{$$=$1;}
  ;

q_form:
      FORALL varlist restr COLON notform
        {$$=qnode(FORALL,$2, $3, $5);}
  | FORALL varlist COLON notform
        {$$=qnode(FORALL,$2,  0, $4);}
  | EXISTS varlist restr COLON notform
        {$$=qnode(EXISTS, $2, $3, $5);}
  | EXISTS varlist COLON notform
        {$$=qnode(EXISTS, $2,  0, $4);}
  | TC LBRACK even_args COLON form RBRACK LPAREN relargs RPAREN
        {$$=qnode(TC, $3, $5, $8);}
  | IFP LBRACK id COMMA lfpargs COLON form RBRACK LPAREN relargs RPAREN
        {$$=fournode(IFP, $3, $5, $7, $10);}
  /* Second order existential for NPC queries */
  | EXISTS sovarlist restr COLON notform
        {$$=qnode(SOE, $2, $3, $5);}
  | EXISTS sovarlist COLON notform
        {$$=qnode(SOE, $2, 0, $4);}
  ;

sovarlist: /* use ONE SO quantifier per SO variable. */
         pred COLON number	{$$=qnode(SOVARLIST, $1, $3, 0);}
/*	| pred COLON number COMMA sovarlist
        {$$=qnode(SOVARLIST, $1, $3, $5);}
  | pred COLON number sovarlist
        {$$=qnode(SOVARLIST, $1, $3, $4);}
*/
  ;
even_args:
         aterm COMMA aterm eargs
        {$$=qnode(TCARGS,$1, $3, $4);}
  ;
eargs:
     {$$=0;}
        |COMMA aterm COMMA aterm eargs
                                {$$=qnode(TCARGS,$2,$4,$5);}
        ;
lfpargs:
       id lfpargs2
        {$$=node(LFPARGS,$1,$2);}
  ;
lfpargs2:
        {$$=0;}
  | COMMA id lfpargs	{$$=node(LFPARGS,$2,$3);}
  ;

restr:
     PERIOD form {$$=$2;}
  ;

varlist:
       VAR	{$$=vlnode(VARLIST, $1, 0);}
  | VAR COMMA varlist {$$=vlnode(VARLIST,  $1, $3);}
  | VAR varlist {$$=vlnode(VARLIST, $1, $2);}
  ;

atomic:
    aterm EQUALS aterm	{$$=node(EQUALS,$1,$3);}
  | aterm NEQUALS aterm	{$$=node(NEQUALS, $1, $3);}
  | aterm LTE aterm		{$$=node(LTE, $1, $3);}
  | aterm LT aterm		{$$=node(LT,$1, $3);}
  | LPAREN form RPAREN	{$$=$2;}
  | PRED LPAREN relargs RPAREN {$$=vlnode(PRED,$1,$3);}
  | TRUE			{$$=node(TRUE,0,0);}
  | FALSE			{$$=node(FALSE,0,0);}
  ;

relargs:
    aterm COMMA relargs	{$$=node(RELARGS,$1,$3);}
  | aterm               {$$=node(RELARGS,$1,0);}
  ;

aterm:
    aterm PLUS mterm   {$$=node(PLUS,$1,$3);}
  | aterm MINUS mterm  {$$=node(MINUS,$1,$3);}
  | mterm              {$$=$1;}
  ;

mterm:
    mterm MULT sterm	  {$$=node(MULT,$1,$3);}
  | sterm			          {$$=$1;}
  ;	

sterm:
    constant            {$$=$1;}
  | number              {$$=$1;}
  | var                 {$$=$1;}
  | LPAREN aterm RPAREN {$$=$2;}
  ;

number:
   NUMBER   {$$=inode(NUMBER,$1);}
  ;

constant:
   CONSTANT {$$=snode(CONSTANT,$1);}
  ;

var:
   VAR      {$$=snode(VAR,$1);}
  ;
pred:
   PRED     {$$=snode(PRED,$1);}
  ;
/* relargs: ;*/

/* end logic section */
 /* begin DE section */

cmd_start:
    cmd PERIOD     {$$=$1;}
  ;

cmd:
    assign_cmd     {$$=$1;}
  | struc_ex_cmd   {$$=$1;}
  | bquery_cmd     {$$=$1;}
  | save_cmd       {$$=$1;}
  | draw_cmd       {$$=$1;}
  | redfindexpr    {$$=$1;}
  | fdexpr         {$$=$1;}
  ;

draw_cmd:
    DRAW LPAREN id RPAREN
        {$$=node(DRAW, $3, 0);}

save_cmd:
    SAVE LPAREN id COMMA FILENAME RPAREN
       {$$=node(SAVE, $3, snode(FILENAME,$5));}
  ;

bquery_cmd:
    id LPAREN id RPAREN {$$=node(ABQUERY, $1, $3);}
  ;

struc_ex_cmd:
    id PERIOD PRED LPAREN relargs RPAREN  
       {$$=node(EXPRED,$1,vlnode(PRED,$3,$5));} 
  | id PERIOD pred 
       {$$=node(EXPREDALL,$1,$3);}
  | id PERIOD constant
       {$$=node(EXCONS,$1,$3);}
  ;

assign_cmd:
  id ASSIGN cmdexpr {$$=node(ASSIGN,$1,$3);}
  ;

cmdexpr:
    cvexpr    {$$=$1;}  /* new vocabulary */
  | csexpr    {$$=$1;} /* new structure */
  | id        {$$=$1;} /* TODO implement this one too :P */
  | crexpr    {$$=$1;} /* new reduction */
  | applyexpr {$$=$1;}
  | cbexpr    {$$=$1;}
  | loadexpr  {$$=$1;}
  | maceexpr  {$$=$1;}
  ;

loadexpr:
  LOAD LPAREN FILENAME RPAREN
      {$$=snode(LOAD,$3);}
  | LOADSTRING LPAREN id COMMA FILENAME RPAREN
      {$$=node(LOADSTRING,$3,snode(LOADSTRING,$5));}
  ;

redfindexpr:
   REDFIND LPAREN id COMMA id RPAREN
      {$$=fournode(REDFIND,$3,$5,0,0);}
   | REDFIND LPAREN id COMMA id COMMA number COMMA number COMMA number RPAREN
      {$$=fournode(REDFIND,$3,$5,$7,node(REDFIND,$9,$11));}
   | REDFIND LPAREN id COMMA id COMMA number COMMA number COMMA number COMMA number RPAREN
      {$$=fournode(REDFIND,$3,$5,$7,node(REDFIND,$9,node(RF_RANGE,$11,$13)));}
  ;

fdexpr:
   FD LPAREN id COMMA id RPAREN
      {$$=fournode(FD,$3,$5,0,0);}
   |	FD LPAREN id COMMA id COMMA number COMMA number RPAREN
      {$$=fournode(FD,$3,$5,node(FD,$7,$9),0);}
   |	FD LPAREN id COMMA id COMMA number COMMA number COMMA number RPAREN
      {$$=fournode(FD,$3,$5,node(FD,$7,$9), node(FD,$11,0));}
  ;

maceexpr:
  MACE LPAREN id COMMA form_start RPAREN
      {$$=qnode(MACE,$3,$5,0);}
  | MACE LPAREN id COMMA form_start COMMA number RPAREN
      {$$=qnode(MACE,$3,$5,$7);}
  ;
cbexpr:
  NEW BQUERY LBRACE id COMMA form_start RBRACE
      {$$=node(BQUERY,$4,$6);}
  ;

applyexpr:
  id LPAREN id RPAREN
      {$$=node(APPLY,$1,$3);}
  ;

crexpr:
  NEW REDUC LBRACE id COMMA id COMMA number COMMA form_start crreldefs RBRACE
      {$$=fournode(REDUC,node(0,$4,$6),$11,$8,$10);}
  | NEW QUERY LBRACE id COMMA id COMMA number COMMA form_start crreldefs RBRACE
      {$$=fournode(REDUC,node(0,$4,$6),$11,$8,$10);}
  ;

cvexpr:	
  NEW VOCAB LBRACE cvarg1 RBRACE
       {$$=arg_node(VOCAB, $4);}
  ;

csexpr:
  NEW STRUC LBRACE csargs RBRACE
       {$$=arg_node(STRUC, $4);}
  ;

cvarg1:
  pred COLON number cvargs
      {$$=qnode(CVRELARG, $1, $3, $4);}
  | cvconsarg1		{$$=$1;}
  ; 

cvargs:
  COMMA pred COLON number cvargs
      {$$=qnode(CVRELARG,$2,$4,$5);}
  | cvconsargs		{$$=$1;}
  ;

cvconsarg1:
      {$$=0;}  /* for no cons args */ 
  | constant cvconsargs	{$$=node(CVCONSARG,$1,$2);}
  ;

cvconsargs:
      {$$=0;}
  | COMMA constant cvconsargs {$$=node(CVCONSARG,$2,$3);}
  ;
;
csargs:
  id COMMA aterm csreldefs /* vocab, universe size, def E, def s */
      {$$=qnode(CSARGS,$1,$3,$4);}
  ;

crreldefs:
    COMMA pred COLON number ASSIGN form_start crreldefs
      {$$=fournode(CRRELDEF, $2, $4, $6, $7);}
  | crconsdefs	{$$=$1;}
  ;

crconsdefs:	{$$=0;}
  | COMMA constant ASSIGN form_start crconsdefs
      {$$=qnode(CRCONSDEF, $2, $4, $5);}
  ;	

csreldefs:
  COMMA pred COLON number ASSIGN form_start csreldefs
      {$$=fournode(CSRELDEF, $2, $4, $6, $7);}
  | csconsdefs {$$=$1;}
  ;

csconsdefs:
      {$$=0;} /* for vocabularies with no constant symbols to def */
  | COMMA constant ASSIGN aterm csconsdefs
      { $$=qnode(CSCONSDEF, $2, $4, $5); }
  ;

id:
    CONSTANT  {$$=snode(ID,$1);}
  | VAR       {$$=snode(ID,$1);}
  | PRED		  {$$=snode(ID, $1);}
  ;


