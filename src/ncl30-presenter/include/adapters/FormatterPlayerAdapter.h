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

#ifndef _FORMATTERPLAYER_H_
#define _FORMATTERPLAYER_H_

#include "system/util/functions.h"
using namespace ::util;

#include "system/io/interface/input/IInputEventListener.h"
#include "system/io/interface/output/ISurface.h"
using namespace ::br::pucrio::telemidia::ginga::core::system::io;

#include "player/IPlayer.h"
using namespace ::br::pucrio::telemidia::ginga::core::player;

#include "ncl/components/Content.h"
#include "ncl/components/ContentNode.h"
#include "ncl/components/NodeEntity.h"
#include "ncl/components/ReferenceContent.h"
using namespace ::br::pucrio::telemidia::ncl::components;

#include "ncl/interfaces/LambdaAnchor.h"
#include "ncl/interfaces/IntervalAnchor.h"
using namespace ::br::pucrio::telemidia::ncl::interfaces;

#include "ncl/connectors/EventUtil.h"
using namespace ::br::pucrio::telemidia::ncl::connectors;

#include "../model/CompositeExecutionObject.h"
#include "../model/ExecutionObject.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::components;

#include "../model/AttributionEvent.h"
#include "../model/FormatterEvent.h"
#include "../model/PresentationEvent.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::event;

#include "IPlayerAdapterManager.h"
#include "IPlayerAdapter.h"
#include "INclEditListener.h"
#include "NominalEventMonitor.h"

//#include <limits>
#include <string>
#include <map>
#include <set>
using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace adapters {
	class FormatterPlayerAdapter : public IPlayerAdapter,
			public IPlayerListener, public IAttributeValueMaintainer,
			public IInputEventListener {

		protected:
			IPlayerAdapterManager* manager;
			static IInputManager* im;
			NominalEventMonitor* anchorMonitor;
			set<string> typeSet;
			ExecutionObject* object;
			IPlayer* player;
			string playerCompName;
			string mrl;

		private:
			bool circularSituation;
			bool isLocked;
			pthread_mutex_t objectMutex;

		public:
			FormatterPlayerAdapter(IPlayerAdapterManager* manager);
			virtual ~FormatterPlayerAdapter();

			bool instanceOf(string s);
			virtual void setNclEditListener(INclEditListener* listener){};

		protected:
			virtual void createPlayer();
            void setDescriptorParams();
			virtual void setPropertyToPlayer(string propertyName, string propertyValue);

		public:
			ISurface* getObjectDisplay();
			virtual bool hasPrepared();
			bool setKeyHandler(bool isHandler);
			virtual bool prepare(
					ExecutionObject* object, FormatterEvent* mainEvent);

		protected:
			void prepare();

		public:
			virtual bool start();
			virtual bool stop();
			virtual bool pause();
			virtual bool resume();
			virtual bool abort();
			virtual void naturalEnd();

		private:
			bool checkRepeat(PresentationEvent* mainEvent);

		public:
			virtual bool unprepare();
			virtual bool setPropertyValue(
				    AttributionEvent* event, string value, Animation* anim);

			bool startAttribution(
					string propName, string propValue);

			string getPropertyValue(void* event);
			double getObjectExpectedDuration();
			void updateObjectExpectedDuration();
			double getMediaTime();
			IPlayer* getPlayer();
			void setTimeBasePlayer(FormatterPlayerAdapter* timeBasePlayer);
			virtual void updateStatus(
					short code,
					string parameter="",
					short type=10);

			bool userEventReceived(IInputEvent* ev);

		private:
			void setVisible(bool visible);

		protected:
			bool lockObject();
			bool unlockObject();

		public:
			virtual void flip(){};
			virtual void timeShift(string direction);
	};
}
}
}
}
}
}

using namespace ::br::pucrio::telemidia::ginga::ncl::adapters;

#endif //_FORMATTERPLAYER_H_
