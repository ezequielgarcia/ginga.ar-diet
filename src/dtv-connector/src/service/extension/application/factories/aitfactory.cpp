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
#include "aitfactory.h"
#include "profile.h"
#include "../application.h"
#include "../applicationextension.h"
#include "../../../service.h"
#include "../../../servicemanager.h"
#include "../../../../demuxer/psi/application/aitdemuxer.h"
#include "../../../../demuxer/psi/psi.h"
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <stdio.h>

namespace tuner {
namespace app {

AitFactory::AitFactory( ApplicationExtension *ext )
	: ApplicationFactory( ext )
{
}

AitFactory::~AitFactory( void )
{
	BOOST_FOREACH( Profile *profile, _profiles) {
		delete profile;
	}
	_profiles.clear();
}

void AitFactory::serviceStarted( Service *srv ) {
	BOOST_FOREACH( const Pmt::ElementaryInfo &elem, srv->elements() ) {
		//	Attach on AIT streams
		if (elem.streamType == PSI_ST_TYPE_AIT) {
			//	Create AIT demuxer
			TSSectionDemuxer *demux = createDemuxer<AITDemuxer,Ait>(
				elem.pid,
				boost::bind(&AitFactory::onAit,this,_1,srv->id()),
				boost::bind(&AitFactory::onExpired,this,_1) );

			//	Start filter
			srvMgr()->startFilter( demux );
		}
	}

	BOOST_FOREACH( Profile *profile, _profiles ) {
		profile->serviceStarted( srv );
	}
}

void AitFactory::serviceStopped( Service *srv ) {
	BOOST_FOREACH( Profile *profile, _profiles ) {
		profile->serviceStopped( srv );
	}
	
	BOOST_FOREACH( const Pmt::ElementaryInfo &elem, srv->elements() ) {
		//	Dettach on AIT streams
		if (elem.streamType == PSI_ST_TYPE_AIT) {
			//	Stop filter
			srvMgr()->stopFilter( elem.pid );
		}
	}

	//	Clean all applications bound to service to stop 
	SignaledApplications::iterator it=_apps.begin();
	while (it != _apps.end()) {
		if ((*it).srvID == srv->id()) {
			it = _apps.erase( it );
		}
		else {
			it++;
		}
	}
}

void AitFactory::onAit( Ait *ait, ID serviceID ) {
	ait->show();

	//	If type of application is supported ...
	Profile *profile = findProfile( ait->appType() );
	if (profile) {
		std::vector<Application *> toAdd;
		SignaledApplications apps;
		std::vector<ApplicationID> wildcards;
		bool insert;
		ID curServiceID;

		//	Process External Application authorisation descriptor
		processExternals( ait->descriptors() );

		//	Remove not signaled applications and process modified ctrl code
		BOOST_FOREACH( const ait::Application &app, ait->applications() ) {
			//	Cases:
			//		a) curApp is present in old AIT
			//			i)  Ctrl code changed           -> Process ctrl code
			//			ii) Ctrl code no changed        -> Do nothing
			//		b) curApp is not present in new AIT -> Add application
			//		c) Resulting apps                   -> Remove application

			//	Create applicationID
			ApplicationID curApp = ApplicationID( app.orgID, app.id );
			insert=false;

			//	Check if app is a wilcard, if so, only process ctrl code!
			if (curApp.isWildcard()) {
				wildcards.push_back( curApp );
				processCtrlCode( curApp, app.ctrlCode );
			}
			else {
				//	Try find in old signaled applications
				SignaledApplications::iterator it=findApp( curApp );
				if (it != _apps.end()) {
					//	Add to new list of applications
					insert=true;
					curServiceID = (*it).srvID;
				
					//	Check if ctrl code was modified
					if (app.ctrlCode != (*it).ctrlCode) {
						//	a.ii) process ctrl code
						processCtrlCode( curApp, app.ctrlCode );
					}

					//	Remove from old list of applications
					_apps.erase( it );
				}
				else {
					//	b) Create application and if ok, add to new list of applications
					curServiceID = serviceID;
					Application *tmpApp = profile->create( curApp, curServiceID, app, ait->descriptors() );
					if (tmpApp) {
						toAdd.push_back( tmpApp );
						insert=true;
					}
				}
			}

			//	Add to current applications
			if (insert) {
				SignaledApplication tmp;
				tmp.app      = curApp;
				tmp.ctrlCode = app.ctrlCode;
				tmp.srvID    = curServiceID;
				apps.push_back( tmp );
			}
		}

		//	c) Process applications not signaled
		processNonSignaled( wildcards );

		//	Assign new applications
		std::copy( apps.begin(), apps.end(), std::back_inserter(_apps) );

		//	Add new applications to extension
		BOOST_FOREACH( Application *nApp, toAdd ) {
			extension()->add( nApp );
		}
	}
	else {
		printf( "[AitFactory] Warning, interactive application not supported: type=%04x\n", ait->appType() );
	}

	delete ait;	
}

void AitFactory::onExpired( ID /*pid*/ ) {
	printf( "[AitFactory] AIT expired!\n" );
}

//	Aux applications
struct AppFinder {
	AppFinder( const ApplicationID &id ) : _id(id) {}
	bool operator()( const AitFactory::SignaledApplication &item ) const {
		return item.app == _id;
	}
	const ApplicationID &_id;
};

AitFactory::SignaledApplications::iterator AitFactory::findApp( const ApplicationID &appID ) {
	return std::find_if( _apps.begin(), _apps.end(), AppFinder(appID) );
}

struct ExternalAppFinder {
	ExternalAppFinder( const ApplicationID &id ) : _id(id) {}
	bool operator()( const ait::ExternalApplication &item ) const {
		//	Construct appID
		ApplicationID appID(item.orgID,item.id);
		return appID == _id;
	}
	const ApplicationID &_id;
};

void AitFactory::processNonSignaled( const std::vector<ApplicationID> &wildcards ) {
	printf( "[AitFactory] Processing not signaled applications: old=%d, externals=%d, wildcards=%d\n",
		_apps.size(), _externals.size(), wildcards.size() );

	SignaledApplications::iterator it=_apps.begin();
	while (it != _apps.end()) {
		const ApplicationID &appID = (*it).app;
		bool remove=false;
		
		//	Check if appID is on the External Application Authorisation
		ait::ExternalApplications::const_iterator itExternals=std::find_if( _externals.begin(), _externals.end(), ExternalAppFinder(appID) );
		if (itExternals == _externals.end()) {
			//	Check if referenced in a wilcards
			std::vector<ApplicationID>::const_iterator itWildcards=std::find( wildcards.begin(), wildcards.end(), appID );
			remove=(itWildcards == wildcards.end());
		}
		else {
			//	Change priority
			extension()->changePriority( appID, (*itExternals).priority );
		}

		if (remove) {
			printf( "[AitFactory] Remove application: appID=%s\n", appID.asString().c_str() );
			extension()->remove( appID );
			it = _apps.erase( it );
		}
		else {
			it++;
		}
	}
}

void AitFactory::processCtrlCode( const ApplicationID &appID, BYTE ctrlCode ) {
	printf( "[AitFactory] Processing ctrl code: appID=%s, code=%x\n",
		appID.asString().c_str(), ctrlCode );
	
	switch (ctrlCode) {
		case ait::control_code::autostart:
			extension()->start( appID );
			break;
		case ait::control_code::present:
			//	Do nothing
			break;
		case ait::control_code::prefetch:
			extension()->download( appID );
			break;
		case ait::control_code::destroy:
			extension()->stop( appID );
			break;
		case ait::control_code::kill:
			extension()->stop( appID, true );
			break;
		default:
			printf( "[AitFactory] Warning, AIT ctrlcode not supported!\n" );
			break;
	};
}

void AitFactory::processExternals( const desc::MapOfDescriptors &aitDescs ) {
	desc::MapOfDescriptors::const_iterator it=aitDescs.find( AIT_EXTERNAL_APP_AUTH_DESC );
	if (it != aitDescs.end()) {
		printf( "[Profile] Processing Exteranal Application Authorisation Descriptor\n" );
		
		//	Copy externals applications
		_externals = (*it).second.get<ait::ExternalApplications>();
	}
	else {
		//	Clear all external applications
		_externals.clear();
	}
}

//	Aux profile
struct FindProfileByType {
	FindProfileByType( ID type ) : _type(type) {}
	bool operator()( Profile *profile ) const {
		return profile->isSupported( _type );
	}
	ID _type;
};

void AitFactory::addProfile( Profile *profile ) {
	_profiles.push_back( profile );
}

Profile *AitFactory::findProfile( ID type ) const {
	FindProfileByType finder(type);
	Profiles::const_iterator it = std::find_if( _profiles.begin(), _profiles.end(), finder );
	return (it != _profiles.end()) ? (*it) : NULL;
}

Application *AitFactory::create( const ApplicationID &appID, const std::string &file ) {
	Application *app=NULL;	
	BOOST_FOREACH( Profile *profile, _profiles ) {
		app = profile->create( appID, file );
		if (app) {
			break;
		}
	}
	return app;
}

}
}

