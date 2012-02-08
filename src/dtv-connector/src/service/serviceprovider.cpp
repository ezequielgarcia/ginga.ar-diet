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
#include "serviceprovider.h"
#include "service.h"
#include "extension/extension.h"
#include "../provider/provider.h"
#include "../provider/taskqueue.h"
#include "../resourcemanager.h"
#include "../demuxer/ts.h"
#include "../demuxer/psi/psi.h"
#include "../demuxer/tssectiondemuxer.h"
#include "../demuxer/psi/patdemuxer.h"
#include "../demuxer/psi/pmtdemuxer.h"
#include "../demuxer/psi/sdtdemuxer.h"
#include "../demuxer/psi/nitdemuxer.h"
#include "../demuxer/psi/dsmcc/module/memorydata.h"
#include <set>
#include <stdio.h>

#define READY_STATE (sps::pat | sps::nit | sps::sdt)

namespace tuner {

//	Service Provider State
namespace sps {

enum type {
       pat=0x01,
       nit=0x02,
       sdt=0x04
};

}

struct FindByPid {
	ID _pid;
	FindByPid( ID pid ) : _pid( pid ) {}
	bool operator()( const std::pair<ID,Service *> &p ) const {
		return p.second->pid() == _pid;
	}
};

ServiceProvider::ServiceProvider( Provider *provider, ResourceManager *resMgr )
	: ServiceManager( resMgr )
{
	assert(provider);

	_endScanCalled = false;
	_ready         = 0;
	_provider      = provider;
	_tsID          = INVALID_TS_ID;
	_nitID         = NIT_ID_RESERVED;
	_nitPID        = TS_PID_NULL;
	_running       = false;

	//	Initialize pool of memory data
	tuner::dsmcc::MemoryData::initialize( resMgr->memoryblocks(), 4096 );
}

ServiceProvider::~ServiceProvider( void )
{
	assert(_ready == 0);
	assert(!_running);
	assert(_services.empty());
	assert(_servicesQueued.empty());

	//	Cleanup pool of memory data
	tuner::dsmcc::MemoryData::cleanup();
}

bool ServiceProvider::isRunning() const {
	return _running;
}

bool ServiceProvider::start( bool withExtensions/*=true*/ ) {
	bool result=false;
	if (!_running) {
		printf( "[ServiceProvider] Start: enable extensions: %d\n", withExtensions );
		enqueue( new BasicTask( boost::bind(&ServiceProvider::onStart,this,withExtensions) ) );
		_running = true;
		result=true;
	}
	return result;
}

void ServiceProvider::stop( void ) {
	if (_running) {
		printf( "[ServiceProvider] Stop\n" );
		enqueue( new BasicTask( boost::bind(&ServiceProvider::onStop,this) ) );
		_running = false;
	}
}

void ServiceProvider::startService( ID service ) {
	if (_running) {	
		printf( "[ServiceProvider] Start service id=%x\n", service );
		enqueue( new BasicTask( boost::bind(&ServiceProvider::onStartServiceID,this,service) ) );
	}
}

void ServiceProvider::stopService( ID service ) {
	if (_running) {
		printf( "[ServiceProvider] Stop service id=%x\n", service );
		enqueue( new BasicTask( boost::bind(&ServiceProvider::onStopServiceID,this,service) ) );		
	}
}

void ServiceProvider::stopServices( void ) {
	if (_running) {
		printf( "[ServiceProvider] Stop services\n" );
		enqueue( new BasicTask( boost::bind(&ServiceProvider::onStopServices,this) ) );		
	}
}

//	Filters
bool ServiceProvider::startFilter( TSSectionDemuxer *demux ) {
	return _provider->startFilter( demux );
}

void ServiceProvider::stopFilter( ID pid ) {
	_provider->stopFilter( pid );
}

bool ServiceProvider::startPesFilter( ID pid ) {
	return _provider->startPesFilter( pid );
}

std::string ServiceProvider::streamPipe( void ) {
	return _provider->streamPipe();
}

//	Enqueue task into service manager thread
void ServiceProvider::enqueue( Task *task ) {
	_provider->enqueue( task );
}

//	Getters
DWORD ServiceProvider::currentTS() const {
	return _tsID;
}

ID ServiceProvider::currentNit() const {
	return _nitID;
}

void ServiceProvider::onServiceScanned( const OnServiceScanned &callback ) {
	_onServiceScanned = callback;
}

void ServiceProvider::onEndScan( const OnEndScan &callback ) {
	_onEndScan = callback;
}

//	Services
Service *ServiceProvider::findService( ID service ) {
	Services::const_iterator it=_services.find( service );
	return (it != _services.end()) ? (*it).second : NULL;
}

void ServiceProvider::killServices( void ) {
	Service *srv;

	printf( "[ServiceProvider] Kill services\n" );

	//	Update state ...
	setFlags( READY_STATE, false );

	//	Iterate throw services ...
	BOOST_FOREACH( const ServicesElement &elem, _services ) {
		srv = elem.second;
		
		//	Save pid filter to stop
		stopFilter( srv->pid() );

		//	Delete service
		delete srv;
	}
	_services.clear();

 	//	If NIT was available, stop filter
	stopNit();

	_tsID  = INVALID_TS_ID;
}

void ServiceProvider::onStart( bool withExtensions ) {
	printf( "[ServiceProvider] On start begin\n" );

	//	Checks
	assert(_services.empty());
	assert(_servicesQueued.empty());
	assert(_tsID == INVALID_TS_ID);
	assert(_endScanCalled == false);

	//	Clean temporary files
	resMgr()->clean();
	
	//	Filter the PAT
	startFilter( createDemuxer<PATDemuxer,Pat>(
			TS_PID_PAT,
			boost::bind(&ServiceProvider::onPat,this,_1),
			boost::bind(&ServiceProvider::onPatExpired,this),
			boost::bind(&ServiceProvider::onPatTimeout,this)
		));

	//	Start extensions
	exStart( withExtensions );

	printf( "[ServiceProvider] On start end\n" );	
}

void ServiceProvider::onStop( void ) {
	printf( "[ServiceProvider] On stop begin\n" );

	//	Kill services
	killServices();
	_servicesQueued.clear();
	_endScanCalled = false;
	
	//	Stop extensions
	exStop();	
	
	//	Stop PAT filter
	stopFilter( TS_PID_PAT );

	printf( "[ServiceProvider] On stop end\n" );	
}

void ServiceProvider::onStopServices( void ) {
	printf( "[ServiceProvider] On stop services\n" );

	//	Stop all services runnning
	BOOST_FOREACH( const ServicesElement &elem, _services ) {
		onStopService( elem.second );
	}

	//	Cleanup all services queued
	_servicesQueued.clear();
}

void ServiceProvider::onStopServiceID( ID service ) {
	printf( "[ServiceProvider] On stop service by ID begin: %04x\n", service );

	//	Find service ...
	Service *srv = findService( service );
	if (srv) {
		//	Stop service ...
		onStopService( srv );
	}
	else {
		//	Find service in services queued to run
		std::vector<ID>::iterator it=std::find( _servicesQueued.begin(), _servicesQueued.end(), service );
		if (it != _servicesQueued.end()) {
			//	Only remove from list ...
			_servicesQueued.erase( it );
		}
	}

	printf( "[ServiceProvider] On stop service by ID end: %04x\n", service );	
}

bool ServiceProvider::onStopService( Service *srv ) {
	bool result=false;
	
	if (srv->state() == service::state::running) {
		printf( "[ServiceProvider] On stop service: %04x\n", srv->id() );

		//	Mark service as ready
		srv->state( service::state::ready );

		//	Notify to extensions of service was stopped
		exServiceStopped( srv );
		
		result=true;
	}
	
	return result;
}

void ServiceProvider::onStartServiceID( ID service ) {
	printf( "[ServiceProvider] On start service by ID: %04x\n", service );
	
	//	Check if service ready ...
	Service *srv = findService( service );
	if (srv && isReady() && srv->state() == service::state::ready) {
		//	Start service ...
		onStartService( srv );
	}
	else {
		//	Check if service already enqueued ...
		std::vector<ID>::iterator it=std::find( _servicesQueued.begin(), _servicesQueued.end(), service );
		if (it == _servicesQueued.end()) {		
			//	enqueue and wait ...
			_servicesQueued.push_back( service );
		}
	}
}

void ServiceProvider::onStartService( Service *srv ) {
	printf( "[ServiceProvider] On start service begin: id=%x\n", srv->id() );

	assert( srv->state() == service::state::ready );
	
	//	Notify to extensions of service was started
	exServiceStarted( srv );
	
	//	Mark service as running
	srv->state( service::state::running );

	printf( "[ServiceProvider] On start service end: id=%x\n", srv->id() );
}

void ServiceProvider::onExpireService( Service *srv, bool needExpire/*=true*/ ) {
	if (srv->state() >= service::state::ready) {
		printf( "[ServiceProvider] On expire service: serviceID=%04x, needExpire=%d\n",
			srv->id(), needExpire );
		
		//	Stop service if running
		bool stopped=onStopService( srv );

		//	Set new status
		srv->state( needExpire ? service::state::expired : service::state::complete );

		//	Notify to extensions that service was killed
		exServiceExpired( srv );

		//	Queue running state
		if (stopped) {
			onStartServiceID( srv->id() );
		}
	}
}

void ServiceProvider::notifyEndScan( bool allScanned/*=false*/ ) {
	if (!_endScanCalled) {
		//	Check if last scanned ...
		if (!allScanned) {
			allScanned=true;
			BOOST_FOREACH( const ServicesElement &elem, _services ) {
				allScanned &= (elem.second->state() >= service::state::timeout);
			}
		}

		if (allScanned) {
			printf( "[ServiceProvider] Notify end scan\n" );
			_endScanCalled = true;
			if (!_onEndScan.empty()) {
				_onEndScan();
			}
		}
	}
}

void ServiceProvider::notifyScan( Service *srv ) {
	if (!_onServiceScanned.empty()) {
		_onServiceScanned( srv );
	}
	notifyEndScan();
}

void ServiceProvider::onServiceReady( Service *srv ) {
	printf( "[ServiceProvider] Service ready: %04x\n", srv->id() );

	assert( srv->state() == service::state::complete );
	srv->state( service::state::ready );

	srv->show();
	
	//	Notify to dependent of a service was ready
	notifyScan( srv );
			
	//	Report service ready
	exServiceReady( srv );
			
	//	If serviceID was queued to run
	std::vector<ID>::iterator it=std::find( _servicesQueued.begin(), _servicesQueued.end(), srv->id() );
	if (it != _servicesQueued.end()) {
		//	Start service
		onStartService( srv );
		_servicesQueued.erase( it );
	}
}

void ServiceProvider::stopNit( void ) {
	if (_nitPID != TS_PID_NULL) {
		//	Stop NIT
		stopFilter( _nitPID );

		//	SDT pid
		stopFilter( TS_PID_SDT );

		//	Initialize NIT state
		_nitID  = NIT_ID_RESERVED;
		_nitPID = TS_PID_NULL;	
	}
}

//	Tables callback
void ServiceProvider::onPat( Pat *pat ) {
	printf( "[ServiceProvider] On PAT begin\n" );

	pat->show();
	assert( _tsID == INVALID_TS_ID );

	//  Loop into PAT programs, create the services and filter each PMT
	Service *srv;
	Pat::Programs &programs = pat->programs();
	std::set<ID> pids;
	BOOST_FOREACH( const Pat::ProgramInfo &prog, programs ) {
		//	Create service
		srv = new Service( prog.program, prog.pid );
			
		//	Add service to list
		_services[ prog.program ] = srv;

		//	Only add pid if not present ...
		pids.insert( prog.pid );
	}

	//	set tsID
	_tsID = pat->tsID();
	setFlags( sps::pat, true );

	//	Start PMTs filters
	BOOST_FOREACH( ID pid, pids ) {
		//	Filter pmt for each service
		startFilter( createDemuxer<PMTDemuxer,Pmt>(
				pid,
				boost::bind(&ServiceProvider::updateServices<Pmt>,this,_1),
				boost::bind(&ServiceProvider::onPmtExpired,this,pid),
				boost::bind(&ServiceProvider::onPmtTimeout,this,pid)
			));
	}

	//  Filter NIT if available
	_nitPID = pat->nitPid();
	if (_nitPID != TS_PID_NULL) {
		startFilter( createDemuxer<NITDemuxer,Nit>(
				_nitPID,
				boost::bind(&ServiceProvider::onNit,this,_1),
				boost::bind(&ServiceProvider::onExpired,this,(sps::nit|sps::sdt)),
				boost::bind(&ServiceProvider::onTimeout,this,(sps::nit|sps::sdt))
			));
	}
	else {
		//	NIT not present, so SDT too
		setFlags( (sps::nit|sps::sdt), true );
	}
	
	delete pat;

	printf( "[ServiceProvider] On PAT end\n" );	
}

void ServiceProvider::onPatExpired( void ) {
	printf( "[ServiceProvider] PAT Expired\n" );
	//	Kill all services
	killServices();
}

void ServiceProvider::onPatTimeout( void ) {
	printf( "[ServiceProvider] PAT timeout\n" );

	if (!_endScanCalled) {	
		//	Force all scanned ...
		notifyEndScan( true );

		//	Kill all services
		killServices();
	}
}

void ServiceProvider::onNit( Nit *nit ) {
	printf( "[ServiceProvider] On NIT begin\n" );
	
	if (nit->actual()) {
		//	Update NIT
		_nitID = nit->networkID();

		//	Check consistency
		ID oNit = nit->getNetwork( currentTS() );
		if (oNit != NIT_ID_RESERVED && oNit != nit->networkID()) {
			printf( "[ServiceProvider] Warning, NIT with errors: oNit=%04x, network tsID=%04x, using=%4x\n", oNit, _nitID, oNit );
			_nitID = oNit;
		}

		//	Process NIT
		updateServices<Nit>( nit );

		//	Always restart SDT filter, so if NIT ID changed, a new SDT can be processed
		setFlags( sps::sdt, false );
		stopFilter( TS_PID_SDT );
		startFilter( createDemuxer<SDTDemuxer,Sdt>(
				TS_PID_SDT,
				boost::bind(&ServiceProvider::onSdt,this,_1),
				boost::bind(&ServiceProvider::onExpired,this,sps::sdt),
				boost::bind(&ServiceProvider::onTimeout,this,sps::sdt)
			));

		//	Update status
		setFlags( sps::nit, true );
	}
	else {
		//	NIT is for other network or ts, ignore them
		nit->show();
		delete nit;
	}

	printf( "[ServiceProvider] On NIT end\n" );	
}

void ServiceProvider::onSdt( Sdt *sdt ) {
	printf( "[ServiceProvider] On SDT begin\n" );
	
	if (sdt->actual() &&
		sdt->nitID() == currentNit() &&
		sdt->tsID() == currentTS())
	{
		updateServices<Sdt>( sdt );

		//	Mark SDT ready!
		setFlags( sps::sdt, true );
	}
	else {
		//	SDT is for other network or ts, ignore them
		sdt->show();
		delete sdt;
	}

	printf( "[ServiceProvider] On SDT end\n" );	
}

bool ServiceProvider::isReady( void ) const {
	return _ready == READY_STATE;	
}

void ServiceProvider::setFlags( DWORD mask, bool set ) {
	//	Backup old state
	bool wasReady = isReady();

	printf( "[ServiceProvider] Set flags begin: mask=%08lx, state=%08lx, set=%d, wasReady=%d\n",
		mask, _ready, set, wasReady );

	//	Set/Clean mask
	if (set) {
		_ready |= mask;
	}
	else {
		_ready &= ~mask;
	}

	//	State changed ...
	if (wasReady != isReady()) {
		if (wasReady) {
			//	Expire all services
			expire();
		}
		else {
			//	Set ready
			setReady();
		}
	}

	printf( "[ServiceProvider] Set flags end: mask=%08lx, state=%08lx, set=%d, wasReady=%d, isReady=%d\n",
		mask, _ready, set, wasReady, isReady() );
}

void ServiceProvider::expire( void ) {
	printf( "[ServiceProvider] Service provider expired\n" );

	//	Expire all services
	BOOST_FOREACH( const ServicesElement &elem, _services ) {
		onExpireService( elem.second, false );
	}

	//	Notify to extensions of service provider wasn't ready
	exReady( false );
}

void ServiceProvider::setReady( void ) {
	printf( "[ServiceProvider] Service provider ready\n" );
	
	//	Notify to extensions of service provider was ready
	exReady( true );

	//	Notify all services ready
	Service *srv;
	BOOST_FOREACH( const ServiceProvider::ServicesElement &elem, _services ) {
		srv = elem.second;
		if (srv->state() == service::state::complete) {
			onServiceReady( srv );
		}
	}
}

void ServiceProvider::serviceChanged( Service *srv ) {
	if (srv && isReady() && srv->state() == service::state::complete) {
		onServiceReady( srv );
	}	
}

void ServiceProvider::onTimeout( DWORD mask ) {
	printf( "[ServiceProvider] Timeout: mask=%08lx\n", mask );	
	if (!_endScanCalled) {
		setFlags( mask, true );
	}
}

void ServiceProvider::onExpired( DWORD mask ) {
	printf( "[ServiceProvider] Expired: mask=%08lx\n", mask );		
	setFlags( mask, false );
}

void ServiceProvider::onPmtExpired( ID pid ) {
	//	Find if pid changed is from a service (for PMT changes)
	Services::iterator it=std::find_if( _services.begin(), _services.end(), FindByPid(pid) );
	if (it != _services.end()) {
		onExpireService( (*it).second );
	}
}

void ServiceProvider::onPmtTimeout( ID pid ) {
	//	Find if pid changed is from a service (for PMT changes)
	Services::iterator it=std::find_if( _services.begin(), _services.end(), FindByPid(pid) );
	if (it != _services.end()) {
		Service *srv = (*it).second;
		if (srv->state() <= service::state::present) {
			(*it).second->state( service::state::timeout );
			notifyEndScan();
		}
	}
}

}
