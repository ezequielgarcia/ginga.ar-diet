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

#ifndef DataWrapperListener_H_
#define DataWrapperListener_H_

#include "system/io/IGingaLocatorFactory.h"
using namespace ::br::pucrio::telemidia::ginga::core::system::io;

#include "player/INCLPlayer.h"
using namespace ::br::pucrio::telemidia::ginga::core::player;

#include "tuner/ITuner.h"
using namespace ::br::pucrio::telemidia::ginga::core::tuning;

#include "tsparser/IEventInfo.h"
using namespace ::br::pucrio::telemidia::ginga::core::tsparser;

#include "tsparser/IShortEventDescriptor.h"
using namespace ::br::pucrio::telemidia::ginga::core::tsparser::si::descriptors;

#include "dataprocessing/dsmcc/IStreamEventListener.h"
#include "dataprocessing/IEPGListener.h"
using namespace ::br::pucrio::telemidia::ginga::core::dataprocessing;

#include "dataprocessing/dsmcc/npt/INPTListener.h"
using namespace ::br::pucrio::telemidia::ginga::core::dataprocessing::dsmcc::npt;

#include "dataprocessing/dsmcc/carousel/IServiceDomainListener.h"
#include "dataprocessing/dsmcc/carousel/object/IObjectListener.h"
using namespace ::br::pucrio::telemidia::ginga::core::dataprocessing::carousel;

#include "IPresentationEngineManager.h"

#include <string>
using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace lssm {
  class DataWrapperListener : public IStreamEventListener,
		public IObjectListener, public IEPGListener, IServiceDomainListener {

	private:
		IPresentationEngineManager* pem;
		string documentUri;
		bool autoMount;

	public:
		DataWrapperListener(IPresentationEngineManager* pem);
		virtual ~DataWrapperListener();
		void autoMountOC(bool autoMountIt);
		void objectMounted(string ior, string clientUri, string name);
		void receiveEventInfo(set<IEventInfo*>* events);
		void receiveStreamEvent(IStreamEvent* event);
		void serviceDomainMounted(
				map<string, string>* names, map<string, string>* paths);
  };
}
}
}
}
}

#endif /*DataWrapperListener_H_*/
