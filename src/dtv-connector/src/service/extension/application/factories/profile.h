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
#include "../../../../demuxer/psi/application/types.h"


namespace tuner {

class Service;

namespace app {

class ApplicationExtension;
class Application;

class Profile {
public:
	Profile( ApplicationExtension *ext );
	virtual ~Profile( void );

	virtual bool isSupported( ID appType ) const=0;
	virtual bool isProfileSupported( const ait::ApplicationProfile &appProfile ) const=0;

	virtual void serviceStarted( Service *srv );
	virtual void serviceStopped( Service *srv );

	Application *create( const ApplicationID &appID, ID &serviceID, const ait::Application &app, const desc::MapOfDescriptors &aitDescs );
	virtual Application *create( const ApplicationID &appID, const std::string &file ) const=0;

protected:
	//	Aux creation
	virtual Application *create( const ApplicationID &appID ) const=0;
	
	//	Aux descriptors
	virtual void process( Application *app );
	bool processApplicationDescriptor( bool &serviceBound, BYTE &visibility, BYTE &priority, std::vector<BYTE> &transports );
	bool processApplicationNameDescriptor( std::string &name, std::string &language );
	bool processTransportProtocol( BYTE &tag, std::vector<BYTE> &transports );
	void processIcons( Application *app );	
	void processGraphicsConstraints( Application *app );
	bool findDesc( BYTE tag, desc::MapOfDescriptors::const_iterator &it ) const {
		return desc::findDesc( *_aitDescs, *_appDescs, tag, it );	
	}

	//	Getters
	ApplicationExtension *extension() const;
	
private:
	const desc::MapOfDescriptors *_aitDescs;
	const desc::MapOfDescriptors *_appDescs;
	ApplicationExtension *_extension;
};

}
}
