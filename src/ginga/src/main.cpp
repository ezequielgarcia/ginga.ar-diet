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

#include "system/util/functions.h"
using namespace ::util;

#include "generated/config.h"
#include <ncl/layout/DeviceLayout.h>
#include <system/io/LocalDeviceManager.h>
#include <multidevice/services/DeviceDomain.h>
#include <ncl-presenter/multidevice/FormatterMultiDevice.h>
#include <ncl-presenter/multidevice/FormatterPassiveDevice.h>
#include <ncl-presenter/multidevice/FormatterActiveDevice.h>
#include <lssm/CommonCoreManager.h>
#include <lssm/PresentationEngineManager.h>

#include "ncl/layout/IDeviceLayout.h"
using namespace ::br::pucrio::telemidia::ncl::layout;

#include <system/io/ILocalDeviceManager.h>
using namespace ::br::pucrio::telemidia::ginga::core::system::io;

#include <multidevice/services/IDeviceDomain.h>
using namespace ::br::pucrio::telemidia::ginga::core::multidevice;

#include <ncl-presenter/multidevice/IFormatterMultiDevice.h>
using namespace ::br::pucrio::telemidia::ginga::ncl::multidevice;

#include <lssm/ICommonCoreManager.h>
#include <lssm/IPresentationEngineManager.h>
using namespace ::br::pucrio::telemidia::ginga::lssm;

//#include "../config.h"

#include <system/util/mutex/dfbsurfacemutex.h>

#include <util/debugging_aids.h>

#include <string>
#include <iostream>
using namespace std;

void printHelp() {
	cout << endl << "Usage: gingaNcl [OPTIONS]  [<--ncl> NCLFILE] [-c [connectorFile]]" << endl;
	cout << "Example: gingaNcl --ncl test.ncl" << endl;
	cout << endl << "OPTIONS are:" << endl;
	cout << "-h, --help                    Display this information." << endl;
	cout << "-c, --connector [connectorFile] Start connector using connectorFile as pipe. Else use the default pipe." <<endl;
	cout << "-i, --insert-delay <value>    Insert a delay before application ";
	cout << "processing." << endl;
	cout << "-i, --insert-oc-delay <value> Insert a delay before tune main A/V ";
	cout << "(to exclusively process OC elementary streams)." << endl;
	cout << "-v, --version                 Display version." << endl;
	cout << "-b, --build                   Display build information." << endl;
	cout << "    --baseId                  Set the BaseId name." << endl;
	cout << "    --enable-log [mode]       Enable log mode:" << endl;
	cout << "                                Turn on verbosity, gather all";
	cout << " output" << endl;
	cout << "                                messages and write them into a";
	cout << " device" << endl;
	cout << "                                according to mode." << endl;
	cout << "                                  [mode] can be:" << endl;
	cout << "                                     'file'   write messages into";
	cout << " a file" << endl;
	cout << "                                              named logFile.txt.";
	cout << endl;
	cout << "                                     'stdout' write messages into";
	cout << " /dev/stdout" << endl;
	cout << "                                     'null'   write messages into";
	cout << " /dev/null" << endl;
	cout << "                                              (default mode).";
	cout << endl;
	cout << "    --x-offset <value>        Offset of left display coord." << endl;
	cout << "    --y-offset <value>        Offset of top display coord." << endl;
	cout << "    --set-width <value>       Force a width display value." << endl;
	cout << "    --set-height <value>      Force a height display value.";
	cout << endl;
	cout << "    --device-class <value>    Define the device class through the";
	cout << endl;
	cout << "                                    presentation" << endl;
	cout << "    --enable-gfx            Enable gfx loading presented during";
	cout << " application processing." << endl;
	cout << "    --enable-automount        Enable trigger for main.ncl ";
	cout << " applications received via a transport protocol." << endl;
	cout << "    --enable-remove-oc-filter Enable processor economies after";
	cout << " first time that the object carousel is mounted." << endl;
	
}

void printVersion() {
	printf("Ginga.ar %s by LIFIA\n",GINGA_VERSION);
	printf("Based on Ginga 0.11.2\n");
	printf("Copyright 2002-2010 The TeleMidia PUC-Rio Team.\n");
	printf("Copyright 2010 LIFIA - Facultad de Informática - Univ. Nacional de La Plata \n");
	printf("Copyright 2010 PBEGFV - MICROTROL - Rosario, Argentina \n");
}

void printBuildInfo(){
	printf("Ginga.ar %s.%s\n",GINGA_VERSION,SVN_REVISION);
}

string updateFileUri(string file) {
	if (!isAbsolutePath(file)) {
		return getCurrentPath() + file;
	}

	return file;
}

int main(int argc, char *argv[]) {
	ICommonCoreManager* ccm = NULL;
	IPresentationEngineManager* pem = NULL;
	ILocalDeviceManager* dm = NULL;
	IFormatterMultiDevice* fmd = NULL;

	string nclFile       = "", param = "", bgUri = "", cmdFile = "", connectorFile="";
	int i, devClass      = 0;
	int xOffset          = 0, yOffset = 0, w = 0, h = 0, maxTransp = 0;
	double delayTime     = 0;
	double ocDelay       = 0;
	bool isRemoteDoc     = false;
	bool removeOCFilter  = false;
	bool forceQuit       = true;
	bool enableGfx       = false;
	bool autoMount       = false;
	bool startConnector = false;
	//int baseId = -1;
	string baseId;

	setLogToNullDev();

	for (i = 1; i < argc; i++) {
		if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0)) {
			printHelp();
			return 0;

		} else if ((strcmp(argv[i], "-v") == 0) ||
			    (strcmp(argv[i], "--version")) == 0) {

			printVersion();
			return 0;
		} else if ((strcmp(argv[i], "-b") == 0) ||
			(strcmp(argv[i], "--build")) == 0) {
				printBuildInfo();
				return 0;
		} else if ((strcmp(argv[i], "--ncl") == 0) && ((i + 1) < argc)) {
			nclFile.assign(argv[i+1], strlen(argv[i+1]));

		} else if ((strcmp(argv[i], "--enable-log") == 0) && ((i + 1) < argc)) {
			if (strcmp(argv[i+1], "file") == 0) {
				setLogToFile();

			} else if (strcmp(argv[i+1], "stdout") == 0) {
				setLogToStdoutDev();
			}

		} else if ((strcmp(argv[i], "--force-quit") == 0) && ((i + 1) < argc)) {
			if (strcmp(argv[i+1], "false") == 0) {
				forceQuit = false;
			}

		} else if ((strcmp(argv[i], "--x-offset") == 0) && ((i + 1) < argc)) {
			if (isNumeric(argv[i+1])) {
				param = argv[i+1];
				xOffset = stof(param);
			}

		} else if ((strcmp(argv[i], "--y-offset") == 0) && ((i + 1) < argc)) {
			if (isNumeric(argv[i+1])) {
				param = argv[i+1];
				yOffset = stof(param);
			}

		} else if ((strcmp(argv[i], "--set-width") == 0) && ((i + 1) < argc)) {
			if (isNumeric(argv[i+1])) {
				param = argv[i+1];
				w = stof(param);
			}

		} else if ((strcmp(argv[i], "--set-height") == 0) && ((i + 1) < argc)) {
			if (isNumeric(argv[i+1])) {
				param = argv[i+1];
				h = stof(param);
			}

		} else if ((strcmp(argv[i], "--set-max-transp") == 0) &&
				((i + 1) < argc)) {

			if (isNumeric(argv[i+1])) {
				param = argv[i+1];
				maxTransp = stof(param);
			}

		} else if ((strcmp(argv[i], "--set-bg-image") == 0) &&
				((i + 1) < argc)) {

			param = argv[i+1];
			if (fileExists(param)) {
				bgUri = param;
				cout << "main bgUri = '" << bgUri << "'";
				cout << " OK" << endl;

			} else {
				cout << "main Warning: can't set bgUri '" << param << "'";
				cout << " file does not exist" << endl;
			}

		} else if (((strcmp(argv[i], "--device-class") == 0) ||
				(strcmp(argv[i], "--dev-class") == 0)) && ((i + 1) < argc)) {

			if (strcmp(argv[i+1], "1") == 0 ||
					strcmp(argv[i+1], "passive") == 0) {

				devClass = 1;

			} else if (strcmp(argv[i+1], "2") == 0 ||
					strcmp(argv[i+1], "active") == 0) {

				devClass = 2;
			}

		} else if (((strcmp(argv[i], "-i") == 0) ||
				(strcmp(argv[i], "--insert-delay") == 0)) &&
				((i + 1) < argc)) {

			if (isNumeric(argv[i+1])) {
				param     = argv[i+1];
				delayTime = stof(param);
			}

		} else if (strcmp(argv[i], "--insert-oc-delay") == 0 &&
				((i + 1) < argc)) {

			if (isNumeric(argv[i+1])) {
				param   = argv[i+1];
				ocDelay = stof(param);
			}

		} else if (strcmp(argv[i], "--enable-gfx") == 0) {
			enableGfx = true;
		}else if ((strcmp(argv[i], "-r") == 0) || (strcmp(argv[i], "--enable-rendering-layer-controller") == 0)) {
			::util::Singleton<DFBSurfaceMutex> _DFBSurfaceMutexSingleton;
			_DFBSurfaceMutexSingleton.instance().enable();
		} else if (strcmp(argv[i], "--enable-automount") == 0) {
			autoMount = true;

		} else if (strcmp(argv[i], "--enable-remove-oc-filter") == 0) {
			removeOCFilter = true;

		} else if ((strcmp(argv[i], "--enable-cmdfile") == 0) &&
				((i + 1) < argc)) {

			cmdFile.assign(argv[i+1], strlen(argv[i+1]));
			cout << "argv = '" << argv[i+1] << "' cmdFile = '";
			cout << cmdFile << "'" << endl;
		}else if ((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "--connector") == 0)) {
			startConnector = true;
			if (i + 1 < argc ){
				connectorFile.assign(argv[i+1], strlen(argv[i+1]));
			}
		}else if ((strcmp(argv[i], "--baseId") == 0) && (i + 1 < argc)){
			//if (isNumeric(argv[i+1])) {
				param = argv[i +1];
				//baseId = stof(param);
				baseId = param;
			//}
		}
	}

	if (delayTime > 0) {
		::usleep(delayTime);
	}

	std::cout << "Setting up terminate handler" << std::endl;
	std::set_terminate(custom_terminate);

	initTimeStamp();
	initializeCurrentPath();

	if (nclFile != "") {
		nclFile = updateFileUri(nclFile);

		if (argc > 1 && nclFile.substr(0, 1) != "/") {
			cout << "ginga main() remote NCLFILE" << endl;
			isRemoteDoc = true;
		}
	}

	printf("\n   --------------------------| Ginga.ar %s |--------------------------\n",GINGA_VERSION);
	printf("\n[Ginga] processing file: %s\n ", nclFile.c_str());

	if (devClass == 1) {
		dm  = LocalDeviceManager::getInstance();
		dm->createDevice("systemScreen(0)");
		fmd = new FormatterPassiveDevice(NULL, xOffset, yOffset, w, h);

		if (bgUri != "") {
			fmd->setBackgroundImage(bgUri);
		}
		getchar();

	} else if (devClass == 2) {
		dm  = LocalDeviceManager::getInstance();
		dm->createDevice("systemScreen(0)");
		fmd = new FormatterActiveDevice(NULL, xOffset, yOffset, w, h);

		if (bgUri != "") {
			fmd->setBackgroundImage(bgUri);
		}

		getchar();

	} else {

		if (nclFile == "") {
			enableGfx = false;
		}

		pem = new PresentationEngineManager(devClass, xOffset, yOffset, w, h, enableGfx);

		if (pem == NULL) {
			return -2;
		}
		
		if(baseId != ""){
			pem->setCurrentPrivateBaseId(baseId);
		}
		if (startConnector){
			pem->startConnector(connectorFile);
		}

		if (bgUri != "") {
			cout << endl << endl;
			cout << "main '" << bgUri << "'" << endl;
			cout << endl << endl;
			pem->setBackgroundImage(bgUri);
		}

		if (cmdFile != "") {
			cmdFile = updateFileUri(cmdFile);
			pem->setCmdFile(cmdFile);
		}

		if (nclFile == "") {
			pem->setIsLocalNcl(false);
			pem->autoMountOC(autoMount);

			ccm = new CommonCoreManager(pem); //, xOffset, yOffset, w, h);

			ccm->removeOCFilterAfterMount(removeOCFilter);
			ccm->setOCDelay(ocDelay);
			ccm->startPresentation();
                        ccm->waitPresentationEnd();

		} else if (fileExists(nclFile) || isRemoteDoc) {
			pem->setIsLocalNcl(forceQuit);
			if (pem->startNclFile(nclFile)){
				pem->waitUnlockCondition();			
			}
		}

		if (pem != NULL) {
			delete pem;
		}

		if (ccm != NULL) {
			delete ccm;
		}

	}

	cout << "[Ginga] Process finished." << endl;
	return 0;
}
