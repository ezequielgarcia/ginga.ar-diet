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

#include "../psidemuxer.h"
#include "sdtt.h"

namespace tuner {

class SDTTDemuxer : public PSIDemuxer {
public:
    SDTTDemuxer( ID pid );
    virtual ~SDTTDemuxer( void );

	//	Types
	struct Subtable {
		ID modelID;
		Version version;
		ID tsID;
		ID nitID;
		ID serviceID;

		bool isEqual( const Subtable &sub ) const {
			return modelID == sub.modelID &&
				tsID == sub.tsID &&
				nitID == sub.nitID &&
				serviceID == sub.serviceID;
		}
	};

	//	Getters
	virtual bool hasSubtable() const;	

    //  Signal
    typedef boost::function<void ( Sdtt * )> ParsedCallback;
    void onParsed( const ParsedCallback &callback );

protected:
    virtual void onSection( BYTE *section, SIZE_T len );
	virtual Version updateVersion( Version ver, BYTE *section, bool apply );
	SSIZE_T parseContentDescriptor( BYTE *data, WORD schedLen, WORD contentLen, Sdtt::ElementaryInfo &element );
	SSIZE_T parseDownloadContentDescriptor( BYTE *data, SIZE_T len, std::vector<Sdtt::DownloadContentDescriptor> &desc );
	virtual bool supportMultipleSections( void ) const;

private:
    ParsedCallback _onParsed;
	std::vector<Subtable> _tables;	
};

}
