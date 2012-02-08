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

#include "../../../include/model/LinkCompoundAction.h"
#include "../../../include/model/LinkSimpleAction.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace link {
	LinkCompoundAction::LinkCompoundAction(short op) : LinkAction() {
		actions = new vector<LinkAction*>;
		this->op= op;
		typeSet.insert("LinkCompoundAction");
		hasStart = false;
		runing = false;
	}

	LinkCompoundAction::~LinkCompoundAction() {
		vector<LinkAction*>::iterator i;
		LinkAction* action;

		for (i = actions->begin(); i != actions->end(); ++i) {
			action = (LinkAction*)(*i);
			action->removeActionProgressionListener(this);
			delete action;
			action = NULL;
		}

		actions->clear();
		delete actions;
		actions = NULL;
	}

	short LinkCompoundAction::getOperator() {
		return op;
	}

	void LinkCompoundAction::addAction(LinkAction* action) {
		vector<LinkAction*>::iterator i;

		if (runing) {
			return;
		}

		action->addActionProgressionListener(this);
		i = actions->begin();
		while (i != actions->end()) {
			if (*i == action) {
				cout << "LinkCompoundAction::addAction Warning!";
				cout << " Trying to add same action twice";
				cout << endl;
				return;
			}
			++i;
		}
		actions->push_back(action);
	}

	vector<LinkAction*>* LinkCompoundAction::getActions() {
		vector<LinkAction*>* acts;

		if (runing) {
			return NULL;
		}

		if (actions->empty()) {
			return NULL;
		}

		acts = new vector<LinkAction*>(*actions);
		return acts;
	}

	vector<FormatterEvent*>* LinkCompoundAction::getEvents() {
		vector<LinkAction*>* acts;
		vector<LinkAction*>::iterator i;
		LinkAction* action;
		vector<FormatterEvent*>* events;
		vector<FormatterEvent*>* actionEvents;
		vector<FormatterEvent*>::iterator j;

		if (runing) {
			return NULL;
		}

		if (actions->empty()) {
			return NULL;
		}

		acts = new vector<LinkAction*>(*actions);
		events = new vector<FormatterEvent*>;

		for (i = acts->begin(); i != acts->end(); ++i) {
			action = (LinkAction*)(*i);
			actionEvents = action->getEvents();
			if (actionEvents != NULL) {
				for (j = actionEvents->begin(); j != actionEvents->end(); ++j) {
					events->push_back(*j);
				}
				delete actionEvents;
				actionEvents = NULL;
			}
		}

		delete acts;
		if (events->empty()) {
			delete events;
			return NULL;
		}

		return events;
	}

	vector<LinkAction*>* LinkCompoundAction::getImplicitRefRoleActions() {
		vector<LinkAction*>* acts;
		vector<LinkAction*>::iterator i;
		vector<LinkAction*>* assignmentActs;
		vector<LinkAction*>* refActs;
		vector<LinkAction*>::iterator j;

		if (runing) {
			return NULL;
		}

		if (actions->empty()) {
			return NULL;
		}

		acts = new vector<LinkAction*>(*actions);
		refActs = new vector<LinkAction*>;

		for (i = acts->begin(); i != acts->end(); ++i) {
			assignmentActs = (*i)->getImplicitRefRoleActions();
			if (assignmentActs != NULL) {
				for (j = assignmentActs->begin();
						j != assignmentActs->end(); ++j) {

					refActs->push_back(*j);
				}
				delete assignmentActs;
				assignmentActs = NULL;
			}
		}

		delete acts;
		if (refActs->empty()) {
			delete refActs;
			return NULL;
		}

		return refActs;
	}

	void LinkCompoundAction::run() {
		int i, size;
		LinkAction *action;

		runing = true;

		LinkAction::run();

		size = actions->size();
		pendingActions = size;
		hasStart = false;

		if (op == CompoundAction::OP_PAR) {
			for (i = 0; i < size; i++) {
				action = (LinkAction*)(actions->at(i));
				/*if (action->instanceOf("LinkSimpleAction")) {
					cout << "LinkCompoundAction::run PAR start action '";
					cout << ((LinkSimpleAction*)action)->getEvent()->getId();
					cout << "'" << endl;
				}*/
				//actionThread = new Thread(action);
				action->start();
			}

		} else {
			for (i = 0; i < size; i++) {
				action = (LinkAction*)(actions->at(i));
				/*if (action->instanceOf("LinkSimpleAction")) {
					cout << "LinkCompoundAction::run SEQ start action '";
					cout << ((LinkSimpleAction*)action)->getEvent()->getId();
					cout << "'" << endl;
				}*/
				action->run();
			}
		}
	}

	void LinkCompoundAction::actionProcessed(bool start) {
		pendingActions--;
		hasStart = (hasStart || start);
		if (pendingActions == 0) {
			notifyProgressionListeners(hasStart);
		}
	}
}
}
}
}
}
}
}
