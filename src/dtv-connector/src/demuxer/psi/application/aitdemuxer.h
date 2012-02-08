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
#include "ait.h"
#include <vector>

namespace tuner {

class AITDemuxer : public PSIDemuxer {
public:
	AITDemuxer( ID pid );
	virtual ~AITDemuxer( void );

	//	Types
	struct Subtable {
		ID appType;
		Version version;

		bool isEqual( const Subtable &sub ) const {
			return appType == sub.appType;
		}
	};

	//	Getters
	virtual ID tableID() const;

	//  Signal
	typedef boost::function<void ( Ait * )> ParsedCallback;
	void onParsed( const ParsedCallback &callback );

protected:
	//	PSIDmeuxer virtual methods
	virtual void onSection( BYTE *section, SIZE_T len );
	virtual Version updateVersion( Version ver, BYTE *section, bool apply );
	virtual bool supportMultipleSections( void ) const;	

	//	AIT descriptors aux
	SSIZE_T parseDescriptors( BYTE *buf, SIZE_T len, desc::MapOfDescriptors &descriptors );
	SSIZE_T parseAppDesc( BYTE *buf, SIZE_T len, desc::MapOfDescriptors &descriptors );
	SSIZE_T parseTransportProtocolDesc( BYTE *buf, SIZE_T len, desc::MapOfDescriptors &descriptors );
	SSIZE_T parseAppNameDesc( BYTE *buf, SIZE_T len, desc::MapOfDescriptors &descriptors );
	SSIZE_T parseGingaApp( BYTE *buf, SIZE_T len, BYTE tag, desc::MapOfDescriptors &descriptors );
	SSIZE_T parseGingaAppLocation( BYTE *buf, SIZE_T len, BYTE tag, desc::MapOfDescriptors &descriptors );
	SSIZE_T parseExternalAppAuth( BYTE *buf, SIZE_T len, BYTE tag, desc::MapOfDescriptors &descriptors );
	SSIZE_T parseIconsDescriptor( BYTE *buf, SIZE_T len, BYTE tag, desc::MapOfDescriptors &descriptors );
	SSIZE_T parseGraphicsConstraintsDescriptor( BYTE *buf, SIZE_T len, BYTE tag, desc::MapOfDescriptors &descriptors );

private:
	ParsedCallback _onParsed;
	std::vector<Subtable> _tables;	
};

}
