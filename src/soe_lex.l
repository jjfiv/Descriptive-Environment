/* ISC LICENSE */
/* soe.l
 * Skip Jordan
 * Basic input to lex for SO\E logic.
 *
 * chj	10/19/06	Created.
 * chj  11/13/06	Support for commands.
 * chj  12/21/06	Changes for old lex on UNICOS 
 * chj  11/1/11		Loading
 * chj	11/11/11	IFP
 * chj	12/1/11		Mace4
 * chj 	12/5/11		XOR,draw
 * chj	 4/3/12		Reduction finding
 * chj	 1/24/13	formula distance
 * chj	 1/29/13	loadstring
 */
/* Note for UNICOS: If lex is old and doesn't produce yy_scan_string,
 * use a lex.yy.c made elsewhere.  Old lex can handle this file, but
 * cmd/cmd.c uses yy_scan_string and yy_delete_buffer. chj
 */

%{
#include <cstdio>
extern char* dupstr(const char *);
extern int fileno(FILE *);
#include "soe_parse.tab.hh"

 /*extern int yylval;*/
 /*extern YYSTYPE yylval;*/
%}

%%
 /*Begin logic section*/
"TC"	{return TC;}
"IFP"   {return IFP;}
"\\A"   {return FORALL;}
"\\E"   {return EXISTS;}
"\\exists" {return EXISTS;}
"\\forall" {return FORALL;}
"("	{return LPAREN;}
")"	{return RPAREN;}
"["	{return LBRACK;}
"]"	{return RBRACK;}
"&"	{return AND;}
"|"	{return OR;}
"~"	{return NOT;}
"<->"	{return IFF;}
"->"	{return IMPLIES;}
"^"	{return XOR;}
"<"	{return LT;}
"<="	{return LTE;}
"="	{return EQUALS;}
"!="	{return NEQUALS;}
"\\t"	{return TRUE;}
"\\f"   {return FALSE;}
"*"	{return MULT;}
"+"	{return PLUS;}
"-"	{return MINUS;}
","	{return COMMA;}
"."	{return PERIOD;}
":"	{return COLON;} /* End logic section, begin descriptive env section */

"redfind"       {return REDFIND;}
"fd"		{return FD;}
"{"		{return LBRACE;}
"}"		{return RBRACE;}
"is"		{return ASSIGN;}
":="		{return ASSIGN;}
"new"		{return NEW;}
"reduction" 	{return REDUC;}
"bquery" 	{return BQUERY;}
"query"		{return QUERY;}
"structure"	{return STRUC;}
"vocabulary"	{return VOCAB;}
"load"		{return LOAD;}
"loadstring"	{return LOADSTRING;}
"save"		{return SAVE;}
"size"		{return SIZE;}
"draw"		{return DRAW;}
"mace"		{return MACE;} /* End of de section, begin boring section */

[a-t][a-z]*[0-9]*      {yylval.string=dupstr(yytext); return CONSTANT;}
[0-9]*                 {yylval.integer = atoi(yytext); return NUMBER;}
[u-z][a-z]*[0-9]*      {yylval.string=dupstr(yytext); return VAR;}
[A-Z][a-z]*[0-9]*      {yylval.string=dupstr(yytext);return PRED;}
\"[A-Za-z0-9\/\.\\\:\;\,\ \-\_]+\" {yylval.string=dupstr(yytext); return FILENAME;}

"\n"	{return NL;}
[ \t]	{ } /* eat whitespace */
.	{return ERROR;}
%%
#ifndef yywrap
int yywrap() {return 1;}
#endif
