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

#include "../../include/link/Link.h"

#include "../../include/components/ContextNode.h"
using namespace ::br::pucrio::telemidia::ncl::components;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ncl {
namespace link {
	Link::Link(string id, Connector* connector) : Entity(id) {
		roleBinds = new map<string, vector<Bind*>*>;
		binds = new vector<Bind*>;
		this->connector = connector;
		parameters = new map<string, Parameter*>;
		composition = NULL;
		typeSet.insert("Link");
	}

	Link::~Link() {
		map<string, vector<Bind*>*>::iterator i;
		vector<Bind*>::iterator j;
		map<string, Parameter*>::iterator k;
		vector<Bind*>* rmBinds;

		if (roleBinds != NULL) {
			i = roleBinds->begin();
			while (i != roleBinds->end()) {
				rmBinds = i->second;
				rmBinds->clear();
				delete rmBinds;
				++i;
			}
			delete roleBinds;
			roleBinds = NULL;
		}

		if (binds != NULL) {
			j = binds->begin();
			while (j != binds->end()) {
				delete *j;
				++j;
			}
			delete binds;
			binds = NULL;
		}

		if (parameters != NULL) {
			delete parameters;
			parameters = NULL;
		}
	}

	Bind* Link::bind(Node* node, InterfacePoint* interfPt,
		    GenericDescriptor* desc, string roleId) {

		Role* role;

		role = connector->getRole(roleId);
		if (role == NULL)
			return NULL;

		return bind(node, interfPt, desc, role);
	}

	Bind* Link::bind(Node* node,
		    InterfacePoint* interfPt,
		    GenericDescriptor* desc, Role* role) {

		Bind *bind;
		vector<Bind*> *roleBindList;
		int maxConn;
		string label;

		label = role->getLabel();
		if (roleBinds->count(label) == 0) {
			roleBindList = new vector<Bind*>;
			(*roleBinds)[label] = roleBindList;

		} else {
			roleBindList = (*roleBinds)[label];
		}

		// se pode fazer o bind se o numero de binds for menor
		// que a cardinalidade maxima do papel
		maxConn = role->getMaxCon();

		if (maxConn == Role::UNBOUNDED ||
			    (int)(roleBindList->size()) < maxConn) {

			bind = new Bind(node, interfPt, desc, role);
			roleBindList->push_back(bind);
			binds->push_back(bind);
			return bind;

		} else {
			return NULL;
		}
	}

	bool Link::isConsistent() {
		vector<Role*>::iterator iterator;
		Role *role;
		int minConn, maxConn;
		vector<Role*> *roles;

		roles = connector->getRoles();
		iterator = roles->begin();
		while (iterator != roles->end()) {
			role = *iterator;
			//if (role instanceof ICardinalityRole) {
			minConn = role->getMinCon();
			maxConn = role->getMaxCon();
			//}
			//else {
			//	minConn = 1;
			//	maxConn = 1;
			//}
			if ((int)getNumRoleBinds(role) < minConn ||
				    (maxConn != Role::UNBOUNDED &&
				    (int)getNumRoleBinds(role) > maxConn)) {

				delete roles;
				return false;
			}
		}
		delete roles;
		return true;
	}

	Bind* Link::getBind(Node* node, InterfacePoint* interfPt,
		    GenericDescriptor* desc, Role* role) {

		map<string, vector<Bind*>*>::iterator i;


		bool containsKey = false;
		for (i = roleBinds->begin(); i != roleBinds->end(); ++i)
			if (i->first == role->getLabel())
				containsKey = true;

		if (!containsKey)
			return NULL;

		Bind* bind;
		vector<Bind*>::iterator bindIterator;

		vector<Bind*>* roleBindList;
		roleBindList = (*roleBinds)[role->getLabel()];

		for (bindIterator = roleBindList->begin();
			    bindIterator != roleBindList->end(); ++roleBindList) {

			bind = (*bindIterator);
			if ((bind->getNode() == node)
			    && (bind->getInterfacePoint() == interfPt)
			    && (bind->getDescriptor() == desc))
			        return bind;
		}

		return NULL;
	}

	vector<Bind*>* Link::getBinds() {
		if (binds->empty())
			return NULL;

		return binds;
	}

	Connector* Link::getConnector() {
		return connector;
	}

	void Link::setParentComposition(LinkComposition* composition) {
		if (composition == NULL ||
			    ((ContextNode*)composition)->containsLink(this)) {

			this->composition = composition;
		}
	}

	LinkComposition* Link::getParentComposition() {
		return composition;
	}

	unsigned int Link::getNumBinds() {
		return binds->size();
	}

	unsigned int Link::getNumRoleBinds(Role* role) {
		map<string, vector<Bind*>*>::iterator i;

		bool containsKey = false;
		for (i = roleBinds->begin(); i != roleBinds->end(); ++i)
			if (i->first == role->getLabel())
				containsKey = true;

		if (!containsKey)
			return 0;

		vector<Bind*>* roleBindList;
		roleBindList = (*roleBinds)[role->getLabel()];
		return roleBindList->size();
	}

	vector<Bind*>* Link::getRoleBinds(Role* role) {
		if (roleBinds->empty()) {
			return NULL;
		}

		vector<Bind*>* roleBindList;
		map<string, vector<Bind*>*>::iterator i;
		for (i = roleBinds->begin(); i != roleBinds->end(); ++i) {
			if (i->first == role->getLabel()) {
				roleBindList = i->second;
				return roleBindList;
			}
		}

		return NULL;
	}

	bool Link::isMultiPoint() {
		if (binds->size() > 2)
			return true;
		else
			return false;
	}

	string Link::toString() {
		string linkStr;
		vector<Bind*>::iterator iterator;
		Bind* bind;

		linkStr = (getId()) + "\n" +
			    ((this->getConnector())->getId()) + "\n";

		iterator = binds->begin();
		while (iterator++ != binds->end()) {
			bind = (*iterator);
			linkStr += ((bind->getRole())->getLabel())
			  + " "
			  + ((bind->getNode())->getId())
			  + " "
			  + ((bind->getInterfacePoint())->getId()) + "\n"
			  + ((bind->getRole())->getLabel());
			++iterator;
		}
		return linkStr;
	}

	void Link::setConnector(Connector* connector) {
		this->connector = connector;

		roleBinds->clear();
		binds->clear();
	}

	bool Link::unBind(Bind *bind) {
		vector<Bind*> *roleBindList;
		bool containsBind = false;

		vector<Bind*>::iterator it;
		for (it = binds->begin(); it != binds->end(); ++it) {
			if(bind == *it) {
				containsBind = true;
				binds->erase(it);
				break;
			}
		}

		if (!containsBind) {
			return false;
		}

		if( roleBinds->count( bind->getRole()->getLabel() )==1 ) {
			roleBindList = (*roleBinds)[ bind->getRole()->getLabel() ];
			vector<Bind*>::iterator i;
			for (i = roleBindList->begin(); i != roleBindList->end(); ++i) {
				if( *i == bind ) {
					roleBindList->erase( i );
					break;
				}
			}
		}
		return true;
	}

	void Link::addParameter(Parameter* parameter) {
		if (parameter == NULL)
			return;

		(*parameters)[parameter->getName()] = parameter;
	}

	vector<Parameter*>* Link::getParameters() {
		if (parameters->empty())
			return NULL;

		vector<Parameter*>* params;
		params = new vector<Parameter*>;

		map<string, Parameter*>::iterator i;
		for (i=parameters->begin();i!=parameters->end();++i)
			params->push_back(i->second);

		return params;
	}

	Parameter* Link::getParameter(string name) {
		if (parameters->empty())
			return NULL;

		map<string, Parameter*>::iterator i;
		for (i=parameters->begin();i!=parameters->end();++i)
			if (i->first == name)
				return (Parameter*)(i->second);

		return NULL;
	}

	void Link::removeParameter(Parameter *parameter) {
		if (parameters->empty())
			return;

		map<string, Parameter*>::iterator i;
		for (i=parameters->begin();i!=parameters->end();++i) {
			if (i->first == parameter->getName()) {
				parameters->erase(i);
				return;
			}
		}
	}

	void Link::updateConnector(Connector *newConnector) {
		int i, size;
		Bind *bind;
		Role *newRole;

		if (this->connector == NULL) {
			//TODO test if the set of roles is identical
			return;
		}

		size = binds->size();
		for (i = 0; i < size; i++) {
			bind = (Bind*)(*binds)[i];
			newRole = newConnector->getRole(bind->getRole()->getLabel());
			bind->setRole(newRole);
		}
		this->connector = newConnector;
	}

	bool Link::containsNode(Node *node, GenericDescriptor *descriptor) {
		return containsNode(node, descriptor, getBinds());
	}

	bool Link::containsNode(Node *node, GenericDescriptor *descriptor,
		    vector<Bind*> *binds) {

		Bind *bind;
		Node *bindNode;
		InterfacePoint *bindInterface;
		GenericDescriptor *bindDescriptor;

		vector<Bind*>::iterator i;
		i = binds->begin();
		while (i != binds->end()) {
			bind = (*i);
			bindInterface = bind->getInterfacePoint();
			if (bindInterface != NULL &&
					bindInterface->instanceOf("Port")) {
				bindNode = ((Port*)bindInterface)->getEndNode();

			} else {
				bindNode = bind->getNode();
			}

			/*cout << "Link::containsNode is '" << node->getId() << "' == '";
			cout << bindNode->getId() << "'??" << endl;*/

			if (node == bindNode) {
				bindDescriptor = bind->getDescriptor();
				if (bindDescriptor != NULL) {
					if (bindDescriptor == descriptor) {
						return true;
					}

				} else {
					if (((NodeEntity*)node->getDataEntity())->
						    getDescriptor() == descriptor) {

						return true;
					}
				}
			}
			++i;
		}
		return false;
	}
}
}
}
}
}
