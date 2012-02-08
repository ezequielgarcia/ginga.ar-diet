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
#include "objectcarouseldemuxer.h"
#include "dsmccdemuxer.h"
#include "module.h"
#include "biop/ior.h"
#include "biop/object.h"
#include "../psi.h"
#include <util/mcr.h>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>


namespace tuner {
namespace dsmcc {

namespace fs = boost::filesystem;

ObjectCarouselDemuxer::ObjectCarouselDemuxer( ID tag, const std::string &rootPath, DSMCCDemuxer *demux )
{
	assert(demux);
	
	_tag   = tag;
	_demux = demux;
	_root  = NULL;
	_rootPath = rootPath;

	//	Setup DSMCCDemuxer
	dsmcc::DSMCCDemuxer::DSICallback onParsed = boost::bind( &ObjectCarouselDemuxer::onDSI,this,_1 );
	_demux->onDSI( onParsed );
}

ObjectCarouselDemuxer::~ObjectCarouselDemuxer( void )
{
	cleanup();
	cleanupRoot();
}

DSMCCDemuxer *ObjectCarouselDemuxer::demux( void ) const {
	return _demux;
}

//	Notifications
void ObjectCarouselDemuxer::onMounted( const OnMounted &callback ) {
	_onMounted = callback;
}

//	Generic aux
void ObjectCarouselDemuxer::cleanupRoot( void )  {
	if (fs::exists( _rootPath )) {
		fs::remove_all( _rootPath );
	}
}
	
void ObjectCarouselDemuxer::cleanup( bool freeResources/*=true*/ )  {
	//	Cleanup mounted carousel
	if (_root) {
		if (freeResources) {
			_root->freeResources();
		}
		delete _root;
		_root = NULL;
	}

	//	Delete temporary objects
	BOOST_FOREACH( dsmcc::biop::Object *obj, _objects ) {
		if (freeResources) {
			obj->freeResources();
		}
		delete obj;
	}
	_objects.clear();
}

//	Tables
void ObjectCarouselDemuxer::onDSI( dsmcc::DSI *dsi ) {
	dsi->show();

    //  Check if object carousel is present (Chapter 7 - ISO DSM-CC)
    int len = dsi->privateData().length();	
    if (len > 0) {
        SSIZE_T offset = 0;
        BYTE *payload = (BYTE *)dsi->privateData().buffer();

        dsmcc::biop::Ior ior;
        offset += ior.parse( payload, len );
        if (ior.isValid()) {
			//	Identify root directory
			_rootLocation = ior.location();
			printf( "[ObjectCarouselDemuxer] Root location: %s\n", _rootLocation.asString().c_str() );

			//	Check tag is correct
			if (_tag == ior.associationTag()) {
				//	Callback to execute in filter context
				dsmcc::DSMCCDemuxer::DIICallback onParsed = boost::bind( &ObjectCarouselDemuxer::onDII,this,_1);
				_demux->onDII( onParsed );
			}
			else {
				printf( "[ObjectCarouselDemuxer] Tag incorrect or invalid. Ignoring DSI: tag=%x\n", ior.associationTag() );
			}
        }
    }	
	
	delete dsi;
}

void ObjectCarouselDemuxer::onDII( dsmcc::DII *dii ) {
	dii->show();
	
	//	Callback to execute in filter context
	dsmcc::DSMCCDemuxer::ModuleCallback onModule = boost::bind( &ObjectCarouselDemuxer::onModule,this,_1);
	_demux->onModule( onModule );
	
	//	Filter all modules present in the DII
	_demux->filterModules( dii->downloadID(), dii->modules(), dii->blockSize() );

	//	TODO: Object carousel isn't using DownloadID???

	delete dii;
}

void ObjectCarouselDemuxer::onModule( dsmcc::Module *module ) {
    //  Goto begin of module
    SSIZE_T offset=0;
	dsmcc::biop::ModuleInfo biopInfo;

	//	Parse module info as biop::ModuleInfo
	biop::parseModuleInfo( module->info(), biopInfo );

	//	Parse user data as module descriptors
	if (biopInfo.userInfo.length()) {
		desc::MapOfDescriptors descs;
		dsmcc::module::parseDescriptors( biopInfo.userInfo, descs );

		//	Check if module is compressed
		desc::MapOfDescriptors::const_iterator it=descs.find( MODULE_DESC_COMPRESSED );
		bool isCompressed = (it != descs.end());
		if (!isCompressed) {
			it=descs.find( MODULE_DESC_COMPRESSION );
			isCompressed = (it != descs.end());		
		}

		if (isCompressed) {
			const dsmcc::module::CompressionTypeDescriptor &compressDesc = (*it).second.get<dsmcc::module::CompressionTypeDescriptor>();
			module->inflate( compressDesc.type, compressDesc.originalSize );
		}
	}

    while (offset < module->size()) {
        //  parse BIOP object
        dsmcc::biop::Object *object=dsmcc::biop::Object::parseObject( _demux->resourceMgr(), module, offset );
        if (object) {
            //  Update carousel with object parsed
            updateCarousel( object );
        }
        else {
			//	Object not parsed, so carousel cannot be mounted, so abort ...
            printf( "[ObjectCarouselDemuxer] Warning, BIOP object not parsed: moduleID=%d, moduleSize=%ld, offset=%ld\n",
				module->id(), module->size(), offset );
			break;
        }
    }

    //printf( "[ObjectCarouselDemuxer] Module %d parsed complete!\n", module->id() );
	
	delete module;
}

void ObjectCarouselDemuxer::updateCarousel( dsmcc::biop::Object *object ) {
	//object->show();
	
	//	printf( "[ObjectCarouselDemuxer] root=%p\n", _root );
	if (*object == _rootLocation) {
		cleanupRoot();
        object->process( NULL, _rootPath, _objects );
        _root = object;
    }
    else {
        _objects.push_back( object );
        if (_root) {
            _root->onObject( _objects );
        }
    }

    if (_root && _root->isComplete() && !_onMounted.empty()) {
		mountCarousel();
    }
}

void ObjectCarouselDemuxer::mountCarousel( void ) {
	printf( "[ObjectCarouselDemuxer] Mounted called\n" );

	//	Notify to dependent
	dsmcc::Events events;
	_root->getEvents( events );
	_onMounted( _rootPath, events );

	//	Cleanup, so if a update arrive, process all object again!
	cleanup( false );
}

}
}

