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

#ifndef SMILNCMDOCUMENTCONVERTER_H_
#define SMILNCMDOCUMENTCONVERTER_H_

#include "../framework/DocumentParser.h"
using namespace br::pucrio::telemidia::converter::framework;


#include "ncl/connectors/CausalConnector.h"
#include "ncl/connectors/ConnectorBase.h"
using namespace ::br::pucrio::telemidia::ncl::connectors;

#include "ncl/NclDocument.h"
using namespace ::br::pucrio::telemidia::ncl;

#include "ncl/layout/RegionBase.h"
using namespace ::br::pucrio::telemidia::ncl::layout;

#include "ncl/components/ContentNode.h"
#include "ncl/components/ContextNode.h"
#include "ncl/components/AbsoluteReferenceContent.h"
#include "ncl/components/NodeEntity.h"
using namespace ::br::pucrio::telemidia::ncl::components;

#include "ncl/descriptor/DescriptorBase.h"
#include "ncl/descriptor/Descriptor.h"
using namespace ::br::pucrio::telemidia::ncl::descriptor;

#include "ncl/link/CausalLink.h"
using namespace ::br::pucrio::telemidia::ncl::link;


namespace br {
namespace pucrio {
namespace telemidia {
namespace converter {
namespace smil {
  class SmilNcmDocumentConverter : public DocumentParser {
	public:
		SmilNcmDocumentConverter(IDeviceLayout* deviceLayout);
		virtual void* parseRootElement(DOMElement *rootElement);

	private:
		int autoId;
		IDeviceLayout* deviceLayout;

		virtual void initialize() {}

		string getNextId();

		void *parseSmil(DOMElement *parentElement, void *objGrandParent);
		void *createSmil(DOMElement *parentElement, void *objGrandParent);

		void *parseHead(DOMElement *parentElement, void *objGrandParent);
		void *createHead(DOMElement *parentElement, void *objGrandParent);

		void *parseBody(DOMElement *parentElement, void *objGrandParent);
		void *createBody(DOMElement *parentElement, void *objGrandParent);
		void *posCompileBody(DOMElement *parentElement, void *parentObject);

		void *createContext(DOMElement *parentElement, void *objGrandParent);

		void *parseMedia(DOMElement *parentElement, void *objGrandParent);
		void *createMedia(DOMElement *parentElement, void *objGrandParent);

		void *parseSeq(DOMElement *parentElement, void *objGrandParent);
		void *parsePar(DOMElement *parentElement, void *objGrandParent);

	};
}
}
}
}
}

#endif /*SMILNCMDOCUMENTCONVERTER_H_*/
