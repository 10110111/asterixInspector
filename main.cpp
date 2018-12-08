/*
 * This application is free software and is released under the terms of
 * the BSD license. See LICENSE file for details.
 *
 * Copyright (c) 2010 Volker Poplawski (volker@openbios.org)
 */

#include <QtWidgets>

#include <iostream>
#include <getopt.h>
#include <locale.h>
#include "global.h"
#include "mainWindow.h"

#include <map>

void printUsage( const QString& appname )
{
  std::cout <<  "Usage: " << appname.toStdString() << " [OPTIONS] [ASTERIXFILE]" << std::endl;
  std::cout <<  "OPTIONS:" << std::endl;
  std::cout <<  "  -h                 Print this text" << std::endl;
  std::cout <<  "  --specs DIRECTORY  Add specifications from DIRECTORY" << std::endl;
// std::cout << "  -V             Print version and exit" << std::endl;
  std::cout << std::endl;
}


void processCommandLine( int argc, char** argv )
{
  const char* optstring = "h?";
  int option_index;
  int c;

//  extern char* optarg;
  extern int optind;

  static struct option long_options[] = {
    {"specs",      required_argument, 0, 0},
    {0, 0, 0, 0}
  };


  while(1) {
    if ((c = getopt_long( argc, argv, optstring, long_options, &option_index )) == -1)
      break; // no more recognized options

    switch (c) {
      case 0:      // longoption
      {
        if (option_index == 0)
        {
          g_specsDirectory = QString::fromUtf8(optarg);
        }
      }
      break;

      case 'h':
      case '?':
        printUsage( argv[0] );
        exit( 0 );
        break;

      case 'V':
        exit( 0 );
        break;

      default:
        break;
    }
  }

  // options left over?
  if (optind < argc) {
    g_openFileName = argv[optind++];

    if (optind < argc) {
      printUsage( argv[0] );
      exit( 1 );
    }
  }
}



int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QApplication::setApplicationName("asterixInspector");

  processCommandLine(argc, argv);

  // UAP parser uses sscanf() to parse floats. Force sane locale.
  setlocale(LC_NUMERIC, "C"); 

  MainWindow w;
  w.show();

  return app.exec();
}
