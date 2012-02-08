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

#include "../../../include/model/FormatterEvent.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace event {
	FormatterEvent::FormatterEvent(string id, void* executionObject) {
		this->id = id;
		currentState = EventUtil::ST_SLEEPING;
		occurrences = 0;
		coreListeners = new set<IEventListener*>;
		linksListeners = new set<IEventListener*>;
		objectsListeners = new set<IEventListener*>;
		this->executionObject = executionObject;
		typeSet.insert("FormatterEvent");
		deleting = false;
		pthread_mutex_init(&mutex, NULL);
	}

	FormatterEvent::~FormatterEvent() {
		set<IEventListener*>::iterator i;
		deleting = true;

		pthread_mutex_lock(&mutex);
		this->executionObject = NULL;

		// TODO, avoid to leave a link bind with an inconsistent event

		// After TODO
		if (coreListeners != NULL) {
			coreListeners->clear();
			delete coreListeners;
			coreListeners = NULL;
		}

		if (linksListeners != NULL) {
			i = linksListeners->begin();
			while (i != linksListeners->end()) {
				(*i)->stopListening(this);
				++i;
			}
			linksListeners->clear();
			delete linksListeners;
			linksListeners = NULL;
		}

		if (objectsListeners != NULL) {
			i = objectsListeners->begin();
			while (i != objectsListeners->end()) {
				(*i)->stopListening(this);
				++i;
			}
			objectsListeners->clear();
			delete objectsListeners;
			objectsListeners = NULL;
		}

		pthread_mutex_unlock(&mutex);
		pthread_mutex_destroy(&mutex);
	}

	bool FormatterEvent::instanceOf(string s) {
		if (typeSet.empty()) {
			return false;
		} else {
			return (typeSet.find(s) != typeSet.end());
		}
	}

	void FormatterEvent::setId(string id) {
		this->id = id;
	}

	void FormatterEvent::addEventListener(IEventListener* listener) {
		short pType = listener->getPriorityType();

		pthread_mutex_lock(&mutex);

		if (pType == IEventListener::PT_CORE) {
			coreListeners->insert(listener);

		} else if (pType == IEventListener::PT_LINK) {
			linksListeners->insert(listener);

		} else if (pType == IEventListener::PT_OBJECT) {
			objectsListeners->insert(listener);
		}

		pthread_mutex_unlock(&mutex);
	}

	bool FormatterEvent::containsEventListener(IEventListener* listener) {
		pthread_mutex_lock(&mutex);
		if (coreListeners->count(listener) != 0 ||
				linksListeners->count(listener) != 0 ||
				objectsListeners->count(listener) != 0) {

			pthread_mutex_unlock(&mutex);
			return true;
		}
		pthread_mutex_unlock(&mutex);
		return false;
	}

	short FormatterEvent::getNewState(short transition) {
		switch (transition) {
			case EventUtil::TR_STOPS:
				return EventUtil::ST_SLEEPING;

			case EventUtil::TR_STARTS:
			case EventUtil::TR_RESUMES:
				return EventUtil::ST_OCCURRING;

			case EventUtil::TR_PAUSES:
				return EventUtil::ST_PAUSED;

			case EventUtil::TR_ABORTS:
				return ST_ABORTED;

			default:
				return -1;
		}
	}

	short FormatterEvent::getTransition(short newState) {
		switch (currentState) {
			case EventUtil::ST_SLEEPING:
				switch (newState) {
					case EventUtil::ST_OCCURRING:
						return EventUtil::TR_STARTS;
					default:
						return -1;
				}

			case EventUtil::ST_OCCURRING:
				switch (newState) {
					case EventUtil::ST_SLEEPING:
						return EventUtil::TR_STOPS;
					case EventUtil::ST_PAUSED:
						return EventUtil::TR_PAUSES;
					case ST_ABORTED:
						return EventUtil::TR_ABORTS;
					default:
						return -1;
				}

			case EventUtil::ST_PAUSED:
				switch (newState) {
					case EventUtil::ST_OCCURRING:
						return EventUtil::TR_RESUMES;
					case EventUtil::ST_SLEEPING:
						return EventUtil::TR_STOPS;
					case ST_ABORTED:
						return EventUtil::TR_ABORTS;
					default:
						return -1;
				}

			default:
				return -1;
		}
	}

	bool FormatterEvent::abort() {
		switch (currentState) {
			case EventUtil::ST_OCCURRING:
			case EventUtil::ST_PAUSED:
				return changeState(ST_ABORTED, EventUtil::TR_ABORTS);

			default:
				return false;
		}
	}

	bool FormatterEvent::start() {
		switch (currentState) {
			case EventUtil::ST_SLEEPING:
				return changeState(
					    EventUtil::ST_OCCURRING, EventUtil::TR_STARTS);
			default:
				return false;
		}
	}

	bool FormatterEvent::stop() {
		switch (currentState) {
			case EventUtil::ST_OCCURRING:
			case EventUtil::ST_PAUSED:
				return changeState(EventUtil::ST_SLEEPING, EventUtil::TR_STOPS);
			default:
				return false;
		}
	}

	bool FormatterEvent::pause() {
		switch (currentState) {
			case EventUtil::ST_OCCURRING:
				return changeState(EventUtil::ST_PAUSED, EventUtil::TR_PAUSES);

			default:
				return false;
		}
	}

	bool FormatterEvent::resume() {
		switch (currentState) {
			case EventUtil::ST_PAUSED:
				return changeState(
					    EventUtil::ST_OCCURRING, EventUtil::TR_RESUMES);

			default:
				return false;
		}
	}

	void FormatterEvent::setCurrentState(short newState) {
		currentState = newState;
	}

	bool FormatterEvent::changeState(short newState, short transition) {
		set<IEventListener*>::iterator i;
		set<IEventListener*>* coreClone, *linkClone, *objectClone;
		short previousState;

		if (transition == EventUtil::TR_STOPS) {
			occurrences++;
		}

		previousState = currentState;
		currentState = newState;

		if (deleting) {
			return false;
		}

		pthread_mutex_lock(&mutex);
		coreClone = new set<IEventListener*>(*coreListeners);
		linkClone = new set<IEventListener*>(*linksListeners);
		objectClone = new set<IEventListener*>(*objectsListeners);
		pthread_mutex_unlock(&mutex);

		i = coreClone->begin();
		while (i != coreClone->end()) {
			if (deleting) {
				break;
			}

			if (*i != NULL) {
				((IEventListener*)(*i))->eventStateChanged(
					    (void*)this, transition, previousState);
			}
			++i;
		}

		i = linkClone->begin();
		while (i != linkClone->end()) {
			if (deleting) {
				break;
			}

			if (*i != NULL) {
				((IEventListener*)(*i))->eventStateChanged(
					    (void*)this, transition, previousState);
			}
			++i;
		}

		i = objectClone->begin();
		while (i != objectClone->end()) {
			if (deleting) {
				break;
			}

			if (*i != NULL) {
				((IEventListener*)(*i))->eventStateChanged(
					    (void*)this, transition, previousState);
			}
			++i;
		}

		coreClone->clear();
		delete coreClone;
		coreClone = NULL;

		linkClone->clear();
		delete linkClone;
		linkClone = NULL;

		objectClone->clear();
		delete objectClone;
		objectClone = NULL;

		if (currentState == ST_ABORTED) {
			currentState = EventUtil::ST_SLEEPING;
		}

		return true;
	}

	void FormatterEvent::clearEventListeners() {
		pthread_mutex_lock(&mutex);
		coreListeners->clear();
		linksListeners->clear();
		objectsListeners->clear();
		pthread_mutex_unlock(&mutex);
	}

	/*int compareTo(Object object) {

	}*/

	short FormatterEvent::getCurrentState() {
		return currentState;
	}

	void* FormatterEvent::getExecutionObject() {
		return executionObject;
	}

	void FormatterEvent::setExecutionObject(void* object) {
		executionObject = object;
	}

	string FormatterEvent::getId() {
		return id;
	}

	long FormatterEvent::getOccurrences() {
		return occurrences;
	}

	bool FormatterEvent::removeEventListener(IEventListener* listener) {
		bool removed = false;

		set<IEventListener*>::iterator i;
		pthread_mutex_lock(&mutex);
		i = coreListeners->find(listener);
		if (i != coreListeners->end()) {
			coreListeners->erase(i);
			removed = true;
		}

		i = linksListeners->find(listener);
		if (i != linksListeners->end()) {
			linksListeners->erase(i);
			removed = true;
		}

		i = objectsListeners->find(listener);
		if (i != objectsListeners->end()) {
			objectsListeners->erase(i);
			removed = true;
		}

		pthread_mutex_unlock(&mutex);
		return removed;
	}

	string FormatterEvent::getStateName(short state) {
		switch (state) {
			case EventUtil::ST_OCCURRING:
				return "occurring";

			case EventUtil::ST_PAUSED:
				return "paused";

			case EventUtil::ST_SLEEPING:
				return "sleeping";

			default:
				return "";
		}
	}
}
}
}
}
}
}
}
