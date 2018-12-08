/* Evaluates expressions of the <scale> fields.

   (c) 2012 by Volker Poplawski
*/


/* everything between %{ and }% is copied to the output */
%{
#define YYSTYPE double
#include <math.h>
#include <stdio.h>

extern "C" int yylex();

void yyerror (char const *);

double evaluated;
%}
    


%token NUM
%left '-' '+'
%left '*' '/'
%left NEG     /* negation--unary minus */
%right '^'    /* exponentiation */

/* end of definition section */

%% 

/* start of rule section */



input:  exp { evaluated = $1; }
;
     
exp:      NUM                { $$ = $1;         }
	| exp '+' exp        { $$ = $1 + $3;    }
	| exp '-' exp        { $$ = $1 - $3;    }
	| exp '*' exp        { $$ = $1 * $3;    }
	| exp '/' exp        { $$ = $1 / $3;    }
	| '-' exp  %prec NEG { $$ = -$2;        }
	| exp '^' exp        { $$ = pow ($1, $3); }
	| '(' exp ')'        { $$ = $2;         }
;

/* end of rule section */

%%

/* start of subroutine section */

#include <QtCore>

#include "scale_expression.flex.h"
#include "scaleExpressionParser.h"


void yyerror(const char* s)
{
  ScaleExpressionParser::lastError = QString(s);
}


double parseScaleExpression(const QString& expression, bool &error)
{
  error = false;
  ScaleExpressionParser::lastError.clear();

  YY_BUFFER_STATE bufferState = yy_scan_string(expression.toUtf8());
  // yydebug = 1;
  if (yyparse())
    error = true;

  yy_delete_buffer(bufferState);

  return evaluated;
}
