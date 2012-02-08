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

#include "generated/config.h"
#include "../include/PresentationEngineManager.h"
#include <ncl-presenter/editingcommandevents.h>

#include <connector/connector/connector.h>
#include <connector/connector/handler/editingcommandhandler.h>
#include <connector/connector/handler/exitcommandhandler.h>
#include <connector/connector/handler/keepalivehandler.h>
#include <system/io/GingaLocatorFactory.h>
#include <system/io/LocalDeviceManager.h>
#include <system/io/InputManager.h>
#include <ncl-presenter/FormatterMediator.h>
#include <player/ShowButton.h>
#include <player/IApplicationPlayer.h>
#include <system/io/interface/input/CodeMap.h>
#include <system/io/IGingaLocatorFactory.h>
#include <system/io/IInputManager.h>
#include <system/io/ILocalDeviceManager.h>

using namespace ::br::pucrio::telemidia::ginga::core::player;
using namespace ::br::pucrio::telemidia::ginga::ncl;
using namespace ::br::pucrio::telemidia::ginga::core::system::io;
using namespace ::br::pucrio::telemidia::ginga::lssm;

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

struct inputEventNotification {
	PresentationEngineManager* p;
	int code;
	string parameter;
	vector<string>* cmds;
	bool enableGfx;
};

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace lssm {
	ILocalDeviceManager* PresentationEngineManager::dm = NULL;
	IInputManager* PresentationEngineManager::im       = NULL;

	void* PresentationEngineManager::editingCommandStart (void* ptr) {
		PresentationEngineManager* _this = (PresentationEngineManager*)ptr;
		_this->editingCommandsDispatcherLoop();
		return NULL;
	}

	PresentationEngineManager::PresentationEngineManager(
		int devClass,
		int xOffset,
		int yOffset,
		int width,
		int height,
		bool enableGfx) : Thread::Thread() {

		int deviceNumber  = 0;
		string deviceName = "systemScreen(" + itos(devClass) + ")";

		if (dm == NULL) {
			dm = LocalDeviceManager::getInstance();
		}

		x = 0;
		if (xOffset > 0) {
			x = xOffset;
		}

		y = 0;
		if (yOffset > 0) {
			y = yOffset;
		}

		deviceNumber = dm->createDevice(deviceName);
		_dfb = ((LocalDeviceManager *)dm)->_deviceScreen;

		w = dm->getDeviceWidth(deviceNumber);
		if (width > 0 && (width < w || w == 0)) {
			w = width;
		}

		h = dm->getDeviceHeight(deviceNumber);
		if (height > 0 && (height < h || h == 0)) {
			h = height;
		}

		cout << "[PresentationEngineManager] Device screen information, x: "<< x <<", y: "<< y <<", w: "<< w <<", h: " << h << endl;

		if (h == 0 || w == 0) {
			cout << "PEM::PEM Warning! Trying to create an ";
			cout << "invalid window" << endl;
			w = 1;
			h = 1;
		}

		this->devClass             = devClass;
		this->enableGfx            = enableGfx;
		this->formattersToRelease  = new set<INCLPlayer*>;
		this->formatters           = new map<string, INCLPlayer*>;
		this->dsmccListener        = NULL;

		this->paused               = false;
		this->standAloneApp        = true;
		this->isLocalNcl           = true;
		this->closed               = false;
		//this->currentPrivateBaseId = -1;
		this->setCurrentPrivateBaseId("-1");
		this->commands             = NULL;
		this->timeBaseProvider     = NULL;
		//this->documentIDs          = new map<string, string>;
		this->connector            = NULL;

		pthread_mutex_init (&releasingMutex, NULL);

		im   = InputManager::getInstance();
		privateBaseManager = PrivateBaseManager::getInstance();

		im->addInputEventListener(this);

		// Editing commands dispatcher initialization.
		pthread_mutex_init(&editingCommandsLock, NULL);
		pthread_cond_init(&editingCommandsCond, NULL);
		editingCommandsDispatcherDone = false;
		pthread_create(&editingCommandsDispatcher, NULL, &editingCommandStart, (void*)this);
	}

	PresentationEngineManager::~PresentationEngineManager() {
		if (!closed) {
			close();
		}

		//delete documentIDs;
		delete connector;
		delete _dfb; //Added to force DFB desinitialization
		pthread_mutex_destroy(&releasingMutex);

		editingCommandsDispatcherDone = true;
		pthread_mutex_lock(&editingCommandsLock);
		pthread_cond_signal(&editingCommandsCond);
		pthread_mutex_unlock(&editingCommandsLock);

		pthread_join (editingCommandsDispatcher, NULL);
		
		pthread_cond_destroy(&editingCommandsCond);
		pthread_mutex_destroy(&editingCommandsLock);
		editingCommandsDispatcherDone = false;
	}

	void PresentationEngineManager::autoMountOC(bool autoMountIt) {
	}
/*
	void PresentationEngineManager::setCurrentPrivateBaseId(
		unsigned int baseId) {

		cout << "PresentationEngineManager::setCurrentPrivateBaseId '";
		cout << baseId << "'";
		cout << endl;
		this->currentPrivateBaseId = (int)baseId;
	}
*/

	void PresentationEngineManager::setCurrentPrivateBaseId(
		std::string baseId) {

		cout << "PresentationEngineManager::setCurrentPrivateBaseId '";
		cout << baseId << "'";
		cout << endl;
		this->currentPrivateBaseId = baseId;
	}
	
	void PresentationEngineManager::setTimeBaseProvider(
		ITimeBaseProvider* tmp) {

		timeBaseProvider = tmp;
	}

	void PresentationEngineManager::setTimeBaseInfo(INCLPlayer* p) {
		if (timeBaseProvider != NULL) {
			p->setTimeBaseProvider(timeBaseProvider);
		}
	}

	void PresentationEngineManager::editingCommand(string editingCommand) {
		vector<string>* args;
		vector<string>::iterator i;
		IGingaLocatorFactory* glf = NULL;
		//NclDocument* document     = NULL;
		INCLPlayer* docPlayer     = NULL;
		string /*commandTag,*/ privateDataPayload, baseId, docId;
		unsigned char commandTag;
		string docIor, docUri, arg, uri, ior, uName;

#if HAVE_COMPSUPPORT
		glf = ((GingaLocatorFactoryCreator*)(cm->getObject(
					"GingaLocatorFactory")))();
#else
		glf = GingaLocatorFactory::getInstance();
#endif

		/*
				commandTag         = EventDescriptor::getCommandTag(editingCommand);
				privateDataPayload = EventDescriptor::getPrivateDataPayload(
						editingCommand);
		*/
		args   = split(privateDataPayload, ",");
		i      = args->begin();
		baseId = -1; //EventDescriptor::extractMarks(*i);
		++i;

		//parse command
		if (commandTag == EC_OPEN_BASE) {


		} else if (commandTag == EC_ACTIVATE_BASE) {


		} else if (commandTag == EC_DEACTIVATE_BASE) {


		} else if (commandTag == EC_SAVE_BASE) {


		} else if (commandTag == EC_CLOSE_BASE) {


		} else if (commandTag == EC_ADD_DOCUMENT) {
			cout << "PresentationEngineManager::editingCommand (addDocument)";
			cout << endl;

			docUri = "";
			docIor = "";
			while (i != args->end()) {
				/*
					if ((*i).find("x-sbtvd://") != std::string::npos) {
						uri = EventDescriptor::extractMarks(*i);
						++i;
						ior = EventDescriptor::extractMarks(*i);

						if (uri.find("x-sbtvd://") != std::string::npos) {
							uri = uri.substr(uri.find("x-sbtvd://") + 10,
									uri.length() - (uri.find("x-sbtvd://") + 10));
						}

						if (docUri == "") {
							docUri = uri;
							docIor = ior;
						}

						cout << "PresentationEngineManager::editingCommand ";
						cout << " command '" << arg << "' creating locator ";
						cout << "uri '" << uri << "', ior '" << ior;
						cout << "'" << endl;
						if (glf != NULL) {
							glf->createLocator(uri, ior);
						}
					} else { */
				uri = *i;
				ior = uri;
				if (docUri == "") {
					docIor = uri;
					docUri = uri;
				}
				//}

				++i;
			}
			if (docUri == docIor) {
				cout << "PresentationEngineManager::editingCommand";
				cout << " calling addDocument '" << docUri;
				cout << "' in private base '" << baseId;
				cout << endl;

				lock();
				docPlayer = createNclPlayer(baseId, docUri);
				unlock();

			} else {
				cout << "PresentationEngineManager::editingCommand calling ";
				cout << "getLocation '" << docUri << "' for ior '";
				cout << docIor << "'" << endl;

				if (glf != NULL) {
					uri = glf->getLocation(docUri);
					uName = glf->getName(docIor);
					lock();
					docPlayer = createNclPlayer(baseId, uri + uName);
					unlock();
				}
			}

		} else {
			cout << "PresentationEngineManager::editingCommand not to base";
			cout << endl;

			//docId  = EventDescriptor::extractMarks(*i);
			//docPlayer = getNclPlayer(baseId, docId);
			docPlayer = getNclPlayer(docId);
			if (docPlayer != NULL) {
				if (commandTag == EC_START_DOCUMENT) {
					setTimeBaseInfo(docPlayer);
				}
				/* Warning Este metodo no va mas */
				/* Suponemos que no pasa mas por aca, si pasa avisale al laucha */
				//docPlayer->editingCommand ( commandTag, privateDataPayload );
				assert(false);

			} else {
				cout << "PresentationEngineManager::editingCommand can't ";
				cout << "find NCL player for '" << docId << "'";
				cout << endl;
			}
		}

	}

	void PresentationEngineManager::setBackgroundImage(string uri) {
		updateFormatters(UC_BACKGROUND, uri);
	}

	void PresentationEngineManager::getScreenShot() {
		updateFormatters(UC_PRINTSCREEN);
	}

	void PresentationEngineManager::close() {
		map<int, set<INCLPlayer*>*>::iterator i;

		closed = true;

		if (im != NULL) {
			im->removeInputEventListener(this);
			im->release();
			im = NULL;
		}


		lock();
		DEL(formattersToRelease);
		DEL(formatters);
		unlock();

		if (dm != NULL) {
			//dm->clearWidgetPools();
			dm->release();
		}
	}

	void PresentationEngineManager::registerKeys() {
		set<int>* keys;
		keys = new set<int>;

		keys->insert(CodeMap::KEY_GREATER_THAN_SIGN);
		keys->insert(CodeMap::KEY_LESS_THAN_SIGN);

		keys->insert(CodeMap::KEY_SUPER);
		keys->insert(CodeMap::KEY_PRINTSCREEN);

		keys->insert(CodeMap::KEY_F10);
		keys->insert(CodeMap::KEY_POWER);

		keys->insert(CodeMap::KEY_F11);
		keys->insert(CodeMap::KEY_STOP);
		keys->insert(CodeMap::KEY_EXIT);

		keys->insert(CodeMap::KEY_F12);
		keys->insert(CodeMap::KEY_PAUSE);

		if (commands != NULL && !commands->empty()) {
			keys->insert(CodeMap::KEY_PLUS_SIGN);
		}

		if (im != NULL) {
			im->addInputEventListener(this, keys);
		}
	}

	bool PresentationEngineManager::getIsLocalNcl() {
		return this->isLocalNcl;
	}

	void PresentationEngineManager::setIsLocalNcl(bool isLocal) {
		this->isLocalNcl = isLocal;
	}

	INCLPlayer* PresentationEngineManager::createNclPlayer(
			string baseId, string fname) {

		NclPlayerData* data   = NULL;
		NclDocument* document = NULL;
		INCLPlayer* formatter = NULL;
		string documentId;

/*		if (formatters->find(documentId) != formatters->end()) {
			formatter = (*formatters)[documentId];
			std::cout << "[PresentationEngineManager] Returning previous formatter, id: " << documentId << std::endl;
		} else {*/
			data           = createNclPlayerData();
			data->baseId   = baseId;
			data->playerId = fname;

			formatter = new FormatterMediator(data);

                        document = (NclDocument*)(formatter->setCurrentDocument(fname));

			if ( not document ) {
				cout << "[PresentationEngineManager] ERROR document ID is NULL" << endl;
				return NULL;
			}

			documentId = document->getId();
			data->docId = documentId ;

			cout << "[PresentationEngineManager] Document ID: " << documentId  << endl;

			if ( formatters->empty() ) {
				registerKeys();
			}

			formatter->addListener(this);
			(*formatters)[documentId] = formatter;
		//}

		return formatter;
	}

	NclPlayerData* PresentationEngineManager::createNclPlayerData() {
		NclPlayerData* data     = NULL;

		data = new NclPlayerData;
		data->baseId            = "";
		data->playerId          = "";
		data->devClass          = devClass;
		data->x                 = x;
		data->y                 = y;
		data->w                 = w;
		data->h                 = h;
		data->enableGfx         = enableGfx;
		data->parentDocId       = "";
		data->nodeId            = "";
		data->docId             = "";
		data->focusManager      = NULL;

		return data;
	}

	bool PresentationEngineManager::startNclFile(string fname) {
		//INCLPlayer* formatter = createNclPlayer ( itos ( currentPrivateBaseId ), fname );
		INCLPlayer* formatter = createNclPlayer (currentPrivateBaseId, fname );
		
		if ( formatter == NULL ) {
			cout << "[PresentationEngineManager] presentation could not be started." << endl;
			return false;
		}

		setTimeBaseInfo ( formatter );
		formatter->play();
		return true;
	}

	//#if HAVE_GINGAJ
	void PresentationEngineManager::openNclDocument(
			string docUri, int x, int y, int w, int h) {

		cout << "PresentationEngineManager::openNclDocument docUri '";
		cout << docUri << "' x = '" << x << "', y = '" << y << "', w = '";
		cout << w << "', h = '" << h << "'" << endl;
	}

	//#if HAVE_DSMCC
	void* PresentationEngineManager::getDsmccListener() {
		return this->dsmccListener;
	}

	void PresentationEngineManager::pausePressed() {
		if (paused) {
			updateFormatters(UC_RESUME);

		} else {
			updateFormatters(UC_PAUSE);
		}
	}

	void PresentationEngineManager::startConnector(string connectorFile) {
		std::cout << " * Connector Started *" << std::endl;
		connector = new connector::Connector( (!connectorFile.empty()) ? connectorFile : GINGA_CONNECTOR_SOCKET, false );
		setupEditingCommandHandler();
		setupExitCommandHandler();
		setupKeepAliveHandler();
		// Attach connector to Input Manager
		im->setConnector(connector);
	}

	/*
		void PresentationEngineManager::startPresentation(string documentId) {

			INCLPlayer* formatter = getNclPlayer(documentId);

			if (formatter == NULL) {
				cout << "[PresentationEngineManager] presentation could not be started." << endl;
				return;
			}

			setTimeBaseInfo(formatter);
			formatter->play();
		}*/

	void PresentationEngineManager::stopPresentation() {
		cout << "PresentationEngineManager::stopPresentation" << endl;

		updateFormatters(UC_STOP);
		checkStatus();
	}

	void PresentationEngineManager::setCmdFile(string cmdFile) {
		ifstream fis;
		string cmd;

		fis.open(cmdFile.c_str(), ifstream::in);
		if (!fis.is_open()) {
			cout << "PresentationEngineManager";
			cout << "::setCmdFile Warning! can't open '" << cmdFile;
			cout << "'" << endl;
			return;
		}

		if (commands != NULL) {
			delete commands;
			commands = NULL;
		}

		commands = new vector<string>;

		while (fis.good()) {
			fis >> cmd;
			if (cmd != "" && cmd.substr(0, 1) != "#") {
				commands->push_back(cmd);
			}
		}
	}

	void PresentationEngineManager::waitUnlockCondition() {
		Thread::waitForUnlockCondition();
	}

	void PresentationEngineManager::presentationCompleted(string formatterId) {
		releaseFormatter(formatterId);
		cout << "[PresentationEngineManager] presentation completed." << endl;
	}

	void PresentationEngineManager::releaseFormatter(string formatterId) {
		map<string, INCLPlayer*>::iterator i;
		INCLPlayer* formatter;

		lock();
		if (formatterId == "") {
			i = formatters->begin();

		} else {
			i = formatters->find(formatterId);
		}

		if (i != formatters->end()) {
			formatter = i->second;
			formatters->erase(i);
			formattersToRelease->insert(formatter);
			unlock();
			pthread_mutex_lock(&releasingMutex);
			// Thread::start();
			// Lock createNclPlayer until started thread is done...
			run();
		} else {
			unlock();
		}

		checkStatus();
	}

	bool PresentationEngineManager::checkStatus() {
		bool checked = false;

		lock();
		if (formatters->empty() && isLocalNcl) {
			if (this->enableGfx){
				ShowButton::getInstance()->stop();
				USLEEP(900000);
			}


			checked = true;
		}
		unlock();

		if (checked) {
			close();
			unlockConditionSatisfied();
		}

		return checked;
	}

	void PresentationEngineManager::updateStatus(
		short code, string parameter, short type) {

		struct inputEventNotification* ev;

		switch (code) {
		case IPlayer::PL_NOTIFY_STOP:
			if (type == IPlayer::TYPE_PRESENTATION) {
				ev = new struct inputEventNotification;

				ev->p         = this;
				ev->parameter = parameter;
				ev->code      = code;
				ev->enableGfx = enableGfx;

				/*pthread_t notifyThreadId_;

				pthread_create(
						&notifyThreadId_,
						0,
						PresentationEngineManager::eventReceived,
						(void*)ev);

				pthread_detach(notifyThreadId_);
									*/
				this->eventReceived((void*)ev);
			}
			break;

		default:
			break;
		}
	}

	bool PresentationEngineManager::userEventReceived(IInputEvent* ev) {
		struct inputEventNotification* evR;

		evR            = new struct inputEventNotification;
		evR->p         = this;
		evR->parameter = "";
		evR->code      = ev->getKeyCode();
		evR->cmds      = commands;
		evR->enableGfx = enableGfx;

		pthread_t notifyThreadId_;
		pthread_create(
			&notifyThreadId_,
			0, PresentationEngineManager::eventReceived, (void*)evR);

		pthread_detach(notifyThreadId_);
		return true;
	}

	void* PresentationEngineManager::eventReceived(void* ptr) {
		struct inputEventNotification* ev;
		PresentationEngineManager* p;
		string parameter;
		vector<string>* cmds;

		ev             = (struct inputEventNotification*)ptr;

		const int code = ev->code;
		p              = ev->p;
		parameter      = ev->parameter;
		cmds           = ev->cmds;
		bool enableGfx = ev->enableGfx;

		if (code == CodeMap::KEY_EXIT){
			cout << "[Ginga] Process killed by user request." << endl;
			kill(getpid(),SIGKILL);

		} else if (parameter != "" && code == IPlayer::PL_NOTIFY_STOP) {
			USLEEP(100000);
			p->presentationCompleted(parameter);

		} else if (code == CodeMap::KEY_POWER || code == CodeMap::KEY_F10) {
			if (enableGfx){
				ShowButton::getInstance()->stop();
				USLEEP(900000);
				printTimeStamp();
			}
			cout << "PresentationEngineManager::eventReceived power off!" << endl;
			p->setIsLocalNcl(true);
			p->stopPresentation();

		} else if (code == CodeMap::KEY_STOP || code == CodeMap::KEY_F11) {
			if (enableGfx){
				ShowButton::getInstance()->stop();
				USLEEP(900000);
				printTimeStamp();
			}
			cout << "PresentationEngineManager::eventReceived stop apps!";
			cout << endl;
			p->stopPresentation();

		} else if (code == CodeMap::KEY_PRINTSCREEN ||
					code == CodeMap::KEY_SUPER) {

			p->getScreenShot();

		} else if (code == CodeMap::KEY_PAUSE || code == CodeMap::KEY_F12) {
			if (enableGfx){
				ShowButton::getInstance()->pause();
			}
			p->pausePressed();

		} else if (code == CodeMap::KEY_PLUS_SIGN && cmds != NULL) {
			if (!cmds->empty()) {
				string cmd = *(cmds->begin());
				cout << "RUNNING CURRENT COMMAND '" << cmd;
				cout << "'" << endl;

				p->readCommand(cmd);
				cmds->erase(cmds->begin());
				cmds->push_back(cmd);
			}

		} else if (code == CodeMap::KEY_GREATER_THAN_SIGN) {
			cout << ">> TIME SHIFT >>" << endl;
			p->updateFormatters(UC_SHIFT, "forward");

		} else if (code == CodeMap::KEY_LESS_THAN_SIGN) {
			cout << "<< TIME SHIFT <<" << endl;
			p->updateFormatters(UC_SHIFT, "backward");
		}

		delete ev;
		return NULL;
	}

	void PresentationEngineManager::readCommand(string command) {
		string cmdTag = "", cmdParams = "", editingCmd = "", cmdHeader = "";

		if (command.find("addDocument") != std::string::npos) {
			cmdTag = "5";

		} else if (command.find("startDocument") != std::string::npos) {
			cmdTag = "7";
		}

		if (cmdTag != "" && command.find("(") != std::string::npos) {
			cmdParams = command.substr(
							command.find_first_of("(") + 1,
							command.length() - (command.find_first_of("(") + 1));

			if (cmdParams.find(")") != std::string::npos) {
				cmdParams     = cmdParams.substr(
									0, cmdParams.find_last_of(")"));

				editingCmd    = cmdTag + "1" + cmdParams;
				cmdHeader     = "02000000000";
				cmdHeader[10] = editingCmd.length();
				editingCommand(cmdHeader + editingCmd);
			}
		}
	}

	/*
		void PresentationEngineManager::addRegion(string location) {
			if (location != "" && formatter != NULL) {
				//TODO choose the parent region
				formatter->addRegion(currentDocument->getId(), NULL, location);
			}
		}

		void PresentationEngineManager::removeRegion(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeRegion(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addRegionBase(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addRegionBase(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeRegionBase(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeRegionBase(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addRule(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addRule(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeRule(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeRule(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addRuleBase(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addRuleBase(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeRuleBase(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeRuleBase(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addConnector(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addConnector(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeConnector(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeConnector(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addConnectorBase(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addConnectorBase(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeConnectorBase(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeConnectorBase(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addTransition(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addTransition(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeTransition(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeTransition(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addTransitionBase(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addTransitionBase(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeTransitionBase(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeTransitionBase(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addDescriptor(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addDescriptor(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeDescriptor(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeDescriptor(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addDescriptorBase(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addDescriptorBase(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeDescriptorBase(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeDescriptorBase(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addImportBase(string id, string location) {
			if (location != "" && id != "" && formatter != NULL) {
				formatter->addImportBase(currentDocument->getId(), id, location);
			}
		}

		void PresentationEngineManager::removeImportBase(
				string id, string location) {

			if (id != "" && location != "" && formatter != NULL) {
				formatter->removeImportBase(currentDocument->getId(), id, location);
			}
		}

		void PresentationEngineManager::addImportedDocumentBase(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addImportedDocumentBase(
						currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeImportedDocumentBase(string id) {
			if (id != "" && formatter != NULL) {
				formatter->removeImportedDocumentBase(currentDocument->getId(), id);
			}
		}

		void PresentationEngineManager::addImportNCL(string location) {
			if (location != "" && formatter != NULL) {
				formatter->addImportNCL(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::removeImportNCL(string location) {
			if (location != "" && formatter != NULL) {
				formatter->removeImportNCL(currentDocument->getId(), location);
			}
		}

		void PresentationEngineManager::addNode(string id, string location) {
			if (location != "" && id != "" && formatter != NULL) {
				formatter->addNode(currentDocument->getId(), id, location);
			}
		}

		void PresentationEngineManager::removeNode(
				string compositionId, string nodeId) {

			if (compositionId != "" && nodeId != "" && formatter != NULL) {
				formatter->removeNode(
						currentDocument->getId(), compositionId, nodeId);
			}
		}

		void PresentationEngineManager::addInterface(string id, string location) {
			if (location != "" && id != "" && formatter != NULL) {
				formatter->addInterface(currentDocument->getId(), id, location);
			}
		}

		void PresentationEngineManager::removeInterface(
				string nodeId, string interfaceId) {

			if (nodeId != "" && interfaceId != "" && formatter != NULL) {
				formatter->removeInterface(
						currentDocument->getId(), nodeId, interfaceId);
			}
		}

		void PresentationEngineManager::addLink(string id, string location) {
			if (location != "" && id != "" && formatter != NULL) {
				formatter->addLink(currentDocument->getId(), id, location);
			}
		}

		void PresentationEngineManager::removeLink(
				string compositionId, string linkId) {

			if (compositionId != "" && linkId != "" && formatter != NULL) {
				formatter->removeLink(
						currentDocument->getId(), compositionId, linkId);
			}
		}
	*/


	INCLPlayer* PresentationEngineManager::getNclPlayer(string documentId) {
		map<string, INCLPlayer*>::iterator i;
		INCLPlayer* nclPlayer = NULL;

		lock();

		i = formatters->find(documentId);
		if (i != formatters->end()) {
			nclPlayer = i->second;
		}

		unlock();
		return nclPlayer;
	}
	/*
		INCLPlayer* PresentationEngineManager::getNclPlayer(string docLocation) {
			map<string, INCLPlayer*>::iterator i;
			INCLPlayer* nclPlayer;

			lock();
			string cp = getCurrentPath();

			i = formatters->find(docLocation);
			if (i == formatters->end() && docLocation.find(cp) != string::npos) {
							// TODO Verify if still needed.
				cout << "[14] PresentationEngineManager::getNclPlayer(string): HACK: " << docLocation << "::>";
				docLocation = docLocation.substr(cp.size());
				cout << docLocation << endl;
				i = formatters->find(docLocation);
			}
			if (i != formatters->end()) {
				nclPlayer = i->second;
				unlock();

				return nclPlayer;
			}

			unlock();
			return NULL;
		}

		INCLPlayer* PresentationEngineManager::getNclPlayer(
				string baseId, string docId) {

			string docLocation;

			docLocation = privateBaseManager->getDocumentLocation(baseId, docId);
			return getNclPlayer(docLocation);
		}
	*/
	void PresentationEngineManager::updateFormatters (
		short command, string parameter ) {

		map<string, INCLPlayer*>::iterator i;
		INCLPlayer* formatter;

		lock();
		i = formatters->begin();
		while (i != formatters->end()) {
			formatter = i->second;
			switch (command) {
			case UC_STOP:
				formatter->removeListener(this);
				formatter->stop();
				formattersToRelease->insert(formatter);
				break;

			case UC_PRINTSCREEN:
				cout << "PRINTSCREEN" << endl;
				formatter->getScreenShot();
				unlock();
				return;

			case UC_BACKGROUND:
				formatter->setBackgroundImage ( parameter );
				break;

			case UC_PAUSE:
				paused = true;
				formatter->pause();
				break;

			case UC_RESUME:
				paused = false;
				formatter->resume();
				break;

			case UC_SHIFT:
				cout << "PresentationEngineManager::updateFormatters";
				cout << " shifting time" << endl;
				formatter->timeShift ( parameter );
				unlock();
				return;

			default:
				break;
			}
			++i;
		}

		if (command == UC_STOP) {
			cout << "PresentationEngineManager::updateFormatters";
			cout << " UC_STOP" << endl;
			formatters->clear();
			Thread::start();
		}

		unlock();
	}

	void PresentationEngineManager::run() {
		set<INCLPlayer*>::iterator i;

		lock();
		if (formattersToRelease != NULL && !formattersToRelease->empty()) {
			i = formattersToRelease->begin();
			while (i != formattersToRelease->end()) {
				delete *i;
				++i;
			}
			formattersToRelease->clear();
		}
		unlock();
		pthread_mutex_unlock(&releasingMutex);
	}

	void PresentationEngineManager::connectorKeepAliveEvent(connector::KeepAliveData *data) {
		keepAliveHandler->send(connector);
	}

	void PresentationEngineManager::addDocumentEvent (AddDocumentEC *event) {
	    std::cout << "[PresentationEngineManager] AddDocument Event received " << std::endl;
		
		if ( (event->getBaseId() != currentPrivateBaseId) and (event->getBaseId() != "null") ) {
		  std::cout << "[PresentationEngineManager] BaseId '" << event->getBaseId() << "' not found" << std::endl;
		  return;
		}
		
		INCLPlayer* docPlayer     = NULL;

		for ( int i = 0; i < event->getDocumentCount(); ++i ) {
			if ( event->getDocumentIor ( i ) == "" ) {
				std::string docUri = event->getDocumentUri(i);


				/*
				//docPlayer = getNclPlayer ( event->getDocumentID() ); TODO

				// HACK Remove document if it exists...
				if ( docPlayer != NULL ) {
					std::cout << "***** Eliminando documento existente!!!!\n\n\n" << std::endl;
					StopDocumentEC* stopec = new StopDocumentEC();
					stopec->setBaseID ( event->getBaseID() );

					// TODO Should NPT be delayed???
					stopec->setNPT ( 0 );
					stopec->setData ( event->getDocumentUri ( i ) );
					std::cout << "PEM::connectorAddDocumentEvent stopping previous" << std::endl;
					stopDocumentEvent ( stopec );
					std::cout << "PEM::connectorAddDocumentEvent stopping previous done" << std::endl;
				}*/

				std::cout << "PEM::connectorAddDocumentEvent createNclPlayer" << std::endl;
				pthread_mutex_lock ( &releasingMutex );
				lock();
				//docPlayer = createNclPlayer ( event->getBaseId(), docUri );
				docPlayer = createNclPlayer ( currentPrivateBaseId, docUri );
				unlock();
				pthread_mutex_unlock ( &releasingMutex );

				std::cout << "PEM::connectorAddDocumentEvent createNclPlayer done" << std::endl;
			} else {
				// TODO This must be an StreamEvent received addDocument.
				std::cout << "[PresentationEngineManager] Unsupported AddDocument format: IOR" << std::endl;
			}
		}
		std::cout << "[PresentationEngineManager] AddDocument processed." << std::endl;		
	}

	void PresentationEngineManager::startDocumentEvent (StartDocumentEC *event) {
		std::cout << "[PresentationEngineManager] StartDocument Event received " << std::endl;
		
		if ( (event->getBaseId() != currentPrivateBaseId) and (event->getBaseId() != "null") ) {
		  std::cout << "[PresentationEngineManager] BaseId '" << event->getBaseId() << "' not found" << std::endl;
		  return;
		}
		
		INCLPlayer* docPlayer = getNclPlayer ( event->getDocumentId() );

		if ( docPlayer != NULL ) {
			setTimeBaseInfo ( docPlayer );
			docPlayer->editingCommand (event);
		}
	}

	void PresentationEngineManager::stopDocumentEvent (StopDocumentEC *event) {
		std::cout << "[PresentationEngineManager] StopDocument Event received " << std::endl;
		
		if ( (event->getBaseId() != currentPrivateBaseId) and (event->getBaseId() != "null") ) {
		  std::cout << "[PresentationEngineManager] BaseId '" << event->getBaseId() << "' not found" << std::endl;
		  return;
		}
		
		INCLPlayer* docPlayer = getNclPlayer ( event->getDocumentId() );

		if ( docPlayer != NULL ) {
			setTimeBaseInfo ( docPlayer );
			docPlayer->editingCommand ( event );
		}
	}

	void PresentationEngineManager::addLinkEvent ( AddLinkEC *event ) {
		std::cout << "[PresentationEngineManager] AddLink Event received " << std::endl;
		
		if ( (event->getBaseId() != currentPrivateBaseId) and (event->getBaseId() != "null") ) {
		  std::cout << "[PresentationEngineManager] BaseId '" << event->getBaseId() << "' not found" << std::endl;
		  return;
		}
		
		INCLPlayer* docPlayer = getNclPlayer ( event->getDocumentId() );

		if ( docPlayer != NULL ) {
			setTimeBaseInfo ( docPlayer );
			docPlayer->editingCommand (event);
		}
	}

	void PresentationEngineManager::setPropertyValueEvent ( SetPropertyValueEC *event ) {
		std::cout << "[PresentationEngineManager] SetPropertyValue Event received " << std::endl;

		if ( (event->getBaseId() != currentPrivateBaseId) and (event->getBaseId() != "null") ) {
		  std::cout << "[PresentationEngineManager] BaseId '" << event->getBaseId() << "' not found" << std::endl;
		  return;
		}
		
		INCLPlayer* docPlayer = getNclPlayer (event->getDocumentId());

		if ( docPlayer != NULL ) {
			setTimeBaseInfo ( docPlayer );
			docPlayer->editingCommand ( event );
		}
	}

	void PresentationEngineManager::addNodeEvent ( AddNodeEC *event ) {
		std::cout << "[PresentationEngineManager] AddNode Event received " << std::endl;
		
		if ( (event->getBaseId() != currentPrivateBaseId) and (event->getBaseId() != "null") ) {
		  std::cout << "[PresentationEngineManager] BaseId '" << event->getBaseId() << "' not found" << std::endl;
		  return;
		}
		
		INCLPlayer* docPlayer = getNclPlayer ( event->getDocumentId() );

		if ( docPlayer != NULL ) {
			setTimeBaseInfo ( docPlayer );
			docPlayer->editingCommand ( event );
		}
	}

	void PresentationEngineManager::addInterfaceEvent ( AddInterfaceEC *event ) {
		std::cout << "[PresentationEngineManager] AddInterface Event received " << std::endl;
		
		if ( (event->getBaseId() != currentPrivateBaseId) and (event->getBaseId() != "null") ) {
		  std::cout << "[PresentationEngineManager] BaseId '" << event->getBaseId() << "' not found" << std::endl;
		  return;
		}
		
		INCLPlayer* docPlayer = getNclPlayer ( event->getDocumentId() );

		if ( docPlayer != NULL ) {
			setTimeBaseInfo ( docPlayer );
			docPlayer->editingCommand ( event );
		}
	}

	void PresentationEngineManager::editingCommandsDispatcherLoop ()
	{
		std::cout << "[PresentationEngineManager] EC dispatcher working!" << std::endl;
		connector::EditingCommandData* ec = NULL;
		while (!editingCommandsDispatcherDone) {
			pthread_mutex_lock(&editingCommandsLock);
			while (!editingCommandsDispatcherDone && editingCommandsQueue.empty())
				pthread_cond_wait(&editingCommandsCond, &editingCommandsLock);

			// dequeue EC
			if (!editingCommandsQueue.empty()) {
				ec = editingCommandsQueue.front();
				editingCommandsQueue.pop_front();
			}
			pthread_mutex_unlock(&editingCommandsLock);

			// process EC
			if (ec != NULL) {
				this->connectorEditingCommandProcess(ec);
				delete ec;
				ec = NULL;
			}
		}
		std::cout << "[PresentationEngineManager] EC dispatcher finished." << std::endl;
	}
	
    void PresentationEngineManager::connectorEditingCommandEvent(connector::EditingCommandData *event)
    //* Handler para el manejo de editingCommands enviados desde el connector *//
	{
		pthread_mutex_lock(&editingCommandsLock);
		connector::EditingCommandData * copy = new connector::EditingCommandData();
		copy->copyFrom(event);
		editingCommandsQueue.push_back(copy);
		pthread_cond_signal(&editingCommandsCond);
		pthread_mutex_unlock(&editingCommandsLock);
	}
	
    void PresentationEngineManager::connectorEditingCommandProcess(connector::EditingCommandData *event)
    {
        unsigned char commandTag = event->getCommandTag();
        switch (commandTag) {
            case EC_ADD_DOCUMENT: {
                    AddDocumentEC* adec = new AddDocumentEC();
		    if (adec->parsePayload(event))
		      addDocumentEvent(adec);
		    delete adec;
            }
            break;
            case EC_START_DOCUMENT: {
                    StartDocumentEC* sdec = new StartDocumentEC();
                    if (sdec->parsePayload(event)) 
		      startDocumentEvent(sdec);
		    delete sdec;
            }
            break;
            case EC_STOP_DOCUMENT: {
                    StopDocumentEC* sdec = new StopDocumentEC();
                    if (sdec->parsePayload(event)) 
		      stopDocumentEvent(sdec);
		    delete sdec;
            }
            break;
            case EC_ADD_LINK: {
                    AddLinkEC* alec = new AddLinkEC();
                    if (alec->parsePayload(event))
		      addLinkEvent(alec);
		    delete alec;
            }
            break;
            case EC_SET_PROPERTY_VALUE: {
                    SetPropertyValueEC* spvec = new SetPropertyValueEC();
                    if (spvec->parsePayload(event))
		      setPropertyValueEvent(spvec);
		    delete spvec;
            }
            break;
            case EC_ADD_NODE: {
                    AddNodeEC* anec = new AddNodeEC();
                    if (anec->parsePayload(event))
		      addNodeEvent(anec);
		    delete anec;
            }
            break;
            case EC_ADD_INTERFACE: {
                    AddInterfaceEC* aiec = new AddInterfaceEC();
                    if (aiec->parsePayload(event))
		      addInterfaceEvent(aiec);
		    delete aiec;
            }
            break;
            default:
                    break;
        }
    }

	void PresentationEngineManager::setupEditingCommandHandler() {
		// Here we register the editing command handlers
		editingCmdHandler = new connector::EditingCommandHandler();
		editingCmdHandler->onEditingCommandHandler (
			boost::bind ( &PresentationEngineManager::connectorEditingCommandEvent, this, _1 ) );
		connector->addHandler(connector::messages::editingCommand, editingCmdHandler);
	}

	void PresentationEngineManager::setupKeepAliveHandler() {
		keepAliveHandler = new connector::KeepAliveHandler();
		keepAliveHandler->onKeepAliveEvent(
			boost::bind(&PresentationEngineManager::connectorKeepAliveEvent, this, _1));
		connector->addHandler(connector::messages::keepAlive, keepAliveHandler);
	}

	void PresentationEngineManager::setupExitCommandHandler() {
		exitCommandHandler = new connector::ExitCommandHandler();
		exitCommandHandler->onExitEvent(
			boost::bind(&PresentationEngineManager::connectorExitEvent, this));
		connector->addHandler(connector::messages::exit, exitCommandHandler);
	}

	void PresentationEngineManager::connectorExitEvent() {
		std::cout << "[PresentationEngineManager] Exit command received." << std::endl;
		stopPresentation();
	}

}
}
}
}
}

using namespace ::br::pucrio::telemidia::ginga::lssm;
extern "C" IPresentationEngineManager* createPEM(
		int devClass, int xOffset, int yOffset, int w, int h, bool enableGfx) {

	return new PresentationEngineManager(
			devClass, xOffset, yOffset, w, h, enableGfx);
}

extern "C" void destroyPEM(IPresentationEngineManager* pem) {
	delete pem;
}

