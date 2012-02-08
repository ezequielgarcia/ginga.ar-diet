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

#include "servicemanager.h"
#include "service.h"
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <map>
#include <vector>

namespace tuner {

//	Tables
class Pat;
class Pmt;
class Sdt;
class Nit;
class Provider;

class ServiceProvider : public ServiceManager {
public:
	ServiceProvider( Provider *prov, ResourceManager *resources );
	virtual ~ServiceProvider( void );

	//	Types
	typedef boost::function<void (Service *)> OnServiceScanned;
	void onServiceScanned( const OnServiceScanned &callback );
	typedef boost::function<void (void)> OnEndScan;	
	void onEndScan( const OnEndScan &callback );

	//	Start/Stop
	bool start( bool withExtensions=true );
	void stop( void );
	bool isRunning() const;

	//	Start/Stop services
	void startService( ID serviceID );
	void stopService( ID serviceID );
	void stopServices( void );

	//	Filters
	virtual bool startFilter( TSSectionDemuxer *demux );
	virtual void stopFilter( ID pid );
	virtual bool startPesFilter( ID pid );
	virtual std::string streamPipe( void );	
	
	//	Enqueue task into service manager thread
	virtual void enqueue( Task *task );

	//	Getters
	virtual DWORD currentTS() const;
	virtual ID currentNit() const;	

protected:
	//	Types
	typedef std::map<ID,Service *> Services;
	typedef std::pair<ID,Service *> ServicesElement;

	//	Services
	Service *findService( ID service );	
	void killServices();
	void onStart( bool withExtensions );	
	void onStop( void );
	void onStopServices( void );
	void onStopServiceID( ID service );
	bool onStopService( Service *srv );
	void onStartServiceID( ID service );
	void onStartService( Service *srv );
	void onExpireService( Service *srv, bool needExpire=true );
	void onServiceReady( Service *srv );

	//	State
	void setFlags( DWORD flag, bool set );
	bool isReady() const;
	void expire( void );
	void setReady( void );
	void stopNit( void );

	//	Scan
	void notifyEndScan( bool allScanned=false );
	void notifyScan( Service *srv );

	//	Tables callback
	void onPat( Pat *pat );
	void onPatExpired( void );
	void onPatTimeout( void );
	void onPmtExpired( ID pid );
	void onPmtTimeout( ID pid );
	void onNit( Nit *nit );
	void onSdt( Sdt *sdt );
	void onExpired( DWORD flags );
	void onTimeout( DWORD falgs );
	void processServices( Pat *pat );
	template<class T> inline void updateServices( T *ptr );
	void serviceChanged( Service *srv );	

private:
	DWORD _ready;
	ID    _nitID;
	ID    _nitPID;
	DWORD _tsID;
	bool  _running;
	Services  _services;
	Provider *_provider;
	//	Status    _ready;
	std::vector<ID>   _servicesQueued;
	OnServiceScanned  _onServiceScanned;
	OnEndScan         _onEndScan;
	bool              _endScanCalled;
};

template<class T>
inline void ServiceProvider::updateServices( T *ptr ) {
	Service *srv;

	ptr->show();
	BOOST_FOREACH( const ServiceProvider::ServicesElement &elem, _services ) {
		srv = elem.second;
		//	Process table and check if changed
		if (srv->process( ptr )) {
			//	Notify of service changed
			serviceChanged( srv );
		}
	}
	delete ptr;
}

}
