/*******************************************************************************

  Copyright (C) 2010, 2011 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of DTV-connector implementation.

    DTV-connector is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 2 of the License.

    DTV-connector is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de DTV-connector.

    DTV-connector es Software Libre: Ud. puede redistribuirlo y/o modificarlo
  bajo los términos de la Licencia Pública General Reducida GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    DTV-connector se distribuye esperando que resulte de utilidad, pero SIN NINGUNA
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública
  General Reducida GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General Reducida GNU
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#include "serviceplayer.h"
#include "player.h"
#include "../../service.h"
#include "../../../demuxer/ts.h"
#include <boost/foreach.hpp>

namespace tuner {

ServicePlayer::ServicePlayer( Player *player )
{
	assert(player);
	_player = player;
}

ServicePlayer::~ServicePlayer( void )
{
}

bool ServicePlayer::isStarted( Service *srv ) const {
	return std::find( _playing.begin(), _playing.end(), srv->id() ) == _playing.end() ? false : true;
}

bool ServicePlayer::startService( ServiceManager *mgr, Service *srv ) {
	bool started=false;
	
	//	Check if service already started
	if (!isStarted( srv ) && play( mgr, srv )) {
		_playing.push_back( srv->id() );
		started=true;
	}

	return started;
}

void ServicePlayer::stopService( ServiceManager *mgr, Service *srv ) {
	std::vector<ID>::iterator it=std::find( _playing.begin(), _playing.end(), srv->id() );
	if (it != _playing.end()) {
		stop( mgr, srv );
		_playing.erase( it );
	}
}

Player *ServicePlayer::player() {
	return _player;
}

bool ServicePlayer::isAudio( BYTE streamType ) const {
	return streamType == 0x03 ||
		streamType == 0x04 ||
		streamType == 0x0F ||
		streamType == 0x11;
}

bool ServicePlayer::haveAudio( Service *srv ) const {
	bool result=false;
	BOOST_FOREACH( Pmt::ElementaryInfo info, srv->elements() ) {
		if (isAudio( info.streamType )) {
			result=true;
			break;
		}
	}
	return result;	
}

bool ServicePlayer::isVideo( BYTE streamType ) const {
	return streamType == 0x01 ||
		streamType == 0x02 ||
		streamType == 0x1b;
}

bool ServicePlayer::haveVideo( Service *srv ) const {
	bool result=false;
	BOOST_FOREACH( Pmt::ElementaryInfo info, srv->elements() ) {
		if (isVideo( info.streamType )) {
			result=true;
			break;
		}
	}
	return result;
}

}

