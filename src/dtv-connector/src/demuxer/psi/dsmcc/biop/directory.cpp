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
#include "directory.h"
#include "ior.h"
#include "../module.h"
#include <util/mcr.h>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

namespace tuner {
namespace dsmcc {
namespace biop {

namespace fs = boost::filesystem;

Directory::Directory( const ObjectLocation &loc )
: Object( loc )
{
}

Directory::~Directory()
{
	CLEAN_ALL(Object *,_subs);
}

bool Directory::parseBody( ResourceManager */*resMgr*/, Module *module, SSIZE_T off, DWORD /*bodyLen*/ ) {
    char fileName[255];
    BYTE buf[255];
    Ior ior;
	BindType bind;

	//	TODO: Read all body

	WORD bindingsCount;
	if (!module->readW( off, bindingsCount )) {
		printf( "[dsmcc::Directory] Warning: Cannot read bindings count\n" );
		return false;
	}
	
    for (int binding=0; binding<bindingsCount; binding++) {
        BYTE nameComponentCount;
		if (!module->readB( off, nameComponentCount )) {
			printf( "[dsmcc::Directory] Warning: Cannot read name component count: bind=%d\n", binding );
			return false;
		}

        if (nameComponentCount != 1) {
            printf( "[dsmcc::Directory] Warning: Name component count invalid %x\n", nameComponentCount );
            return false;
        }

        BYTE idLen;
		if (!module->readB( off, idLen )) {
			printf( "[dsmcc::Directory] Warning: Cannot read ID len\n" );
			return false;
		}		

		//	Read filename
        if (module->read( off, (BYTE *)fileName, idLen ) != idLen) {
			printf( "[dsmcc::Directory] Warning: Cannot read file name\n" );
			return false;			
		}
        off += idLen;

        BYTE kindLen;
		if (!module->readB( off, kindLen )) {
			printf( "[dsmcc::Directory] Warning: Cannot read kind len\n" );
			return false;
		}
        off += kindLen;

        BYTE bindType;
        if (!module->readB( off, bindType )) {
			printf( "[dsmcc::Directory] Warning: Cannot read bind type\n" );
			return false;
		}

		//	Read IOR
        module->read( off, buf, 255 );
        off += ior.parse( buf, 255 );
		
		//	Skip objectInfoLen2
		WORD objectInfoLen2;
		if (!module->readW( off, objectInfoLen2 )) {
			printf( "[dsmcc::Directory] Warning: Cannot read ObjectInfoLen2\n" );
			return false;
		}
        off += objectInfoLen2;

		//	Fill data
		bind.obj  = ior.location();
		bind.name = fileName;
		bind.type = bindType;
        _binding.push_back( bind );

       // printf( "\tentry: filename=%s, bindindType=%x, info2len=%d\n",
       //     fileName, bindType, objectInfoLen2 );
    }
	return true;
}

Object *Directory::find( BindType &bind, std::list<Object *> &objects ) {
    Object *tmp=NULL;

    BOOST_FOREACH( Object *obj, objects ) {
        //  Check for key and type of object
		if (*obj == bind.obj && obj->type() == bind.type) {
            tmp = obj;
            break;
        }
    }
    return tmp;
}

void Directory::onObject( std::list<Object *> &unprocessed ) {
    processBinding( unprocessed );
    BOOST_FOREACH( Object *obj, _subs ) {
        obj->onObject( unprocessed );
    }
}

void Directory::getEvents( std::vector<dsmcc::Event *> &events ) {
    BOOST_FOREACH( Object *obj, _subs ) {
        obj->getEvents( events );
    }	
}

void Directory::freeResources( void ) {
	//	Free resources on childs
    BOOST_FOREACH( Object *obj, _subs ) {
        obj->freeResources();
    }
	//	Clean directory
	if (fs::exists( name() )) {
		fs::remove_all( name() );
	}
}

void Directory::processBinding( std::list<Object *> &unprocessed ) {
    Object *obj;
    std::list<BindType>::iterator it;

    it=_binding.begin();
    while (it!=_binding.end()) {
        //  find key in objects
        obj = find( (*it), unprocessed );
        if (obj) {
            //  Process object
            obj->process( this, (*it).name, unprocessed );

            //  Remove object from list of objects not processed
            unprocessed.remove( obj );

            //  Remove bind from list of binding not processed
            it = _binding.erase( it );

            //  Add sub-object reference
            _subs.push_back( obj );
        }
        else {
            it++;
        }
    }
}

void Directory::process( Object *parent, const std::string &objName, std::list<Object *> &unprocessed ) {
    name( getPath(parent,objName) );

    //  Create directory
    if (!fs::create_directory( name() )) {
        printf( "[dsmcc::Directory] Cannot create directory %s\n", name().c_str() );
        throw;
    }

    //  process bindings
    processBinding( unprocessed );

    printf( "[dsmcc::Directory]   Object name %s processed\n", name().c_str() );
}

BYTE Directory::type() const {
    return 2;
}

void Directory::show() const {
	Object::show();
	BOOST_FOREACH( const BindType &bind, _binding ) {
		printf( "[dsmcc::Object]\t %s, name=%s, type=%02x\n",
			bind.obj.asString().c_str(), bind.name.c_str(), bind.type );
	}
}

bool Directory::isComplete() const {
    bool result=false;

	// printf( "[dsmcc::Directory] IsComplete: name=%s, binding=%d\n", name().c_str(), _binding.size() );
	// BOOST_FOREACH(const BindType &bind, _binding) {
	// 	printf( "[dsmcc::Directory]\t Incomplete binding: carouselID=%lx, key=%lx, name=%s, type=%x\n",
	// 		bind.obj.carouselID, bind.obj.keyID, bind.name.c_str(), bind.type );
	// }

	//	Binding (list of sub objects) complete? 
    if (_binding.empty()) {
		if (!_subs.size()) {
			//	Directory empty
			result=true;
		}
		else {
			//	Check if all subtree is complete ...
			BOOST_FOREACH( Object *obj, _subs ) {
				result = obj->isComplete();
				if (!result) {
					break;
				}
			}
		}
    }
    return result;
}

}
}
}
