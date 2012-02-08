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
#pragma once

#include "../applicationid.h"
#include <util/keydefs.h>
#include <boost/function.hpp>
#include <vector>

namespace tuner {

class ServiceManager;

namespace app {

class ApplicationExtension;
class Application;
class Spawner;

class ApplicationController {
public:
	ApplicationController( ServiceManager *mgr );
	virtual ~ApplicationController( void );

	//	Create and setup extension
	void setup( void );
	ApplicationExtension *extension();

	//	Start/stop/scan (controller thread)
	typedef boost::function<void (void)> EndScanApplicationCallback;
	void scanApplications( const std::string path, int maxDepth=2, const EndScanApplicationCallback &callback=EndScanApplicationCallback() );

	void start( const ApplicationID &id );
	void stop( const ApplicationID &id );
	void stopAll();
	void status( const ApplicationID &id, status::type st );
	void enableSpawner( bool mustEnable );

	//	Application notifications (service thread)	
	virtual void onApplicationAdded( Application */*app*/ ) {}	
	virtual void onApplicationRemoved( Application */*app*/ ) {}
	virtual void onApplicationChanged( Application */*app*/ ) {}

	//	Application operations (service thread)
	virtual void reserveKeys( const std::vector<util::key::type> &keys )=0;
	virtual void resizeVideo( float x, float y, float w, float h )=0;
	virtual void speechText( std::string speech, bool queue, unsigned int delay )=0;
	virtual void stopSpeechText(void)=0;
	virtual void audioOn(void)=0;
	virtual void audioOff(void)=0;
	virtual video::mode::type setupVideoResolution( const std::vector<video::mode::type> &modes );

	//	Spawner (service thread)
	virtual Spawner *spawner()=0;

protected:
	virtual void setupFactories( ApplicationExtension *ext )=0;
	void doEnable( bool mustEnable );

private:
	ApplicationExtension *_extension;
	ServiceManager *_mgr;	
};
	
}
}

