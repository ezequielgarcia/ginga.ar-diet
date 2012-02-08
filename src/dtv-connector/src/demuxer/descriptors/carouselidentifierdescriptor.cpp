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
#include "carouselidentifierdescriptor.h"

namespace tuner {
namespace desc {

CarouselEnhancedBoot::CarouselEnhancedBoot( const Buffer &priv )
{
	BYTE *data = (BYTE *)priv.buffer();
	SSIZE_T offset=0;

	_moduleVersion    = RB(data,offset);
	_moduleID         = RW(data,offset);
	_blockSize        = RW(data,offset);
	_moduleSize       = RDW(data,offset);
	_compressedMethod = RB(data,offset);
	_originalSize     = RDW(data,offset);
	_timeout          = RB(data,offset);

	// SIZE_T len = priv.length();
	// offset += biop::Object::objectKey( data+offset, len-offset, _key );
	// if (len > offset) {
	// 	_privateData.copy( (char *)(data+offset), len-offset );
	// }
}

CarouselEnhancedBoot::~CarouselEnhancedBoot( void )
{
}

bool CarouselEnhancedBoot::operator==( const CarouselEnhancedBoot &other ) const {
	return _moduleVersion == other._moduleVersion &&
	_moduleID == other._moduleID &&
	_blockSize == other._blockSize &&
	_moduleSize == other._moduleSize &&
	_compressedMethod == other._compressedMethod &&
	_originalSize == other._originalSize &&
	_timeout == other._timeout &&
	//	_key == other._key &&
	_privateData == other._privateData;
}


CarouselIdentifierDescriptor::CarouselIdentifierDescriptor( DWORD carouselID, BYTE format, Buffer &privateData )
{
	_carouselID = carouselID;
	_format     = format;
	if (format == 1) {
		_formatData = CarouselEnhancedBoot( privateData );
	}
	else if (!format) {
		_formatData = privateData;
	}
}

CarouselIdentifierDescriptor::~CarouselIdentifierDescriptor( void )
{
}

bool CarouselIdentifierDescriptor::operator==( const CarouselIdentifierDescriptor &other ) const {
	return _carouselID == other._carouselID && _format == other._format && _formatData == other._formatData;
}

DWORD CarouselIdentifierDescriptor::carouselID() const {
	return _carouselID;
}

BYTE CarouselIdentifierDescriptor::format() const {
	return _format;
}

}
}
