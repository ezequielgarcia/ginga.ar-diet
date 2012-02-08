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

#include "../../../include/adapters/application/ApplicationPlayerAdapter.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace adapters {
namespace application {
	ApplicationPlayerAdapter::ApplicationPlayerAdapter(
			IPlayerAdapterManager* manager) : FormatterPlayerAdapter(manager) {

		typeSet.insert("ApplicationPlayerAdapter");
		pthread_mutex_init(&eventMutex, NULL);
		preparedEvents = new map<string, FormatterEvent*>;
		currentEvent = NULL;
		editingCommandListener = NULL;
	}

	ApplicationPlayerAdapter::~ApplicationPlayerAdapter() {
		lockEvent();
		if (preparedEvents != NULL) {
			delete preparedEvents;
			preparedEvents = NULL;
		}
		currentEvent = NULL;
		unlockEvent();
		pthread_mutex_destroy(&eventMutex);
	}

	void ApplicationPlayerAdapter::setNclEditListener(
			INclEditListener* listener) {

		this->editingCommandListener = listener;
	}

	bool ApplicationPlayerAdapter::hasPrepared() {
		bool presented;

		if (object == NULL || player == NULL) {
			return false;
		}

		presented = player->hasPresented();
		if (presented) {
			return false;
		}

		return true;
	}

	bool ApplicationPlayerAdapter::prepare(
			ExecutionObject* object, FormatterEvent* event) {

		Content* content;

		lockEvent();
		if (object == NULL) {
			unlockEvent();
			return false;
		}

		if (this->object != object) {
			preparedEvents->clear();

			lockObject();
			this->object = object;
			unlockObject();

			if (this->object->getDataObject() != NULL &&
					this->object->getDataObject()->getDataEntity() != NULL) {

				content = ((NodeEntity*)(object->getDataObject()->
						getDataEntity()))->getContent();

				if (content != NULL && content->instanceOf(
						"ReferenceContent")) {

					this->mrl = ((ReferenceContent*)content)->
						    getCompleteReferenceUrl();
				} else {
					this->mrl = "";
				}
			}

			if (anchorMonitor != NULL) {
				anchorMonitor->stopMonitor();
				delete anchorMonitor;
				anchorMonitor = NULL;
			}

			if (player != NULL) {
				delete player;
				player = NULL;
			}
			createPlayer();
		}

		if (event->getCurrentState() == EventUtil::ST_SLEEPING) {
			this->object->prepare(event, 0);
			unlockEvent();
			prepare(event);
			return true;
		}

		unlockEvent();
		return false;
	}

	void ApplicationPlayerAdapter::prepare(FormatterEvent* event) {
		double duration;
		IntervalAnchor* intervalAnchor;
		CascadingDescriptor* descriptor;
		LayoutRegion* region;

		lockEvent();
		if (event->getCurrentState() != EventUtil::ST_SLEEPING) {
			unlockEvent();
			return;
		}

		descriptor = object->getDescriptor();
		if (descriptor != NULL) {
			region = descriptor->getRegion();
			if (region != NULL) {
				player->setNotifyContentUpdate(
						region->getDeviceClass() == 1);
			}
		}

		if (event->instanceOf("AnchorEvent")) {
			if ((((AnchorEvent*)event)->getAnchor())->instanceOf(
					"LambdaAnchor")) {

				duration = ((PresentationEvent*)event)->getDuration();

				if (duration < IntervalAnchor::OBJECT_DURATION) {
					player->setScope(
							"",
							IPlayer::TYPE_PRESENTATION,
							0.0, duration / 1000);
				}

			} else if (((((AnchorEvent*)event)->getAnchor()))->
					instanceOf("IntervalAnchor")) {

				intervalAnchor = (IntervalAnchor*)(
						((AnchorEvent*)event)->getAnchor());

				player->setScope(
						((AnchorEvent*)event)->getAnchor()->getId(),
						IPlayer::TYPE_PRESENTATION,
						(intervalAnchor->getBegin() / 1000),
						(intervalAnchor->getEnd() / 1000));

			} else if (((((AnchorEvent*)event)->getAnchor()))->
					instanceOf("LabeledAnchor")) {

				player->setScope(((LabeledAnchor*)((AnchorEvent*)event)->
						getAnchor())->getLabel(), IPlayer::TYPE_PRESENTATION);
			}
		}

		(*preparedEvents)[event->getId()] = event;
		unlockEvent();
	}

	bool ApplicationPlayerAdapter::start() {
		if (im != NULL) {
			im->addInputEventListener(this, object->getInputEvents());
		}

		if (object->start()) {
			player->play();
			return true;

		} else {
			if (im != NULL) {
				im->removeInputEventListener(this);
			}
		}
		return false;
	}

	bool ApplicationPlayerAdapter::stop() {
		map<string, FormatterEvent*>::iterator i;
		FormatterEvent* event;

		/*cout << "ApplicationPlayerAdapter::stop() '" << object->getId();
		cout << "'" << endl;*/

		if (im != NULL) {
			im->removeInputEventListener(this);
		}

		if (player != NULL && !player->isForcedNaturalEnd()) {
			/*cout << "ApplicationPlayerAdapter::stop() call player->stop";
			cout << endl;*/

			player->stop();
			player->notifyReferPlayers(EventUtil::TR_STOPS);

		} else {
			i = preparedEvents->begin();
			while (i != preparedEvents->end()) {
				event = i->second;
				if (event != NULL && event->instanceOf("AnchorEvent") &&
						((AnchorEvent*)event)->getAnchor() != NULL &&
						((AnchorEvent*)event)->getAnchor()->instanceOf(
								"LambdaAnchor")) {

					currentEvent = event;
					currentEvent->stop();

					/*cout << "ApplicationPlayerAdapter::stop 1 call unprepare";
					cout << endl;*/
					unprepare();
					return true;
				}
				++i;
			}
		}

		if (object != NULL && object->stop()) {
			/*cout << "ApplicationPlayerAdapter::stop 2 call unprepare";
			cout << endl;*/

			unprepare();
			return true;
		}

		cout << "ApplicationPlayerAdapter::stop() Warning! Should never ";
		cout << "reaches here!" << endl;
		return false;
	}

	bool ApplicationPlayerAdapter::pause() {
		if (object != NULL && object->pause()) {
			player->pause();
			player->notifyReferPlayers(EventUtil::TR_PAUSES);
			return true;

		} else {
			return false;
		}
	}

	bool ApplicationPlayerAdapter::resume() {
		if (object != NULL && object->resume()) {
			player->resume();
			player->notifyReferPlayers(EventUtil::TR_RESUMES);
			return true;
		}
		return false;
	}

	bool ApplicationPlayerAdapter::abort() {
		if (im != NULL) {
			im->removeInputEventListener(this);
		}

		player->stop();
		player->notifyReferPlayers(EventUtil::TR_ABORTS);

		if (object != NULL && object->abort()) {
			unprepare();
			return true;
		}

		return false;
	}

	bool ApplicationPlayerAdapter::unprepare() {
		map<string, FormatterEvent*>::iterator i;

		/*cout << "ApplicationPlayerAdapter::unprepare() '" << object->getId();
		cout << "'" << endl;*/

		lockEvent();
		if (currentEvent == NULL) {
			cout << "ApplicationPlayerAdapter::unprepare currentEvent ";
			cout << "is NULL" << endl;

			if (object != NULL) {
				manager->removePlayer(object);
				object->unprepare();
			}

			unlockEvent();
			return true;
		}

		if (currentEvent->getCurrentState() == EventUtil::ST_OCCURRING ||
				currentEvent->getCurrentState() == EventUtil::ST_PAUSED) {

			/*cout << "ApplicationPlayerAdapter::unprepare call stop";
			cout << endl;*/

			currentEvent->stop();
		}

		if (preparedEvents->count(currentEvent->getId()) != 0 &&
				preparedEvents->size() == 1) {

			if (object != NULL) {
				object->unprepare();
			}

			preparedEvents->clear();
			manager->removePlayer(object);
			object = NULL;

		} else {
			if (object != NULL) {
				object->unprepare();
			}

			i = preparedEvents->find(currentEvent->getId());
			if (i != preparedEvents->end()) {
				preparedEvents->erase(i);
			}
		}
		unlockEvent();
		return true;
	}

	void ApplicationPlayerAdapter::naturalEnd() {
		map<string, FormatterEvent*>::iterator i;
		FormatterEvent* event;

		/*cout << "ApplicationPlayerAdapter::naturalEnd '";
		cout << object->getId() << "'";
		cout << endl;*/

		if (player != NULL) {
			player->notifyReferPlayers(EventUtil::TR_STOPS);
		}

		i = preparedEvents->begin();
		while (i != preparedEvents->end()) {
			event = i->second;
			if (event != NULL && event->instanceOf("AnchorEvent") &&
					((AnchorEvent*)event)->getAnchor() != NULL &&
					((AnchorEvent*)event)->getAnchor()->instanceOf(
							"LambdaAnchor")) {

				currentEvent = event;
				currentEvent->stop();
				unprepare();
				return;
			}
			++i;
		}

		if (object != NULL && object->stop()) {
			/*cout << "ApplicationPlayerAdapter::naturalEnd call unprepare";
			cout << endl;*/
			unprepare();
		}
	}

	void ApplicationPlayerAdapter::updateStatus(
			short code, string param, short type) {

		switch(code) {
			case IPlayer::PL_NOTIFY_START:
				if (object != NULL) {
					startEvent(param, type);
				}
				break;

			case IPlayer::PL_NOTIFY_PAUSE:
				if (object != NULL) {
					pauseEvent(param, type);
				}
				break;

			case IPlayer::PL_NOTIFY_RESUME:
				if (object != NULL) {
					resumeEvent(param, type);
				}
				break;

			case IPlayer::PL_NOTIFY_STOP:
				if (object != NULL) {
					if (param == "") {
						naturalEnd();
					} else {
						/*cout << "ApplicationPlayerAdapter::updateStatus";
						cout << " call stopEvent '" << param << "'";
						cout << " type '" << type << "'";
						cout << endl;*/
						stopEvent(param, type);
					}
				}
				break;

			case IPlayer::PL_NOTIFY_ABORT:
				if (object != NULL) {
					abortEvent(param, type);
				}
				break;

			case IPlayer::PL_NOTIFY_NCLEDIT:
				if (editingCommandListener != NULL) {
					editingCommandListener->nclEdit(param);
				}
				break;
		}
	}

	bool ApplicationPlayerAdapter::startEvent(string anchorId, short type) {
		FormatterEvent* event;

		event = object->getEventFromAnchorId(anchorId);
		if (event != NULL) {
			if ((event->instanceOf("PresentationEvent") &&
					type == IPlayer::TYPE_PRESENTATION) ||
					(event->instanceOf("AttributionEvent") &&
							type == IPlayer::TYPE_ATTRIBUTION)) {

				return event->start();
			}
		}

		return false;
	}

	bool ApplicationPlayerAdapter::stopEvent(string anchorId, short type) {
		FormatterEvent* event;

		if (object->getId() == anchorId) {
			naturalEnd();
			return false;

		} else {
			event = object->getEventFromAnchorId(anchorId);
		}

		if (event != NULL) {
			if ((event->instanceOf("PresentationEvent") &&
					type == IPlayer::TYPE_PRESENTATION) ||
					(event->instanceOf("AttributionEvent") &&
							type == IPlayer::TYPE_ATTRIBUTION)) {

				return event->stop();
			}

		} else {
			cout << "ApplicationPlayerAdapter::stopEvent event not found '";
			cout << anchorId;
			cout << "' in object '" << object->getId();
			cout << endl;
		}

		return false;
	}

	bool ApplicationPlayerAdapter::abortEvent(string anchorId, short type) {
		FormatterEvent* event;

		event = object->getEventFromAnchorId(anchorId);
		if (event != NULL) {
			if ((event->instanceOf("PresentationEvent") &&
					type == IPlayer::TYPE_PRESENTATION) ||
					(event->instanceOf("AttributionEvent") &&
							type == IPlayer::TYPE_ATTRIBUTION)) {

				return event->abort();
			}
		}

		return false;
	}

	bool ApplicationPlayerAdapter::pauseEvent(string anchorId, short type) {
		FormatterEvent* event;

		event = object->getEventFromAnchorId(anchorId);
		if (event != NULL) {
			if ((event->instanceOf("PresentationEvent") &&
					type == IPlayer::TYPE_PRESENTATION) ||
					(event->instanceOf("AttributionEvent") &&
							type == IPlayer::TYPE_ATTRIBUTION)) {

				return event->pause();
			}
		}

		return false;
	}

	bool ApplicationPlayerAdapter::resumeEvent(string anchorId, short type) {
		FormatterEvent* event;

		event = object->getEventFromAnchorId(anchorId);
		if (event != NULL) {
			if ((event->instanceOf("PresentationEvent") &&
					type == IPlayer::TYPE_PRESENTATION) ||
					(event->instanceOf("AttributionEvent") &&
							type == IPlayer::TYPE_ATTRIBUTION)) {

				return event->resume();
			}
		}

		return false;
	}

	void ApplicationPlayerAdapter::lockEvent() {
		pthread_mutex_lock(&eventMutex);
	}

	void ApplicationPlayerAdapter::unlockEvent() {
		pthread_mutex_unlock(&eventMutex);
	}
}
}
}
}
}
}
}
