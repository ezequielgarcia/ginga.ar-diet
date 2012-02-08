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
#include "dsmcc.h"

namespace tuner {
namespace dsmcc {

void show( const EventName &ev ) {
	printf( "[biop::Event] Name=%s, ID=%04x\n",
		ev.name.c_str(), ev.id );
}

namespace compatibility {

SSIZE_T parse( BYTE *data, SIZE_T len, Descriptors &descriptors ) {
	SSIZE_T offset = 0;

	WORD compatibilityLen = RW(data,offset);
	if (!compatibilityLen) {
		//	Ignore if not present
		return 2;
	}
	WORD descriptorCount  = RW(data,offset);

	printf( "[dsmcc::compatiblity] Compatibility descriptor: dataLen=%ld, descLen=%d, count=%d\n",
	 	len, compatibilityLen, descriptorCount );

	//	Check if len is ok
	if (len < compatibilityLen) {
		printf( "[dsmcc::compatiblity] No data available to parse Compatibility descriptor: descLen=%ld, available=%d\n",
			len, compatibilityLen );
		return compatibilityLen+2;
	}

	//	Parse descriptors
	for (WORD desc=0; desc<descriptorCount; desc++) {
		Descriptor desc;

		desc.type      = RB(data,offset);
		
		//BYTE dLen      = RB(data,offset);
		offset += 1;
		
		desc.specifier = RDW(data,offset);
		desc.model     = RW(data,offset);
		desc.version   = RW(data,offset);

		//	Parse sub descriptors
		BYTE subCount = RB(data,offset);		
		for (BYTE sub=0; sub<subCount; sub++) {
			BYTE subType = RB(data,offset);
			BYTE subLen  = RB(data,offset);
			//	AdditionalInformation
			offset += subLen;
			printf( "[dsmcc::compatibility] Warning, subdescriptor not parsed: count=%d, type=%x, len=%x\n", subCount, subType, subLen );
		}

		descriptors.push_back( desc );
	}
	
	return compatibilityLen+2; 
}

}

namespace module {

SSIZE_T parse( BYTE *data, SIZE_T /*len*/, Modules &modules, bool skipVersion/*=false*/ ) {
	SSIZE_T offset = 0;
	
	WORD numberOfModules = RW(data,offset);
    for (int mod=0; mod<numberOfModules; mod++) {
		Type module;

        module.id      = RW(data,offset);
        module.size    = RDW(data,offset);
		module.version = (skipVersion) ? 0 : RB(data,offset);
        BYTE moduleInfoLen = RB(data,offset);
        if (moduleInfoLen) {
            module.info.copy( (char *)(data+offset), moduleInfoLen );
            offset += moduleInfoLen;
        }

		modules.push_back( module );
    }

	return offset;
}

void parseDescriptors( const util::Buffer &info, desc::MapOfDescriptors &descriptors ) {
	SIZE_T len = info.length();
	BYTE *data = (BYTE *)info.buffer();
	SSIZE_T offset = 0;

	while (offset < len) {
		BYTE dTag = RB(data,offset);
		BYTE dLen = RB(data,offset);
		SSIZE_T parsed = offset;

		printf( "[dsmcc::module] Parse tag descriptor: tag=%x, len=%d\n", dTag, dLen );

		switch (dTag) {
			case MODULE_DESC_TYPE:	//	Type descriptor
			{
				std::string type( (char *)(data+parsed), dLen );
				descriptors[dTag] = type;
				parsed += dLen;
				break;
			}
			case MODULE_DESC_NAME:	//	Name descriptor
			{
				std::string name( (char *)(data+parsed), dLen );
				descriptors[dTag] = name;
				parsed += dLen;
				break;
			}
			case MODULE_DESC_INFO:	//	Info descriptor
			{
				module::InfoDescriptor desc;
				desc.language = std::string( (char *)(data+parsed), 3 );
				desc.text     = std::string( (char *)(data+parsed+3), dLen-3 );
				descriptors[dTag] = desc;
				parsed += dLen;
				break;
			}
			case MODULE_DESC_LINK:	//	Module link descriptor
			{
				module::LinkDescriptor desc;
				desc.position = RB(data,parsed);
				desc.moduleID = RW(data,parsed);
				descriptors[dTag] = desc;
				break;
			}
			case MODULE_DESC_CRC32:	//	CRC32 descriptor
			{
				DWORD crc = RDW(data,parsed);
				descriptors[dTag] = crc;				
				break;
			}
			case MODULE_DESC_LOCATION:	//	Location descriptor
			{
				BYTE location = RB(data,parsed);
				descriptors[dTag] = location;				
				break;
			}
			case MODULE_DESC_EST_DOWNLOAD:	//	Estimated download time descriptor
			{
				DWORD est = RDW(data,parsed);
				descriptors[dTag] = est;				
				break;
			}
			case MODULE_DESC_COMPRESSION:	//	Compression type descriptor
			case MODULE_DESC_COMPRESSED:
			{
				module::CompressionTypeDescriptor desc;
				desc.type = RB(data,parsed);
				desc.originalSize = RDW(data,parsed);
				descriptors[dTag] = desc;								
				break;
			}
		};

		offset += dLen;
		if (offset != parsed) {
			printf( "[dsmcc::module] Descriptor not parsed or parsed was incomplete: dTag=%x, len=%d, parsed=%ld\n",
				dTag, dLen, parsed );
		}		
	}

	//printf( "[dsmcc::module] Descriptors parsed: %d\n", descriptors.size() );
}

}
}
}
