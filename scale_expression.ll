/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2012 Volker Poplawski (volker@openbios.org)
 */

/* tell flex the input is not interactive */
%option never-interactive

%option noyywrap

%{

#define YYSTYPE double
extern "C" int yylex();
#include "scale_expression.bison.h"

%}

DIGIT                     [0-9]

%%


"+"                       {  return '+'; }
"-"                       {  return '-'; }
"*"                       {  return '*'; }
"/"                       {  return '/'; }
"("                       {  return '('; }
")"                       {  return ')'; }
"^"                       {  return '^'; }

[ \t\n\r]+                ;

{DIGIT}+"."{DIGIT}*       { sscanf( yytext, "%lf", &yylval ); return NUM; }

{DIGIT}+                  { sscanf( yytext, "%lf", &yylval ); return NUM; }
%%

