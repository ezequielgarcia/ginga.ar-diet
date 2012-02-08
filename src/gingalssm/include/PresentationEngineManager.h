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

#ifndef PRESENTATIONENGINEMANAGER_H_
#define PRESENTATIONENGINEMANAGER_H_

#include "IPresentationEngineManager.h"
#include "player/IPlayerListener.h"
#include "ncl/layout/LayoutRegion.h"
#include "ncl/NclDocument.h"
#include <ncl-presenter/privatebase/IPrivateBaseManager.h>
#include <system/thread/Thread.h>
#include <system/io/IInputManager.h>
#include <system/io/ILocalDeviceManager.h>
#include <system/io/interface/device/IDeviceScreen.h>
#include <system/io/interface/input/IInputEventListener.h>
#include <util/functions.h>
#include <util/mcr.h>

#include <connector/connector/connector.h>

#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <map>

using namespace std;
namespace sys    = ::br::pucrio::telemidia::ginga::core::system;
namespace player = ::br::pucrio::telemidia::ginga::core::player;
namespace brncl  = ::br::pucrio::telemidia::ncl;
namespace ncl    = ::br::pucrio::telemidia::ginga::ncl;

namespace connector {
class Connector;
class EditingCommandHandler;
class EditingCommandData;
class ExitCommandHandler;
class KeepAliveHandler;
class KeepAliveData;
}

namespace br{
namespace pucrio{
namespace telemidia{
namespace ginga{
namespace ncl{
  //forward declarations
  class DocumentEC;
  class AddDocumentEC;
  class StartDocumentEC;
  class StopDocumentEC;
  class AddLinkEC;
  class SetPropertyValueEC;
  class AddNodeEC;
  class AddInterfaceEC;
}
}
}
}
}

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace lssm {

  

class PresentationEngineManager : public IPresentationEngineManager,
		public player::IPlayerListener,
		public sys::io::IInputEventListener,
		public sys::thread::Thread
{

	private:
		static const short UC_BACKGROUND  = 0;
		static const short UC_PRINTSCREEN = 1;
		static const short UC_STOP        = 2;
		static const short UC_PAUSE       = 3;
		static const short UC_RESUME      = 4;
		static const short UC_SHIFT       = 5;

		int devClass;
		int x;
		int y;
		int w;
		int h;
		bool enableGfx;
		IDeviceScreen *_dfb;

		ncl::IPrivateBaseManager* privateBaseManager;
		connector::Connector *connector;
		map<string, INCLPlayer*>* formatters;
		set<INCLPlayer*>* formattersToRelease;

                // Added to avoid creating NCLPlayers while
                // they are deleted...
                pthread_mutex_t releasingMutex;

		bool paused;
		string iconPath;
		bool standAloneApp;
		bool isLocalNcl;
		void* dsmccListener;
		void* tuner;
		bool closed;
		ITimeBaseProvider* timeBaseProvider;
		//int currentPrivateBaseId;
		std::string currentPrivateBaseId;
		vector<string>* commands;
		static sys::io::ILocalDeviceManager* dm;
		static sys::io::IInputManager* im;

             //   map< string, string> *documentIDs;
		connector::EditingCommandHandler *editingCmdHandler;
		connector::ExitCommandHandler * exitCommandHandler;
		connector::KeepAliveHandler *keepAliveHandler;

		/**
		 * @brief The editingCommands dispatcher thread id.
		 */
		pthread_t editingCommandsDispatcher;

		/**
		* @brief The editingCommands queue lock
		*/
		pthread_mutex_t editingCommandsLock;
		
		/**
		* @brief A condition variable to signal either new editing
		* commands are available, or dispatcher thread should exit.
		*/
		pthread_cond_t  editingCommandsCond;

		/**
		* @brief A queue of EditingCommands.
		*/
		std::deque<connector::EditingCommandData*> editingCommandsQueue;
		
		/**
		* @brief States if dispatcher should quit.
		*/
		bool editingCommandsDispatcherDone;

	public:
		PresentationEngineManager(
				int devClass,
				int xOffset,
				int yOffset,
				int width,
				int height,
				bool disableGfx);

		virtual ~PresentationEngineManager();

		void autoMountOC(bool autoMountIt);
		//void setCurrentPrivateBaseId(unsigned int baseId);
		void setCurrentPrivateBaseId(std::string baseId);
		void setTimeBaseProvider(ITimeBaseProvider* tmp);
		void editingCommand(string editingCommand);
		void setBackgroundImage(string uri);

	private:
		void close();
		void registerKeys();
		void setupEditingCommandHandler();
		void setupExitCommandHandler();
		void setupKeepAliveHandler();
		void setTimeBaseInfo(INCLPlayer* nclPlayer);


	public:
		void getScreenShot();
		bool getIsLocalNcl();
		void setIsLocalNcl(bool isLocal);

	private:
		INCLPlayer* createNclPlayer(string baseId, string fname);

	public:
		NclPlayerData* createNclPlayerData();
		bool startNclFile(string fname);

		//#if HAVE_GINGAJ
		void openNclDocument(string docUri, int x, int y, int w, int h);

		//#if HAVE_DSMCC
		void* getDsmccListener();
		void pausePressed();
		void startConnector(string connectorFile);
		void stopPresentation();

		void setCmdFile(string cmdFile);
		void waitUnlockCondition();

	private:
		void presentationCompleted(string formatterId);
		void releaseFormatter(string formatterId);
		bool checkStatus();

		void updateStatus(short code, string parameter, short type);
		bool userEventReceived(IInputEvent* ev);

		static void* eventReceived(void* ptr);
		void readCommand(string command);
		void setPropertyValue(
			    string nodeId, string attributeId, string value);


		INCLPlayer* getNclPlayer(string documentId);

		//INCLPlayer* getNclPlayer(string docLocation);
		//INCLPlayer* getNclPlayer(string baseId, string docId);
		void updateFormatters(short command, string parameter="");
		void run();

		// Connector Handler Events
		/**
		 * @brief Queues a newly arrived editingCommand to the processing queue.
		 */
		void connectorEditingCommandEvent(connector::EditingCommandData *event);
		
		/**
		* @brief Dispatches an editing command.
		*/
		void connectorEditingCommandProcess(connector::EditingCommandData *event);

		/**
		* @brief Editing command dispatchers starting function.
		*/
		void editingCommandsDispatcherLoop();

		/**
		* @brief Editing command dispatcher starter.
		*/
		static void* editingCommandStart (void* ptr);

 		// Editing commands
 		void addDocumentEvent(ncl::AddDocumentEC *event);
		void startDocumentEvent(ncl::StartDocumentEC *event);
		void stopDocumentEvent(ncl::StopDocumentEC *event);
		void addLinkEvent(ncl::AddLinkEC *event);
		void setPropertyValueEvent(ncl::SetPropertyValueEC *event);
		void addNodeEvent(ncl::AddNodeEC *event);
		void addInterfaceEvent(ncl::AddInterfaceEC *event);

 		void connectorExitEvent();
		void connectorKeepAliveEvent(connector::KeepAliveData *data);
};
}
}
}
}
}

#endif /*PRESENTATIONENGINEMANAGER_H_*/
