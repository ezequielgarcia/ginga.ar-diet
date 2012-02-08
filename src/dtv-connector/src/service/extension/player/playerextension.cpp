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
#include "playerextension.h"
#include "serviceplayer.h"
#include "../../service.h"
#include <boost/foreach.hpp>
#include <stdio.h>

namespace tuner {

PlayerExtension::PlayerExtension( ServiceManager *srvMgr )
	: Extension( srvMgr )
{
}

PlayerExtension::~PlayerExtension( void )
{
	cleanPlayers();
}

//	Service notification
void PlayerExtension::onServiceStarted( Service *srv ) {
	//	Check if stream player can run service
	ServicePlayer *player = findPlayer( srv->type() );
	if (!(player && player->startService( srvMgr(), srv ))) {
		//	TODO: Notify a error	 
		printf( "[PlayerExtension] Warning, cannot play service: type=%x\n",
			srv->type() );
	}
}

void PlayerExtension::onServiceStopped( Service *srv ) {
	//	Find service player by type
	ServicePlayer *player = findPlayer( srv->type() );
	if (player) {
		//	Stop player
		player->stopService( srvMgr(), srv );
	}
}

//	Players
void PlayerExtension::registerPlayer( service::type::type type, ServicePlayer *player ) {
	printf( "[PlayerExtension] Register player of type %x\n", type );
	ServicePlayer *old = findPlayer( type );
	if (old) {
		delete old;
	}
	_players[type] = player;
}

bool PlayerExtension::isPlayable( Service *srv ) const {
	bool result=false;
	ServicePlayer *player = findPlayer( srv->type() );
	if (player) {
		result=player->isValid( srv );
	}
	return result;	
}

ServicePlayer *PlayerExtension::findPlayer( BYTE type ) const {
	ServicePlayer *player = NULL;
	
	MapOfPlayer::const_iterator it=_players.find( (service::type::type)type );
	if (it != _players.end()) {
		player = (*it).second;
	}
	return player;
}

void PlayerExtension::cleanPlayers( void ) {
	MapOfPlayerItem item;
	BOOST_FOREACH(item, _players) {
		delete item.second;
	}
	_players.clear();
}

}

