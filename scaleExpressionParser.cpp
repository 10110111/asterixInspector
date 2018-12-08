/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */
#include "scaleExpressionParser.h"


extern double parseScaleExpression(const QString& expression, bool& error);

QString ScaleExpressionParser::lastError;

ScaleExpressionParser::ScaleExpressionParser()
{
}


double ScaleExpressionParser::parse(const QString &expression, bool& error)
{
  return ::parseScaleExpression(expression, error);
}

