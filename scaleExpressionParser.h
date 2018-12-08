#ifndef SCALEEXPRESSIONPARSER_H
#define SCALEEXPRESSIONPARSER_H

#include <QtCore>


class ScaleExpressionParser
{
  public:
    ScaleExpressionParser();

    static double parse(const QString& expression, bool& error);

    static QString lastError;
};

#endif // SCALEEXPRESSIONPARSER_H
