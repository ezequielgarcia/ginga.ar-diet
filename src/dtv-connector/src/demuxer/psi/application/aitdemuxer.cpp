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
#include "aitdemuxer.h"
#include "../psi.h"
#include <util/string.h>
#include <stdio.h>

//	Implemented based on
//		- ARIB STD - B23 Part2
//		- ETSI TS 102 809 V.1.1.1 (2010-01)

#define LOOP_LEN(v,s,o)   WORD v = RW((s),(o)); v &= 0x0FFF;
#define READ_STR(v,s,b,o) v = std::string( (char *)((b)+(o)), s ); (o) += s;
#define READ_STRL(v,b,o)  { BYTE nLenTemp = RB((b),(o)); READ_STR(v,nLenTemp,b,o); }

namespace tuner {

AITDemuxer::AITDemuxer( ID pid )
: PSIDemuxer( pid )
{
}

AITDemuxer::~AITDemuxer()
{
}

//	Getters
ID AITDemuxer::tableID() const {
	return PSI_TID_AIT;
}

//  Signals
void AITDemuxer::onParsed( const ParsedCallback &callback ) {
    _onParsed = callback;
}

//  TSSectionDemuxer Virtual method
void AITDemuxer::onSection( BYTE *section, SIZE_T /*len*/ ) {
	SSIZE_T offset=PSI_PAYLOAD_OFFSET;
	std::vector<ait::Application> applications;
	desc::MapOfDescriptors descriptors;

    //  Application Type
    ID appType = PSI_EXTENSION(section);

	//	Common descriptors
	LOOP_LEN(descLen,section,offset);
	if (descLen) {
		offset += parseDescriptors( section+offset, descLen, descriptors );
	}

	//	Parse Applications
	LOOP_LEN(appLoopEnd,section,offset);
	SSIZE_T end = offset + appLoopEnd;
	while (offset < end) {
		ait::Application app;

		//	Parse Application Identifier
		app.orgID    = RDW(section,offset);
		app.id       = RW(section,offset);
		app.ctrlCode = RB(section,offset);
		
		LOOP_LEN(appDescLen,section,offset);
		if (appDescLen) {
			offset += parseDescriptors( section+offset, appDescLen, app.descriptors );
		}

		applications.push_back( app );
	}

    Ait *ait = new Ait( PSI_VERSION(section), appType, applications, descriptors );
    _onParsed( ait );
}

template<typename T>
struct SubtableFinder {
	SubtableFinder( const T &table ) : _table(table) {}
	bool operator()( const T &table ) {
		return _table.isEqual( table );
	}
	const T &_table;
};

template<class T>
Version chgVersion( std::vector<T> &tables, const T &subtable ) {
	bool result=false;
	SubtableFinder<T> finder( subtable );
	Version old = PSI_INVALID_VERSION;	

	//	Find subtable
	typename std::vector<T>::iterator it = std::find_if( tables.begin(), tables.end(), finder );
	if (it != tables.end()) {
		//	Found sub table: check version!
		if ((*it).version != subtable.version) {
			//	Version changed!
			old = (*it).version;
			(*it).version = subtable.version;
		}
    }
	else {
		tables.push_back( subtable );
	}
	return old;
}

Version AITDemuxer::updateVersion( Version ver, BYTE *section, bool apply ) {
	AITDemuxer::Subtable sub;	

	//	Subtable is: pid+tableID(0x74)+appType+version
    sub.appType = PSI_EXTENSION(section);
    sub.version = ver;
	return psi::chgVersion( _tables, sub, apply );	
}
 
SSIZE_T AITDemuxer::parseDescriptors( BYTE *buf, SIZE_T len, desc::MapOfDescriptors &descriptors ) {
	SSIZE_T offset=0;
	SSIZE_T bytes;

	while (offset < len) {
		BYTE dTag = RB(buf,offset);
		BYTE dLen = RB(buf,offset);
		BYTE *ptr = buf+offset;

		if (!dLen) {
			printf( "[AITDemuxer] Warning, descriptor %02x of %d bytes bad formated\n", dTag, dLen );
			continue;
		}

		switch (dTag) {
			case AIT_APP_DESC:
				bytes = parseAppDesc( ptr, dLen, descriptors );
				break;
			case AIT_TRANSPORT_PROTOCOL_DESC:
				bytes = parseTransportProtocolDesc( ptr, dLen, descriptors );
				break;
			case AIT_APP_NAME_DESC:
				bytes = parseAppNameDesc( ptr, dLen, descriptors );
				break;
			case AIT_GINGA_J_APP_DESC:
			case AIT_GINGA_NCL_APP_DESC:
				bytes = parseGingaApp( ptr, dLen, dTag, descriptors );
				break;
			case AIT_GINGA_J_APP_LOCATION_DESC:
			case AIT_GINGA_NCL_APP_LOCATION_DESC:
				bytes = parseGingaAppLocation( ptr, dLen, dTag, descriptors );
				break;
			case AIT_EXTERNAL_APP_AUTH_DESC:
				bytes = parseExternalAppAuth( ptr, dLen, dTag, descriptors );
				break;
			case AIT_APP_ICONS_DESC:
				bytes = parseIconsDescriptor( ptr, dLen, dTag, descriptors );
				break;
			case AIT_GRAPHICS_CONSTRAINTS_DESC:
				bytes = parseGraphicsConstraintsDescriptor( ptr, dLen, dTag, descriptors );
				break;				
			default:
				bytes = 0;
				printf( "[AITDemuxer] Warning, descriptor %02x of %d bytes not parsed\n", dTag, dLen );
		}

		if (bytes && bytes != dLen) {
			printf( "[AITDemuxer] Warning, descriptor %02x incomplete parsed: dLen=%d, bytes=%ld\n", dTag, dLen, bytes );
		}
		
		offset += dLen;
	}

	return len;
}

SSIZE_T AITDemuxer::parseAppDesc( BYTE *buf, SIZE_T len, desc::MapOfDescriptors &descriptors ) {
	SSIZE_T offset=0;
	ait::ApplicationDescriptor app;

	BYTE profilesLen = RB(buf,offset);
	SSIZE_T end = offset + profilesLen;	
	while (offset < end) {
		ait::ApplicationProfile profile;
		
		profile.profile = RW(buf,offset);
		profile.major   = RB(buf,offset);
		profile.minor   = RB(buf,offset);
		profile.micro   = RB(buf,offset);

		app.profiles.push_back( profile );
	}

	BYTE data = RB(buf,offset);
	app.serviceBoundFlag = (data & 0x80) ? true : false;
	app.visibility       = ((data & 0x60) >> 5);
	
	app.priority         = RB(buf,offset);
	
	while (offset < len) {
		BYTE transportProtocolLabel = RB(buf,offset);
		app.transports.push_back( transportProtocolLabel );
	}

	ait::show( app );
	descriptors[AIT_APP_DESC] = app;
	
	return len;
}

SSIZE_T AITDemuxer::parseTransportProtocolDesc( BYTE *buf, SIZE_T len, desc::MapOfDescriptors &descriptors ) {
	SSIZE_T offset = 0;
	ait::TransportDescriptor transports;

	//	Find descriptor
	desc::MapOfDescriptors::iterator it=descriptors.find(AIT_TRANSPORT_PROTOCOL_DESC);
	if (it != descriptors.end()) {
		transports = descriptors[AIT_TRANSPORT_PROTOCOL_DESC].get<ait::TransportDescriptor>();
	}

	ait::TransportProtocolDescriptor desc;
		
	desc.protocolID = RW(buf,offset);
	desc.label      = RB(buf,offset);
	switch (desc.protocolID) {
		case TP_OBJECT_CAROUSEL:
		case TP_DATA_CAROUSEL:
		{
			ait::DSMCCTransportProtocolDescriptor dsmcc;
			
			dsmcc.remote  = RB(buf,offset);
			dsmcc.remote &= 0x80;
			if (dsmcc.remote) {
				dsmcc.networkID = RW(buf,offset);
				dsmcc.tsID      = RW(buf,offset);
				dsmcc.serviceID = RW(buf,offset);
			}
			else {
				dsmcc.networkID = 0;
				dsmcc.tsID      = 0;
				dsmcc.serviceID = 0;
			}
			dsmcc.componentTag = RB(buf,offset);
			desc.info = dsmcc;
			break;
		}
		default:
			printf( "[AITDemuxer] Warning, transport protocol descriptor of protocolID=%04x not parsed\n", desc.protocolID );
	}

	transports.push_back( desc );
	ait::show( transports );
	
	descriptors[AIT_TRANSPORT_PROTOCOL_DESC] = transports;
	
	return len;
}

SSIZE_T AITDemuxer::parseAppNameDesc( BYTE *buf, SIZE_T len, desc::MapOfDescriptors &descriptors ) {
	SSIZE_T offset = 0;
	ait::ApplicationNameDescriptor desc;

	while (offset < len) {
		ait::ApplicationName appName;

		READ_STR(appName.language,3,buf,offset);
		READ_STRL(appName.name,buf,offset);

		desc.push_back( appName );
	}

	ait::show( desc );
	descriptors[AIT_APP_NAME_DESC] = desc;
	
	return len;	
}

SSIZE_T AITDemuxer::parseGingaApp( BYTE *buf, SIZE_T len, BYTE tag, desc::MapOfDescriptors &descriptors ) {
	SSIZE_T offset = 0;
	ait::GingaApplicationDescriptor desc;
	std::string tmp;

	while (offset < len) {
		READ_STRL(tmp,buf,offset);
		desc.push_back( tmp );
	}

	ait::show( desc );
	descriptors[tag] = desc;
	
	return len;
}

SSIZE_T AITDemuxer::parseGingaAppLocation( BYTE *buf, SIZE_T len, BYTE tag, desc::MapOfDescriptors &descriptors ) {
	SSIZE_T offset=0;
	ait::GingaApplicationLocationDescriptor desc;

	READ_STRL(desc.base,buf,offset);
	READ_STRL(desc.classPath,buf,offset);
	BYTE rest=(BYTE)len-offset;
	READ_STR(desc.initialClass,rest,buf,offset);

	ait::show( desc );
	descriptors[tag] = desc;
	
	return len;
}

SSIZE_T AITDemuxer::parseExternalAppAuth( BYTE *buf, SIZE_T len, BYTE tag, desc::MapOfDescriptors &descriptors ) {
	SSIZE_T offset=0;
	ait::ExternalApplications desc;

	while (offset < len) {	
		ait::ExternalApplication app;

		app.orgID    = RDW(buf,offset);
		app.id       = RW(buf,offset);
		app.priority = RB(buf,offset);

		desc.push_back( app );
	}
	ait::show( desc );
	descriptors[tag] = desc;
	return len;
}

SSIZE_T AITDemuxer::parseIconsDescriptor( BYTE *buf, SIZE_T /*len*/, BYTE tag, desc::MapOfDescriptors &descriptors ) {
	SSIZE_T off=0;
	ait::IconsDescriptor desc;

	std::string locator;
	READ_STRL(locator,buf,off);
	WORD flags = RW(buf,off);

	//	Make icon name
	WORD mask=0;
	for (BYTE i=0; i<16; i++) {
		mask = 1 << i;
		if (mask & flags) {
			std::string name = util::format( "%s/dvb.icon.%04x", locator.c_str(), mask );
			desc.push_back( name );
		}
	}
	descriptors[tag] = desc;
	return off;
}

SSIZE_T AITDemuxer::parseGraphicsConstraintsDescriptor( BYTE *buf, SIZE_T len, BYTE tag, desc::MapOfDescriptors &descriptors ) {
	SSIZE_T off=0;
	ait::GraphicsConstraintsStruct desc;

	BYTE data = RB(buf,off);

	desc.canRunWithoutVisibleUI = (data & 0x04) ? true : false;
	desc.handleConfigurationChanged = (data & 0x02) ? true : false;
	desc.handleControlledVideo = (data & 0x01) ? true : false;
	while (off < len) {
		BYTE mode = RB(buf,off);
		desc.supportedModes.push_back( mode );
	}
	descriptors[tag] = desc;
	return off;
}

bool AITDemuxer::supportMultipleSections( void ) const {
	//	TODO: Complete!
	return false;
}

}
