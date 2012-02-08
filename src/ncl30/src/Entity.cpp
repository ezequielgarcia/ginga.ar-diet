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

#include "../include/Entity.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ncl {
	Entity::Entity(string id) {
		this->id = id;
		typeSet.insert("Entity");
	}

	Entity::~Entity() {

	}


	void Entity::printHierarchy() {
		set<string>::iterator i;

		i = typeSet.begin();
		while (i != typeSet.end()) {
			cout << *i << " ";
			++i;
		}
	}

	bool Entity::instanceOf(string s) {
		if (!typeSet.empty()) {
			/*wclog << "Entity instanceOf for " << s << " with the following set:" << endl;
			for(set<string>::iterator it = typeSet.begin(); it!=typeSet.end(); it++) {
				wclog << "[" << *it << "] ";
			}
			wclog << ((typeSet.find(s) != typeSet.end()) ? "true" : "false") << endl;*/
			return ( typeSet.find(s) != typeSet.end() );
		} else {
			//wclog << "Entity instanceOf for " << s << " has an empty set" << endl;
			return false;
		}
	}

	int Entity::compareTo(Entity* otherEntity) {
		string otherId;
		int cmp;

		otherId = (static_cast<Entity*>(otherEntity))->getId();

		if (id == "")
			return -1;

		if (otherId == "")
			return 1;

		cmp = id.compare(otherId);
		switch (cmp) {
			case 0 :
				return 0;
			default :
				if (cmp < 0)
					return -1;
				else
					return 1;
		}
	}

	/*bool Entity::equals(Entity* otherEntity) {
		string otherId;

		otherId = (static_cast<Entity*>(otherEntity))->getId();
		return (id.compare(otherId) == 0);
	}*/

	string Entity::getId() {
		return id;
	}

	void Entity::setId(string someId) {
		id = someId;
	}

	string Entity::toString() {
		return id;
	}

	Entity *Entity::getDataEntity() {
		return this;
	}
}
}
}
}
