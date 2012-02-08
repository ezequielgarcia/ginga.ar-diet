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

#include "../provider.h"
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <linux/dvb/frontend.h>

namespace tuner {
namespace dvb {

class DVBProvider : public Provider {
public:
	DVBProvider( void );
	DVBProvider( int adapter, int frontend, int demux, int maxFilters=20 );
	virtual ~DVBProvider( void );

	//	Getters
	boost::asio::io_service &io();
	const std::string &device();

	//	Start PES filters
	virtual bool startPesFilter( ID pid );
	virtual std::string streamPipe( void ) const;

	//	Read a section
	void readSection( boost::asio::posix::stream_descriptor *io, ID pid, util::Buffer *buf=NULL );

protected:
	//	Network methods
	virtual bool startNetwork( const std::string &net );
	virtual void stopNetwork( void );
	virtual bool firstNetwork( std::string &network );
	virtual bool nextNetwork( std::string &network );

	//	Aux filters engine
	virtual bool startEngineFilter( void );
	virtual void stopEngineFilter( void );
	void onCheckStatus( const boost::system::error_code& error, int chan );
	void onReadSection( boost::asio::posix::stream_descriptor *io, ID pid, util::Buffer *buf, const boost::system::error_code& error, size_t transferred );	
	
	//	Filter methods
	virtual Filter *createFilter( TSSectionDemuxer *sectionDemux );
	virtual int maxFilters() const;	

	//	Aux
	void init( int adapter, int frontend, int demux, int maxFilters );
	bool openFrontend( void );
	bool tune( void );
	bool scan( std::string &network );
	int frequency( int chan );
	void loop( void );
	bool isTunerLocked( int chan );

private:
	enum fe_spectral_inversion _spectral_inversion;
	int _frontendFD;
	std::string _pipe;
	std::string _frontend;
	std::string _demux;
	int _maxFilters;	
	int _chan;
	boost::asio::io_service _io;
	boost::asio::deadline_timer _checkStatusTimer;		
	boost::thread _thread;
};

}
}

