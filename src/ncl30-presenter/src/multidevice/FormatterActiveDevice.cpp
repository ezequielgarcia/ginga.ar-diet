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

#include "../../include/multidevice/FormatterActiveDevice.h"

#include "../../include/multidevice/FMDComponentSupport.h"

#include "system/io/interface/input/CodeMap.h"

#include "player/INCLPlayer.h"
using namespace ::br::pucrio::telemidia::ginga::core::player;

#include "ncl/layout/DeviceLayout.h"
using namespace ::br::pucrio::telemidia::ncl::layout;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace multidevice {
	FormatterActiveDevice::FormatterActiveDevice(
			IDeviceLayout* deviceLayout, int x, int y, int w, int h) :
				FormatterMultiDevice(deviceLayout, x, y, w, h) {

		set<int>* evs;

		contentsInfo = new map<string, string>;
		deviceClass = IDeviceDomain::CT_ACTIVE;
		deviceLayout->addDevice(
				"systemScreen(1)", 0, 0, DV_QVGA_WIDTH, DV_QVGA_HEIGHT);

#if HAVE_COMPSUPPORT
		serialized = ((WindowCreator*)(cm->getObject("Window")))(
				0, 0, DV_QVGA_WIDTH, DV_QVGA_HEIGHT);
#else
		serialized = new DFBWindow(0, 0, DV_QVGA_WIDTH, DV_QVGA_HEIGHT);
#endif

		evs = new set<int>;
		evs->insert(CodeMap::KEY_TAP);
		im->addInputEventListener(this, evs);

		serialized->setCaps(serialized->getCap("ALPHACHANNEL"));
		serialized->draw();

		if (rdm == NULL) {
#if HAVE_COMPSUPPORT
			rdm = ((IRemoteDeviceManagerCreator*)(cm->getObject(
					"RemoteDeviceManager")))(deviceClass);
#else
			rdm = RemoteDeviceManager::getInstance();
			IDeviceDomain* domain = new ActiveDeviceDomain();
			((RemoteDeviceManager*)rdm)->setDeviceDomain(domain);
#endif
		}

		rdm->setDeviceInfo(deviceClass, w, h);
		rdm->addListener(this);

		mainLayout = new FormatterLayout(x, y, w, h);
		(*layoutManager)[deviceClass] = mainLayout;
	}

	FormatterActiveDevice::~FormatterActiveDevice() {

	}

	void FormatterActiveDevice::connectedToBaseDevice(unsigned int domainAddr) {
		cout << "FormatterActiveDevice::connectedToDomainService '";
		cout << domainAddr << "'" << endl;

		hasRemoteDevices = true;
		im->addInputEventListener(this, NULL);
	}

	bool FormatterActiveDevice::receiveRemoteEvent(
			int remoteDevClass,
			int eventType,
			string eventContent) {

		vector<string>* args;

		if (remoteDevClass == IDeviceDomain::CT_BASE) {
			if (eventType == IDeviceDomain::FT_PRESENTATIONEVENT) {
				if (eventContent.find("::") != std::string::npos) {
					args = split(eventContent, "::");
					if (args->size() == 2) {
						if ((*args)[0] == "start") {
							player->play();

						} else if ((*args)[0] == "stop") {
							player->stop();
						}
					}
					delete args;
					return true;
				}
			}
		}

		return false;
	}

	bool FormatterActiveDevice::receiveRemoteContent(
			int remoteDevClass,
			string contentUri) {

		map<string, string>::iterator i;
		ISurface* s;

		cout << "FormatterActiveDevice::receiveRemoteContent from class '";
		cout << remoteDevClass << "' and contentUri '" << contentUri << "'";
		cout << endl;

		i = contentsInfo->find(contentUri);
		if (contentUri.find(".ncl") != std::string::npos &&
				i != contentsInfo->end()) {

			NclPlayerData* data = new NclPlayerData;
			data->baseId            = i->second;
			data->playerId          = i->second;
			data->devClass          = deviceClass;
			data->x                 = xOffset;
			data->y                 = yOffset;
			data->w                 = defaultWidth;
			data->h                 = defaultHeight;
			data->enableGfx         = false;
			data->parentDocId       = "";
			data->nodeId            = "";
			data->docId             = "";
			data->focusManager      = NULL;

#if HAVE_COMPSUPPORT
			player = ((NCLPlayerCreator*)(cm->getObject("Formatter")))(data);

			s = ((SurfaceCreator*)(cm->getObject("Surface")))(
					NULL, 0, 0);
#else
			player = (INCLPlayer*)(new FormatterMediator(data));

			s = new DFBSurface();
#endif

			player->setSurface(s);
			((INCLPlayer*)player)->setCurrentDocument(contentUri);
			((INCLPlayer*)player)->setParentLayout(mainLayout);
			return true;
		}

		return false;
	}

	bool FormatterActiveDevice::receiveRemoteContentInfo(
			string contentId, string contentUri) {

		(*contentsInfo)[contentUri] = contentId;
		return true;
	}

	bool FormatterActiveDevice::userEventReceived(IInputEvent* ev) {
		string mnemonicCode;
		int currentX;
		int currentY;
		int code;

		code = ev->getKeyCode();

		/*cout << "FormatterBaseDevice::userEventReceived CLICK";
		cout << endl;*/
		if (code == CodeMap::KEY_TAP) {
			ev->getAxisValue(&currentX, &currentY, NULL);
			tapObject(deviceClass, currentX, currentY);
		}

		return true;
	}
}
}
}
}
}
}
