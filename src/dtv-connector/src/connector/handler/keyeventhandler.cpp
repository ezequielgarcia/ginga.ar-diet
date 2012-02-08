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
#include "keyeventhandler.h"
#include "../connector.h"
#include <util/buffer.h>
#include <boost/foreach.hpp>

#define RELEASE_KEYS 0
#define RESERVE_KEYS 1

namespace connector {

KeyEventHandler::KeyEventHandler( void )
{
}

KeyEventHandler::~KeyEventHandler( void )
{
}

#define MAX_KEYS 200
#define KEY_BUF_SIZE ((200*4)+4)
void KeyEventHandler::send( Connector *conn, const std::set< util::key::type > &keys ) {
	util::BYTE buf[KEY_BUF_SIZE];
	int pos=MESSAGE_HEADER;
	int index;
	util::BYTE *ptr;

	assert( keys.size() < MAX_KEYS );

	//	Make payload
	BOOST_FOREACH( const util::key::type &elem, keys ) {
		util::DWORD value = (util::DWORD) elem;
		index=0;
		ptr = (util::BYTE *)&value;
		
		buf[pos++] = ptr[index++];
		buf[pos++] = ptr[index++];
		buf[pos++] = ptr[index++];
		buf[pos++] = ptr[index++];		
	}
	MAKE_MSG(buf,messages::keyEvent,pos);
	
	util::Buffer msg( (char *)buf, pos, false );
	conn->send( &msg );
}

void KeyEventHandler::onKeyEvent( const KeyEventCallback &callback ) {
	_onKeyEvent = callback;
}

void KeyEventHandler::process( util::Buffer *msg ) {
	if (!_onKeyEvent.empty()) {
		KeyEventData msgData;
		util::WORD msgSize   = MSG_PAYLOAD_SIZE(msg->buffer());
		util::BYTE *payload  = MSG_PAYLOAD(msg->buffer());
		util::SSIZE_T offset = 0;
		util::BYTE *ptr;
		
		//	Key event
		ptr = (util::BYTE *)(payload+offset);
		while (offset < msgSize) {
			// Fixes Unaligned userspace access
			util::DWORD ins = GET_BE_DWORD( ptr );
			msgData.keys.insert( (util::key::type) ins );
			ptr += sizeof(util::DWORD);
			offset+=sizeof(util::DWORD);
		}
		_onKeyEvent( &msgData );
	}
}

}
