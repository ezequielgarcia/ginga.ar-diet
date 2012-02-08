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

#include "../../../include/model/CompositeExecutionObject.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace components {
	CompositeExecutionObject::CompositeExecutionObject(
		    string id,
		    Node* dataObject,
		    bool handling) : ExecutionObject(id, dataObject, handling) {

		initializeCompositeExecutionObject(id, dataObject, NULL);
	}

	CompositeExecutionObject::CompositeExecutionObject(
		    string id,
		    Node* dataObject,
		    CascadingDescriptor* descriptor,
		    bool handling) : ExecutionObject(
		    id, dataObject, descriptor, handling) {

		initializeCompositeExecutionObject(id, dataObject, descriptor);
	}

	CompositeExecutionObject::~CompositeExecutionObject() {
		ExecutionObject* object;
		FormatterLink* link;
		set<FormatterLink*>::iterator i;
		map<string, ExecutionObject*>::iterator j;

		deleting = true;

		lockComposite();

		if (runningEvents != NULL) {
			runningEvents->clear();
			delete runningEvents;
			runningEvents = NULL;
		}

		if (pausedEvents != NULL) {
			pausedEvents->clear();
			delete pausedEvents;
			pausedEvents = NULL;
		}

		if (pendingLinks != NULL) {
			pendingLinks->clear();
			delete pendingLinks;
			pendingLinks = NULL;
		}

		if (links != NULL) {
			i = links->begin();
			while (i != links->end()) {
				link = *i;
				if (link != NULL) {
					delete link;
					link = NULL;
				}
				++i;
			}
			links->clear();
			delete links;
			links = NULL;
		}

		if (uncompiledLinks != NULL) {
			uncompiledLinks->clear();
			delete uncompiledLinks;
			uncompiledLinks = NULL;
		}

		if (execObjList != NULL) {
			j = execObjList->begin();
			while (j != execObjList->end()) {
				object = j->second;
				if (object != this && object != NULL) {
					delete object;
					object = NULL;
				}
				++j;
			}

			execObjList->clear();
			delete execObjList;
			execObjList = NULL;
		}
		
		typeSet.clear();

		unlockComposite();
		pthread_mutex_destroy(&mutexComposite);
	}

	void CompositeExecutionObject::stopListening(void* event) {
		set<FormatterEvent*>::iterator i;

		if (runningEvents != NULL) {
			i = runningEvents->find((FormatterEvent*)event);
			if (i != runningEvents->end()) {
				runningEvents->erase(i);
			}
		}

		if (pausedEvents != NULL) {
			i = pausedEvents->find((FormatterEvent*)event);
			if (i != pausedEvents->end()) {
				pausedEvents->erase(i);
			}
		}
	}

	void CompositeExecutionObject::initializeCompositeExecutionObject(
		    string id,
		    Node* dataObject,
		    CascadingDescriptor* descriptor) {

		typeSet.insert("CompositeExecutionObject");
		pthread_mutex_init(&mutexComposite, NULL);

		vector<Link*>* compositionLinks = NULL;

		execObjList     = new map<string, ExecutionObject*>;
		links           = new set<FormatterLink*>;
		uncompiledLinks = new set<Link*>;

		Entity* entity;
		entity = dataObject->getDataEntity();

		if (entity != NULL && entity->instanceOf("LinkComposition")) {
			if (entity->instanceOf("ContextNode")) {
				ContextNode* compositeNode;

				compositeNode = (ContextNode*)entity;
				compositionLinks = compositeNode->getLinks();
				if (compositionLinks != NULL) {
					vector<Link*>::iterator i;
					i = compositionLinks->begin();
					while (i != compositionLinks->end()) {
						uncompiledLinks->insert(*i);
						++i;
					}
				}
			}
		}

		runningEvents = new set<FormatterEvent*>;
		pausedEvents = new set<FormatterEvent*>;
		pendingLinks = new map<FormatterLink*, int>;
	}

	bool CompositeExecutionObject::addExecutionObject(ExecutionObject* obj) {
		string objId;

		if (obj == NULL) {
			return false;
		}

		lockComposite();
		objId = obj->getId();
		if (execObjList->count(objId) != 0) {
			cout << "CompositeExecutionObject::addExecutionObject Warning! ";
			cout << "trying to add same obj twice: '" << objId << "'" << endl;
			unlockComposite();
			return false;
		}

		(*execObjList)[objId] = obj;
		unlockComposite();
		obj->addParentObject((void*)this, getDataObject());
		return true;
	}

	bool CompositeExecutionObject::containsExecutionObject(string execObjId) {
		if (getExecutionObject(execObjId) != NULL)
			return true;
		else
			return false;
	}

	ExecutionObject* CompositeExecutionObject::getExecutionObject(string id) {
		lockComposite();
		if (execObjList->empty()) {
			unlockComposite();
			return NULL;
		}

		map<string, ExecutionObject*>::iterator i;
		ExecutionObject* execObj;

		i = execObjList->find(id);
		if (i != execObjList->end()) {
			execObj = i->second;
			unlockComposite();
			return execObj;
		}

		unlockComposite();
		return NULL;
	}

	map<string, ExecutionObject*>*
			CompositeExecutionObject::getExecutionObjects() {

		map<string, ExecutionObject*>* objs;
		lockComposite();
		if (execObjList == NULL || execObjList->empty()) {
			unlockComposite();
			return NULL;
		}
		objs = new map<string, ExecutionObject*>(*execObjList);
		unlockComposite();
		return objs;
	}

	int CompositeExecutionObject::getNumExecutionObjects() {
		int size = 0;
		lockComposite();
		if (execObjList != NULL) {
			size = execObjList->size();
		}
		unlockComposite();
		return size;
	}

	map<string, ExecutionObject*>*
		    CompositeExecutionObject::recursivellyGetExecutionObjects() {

		map<string, ExecutionObject*>* objects;
		ExecutionObject* childObject;
		map<string, ExecutionObject*>* grandChildrenObjects;
		map<string, ExecutionObject*>::iterator i, j;

		objects = new map<string, ExecutionObject*>;
		lockComposite();
		i = execObjList->begin();
		while (i != execObjList->end()) {
			childObject = i->second;
			(*objects)[i->first] = childObject;
			if (childObject->instanceOf("CompositeExecutionObject")) {
				grandChildrenObjects = ((CompositeExecutionObject*)
					    childObject)->recursivellyGetExecutionObjects();

				j = grandChildrenObjects->begin();
				while (j != grandChildrenObjects->end()) {
					(*objects)[j->first] = j->second;
					++j;
				}
				delete grandChildrenObjects;
				grandChildrenObjects = NULL;
			}
			++i;
		}
		unlockComposite();
		return objects;
	}

	bool CompositeExecutionObject::removeExecutionObject(ExecutionObject* obj) {
		map<string, ExecutionObject*>::iterator i;

		lockComposite();
		if (execObjList) {
			i = execObjList->find(obj->getId());
			if (i != execObjList->end()) {
				execObjList->erase(i);
				unlockComposite();
				return true;
			}
		}
		unlockComposite();
		return false;
	}

	set<Link*>* CompositeExecutionObject::getUncompiledLinks() {
		return uncompiledLinks;
	}

	bool CompositeExecutionObject::containsUncompiledLink(Link* dataLink) {
		lockComposite();
		if (uncompiledLinks->count(dataLink) != 0) {
			unlockComposite();
			return true;
		}

		unlockComposite();
		return false;
	}

	void CompositeExecutionObject::removeLinkUncompiled(Link* ncmLink) {
		set<Link*>::iterator i;

		lockComposite();
		i = uncompiledLinks->find(ncmLink);
		if (i != uncompiledLinks->end()) {
			uncompiledLinks->erase(i);
			unlockComposite();
			return;
		}
		unlockComposite();
	}

	void CompositeExecutionObject::setLinkCompiled(FormatterLink* link) {
		if (link == NULL) {
			cout << "CompositeExecutionObject::setLinkCompiled Warning! ";
			cout << "trying to compile a NULL link" << endl;
			return;
		}

		Link* compiledLink;
		compiledLink = link->getNcmLink();

		if (compiledLink == NULL) {
			cout << "CompositeExecutionObject::setLinkCompiled Warning! ";
			cout << "formatterLink has returned a NULL ncmLink" << endl;
			return;
		}

		lockComposite();
		if (links != NULL) {
			links->insert(link);
		}
		unlockComposite();
	}

	void CompositeExecutionObject::setLinkUncompiled(FormatterLink* link) {
		Link* uncompiledLink;
		set<FormatterLink*>::iterator i;

		lockComposite();
		uncompiledLink = link->getNcmLink();
		uncompiledLinks->insert(uncompiledLink);

		i = links->find(link);
		if (i != links->end()) {
			links->erase(i);
		}

		delete link;
		link = NULL;

		unlockComposite();
	}

	void CompositeExecutionObject::addNcmLink(Link* ncmLink) {
		lockComposite();
		if (uncompiledLinks->count(ncmLink) != 0) {
			cout << "CompositeExecutionObject::addNcmLink Warning! ";
			cout << "Trying to add same link twice" << endl;
		}
		uncompiledLinks->insert(ncmLink);
		unlockComposite();
	}

	void CompositeExecutionObject::removeNcmLink(Link* ncmLink) {
		bool contains;
  		FormatterLink* link;
  		Link* compiledNcmLink;
	  	set<Link*>::iterator i;
  		set<FormatterLink*>::iterator j;

  		contains = false;

	  	lockComposite();
		i = uncompiledLinks->find(ncmLink);
		if (i != uncompiledLinks->end()) {
			contains = true;
			uncompiledLinks->erase(i);
		}

	  	if (!contains) {
			for (j = links->begin(); j != links->end(); ++j) {
	  			link = *j;
	  			compiledNcmLink = link->getNcmLink();
	  			if (ncmLink == compiledNcmLink) {
	  				links->erase(j);
	  				delete link;
	  				link = NULL;
	  				unlockComposite();
	  				return;
	  			}
	  		}
	  	}
	  	unlockComposite();
	}

	void CompositeExecutionObject::setAllLinksAsUncompiled(bool isRecursive) {
		ExecutionObject* childObject;
		map<string, ExecutionObject*>::iterator i;

		FormatterLink* link;
		Link* ncmLink;
		set<FormatterLink*>::iterator j;

		lockComposite();
		j = links->begin();
		while (j != links->end()) {
			link = *j;
			ncmLink = link->getNcmLink();
			if (uncompiledLinks->count(ncmLink) != 0) {
				cout << "CompositeExecutionObject::setAllLinksAsUncompiled ";
				cout << "Warning! Trying to add same link twice" << endl;
			}
			uncompiledLinks->insert(ncmLink);
			delete link;
			link = NULL;
			++j;
		}
		links->clear();

		if (isRecursive) {
			i = execObjList->begin();
			while (i != execObjList->end()) {
				childObject = i->second;
				if (childObject->instanceOf("CompositeExecutionObject")) {
					((CompositeExecutionObject*)childObject)->
						    setAllLinksAsUncompiled(isRecursive);
				}
				++i;
			}
		}
		unlockComposite();
	}

	set<FormatterLink*>* CompositeExecutionObject::getLinks() {
		return links;
	}

	void CompositeExecutionObject::setParentsAsListeners() {
		map<Node*, void*>::iterator parentObjects;
		CompositeExecutionObject* parentObject;

		lockComposite();
		parentObjects = parentTable->begin();
		while (parentObjects != parentTable->end()) {
			parentObject = (CompositeExecutionObject*)(parentObjects->second);

			// register parent as a composite presentation listener
			wholeContent->addEventListener(parentObject);
			++parentObjects;
		}
		unlockComposite();
	}

	void CompositeExecutionObject::unsetParentsAsListeners() {
		map<Node*, void*>::iterator parentObjects;
		CompositeExecutionObject* parentObject;

		if (deleting) {
			return;
		}

		lockComposite();
		parentObjects = parentTable->begin();
		while (parentObjects != parentTable->end()) {
			if (deleting) {
				unlockComposite();
				return;
			}

			parentObject = (CompositeExecutionObject*)(parentObjects->second);

			// register parent as a composite presentation listener
			wholeContent->removeEventListener(parentObject);
			++parentObjects;
		}
		unlockComposite();
	}

	void CompositeExecutionObject::eventStateChanged(
		    void* changedEvent, short transition, short previousState) {

		FormatterEvent* event;
		set<FormatterEvent*>::iterator i;

		event = (FormatterEvent*)changedEvent;
		if (!(event->instanceOf("PresentationEvent"))) {
			cout << "CompositeExecutionObject::eventStateChanged returning";
			cout << " because event is not of presentation type" << endl;
			return;
		}

		/*cout << "CompositeExecutionObject::eventStateChanged id = '";
		cout << id << "' transition = '" << transition << "'";
		cout << endl;*/
		switch (transition) {
			case EventUtil::TR_STARTS:
				if (runningEvents->empty() && pausedEvents->empty()) {
					setParentsAsListeners();
					/*cout << "CompositeExecutionObject::eventStateChanged id = '";
					cout << id << "' starting whole content";
					cout << endl;*/
					wholeContent->start();
				}

				runningEvents->insert(event);
				break;

			case EventUtil::TR_ABORTS:
				lastTransition = transition;
				if (previousState == EventUtil::ST_OCCURRING) {
					i = runningEvents->find(event);
					if (i != runningEvents->end()) {
						runningEvents->erase(i);
					}

				} else if (previousState == EventUtil::ST_PAUSED) {
					i = pausedEvents->find(event);
					if (i != pausedEvents->end()) {
						pausedEvents->erase(i);
					}
				}

				if (runningEvents->empty() && pausedEvents->empty() &&
					    pendingLinks->empty()) {

					wholeContent->abort();
					unsetParentsAsListeners();

					/*cout << "CompositeExecutionObject::";
					cout << "eventStateChanged(" << id;
					cout << ") ALL DONE ABORTS!" << endl;*/
				}
				break;

			case EventUtil::TR_STOPS:
				if (((PresentationEvent*)event)->getRepetitions() == 0) {
					lastTransition = transition;
					if (previousState == EventUtil::ST_OCCURRING) {
						i = runningEvents->find(event);
						if (i != runningEvents->end()) {
							runningEvents->erase(i);
						}

					} else if (previousState == EventUtil::ST_PAUSED) {
						i = pausedEvents->find(event);
						if (i != pausedEvents->end()) {
							pausedEvents->erase(i);
						}
					}

					if (runningEvents->empty() && pausedEvents->empty() &&
						    pendingLinks->empty()) {

						wholeContent->stop();
						unsetParentsAsListeners();

						/*cout << "CompositeExecutionObject::";
						cout << "eventStateChanged(" << id;
						cout << ") ALL DONE STOPS!" << endl;*/
					}

					/*if (runningEvents->empty() && pausedEvents->empty() &&
						    !pendingLinks->empty()) {

						cout << "CompositeExecutionObject::";
						cout << "eventStateChanged(" << id;
						cout << ") pendingLinks = '" << pendingLinks->size();
						cout << "' => '";
						FormatterLink* p;
						map<FormatterLink*, int>::iterator u;
						u = pendingLinks->begin();
						while (u != pendingLinks->end()) {
							p = u->first;
							cout << p->getNcmLink()->getId();
							cout << "', ";
							++u;
						}
						cout << " and no more" << endl;
					}*/

				} else {
					/*cout << "CompositeExecutionObject::";
					cout << "eventStateChanged(" << id;
					cout << ") REPEATING" << endl;*/
				}
				break;

			case EventUtil::TR_PAUSES:
				i = runningEvents->find(event);
				if (i != runningEvents->end()) {
					runningEvents->erase(i);
				}

				pausedEvents->insert(event);
				if (runningEvents->empty()) {
					wholeContent->pause();
				}
				break;

			case EventUtil::TR_RESUMES:
				i = pausedEvents->find(event);
				if (i != pausedEvents->end()) {
					pausedEvents->erase(i);
				}

				runningEvents->insert(event);
				if (runningEvents->size() == 1) {
					wholeContent->resume();
				}
				break;

			default:
				break;
		}
	}

	short CompositeExecutionObject::getPriorityType() {
		return IEventListener::PT_OBJECT;
	}

	void CompositeExecutionObject::linkEvaluationStarted(
		    FormatterCausalLink* link) {

		int linkNumber;
		FormatterLink* evalLink;

		lockComposite();
		evalLink = link;
		if (pendingLinks->count(evalLink) != 0) {
			linkNumber = (*pendingLinks)[evalLink];
			(*pendingLinks)[evalLink] = linkNumber + 1;

		} else {
			(*pendingLinks)[evalLink] = 1;
		}
		unlockComposite();
	}

	void CompositeExecutionObject::linkEvaluationFinished(
		    FormatterCausalLink* link,
		    bool start) {

		int linkNumber;
		FormatterLink* finishedLink;
		map<FormatterLink*, int>::iterator i;

		lockComposite();
		finishedLink = link;
		i = pendingLinks->find(finishedLink);
		if (i != pendingLinks->end()) {
			linkNumber = i->second;
			if (linkNumber == 1) {
				pendingLinks->erase(i);
				if (runningEvents->empty() && pausedEvents->empty() &&
					    pendingLinks->empty()) {

					if (start) {
						// if nothing starts the composition may
						// stay locked as occurring

					} else if (lastTransition == EventUtil::TR_STOPS) {
						unlockComposite();
						wholeContent->stop();
						if (deleting) {
							return;
						}
						unsetParentsAsListeners();
						lockComposite();
					} else {
						unlockComposite();
						wholeContent->abort();
						if (deleting) {
							return;
						}
						unsetParentsAsListeners();
						lockComposite();
					}

				} else {
					/*if (runningEvents->size() == 1 && pausedEvents->empty()) {
						listRunningObjects();
					}*/
				}

			} else {
				(*pendingLinks)[finishedLink] = linkNumber - 1;
			}
		}
		unlockComposite();
	}

	bool CompositeExecutionObject::setPropertyValue(
			AttributionEvent* event, string value, Animation* anim) {

		if (event->getAnchor()->getPropertyName() == "visible") {
			if (value == "true") {
				visible = true;

			} else if (value == "false") {
				visible = false;
			}
			return false;
		}

		return ExecutionObject::setPropertyValue(event, value, anim);
	}

	void CompositeExecutionObject::listRunningObjects() {
		map<string, ExecutionObject*>::iterator i;
		vector<FormatterEvent*>::iterator j;
		vector<FormatterEvent*>* events;
		ExecutionObject* object;
		FormatterEvent* event;

		cout << "CompositeExecutionObject::listRunningObjects for '";
		cout << id << "': ";
		i = execObjList->begin();
		while (i != execObjList->end()) {
			object = i->second;
			events = object->getEvents();
			if (events != NULL) {
				j = events->begin();
				while (j != events->end()) {
					event = *j;
					if (event->getCurrentState() == EventUtil::ST_OCCURRING) {
						cout << "'" << i->first << "', ";
					}
					++j;
				}
				delete events;
				events = NULL;
			}
			++i;
		}
		cout << " runingEvents->size = '" << runningEvents->size() << endl;
	}

	void CompositeExecutionObject::lockComposite() {
		pthread_mutex_lock(&mutexComposite);
	}

	void CompositeExecutionObject::unlockComposite() {
		pthread_mutex_unlock(&mutexComposite);
	}
}
}
}
}
}
}
}
