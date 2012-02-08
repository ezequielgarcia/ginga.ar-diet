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
#include "types.h"
#include <boost/foreach.hpp>
#include <stdio.h>

namespace tuner {
namespace ait {

void show( const ApplicationDescriptor &app ) {
	printf( "[ait::Descriptor] srvBoundFlag=%d, visibility=%x, priority=%d\n",
		app.serviceBoundFlag, app.visibility, app.priority );

	BOOST_FOREACH( const ApplicationProfile &profile, app.profiles ) {
		printf( "[ait::Descriptor]\t profile=%04x, version=(%02x,%02x,%02x)\n",
			profile.profile, profile.major, profile.minor, profile.micro );
	}

	BOOST_FOREACH( BYTE label, app.transports ) {
		printf( "[ait::Descriptor]\t Transport label: %02x\n", label );
	}
}

void show( const ExternalApplications &apps ) {
	BOOST_FOREACH( const ExternalApplication &app, apps ) {
		printf( "[ait::ExternalApplication] orgID=%08lx, appID=%04x, priority=%02x\n", app.orgID, app.id, app.priority );
	}
}

void show( const TransportDescriptor &transports ) {
	printf( "[ait::Transports] size=%d\n", transports.size() );
	BOOST_FOREACH( const TransportProtocolDescriptor &transport, transports ) {
		printf( "[ait::TPDescriptor] protocolID=%04x, label=%x\n",
			transport.protocolID, transport.label );

		switch (transport.protocolID) {
			case TP_OBJECT_CAROUSEL:
			case TP_DATA_CAROUSEL:
			{
				const DSMCCTransportProtocolDescriptor dsmcc = transport.info.get<DSMCCTransportProtocolDescriptor>();
				if (dsmcc.remote) {
					printf( "[ait::TPDesc]\t DSMCC: remote=%02x, networkID=%04x, tsID=%04x, serviceID=%04x, tag=%02x\n",
						dsmcc.remote, dsmcc.networkID, dsmcc.tsID, dsmcc.serviceID, dsmcc.componentTag );
				}
				else {
					printf( "[ait::TPDesc]\t DSMCC: remote=%02x, tag=%02x\n",
						dsmcc.remote, dsmcc.componentTag );
				}
				break;
			}
			default:
				break;
		};
	}
}

void show( const ApplicationNameDescriptor &desc ) {
	BOOST_FOREACH( const ApplicationName &app, desc ) {
		printf( "[ait::NameDescriptor] Name=%s, language=%s\n",
			app.name.c_str(), app.language.c_str() );
	}
}

void show( const GingaApplicationDescriptor &desc ) {
	BOOST_FOREACH( const std::string &param, desc ) {
		printf( "[ait::GingaApp] param=%s\n", param.c_str() );
	}
}

void show( const GingaApplicationLocationDescriptor &desc ) {
	printf( "[ait::GingaAppLocation] base=%s, classPath=%s, initial=%s\n",
		desc.base.c_str(), desc.classPath.c_str(), desc.initialClass.c_str() );
}

void show( const Application &app ) {
	printf( "[ait::Application] Application: organization=%lx, appID=%x, ctrlCode=%x, descriptors=",
		app.orgID, app.id, app.ctrlCode );
	desc::show( app.descriptors );
}

}
}
