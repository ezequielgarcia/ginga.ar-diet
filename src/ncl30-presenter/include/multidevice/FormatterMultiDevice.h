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

#ifndef _FormatterMultiDevice_H_
#define _FormatterMultiDevice_H_

#include "system/io/ILocalDeviceManager.h"
#include "system/io/interface/input/IInputEventListener.h"
using namespace ::br::pucrio::telemidia::ginga::core::system::io;

#include "multidevice/services/IDeviceDomain.h"
#include "multidevice/IRemoteDeviceManager.h"

#include "ncl/layout/LayoutRegion.h"
using namespace ::br::pucrio::telemidia::ncl::layout;

#include "ncl/components/Content.h"
#include "ncl/components/NodeEntity.h"
#include "ncl/components/ReferenceContent.h"
using namespace ::br::pucrio::telemidia::ncl::components;

#include "../model/CascadingDescriptor.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::presentation;

#include "../model/ExecutionObject.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::components;

#include "../model/FormatterLayout.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::presentation;

#include "IFormatterMultiDevice.h"

#include <pthread.h>

#include <map>
using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace multidevice {
	class FormatterMultiDevice :
			public IFormatterMultiDevice,
			public IInputEventListener {

		protected:
			static ILocalDeviceManager* dm;
			static IInputManager* im;
			static pthread_mutex_t mutex;
			static IRemoteDeviceManager* rdm;
			IDeviceLayout* deviceLayout;
			map<int, FormatterLayout*>* layoutManager;
			vector<string>* activeUris;
			string activeBaseUri;
			FormatterLayout* mainLayout;
			IWindow* serialized;
			IWindow* printScreen;
			IWindow* bitMapScreen;
			int xOffset;
			int yOffset;
			int defaultWidth;
			int defaultHeight;
			int deviceClass;
			bool hasRemoteDevices;

			static const int DV_QVGA_WIDTH  = 480;
			static const int DV_QVGA_HEIGHT = 320;

		public:
			FormatterMultiDevice(
					IDeviceLayout* deviceLayout, int x, int y, int w, int h);

			virtual ~FormatterMultiDevice();

			void setBackgroundImage(string uri);
			void* getMainLayout();
			void* getFormatterLayout(int devClass);
			string getScreenShot();

		protected:
			string serializeScreen(int devClass, IWindow* mapWindow);
			virtual void postMediaContent(int destDevClass);

		public:
			FormatterLayout* getFormatterLayout(
					CascadingDescriptor* descriptor);

			void prepareFormatterRegion(
					ExecutionObject* executionObject,
					ISurface* renderedSurface);

			void showObject(ExecutionObject* executionObject);
			void hideObject(ExecutionObject* executionObject);

		protected:
			virtual bool newDeviceConnected(int newDevClass, int w, int h);
			virtual void connectedToBaseDevice(unsigned int domainAddr)=0;

			virtual bool receiveRemoteEvent(
					int remoteDevClass,
					int eventType,
					string eventContent);

			virtual bool receiveRemoteContent(
					int remoteDevClass,
					char *stream, int streamSize) {

				return false;
			};

			virtual bool receiveRemoteContentInfo(
					string contentId, string contentUri) {

				return false;
			};

			void renderFromUri(IWindow* win, string uri);
			void tapObject(int devClass, int x, int y);

			virtual bool receiveRemoteContent(
					int remoteDevClass,
					string contentUri) {

				return false;
			};

		public:
			void addActiveUris(string baseUri, vector<string>* uris);
			virtual void updatePassiveDevices();

		protected:
			void updateStatus(short code, string parameter, short type);
			virtual bool userEventReceived(IInputEvent* ev)=0;
	};
}
}
}
}
}
}

#endif /* _FormatterMultiDevice_H_ */
