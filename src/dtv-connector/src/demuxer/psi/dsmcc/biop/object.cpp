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
#include "object.h"
#include "biop.h"
#include "file.h"
#include "directory.h"
#include "streamevent.h"
#include "../module.h"
#include <boost/filesystem.hpp>
#include <stdexcept>
#include <stdio.h>

namespace tuner {
namespace dsmcc {
namespace biop {

namespace fs = boost::filesystem;

SSIZE_T Object::objectKey( const BYTE *data, SIZE_T len, ObjectKeyType &key ) {
    SSIZE_T off=0;

    //  Object key len
    BYTE objectKeyLen = RB(data,off);
    if (objectKeyLen > 4) {
        throw std::bad_cast();
    }
    assert(len >= SIZE_T(1+objectKeyLen));
	
    //  Object key
	key = RDW( data, off );

	//printf( "[dsmcc::Object] Parsing object key: len=%08lx, key=%08lx\n", len, key );

    return off;
}

Object *Object::parseObject( ResourceManager *resMgr, Module *module, SSIZE_T &off ) {
    Object *obj;
	ObjectLocation loc;
    BYTE buf[BIOP_MAX_HEADER];

    SSIZE_T offset = 0;
    SIZE_T len = module->read( off, buf, BIOP_MAX_HEADER );
    if (len < BIOP_MAX_HEADER) {
        printf( "[dsmcc::Object] Warning: Not enough buffer: len=%ld, offset=%ld\n", len, offset );
        return NULL;
    }

    DWORD magic = RDW(buf,offset);
    if (magic != BIOP_MAGIC) {
        printf( "[dsmcc::Object] Warning: Bad magic number: magic=%x, found=%lx\n", BIOP_MAGIC, magic );
        return NULL;
    }

    WORD biopVersion = RW(buf,offset);
    if (biopVersion != BIOP_VER) {
        printf( "[dsmcc::Object] Warning: Invalid version\n" );
        return NULL;
    }

    BYTE byteOrder = RB(buf,offset);
    if (byteOrder) {
        printf( "[dsmcc::Object] Warning: Invalid byte order\n" );
        return NULL;
    }

    BYTE msgType = RB(buf,offset);
    if (msgType) {
        printf( "[dsmcc::Object] Warning: Invalid type\n" );
        return NULL;
    }

    //  Check BIOP message size
    DWORD msgSize = RDW(buf,offset);
    if (module->size()-off-offset < msgSize) {
        printf( "[dsmcc::Object] Warning: Not enough bytes for BIOP message: msgSize=%ld, rest%ld\n", msgSize, module->size()-off-offset );
        return NULL;
    }
    //  12 bytes parsed = BIOP_MIN_HEADER

    offset += objectKey( buf+offset, len-offset, loc.keyID );

    DWORD objectKindLen = RDW(buf,offset);
    if (objectKindLen != 4) {
        printf( "[dsmcc::Object] Warning: Invalid object kind\n" );
        return NULL;
    }

	//	Chapter 11: The downloadID field of the DownloadDataBlock messages shall have the same value
	//	as the carouselID field of the U-U Object Carousel
	loc.carouselID = module->downloadID();
	loc.moduleID   = module->id();

    DWORD objectKind = RDW(buf,offset);
    switch (objectKind) {
        case BIOP_OBJECT_FILE:
            obj = new File( loc );
            break;
        case BIOP_OBJECT_DIR:
        case BIOP_OBJECT_GW:
            obj = new Directory( loc );
            break;
		// case BIOP_OBJECT_STR:
		// 	break;
		case BIOP_OBJECT_STE:
			obj = new StreamEvent( loc );
			break;
        default:
            obj = NULL;
            printf( "[dsmcc::Object] Warning: BIOP object kind %lx not processed\n", objectKind );
    }

	WORD objectInfoLen = RW(buf,offset);
	
    //  Update offset (27 bytes parsed = BIOP_MAX_HEADER)
    off += offset;
    if (obj && !obj->parse( resMgr, module, off, objectInfoLen )) {
		delete obj;
		obj = NULL;
	}
	
    return obj;
}

Object::Object( const ObjectLocation &loc )
	: _loc( loc )
{
}

Object::~Object()
{
}

//  Getters
const std::string &Object::name() const {
    return _name;
}

void Object::name( const std::string &name ) {
    _name = name;
}

bool Object::operator==( const ObjectLocation &loc ) {
	return _loc == loc;
}

void Object::show() const {
	printf( "[dsmcc::Object]\t %s, type=%01x, name=%s\n",
		_loc.asString().c_str(), type(), _name.c_str() );
}

//	Parser
void Object::getEvents( std::vector<Event *> &/*events*/ ) {
}

void Object::freeResources( void ) {
}

void Object::onObject( std::list<Object *> & /*unprocessed*/ ) {
}

bool Object::parse( ResourceManager *resMgr, Module *module, SSIZE_T &off, WORD objectInfoLen ) {
	//	Need parse Object Info
	if (objectInfoLen && needObjectInfo()) {
		//	Parse ObjectInfo
		if (!parseObjectInfo( module, off, objectInfoLen )) {
			printf( "[biop::Object] Warning, Cannot parse biop::ObjectInfo\n" );
			return false;
		}
	}
	//	Update offset to skip objectInfo
	off += objectInfoLen;

	//	Read Service Context List Count
	BYTE serviceContextListCount;	
	if (!module->readB( off, serviceContextListCount )) {
		printf( "[biop::Object] Warning, Cannot read bytes to parse ServiceContextList\n" );
		return false;
	}

	//	Parse Service Context List
	for (BYTE context=0; context<serviceContextListCount; context++) {
		if (!parseContext( module, off )) {
			printf( "[biop::Object] Warning, Cannot parse Service Context List: context=%d", context );
			return false;
		}
	}

	//	Skip body len
	DWORD bodyLen;
	if (!module->readDW( off, bodyLen )) {
		printf( "[biop::Object] Warning, Cannot parse body len" );
		return false;
	}
	SSIZE_T old = off;
	off += bodyLen;
	
	//	Parse body
	return parseBody( resMgr, module, old, bodyLen );
}

bool Object::needObjectInfo( void ) const {
	return false;
}

bool Object::parseObjectInfo( Module */*module*/, SSIZE_T /*offset*/, WORD /*objectInfoLen*/ ) {
	return false;
}

bool Object::parseContext( Module */*module*/, SSIZE_T &/*offset*/ ) {
	return false;
}

//  Aux
std::string Object::getPath( Object *parent, const std::string &name ) {
	fs::path p;

    //  Make object name
    if (parent) {
        p = parent->name();
    }
    p /= name;
    return p.string();
}

}
}
}
