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

#include "../provider/filterdemuxer.h"

namespace tuner {

class TSSectionDemuxer : public FilterDemuxer {
public:
	TSSectionDemuxer( ID pid );
    virtual ~TSSectionDemuxer();

	//	Filter demuxer methods
    virtual ID pid() const;
	virtual void process( util::Buffer *buf );

    //  Getters	
	virtual ID tableID() const=0;
	virtual WORD maxSectionBytes() const=0;
	virtual DWORD timeout() const;
	virtual DWORD frequency() const=0;

    //  Operations
	void checkCRC( bool enableCheck );
    virtual void startData( BYTE *tsPayload, SIZE_T len )=0;
    virtual void pushData( BYTE *tsPayload, SIZE_T len )=0;

protected:
	bool needCheckCRC() const;
	virtual void timeoutExpired();

private:
	bool _enableCRC;
    ID   _pid;
};

template<class Demuxer, typename T>
inline TSSectionDemuxer *createDemuxer(
       ID pid,
       const typename Demuxer::ParsedCallback &onParsed )
{
       Demuxer *demux = new Demuxer( pid );

       demux->onParsed( onParsed );
	   
	   return demux;
}

template<class Demuxer, typename T>
inline TSSectionDemuxer *createDemuxer(
       ID pid,
       const typename Demuxer::ParsedCallback &onParsed,
       const typename Demuxer::ExpiredCallback &onExpired )
{
       Demuxer *demux = new Demuxer( pid );

       demux->onParsed( onParsed );
	   demux->onExpired( onExpired );
	   return demux;
}

template<class Demuxer, typename T>
inline TSSectionDemuxer *createDemuxer(
       ID pid,
       const typename Demuxer::ParsedCallback &onParsed,
       const typename Demuxer::ExpiredCallback &onExpired,
	   const typename Demuxer::TimeoutCallback &onTimeout )
{
       Demuxer *demux = new Demuxer( pid );

       demux->onParsed( onParsed );
	   demux->onExpired( onExpired );
	   demux->onTimeout( onTimeout );
	   return demux;
}

}
