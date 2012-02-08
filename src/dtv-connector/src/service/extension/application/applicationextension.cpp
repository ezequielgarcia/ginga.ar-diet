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
#include "applicationextension.h"
#include "application.h"
#include "applicationfactory.h"
#include "controller/applicationcontroller.h"
#include "../../serviceprovider.h" 
#include "../../service.h"
#include "../../../provider/provider.h"
#include "../../../provider/taskqueue.h"
#include "../../../resourcemanager.h"
#include "../../../demuxer/tssectiondemuxer.h"
#include "../../../demuxer/psi/dsmcc/datacarouselhelper.h"
#include <util/mcr.h>
#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <stdio.h>


namespace tuner {
namespace app {

namespace fs = boost::filesystem;

class FindApplicationIterator {
public:
	FindApplicationIterator( void ) {}
	virtual ~FindApplicationIterator( void ) {}

	virtual bool operator()( Application */*app*/ ) const {
		return false;
	}
};

struct AllApplications : public FindApplicationIterator {
	bool operator()( app::Application */*app*/ ) const {
		return true;
	}
};

struct AppFinderByTag : public FindApplicationIterator {
	AppFinderByTag( BYTE tag ) : _tag(tag) {}
	bool operator()( app::Application *app ) const {
		return app->componentTag() == _tag;
	}
	BYTE _tag;
};

struct AppFinderByAppID : public FindApplicationIterator {
	AppFinderByAppID( const app::ApplicationID &id ) : _id(id) {}
	bool operator()( app::Application *app ) const {
		return *app == _id;
	}
	const app::ApplicationID &_id;
};

struct AppFinderByBounded : public FindApplicationIterator {
	AppFinderByBounded( ID srvID ) : _srvID(srvID) {}
	bool operator()( app::Application *app ) const {
		return app->isBoundToService( _srvID );
	}
	ID _srvID;
};

ApplicationExtension::ApplicationExtension( ServiceManager *srvMgr, ApplicationController *ctrl )
	: Extension( srvMgr )
{
	assert(ctrl);
	_ctrl = ctrl;
	_dsmcc  = new dsmcc::DataCarouselHelper();
}

ApplicationExtension::~ApplicationExtension( void )
{
	CLEAN_ALL( ApplicationFactory *, _factories );
	delete _dsmcc;
	removeAll();
}

//	Factories method
void ApplicationExtension::addFactory( ApplicationFactory *factory ) {
	_factories.push_back( factory );
}

//	Aux filesystem
void ApplicationExtension::scan( const std::string path, int maxDepth, const EndScanApplicationCallback &callback ) {
	//	Remove all scanned applications
	ApplicationID wildcard(0,0xFFFF);
	remove( wildcard );

	//	Scan applications
	int appID = 1;
	try {
		scanApplications( path, appID, maxDepth, 0 );
	} catch (...) {
		printf( "[ApplicationExtension] Warning, scan of applications terminated with errors\n" );
	}

	//	On finish, call to callback
	if (!callback.empty()) {
		callback();
	}
}

void ApplicationExtension::scanApplications( const std::string &appPath, int &id, int maxDepth, int curDepth ) {
	printf( "[ApplicationExtension] Scan applications on %s\n", appPath.c_str() );

	curDepth++;
	if (fs::exists( appPath )) {
		fs::directory_iterator end_itr;
		for (fs::directory_iterator itr( appPath ); itr != end_itr; ++itr ) {
			if (fs::is_directory( itr->status() ) && curDepth < maxDepth) {
				scanApplications( itr->path().string(), id, maxDepth, curDepth );
			}
			else if (fs::is_regular_file( itr->status() )) {
				fs::path file = itr->path();

				//	factories to detect new applications?
				BOOST_FOREACH( ApplicationFactory *factory, _factories ) {
					//	Try detect file ...
					ApplicationID appID(0,id);
					Application *app = factory->tryFile( file.string(), appID );
					if (app) {
						add( app );
						id++;
					}
				}
			}
		}
	}
}

//	Add/Remove application
void ApplicationExtension::add( Application *app ) {
	assert( app );

	const ApplicationID &appID = app->appID();

	if (!appID.isWildcard()) {
		printf( "[ApplicationExtension] Add application: id=%s\n", appID.asString().c_str() );

		//	Notificate that a new application was added
		if (app->visibility() != visibility::none) {
			ctrl()->onApplicationAdded( app );
		}

		app->show();

		//	Add application to list
		_applications.push_back( app );
	
		//	Need download or start application
		if (app->needDownload()) {
			app->startDownload();
		}
		else if (app->autoStart()) {
			app->start();
		}
	}
	else {
		printf( "[ApplicationExtension] Warning, application ID is not valid: id=%s\n", appID.asString().c_str() );
	}
}

void ApplicationExtension::removeAll( void ) {
	AllApplications iter;
	remove( &iter );
}

void ApplicationExtension::remove( const ApplicationID &appID ) {
	AppFinderByAppID iter(appID);
	remove( &iter );
}

void ApplicationExtension::remove( struct FindApplicationIterator *iter ) {
	//	Find application
	Application *app;
	std::vector<Application *>::iterator it = _applications.begin();
	while (it != _applications.end()) {
		app = (*it);
		
		if ((*iter)( app )) {
			printf( "[ApplicationExtension] Remove application: app=%s\n", app->name().c_str() );
			
			//	Kill application and stop download
			app->stop( true );

			//	Unmount filesystem
			app->unmount();
			
			//	Notificate that a application was removed
			if (app->visibility() != visibility::none) {
				ctrl()->onApplicationRemoved( app );
			}
			delete app;

			//	Remove from list
			it=_applications.erase( it );
		}
		else {
			it++;
		}
	}
}

void ApplicationExtension::changePriority( const ApplicationID &appID, BYTE priority ) {
	//	Find all applications matching appID and change the priority
	bool any=loopApplications(
		AppFinderByAppID(appID),
		boost::bind(&Application::priority,_1) );

	if (any) {
		printf( "[ApplicationExtension] Change priority: appID=%s, priority=%d\n",
			appID.asString().c_str(), priority );
	}
	else {
		printf( "[ApplicationExtension] Warning, cannot change priority: appID=%s, priority=%d\n",
			appID.asString().c_str(), priority );
	}
 }

//	Aux applications
void ApplicationExtension::start( const ApplicationID &appID ) {
	//	Stop all applications matching appID
	bool any=loopApplications(
		AppFinderByAppID(appID),
		boost::bind(&Application::start,_1) );

	if (!any) {
		printf( "[ApplicationExtension] Warning, cannot start application; application not found: id=%s\n",
			appID.asString().c_str() );
	}
}

void ApplicationExtension::stop( const ApplicationID &appID, bool kill/*=false*/ ) {
	//	Stop all applications matching appID
	bool any=loopApplications(
		AppFinderByAppID(appID),
		boost::bind(&Application::stop,_1,kill) );

	if (!any) {
		printf( "[ApplicationExtension] Warning, cannot stop application; application not found: id=%s\n",
			appID.asString().c_str() );
	}
}

void ApplicationExtension::stopAll( void ) {
	printf( "[ApplicationExtension] On stop all application\n" );
	loopApplications(
		AllApplications(),
		boost::bind(&Application::stop,_1,true) );
}

void ApplicationExtension::status( const ApplicationID &appID, status::type st ) {
	//	Send change of state to all applications matching appID
	bool any=loopApplications(
		AppFinderByAppID(appID),
		boost::bind(&Application::status,_1,st) );

	if (!any) {
		printf( "[ApplicationExtension] Warning, cannot change application status; application not found: id=%s\n",
			appID.asString().c_str() );
	}
}

//	Service Provider notifications
static bool collectDSMCC( BYTE streamType ) {
	return streamType == PSI_ST_TYPE_B || streamType == PSI_ST_TYPE_C;
}

void ApplicationExtension::onServiceStarted( Service *srv ) {
	printf( "[ApplicationExtension] On Service started begin\n" );

	//	Start factories on service
	BOOST_FOREACH( ApplicationFactory *factory, _factories ) {
		factory->serviceStarted( srv );
	}

	//	Collect all DSMCC tags for object carousel
	_dsmcc->collectTags( srv, &collectDSMCC );

	printf( "[ApplicationExtension] On Service started end\n" );	
}

void ApplicationExtension::onServiceStopped( Service *srv ) {
	printf( "[ApplicationExtension] On service stopped begin\n" );

	//	Stop factories
	BOOST_FOREACH( ApplicationFactory *factory, _factories ) {
		factory->serviceStopped( srv );
	}	

	//	Remove all applications bounded to service
	AppFinderByBounded iter(srv->id());
	remove( &iter );

	//	Clear all tags referenced by the service
	std::vector<BYTE> tags=_dsmcc->clearTags( srv );

	//	Stop all tags
	BOOST_FOREACH( BYTE tag, tags ) {
		loopApplications(
			AppFinderByTag(tag),
			boost::bind(&Application::stopDownload,_1) );
	}

	printf( "[ApplicationExtension] On service stopped end\n" );	
}

//	Aux download
void ApplicationExtension::download( const ApplicationID &appID ) {
	//	Start all downloads matching appID
	bool any=loopApplications(
		AppFinderByAppID(appID),
		boost::bind(&Application::startDownload,_1) );

	if (!any) {
		printf( "[ApplicationExtension] Warning, cannot download application; application not found: id=%s\n",
			appID.asString().c_str() );
	}
}

ID ApplicationExtension::findTag( ID tag, ID service ) {
	return service ? _dsmcc->findTag( service, tag ) : _dsmcc->findTag( tag );
}

}
}

