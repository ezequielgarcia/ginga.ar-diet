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

#include "servicemanager.h"
#include "extension/extension.h"
#include "../resourcemanager.h"
#include "../provider/provider.h"
#include <util/mcr.h>
#include <assert.h>

namespace tuner {

ServiceManager::ServiceManager( ResourceManager *resMgr )
{
	assert(resMgr);
	_resMgr = resMgr;
	_enableExtensions = true;
}

ServiceManager::~ServiceManager( void )
{
	//	Destroy extensions
	CLEAN_ALL(Extension *, _extensions);
	
	delete _resMgr;
}

//	Extensions
void ServiceManager::addExtension( Extension *extension ) {
	_extensions.push_back( extension );
}

void ServiceManager::exStart( bool withExtensions ) {
	//	Initialize
	_enableExtensions = withExtensions;

	if (_enableExtensions) {
		BOOST_FOREACH(Extension *ext, _extensions) {
			ext->start();
		}
	}
}

void ServiceManager::exStop( void ) {
	if (_enableExtensions) {
		BOOST_FOREACH(Extension *ext, _extensions) {
			ext->stop();
		}
	}
}

void ServiceManager::exReady( bool ready ) {
	if (_enableExtensions) {
		BOOST_FOREACH(Extension *ext, _extensions) {
			ext->onReady( ready );
		}
	}
}

void ServiceManager::exServiceStarted( Service *srv ) {
	if (_enableExtensions) {
		BOOST_FOREACH(Extension *ext, _extensions) {
			ext->onServiceStarted( srv );		
		}
	}
}

void ServiceManager::exServiceStopped( Service *srv ) {
	if (_enableExtensions) {
		BOOST_FOREACH(Extension *ext, _extensions) {
			ext->onServiceStopped( srv );		
		}
	}
}

void ServiceManager::exServiceReady( Service *srv ) {
	if (_enableExtensions) {
		BOOST_FOREACH(Extension *ext, _extensions) {
			ext->onServiceReady( srv );		
		}
	}
}

void ServiceManager::exServiceExpired( Service *srv ) {
	if (_enableExtensions) {
		BOOST_FOREACH(Extension *ext, _extensions) {
			ext->onServiceExpired( srv );		
		}
	}
}

//	Getters
ResourceManager *ServiceManager::resMgr() {
	return _resMgr;
}

}
