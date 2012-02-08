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

#include "../tssectiondemuxer.h"
#include <boost/function.hpp>
#include <vector>

namespace util {
	class Buffer;
}

namespace tuner {

class PSIDemuxer : public TSSectionDemuxer {
public:
	PSIDemuxer( ID pid );
	virtual ~PSIDemuxer( void );

	//	Getters
	virtual ID tableID() const;
	virtual WORD maxSectionBytes() const;
	virtual bool hasSubtable() const;

	//  Operations
	virtual void startData( BYTE *tsPayload, SIZE_T len );
	virtual void pushData( BYTE *tsPayload, SIZE_T len );

	//  Signals
	typedef boost::function<void ( ID pid )> ExpiredCallback;
	void onExpired( const ExpiredCallback &callback );

	//  Signals
	typedef boost::function<void ( ID pid )> TimeoutCallback;
	void onTimeout( const TimeoutCallback &callback );

protected:
	//	Section parsing
	virtual void onSection( BYTE *section, SIZE_T len )=0;		
	virtual void expire( Version verExpired );
	virtual bool syntax() const;
	virtual void timeoutExpired();
	bool checkCRC( BYTE *payload, int packetLen );	

	//  Version handling
	virtual bool useStandardVersion() const;
	virtual bool versionChanged( BYTE *section );
	virtual Version updateVersion( Version ver, BYTE *section, bool apply );
	virtual void onVersionChanged( void );	

	//	Support for multiple sections
	virtual bool supportMultipleSections( void ) const;
	virtual bool sectionParsed( BYTE num ) const;
	virtual void onComplete( BYTE *section, SIZE_T len );
	void parseSection( BYTE *section, SIZE_T len );

	//	Aux
	virtual DWORD frequency() const;
	SSIZE_T startHeader( BYTE *payload, SIZE_T len );
	SSIZE_T endSection( BYTE *payload, SIZE_T len );
	Buffer *getBuffer();
	bool checkPID( ID pid, ID ignore=0xFFFF );

private:
	ExpiredCallback _onExpired;
	TimeoutCallback _onTimeout;
	Version         _lastExpired;
	
	int               _sections;
	std::vector<bool> _secsParsed;

	Buffer *_buffer;
	bool    _error;
	bool    _stuffingByteFlag;
};

}
