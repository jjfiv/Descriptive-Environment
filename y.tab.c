#ifndef lint
static const char yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20110908

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)

#define YYPREFIX "yy"

#define YYPURE 0

#line 37 "soe.y"
	#include "parse.h"
#line 40 "soe.y"
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union {
	int integer;
	char *string;
	struct node *n;
	} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
#line 34 "y.tab.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

/* Parameters sent to yyerror. */
#define YYERROR_DECL() yyerror(const char *s)
#define YYERROR_CALL(msg) yyerror(msg)

extern int YYPARSE_DECL();

#define CONSTANT 257
#define VAR 258
#define NUMBER 259
#define MULT 260
#define PLUS 261
#define MINUS 262
#define AND 263
#define OR 264
#define NOT 265
#define IFF 266
#define IMPLIES 267
#define XOR 268
#define LT 269
#define LTE 270
#define EQUALS 271
#define NEQUALS 272
#define TRUE 273
#define FALSE 274
#define PRED 275
#define FORALL 276
#define EXISTS 277
#define TC 278
#define IFP 279
#define SOE 280
#define LPAREN 281
#define RPAREN 282
#define LBRACK 283
#define RBRACK 284
#define LBRACE 285
#define RBRACE 286
#define COMMA 287
#define PERIOD 288
#define COLON 289
#define ASSIGN 290
#define NEW 291
#define REDFIND 292
#define FD 293
#define VOCAB 294
#define STRUC 295
#define QUOTE 296
#define REDUC 297
#define BQUERY 298
#define QUERY 299
#define LOAD 300
#define LOADSTRING 301
#define SAVE 302
#define MACE 303
#define DRAW 304
#define SIZE 305
#define CVRELARG 306
#define CVCONSARG 307
#define CSARGS 308
#define CSRELDEF 309
#define CSRELDEFI 310
#define CSCONSDEF 311
#define ID 312
#define FNODEIL 313
#define FNODEIR 314
#define NL 315
#define FILENAME 316
#define ERROR 317
#define VARLIST 318
#define VLR 319
#define EXPREDALL 320
#define TCARGS 321
#define CRRELDEF 322
#define SOVARLIST 323
#define LFPARGS 324
#define RELARGS 325
#define EXCONS 326
#define EXPRED 327
#define APPLY 328
#define CRCONSDEF 329
#define ABQUERY 330
#define RF_RANGE 331
#define YYERRCODE 256
static const short yylhs[] = {                           -1,
    0,   12,    1,    2,    2,    2,    3,    3,    3,    3,
    4,    4,    5,    5,    8,    8,    8,    8,    8,    8,
    8,    8,   43,   36,   37,   37,   47,   48,   48,   10,
    9,    9,    9,    6,    6,    6,    6,    6,    6,    6,
    6,   29,   29,   13,   13,   13,   14,   14,    7,    7,
    7,    7,   28,   30,   31,   32,   11,   15,   15,   15,
   15,   15,   15,   15,   42,   46,   26,   35,   35,   35,
   16,   18,   18,   18,   18,   18,   18,   18,   18,   45,
   45,   49,   49,   49,   50,   50,   50,   44,   44,   41,
   39,   38,   38,   19,   20,   33,   33,   21,   21,   34,
   34,   23,   23,   22,   25,   25,   40,   40,   24,   24,
   27,   27,   17,   17,   17,
};
static const short yylen[] = {                            2,
    2,    1,    1,    1,    3,    3,    1,    3,    3,    3,
    2,    1,    1,    1,    5,    4,    5,    4,    9,   11,
    5,    4,    3,    4,    0,    5,    2,    0,    3,    2,
    1,    3,    2,    3,    3,    3,    3,    3,    4,    1,
    1,    3,    1,    3,    3,    1,    3,    1,    1,    1,
    1,    3,    1,    1,    1,    1,    2,    1,    1,    1,
    1,    1,    1,    1,    4,    6,    4,    6,    3,    3,
    3,    1,    1,    1,    1,    1,    1,    1,    1,    4,
    6,    6,   12,   14,    6,   10,   12,    6,    8,    7,
    4,   12,   12,    5,    5,    4,    1,    5,    1,    0,
    2,    0,    3,    4,    7,    1,    0,    5,    7,    1,
    0,    5,    1,    1,    1,
};
static const short yydefred[] = {                         0,
  113,  114,  115,    0,    0,    0,    0,    0,    0,    0,
   58,    0,   60,   59,   62,   61,   63,   64,    0,    0,
    0,    0,    1,   57,    0,    0,    0,    0,    0,    0,
    0,    0,   54,    0,   70,   69,    0,    0,    0,    0,
    0,   71,   72,   73,   75,   76,   77,   79,   78,    0,
    0,    0,   65,   67,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   55,   53,    0,
   48,    0,    0,   50,    0,   49,   51,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   82,    0,   85,    0,
   66,    0,    0,    0,    0,    0,   68,   56,    0,    0,
    0,   97,    0,    0,    0,    0,    0,   80,    0,    0,
   91,    0,    0,   52,    0,    0,   42,   47,    0,  101,
    0,   94,    0,   95,    0,    0,    0,    0,    0,   40,
   41,    0,    0,    0,    0,    0,    0,    2,    3,    0,
    7,   12,   14,   13,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   81,   11,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   88,    0,    0,    0,    0,    0,    0,    0,  103,
    0,   96,   99,    0,  104,  110,    0,   90,    0,    0,
    0,   33,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   38,    8,    9,    6,    5,   10,    0,
    0,    0,    0,    0,    0,   86,    0,    0,    0,    0,
    0,    0,   39,   32,   30,   16,    0,   18,    0,   23,
   22,    0,    0,    0,    0,   89,    0,    0,    0,    0,
    0,    0,    0,   15,   17,   21,    0,    0,    0,    0,
   83,    0,   87,    0,    0,    0,    0,    0,    0,   24,
    0,    0,   27,    0,    0,   98,    0,  112,    0,    0,
    0,  106,    0,    0,    0,    0,    0,   84,    0,    0,
    0,   92,   93,    0,    0,   29,    0,  109,    0,    0,
    0,   19,    0,    0,    0,   26,    0,    0,  108,    0,
   20,    0,  105,
};
static const short yydgoto[] = {                          8,
  138,  139,  140,  141,  142,  143,   71,  144,  159,  195,
    9,  145,  146,   73,   10,   11,  249,   42,   43,   44,
  182,  104,  183,  185,  271,   13,  186,   74,   75,   76,
   77,   36,  101,  102,   14,  202,  260,   45,   46,  272,
   47,   15,  162,   48,   49,   16,  250,  263,   17,   18,
};
static const short yysindex[] = {                      -242,
    0,    0,    0, -271, -255, -220, -211,    0, -210, -193,
    0, -149,    0,    0,    0,    0,    0,    0, -226, -226,
 -226, -226,    0,    0, -226, -230, -157, -213, -178, -160,
 -140, -129,    0, -124,    0,    0, -123, -115,  -91,  -88,
  -78,    0,    0,    0,    0,    0,    0,    0,    0, -226,
 -226, -155,    0,    0, -161, -122,  -84,  -69,  -65,  -58,
 -103, -226, -226, -226, -131, -128,  -77,    0,    0, -161,
    0, -244,  -17,    0,  -54,    0,    0, -216, -226, -226,
 -226, -226,  -37,  -41,  -40,  -34,    0,  -10,    0,  -10,
    0, -227, -161, -161, -161, -161,    0,    0,  -36,  -33,
  -28,    0,  -27,  -25,  -24,  -23,  -21,    0,  -63,   91,
    0,  -20,   -7,    0,  -17,  -17,    0,    0,    2,    0,
  -10,    0, -161,    0, -226,   91, -226,   -1,   91,    0,
    0,  -19,   24, -246,    1,    4,   91,    0,    0,  -45,
    0,    0,    0,    0, -118,   82,  -10,  -10,  -36,   -4,
 -241,   -2,    6,    8,    0,    0, -161, -251, -249, -208,
   -3, -197, -161, -226,   19,  112,   91,   91,   91,   91,
   91,    0,  -10, -161, -161, -161, -161,   29, -117,    0,
 -216,    0,    0, -216,    0,    0,  -10,    0,  -10,   33,
   24,    0,   91,   91,   22,   91,   30,  -10,   91,   32,
 -214,   35,   39,    0,    0,    0,    0,    0,    0,   45,
 -145, -145, -145, -145,  -10,    0,  -10,   40,   27,   42,
   47,   51,    0,    0,    0,    0,   91,    0,   91,    0,
    0,   91, -161,   91, -226,    0, -102,   60,  -10, -161,
  -10,   91,   91,    0,    0,    0, -184,   61,   59,   58,
    0,  -10,    0,   -4, -174,   68,   74,   74, -161,    0,
    9, -226,    0,   91,   81,    0,    2,    0,   91, -216,
   85,    0,   89, -172, -161, -226,   92,    0,   90,   88,
   96,    0,    0, -161,   97,    0,   99,    0,   91,  -10,
 -184,    0, -161,  100,   98,    0,  104,    2,    0,   91,
    0,   74,    0,
};
static const short yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  101,    0,    0,    0,    0,    0,    0,
  102,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  109,  -32,    0,    0,    0,    0,  106,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  107,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    7,   36,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0, -137,
    0,    0,    0,    0,    0,    0,    0,    0,  107,  107,
  110,    0,    0,    0,    0,    0,    0, -111,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -80,  -72,   46,   73,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,  108,    0,  111,    0,
    0,    0,    0,  107,  110,    0,  113,  113,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  110,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  108,    0,    0,  113,    0,    0,    0,    0,    0,    0,
    0,  113,    0,
};
static const short yygindex[] = {                         0,
 -135,   28,    0,  -92,    0,    0,  192,    0, -120,  -29,
    0, -121,  -51,  132,    0,    0,    3,    0,    0,    0,
  141,    0,  -93,  119, -245,    0,  146,  -79,  -94,  -26,
    0,  -70,    0,    0,    0,    0,  114,    0,    0,  115,
    0,    0,    0,    0,    0,    0,  126,    0,    0,    0,
};
#define YYTABLESIZE 409
static const short yytable[] = {                         35,
  117,  165,   12,   72,  153,  120,  158,  100,  112,   19,
  113,  158,  273,  160,    1,    2,   93,   94,   92,   93,
   94,   28,   29,   30,   31,   20,   33,   32,   98,   41,
    1,    2,    3,   93,   94,  191,  156,  192,  193,  194,
   33,  150,   95,   72,   34,  184,   93,   94,    3,    4,
    5,   99,   65,   66,  114,  180,  303,  225,   98,    6,
   21,    7,  190,  161,   84,   85,   86,  178,  179,   22,
  224,  151,  233,   50,  205,  206,   93,   94,  209,  193,
  196,  103,  105,  106,  107,  166,   93,   94,   93,   94,
  193,  199,  149,  210,   24,   33,   68,   69,  248,    1,
    2,  226,  259,  228,   23,   72,  231,  221,   51,  222,
  218,  201,  267,  220,  284,   93,   94,    3,  230,   70,
  257,  258,  211,  212,  213,  214,   52,  152,  277,  154,
  197,   25,  200,   37,  244,  237,  245,  238,   26,  246,
   27,   53,   38,   39,    4,   40,    4,  279,    4,    4,
   87,    4,   54,   89,  149,   88,   55,  219,   90,  254,
   67,  256,   78,  172,  216,   61,  203,  294,  173,  217,
   56,   57,  265,   58,   59,   60,   31,   31,  302,  251,
  285,  247,   37,   37,  252,   37,   37,   37,  255,   62,
   36,   36,   63,   36,   36,   36,  207,  208,  297,  281,
   79,   37,   64,   37,   91,   37,   37,  274,   37,   36,
  295,   36,   83,   36,   36,   80,   36,  167,  168,   81,
  169,  170,  171,   72,  115,  116,   82,   97,   46,   46,
   46,   46,  291,   46,   46,   46,   46,   46,   46,   46,
  219,   72,   96,  280,  108,  109,  110,  111,   69,   46,
  119,   46,  128,   46,   46,  121,   46,  122,   33,  123,
  124,  157,  125,  126,  276,  127,  147,   44,   44,   44,
   44,  280,   44,   44,   44,   44,   44,   44,   44,  148,
  155,  158,  181,  163,  187,  198,  164,  118,   44,  275,
   44,  188,   44,   44,  189,   44,   45,   45,   45,   45,
  204,   45,   45,   45,   45,   45,   45,   45,   34,   34,
  227,   34,   34,   34,  223,  215,  240,   45,  229,   45,
  232,   45,   45,  234,   45,  235,  236,   34,  239,   34,
  241,   34,   34,  242,   34,   35,   35,  243,   35,   35,
   35,  253,   93,   94,  261,  262,  264,   33,   68,   69,
  174,  175,  176,  177,   35,  129,   35,  269,   35,   35,
  270,   35,  278,  130,  131,  132,  133,  134,  135,  136,
  282,  137,   93,   94,  283,  287,  184,  289,  292,  293,
  174,  175,  176,  177,  290,  301,  298,  300,   56,   74,
   43,  100,  102,  114,  266,  111,   25,  288,  107,   28,
  268,  286,    0,    0,  296,    0,    0,    0,  299,
};
static const short yycheck[] = {                         26,
   95,  137,    0,   55,  126,   99,  258,   78,   88,  281,
   90,  258,  258,  134,  257,  258,  261,  262,   70,  261,
  262,   19,   20,   21,   22,  281,  257,   25,  275,   27,
  257,  258,  275,  261,  262,  287,  129,  158,  288,  289,
  257,  121,  287,   95,  275,  287,  261,  262,  275,  292,
  293,   78,   50,   51,  282,  149,  302,  193,  275,  302,
  281,  304,  157,  134,   62,   63,   64,  147,  148,  281,
  191,  123,  287,  287,  167,  168,  261,  262,  171,  288,
  289,   79,   80,   81,   82,  137,  261,  262,  261,  262,
  288,  289,  119,  173,  288,  257,  258,  259,  234,  257,
  258,  194,  287,  196,  315,  157,  199,  187,  287,  189,
  181,  163,  287,  184,  287,  261,  262,  275,  198,  281,
  242,  243,  174,  175,  176,  177,  287,  125,  264,  127,
  160,  281,  162,  291,  227,  215,  229,  217,  288,  232,
  290,  282,  300,  301,  282,  303,  284,  269,  286,  287,
  282,  289,  282,  282,  181,  287,  281,  184,  287,  239,
  316,  241,  285,  282,  282,  281,  164,  289,  287,  287,
  294,  295,  252,  297,  298,  299,  288,  289,  300,  282,
  275,  233,  263,  264,  287,  266,  267,  268,  240,  281,
  263,  264,  281,  266,  267,  268,  169,  170,  293,  270,
  285,  282,  281,  284,  282,  286,  287,  259,  289,  282,
  290,  284,  316,  286,  287,  285,  289,  263,  264,  285,
  266,  267,  268,  275,   93,   94,  285,  282,  261,  262,
  263,  264,  284,  266,  267,  268,  269,  270,  271,  272,
  267,  293,  260,  270,  282,  287,  287,  282,  259,  282,
  287,  284,  316,  286,  287,  289,  289,  286,  257,  287,
  286,  281,  287,  287,  262,  287,  287,  261,  262,  263,
  264,  298,  266,  267,  268,  269,  270,  271,  272,  287,
  282,  258,  287,  283,  287,  289,  283,   96,  282,  281,
  284,  286,  286,  287,  287,  289,  261,  262,  263,  264,
  282,  266,  267,  268,  269,  270,  271,  272,  263,  264,
  289,  266,  267,  268,  282,  287,  290,  282,  289,  284,
  289,  286,  287,  289,  289,  287,  282,  282,  289,  284,
  289,  286,  287,  287,  289,  263,  264,  287,  266,  267,
  268,  282,  261,  262,  284,  287,  289,  257,  258,  259,
  269,  270,  271,  272,  282,  265,  284,  290,  286,  287,
  287,  289,  282,  273,  274,  275,  276,  277,  278,  279,
  286,  281,  261,  262,  286,  284,  287,  290,  282,  281,
  269,  270,  271,  272,  289,  282,  287,  290,  288,  288,
  282,  286,  286,  282,  254,  286,  289,  279,  286,  289,
  255,  276,   -1,   -1,  291,   -1,   -1,   -1,  294,
};
#define YYFINAL 8
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 331
#if YYDEBUG
static const char *yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"CONSTANT","VAR","NUMBER","MULT",
"PLUS","MINUS","AND","OR","NOT","IFF","IMPLIES","XOR","LT","LTE","EQUALS",
"NEQUALS","TRUE","FALSE","PRED","FORALL","EXISTS","TC","IFP","SOE","LPAREN",
"RPAREN","LBRACK","RBRACK","LBRACE","RBRACE","COMMA","PERIOD","COLON","ASSIGN",
"NEW","REDFIND","FD","VOCAB","STRUC","QUOTE","REDUC","BQUERY","QUERY","LOAD",
"LOADSTRING","SAVE","MACE","DRAW","SIZE","CVRELARG","CVCONSARG","CSARGS",
"CSRELDEF","CSRELDEFI","CSCONSDEF","ID","FNODEIL","FNODEIR","NL","FILENAME",
"ERROR","VARLIST","VLR","EXPREDALL","TCARGS","CRRELDEF","SOVARLIST","LFPARGS",
"RELARGS","EXCONS","EXPRED","APPLY","CRCONSDEF","ABQUERY","RF_RANGE",
};
static const char *yyrule[] = {
"$accept : ROOT",
"ROOT : cmd_start NL",
"form_start : form",
"form : impform",
"impform : andform",
"impform : andform IMPLIES impform",
"impform : andform IFF impform",
"andform : notform",
"andform : andform AND notform",
"andform : andform OR notform",
"andform : andform XOR notform",
"notform : NOT notform",
"notform : qform",
"qform : q_form",
"qform : atomic",
"q_form : FORALL varlist restr COLON notform",
"q_form : FORALL varlist COLON notform",
"q_form : EXISTS varlist restr COLON notform",
"q_form : EXISTS varlist COLON notform",
"q_form : TC LBRACK even_args COLON form RBRACK LPAREN relargs RPAREN",
"q_form : IFP LBRACK id COMMA lfpargs COLON form RBRACK LPAREN relargs RPAREN",
"q_form : EXISTS sovarlist restr COLON notform",
"q_form : EXISTS sovarlist COLON notform",
"sovarlist : pred COLON number",
"even_args : aterm COMMA aterm eargs",
"eargs :",
"eargs : COMMA aterm COMMA aterm eargs",
"lfpargs : id lfpargs2",
"lfpargs2 :",
"lfpargs2 : COMMA id lfpargs",
"restr : PERIOD form",
"varlist : VAR",
"varlist : VAR COMMA varlist",
"varlist : VAR varlist",
"atomic : aterm EQUALS aterm",
"atomic : aterm NEQUALS aterm",
"atomic : aterm LTE aterm",
"atomic : aterm LT aterm",
"atomic : LPAREN form RPAREN",
"atomic : PRED LPAREN relargs RPAREN",
"atomic : TRUE",
"atomic : FALSE",
"relargs : aterm COMMA relargs",
"relargs : aterm",
"aterm : aterm PLUS mterm",
"aterm : aterm MINUS mterm",
"aterm : mterm",
"mterm : mterm MULT sterm",
"mterm : sterm",
"sterm : constant",
"sterm : number",
"sterm : var",
"sterm : LPAREN aterm RPAREN",
"number : NUMBER",
"constant : CONSTANT",
"var : VAR",
"pred : PRED",
"cmd_start : cmd PERIOD",
"cmd : assign_cmd",
"cmd : struc_ex_cmd",
"cmd : bquery_cmd",
"cmd : save_cmd",
"cmd : draw_cmd",
"cmd : redfindexpr",
"cmd : fdexpr",
"draw_cmd : DRAW LPAREN id RPAREN",
"save_cmd : SAVE LPAREN id COMMA FILENAME RPAREN",
"bquery_cmd : id LPAREN id RPAREN",
"struc_ex_cmd : id PERIOD PRED LPAREN relargs RPAREN",
"struc_ex_cmd : id PERIOD pred",
"struc_ex_cmd : id PERIOD constant",
"assign_cmd : id ASSIGN cmdexpr",
"cmdexpr : cvexpr",
"cmdexpr : csexpr",
"cmdexpr : id",
"cmdexpr : crexpr",
"cmdexpr : applyexpr",
"cmdexpr : cbexpr",
"cmdexpr : loadexpr",
"cmdexpr : maceexpr",
"loadexpr : LOAD LPAREN FILENAME RPAREN",
"loadexpr : LOADSTRING LPAREN id COMMA FILENAME RPAREN",
"redfindexpr : REDFIND LPAREN id COMMA id RPAREN",
"redfindexpr : REDFIND LPAREN id COMMA id COMMA number COMMA number COMMA number RPAREN",
"redfindexpr : REDFIND LPAREN id COMMA id COMMA number COMMA number COMMA number COMMA number RPAREN",
"fdexpr : FD LPAREN id COMMA id RPAREN",
"fdexpr : FD LPAREN id COMMA id COMMA number COMMA number RPAREN",
"fdexpr : FD LPAREN id COMMA id COMMA number COMMA number COMMA number RPAREN",
"maceexpr : MACE LPAREN id COMMA form_start RPAREN",
"maceexpr : MACE LPAREN id COMMA form_start COMMA number RPAREN",
"cbexpr : NEW BQUERY LBRACE id COMMA form_start RBRACE",
"applyexpr : id LPAREN id RPAREN",
"crexpr : NEW REDUC LBRACE id COMMA id COMMA number COMMA form_start crreldefs RBRACE",
"crexpr : NEW QUERY LBRACE id COMMA id COMMA number COMMA form_start crreldefs RBRACE",
"cvexpr : NEW VOCAB LBRACE cvarg1 RBRACE",
"csexpr : NEW STRUC LBRACE csargs RBRACE",
"cvarg1 : pred COLON number cvargs",
"cvarg1 : cvconsarg1",
"cvargs : COMMA pred COLON number cvargs",
"cvargs : cvconsargs",
"cvconsarg1 :",
"cvconsarg1 : constant cvconsargs",
"cvconsargs :",
"cvconsargs : COMMA constant cvconsargs",
"csargs : id COMMA aterm csreldefs",
"crreldefs : COMMA pred COLON number ASSIGN form_start crreldefs",
"crreldefs : crconsdefs",
"crconsdefs :",
"crconsdefs : COMMA constant ASSIGN form_start crconsdefs",
"csreldefs : COMMA pred COLON number ASSIGN form_start csreldefs",
"csreldefs : csconsdefs",
"csconsdefs :",
"csconsdefs : COMMA constant ASSIGN aterm csconsdefs",
"id : CONSTANT",
"id : VAR",
"id : PRED",

};
#endif

int      yydebug;
int      yynerrs;

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

typedef struct {
    unsigned stacksize;
    short    *s_base;
    short    *s_mark;
    short    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;

#if YYDEBUG
#include <stdio.h>		/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = data->s_mark - data->s_base;
    newss = (short *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != 0)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack)) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if ((yyn = yysindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
        {
            goto yyoverflow;
        }
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if ((yyn = yyrindex[yystate]) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag) goto yyinrecovery;

    yyerror("syntax error");

    goto yyerrlab;

yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if ((yyn = yysindex[*yystack.s_mark]) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
                {
                    goto yyoverflow;
                }
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == 0) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            yys = 0;
            if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
            if (!yys) yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);
    switch (yyn)
    {
case 1:
#line 133 "soe.y"
	{cmdtree=node(0,yystack.l_mark[-1].n,0); yyval.n=yystack.l_mark[-1].n;}
break;
case 2:
#line 137 "soe.y"
	{parsetree=node(0,yystack.l_mark[0].n,0);
				 yyval.n=yystack.l_mark[0].n;}
break;
case 3:
#line 141 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 4:
#line 145 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 5:
#line 146 "soe.y"
	{yyval.n=node(IMPLIES,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 6:
#line 147 "soe.y"
	{yyval.n=node(IFF,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 7:
#line 151 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 8:
#line 152 "soe.y"
	{yyval.n=node(AND,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 9:
#line 153 "soe.y"
	{yyval.n=node(OR,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 10:
#line 154 "soe.y"
	{yyval.n=node(XOR,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 11:
#line 158 "soe.y"
	{yyval.n=node(NOT,yystack.l_mark[0].n,0);}
break;
case 12:
#line 159 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 13:
#line 163 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 14:
#line 165 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 15:
#line 170 "soe.y"
	{yyval.n=qnode(FORALL,yystack.l_mark[-3].n, yystack.l_mark[-2].n, yystack.l_mark[0].n);}
break;
case 16:
#line 172 "soe.y"
	{yyval.n=qnode(FORALL,yystack.l_mark[-2].n,  0, yystack.l_mark[0].n);}
break;
case 17:
#line 174 "soe.y"
	{yyval.n=qnode(EXISTS, yystack.l_mark[-3].n, yystack.l_mark[-2].n, yystack.l_mark[0].n);}
break;
case 18:
#line 176 "soe.y"
	{yyval.n=qnode(EXISTS, yystack.l_mark[-2].n,  0, yystack.l_mark[0].n);}
break;
case 19:
#line 178 "soe.y"
	{yyval.n=qnode(TC, yystack.l_mark[-6].n, yystack.l_mark[-4].n, yystack.l_mark[-1].n);}
break;
case 20:
#line 180 "soe.y"
	{yyval.n=fournode(IFP, yystack.l_mark[-8].n, yystack.l_mark[-6].n, yystack.l_mark[-4].n, yystack.l_mark[-1].n);}
break;
case 21:
#line 183 "soe.y"
	{yyval.n=qnode(SOE, yystack.l_mark[-3].n, yystack.l_mark[-2].n, yystack.l_mark[0].n);}
break;
case 22:
#line 185 "soe.y"
	{yyval.n=qnode(SOE, yystack.l_mark[-2].n, 0, yystack.l_mark[0].n);}
break;
case 23:
#line 189 "soe.y"
	{yyval.n=qnode(SOVARLIST, yystack.l_mark[-2].n, yystack.l_mark[0].n, 0);}
break;
case 24:
#line 198 "soe.y"
	{yyval.n=qnode(TCARGS,yystack.l_mark[-3].n, yystack.l_mark[-1].n, yystack.l_mark[0].n);}
break;
case 25:
#line 201 "soe.y"
	{yyval.n=0;}
break;
case 26:
#line 203 "soe.y"
	{yyval.n=qnode(TCARGS,yystack.l_mark[-3].n,yystack.l_mark[-1].n,yystack.l_mark[0].n);}
break;
case 27:
#line 207 "soe.y"
	{yyval.n=node(LFPARGS,yystack.l_mark[-1].n,yystack.l_mark[0].n);}
break;
case 28:
#line 210 "soe.y"
	{yyval.n=0;}
break;
case 29:
#line 211 "soe.y"
	{yyval.n=node(LFPARGS,yystack.l_mark[-1].n,yystack.l_mark[0].n);}
break;
case 30:
#line 215 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 31:
#line 219 "soe.y"
	{yyval.n=vlnode(VARLIST, yystack.l_mark[0].string, 0);}
break;
case 32:
#line 220 "soe.y"
	{yyval.n=vlnode(VARLIST,  yystack.l_mark[-2].string, yystack.l_mark[0].n);}
break;
case 33:
#line 221 "soe.y"
	{yyval.n=vlnode(VARLIST, yystack.l_mark[-1].string, yystack.l_mark[0].n);}
break;
case 34:
#line 225 "soe.y"
	{yyval.n=node(EQUALS,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 35:
#line 226 "soe.y"
	{yyval.n=node(NEQUALS, yystack.l_mark[-2].n, yystack.l_mark[0].n);}
break;
case 36:
#line 227 "soe.y"
	{yyval.n=node(LTE, yystack.l_mark[-2].n, yystack.l_mark[0].n);}
break;
case 37:
#line 228 "soe.y"
	{yyval.n=node(LT,yystack.l_mark[-2].n, yystack.l_mark[0].n);}
break;
case 38:
#line 229 "soe.y"
	{yyval.n=yystack.l_mark[-1].n;}
break;
case 39:
#line 230 "soe.y"
	{yyval.n=vlnode(PRED,yystack.l_mark[-3].string,yystack.l_mark[-1].n);}
break;
case 40:
#line 231 "soe.y"
	{yyval.n=node(TRUE,0,0);}
break;
case 41:
#line 232 "soe.y"
	{yyval.n=node(FALSE,0,0);}
break;
case 42:
#line 236 "soe.y"
	{yyval.n=node(RELARGS,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 43:
#line 237 "soe.y"
	{yyval.n=node(RELARGS,yystack.l_mark[0].n,0);}
break;
case 44:
#line 241 "soe.y"
	{yyval.n=node(PLUS,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 45:
#line 242 "soe.y"
	{yyval.n=node(MINUS,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 46:
#line 243 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 47:
#line 247 "soe.y"
	{yyval.n=node(MULT,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 48:
#line 248 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 49:
#line 252 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 50:
#line 253 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 51:
#line 254 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 52:
#line 255 "soe.y"
	{yyval.n=yystack.l_mark[-1].n;}
break;
case 53:
#line 258 "soe.y"
	{yyval.n=inode(NUMBER,yystack.l_mark[0].integer);}
break;
case 54:
#line 261 "soe.y"
	{yyval.n=snode(CONSTANT,yystack.l_mark[0].string);}
break;
case 55:
#line 264 "soe.y"
	{yyval.n=snode(VAR,yystack.l_mark[0].string);}
break;
case 56:
#line 267 "soe.y"
	{yyval.n=snode(PRED,yystack.l_mark[0].string);}
break;
case 57:
#line 275 "soe.y"
	{yyval.n=yystack.l_mark[-1].n;}
break;
case 58:
#line 279 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 59:
#line 280 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 60:
#line 281 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 61:
#line 282 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 62:
#line 283 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 63:
#line 284 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 64:
#line 285 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 65:
#line 290 "soe.y"
	{yyval.n=node(DRAW, yystack.l_mark[-1].n, 0);}
break;
case 66:
#line 294 "soe.y"
	{yyval.n=node(SAVE, yystack.l_mark[-3].n, snode(FILENAME,yystack.l_mark[-1].string));}
break;
case 67:
#line 298 "soe.y"
	{yyval.n=node(ABQUERY, yystack.l_mark[-3].n, yystack.l_mark[-1].n);}
break;
case 68:
#line 303 "soe.y"
	{yyval.n=node(EXPRED,yystack.l_mark[-5].n,vlnode(PRED,yystack.l_mark[-3].string,yystack.l_mark[-1].n));}
break;
case 69:
#line 304 "soe.y"
	{yyval.n=node(EXPREDALL,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 70:
#line 305 "soe.y"
	{yyval.n=node(EXCONS,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 71:
#line 308 "soe.y"
	{yyval.n=node(ASSIGN,yystack.l_mark[-2].n,yystack.l_mark[0].n);}
break;
case 72:
#line 312 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 73:
#line 313 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 74:
#line 314 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 75:
#line 315 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 76:
#line 316 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 77:
#line 317 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 78:
#line 318 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 79:
#line 319 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 80:
#line 326 "soe.y"
	{yyval.n=snode(LOAD,yystack.l_mark[-1].string);}
break;
case 81:
#line 328 "soe.y"
	{yyval.n=node(LOADSTRING,yystack.l_mark[-3].n,snode(LOADSTRING,yystack.l_mark[-1].string));}
break;
case 82:
#line 333 "soe.y"
	{yyval.n=fournode(REDFIND,yystack.l_mark[-3].n,yystack.l_mark[-1].n,0,0);}
break;
case 83:
#line 335 "soe.y"
	{yyval.n=fournode(REDFIND,yystack.l_mark[-9].n,yystack.l_mark[-7].n,yystack.l_mark[-5].n,node(REDFIND,yystack.l_mark[-3].n,yystack.l_mark[-1].n));}
break;
case 84:
#line 338 "soe.y"
	{yyval.n=fournode(REDFIND,yystack.l_mark[-11].n,yystack.l_mark[-9].n,yystack.l_mark[-7].n,node(REDFIND,yystack.l_mark[-5].n,node(RF_RANGE,yystack.l_mark[-3].n,yystack.l_mark[-1].n)));}
break;
case 85:
#line 343 "soe.y"
	{yyval.n=fournode(FD,yystack.l_mark[-3].n,yystack.l_mark[-1].n,0,0);}
break;
case 86:
#line 345 "soe.y"
	{yyval.n=fournode(FD,yystack.l_mark[-7].n,yystack.l_mark[-5].n,node(FD,yystack.l_mark[-3].n,yystack.l_mark[-1].n),0);}
break;
case 87:
#line 347 "soe.y"
	{yyval.n=fournode(FD,yystack.l_mark[-9].n,yystack.l_mark[-7].n,node(FD,yystack.l_mark[-5].n,yystack.l_mark[-3].n),
					     node(FD,yystack.l_mark[-1].n,0));}
break;
case 88:
#line 353 "soe.y"
	{yyval.n=qnode(MACE,yystack.l_mark[-3].n,yystack.l_mark[-1].n,0);}
break;
case 89:
#line 355 "soe.y"
	{yyval.n=qnode(MACE,yystack.l_mark[-5].n,yystack.l_mark[-3].n,yystack.l_mark[-1].n);}
break;
case 90:
#line 359 "soe.y"
	{yyval.n=node(BQUERY,yystack.l_mark[-3].n,yystack.l_mark[-1].n);}
break;
case 91:
#line 363 "soe.y"
	{yyval.n=node(APPLY,yystack.l_mark[-3].n,yystack.l_mark[-1].n);}
break;
case 92:
#line 367 "soe.y"
	{yyval.n=fournode(REDUC,node(0,yystack.l_mark[-8].n,yystack.l_mark[-6].n),yystack.l_mark[-1].n,yystack.l_mark[-4].n,yystack.l_mark[-2].n);}
break;
case 93:
#line 369 "soe.y"
	{yyval.n=fournode(REDUC,node(0,yystack.l_mark[-8].n,yystack.l_mark[-6].n),yystack.l_mark[-1].n,yystack.l_mark[-4].n,yystack.l_mark[-2].n);}
break;
case 94:
#line 373 "soe.y"
	{yyval.n=arg_node(VOCAB, yystack.l_mark[-1].n);}
break;
case 95:
#line 377 "soe.y"
	{yyval.n=arg_node(STRUC, yystack.l_mark[-1].n);}
break;
case 96:
#line 381 "soe.y"
	{yyval.n=qnode(CVRELARG, yystack.l_mark[-3].n, yystack.l_mark[-1].n, yystack.l_mark[0].n);}
break;
case 97:
#line 382 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 98:
#line 386 "soe.y"
	{yyval.n=qnode(CVRELARG,yystack.l_mark[-3].n,yystack.l_mark[-1].n,yystack.l_mark[0].n);}
break;
case 99:
#line 387 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 100:
#line 391 "soe.y"
	{yyval.n=0;}
break;
case 101:
#line 392 "soe.y"
	{yyval.n=node(CVCONSARG,yystack.l_mark[-1].n,yystack.l_mark[0].n);}
break;
case 102:
#line 394 "soe.y"
	{yyval.n=0;}
break;
case 103:
#line 395 "soe.y"
	{yyval.n=node(CVCONSARG,yystack.l_mark[-1].n,yystack.l_mark[0].n);}
break;
case 104:
#line 400 "soe.y"
	{yyval.n=qnode(CSARGS,yystack.l_mark[-3].n,yystack.l_mark[-1].n,yystack.l_mark[0].n);}
break;
case 105:
#line 405 "soe.y"
	{yyval.n=fournode(CRRELDEF, yystack.l_mark[-5].n, yystack.l_mark[-3].n, yystack.l_mark[-1].n, yystack.l_mark[0].n);}
break;
case 106:
#line 406 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 107:
#line 409 "soe.y"
	{yyval.n=0;}
break;
case 108:
#line 411 "soe.y"
	{yyval.n=qnode(CRCONSDEF, yystack.l_mark[-3].n, yystack.l_mark[-1].n, yystack.l_mark[0].n);}
break;
case 109:
#line 416 "soe.y"
	{yyval.n=fournode(CSRELDEF, yystack.l_mark[-5].n, yystack.l_mark[-3].n, yystack.l_mark[-1].n, yystack.l_mark[0].n);}
break;
case 110:
#line 417 "soe.y"
	{yyval.n=yystack.l_mark[0].n;}
break;
case 111:
#line 420 "soe.y"
	{yyval.n=0;}
break;
case 112:
#line 423 "soe.y"
	{yyval.n=qnode(CSCONSDEF, yystack.l_mark[-3].n, yystack.l_mark[-1].n, yystack.l_mark[0].n);}
break;
case 113:
#line 428 "soe.y"
	{yyval.n=snode(ID,yystack.l_mark[0].string);}
break;
case 114:
#line 429 "soe.y"
	{yyval.n=snode(ID,yystack.l_mark[0].string);}
break;
case 115:
#line 430 "soe.y"
	{yyval.n=snode(ID, yystack.l_mark[0].string);}
break;
#line 1212 "y.tab.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            if ((yychar = YYLEX) < 0) yychar = 0;
#if YYDEBUG
            if (yydebug)
            {
                yys = 0;
                if (yychar <= YYMAXTOKEN) yys = yyname[yychar];
                if (!yys) yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == 0) goto yyaccept;
        goto yyloop;
    }
    if ((yyn = yygindex[yym]) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack))
    {
        goto yyoverflow;
    }
    *++yystack.s_mark = (short) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    yyerror("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
