/******************************************************************************
Este arquivo eh parte da implementacao do ambiente declarativo do middleware
Ginga (Ginga-NCL).

Direitos Autorais Reservados (c) 1989-2007 PUC-Rio/Laboratorio TeleMidia

Este programa eh software livre; voce pode redistribui-lo e/ou modificah-lo sob
os termos da Licenca Publica Geral GNU versao 2 conforme publicada pela Free
Software Foundation.

Este programa eh distribuido na expectativa de que seja util, porem, SEM
NENHUMA GARANTIA; nem mesmo a garantia implicita de COMERCIABILIDADE OU
ADEQUACAO A UMA FINALIDADE ESPECIFICA. Consulte a Licenca Publica Geral do
GNU versao 2 para mais detalhes.

Voce deve ter recebido uma copia da Licenca Publica Geral do GNU versao 2 junto
com este programa; se nao, escreva para a Free Software Foundation, Inc., no
endereco 59 Temple Street, Suite 330, Boston, MA 02111-1307 USA.

Para maiores informacoes:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
******************************************************************************
This file is part of the declarative environment of middleware Ginga (Ginga-NCL)

Copyright: 1989-2007 PUC-RIO/LABORATORIO TELEMIDIA, All Rights Reserved.
  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata


This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License version 2 for more
details.

You should have received a copy of the GNU General Public License version 2
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

For further information contact:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
*******************************************************************************/

#ifndef _SYSTEM_FUNCTIONS_H_
#define _SYSTEM_FUNCTIONS_H_

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <limits>
using namespace std;

#include <sys/timeb.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>

#ifndef PATH_MAX
#define PATH_MAX 512
#endif


// Logging.
#define DEBUG    1
#undef _debug
#define _debug(args...)					\
	do {						\
		if (DEBUG) {				\
			fprintf(stdout, "debug: ");	\
			fprintf(stdout, args);		\
		}					\
	} while(0)					\

#undef _warn
#define _warn(args...)					\
	do {						\
		fprintf(stdout, "warning: ");		\
		fprintf(stdout, args);			\
	} while(0)					\

#undef _error
#define _error(args...)					\
	do {						\
		fprintf(stdout, "ERROR: ");		\
		fprintf(stdout, args);			\
	} while(0)					\

// Other useful macros.
#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))

namespace util {

	void initTimeStamp();
	void printTimeStamp();
	string intToStrHexa(int value);
	int strHexaToInt(string value);
	void setLogToFile();
	void setLogToNullDev();
	void setLogToStdoutDev();
	string upperCase(string s);
	string lowerCase(string s);
	bool isNumeric(void* value);
	string itos(double i);
	float stof(string s);
	float itof(int i);
	string absoluteFile(string basePath, string filename);
	string getPath(string filename);
	vector<string>* split(string str, string token);
	string trim(string str);
	float getPercentualValue(string value);
	bool isPercentualValue(string value);
	bool fileExists(string filename);
	void setDocumentPath(string docPath);
	string getDocumentPath();
	string getCurrentPath();
	double NaN();
	double infinity();
	bool isNaN(double value);
	bool isInfinity(double value);
	void initializeCurrentPath();
	double getCurrentTimeMillis();
	bool isXmlStr(string location);
	bool isAbsolutePath(string path);
	double getNextStepValue(
			double currentStepValue,
			double value,
			int factor,
			double time, double initTime, double dur, int stepSize);

	string updatePath(string dir);
}

#endif //_SYSTEM_FUNCTIONS_H_
