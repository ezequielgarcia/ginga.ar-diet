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
#include "ior.h"
#include "object.h"
#include <stdio.h>
#include <assert.h>

namespace tuner {
namespace dsmcc {
namespace biop {

Ior::Ior( void )
{
	//	Initialize Object Location
	_timeout        = 0;
	_transactionID  = 0;
	_associationTag = 0;	
}

Ior::~Ior()
{
}

//  Getters
const ObjectLocation &Ior::location() const {
	return _location;
}

WORD Ior::associationTag() const {
	return _associationTag;
}

bool Ior::isValid() const {
    return _location.keyID != INVALID_KEY && _transactionID;
}

SSIZE_T Ior::parse( const BYTE *data, SIZE_T len ) {
    SSIZE_T offset=0;

    //  Initialize variables
    _location.keyID = INVALID_KEY;

    //  len
    DWORD typeIdLen = RDW(data,offset);

    //  typeid
   // _typeID.copy( (char *)(data+offset), typeIdLen );
    offset += typeIdLen;

    //  gaps in typeid name
    BYTE gapBytes = typeIdLen % 4;
    if (gapBytes) {
        offset += 4 - gapBytes;
    }

    //  Parse profiles
    DWORD profilesCount = RDW(data,offset);
    if (profilesCount) {
        offset += parseTaggedProfiles( profilesCount, data+offset, len-offset );
    }

    return offset;
}

SSIZE_T Ior::parseTaggedProfiles( DWORD count, const BYTE *data, SIZE_T len ) {
    SSIZE_T offset=0;
    SSIZE_T parsed=0;

    for (DWORD i=0; i<count; ++i) {
        DWORD idTag   = RDW(data,offset);
        DWORD dataLen = RDW(data,offset);

        switch (idTag) {
            case TAG_PROFILE_BIOP:
                parsed = parseProfileBIOP( data+offset, len-offset );
                break;
            default:
                printf( "[dsmcc::Ior] Warning: Cannot parse profile %lx\n", idTag );
        };

        offset += dataLen;
    }

    return offset;
}

SSIZE_T Ior::parseProfileBIOP( const BYTE *data, SIZE_T len ) {
    SSIZE_T offset = 0;

    //  byte_order (set to 0x00)
    BYTE byteOrder = RB(data,offset);
    if (byteOrder) {
        printf( "[dsmcc::Ior] Warning byte order invalid %x\n", byteOrder );
        return offset;
    }

    BYTE count = RB(data,offset);
	for (BYTE c=0; c<count; c++) {
		DWORD tag     = RDW(data,offset);
		BYTE  tagLen  = RB(data,offset);
		SIZE_T parsed = 0;

		//	Check len!
		if (len < tagLen) {
			printf( "[dsmcc::Ior] Warning, invalid tag size: bytes=%ld, size=%d\n", len-offset, tagLen );
			return offset;
		}

		//	Parse tag
		switch (tag) {
			case TAG_PROFILE_OBJECT_LOCATION:
				parsed = parseObjectLocation( data+offset, tagLen );
				break;
			case TAG_PROFILE_CONNBIND:
				parsed = parseConnBinn( data+offset, tagLen );
				break;
			default:
				printf( "[dsmcc::Ior] Warning invalid tag: tag=%lx, len=%d\n", tag, tagLen );
		}
		
		if (tagLen != parsed) {
			printf( "[dsmcc::Ior] Warning parsing of tag %lx incomple: %ld of %d\n", tag, parsed, tagLen );
		}	
		offset += tagLen;
    }
    return offset;
}

SSIZE_T Ior::parseObjectLocation( const BYTE *data, SIZE_T len ) {
	SSIZE_T offset=0;

    //  CarouselID
    _location.carouselID = RDW(data,offset);

    //  ModuleID
    _location.moduleID = RW(data,offset);

     //  Check BIOP version
    WORD version = RW(data,offset);
    if (version != BIOP_VER) {
        printf( "[dsmcc::Ior] Warning version invalid: %x\n", version );
        return offset;
    }

	//	Parse Object key
    offset += Object::objectKey( data+offset, len-offset, _location.keyID );
	return offset;
}

SSIZE_T Ior::parseConnBinn( const BYTE *data, SIZE_T /*len*/ ) {
	SSIZE_T offset=0;

	//	Skip tapsCount + id (Only process the first, as suggested the DVB BlueBook A068 Rev. 3)
	offset += 3;

	WORD use = RW(data,offset);

	//	AssociationTag
	_associationTag = RW(data,offset);

	BYTE selectorLen = RB(data,offset);

	if (use != BIOP_TAP_DELIVERY_PARA_USE || selectorLen != 0xA) {
		printf( "[dsmcc::Ior] Warning use tap not valid: use=%x, len=%d\n", use, selectorLen );
		offset += selectorLen;
		return offset;
	}

	WORD selectorType = RW(data,offset);
	if (selectorType != 1) {
		printf( "[dsmcc::Ior] Warning selector type not valid: %d\n", selectorType );
		offset += selectorLen-2;
		return offset;
	}

	_transactionID = RDW(data,offset);
	_timeout       = RDW(data,offset);
	//printf( "TAP: carouselID=%lx, moduleID=%x, transactionID=%lx, timeout=%lx\n", _carouselID, _moduleID, _transactionID, _timeout );
	return offset;
}

}
}
}
