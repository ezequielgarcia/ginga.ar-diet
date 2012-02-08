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

#include "../../include/multidevice/FormatterPassiveDevice.h"
#include "../../include/multidevice/FMDComponentSupport.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace multidevice {
	FormatterPassiveDevice::FormatterPassiveDevice(
			IDeviceLayout* deviceLayout, int x, int y, int w, int h) :
				FormatterMultiDevice(deviceLayout, x, y, w, h) {

		deviceClass = IDeviceDomain::CT_PASSIVE;
#if HAVE_COMPSUPPORT
		serialized = ((WindowCreator*)(cm->getObject("Window")))(
				x, y, defaultWidth, defaultHeight);
#else
		serialized = new DFBWindow(x, y, defaultWidth, defaultHeight);
#endif

		serialized->setCaps(serialized->getCap("ALPHACHANNEL"));
		serialized->draw();

		if (rdm == NULL) {
#if HAVE_COMPSUPPORT
			rdm = ((IRemoteDeviceManagerCreator*)(cm->getObject(
					"RemoteDeviceManager")))(deviceClass);
#else
			rdm = RemoteDeviceManager::getInstance();
			IDeviceDomain* domain = new PassiveDeviceDomain();
			((RemoteDeviceManager*)rdm)->setDeviceDomain(domain);
#endif
		}

		rdm->setDeviceInfo(deviceClass, w, h);
		rdm->addListener(this);

		im->addInputEventListener(this, NULL);

		mainLayout = new FormatterLayout(x, y, w, h);
		(*layoutManager)[deviceClass] = mainLayout;
	}

	FormatterPassiveDevice::~FormatterPassiveDevice() {

	}

	void FormatterPassiveDevice::connectedToBaseDevice(unsigned int domainAddr) {
		/*cout << "FormatterPassiveDevice::connectedToDomainService '";
		cout << domainAddr << "'" << endl;*/

		hasRemoteDevices = true;
	}

	bool FormatterPassiveDevice::receiveRemoteContent(
			int remoteDevClass,
			string contentUri) {

		/*cout << "FormatterPassiveDevice::receiveRemoteContent from class '";
		cout << remoteDevClass << "' and contentUri '" << contentUri << "'";
		cout << endl;*/

		renderFromUri(serialized, contentUri);
		return true;
	}

	bool FormatterPassiveDevice::userEventReceived(IInputEvent* ev) {
		string mnemonicCode;
		int currentX;
		int currentY;
		int code;

		code = ev->getKeyCode();
		if (code == CodeMap::KEY_F11) {
			std::abort();
		}

		if (!hasRemoteDevices) {
			return false;
		}

		mnemonicCode = CodeMap::getInstance()->getValue(code);

		if (ev->isButtonPressType()) {
			ev->getAxisValue(&currentX, &currentY, NULL);

			mnemonicCode = (mnemonicCode + "," +
					itos(currentX - xOffset) + "," +
					itos(currentY - yOffset));
		}

		/*cout << "FormatterPassiveDevice::userEventReceived posting '";
		cout << mnemonicCode << "'" << endl;*/

		rdm->postEvent(
				IDeviceDomain::CT_BASE,
				IDeviceDomain::FT_SELECTIONEVENT,
				(char*)(mnemonicCode.c_str()),
				(int)(mnemonicCode.length()));

		return false;
	}
}
}
}
}
}
}
