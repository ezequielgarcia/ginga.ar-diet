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

#include "../../../include/model/ExecutionObject.h"
#include "../../../include/model/CompositeExecutionObject.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace components {
	ExecutionObject::ExecutionObject(string id, Node* node, bool handling) {
		initializeExecutionObject(id, node, NULL, handling);
	}

	ExecutionObject::ExecutionObject(
		    string id,
		    Node* node,
		    GenericDescriptor* descriptor,
		    bool handling) {

		initializeExecutionObject(
			    id, node, new CascadingDescriptor(descriptor), handling);
	}

	ExecutionObject::ExecutionObject(
		    string id,
		    Node* node,
		    CascadingDescriptor* descriptor,
		    bool handling) {

		initializeExecutionObject(id, node, descriptor, handling);
	}

	ExecutionObject::~ExecutionObject() {
		map<string, FormatterEvent*>::iterator i;
		map<Node*, Node*>::iterator j;

		FormatterEvent* event;
		Node* parentNode;
		CompositeExecutionObject* parentObject;

		deleting = true;

		dataObject   = NULL;
		wholeContent = NULL;
		mainEvent    = NULL;

		lockEvents();
		if (transMan != NULL) {
			delete transMan;
			transMan = NULL;
		}

		if (events != NULL) {
			i = events->begin();
			while (i != events->end()) {
				event = i->second;
				if (event != NULL) {
					delete event;
					event = NULL;
				}
				++i;
			}

			events->clear();
			delete events;
			events = NULL;
		}
		unlockEvents();

		if (presEvents != NULL) {
			presEvents->clear();
			delete presEvents;
			presEvents = NULL;
		}

		if (selectionEvents != NULL) {
			selectionEvents->clear();
			delete selectionEvents;
			selectionEvents = NULL;
		}

		if (otherEvents != NULL) {
			otherEvents->clear();
			delete otherEvents;
			otherEvents = NULL;
		}

		lockParentTable();
		if (nodeParentTable != NULL) {
			j = nodeParentTable->begin();
			while (j != nodeParentTable->end()) {
				parentNode = j->second;
				if (parentTable->count(parentNode) != 0) {
					parentObject = (CompositeExecutionObject*)
						((*parentTable)[parentNode]);

					parentObject->removeExecutionObject(this);
				}
				++j;
			}

			nodeParentTable->clear();
			delete nodeParentTable;
			nodeParentTable = NULL;
		}

		if (parentTable != NULL) {
			parentTable->clear();
			delete parentTable;
			parentTable = NULL;
		}
		unlockParentTable();

		if (descriptor != NULL) {
			delete descriptor;
			descriptor = NULL;
		}
		unlock();
		pthread_mutex_destroy(&mutex);
		pthread_mutex_destroy(&mutexEvent);
		pthread_mutex_destroy(&mutexParentTable);
	}

	void ExecutionObject::initializeExecutionObject(
			    string id,
			    Node* node,
			    CascadingDescriptor* descriptor,
			    bool handling) {

		typeSet.insert("ExecutionObject");

		this->deleting        = false;
		this->id              = id;
		this->dataObject      = node;
		this->wholeContent    = NULL;
		this->startTime       = infinity();
		this->descriptor      = NULL;

		this->nodeParentTable = new map<Node*, Node*>;
		this->parentTable     = new map<Node*, void*>;
		this->isItCompiled    = false;

		this->events          = new map<string, FormatterEvent*>;
		this->presEvents      = new vector<PresentationEvent*>;
		this->selectionEvents = new set<SelectionEvent*>;
		this->otherEvents     = new vector<FormatterEvent*>;

		this->pauseCount      = 0;
		this->mainEvent       = NULL;
		this->descriptor      = descriptor;
		this->isLocked        = false;
		this->isHandler       = false;
		this->isHandling      = handling;

		this->transMan        = new EventTransitionManager();

		pthread_mutex_init(&mutex, NULL);
		pthread_mutex_init(&mutexEvent, NULL);
		pthread_mutex_init(&mutexParentTable, NULL);
	}

	bool ExecutionObject::instanceOf(string s) {
		return (typeSet.find(s) != typeSet.end());
	}

	int ExecutionObject::compareToUsingId(ExecutionObject* object) {
		return id.compare(object->getId());
	}

	Node* ExecutionObject::getDataObject() {
		return dataObject;
	}

	CascadingDescriptor* ExecutionObject::getDescriptor() {
		return descriptor;
	}

	string ExecutionObject::getId() {
		return id;
	}

	void* ExecutionObject::getParentObject() {
		return getParentObject(dataObject);
	}

	void* ExecutionObject::getParentObject(Node* node) {
		Node* parentNode;
		void* parentObj = NULL;

		lockParentTable();
		if (nodeParentTable->count(node) != 0) {
			parentNode = (*nodeParentTable)[node];
			if (parentTable->count(parentNode) != 0) {
				parentObj = (*parentTable)[parentNode];
			}
		}
		unlockParentTable();

		return parentObj;
	}

	void ExecutionObject::addParentObject(
		    void* parentObject, Node* parentNode) {

		addParentObject(dataObject, parentObject, parentNode);
	}

	void ExecutionObject::addParentObject(
		    Node* node,
		    void* parentObject,
		    Node* parentNode) {

		lockParentTable();
		(*nodeParentTable)[node]   = parentNode;
		(*parentTable)[parentNode] = parentObject;
		unlockParentTable();
	}

	void ExecutionObject::setDescriptor(
		    CascadingDescriptor* cascadingDescriptor) {

		this->descriptor = cascadingDescriptor;
	}

	void ExecutionObject::setDescriptor(GenericDescriptor* descriptor) {
		CascadingDescriptor* cascade;
		cascade = new CascadingDescriptor(descriptor);
		this->descriptor = cascade;
	}

	string ExecutionObject::toString() {
		return id;
	}

	bool ExecutionObject::addEvent(FormatterEvent* event) {
		lockEvents();
		if (events->count(event->getId()) != 0) {
			unlockEvents();
			return false;
		}

		(*events)[event->getId()] = event;
		unlockEvents();
		if (event->instanceOf("PresentationEvent")) {
			addPresentationEvent((PresentationEvent*)event);

		} else if (event->instanceOf("SelectionEvent")) {
			selectionEvents->insert(((SelectionEvent*)event));

		} else {
			otherEvents->push_back(event);
		}

		return true;
	}

	void ExecutionObject::addPresentationEvent(PresentationEvent* event) {
		PresentationEvent* auxEvent;
		double begin, auxBegin;
		int posBeg, posEnd, posMid;

		if ((event->getAnchor())->instanceOf("LambdaAnchor")) {
			presEvents->insert(presEvents->begin(), event);
			wholeContent = (PresentationEvent*)event;

		} else {
			begin = event->getBegin();

			// undefined events are not inserted into transition table
			if (PresentationEvent::isUndefinedInstant(begin)) {
				return;
			}

			if (event->getAnchor()->instanceOf("RelativeTimeIntervalAnchor")) {
				posBeg = 0;
				posEnd = presEvents->size() - 1;
				while (posBeg <= posEnd) {
					posMid = (posBeg + posEnd) / 2;
					auxEvent = (PresentationEvent*)((*presEvents)[posMid]);
					auxBegin = auxEvent->getBegin();
					if (begin < auxBegin) {
						posEnd = posMid - 1;

					} else if (begin > auxBegin) {
						posBeg = posMid + 1;

					} else {
						posBeg = posMid + 1;
						break;
					}
				}

				presEvents->insert(
						(presEvents->begin() + posBeg), event);
			}
		}

		transMan->addPresentationEvent(event);
	}

	int ExecutionObject::compareTo(ExecutionObject* object) {
		int ret;

		ret = compareToUsingStartTime(object);
		if (ret == 0)
			return compareToUsingId(object);
		else
			return ret;
	}

	int ExecutionObject::compareToUsingStartTime(ExecutionObject* object) {
		double thisTime, otherTime;

		thisTime  = startTime;
		otherTime = (object->getExpectedStartTime());

		if (thisTime < otherTime) {
			return -1;

		} else if (thisTime > otherTime) {
			return 1;
		}

		return 0;
	}

	bool ExecutionObject::containsEvent(FormatterEvent* event) {
		bool contains;

		lockEvents();
		contains = (events->count(event->getId()) != 0);
		unlockEvents();

		return contains;
	}

	FormatterEvent* ExecutionObject::getEventFromAnchorId(string anchorId) {
		map<string, FormatterEvent*>::iterator i;
		FormatterEvent* event;
		Anchor* anchor;
		string anchorName = " ";

		lockEvents();
		if (events != NULL) {
			i = events->begin();
			/*cout << "ExecutionObject::getEvent searching '";
			cout << anchorId << "' for '" << id;
			cout << "' with following events = '";*/
			while (i != events->end()) {
				event = i->second;
				if (event != NULL) {
					if (event->instanceOf("AnchorEvent")) {
						anchor = ((AnchorEvent*)event)->getAnchor();
						if (anchor != NULL) {
							if (anchor->instanceOf("IntervalAnchor")) {
								anchorName = anchor->getId();

							} else if (anchor->instanceOf("LabeledAnchor")) {
								anchorName = ((LabeledAnchor*)anchor)->getLabel();

							} else if (anchor->instanceOf("LamdaAnchor")) {
								anchorName = "";
							}

							//cout << anchorName << "' - '";
							if (anchorName == anchorId &&
									!event->instanceOf("SelectionEvent")) {

								unlockEvents();
								return event;
							}
						}

					} else if (event->instanceOf("AttributionEvent")) {
						anchor = ((AttributionEvent*)event)->getAnchor();
						if (anchor != NULL) {
							if (((PropertyAnchor*)anchor)->getPropertyName()
									== anchorId) {

								unlockEvents();
								return event;
							}
						}
					}
				}
				++i;
			}
		}
		//cout << endl;
		unlockEvents();
		return NULL;
	}

	FormatterEvent* ExecutionObject::getEvent(string id) {
		FormatterEvent* ev;
		lockEvents();
		if (events != NULL && events->count(id) != 0) {
			ev = (*events)[id];
			unlockEvents();
			return ev;
		}

		unlockEvents();
		return NULL;
	}

	vector<FormatterEvent*>* ExecutionObject::getEvents() {
		vector<FormatterEvent*>* eventsVector = NULL;
		map<string, FormatterEvent*>::iterator i;

		lockEvents();
		if (events == NULL || events->empty()) {
			unlockEvents();
			return NULL;
		}

		eventsVector = new vector<FormatterEvent*>;
		for (i = events->begin(); i != events->end(); ++i) {
			eventsVector->push_back(i->second);
		}
		unlockEvents();

		return eventsVector;
	}

	set<AnchorEvent*>* ExecutionObject::getSampleEvents() {
		set<AnchorEvent*>* eventsSet;
		map<string, FormatterEvent*>::iterator i;
		FormatterEvent* event;
		ContentAnchor* anchor;

		lockEvents();
		if (events == NULL || events->empty()) {
			unlockEvents();
			return NULL;
		}

		eventsSet = new set<AnchorEvent*>;
		i = events->begin();
		while (i != events->end()) {
			event = i->second;
			if (event->instanceOf("AnchorEvent")) {
				anchor = ((AnchorEvent*)event)->getAnchor();
				if (anchor->instanceOf("SampleIntervalAnchor")) {
					eventsSet->insert((AnchorEvent*)event);
				}
			}
			++i;
		}
		unlockEvents();

		return eventsSet;
	}

	double ExecutionObject::getExpectedStartTime() {
		return startTime;
	}

	PresentationEvent* ExecutionObject::getWholeContentPresentationEvent() {
		return wholeContent;
	}

	void ExecutionObject::setStartTime(double t) {
		startTime = t;
	}

	void ExecutionObject::updateEventDurations() {
		vector<PresentationEvent*>::iterator i;

		i = presEvents->begin();
		while (i != presEvents->end()) {
			updateEventDuration(*i);
			++i;
		}
	}

	void ExecutionObject::updateEventDuration(PresentationEvent* event) {
		double duration;

		if (!containsEvent((FormatterEvent*)event)) {
			return;
		}

		duration = NaN();

		if (descriptor != NULL) {
			if (descriptor->instanceOf("CascadingDescriptor")) {
				if (!isNaN(
					    descriptor->getExplicitDuration()) &&
					    event == wholeContent) {

					duration = descriptor->getExplicitDuration();

				} else if (event->getDuration() > 0) {
					duration = event->getDuration();

				} else {
					duration = 0;
				}
			}

		} else {
			if (event->getDuration() > 0) {
				duration = event->getDuration();

			} else {
				duration = 0;
			}
		}

		if (duration < 0) {
			event->setDuration(NaN());

		} else {
			event->setDuration(duration);
		}
	}

	bool ExecutionObject::removeEvent(FormatterEvent* event) {
		vector<PresentationEvent*>::iterator i;
		set<SelectionEvent*>::iterator j;
		vector<FormatterEvent*>::iterator k;

		if (!containsEvent(event)) {
			return false;
		}
		

		if (event->instanceOf("PresentationEvent")) {
			for (i = presEvents->begin(); i != presEvents->end(); ++i) {
				if (*i == (PresentationEvent*)event) {
					presEvents->erase(i);
					break;
				}
			}
			transMan->removeEventTransition((PresentationEvent*)event);

		} else if (event->instanceOf("SelectionEvent")) {
			j = selectionEvents->find(((SelectionEvent*)event));
			if (j != selectionEvents->end()) {
				selectionEvents->erase(j);
			}

		} else {
			for (k = otherEvents->begin();
				    k != otherEvents->end(); ++i) {

				if (*k == event) {
					otherEvents->erase(k);
					break;
				}
			}
		}

		lockEvents();
		if (events->count(event->getId()) != 0) {
			events->erase(events->find(event->getId()));
		}
		unlockEvents();
		
		return true;
	}

	bool ExecutionObject::isCompiled() {
		return isItCompiled;
	}

	void ExecutionObject::setCompiled(bool status) {
		isItCompiled = status;
	}

	void ExecutionObject::removeNode(Node* node) {
		Node* parentNode;

		if (node != dataObject) {
			lockParentTable();
			if (nodeParentTable->count(node) != 0) {
				parentNode = (*nodeParentTable)[node];

				if (parentTable->count(parentNode) != 0) {
					parentTable->erase(parentTable->find(parentNode));
				}

				if (nodeParentTable->count(node) != 0) {
					nodeParentTable->erase(nodeParentTable->find(node));
				}
			}
			unlockParentTable();
		}
	}

	vector<Node*>* ExecutionObject::getNodes() {
		vector<Node*>* nodes;
		map<Node*, Node*>::iterator i;

		lockParentTable();
		if (nodeParentTable->empty()) {
			unlockParentTable();
			return NULL;
		}

		nodes = new vector<Node*>;
		for (i = nodeParentTable->begin(); i != nodeParentTable->end(); ++i) {
			nodes->push_back(i->first);
		}

		if (nodeParentTable->count(dataObject) == 0) {
			nodes->push_back(dataObject);
		}
		unlockParentTable();

		return nodes;
	}

	NodeNesting* ExecutionObject::getNodePerspective() {
		return getNodePerspective(dataObject);
	}

	NodeNesting* ExecutionObject::getNodePerspective(Node* node) {
		Node* parentNode;
		NodeNesting* perspective;
		CompositeExecutionObject* parentObject;

		lockParentTable();
		if (nodeParentTable->count(node) == 0) {
			if (dataObject == node) {
				perspective = new NodeNesting();

			} else {
				unlockParentTable();
				return NULL;
			}

		} else {
			parentNode = (*nodeParentTable)[node];
			if (parentTable->count(parentNode) != 0) {

				parentObject = (CompositeExecutionObject*)(
					    (*parentTable)[parentNode]);

				perspective = parentObject->getNodePerspective(parentNode);

			} else {
				unlockParentTable();
				return NULL;
			}
		}
		perspective->insertAnchorNode(node);
		unlockParentTable();
		return perspective;
	}

	vector<ExecutionObject*>* ExecutionObject::getObjectPerspective() {
		return getObjectPerspective(dataObject);
	}

	vector<ExecutionObject*>* ExecutionObject::getObjectPerspective(
		    Node* node) {

		Node* parentNode;
		vector<ExecutionObject*>* perspective = NULL;
		CompositeExecutionObject* parentObject;

		lockParentTable();
		if (nodeParentTable->count(node) == 0) {
			if (dataObject == node) {
				perspective = new vector<ExecutionObject*>;

			} else {
				unlockParentTable();
				return NULL;
			}

		} else {
			parentNode = (*nodeParentTable)[node];
			if (parentTable->count(parentNode) != 0) {

				parentObject = (CompositeExecutionObject*)(
				 	    (*parentTable)[parentNode]);

				perspective = parentObject->getObjectPerspective(parentNode);

			} else {
				unlockParentTable();
				return NULL;
			}
		}
		unlockParentTable();

		perspective->push_back(this);
		return perspective;
	}

	vector<Node*>* ExecutionObject::getParentNodes() {
		vector<Node*>* parents;
		map<Node*, Node*>::iterator i;

		lockParentTable();
		if (nodeParentTable->empty()) {
			unlockParentTable();
			return NULL;
		}

		parents = new vector<Node*>;
		for (i = nodeParentTable->begin(); i != nodeParentTable->end(); ++i) {
			parents->push_back(i->second);
		}

		unlockParentTable();
		return parents;
	}

	FormatterEvent* ExecutionObject::getMainEvent() {
		return mainEvent;
	}

	bool ExecutionObject::prepare(FormatterEvent* event, double offsetTime) {
		CompositeExecutionObject* parentObject;
		int size;
		map<Node*, void*>::iterator i;
		double startTime = 0;
		ContentAnchor* contentAnchor;
		FormatterEvent* auxEvent;
		AttributionEvent* attributeEvent;
		PropertyAnchor* attributeAnchor;
		int j;
		string value;

		//cout << "ExecutionObject::prepare(" << id << ")" << endl;
		if (event == NULL || !containsEvent(event) ||
				event->getCurrentState() != EventUtil::ST_SLEEPING) {

			//cout << "ExecutionObject::prepare(" << id << ") ret FALSE" << endl;
			return false;
		}

		if (mainEvent != NULL &&
				mainEvent->getCurrentState() != EventUtil::ST_SLEEPING) {

			return false;
		}

		if (!lock()) {
			return false;
		}

		//cout << "ExecutionObject::prepare(" << id << ") locked" << endl;
		mainEvent = event;
		if (mainEvent->instanceOf("AnchorEvent")) {
			contentAnchor = ((AnchorEvent*)mainEvent)->getAnchor();
			if (contentAnchor != NULL &&
					contentAnchor->instanceOf("LabeledAnchor")) {

				lockParentTable();
				i = parentTable->begin();
				while (i != parentTable->end()) {
					parentObject = (CompositeExecutionObject*)(i->second);
					// register parent as a mainEvent listener
					mainEvent->addEventListener(parentObject);
					++i;
				}
				unlockParentTable();
				return true;
			}
		}

		if (mainEvent->instanceOf("PresentationEvent")) {
			startTime = ((PresentationEvent*)mainEvent)->
					getBegin() + offsetTime;

			if (startTime > ((PresentationEvent*)mainEvent)->getEnd()) {
				return false;
			}
		}

		lockParentTable();
		i = parentTable->begin();
		while (i != parentTable->end()) {
			parentObject = (CompositeExecutionObject*)(i->second);
			// register parent as a mainEvent listener
			mainEvent->addEventListener(parentObject);
			++i;
		}
		unlockParentTable();

		prepareTransitionEvents(ContentAnchor::CAT_TIME, startTime);

		if (otherEvents != NULL) {
			size = otherEvents->size();
			for (j = 0; j < size; j++) {
				auxEvent = (*otherEvents)[j];
				if (auxEvent->instanceOf("AttributionEvent")) {
					attributeEvent = (AttributionEvent*)auxEvent;
					attributeAnchor = attributeEvent->getAnchor();
					value = attributeAnchor->getPropertyValue();
					if (value != "") {
						attributeEvent->setValue(value);
					}
				}
			}
		}

		this->offsetTime = startTime;
		return true;
	}

	bool ExecutionObject::start() {
		ContentAnchor* contentAnchor;

		//cout << "ExecutionObject::start(" << id << ")" << endl;
		if (mainEvent == NULL && wholeContent == NULL) {
			return false;
		}

		if (mainEvent != NULL &&
				mainEvent->getCurrentState() != EventUtil::ST_SLEEPING) {

			return false;
		}

		if (mainEvent == NULL) {
			prepare(wholeContent, 0.0);
		}

		if (mainEvent->instanceOf("AnchorEvent")) {
			contentAnchor = ((AnchorEvent*)mainEvent)->getAnchor();
			if (contentAnchor != NULL &&
					contentAnchor->instanceOf("LabeledAnchor")) {

				mainEvent->start();
				return true;
			}
		}

		transMan->start(offsetTime);
		return true;
	}

	void ExecutionObject::updateTransitionTable(
			double value, IPlayer* player, short int transType) {

		transMan->updateTransitionTable(value, player, mainEvent, transType);
	}

	void ExecutionObject::resetTransitionEvents(short int transType) {
		transMan->resetTimeIndexByType(transType);
	}

	void ExecutionObject::prepareTransitionEvents(
			short int transType, double startTime) {

		transMan->prepare(mainEvent == wholeContent, startTime, transType);
	}

	set<double>* ExecutionObject::getTransitionsValues(short int transType) {
		return transMan->getTransitionsValues(transType);
	}

	EventTransition* ExecutionObject::getNextTransition() {
		if (mainEvent == NULL ||
				mainEvent->getCurrentState() == EventUtil::ST_SLEEPING ||
				!mainEvent->instanceOf("PresentationEvent")) {

			return NULL;
		}

		return transMan->getNextTransition(mainEvent);
	}

	bool ExecutionObject::stop() {
		ContentAnchor* contentAnchor;
		double endTime;

		//cout << "ExecutionObject::stop(" << id << ")" << endl;

		if (mainEvent == NULL) {
			/*cout << "ExecutionObject::stop for '" << id << "'";
			cout << " returns false because mainEvent is NULL" << endl;*/
			return false;
		}

		if (mainEvent->getCurrentState() == EventUtil::ST_SLEEPING) {
			/*cout << "ExecutionObject::stop for '" << id << "'";
			cout << " returns false because mainEvent is SLEEPING" << endl;*/
			return false;
		}

		if (mainEvent->instanceOf("PresentationEvent")) {
			endTime = ((PresentationEvent*)mainEvent)->getEnd();
			transMan->stop(endTime);

		} else if (mainEvent->instanceOf("AnchorEvent")) {
			contentAnchor = ((AnchorEvent*)mainEvent)->getAnchor();
			if (contentAnchor != NULL &&
					contentAnchor->instanceOf("LabeledAnchor")) {

				/*cout << "ExecutionObject::stop for '" << id << "'";
				cout << " call mainEvent '" << mainEvent->getId();
				cout << "' stop" << endl;*/
				mainEvent->stop();
			}
		}

		transMan->resetTimeIndex();
		pauseCount = 0;
		return true;
	}

	bool ExecutionObject::abort() {
		ContentAnchor* contentAnchor;
		short objectState;
		double endTime;

		//cout << "ExecutionObject::abort(" << id << ")" << endl;
		if (mainEvent == NULL) {
			return false;
		}

		objectState = mainEvent->getCurrentState();

		if (objectState == EventUtil::ST_SLEEPING) {
			return false;
		}

		if (mainEvent->instanceOf("PresentationEvent")) {
			endTime = ((PresentationEvent*)mainEvent)->getEnd();
			transMan->abort(endTime);

		} else if (mainEvent->instanceOf("AnchorEvent")) {
			contentAnchor = ((AnchorEvent*)mainEvent)->getAnchor();
			if (contentAnchor != NULL &&
					contentAnchor->instanceOf("LabeledAnchor")) {

				mainEvent->abort();
			}
		}

		transMan->resetTimeIndex();
		pauseCount = 0;
		return true;
	}

	bool ExecutionObject::pause() {
		FormatterEvent* event;
		vector<FormatterEvent*>* evs;
		vector<FormatterEvent*>::iterator i;

		//cout << "ExecutionObject::pause(" << id << ")" << endl;
		if (mainEvent == NULL ||
				mainEvent->getCurrentState() == EventUtil::ST_SLEEPING) {

			return false;
		}

		evs = getEvents();
		if (evs != NULL) {
			if (pauseCount == 0) {
				i = evs->begin();
				while (i != evs->end()) {
					event = *i;
					if (event->getCurrentState() == EventUtil::ST_OCCURRING) {
						event->pause();
					}
					++i;
				}
			}
			delete evs;
			evs = NULL;
		}

		pauseCount++;
		return true;
	}

	bool ExecutionObject::resume() {
		FormatterEvent* event;
		vector<FormatterEvent*>* evs;
		vector<FormatterEvent*>::iterator i;

		//cout << "ExecutionObject::resume(" << id << ")" << endl;
		if (pauseCount == 0) {
			return false;

		} else {
			pauseCount--;
			if (pauseCount > 0) {
				return false;
			}
		}

		evs = getEvents();
		if (evs != NULL) {
			if (pauseCount == 0) {
				i = evs->begin();
				while (i != evs->end()) {
					event = *i;
					if (event->getCurrentState() == EventUtil::ST_PAUSED) {
						event->resume();
					}
					++i;
				}
			}
			delete evs;
			evs = NULL;
		}

		return true;
	}

	bool ExecutionObject::setPropertyValue(
			string propName, vector<string>* params) {

		FormatterRegion* region = NULL;
		LayoutRegion* ncmRegion = NULL;
		bool error = false;

		if (descriptor == NULL || descriptor->getFormatterRegion() == NULL) {
			cout << "ExecutionObject::setPropertyValue : setPropertyy: "<<propName << " could ";
			cout << "not be performed. Descriptor or formatterRegion is NULL";
			cout << endl;
			return false;
		}

		region = descriptor->getFormatterRegion();
		ncmRegion = region->getLayoutRegion();

		lock(); //lock the object

		if (ncmRegion == NULL) {
			cout << "ExecutionObject::setPropertyValue : The ncmRegion ";
			cout << " is NULL (PROBLEM)!" << endl;
			error = true;

		} else if (propName == "size") {
			if (params->size() == 2) {
				if (isPercentualValue((*params)[0])) {
					ncmRegion->setTargetWidth(getPercentualValue(
							(*params)[0]), true);
				} else {
					ncmRegion->setTargetWidth(
							(double)(stof((*params)[0])), false);
				}

				if (isPercentualValue((*params)[1])) {
					ncmRegion->setTargetHeight(
							getPercentualValue((*params)[1]), true);

				} else {
					ncmRegion->setTargetHeight(
							(double)(stof((*params)[1])), false);
				}

			} else {
				error = true;
			}

		} else if (propName == "location") {
			if (params->size() == 2) {
				if (isPercentualValue((*params)[0])) {
					ncmRegion->setTargetLeft(
							getPercentualValue((*params)[0]), true);
				} else {
					ncmRegion->setTargetLeft(
							(double)(stof((*params)[0])), false);
				}

				if (isPercentualValue((*params)[1])) {
					ncmRegion->setTargetTop(
							getPercentualValue((*params)[1]), true);
				} else {
					ncmRegion->setTargetTop(
							(double)(stof((*params)[1])), false);
				}

			} else {
				error = true;
			}

		} else if (propName == "bounds") {
			if (params->size() == 4) {
				if (isPercentualValue((*params)[0])) {
					ncmRegion->setTargetLeft(getPercentualValue(
							(*params)[0]), true);

				} else {
					ncmRegion->setTargetLeft(
							(double)(stof((*params)[0])), false);
				}

				if (isPercentualValue((*params)[1])) {
					ncmRegion->setTargetTop(getPercentualValue(
							(*params)[1]), true);
				} else {
					ncmRegion->setTargetTop(
							(double)(stof((*params)[1])), false);
				}

				if (isPercentualValue((*params)[2])) {
					ncmRegion->setTargetWidth(getPercentualValue(
							(*params)[2]), true);
				} else {
					ncmRegion->setTargetWidth(
							(double)(stof((*params)[2])), false);
				}

				if (isPercentualValue((*params)[3])) {
					ncmRegion->setTargetHeight(getPercentualValue(
							(*params)[3]), true);

				} else {
					ncmRegion->setTargetHeight(
							(double)(stof((*params)[3])), false);
				}

			} else {
				error = true;
			}

		} else if (propName == "top" || propName == "left" ||
				propName == "bottom" || propName == "right" ||
				propName == "width" || propName == "height") {

			if (propName == "left") {
				if (isPercentualValue((*params)[0])) {
					ncmRegion->setTargetLeft(
							getPercentualValue((*params)[0]), true);

				} else {
					ncmRegion->setTargetLeft(
							(double)(stof((*params)[0])), false);
				}

			} else if (propName == "top") {
				if (isPercentualValue((*params)[0])) {
					ncmRegion->setTargetTop(
							getPercentualValue((*params)[0]), true);

				} else {
					ncmRegion->setTargetTop(
							(double)(stof((*params)[0])), false);
				}

			} else if (propName == "width") {
				if (isPercentualValue((*params)[0])) {
					ncmRegion->setTargetWidth(
							getPercentualValue((*params)[0]), true);

				} else {
					ncmRegion->setTargetWidth(
							(double)(stof((*params)[0])), false);
				}

			} else if (propName == "height") {
				if (isPercentualValue((*params)[0])) {
					ncmRegion->setTargetHeight(
							getPercentualValue((*params)[0]), true);

				} else {
					ncmRegion->setTargetHeight(
							(double)(stof((*params)[0])), false);
				}

			} else if (propName == "bottom") {
				if (isPercentualValue((*params)[0])) {
					ncmRegion->setTargetBottom(
							getPercentualValue((*params)[0]), true);

				} else {
					ncmRegion->setTargetBottom(
							(double)(stof((*params)[0])), false);
				}

			} else if (propName == "right") {
				if (isPercentualValue((*params)[0])) {
					ncmRegion->setTargetRight(
							getPercentualValue((*params)[0]), true);

				} else {
					ncmRegion->setTargetRight(
							(double)(stof((*params)[0])), false);
				}
			}

		} else {
			/*
			cout << "ExecutionObject::setPropertyValue";
			cout << "(propName,vector<string>*)";
			cout << " [WARNING] There isn't any property in object with name ";
			cout << propName;
			cout << endl;
			*/
			error = true;
		}

		if (!error) {
			//set the values
			ncmRegion->validateTarget();
			region->updateRegionBounds();
			unlock();
			return true;
		}
		unlock();
		return false;
	}

	bool ExecutionObject::setPropertyValue(
			string propName, vector<double>* params) {

		FormatterRegion* region = NULL;
		LayoutRegion* ncmRegion = NULL;

		bool error = false;

		if (descriptor == NULL || descriptor->getFormatterRegion() == NULL) {
			cout << "ExecutionObject::setPropertyValue : setProperty could ";
			cout << "not be performed. Descriptor or formatterRegion is NULL";
			cout << endl;
			return false;
		}

		lock(); // lock the object
		region = descriptor->getFormatterRegion();
		ncmRegion = region->getLayoutRegion();

		if (ncmRegion == NULL) {
			cout << "ExecutionObject::setPropertyValue : The ncmRegion ";
			cout << " is NULL (PROBLEM)!" << endl;
			error = true;

		} else if (propName == "size") {
			if (params->size() == 2) {
				ncmRegion->setTargetWidth((*params)[0], false);
				ncmRegion->setTargetHeight((*params)[1], false);

			} else {
				error = true;
			}

		} else if (propName == "location") {
			if (params->size() == 2) {
				ncmRegion->setTargetLeft((*params)[0], false);
				ncmRegion->setTargetTop((*params)[1], false);

			} else {
				error = true;
			}

		} else if (propName == "bounds") {
			if (params->size() == 4) {
				ncmRegion->setTargetLeft((*params)[0], false);
				ncmRegion->setTargetTop((*params)[1], false);
				ncmRegion->setTargetWidth((*params)[2], false);
				ncmRegion->setTargetHeight((*params)[3], false);

			} else {
				error = true;
			}

		} else if (propName == "left") {
			ncmRegion->setTargetLeft((*params)[0], false);

		} else if (propName == "top") {
			ncmRegion->setTargetTop((*params)[0], false);

		} else if (propName == "width") {
			ncmRegion->setTargetWidth((*params)[0], false);

		} else if (propName == "height") {
			ncmRegion->setTargetHeight((*params)[0], false);

		} else if (propName == "bottom") {
			ncmRegion->setTargetBottom((*params)[0], false);

		} else if (propName == "right") {
			ncmRegion->setTargetRight((*params)[0], false);

		} else if (propName == "transparency") {
			region->setTransparency((*params)[0]);

		} else {
			/*
			cout << "ExecutionObject::setPropertyValue(";
			cout << "propName,vector<string>*)";
			cout << " [WARNING] There is no property in ExecutionObject with ";
			cout << "name " << propName;
			cout << endl;
			 */
			error = true;
		}

		if (!error) {
			//set the values
			ncmRegion->validateTarget();
			region->updateRegionBounds();
			unlock();
			return true;
		}

		unlock();
		return false;
	}


	bool ExecutionObject::setPropertyValue(
		    AttributionEvent* event,
		    string value, Animation* anim) {

		vector<string>* params;
		string propName;

		FormatterRegion* region = NULL;
		LayoutRegion* ncmRegion = NULL;

		if (descriptor == NULL || descriptor->getFormatterRegion() == NULL) {
			cout << "ExecutionObject::setPropertyValue : setProperty could ";
			cout << "not be performed. Descriptor or formatterRegion is NULL";
			cout << endl;
			return false;
		}

		region = descriptor->getFormatterRegion();
		ncmRegion = region->getLayoutRegion();

		if (!containsEvent(event) || value == "") {
			cout << "ExecutionObject::setPropertyValue event '";
			cout << event->getId() << "' not found!" << endl;
			return false;
		}

		propName = (event->getAnchor())->getPropertyName();

		/*cout << "ExecutionObject::setPropertyValue prop '" << propName;
		cout << "' value '" << value << "' for '" << getId() << "'"<<endl;*/

		if (anim != NULL) {
			cout << " animDur = '" << anim->getDuration() << "'";
			cout << " animBy = '" << anim->getBy() << "'";
			cout << " anim = '" << anim << "'" << endl;
		}

		//parsing the parameters passed by the user
		value = trim(value);
		params = split(value, ",");

		if (anim == NULL || stof(anim->getDuration()) <= 0.0) {

			if(setPropertyValue(propName, params)){

				//Now the event->stop() must be call for who call this method
				//event->stop();

				//not in use anymore, so can be deleted
				delete params;
				params = NULL;

				return true;

			} else {
				//not in use anymore
				delete params;
				params = NULL;

				return false;
			}

		}

		//not in use anymore
		delete params;
		params = NULL;

		return false;
	}

	//TODO: The parameter value shall be a string.
	string ExecutionObject::getPropertyValueAsString(string param){
		return itos(getPropertyValueAsDouble(param));
	}

	//TODO: The parameter value shall be a string.
	double ExecutionObject::getPropertyValueAsDouble(string param){

		FormatterRegion* region = NULL;
		LayoutRegion* ncmRegion = NULL;

		if (descriptor == NULL || descriptor->getFormatterRegion() == NULL) {
			cout << "ExecutionObject::setPropertyValue : setProperty could ";
			cout << "not be performed. Descriptor or formatterRegion is NULL";
			cout << endl;
			return false;
		}

		region = descriptor->getFormatterRegion();
		ncmRegion = region->getLayoutRegion();

		double current = 0.0;
		if (param == "width") {
			current = ncmRegion->getWidthInPixels();

		} else if (param == "height") {
			current = ncmRegion->getHeightInPixels();

		} else if (param == "left") {
			current = ncmRegion->getLeftInPixels();

		} else if (param == "top") {
			current = ncmRegion->getTopInPixels();

		} else if (param == "bottom") {
			current = ncmRegion->getBottomInPixels();

		} else if (param == "right") {
			current = ncmRegion->getRightInPixels();

		} else if(param == "transparency"){
			current = region->getTransparency();
		}
		return current;
	}

	bool ExecutionObject::unprepare() {
		if (mainEvent == NULL ||
				mainEvent->getCurrentState() != EventUtil::ST_SLEEPING) {

			unlock();
			/*cout << "ExecutionObject::unprepare(" << id << ") unlocked";
			cout << " ret FALSE" << endl;*/
			return false;
		}

		map<Node*, void*>::iterator i;
		CompositeExecutionObject* parentObject;

		lockParentTable();
		i = parentTable->begin();
		while (i != parentTable->end()) {
			parentObject = (CompositeExecutionObject*)(i->second);
			// register parent as a mainEvent listener
			mainEvent->removeEventListener(parentObject);
			++i;
		}
		unlockParentTable();

		mainEvent = NULL;
		unlock();
		/*cout << "ExecutionObject::unprepare(" << id << ") unlocked";
		cout << endl;*/
		return true;
	}


	void ExecutionObject::setHandling(bool isHandling) {
		this->isHandling = isHandling;
	}

	void ExecutionObject::setHandler(bool isHandler) {
		this->isHandler = isHandler;
	}

	void ExecutionObject::selectionEvent(int accessCode, double currentTime) {
		int selCode;
		SelectionEvent* selectionEvent;
		IntervalAnchor* intervalAnchor;
		FormatterEvent* expectedEvent;
		Anchor* expectedAnchor;
		string anchorId = "";
		set<SelectionEvent*>* selectedEvents;
		set<SelectionEvent*>::iterator i;

		if ((!isHandling && !isHandler) || mainEvent == NULL ||
				mainEvent->getCurrentState() != EventUtil::ST_OCCURRING) {

			return;
		}

		//simulating some keys of keyboard
		if (accessCode == CodeMap::KEY_F1) {
			accessCode = CodeMap::KEY_RED;

		} else if (accessCode == CodeMap::KEY_F2) {
			accessCode = CodeMap::KEY_GREEN;

		} else if (accessCode == CodeMap::KEY_F3) {
			accessCode = CodeMap::KEY_YELLOW;

		} else if (accessCode == CodeMap::KEY_F4) {
			accessCode = CodeMap::KEY_BLUE;

		} else if (accessCode == CodeMap::KEY_F5) {
			accessCode = CodeMap::KEY_MENU;

		} else if (accessCode == CodeMap::KEY_F6) {
			accessCode = CodeMap::KEY_INFO;

		} else if (accessCode == CodeMap::KEY_F7) {
			accessCode = CodeMap::KEY_EPG;

		} else if (accessCode == CodeMap::KEY_PLUS_SIGN) {
			accessCode = CodeMap::KEY_VOLUME_UP;

		} else if (accessCode == CodeMap::KEY_MINUS_SIGN) {
			accessCode = CodeMap::KEY_VOLUME_DOWN;

		} else if (accessCode == CodeMap::KEY_PAGE_UP) {
			accessCode = CodeMap::KEY_CHANNEL_UP;

		} else if (accessCode == CodeMap::KEY_PAGE_DOWN) {
			accessCode = CodeMap::KEY_CHANNEL_DOWN;

		} else if (accessCode == CodeMap::KEY_BACKSPACE) {
			accessCode = CodeMap::KEY_BACK;

		} else if (accessCode == CodeMap::KEY_ESCAPE) {
			accessCode = CodeMap::KEY_EXIT;
		}

		selectedEvents = new set<SelectionEvent*>;
		i = selectionEvents->begin();
		while (i != selectionEvents->end()) {
			selectionEvent = (SelectionEvent*)(*i);
			selCode = selectionEvent->getSelectionCode();
			if (selCode == accessCode) {
				if (selectionEvent->getAnchor()->instanceOf(
					    "LambdaAnchor")) {

					selectedEvents->insert(selectionEvent);

				} else if (selectionEvent->getAnchor()->instanceOf(
					    "IntervalAnchor")) {

					intervalAnchor = (IntervalAnchor*)(
						    selectionEvent->getAnchor());

					if (intervalAnchor->getBegin() <= currentTime
						    && currentTime <= intervalAnchor->getEnd()) {

						selectedEvents->insert(selectionEvent);
					}

				} else {
					expectedAnchor = selectionEvent->getAnchor();
					if (expectedAnchor->instanceOf("LabeledAnchor")) {
						anchorId = ((LabeledAnchor*)expectedAnchor)->getLabel();

					} else {
						anchorId = expectedAnchor->getId();
					}

					expectedEvent = getEventFromAnchorId(anchorId);
					if (expectedEvent != NULL) {
						/*cout << "ExecutionObject::select(" << id << ")";
						cout << " analyzing event '";
						cout << expectedEvent->getId();*/

						if (expectedEvent->getCurrentState() ==
								EventUtil::ST_OCCURRING) {

							selectedEvents->insert(selectionEvent);

						}/* else {
							cout << "' not occurring";
						}

						cout << "'" << endl;*/

					} else {
						cout << "ExecutionObject::select(" << id << ")";
						cout << " can't find event for anchorid = '";
						cout << anchorId << "'" << endl;
					}
				}
			}
			++i;
		}

		i = selectedEvents->begin();
		while (i != selectedEvents->end()) {
			selectionEvent = (*i);
			/*
			cout << "ExecutionObject::select(" << id << ") starting event '";
			cout << selectionEvent->getId() << "'" << endl;*/
			if (!selectionEvent->start()) {
				cout << "ExecutionObject::select Warning cant start '";
				cout << selectionEvent->getId() << "'" << endl;
			}
			++i;
		}

		delete selectedEvents;
		selectedEvents = NULL;
	}

	set<int>* ExecutionObject::getInputEvents() {
		set<SelectionEvent*>::iterator i;
		set<int>* evs;
		SelectionEvent* ev;
		int keyCode;

		evs = new set<int>;
		i = selectionEvents->begin();
		while (i != selectionEvents->end()) {
			ev = (*i);
			keyCode = ev->getSelectionCode();
			evs->insert(keyCode);
			if (keyCode == CodeMap::KEY_RED) {
				evs->insert(CodeMap::KEY_F1);

			} else if (keyCode == CodeMap::KEY_GREEN) {
				evs->insert(CodeMap::KEY_F2);

			} else if (keyCode == CodeMap::KEY_YELLOW) {
				evs->insert(CodeMap::KEY_F3);

			} else if (keyCode == CodeMap::KEY_BLUE) {
				evs->insert(CodeMap::KEY_F4);

			} else if (keyCode == CodeMap::KEY_MENU) {
				evs->insert(CodeMap::KEY_F5);

			} else if (keyCode == CodeMap::KEY_INFO) {
				evs->insert(CodeMap::KEY_F6);

			} else if (keyCode == CodeMap::KEY_EPG) {
				evs->insert(CodeMap::KEY_F7);

			} else if (keyCode == CodeMap::KEY_VOLUME_UP) {
				evs->insert(CodeMap::KEY_PLUS_SIGN);

			} else if (keyCode == CodeMap::KEY_VOLUME_DOWN) {
				evs->insert(CodeMap::KEY_MINUS_SIGN);

			} else if (keyCode == CodeMap::KEY_CHANNEL_UP) {
				evs->insert(CodeMap::KEY_PAGE_UP);

			} else if (keyCode == CodeMap::KEY_CHANNEL_DOWN) {
				evs->insert(CodeMap::KEY_PAGE_DOWN);

			} else if (keyCode == CodeMap::KEY_BACK) {
				evs->insert(CodeMap::KEY_BACKSPACE);

			} else if (keyCode == CodeMap::KEY_EXIT) {
				evs->insert(CodeMap::KEY_ESCAPE);
			}
			++i;
		}

		return evs;
	}

	bool ExecutionObject::lock() {
		if (isLocked) {
			return false;
		}
		isLocked = true;
		pthread_mutex_lock(&mutex);
		return true;
	}

	bool ExecutionObject::unlock() {
		if (!isLocked) {
			return false;
		}

		pthread_mutex_unlock(&mutex);
		isLocked = false;
		return true;
	}

	void ExecutionObject::lockEvents() {
		pthread_mutex_lock(&mutexEvent);
	}

	void ExecutionObject::unlockEvents() {
		pthread_mutex_unlock(&mutexEvent);
	}

	void ExecutionObject::lockParentTable() {
		pthread_mutex_lock(&mutexParentTable);
	}

	void ExecutionObject::unlockParentTable() {
		pthread_mutex_unlock(&mutexParentTable);
	}
}
}
}
}
}
}
}
