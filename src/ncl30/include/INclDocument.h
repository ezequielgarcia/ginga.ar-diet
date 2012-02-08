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

#ifndef INCLDOCUMENT_H_
#define INCLDOCUMENT_H_

#include "switches/IRule.h"
#include "switches/IRuleBase.h"
using namespace ::br::pucrio::telemidia::ncl::switches;

#include "components/IContextNode.h"
#include "components/INode.h"
#include "components/INodeEntity.h"
using namespace ::br::pucrio::telemidia::ncl::components;

#include "connectors/IConnector.h"
#include "connectors/IConnectorBase.h"
using namespace ::br::pucrio::telemidia::ncl::connectors;

#include "descriptor/IGenericDescriptor.h"
#include "descriptor/IDescriptorBase.h"
using namespace ::br::pucrio::telemidia::ncl::descriptor;

#include "layout/ILayoutRegion.h"
#include "layout/IRegionBase.h"
using namespace ::br::pucrio::telemidia::ncl::layout;

#include "transition/ITransition.h"
#include "transition/ITransitionBase.h"
using namespace ::br::pucrio::telemidia::ncl::transition;

#include "metainformation/IMeta.h"
#include "metainformation/IMetadata.h"
using namespace ::br::pucrio::telemidia::ncl::metainformation;

#include "IBase.h"

#include <string>
#include <vector>
using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ncl {
  class INclDocument {
	public:
		bool addDocument(INclDocument* document, string alias, string location);
		void clear();
		IConnector* getConnector(string connectorId);
		IConnectorBase* getConnectorBase();
		ITransition* getTransition(string transitionId);
		ITransitionBase* getTransitionBase();
		IGenericDescriptor* getDescriptor(string descriptorId);
		IDescriptorBase* getDescriptorBase();
		INclDocument* getDocument(string documentId);
		string getDocumentAlias(INclDocument* document);
		IContextNode* getBody();
		string getDocumentLocation(INclDocument* document);
		vector<INclDocument*>* getDocuments();
		string getId();
		INode* getNode(string nodeId);
		ILayoutRegion* getRegion(string regionId);
		IRegionBase* getRegionBase(int devClass);
		map<int, IRegionBase*>* getRegionBases();
		IRule* getRule(string ruleId);
		IRuleBase* getRuleBase();
		bool removeDocument(INclDocument* document);
		void setConnectorBase(IConnectorBase* connectorBase);
		void setTransitionBase(ITransitionBase* transitionBase);
		void setDescriptorBase(IDescriptorBase* descriptorBase);
		void setDocumentAlias(INclDocument* document, string alias);
		void setBody(IContextNode* node);
		void setDocumentLocation(INclDocument* document, string location);
		void setId(string id);
		void addRegionBase(IRegionBase* regionBase);
		void setRuleBase(IRuleBase* ruleBase);
		void addMetainformation(IMeta* meta);
		void addMetadata(IMetadata* metadata);
		vector<IMeta*>* getMetainformation();
		vector<IMetadata*>* getMetadata();
		void removeMetainformation(IMeta* meta);
		void removeMetadata(IMetadata* metadata);
		void removeAllMetainformation();
		void removeAllMetadata();
  };
}
}
}
}

#endif /*INCLDOCUMENT_H_*/
