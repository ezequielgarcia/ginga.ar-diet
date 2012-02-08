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
#include "psidemuxer.h"
#include "psi.h"
#include <util/buffer.h>
#include <util/functions.h>
#include <stdio.h>

namespace tuner {

PSIDemuxer::PSIDemuxer( ID pid )
	: TSSectionDemuxer( pid )
{
	_error = false;
	_buffer = NULL;
	_sections = 0;
	_lastExpired = PSI_INVALID_VERSION;
	_stuffingByteFlag = false;
}

PSIDemuxer::~PSIDemuxer()
{
	delete _buffer;
}

//	Getters
ID PSIDemuxer::tableID() const {
	return PSI_TID_FORBIDDEN;
}

WORD PSIDemuxer::maxSectionBytes() const {
	return TSS_ISO_MAX_BYTES;
}

DWORD PSIDemuxer::frequency() const {
	//	ABNT NBR 15603-2:2007; Table 6 in One o more every n in miliseconds
	return 0;	//	Default 0 seconds
}

bool PSIDemuxer::hasSubtable() const {
	return false;
}

//	Operations
void PSIDemuxer::startData( BYTE *data, SIZE_T len ) {
	//	Loop for payload parsing psi sections or stuffing bit
	SSIZE_T offset=0;
	while (offset < len && *(data+offset) != PSI_STUFFING_BYTE) {		
		//	Parse section header
		offset += startHeader( data+offset, len-offset );
	}
}

SSIZE_T PSIDemuxer::startHeader( BYTE *payload, SIZE_T len ) {
	SSIZE_T bytesUsed=0;

	//	Mark not error
	_error = false;
	
	//	Mark stuffing bytes flags
	_stuffingByteFlag = false;

	//	Check if a complete section is present in payload
	bytesUsed = endSection( payload, len );

	//printf( "[PSIDemuxer] StartHeader: pid=%x, len=%ld, bytesUsed=%ld\n", pid(), len, bytesUsed );
	
	//	Not complete?
	if (!bytesUsed) {
		Buffer *buf = getBuffer();

		//	Check if section was already opened
		if (buf->length() > 0) {
			//	Close section
			buf->resize( 0 );
			printf( "[PSIDemuxer] Warning: section already opened\n" );
		}
		//	add data to new section
		buf->append( (char *)payload, len );
		bytesUsed = len;
	}

	return bytesUsed;
}

void PSIDemuxer::pushData( BYTE *tsPayload, SIZE_T len ) {
	Buffer *buf = getBuffer();

	//	Section is open?
	if (buf->length() > 0) {
		//	Copy payload to buffer
		buf->append( (char *)tsPayload, len );

		//	Check if section was finished ...
		if (endSection( (BYTE *)buf->buffer(), buf->length() ) > 0) {
			buf->resize( 0 );
		}
	}
	else if (_error) {
		printf( "[PSIDemuxer] Warning: Section start with error, ignored\n" );
	}
	else if (!_stuffingByteFlag) {
		Buffer other( (char *)tsPayload, len, false );
		printf( "[PSIDemuxer] Warning: Section not opened pid=%04x, len=%ld, buf=%s\n",
			pid(), len, other.asHexa().c_str() );
	}
}

SSIZE_T PSIDemuxer::endSection( BYTE *payload, SIZE_T len ) {
	SSIZE_T bytesUsed=0;

	//	Generic header present (len included)
	if (len > PSI_GENERIC_HEADER) {
		//	Check section len
		SIZE_T sLen = PSI_SECTION_LENGTH(payload);
		if (sLen > TSS_PRI_MAX_BYTES) { //	Can be implemented via virtual method ... its not worth
			printf( "[PSIDemuxer] Warning: Section len exceded. stuffingFlag=%d, len=%ld, sLen=%ld\n",
				_stuffingByteFlag, len, sLen );

			_error = true;
			return sLen;
		}

		//	Check tableID
		ID tID = PSI_TABLE(payload);
		if (tID == PSI_TID_FORBIDDEN) {
			printf( "[PSIDemuxer] Warning: Table ID forbidden: pid=%x, tid=%x\n", pid(), tID );
			_error = true;
			return sLen;
		}
		else if (tID > ABNT_TID_FORBIDDEN) {
			printf( "[PSIDemuxer] Warning: Table ID forbidden: pid=%x, tid=%x\n", pid(), tID );
			//return sLen;
		}

		//	Only filter tableID if that is not forbidden
		ID filterTableID = tableID();
		if (filterTableID != PSI_TID_FORBIDDEN && filterTableID != tID) {
			_error = true;
			printf( "[PSIDemuxer] Warning: Table ID forbidden or ignored by filter: pid=%x, section tid=%x, filter tid=%x\n",
				pid(), tID, filterTableID );			
			return sLen;
		}

		//	Check syntax of section
		bool packetSyntax = PSI_SYNTAX(payload);
		if (packetSyntax != syntax()) {
			printf( "[PSIDemuxer] Warning: Syntax not valid for this psi section: pid=%x, syntax=%d, tid=%x, len=%ld, sectionLen=%ld\n",
				pid(), packetSyntax, tID, len, sLen );
			_error = true;
			return sLen;
		}

		//	Is section complete?
		if (len >= sLen) {
			int packetLen = sLen - PSI_CRC_SIZE;

			//	If generic syntax
			if (packetSyntax) {
				//	Is CRC correct?
				bool crcOK = needCheckCRC() ? checkCRC( payload, packetLen ) : true;
				if (crcOK) {
					if (useStandardVersion()) {
						BYTE curSection	 = PSI_NUMBER(payload);
						BYTE lastSection = PSI_LAST(payload);

						//	Check if the version was changed ...
						if (versionChanged(payload)) {
							if (supportMultipleSections()) {
								parseSection( payload, packetLen );
							}
							else if (!curSection && !lastSection) {
								onSection( payload, packetLen );
							}
							else {
								printf( "[PSIDemuxer] Warning, demuxer not support multiple sections, but cur y last is not 0: pid=%04x, cur=%02x, last=%02x\n",
									pid(), curSection, lastSection );
							}
						}
						else if (supportMultipleSections()) {
							if (!sectionParsed(curSection)) {
								parseSection( payload, packetLen );
							}
						}
					}
					else {
						//	Call custom parser!
						onSection( payload, packetLen );
					}
				}
				else {
					printf( "[PSIDemuxer] Warning: CRC error!\n" );
				}
			}
			else {
				//	Private table, continue parsing (No CRC check!)
				onSection( payload, packetLen );
			}

			//	Section finished
			bytesUsed = sLen;
			SIZE_T resto = len - bytesUsed;
			_stuffingByteFlag =	 (resto > 0 && payload[bytesUsed] == PSI_STUFFING_BYTE);
			//printf( "[PSIDemuxer] End section=%x, len=%ld, sectionLen=%ld, resto=%ld, stuffing=%d\n", pid(), len, sLen, resto, _stuffingByteFlag );
		}
	}

	// int x = PSI_SECTION_LENGTH(payload);
	// printf( "[PSIDemuxer] End section=%x, len=%ld, secLen=%d, used=%ld, syntax=%d\n", pid(), len, x, bytesUsed, PSI_SYNTAX(payload) );

	return bytesUsed;
}

bool PSIDemuxer::checkCRC( BYTE *payload, int packetLen ) {
	DWORD crc  = GET_CRC( payload + packetLen );
	DWORD calc = util::crc_calc( -1, payload, packetLen );
	return (crc == calc);
}

bool PSIDemuxer::useStandardVersion() const {
	return true;
}

Version PSIDemuxer::updateVersion( Version /*ver*/, BYTE */*section*/, bool /*apply*/ ) {
	return PSI_INVALID_VERSION;
}

void PSIDemuxer::onVersionChanged( void ) {
	_sections = 0;
	_secsParsed.clear();
}

bool PSIDemuxer::versionChanged( BYTE *section ) {
	bool result=false;

	bool apply		   = PSI_NEXT(section);
	Version newVersion = PSI_VERSION(section);
	Version oldVersion = updateVersion( newVersion, section, apply );
	if (newVersion != oldVersion) {
		expire( oldVersion );

		//	Section is applicable (current_next_indicator)
		if (apply) {
			result=true;
			onVersionChanged();
		}

		printf( "[PSIDemuxer] Version changed: pid=%04x, old=%04x, new=%04x, lastExpired=%04x, apply=%d\n",
			pid(), oldVersion, newVersion, _lastExpired, result );
	}
	return result;
}

bool PSIDemuxer::supportMultipleSections( void ) const {
	return syntax() && useStandardVersion();
}

bool PSIDemuxer::sectionParsed( BYTE num ) const {
	bool result=false;
	if (num < _secsParsed.size()) {
		result=_secsParsed[num];
	}
	return result;
}

void PSIDemuxer::parseSection( BYTE *section, SIZE_T len ) {
	//	Get current and last section number
	BYTE curSection	 = PSI_NUMBER(section);
	WORD lastSection = PSI_LAST(section)+1;

	//	First section to parse?
	bool parse=true;
	if (!_sections && lastSection) {
		_secsParsed.resize( lastSection, false );
	}
	else if (_secsParsed.size() != lastSection || curSection > lastSection) {
		parse=false;
	}

	printf( "[PSIDemuxer] Parse sections: sections=%d, cur=%x, last=%x, parse=%d\n",
		_sections, curSection, lastSection, parse );

	if (parse) {
		//	Mark section parsed ...
		_secsParsed[curSection] = true;
		_sections++;

		//	Parse section
		onSection( section, len );

		//	If section complete?
		if (lastSection == _sections) {
			onComplete( section, len );
		}
	}
	else {
		printf( "[PSIDemxuer] Warning, last section changed without a change in the version. Ignoring section\n" );
	}
}

void PSIDemuxer::onComplete( BYTE */*section*/, SIZE_T /*len*/ ) {
}

bool PSIDemuxer::syntax() const {
	return true;
}

Buffer *PSIDemuxer::getBuffer() {
	if (!_buffer)  {
		_buffer = new Buffer( TSS_PRI_MAX_BYTES );
	}
	return _buffer;
}

void PSIDemuxer::onTimeout( const TimeoutCallback &callback ) {
	_onTimeout = callback;
}

void PSIDemuxer::timeoutExpired( void ) {
	printf( "[PSIDemuxer] Warning, timeout expired: pid=%04x\n", pid() );
	if (!_onTimeout.empty()) {
		_onTimeout( pid() );
	}
}

void PSIDemuxer::onExpired( const ExpiredCallback &callback ) {
	_onExpired = callback;
}

void PSIDemuxer::expire( Version versionExpired ) {
	if (!_onExpired.empty() && versionExpired != _lastExpired) {
		_onExpired( pid() );
		_lastExpired = versionExpired;
	}
}

bool PSIDemuxer::checkPID( ID pid, ID ignore/*=0xFFFF*/ ) {
	bool result;
	
	if (ignore != 0xFFFF && pid == ignore) {
		result=true;
	}
	else {
		switch (pid) {
			case TS_PID_NULL:
			case TS_PID_PAT:
			case TS_PID_CAT:
			case TS_PID_TSDT:
			case TS_PID_SDTT_LOW:
			case TS_PID_SDTT_HIGH:
			case TS_PID_NIT:
			case TS_PID_SDT:
			case TS_PID_EIT:
			case TS_PID_TOT:
				result=false;
				break;
			default:
				result=true;
				break;
		};
	}
	return result;
}

}
