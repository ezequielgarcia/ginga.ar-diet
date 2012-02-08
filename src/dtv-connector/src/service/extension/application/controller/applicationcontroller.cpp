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
#include "applicationcontroller.h"
#include "spawner.h"
#include "../applicationextension.h"
#include "../../../servicemanager.h"
#include "../../../../provider/taskqueue.h"

namespace tuner {
namespace app {

ApplicationController::ApplicationController( ServiceManager *mgr )
{
	_mgr = mgr;
	_extension = NULL;
}

ApplicationController::~ApplicationController( void )
{
}

//	Create and setup extension
void ApplicationController::setup( void ) {
	//	Create extension
	_extension = new ApplicationExtension( _mgr, this );

	//	Add application factories
	setupFactories( _extension );
}

ApplicationExtension *ApplicationController::extension() {
	assert(_extension);
	return _extension;
}

//	Start/stop/scan
void ApplicationController::scanApplications( const std::string path, int maxDepth/*=2*/, const EndScanApplicationCallback &callback ) {
	//	Enqueue into provider thread
	_mgr->enqueue( new BasicTask( boost::bind(&ApplicationExtension::scan,_extension,path,maxDepth,callback) ) );
}

void ApplicationController::start( const ApplicationID &id ) {
	//	Enqueue into provider thread
	_mgr->enqueue( new BasicTask( boost::bind(&ApplicationExtension::start,_extension,id) ) );
}

void ApplicationController::stop( const ApplicationID &id ) {
	//	Enqueue into provider thread
	_mgr->enqueue( new BasicTask( boost::bind(&ApplicationExtension::stop,_extension,id,false) ) );
}

void ApplicationController::stopAll( void ) {
	//	Enqueue into provider thread
	_mgr->enqueue( new BasicTask( boost::bind(&ApplicationExtension::stopAll,_extension) ) );
}

void ApplicationController::status( const ApplicationID &id, status::type st ) {
	//	Enqueue into provider thread
	_mgr->enqueue( new BasicTask( boost::bind(&ApplicationExtension::status,_extension,id,st) ) );
}

void ApplicationController::enableSpawner( bool mustEnable ) {
	//	Enqueue into provider thread
	_mgr->enqueue( new BasicTask( boost::bind(&ApplicationController::doEnable,this,mustEnable) ) );
}

void ApplicationController::doEnable( bool mustEnable ) {
	spawner()->enable( mustEnable );
}

video::mode::type ApplicationController::setupVideoResolution( const std::vector<video::mode::type> &/*modes*/ ) {
	return video::mode::sd;
}
	
}
}

