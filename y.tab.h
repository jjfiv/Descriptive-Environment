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
extern YYSTYPE yylval;
