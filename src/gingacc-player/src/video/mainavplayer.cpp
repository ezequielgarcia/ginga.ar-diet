/*******************************************************************************

  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of Ginga implementation.

    This program is free software: you can redistribute it and/or modify it 
  under the terms of the GNU General Public License as published by the Free 
  Software Foundation, either version 2 of the License.

    Ginga is distributed in the hope that it will be useful, but WITHOUT ANY 
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de Ginga.

    Este programa es Software Libre: Ud. puede redistribuirlo y/o modificarlo 
  bajo los términos de la Licencia Pública General GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    Ginga se distribuye esperando que resulte de utilidad, pero SIN NINGUNA 
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN 
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública 
  General GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General GNU 
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#include "../../include/mainavaplayer.h"

#include <system/io/InputManager.h>
#include<connector/connector/handler/videoresizehandler.h>

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace player {

	MainAVPlayer::MainAVPlayer(string mrl) : Player(mrl) {
		_soundLevel	= 1.0;
		_connector		= InputManager::getInstance()->getConnector();
	}

	MainAVPlayer::~MainAVPlayer() {}

	void MainAVPlayer::setPropertyValue(string name, string value, double duration, double by) {
		//float fValue = 1.0;
		if (name == "soundLevel") {
			/*if (value != "") {
				fValue = stof(value);
			}*/
			//TODO connector.setSoundLevel(fValue);
		}
		Player::setPropertyValue(name, value, duration, by);
	}

	void MainAVPlayer::setVoutWindow(float x, float y, float w, float h) {
		if (_connector){
			cout << "Sending AVDescriptor to connector ";
			cout << "width = " << w << " height = " << h;
			cout << " x = " << x << " y = " << y << endl;

			connector::VideoResizeHandler videoResize;
			connector::AVDescriptor *descriptor = new connector::AVDescriptor(x,y,w,h);
			videoResize.send( _connector, descriptor);
		}
	}
}
}
}
}
}
}