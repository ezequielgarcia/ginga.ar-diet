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

#include "profile.h"
#include "../application.h"
#include "../../../../demuxer/psi/application/types.h"
#include <boost/foreach.hpp>

namespace tuner {
namespace app {

Profile::Profile( ApplicationExtension *ext )
{
	_extension = ext;
}

Profile::~Profile( void )
{
}

void Profile::serviceStarted( Service */*srv*/ ) {
}

void Profile::serviceStopped( Service */*srv*/ ) {
}

ApplicationExtension *Profile::extension() const {
	return _extension;
}

Application *Profile::create( const ApplicationID &appID, ID &serviceID, const ait::Application &app, const desc::MapOfDescriptors &aitDescs ) {
	Application *iApp=NULL;
	std::string name, language;
	bool serviceBound;
	BYTE visibility, priority, tag;
	std::vector<BYTE> transports;

	_aitDescs = &aitDescs;
	_appDescs = &app.descriptors;

	//	Process AIT application descriptor
	if (processApplicationDescriptor( serviceBound, visibility, priority, transports )) {
		//	Process AIT application name descriptor	
		if (processApplicationNameDescriptor( name, language )) {
			//	Process AIT transport descriptor
			if (processTransportProtocol( tag, transports )) {
				//	Try create application
				iApp = create( appID );
				if (iApp) {
					//	Setup AIT Application
					iApp->autoStart( app.ctrlCode == ait::control_code::autostart );
					iApp->autoDownload( app.ctrlCode == ait::control_code::prefetch );
					iApp->componentTag( tag );

					if (serviceBound) {
						iApp->service( serviceID );
					}
					else {
						serviceID = iApp->service();
					}
		
					app::visibility::type vis = visibility::none;
					if (visibility == ait::visibility::both) {
						vis = visibility::both;
					}
					else if (visibility == ait::visibility::api) {
						vis = visibility::api;
					}
		
					iApp->visibility( vis );
					iApp->priority( priority );
					iApp->name( name, language );
					iApp->readOnly( false );

					//	Process other descriptors
					process( iApp );
				}
			}
		}
	}
	
	_aitDescs = NULL;
	_appDescs = NULL;
	return iApp;
}

//	Process AIT descriptors
void Profile::process( Application *app ) {
	//	Process other descriptors
	processGraphicsConstraints( app );
	processIcons( app );
}

bool Profile::processApplicationDescriptor( bool &serviceBound, BYTE &visibility, BYTE &priority, std::vector<BYTE> &transports ) {
	desc::MapOfDescriptors::const_iterator it;

	//	Check application descriptor
	if (!findDesc( AIT_APP_DESC, it )) {
		printf( "[Profile] Warning, Application Descriptor not present\n" );
		return false;
	}
	const ait::ApplicationDescriptor &appDesc = (*it).second.get<ait::ApplicationDescriptor>();

	//	Check profile: ETSI TS 102 812 v1.2.1
	//		The MHP terminal shall only launch applications if the following
	//		expression is true for one of the singalled profiles
	bool appSupported=false;
	BOOST_FOREACH( const ait::ApplicationProfile &appProfile, appDesc.profiles ) {
		//	If profile is supported
		if (isProfileSupported( appProfile )) {
			appSupported=true;
			break;
		}
	}
	if (!appSupported) {
		printf( "[Profile] Warning, Application profile not supported\n" );
		return false;
	}

	serviceBound = appDesc.serviceBoundFlag;
	visibility   = appDesc.visibility;
	priority     = appDesc.priority;
	transports   = appDesc.transports;

	return true;
}

bool Profile::processApplicationNameDescriptor( std::string &name, std::string &language ) {
	desc::MapOfDescriptors::const_iterator it;
	
	//	Check application name descriptor
	if (!findDesc( AIT_APP_NAME_DESC, it )) {
		printf( "[Profile] Warning, ignoring application becouse Application Name Descriptor not present\n" );
		return false;
	}
	const ait::ApplicationNameDescriptor &appNameDesc = (*it).second.get<ait::ApplicationNameDescriptor>();

	//	Check if application name is valid
	if (!appNameDesc.size()) {
		printf( "[Profile] Warning, ignoring application becouse Application Name is invalid\n" );		
		return false;
	}

	//	Get first name
	const ait::ApplicationName &appName = appNameDesc[0];
	name     = appName.name;
	language = appName.language;
	return true;
}

struct TransportFinder {
	TransportFinder( BYTE label ) : _label(label) {}
	bool operator()( const ait::TransportProtocolDescriptor &desc ) const {
		return desc.label == _label;
	}
	BYTE _label;
};

bool Profile::processTransportProtocol( BYTE &tag, std::vector<BYTE> &transports ) {
	desc::MapOfDescriptors::const_iterator it;
	
	//	Check tranport protocol descriptor present
	if (!findDesc( AIT_TRANSPORT_PROTOCOL_DESC, it )) {
		printf( "[Profile] Warning, ignoring application becouse Transport Protocol Descriptor not present\n" );
		return false;
	}
	const ait::TransportDescriptor &transportsDesc = (*it).second.get<ait::TransportDescriptor>();

	//	Try find a transport by label
	ait::TransportDescriptor::const_iterator itTrans=transportsDesc.end();
	BOOST_FOREACH( BYTE label, transports ) {
		itTrans=std::find_if( transportsDesc.begin(), transportsDesc.end(), TransportFinder(label) );
		if (itTrans != transportsDesc.end()) {
			break;
		}
	}
	if (itTrans == transportsDesc.end()) {
		printf( "[Profile] Warning, ignoring application becouse transport label not found!\n" );
		return false;
	}
	const ait::TransportProtocolDescriptor &transportDesc = (*itTrans);
	
	//	Support is only for object carousel transport, ignore others
	//	TODO: Add more transport protocols support
	if (transportDesc.protocolID != TP_OBJECT_CAROUSEL) {
		printf( "[Profile] Warning, ignoring application becouse transport not supported\n" );
		return false;
	}

	//	Check object carousel tranport
	const ait::DSMCCTransportProtocolDescriptor &dsmccDesc = transportDesc.info.get<ait::DSMCCTransportProtocolDescriptor>();
	if (dsmccDesc.remote) {
		printf( "[Profile] Warning, ignoring application becouse object carousel transport is remote\n" );
		return false;
	}
	tag = dsmccDesc.componentTag;
	
	return true;
}

void Profile::processIcons( Application *app ) {
	desc::MapOfDescriptors::const_iterator it;

	printf( "[Profile] Process icons\n" );
	
	//	Check descriptor present
	if (findDesc( AIT_APP_ICONS_DESC, it )) {
		const ait::IconsDescriptor &desc = (*it).second.get<ait::IconsDescriptor>();
		app->addIcons( desc );		
	}
}

void Profile::processGraphicsConstraints( Application *app ) {
	desc::MapOfDescriptors::const_iterator it;

	printf( "[Profile] Process graphics constraints\n" );
	
	//	Check descriptor present
	if (findDesc( AIT_GRAPHICS_CONSTRAINTS_DESC, it )) {
		const ait::GraphicsConstraintsDescriptor &desc = (*it).second.get<ait::GraphicsConstraintsDescriptor>();
		
		BOOST_FOREACH( util::BYTE mode, desc.supportedModes ) {
			video::mode::type videoMode;

			//	Set mode from ETSI TS 102 809 v1.1.1 (2010-01)
			if (mode == ait::graphics_mode::mode_960_540 ||
				mode > ait::graphics_mode::mode_1920_1080)
			{
				videoMode = video::mode::sd;
			}
			else {
				videoMode = (video::mode::type)mode;
			}
					
			app->addVideoMode( videoMode );
		}
	}
}

}
}
