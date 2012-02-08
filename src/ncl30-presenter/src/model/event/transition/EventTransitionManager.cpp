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

#include "../../../../include/model/EventTransitionManager.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace event {
namespace transition {
	EventTransitionManager::EventTransitionManager() {
		currentTransitionIndex = new map<short int, int>;
		startTransitionIndex   = new map<short int, int>;
		transTable             = new map<short int, vector<EventTransition*>*>;
		pthread_mutex_init(&transMutex, NULL);
	}

	EventTransitionManager::~EventTransitionManager() {
		map<short int, vector<EventTransition*>*>::iterator i;
		vector<EventTransition*>::iterator j;

		pthread_mutex_lock(&transMutex);
		if (transTable != NULL) {
			i = transTable->begin();
			while (i != transTable->end()) {
				j = i->second->begin();
				while (j != i->second->end()) {
					delete *j;
					++j;
				}
				i->second->clear();
				delete i->second;
				++i;
			}
			transTable->clear();
			delete transTable;
			transTable = NULL;
		}

		pthread_mutex_unlock(&transMutex);
		pthread_mutex_destroy(&transMutex);
	}

	short int EventTransitionManager::getType(PresentationEvent* event) {
		ContentAnchor* anchor;
		short int valueSyntax;

		anchor = event->getAnchor();
		if (anchor->instanceOf("RelativeTimeIntervalAnchor") ||
				anchor->instanceOf("LambdaAnchor")) {

			return ContentAnchor::CAT_TIME;

		} else if (anchor->instanceOf("SampleIntervalAnchor")) {
			((SampleIntervalAnchor*)anchor)->getValueSyntax(
					&valueSyntax, &valueSyntax);

			return valueSyntax;

		} else {
			return ContentAnchor::CAT_NONE;
		}
	}

	vector<EventTransition*>* EventTransitionManager::getTransitionEvents(
			short int type) {

		vector<EventTransition*>* transitionEvents;
		map<short int, vector<EventTransition*>*>::iterator i;

		if (type == ContentAnchor::CAT_ALL || type == ContentAnchor::CAT_NONE) {
			cout << "EventTransitionManager::getTransitionEvents RETURN NULL";
			cout << endl;
			return NULL;
		}

		pthread_mutex_lock(&transMutex);
		i = transTable->find(type);
		if (i == transTable->end()) {
			transitionEvents    = new vector<EventTransition*>;
			(*transTable)[type] = transitionEvents;

		} else {
			transitionEvents    = i->second;
		}
		pthread_mutex_unlock(&transMutex);

		return transitionEvents;
	}

	void EventTransitionManager::addEventTransition(
			EventTransition* transition, short int type) {

		int beg, end, pos;
		EventTransition* auxTransition;
		vector<EventTransition*>* transitionEvents;

		transitionEvents = getTransitionEvents(type);

		// binary search
		beg = 0;
		end = transitionEvents->size() - 1;
		while (beg <= end) {
			pos = (beg + end) / 2;
			auxTransition = (*transitionEvents)[pos];
			switch (transition->compareTo(auxTransition)) {
				case 0:
					// entrada corresponde a um evento que ja' foi inserido
					return;

				case -1:
					end = pos - 1;
					break;

				case 1:
					beg = pos + 1;
					break;
			}
		}

		transitionEvents->insert((transitionEvents->begin() + beg), transition);
	}

	void EventTransitionManager::removeEventTransition(
			PresentationEvent* event) {

		int i, size;
		vector<EventTransition*>::iterator j;
		EventTransition* transition;
		EventTransition* endTransition;
		vector<EventTransition*>* transitionEvents;
		short int type;

		type             = getType(event);
		transitionEvents = getTransitionEvents(type);
		size             = transitionEvents->size();

		for (i = 0; i < size; i++) {
			transition = (*transitionEvents)[i];
			if (transition->getEvent() == event) {
				if (transition->instanceOf("BeginEventTransition") &&
					    ((BeginEventTransition*)transition)->
					    getEndTransition() != NULL) {

					endTransition = ((BeginEventTransition*)transition)->
						    getEndTransition();

					for (j = transitionEvents->begin();
						    j != transitionEvents->end(); ++j) {

						if (*j == endTransition) {
							transitionEvents->erase(j);
							delete *j;
							break;
						}
					}
				}

				for (j = transitionEvents->begin();
					    j != transitionEvents->end(); ++j) {

					if (*j == transition) {
						transitionEvents->erase(j);
						delete *j;
						break;
					}
				}
				break;
			}
		}
	}

	void EventTransitionManager::resetTimeIndex() {
		map<short int, int>::iterator i;

		pthread_mutex_lock(&transMutex);
		i = startTransitionIndex->begin();
		while (i != startTransitionIndex->end()) {
			(*currentTransitionIndex)[i->first] = i->second;
			++i;
		}
		pthread_mutex_unlock(&transMutex);
	}

	void EventTransitionManager::resetTimeIndexByType(short int type) {
		map<short int, int>::iterator i;

		pthread_mutex_lock(&transMutex);
		i = startTransitionIndex->find(type);
		if (i != startTransitionIndex->end()) {
			(*currentTransitionIndex)[type] = i->second;
		}
		pthread_mutex_unlock(&transMutex);
	}

	void EventTransitionManager::prepare(
			bool wholeContent, double startTime, short int type) {

		vector<EventTransition*>* transitionEvents;
		EventTransition* transition;
		unsigned int transIx, size;

		if (wholeContent && startTime == 0.0) {
			pthread_mutex_lock(&transMutex);
			(*startTransitionIndex)[type] = 0;
			pthread_mutex_unlock(&transMutex);

		} else {
			transitionEvents = getTransitionEvents(type);
			size             = transitionEvents->size();
			transIx          = 0;
			pthread_mutex_lock(&transMutex);
			(*startTransitionIndex)[type] = transIx;
			pthread_mutex_unlock(&transMutex);
			while (transIx < size) {
				transition = (*transitionEvents)[transIx];
				if (transition->getTime() >= startTime) {
					break;
				}

				if (transition->instanceOf("BeginEventTransition")) {
					transition->getEvent()->
						    setCurrentState(EventUtil::ST_OCCURRING);

				} else {
					transition->getEvent()->
						    setCurrentState(EventUtil::ST_SLEEPING);

					transition->getEvent()->incrementOccurrences();
				}
				transIx++;
				pthread_mutex_lock(&transMutex);
				(*startTransitionIndex)[type] = transIx;
				pthread_mutex_unlock(&transMutex);
			}
		}

		resetTimeIndex();
	}

	void EventTransitionManager::start(double offsetTime) {
		vector<EventTransition*>* transitionEvents;
		EventTransition* transition;
		unsigned int transIx, size;

		transitionEvents = getTransitionEvents(ContentAnchor::CAT_TIME);
		size             = transitionEvents->size();

		pthread_mutex_lock(&transMutex);
		if (currentTransitionIndex->count(ContentAnchor::CAT_TIME) == 0) {
			(*currentTransitionIndex)[ContentAnchor::CAT_TIME] = 0;
		}

		transIx = (*currentTransitionIndex)[ContentAnchor::CAT_TIME];
		pthread_mutex_unlock(&transMutex);

		while (transIx < size) {
			transition = (*transitionEvents)[transIx];
			if (transition->getTime() <= offsetTime) {
				if (transition->instanceOf("BeginEventTransition")) {
					transition->getEvent()->start();
				}
				transIx++;
				pthread_mutex_lock(&transMutex);
				(*currentTransitionIndex)[ContentAnchor::CAT_TIME] = transIx;
				pthread_mutex_unlock(&transMutex);

			} else {
				break;
			}
		}
	}

	void EventTransitionManager::stop(double endTime) {
		vector<EventTransition*>* transitionEvents;
		vector<EventTransition*>::iterator i;
		EventTransition* transition;
		FormatterEvent* fev;

		transitionEvents = getTransitionEvents(ContentAnchor::CAT_TIME);

		i = transitionEvents->begin();
		while (i != transitionEvents->end()) {
			transition = *i;
			fev = transition->getEvent();
			if (transition->getTime() > endTime) {
				fev->setCurrentState(EventUtil::ST_SLEEPING);

			} else if (transition->instanceOf("EndEventTransition")) {
				fev->stop();
			}
			++i;
		}
	}

	void EventTransitionManager::abort(double endTime) {
		vector<EventTransition*>* transitionEvents;
		unsigned int transIx, i, size;
		EventTransition* transition;
		FormatterEvent* fev;

		pthread_mutex_lock(&transMutex);
		if (currentTransitionIndex->count(ContentAnchor::CAT_TIME) == 0) {
			(*currentTransitionIndex)[ContentAnchor::CAT_TIME] = 0;
		}

		transIx = (*currentTransitionIndex)[ContentAnchor::CAT_TIME];
		pthread_mutex_unlock(&transMutex);

		transitionEvents = getTransitionEvents(ContentAnchor::CAT_TIME);
		size             = transitionEvents->size();

		for (i = transIx; i < size; i++) {
			transition = (*transitionEvents)[i];
			fev = transition->getEvent();
			if (transition->getTime() > endTime) {
				fev->setCurrentState(EventUtil::ST_SLEEPING);

			} else if (transition->instanceOf("EndEventTransition")) {
				fev->abort();
			}
		}
	}

	void EventTransitionManager::addPresentationEvent(
			PresentationEvent* event) {

		double begin, end;
		BeginEventTransition* beginTransition;
		EndEventTransition* endTransition;
		vector<EventTransition*>* transitionEvents;
		short int type;

		type = getType(event);
		transitionEvents = getTransitionEvents(type);

		if ((event->getAnchor())->instanceOf("LambdaAnchor")) {
			beginTransition  = new BeginEventTransition(0, event);
			transitionEvents->insert(transitionEvents->begin(), beginTransition);
			if (event->getEnd() >= 0) {
				endTransition = new EndEventTransition(
					    event->getEnd(), event, beginTransition);

				transitionEvents->push_back(endTransition);
			}

		} else {
			begin = event->getBegin();

			// undefined events are not inserted into transition table
			if (PresentationEvent::isUndefinedInstant(begin)) {
				return;
			}
			
			beginTransition = new BeginEventTransition(begin, event);
			addEventTransition(beginTransition, type);
			end = event->getEnd();

			if (!PresentationEvent::isUndefinedInstant(end)) {
				endTransition = new EndEventTransition(
					    end, event, beginTransition);

				addEventTransition(endTransition, type);
			}
		}
	}

	void EventTransitionManager::updateTransitionTable(
			double value,
			IPlayer* player,
			FormatterEvent* mainEvent,
			short int transType) {

		EventTransition* transition;
		FormatterEvent* ev;
		vector<EventTransition*>* transitionEvents;
		unsigned int currentIx;

		pthread_mutex_lock(&transMutex);
		if (currentTransitionIndex->count(transType) == 0 ||
				transTable->count(transType) == 0) {

			pthread_mutex_unlock(&transMutex);
			return;
		}

		transitionEvents = (*transTable)[transType];
		currentIx        = (*currentTransitionIndex)[transType];
		pthread_mutex_unlock(&transMutex);

		while (currentIx < transitionEvents->size()) {
			transition = (*transitionEvents)[currentIx];

			if (transition->getTime() <= value) {
				ev = transition->getEvent();
				if (transition->instanceOf("BeginEventTransition")) {
					ev->start();

				} else {
					if (ev == mainEvent && player != NULL) {
						player->setPresented(true);
					}
					ev->stop();
				}

				currentIx++;
				pthread_mutex_lock(&transMutex);
				(*currentTransitionIndex)[transType] = currentIx;
				pthread_mutex_unlock(&transMutex);

			} else {
				break;
			}
		}
	}

	set<double>* EventTransitionManager::getTransitionsValues(
			short int transType) {

		set<double>* transValues;
		unsigned int currentIx, ix;
		vector<EventTransition*>* transitionEvents;
		vector<EventTransition*>::iterator i;

		pthread_mutex_lock(&transMutex);
		if (transTable->count(transType) == 0) {
			pthread_mutex_unlock(&transMutex);
			return NULL;
		}

		if (currentTransitionIndex->count(transType) == 0) {
			if (startTransitionIndex->count(transType) == 0) {
				(*currentTransitionIndex)[transType] = 0;

			} else {
				(*currentTransitionIndex)[transType] = (
						*startTransitionIndex)[transType];
			}
		}

		pthread_mutex_unlock(&transMutex);
		transitionEvents = getTransitionEvents(transType);
		transValues      = new set<double>;

		pthread_mutex_lock(&transMutex);
		currentIx        = (*currentTransitionIndex)[transType];
		pthread_mutex_unlock(&transMutex);

		ix = 0;
		i  = transitionEvents->begin();
		while (i != transitionEvents->end()) {
			if (ix >= currentIx) {
				transValues->insert((*i)->getTime());
			}
			++ix;
			++i;
		}

		return transValues;
	}

	EventTransition* EventTransitionManager::getNextTransition(
			FormatterEvent* mainEvent) {

		EventTransition* transition;
		vector<EventTransition*>* transitionEvents;
		unsigned int currentIx;

		pthread_mutex_lock(&transMutex);
		if (currentTransitionIndex->count(ContentAnchor::CAT_TIME) == 0 ||
				transTable->count(ContentAnchor::CAT_TIME) == 0) {

			pthread_mutex_unlock(&transMutex);
			return NULL;
		}

		transitionEvents = (*transTable)[ContentAnchor::CAT_TIME];
		currentIx        = (*currentTransitionIndex)[ContentAnchor::CAT_TIME];
		pthread_mutex_unlock(&transMutex);

		if (currentIx < transitionEvents->size()) {
			transition = transitionEvents->at(currentIx);
			if (IntervalAnchor::isObjectDuration(
					((PresentationEvent*)mainEvent)->getEnd()) ||
					(transition->getTime() <=
							((PresentationEvent*)mainEvent)->getEnd())) {

				return transition;
			}
		}

		return NULL;
	}
}
}
}
}
}
}
}
}
