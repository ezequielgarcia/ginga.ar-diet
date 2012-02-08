/******************************************************************************
Este arquivo eh parte da implementacao do ambiente declarativo do middleware
Ginga (Ginga-NCL).

Direitos Autorais Reservados (c) 1989-2007 PUC-Rio/Laboratorio TeleMidia

Este programa eh software livre; voce pode redistribui-lo e/ou modificah-lo sob
os termos da Licen� Publica Geral GNU versao 2 conforme publicada pela Free
Software Foundation.

Este programa eh distribu�o na expectativa de que seja util, porem, SEM
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

#include "../../include/util/functions.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

namespace util {
	string gingaDocumentPath;
	string gingaCurrentPath;
	static double timeStamp;

	namespace fs = boost::filesystem;
	
#ifndef	__DARWIN_UNIX03
	wofstream logOutput;
#endif

	void initTimeStamp() {
		timeStamp = getCurrentTimeMillis();
	}

	void printTimeStamp() {
		double printTS = getCurrentTimeMillis() - timeStamp;
		cout << "TimeStamp: " << (printTS / 1000) << endl;
	}

	int strHexaToInt(string value) {
		std::stringstream str1(value);
		int number;

		if (value == "") {
			number = -1;

		} else if (value.find("0x") != string::npos) {
			if (!(str1 >> std::hex >> number)) {
				number = -1;
			}

		} else {
			if (!(str1 >> number)) {
				number = -1;
			}
		}
		return number;
	}

	string intToStrHexa(int value) {
		stringstream str;
		str << std::hex << std::uppercase << value;
		return "0x" + str.str();
	}

	void setLogToFile() {
#ifndef	__DARWIN_UNIX03
		if (logOutput) {
			logOutput.close();
		}

		logOutput.open("logFile.txt");
		if (logOutput) {
			wclog.rdbuf(logOutput.rdbuf());
		}
#endif
	}

	void setLogToNullDev() {
#ifndef	__DARWIN_UNIX03
		if (logOutput) {
			logOutput.close();
		}

		logOutput.open("/dev/null");
		if (logOutput) {
			wclog.rdbuf(logOutput.rdbuf());
		}
#endif
	}

	void setLogToStdoutDev() {
#ifndef	__DARWIN_UNIX03
		if (logOutput) {
			logOutput.close();
		}

		logOutput.open("/dev/stdout");
		if (logOutput) {
			wclog.rdbuf(logOutput.rdbuf());
		}
#endif
	}

	string lowerCase(string s) {
		for (unsigned int j=0; j<s.length(); ++j) {
			s[j]=tolower(s[j]);
		}
		return s;
	}

	string upperCase(string s) {
		if (s == "") {
			return "";
		}

		for (unsigned int j=0; j<s.length(); ++j) {
			s[j]=toupper(s[j]);
		}
		return s;
	}

	bool isNumeric(void* value) {
		bool r = true;
		char* str = (char*)value;

		for (unsigned int i = 0; i < strlen(str); i++) {
			if (i == 0)
				if ((str[i] < '0' || str[i] > '9') &&
					    (str[i] != '.') &&
					    (str[i] != '+' || str[i] != '-'))
					r = false;

			if ((str[i] < '0' || str[i] > '9') && (str[i] != '.'))
				r = false;
		}

		return r;
	}

	string itos(double i) {
		return boost::lexical_cast<string>(i);
	}

	float stof(string s) {
		return boost::lexical_cast<float>(trim(s));
  	}

	float itof(int i) {
		return boost::lexical_cast<float>(i);
	}

	string absoluteFile(string basePath, string fileName) {
		/*uri_t* uriFile = NULL;
		string protocol, path;*/

		if (isXmlStr(fileName)) {
			return fileName;
		}

		if (isAbsolutePath(fileName))
			return fileName;

		fileName = basePath + "/" + fileName;

		/*try {
			uriFile = uri_alloc((char*)filename.c_str(), filename.length());
			protocol = uri_scheme(uriFile);
			path = uri_path(uriFile);
		}
		catch(...) {
			if(uriFile!=NULL)
				uri_free(uriFile);
			return filename;
		}

		if(protocol=="") {
			if(filename.substr(0,1)!="/") { //o que inclui . ou  ..
				filename = basePath + "/" + filename;
			}
		}*/

		//uri_free(uriFile);
		return fileName;
	}

	string getPath(string filename) {
		string path;
		string::size_type i;

		i = filename.find_last_of('/');
		if (i != string::npos) {
			path = filename.substr(0, i);
		} else {
			path = "";
		}

		return path;
	}

	string getCurrentPath() {
		return gingaCurrentPath;
	}

	vector<string>* split(string str, string delimiter) {
		vector<string>* splited;
		splited = new vector<string>;

		if (str == "") {
			return splited;
		}

		if (str.find_first_of(delimiter) == std::string::npos) {
			splited->push_back(str);
			return splited;
		}

		string::size_type lastPos = str.find_first_not_of(delimiter, 0);
		string::size_type pos = str.find_first_of(delimiter, lastPos);

		while (string::npos != pos || string::npos != lastPos) {
		    splited->push_back(str.substr(lastPos, pos - lastPos));
		    lastPos = str.find_first_not_of(delimiter, pos);
		    pos = str.find_first_of(delimiter, lastPos);
		}
		return splited;
	}

	string trim(string str) {
		boost::trim(str);
		return str;
	}

	float getPercentualValue(string value) {
		string actualValue;
		float floatValue;

		
		//retirar o caracter percentual da string
		actualValue = value.substr(0, (value.length() - 1));
		//converter para float
		floatValue = stof(actualValue);

		//se menor que zero, retornar zero
		if (floatValue < 0)
			floatValue = 0;
		//else if (floatValue > 100)
			//se maior que 100, retornar 100
			//floatValue = 100;

		//retornar valor percentual
		return floatValue;
	}

	/**
	 * Testa se uma string indica um valor percentual
	 * @param value string com um valor
	 * @return true se o valor e' percentual; false caso contrario.
	 */
	bool isPercentualValue(string value) {
		if (value.find_last_of("%") == (value.length() - 1))
			return true;
		else
			return false;
	}

	bool fileExists(string fileName) {
		string currentPath;
		fstream fin;
		char path[PATH_MAX] = "";
		char* p = NULL;

if (fileName == "") {
			return false;
		}

		if (fileName.find("<") != std::string::npos) {
			return true;
		}

		if (fileName.find_first_of("/") != 0) {
			getcwd(path, PATH_MAX);
			p = (char*)malloc(strlen(path) + 1);
			if (p)
				strcpy(p, path);
			currentPath = (string)p;
			fileName = currentPath + "/" + fileName;
		}
		return fs::exists( fileName );
	}

	void setDocumentPath(string docPath) {
		if (docPath.find_last_of("/") != docPath.length() - 1) {
			docPath = docPath + "/";
		}
		gingaDocumentPath = docPath;
	}

	string getDocumentPath() {
		return gingaDocumentPath;
	}

	void initializeCurrentPath() {
		char path[PATH_MAX] = "";
		getcwd(path, PATH_MAX);

		gingaCurrentPath.assign(path, strlen(path));

		if (gingaCurrentPath.find_last_of("/") !=
				gingaCurrentPath.length() - 1) {

			gingaCurrentPath = gingaCurrentPath + "/";
		}
	}

	double NaN() {
		return numeric_limits<double>().quiet_NaN();
	}

	double infinity() {
		return numeric_limits<double>().infinity();
	}

	bool isNaN(double value) {
		string sval;
		sval = itos(value);
		/*wclog << "functions::isNaN val = '" << value << "' ";
		wclog << "sval = '" << sval << "'";*/
		if (isNumeric((void*)(sval.c_str()))) {
			return false;
		}

		if (upperCase(sval) == "NAN") {
			return true;
		}

		/*wclog << "isNaN Warning! Value = '" << value << "', ";
		wclog << "sval = " << sval << endl;*/
		return false;
	}

	bool isInfinity(double value) {
		string sval;
		sval = itos(value);
		if (isNumeric((void*)(sval.c_str()))) {
			return false;
		}

		if (upperCase(sval).find("INF") != std::string::npos) {
			return true;
		}

		/*wclog << "isInfinity Warning! Value = '" << value << "', ";
		wclog << "sval = " << sval << endl;*/
		return false;
	}
/*
	int timevalSubtract(
			struct timeval *result, struct timeval *x, struct timeval *y) {

		int nsec;

		if (x->tv_usec < y->tv_usec) {
			nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
			y->tv_usec -= 1000000 * nsec;
			y->tv_sec  += nsec;
		}

		if (x->tv_usec - y->tv_usec > 1000000) {
			nsec = (x->tv_usec - y->tv_usec) / 1000000;
			y->tv_usec += 1000000 * nsec;
			y->tv_sec  -= nsec;
		}

		result->tv_sec  = x->tv_sec - y->tv_sec;
		result->tv_usec = x->tv_usec - y->tv_usec;

		return x->tv_sec < y->tv_sec;
	}

	static struct timeval startTimeMills;
	static bool firstCallTimeMills = true;

	double getCurrentTimeMillis() {
		struct timeval result;
		struct rusage usage;

		if (getrusage(RUSAGE_SELF, &usage) != 0) {
			cout << "getCurrentTimeMillis Warning!";
			cout << " getrusage error" << endl;
			return -1;
		}

		if (firstCallTimeMills) {
			firstCallTimeMills     = false;
			startTimeMills.tv_sec  = usage.ru_utime.tv_sec;
			startTimeMills.tv_usec = usage.ru_utime.tv_usec;
			return 1;
		}

		timevalSubtract(&result, &(usage.ru_utime), &startTimeMills);

		return (double) ((result.tv_sec * 1000) + (result.tv_usec / 1000));
	}
*/

	static double startTimeMills;
	static bool firstCallTimeMills = true;

	double getCurrentTimeMillis() {
		struct timeb t;

		ftime(&t);

		if(firstCallTimeMills) {
			firstCallTimeMills = false;
			startTimeMills = (double)t.time*1000 + (double)t.millitm - 1;
			return 1;
		}

		return (double)t.time*1000 + (double)t.millitm - startTimeMills;
	}

	bool isXmlStr(string location) {
		if (location.find("<") != std::string::npos ||
				location.find("?xml") != std::string::npos ||
				location.find("|") != std::string::npos) {

			return true;
		}

		return false;
	}

	bool isAbsolutePath(string path) {
		string::size_type i, len;

		if (isXmlStr(path)) {
			return true;
		}

		i = path.find_first_of("\\");
		while (i != string::npos) {
			path.replace(i,1,"/");
			i = path.find_first_of("\\");
		}

		len = path.length();
		if ((len >= 10 && path.substr(0,10) == "x-sbtvdts:")
				|| (len >= 9 && path.substr(0,9) == "sbtvd-ts:")
				|| (len >= 7 && path.substr(0,7) == "http://")
			    || (len >= 6 && path.substr(0,6) == "ftp://")
			    || (len >= 1 && path.substr(0,1) == "/")
			    || (len >= 2 && path.substr(1,1) == ":")
			    || (len >= 7 && path.substr(0,7) == "file://")
			    || (len >= 6 && path.substr(0,6) == "tcp://")
			    || (len >= 6 && path.substr(0,6) == "udp://")
			    || (len >= 6 && path.substr(0,6) == "rtp://")) {

			return true;
		}

		return false;
	}

	//factor is not in use. It will be removed.
	double getNextStepValue(
			double initValue,
			double target,
			int factor,
			double time, double initTime, double dur, int durStep) {

		int numSteps;
		double stepSize, nextStepValue;

		//rg
		if (durStep <= 0) { //durStep is negative or not defined
			nextStepValue = initValue + (
					(double)(target-initValue)/dur) * (time-initTime);

		} else if (dur <= 0) {
			return target;

		} else {
			numSteps = dur/(durStep*1000);
			stepSize = (target-initValue)/numSteps;

			//cout << floor((time-initTime)/(durStep*1000)) << endl;
			//cout << stepSize << endl << endl;
			nextStepValue = initValue + (int) (
					(int)((time-initTime)/(durStep*1000))*stepSize);
		}

		return nextStepValue;
	}

	string updatePath(string dir) {
		bool found = false;
		string temp, newDir;
		vector<string>* params;
		vector<string>::iterator it;
		string::size_type pos;

		if (dir.find("<") != std::string::npos) {
			return dir;
		}

		while (true) {
			pos = dir.find_first_of("\\");
			if (pos == std::string::npos) {
				break;
			}
			dir.replace(pos, 1, "/");
		}

		params = split(dir, "/");
		newDir = "";
		it = params->begin();
		while (it != params->end()) {
			if ((it + 1) != params->end()) {
				temp = *(it + 1);
				if (temp != ".." || found) {
					newDir = newDir + "/" + (*it);
				} else {
					++it;
					found = true;
				}

			} else if ((*it) != ".") {
				newDir = newDir + "/" + (*it);
			}
			++it;
		}
		delete params;

		if (found) {
			return updatePath(newDir);

		} else {
			return newDir;
		}
	}

}
