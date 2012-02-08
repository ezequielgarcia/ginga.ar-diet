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
#include "dvbprovider.h"
#include "dvbpesfilter.h"
#include "dvbsectionfilter.h"
#include "generated/config.h"
#include <util/string.h>
#include <util/buffer.h>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

namespace tuner {
namespace dvb {

DVBProvider::DVBProvider( void )
	: _checkStatusTimer( _io )
{
	init( 0, 0, 0, 20 );
}

DVBProvider::DVBProvider( int adapter, int frontend, int demux, int maxFilters/*=20*/ )
	: _checkStatusTimer( _io )
{
	init( adapter, frontend, demux, maxFilters );
}

DVBProvider::~DVBProvider( void )
{
	close( _frontendFD );
}

//	Start/stop network
bool DVBProvider::startNetwork( const std::string &net ) {
	_chan = boost::lexical_cast<int>( net );
	return tune();
}

void DVBProvider::stopNetwork( void ) {
}

//	Scan methods
bool DVBProvider::scan( std::string &network ) {
	bool res=false;
	
	while (!res && _chan < DVB_CHANNELS) {
		res = tune();
		_chan++;
	}
	network = boost::lexical_cast<std::string>(_chan-1);
	return res;
}

bool DVBProvider::firstNetwork( std::string &network ) {
	_chan = 0;
	return scan( network );
}

bool DVBProvider::nextNetwork( std::string &network ) {
	return scan( network );	
}

//	Filter aux
Filter *DVBProvider::createFilter( TSSectionDemuxer *sectionDemux ) {
	return new DVBSectionFilter( sectionDemux, this );
}

int DVBProvider::maxFilters() const {
	return _maxFilters;
}

void DVBProvider::onCheckStatus( const boost::system::error_code& error, int chan ) {
	if (!error) {
		if (!isTunerLocked(chan)) {
			//	TODO: Avisar!
		}

		//	Re-launch timer to check status
		_checkStatusTimer.expires_from_now( boost::posix_time::milliseconds(500) );
		_checkStatusTimer.async_wait( boost::bind( &DVBProvider::onCheckStatus, this, _1, chan) );
	}
}

//	Getters
boost::asio::io_service &DVBProvider::io() {
	return _io;
}

const std::string &DVBProvider::device() {
	return _demux;
}

void DVBProvider::readSection( boost::asio::posix::stream_descriptor *io, ID pid, util::Buffer *buf/*=NULL*/ ) {
	if (!buf) {	
		buf = getNetworkBuffer();
	}
	
	io->async_read_some(
		boost::asio::buffer( buf->buffer(), buf->capacity() ),
		boost::bind(
			&DVBProvider::onReadSection,
			this,
			io,
			pid,
			buf,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred
		)
	);
}

void DVBProvider::onReadSection(
	boost::asio::posix::stream_descriptor *io,
	ID pid,
	util::Buffer *buf,
	const boost::system::error_code& error,
	size_t transferred )
{
	bool tryRead=true;
	
	if (error) {
		if (error.value() == boost::system::errc::timed_out) {
			//	Timeout
			timeout( pid );
		}
		else if (error.value() != boost::system::errc::value_too_large) {
			printf( "[DVBProvider] Read section error!!!: err(%s,%d), pid=%04x, transferred=%d, capacity=%d!\n",
				error.message().c_str(), error.value(), pid, transferred, buf->capacity() );
			tryRead=false;
		}
	}
	
	if (transferred) {
		buf->resize( transferred );
		enqueue( pid, buf );
		buf = NULL;
	}

	if (tryRead) {
		readSection( io, pid, buf );
	}
	else {
		freeNetworkBuffer( buf );
	}
}

//	Start PES filters
bool DVBProvider::startPesFilter( ID pid ) {
    printf( "[DVBProvider] Start PES filter: pid=%04x\n", pid );

    DVBPesFilter *filter = new DVBPesFilter( pid, this );
	FilterPtr ptr(filter);
	return startFilter( ptr );
}

std::string DVBProvider::streamPipe( void ) const {
	return _pipe;
}	

//	Aux filters engine
bool DVBProvider::startEngineFilter( void ) {
	bool result=false;
	
	printf( "[DVBProvider] Start filter engine\n" );

	if (openFrontend()) {
		//	Start demuxer thread
		_thread = boost::thread( boost::bind( &DVBProvider::loop, this ) );
		result=true;
	}
	return result;
}

void DVBProvider::stopEngineFilter( void ) {
	printf( "[DVBProvider] Stop filter engine: loop\n" );

	//	Stop io service
	_io.stop();

	//	Wait thread
	_thread.join();

	printf( "[DVBProvider] Stop filter engine: end\n" );	
}

void DVBProvider::loop( void ) {
	boost::asio::io_service::work work(_io);

	printf( "[DVBProvider] IO loop begin\n" );

	try {
		_io.run();
	}
	catch (std::exception& e) {
		printf( "[DVBProvider] Exception: %s\n", e.what() );
	}

	printf( "[DVBProvider] IO loop end\n" );	
}

//	Aux
void DVBProvider::init( int adapter, int frontend, int demux, int maxFilters ) {
	_chan = 0;
	_frontendFD = -1;
	_spectral_inversion = INVERSION_AUTO;
	_maxFilters = maxFilters;

	//	Create devices name
	_frontend = util::format( "/dev/dvb/adapter%d/frontend%d", adapter, frontend );
	_demux    = util::format( "/dev/dvb/adapter%d/demux%d", adapter, demux );
	_pipe     = util::format( "/dev/dvb/adapter%d/dvr0", adapter );
	printf( "[DVBProvider] Using devices: frontend=%s, demux=%s\n", _frontend.c_str(), _demux.c_str() );
}

bool DVBProvider::openFrontend( void ) {
	struct dvb_frontend_info fe_info;

	//	Open frontend
	if ((_frontendFD = open( _frontend.c_str(), O_RDWR )) < 0) {
		printf( "[DVBProvider] Error, failed to open '%s': %s %d\n", _frontend.c_str(), _demux.c_str(), errno );
		return false;
	}
	
	//	Determine FE type and caps
	if (ioctl(_frontendFD, FE_GET_INFO, &fe_info) == -1) {
		printf( "[DVBProvider] Error, FE_GET_INFO failed: %d\n", errno );
		return false;
	}
	_spectral_inversion = (fe_info.caps & FE_CAN_INVERSION_AUTO) ? INVERSION_AUTO : INVERSION_OFF;	

	printf( "[DVBProvider] Frontend info:\n" );
	printf( "[DVBProvider]\t name=%s, type=%x\n", fe_info.name, fe_info.type );
	printf( "[DVBProvider]\t frequency min=%d, max=%d, step=%d, tolerance=%d\n",
		fe_info.frequency_min, fe_info.frequency_max, fe_info.frequency_stepsize, fe_info.frequency_tolerance );
	printf( "[DVBProvider]\t symbol rate min=%d, max=%d, tolerance=%d\n",
		fe_info.symbol_rate_min, fe_info.symbol_rate_max, fe_info.symbol_rate_tolerance );
	printf( "[DVBProvider]\t notifier delay=%d\n", fe_info.notifier_delay );
	printf( "[DVBProvider]\t caps=%08X\n", fe_info.caps );

	return true;
}

bool DVBProvider::isTunerLocked( int chan ) {
	util::WORD snr, signal;
	fe_status_t st;

	//	Get status
	if (ioctl(_frontendFD, FE_READ_STATUS, &st) == -1) {
		printf( "[DVBProvider] FE_READ_STATUS failed\n" );
		return false;
	}

	// printf( "[DVBProvider] st=%02x: signal=%d, carrier=%d, viterbi=%d, has_sync=%d, has_lock=%d, timeout=%d, reinit=%d\n",
	// 	st, st & FE_HAS_SIGNAL,
	// 	st & FE_HAS_CARRIER, st & FE_HAS_VITERBI,
	// 	st & FE_HAS_SYNC, st & FE_HAS_LOCK,
	// 	st & FE_TIMEDOUT, st & FE_REINIT );

	if (st & FE_HAS_LOCK) {
		//	Get Signal-to-Noise ratio
		if (ioctl(_frontendFD, FE_READ_SNR, &snr) == -1) {
			printf( "[DVBProvider] FE_READ_SNR failed\n" );
			return false;
		}

		//	Get Signal strength
		if (ioctl(_frontendFD, FE_READ_SIGNAL_STRENGTH, &signal) == -1) {
			printf( "[DVBProvider] FE_READ_SIGNAL_STRENGTH failed\n" );
			return false;
		}

		//	Check Signal, SNR
		if (signal > DVB_MIN_SIGNAL && snr < DVB_MAX_SNR) {
			return true;
		}

		printf( "[DVBProvider] Warning, signal out of range: freq=%d, snr=%x, signal=%x\n",
			frequency( chan ), snr, signal );
	}
	return false;
}

int DVBProvider::frequency( int chan ) {
	return DVB_FIRST_FREQUENCY + (chan * DVB_BANDWIDTH * 1000000);
}

bool DVBProvider::tune( void ) {
	struct dvb_frontend_parameters param;	

	printf( "[DVBProvider] Tune started begin: chan=%d, freq=%d\n", _chan, frequency(_chan) );

	memset( &param, 0, sizeof(dvb_frontend_parameters) );
	param.frequency                    = frequency( _chan );
	param.inversion                    = _spectral_inversion;
	param.u.ofdm.bandwidth             = BANDWIDTH_6_MHZ;	//	TODO: Parametrice!?!?!
	param.u.ofdm.code_rate_HP          = FEC_3_4;
	param.u.ofdm.code_rate_LP          = FEC_AUTO;
	param.u.ofdm.constellation         = QAM_AUTO;
	param.u.ofdm.transmission_mode     = TRANSMISSION_MODE_AUTO;
	param.u.ofdm.guard_interval        = GUARD_INTERVAL_AUTO;
	param.u.ofdm.hierarchy_information = HIERARCHY_NONE;

	//	Cancel check status
	_checkStatusTimer.cancel();

	if (ioctl( _frontendFD, FE_SET_FRONTEND, &param ) == -1) {
		printf( "[DVBProvider] Setting frontend parameters failed\n" );
		return false;
	}

	//	Check if can lock ...
	for (int i = 0; i < 10; i++) {
		boost::this_thread::sleep( boost::posix_time::milliseconds(100) );
		
		if (isTunerLocked(_chan)) {
			//	Setup timer to check status
			_checkStatusTimer.expires_from_now( boost::posix_time::milliseconds(500) );
			_checkStatusTimer.async_wait( boost::bind( &DVBProvider::onCheckStatus, this, _1, _chan) );

			printf( "[DVBProvider] Tune started end: chan=%d, freq=%d\n", _chan, param.frequency );
			return true;
		}
	}

	printf( "[DVBProvider] Frontend failed!\n" );
	return false;
}

}
}

