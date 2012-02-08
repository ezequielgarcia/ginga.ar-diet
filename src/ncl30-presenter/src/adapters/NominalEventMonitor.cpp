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

#include "../../include/adapters/NominalEventMonitor.h"

#include "ncl/interfaces/SampleIntervalAnchor.h"
using namespace ::br::pucrio::telemidia::ncl::interfaces;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace adapters {
	NominalEventMonitor::NominalEventMonitor(
			ExecutionObject* object, IPlayerAdapter* player) : Thread() {

		this->running           = false;
		this->deleting          = false;
		this->adapter           = player;
		this->executionObject   = object;
		this->sleepTime         = DEFAULT_SLEEP_TIME;
		this->expectedSleepTime = 0;
		this->timeBaseId        = -1;
		this->timeBaseProvider  = NULL;

		cout << "[NominalEventMonitor] created." << endl;
	}


	NominalEventMonitor::~NominalEventMonitor() {
		deleting = true;
	//	running  = false;
	//	stopped  = true;
		stopMonitor();
		//wakeUp();
		//unlockConditionSatisfied();
/*
		if (timeBaseProvider != NULL) {
			timeBaseProvider->removeTimeListener(timeBaseId, this);
			timeBaseProvider->removeIdListener(this);
		}*/

		//lock();
		adapter          = NULL;
		executionObject  = NULL;
		timeBaseProvider = NULL;
		//unlock();
		cout << "[NominalEventMonitor] released." << endl;
	}

	void NominalEventMonitor::setTimeBaseProvider(
			ITimeBaseProvider* timeBaseProvider) {

		if (timeBaseProvider == NULL) {
			cout << "NominalEventMonitor::setTimeBaseProvider ";
			cout << "nothing to do (1)." << endl;
			return;
		}

		this->timeBaseId       = timeBaseProvider->getCurrentTimeBaseId();
		this->timeBaseProvider = timeBaseProvider;

		timeBaseProvider->addIdListener(this);
		timeBaseProvider->addLoopListener(timeBaseId, this);

		prepareNptTransitionsEvents();
	}

	void NominalEventMonitor::prepareNptTransitionsEvents() {
		AttributionEvent* event;
		set<double>* transValues;
		set<double>::iterator i;

		executionObject->prepareTransitionEvents(
				ContentAnchor::CAT_NPT,
				timeBaseProvider->getCurrentTimeValue(timeBaseId));

		transValues = executionObject->getTransitionsValues(
				ContentAnchor::CAT_NPT);

		if (transValues == NULL || transValues->empty()) {
			cout << "NominalEventMonitor::prepareNptTransitionsEvents ";
			cout << "nothing to do (2)." << endl;
			return;
		}

		executionObject->updateTransitionTable(
				timeBaseProvider->getCurrentTimeValue(timeBaseId),
				NULL,
				ContentAnchor::CAT_NPT);

		i = transValues->begin();
		while (i != transValues->end()) {
			if (!isInfinity(*i)) {
				/*cout << "NominalEventMonitor::prepareNptTransitionsEvents ";
				cout << "add listener contentId '" << timeBaseId;
				cout << "' for value '" << *i;
				cout << "' (current time = '";
				cout << timeBaseProvider->getCurrentTimeValue(timeBaseId);
				cout << "')" << endl;*/

				timeBaseProvider->addTimeListener(timeBaseId, *i, this);
			}
			++i;
		}

		delete transValues;

		event = (AttributionEvent*)(executionObject->getEventFromAnchorId(
				"contentId"));

		if (event != NULL) {
			adapter->setPropertyValue(event, itos(timeBaseId), NULL);
			event->stop();
		}
	}

	void NominalEventMonitor::updateTimeBaseId(
			unsigned char oldContentId,
			unsigned char newContentId) {

		AttributionEvent* event = (AttributionEvent*)(
				executionObject->getEventFromAnchorId("standby"));

		/*cout << "NominalEventMonitor::updateTimeBaseId event = '";
		cout << event << "' old = '" << (oldContentId & 0xFF);
		cout << "' new = '" << (newContentId & 0xFF);
		cout << endl;*/

		if (event != NULL) {
			if (oldContentId == timeBaseId) {
				event->start();
				adapter->setPropertyValue(event, "true", NULL);

			} else if (newContentId == timeBaseId) {
				event->start();
				adapter->setPropertyValue(event, "false", NULL);
			}
			event->stop();
		}
	}

	void NominalEventMonitor::loopDetected() {
		cout << "NominalEventMonitor::loopDetected" << endl;
		executionObject->resetTransitionEvents(ContentAnchor::CAT_NPT);
		prepareNptTransitionsEvents();
	}

	void NominalEventMonitor::valueReached(
			unsigned char timeBaseId, int64_t timeValue) {

		if (this->timeBaseId != timeBaseId) {
			cout << "NominalEventMonitor::valueReached ";
			cout << "Warning! receiving wrong timeBaseId '" << timeBaseId;
			cout << "' (my timeBaseId = '" << this->timeBaseId;
			cout << "')" << endl;
			return;
		}

		/*cout << "NominalEventMonitor::valueReached ";
		cout << "timeBaseId '" << timeBaseId;
		cout << "' value '" << timeValue;
		cout << "'" << endl;*/

		executionObject->updateTransitionTable(
				timeValue, adapter->getPlayer(), ContentAnchor::CAT_NPT);
	}

	void NominalEventMonitor::startMonitor() {
		if (!running && !deleting) {
    		// start monitor only if there is predictable events
			running = true;
    		paused  = false;
    		stopped = false;
    		Thread::start();

    	} else {
    		cout << "NominalEventMonitor::startMonitor(";
    		cout << this << ") Warning! Wrong way to ";
    		cout << "start monitor: running = '" << running;
    		cout << "' deleting = '" << deleting << "'" << endl;
		}
	}

	void NominalEventMonitor::pauseMonitor() {
		if (!isInfinity(expectedSleepTime)) {
			wakeUp();
			paused = true;
		}
	}

	void NominalEventMonitor::resumeMonitor() {
		if (!isInfinity(expectedSleepTime) && paused) {
			paused = false;
			unlockConditionSatisfied();
		}
	}

	void NominalEventMonitor::stopMonitor() {
		stopped = true;

		if (timeBaseProvider != NULL) {
			timeBaseProvider->removeTimeListener(timeBaseId, this);
			timeBaseProvider->removeIdListener(this);
		}

		if (running) {
			running = false;
			if (isInfinity(expectedSleepTime) || paused) {
				unlockConditionSatisfied();
			} else {
				wakeUp();
			}
		}
	}

	void NominalEventMonitor::run() {
		lock();
		EventTransition* nextTransition;
		double time;
		double mediaTime = 0;
		double nextEntryTime;
		nextTransition = NULL;

		/*cout << "====== Anchor Monitor Activated for '";
		cout << executionObject->getId().c_str() << "' (";
		cout << this << " ======" << endl;*/

		while (running) {
			if (deleting) {
				break;
			}

			if (paused) {
				waitForUnlockCondition();

			} else {
				if (executionObject != NULL) {
					nextTransition = executionObject->getNextTransition();

				} else {
					nextTransition = NULL;
					running = false;
				}

				if (nextTransition == NULL) {
					running = false;

				} else {
					nextEntryTime = nextTransition->getTime();
					if (isInfinity(nextEntryTime)) {
						expectedSleepTime = infinity();

					} else {
						mediaTime = (adapter->getPlayer()->getMediaTime()
								* 1000);

      					expectedSleepTime = nextEntryTime - mediaTime;
					}

					/*cout << "ANCHORMONITOR NEXTTRANSITIONTIME = '";
					cout << nextEntryTime << "' MEDIATIME = '" << mediaTime;
					cout << "' EXPECTEDSLEEPTIME = '" << expectedSleepTime;
					cout << "' (" << this << ")" << endl;*/

					if (running && !deleting) {
						if (isInfinity(expectedSleepTime)) {
							waitForUnlockCondition();

						} else {
							Thread::usleep((int)(expectedSleepTime));
						}

						if (running && !deleting) {
							if (executionObject == NULL || adapter == NULL) {
								unlock();
								return;
							}

							mediaTime = (adapter->getPlayer()->getMediaTime()
									* 1000);

							time = nextEntryTime - mediaTime;
							if (time < 0) {
								time = 0;
							}

							if (!paused && time <= DEFAULT_ERROR) {
								// efetua a transicao no estado do evento
								executionObject->updateTransitionTable(
									   mediaTime + DEFAULT_ERROR,
									   adapter->getPlayer(),
									   ContentAnchor::CAT_TIME);
							}
						}
					}
				}
			}
		}

		if (!stopped && adapter != NULL && !deleting) {
			adapter->getPlayer()->forceNaturalEnd();
		}
		unlock();
	}
}
}
}
}
}
}
