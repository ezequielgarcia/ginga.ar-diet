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
#include "../../include/adapters/PlayerAdapterManager.h"
#include "../../include/adapters/AdaptersComponentSupport.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace adapters {
	PlayerAdapterManager::PlayerAdapterManager(NclPlayerData* data) : Thread() {
		objectPlayers = new map<string, IPlayerAdapter*>;
		deletePlayers = new map<string, IPlayerAdapter*>;
		playerNames   = new map<IPlayerAdapter*, string>;
		nclPlayerData = data;

		editingCommandListener = NULL;
		epgFactoryAdapter      = NULL;
		timeBaseProvider       = NULL;

		pthread_mutex_init(&mutexPlayer, NULL);

		readConfigFiles();

		running = true;
		start();
	}

	PlayerAdapterManager::~PlayerAdapterManager() {
		running = false;
		unlockConditionSatisfied();
		wakeUp();

		clear();
		clearDeletePlayers();

		pthread_mutex_lock(&mutexPlayer);
		if (objectPlayers != NULL) {
			delete objectPlayers;
			objectPlayers = NULL;
		}

		if (deletePlayers != NULL) {
			delete deletePlayers;
			deletePlayers = NULL;
		}

		mimeDefaultTable->clear();
		delete mimeDefaultTable;
		mimeDefaultTable = NULL;

		playerTable->clear();
		delete playerTable;
		playerTable = NULL;

		pthread_mutex_unlock(&mutexPlayer);
		pthread_mutex_destroy(&mutexPlayer);
	}

	NclPlayerData* PlayerAdapterManager::getNclPlayerData() {
		return nclPlayerData;
	}

	void PlayerAdapterManager::setTimeBaseProvider(
			ITimeBaseProvider* timeBaseProvider) {

		this->timeBaseProvider = timeBaseProvider;
	}

	ITimeBaseProvider* PlayerAdapterManager::getTimeBaseProvider() {
		return timeBaseProvider;
	}

	void PlayerAdapterManager::setVisible(
			string objectId,
			string visible,
			AttributionEvent* event) {

		map<string, IPlayerAdapter*>::iterator i;
		FormatterPlayerAdapter* player;

		pthread_mutex_lock(&mutexPlayer);
		i = objectPlayers->find(objectId);
		if (i != objectPlayers->end()) {
			player = (FormatterPlayerAdapter*)(i->second);
			player->setPropertyValue(event, visible, NULL);
			event->stop();
		}
		pthread_mutex_unlock(&mutexPlayer);
	}

	bool PlayerAdapterManager::removePlayer(void* exObject) {
		ExecutionObject* object;
		bool removed;

		object = (ExecutionObject*)exObject;
		pthread_mutex_lock(&mutexPlayer);
		removed = removePlayer(object->getId());
		pthread_mutex_unlock(&mutexPlayer);

		return removed;
	}

	bool PlayerAdapterManager::removePlayer(string objectId) {
		map<string, IPlayerAdapter*>::iterator i;
		FormatterPlayerAdapter* player;

		i = objectPlayers->find(objectId);
		if (i != objectPlayers->end()) {
			player = (FormatterPlayerAdapter*)(i->second);
			if (!player->instanceOf("ProgramAVPlayerAdapter")) {
				(*deletePlayers)[objectId] = player;
			}
			objectPlayers->erase(i);
			unlockConditionSatisfied();
			return true;
		}

		return false;
	}

	void PlayerAdapterManager::clear() {
		map<string, IPlayerAdapter*>::iterator i;

		pthread_mutex_lock(&mutexPlayer);
		if (objectPlayers != NULL) {
			i = objectPlayers->begin();
			while (i != objectPlayers->end()) {
				if (removePlayer(i->first)) {
					i = objectPlayers->begin();

				} else {
					++i;
				}
			}

			objectPlayers->clear();
		}
		pthread_mutex_unlock(&mutexPlayer);
	}

	void PlayerAdapterManager::setNclEditListener(INclEditListener* listener) {
		this->editingCommandListener = listener;
	}

	string PlayerAdapterManager::getPlayerClass(
		    CascadingDescriptor* descriptor, NodeEntity* dataObject) {

		string toolName = "";
		string mime     = "";
		string upMime   = "";
		string nodeType = "";

		if (descriptor != NULL) {
			toolName = descriptor->getPlayerName();
		}

		if (dataObject->instanceOf("ContentNode")) {
			mime = ((ContentNode*)dataObject)->getNodeType();
			if (mime == "" && toolName == "") {
				return "";
			}

			if (((ContentNode*)dataObject)->isSettingNode()) {
				return "SETTING_NODE";
			}
		}

		if (toolName == "") {
			// there is no player defined, so it should be chose a player
			// based on the node content type
			upMime = upperCase(mime);
			if (mimeDefaultTable->count(upMime) != 0) {
				toolName = (*mimeDefaultTable)[upMime];
			}

			if (toolName != "") {
				return toolName;
			}

		} else {
			if (playerTable->count(toolName)) {
				return (*playerTable)[toolName];
			}
		}

		toolName = "";
		return toolName;
	}

	void PlayerAdapterManager::readConfigFiles() {
		ifstream fisMime;
		ifstream fisCtrl;
		string line, key, value;

		fisMime.open(
				PREFIX_PATH "/ncl-presenter/config/formatter/players/mimedefs.ini",
				ifstream::in);

		if (!fisMime.is_open()) {
			cout << "PlayerAdapterManager: can't open mimedefs.ini" << endl;
			return;
		}

		mimeDefaultTable = new map<string, string>;
		while (fisMime.good()) {
			fisMime >> line;
			key = upperCase(line.substr(0, line.find_last_of("=")));
			value = line.substr(
					(line.find_first_of("=") + 1),
					line.length() - (line.find_first_of("=") + 1));

			(*mimeDefaultTable)[key] = value;
		}

		fisMime.close();

		fisCtrl.open(PREFIX_PATH "/ncl-presenter/config/formatter/players/ctrldefs.ini");
		if (!fisCtrl.is_open()) {
			cout << "PlayerAdapterManager: can't open ctrldefs.ini" << endl;
			return;
		}

		playerTable = new map<string, string>;
		while (fisCtrl.good()) {
			fisCtrl >> line;
			key = line.substr(0, line.find_last_of("="));
			value = line.substr(
					(line.find_first_of("=") + 1),
					line.length() - (line.find_first_of("=") + 1));

			(*playerTable)[key] = value;
		}

		fisCtrl.close();
	}

	FormatterPlayerAdapter* PlayerAdapterManager::initializePlayer(
		    ExecutionObject* object) {

		CascadingDescriptor* descriptor;
		NodeEntity* dataObject;
		string playerClassName, objId;
		IPlayerAdapter* player = NULL;
		vector<string>* args;

		if (object == NULL) {
			return NULL;
		}

		objId = object->getId();
		descriptor = object->getDescriptor();
		dataObject = (NodeEntity*)(object->getDataObject()->getDataEntity());

		// checking if is a main AV reference
		Content* content;
		string url = "";

		playerClassName = "";
		content = dataObject->getContent();
		if (content != NULL) {
			if (content->instanceOf("ReferenceContent")) {
				url = ((ReferenceContent*)(content))->
					    getCompleteReferenceUrl();

				if (url.length() > 9 && url.substr(0,9) == "sbtvd-ts:") {
					playerClassName = "ProgramAVPlayerAdapter";
				}
			}
		}

		if (playerClassName == "") {
			playerClassName = getPlayerClass(descriptor, dataObject);
		}

		if (playerClassName == "SETTING_NODE") {
			return NULL;
		}

		if (playerClassName == "") {
			cout << "PlayerAdapterManager::initializePlayer creating ";
			cout << "local time player" << endl;
			player = new FormatterPlayerAdapter(this);
			(*objectPlayers)[objId] = player;
			return (FormatterPlayerAdapter*)player;
		}

		args = split(playerClassName, ",");
		if (args->size() < 1) {
			delete args;
			return NULL;

		} else if (args->size() == 1) {
			args->push_back("");
		}

		playerClassName = (*args)[0];

#if HAVE_COMPSUPPORT
		player = ((AdapterCreator*)(cm->getObject(
				playerClassName)))(this, (void*)(((*args)[1]).c_str()));
#else
		if (playerClassName == "SubtitlePlayerAdapter") {
			player = new SubtitlePlayerAdapter(this);

		} else if (playerClassName == "PlainTxtPlayerAdapter") {
			player = new PlainTxtPlayerAdapter(this);

		} else if (playerClassName == "LinksPlayerAdapter") {
			player = new LinksPlayerAdapter(this);

		} else if (playerClassName == "ImagePlayerAdapter") {
			player = new ImagePlayerAdapter(this);

		} else if (playerClassName == "AVPlayerAdapter" || playerClassName == "ProgramAVPlayerAdapter") {
			if ((*args)[1] == "true") {
				player = new AVPlayerAdapter(this, true);
			} else {
				player = new AVPlayerAdapter(this, false);
			}

		} else if (playerClassName == "LuaPlayerAdapter") {
			player = new LuaPlayerAdapter(this);

		} else if (playerClassName == "NCLPlayerAdapter") {
			player = new NCLPlayerAdapter(this);

		} else if (playerClassName != "SETTING_NODE") {
			cout << "PlayerAdapterManager::initializePlayer is creating a ";
			cout << "new time player for '" << objId << "'";
			cout << " playerClassName is '" << playerClassName;
			cout << "'" << endl;
			player = new FormatterPlayerAdapter(this);

		} else {
			cout << "PlayerAdapterManager::initializePlayer is returning a ";
			cout << "NULL player for '" << objId << "'" << endl;
			delete args;
			return NULL;
		}
#endif

		if (player == NULL) {
			delete args;
			return NULL;
		}

		if ((*args)[1] == "epg") {
			epgFactoryAdapter = player;
		}

		if ((*args)[1] == "epg" || (*args)[1] == "nclEdit") {
			((FormatterPlayerAdapter*)player)->setNclEditListener(
					editingCommandListener);
		}

		delete args;

		(*objectPlayers)[objId] = player;
		(*playerNames)[player]  = playerClassName;
		return (FormatterPlayerAdapter*)player;
	}

	FormatterPlayerAdapter* PlayerAdapterManager::getPlayer(
		    ExecutionObject* execObj) {

		map<string, IPlayerAdapter*>::iterator i;
		FormatterPlayerAdapter* player;
		string objId;

		pthread_mutex_lock(&mutexPlayer);
		objId = execObj->getId();
		i = objectPlayers->find(objId);
		if (i == objectPlayers->end()) {
			i = deletePlayers->find(objId);
			if (i == deletePlayers->end()) {
				player = initializePlayer(execObj);
			} else {
				player = (FormatterPlayerAdapter*)(i->second);
				deletePlayers->erase(i);
				(*objectPlayers)[objId] = player;
			}

		} else {
			player = (FormatterPlayerAdapter*)(i->second);
		}
		pthread_mutex_unlock(&mutexPlayer);

		return player;
	}

	void PlayerAdapterManager::setStandByState(bool standBy) {
		map<string, IPlayerAdapter*>::iterator i;
		FormatterPlayerAdapter* player;

		pthread_mutex_lock(&mutexPlayer);
		i = objectPlayers->begin();
		while (i != objectPlayers->end()) {
			player = (FormatterPlayerAdapter*)(i->second);
			if (player->getPlayer() != NULL) {
				player->getPlayer()->setStandByState(standBy);
			}
			++i;
		}
		pthread_mutex_unlock(&mutexPlayer);
	}

	bool PlayerAdapterManager::isEmbeddedApp(NodeEntity* dataObject) {
		string mediaType = "";
		string url = "";
		string::size_type pos;
		Descriptor* descriptor;
		Content* content;

		//first, descriptor
		descriptor = dynamic_cast<Descriptor*>(dataObject->getDescriptor());
		if (descriptor != NULL) {
			mediaType = descriptor->getPlayerName();
			if (mediaType != "") {
				if (mediaType == "NCLetPlayerAdapter" ||
						mediaType == "LuaPlayerAdapter" ||
						mediaType == "LinksPlayerAdapter" ||
						mediaType == "NCLPlayerAdapter") {

					return true;
				}
			}
		}

		//second, media type
		if (dataObject->instanceOf("ContentNode")) {
			mediaType = ((ContentNode*)dataObject)->getNodeType();
			if (mediaType != "") {
				return isEmbeddedAppMediaType(mediaType);
			}
		}

		//finally, content file extension
		content = dataObject->getContent();
		if (content != NULL) {
			if (content->instanceOf("ReferenceContent")) {
				url = ((ReferenceContent*)(content))->
					    getCompleteReferenceUrl();

				if (url != "") {
					pos = url.find_last_of(".");
					if (pos != std::string::npos) {
						pos++;
						mediaType = ContentTypeManager::getInstance()->
								getMimeType(
										url.substr(pos, url.length() - pos));

						return isEmbeddedAppMediaType(mediaType);
					}
				}
			}
		}

		return false;
	}

	bool PlayerAdapterManager::isEmbeddedAppMediaType(string mediaType) {
		mediaType = upperCase(mediaType);

		return (mediaType == "APPLICATION/X-GINGA-NCLUA" ||
			mediaType == "APPLICATION/X-GINGA-NCLET" ||
			mediaType == "APPLICATION/X-GINGA-NCL" ||
			mediaType == "APPLICATION/X-NCL-NCL" ||
			mediaType == "APPLICATION/X-NCL-NCLUA"); 
	}

	void PlayerAdapterManager::pushEPGEventToEPGFactory(map<string, string> t) {
		if (epgFactoryAdapter != NULL) {
			//epgFactoryAdapter->pushEPGEvent(t);
		}
	}

	void PlayerAdapterManager::timeShift(string direction) {
		map<string, IPlayerAdapter*>::iterator i;
		FormatterPlayerAdapter* player;

		pthread_mutex_lock(&mutexPlayer);
		i = objectPlayers->begin();
		while (i != objectPlayers->end()) {
			player = (FormatterPlayerAdapter*)(i->second);
			player->timeShift(direction);
			++i;
		}
		pthread_mutex_unlock(&mutexPlayer);
	}

	void PlayerAdapterManager::clearDeletePlayers() {
		map<string, IPlayerAdapter*>::iterator i;
		map<IPlayerAdapter*, string>::iterator j;
		IPlayerAdapter* player;
		string playerClassName = "";

		pthread_mutex_lock(&mutexPlayer);
		i = deletePlayers->begin();
		while (i != deletePlayers->end()) {
			player = i->second;

			j = playerNames->find(player);
			if (j != playerNames->end()) {
				playerClassName = j->second;
				playerNames->erase(j);
			}

			delete player;
#if HAVE_COMPSUPPORT
			if (trim(playerClassName) != "") {
				printTimeStamp();
				cm->releaseComponentFromObject(playerClassName);
			}
#endif
			++i;
		}
		deletePlayers->clear();
		pthread_mutex_unlock(&mutexPlayer);
	}

	void PlayerAdapterManager::run() {
		set<IPlayerAdapter*>::iterator i;

		while(running) {
			pthread_mutex_lock(&mutexPlayer);
			if (deletePlayers->empty()) {
				pthread_mutex_unlock(&mutexPlayer);
				waitForUnlockCondition();

			} else {
				pthread_mutex_unlock(&mutexPlayer);
			}

			if (!running) {
				return;
			}

			usleep(1000);
			if (running) {
				clearDeletePlayers();
			}
		}
	}
}
}
}
}
}
}
