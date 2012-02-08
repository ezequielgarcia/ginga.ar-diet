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
#include "tsdemuxer.h"
#include "psifilter.h"
#include "../../demuxer/ts.h"
#include "../../demuxer/psi/psi.h"
#include "../../demuxer/psi/psidemuxer.h"
#include <util/resourcepool.h>
#include <util/buffer.h>
#include <boost/foreach.hpp>
#include <stdio.h>

namespace tuner {

TSDemuxer::TSDemuxer()
{
	//	Create ts pool
	_local = new util::Buffer(TS_PACKET_SIZE);
}

TSDemuxer::~TSDemuxer()
{
	delete _local;
}

bool TSDemuxer::startFilter( PSIFilter *filter ) {
	bool result=false;
	assert( filter );
	
	//	printf( "[TSDemuxer] start filter: filter=%p, pid=%04x\n", filter, filter->pid() );	

	_mutex.lock();
	MapOfFilters::const_iterator it = _filters.find( filter->pid() );
	if (it == _filters.end()) {
		_filters[filter->pid()] = filter;
		result=true;
	}
	_mutex.unlock();
	return result;
}

void TSDemuxer::stopFilter( PSIFilter *filter ) {
	//	printf( "[TSDemuxer] stop filter %p\n", filter );
	
	_mutex.lock();
	MapOfFilters::iterator it=_filters.find( filter->pid() );
	if (it != _filters.end()) {
		_filters.erase( it );
	}
	_mutex.unlock();
}

PSIFilter *TSDemuxer::checkContinuity( ID pid, bool tsStart, util::BYTE actualCC ) {
	PSIFilter *filter=NULL;
	
	MapOfFilters::iterator it=_filters.find( pid );
	if (it != _filters.end()) {
		filter = (*it).second;
		
		//	Check continuity counter
		if (!filter->checkContinuity( tsStart, actualCC )) {
			filter = NULL;
		}
	}

	return filter;		
}

void TSDemuxer::reset() {
	//	printf( "[TSDemuxer] Reset TS Demuxer\n" );
	//	Reset local buffer
	_local->resize( 0 );
}

void TSDemuxer::parse( util::Buffer *buf ) {
	SSIZE_T offset = 0;
	SSIZE_T bufLen = buf->length();

	//printf( "[TSDemuxer] push data: buf=%p, size=%d\n", buf, buf->length() );

	//	Process local buffer ...
	while (_local->length() && offset < bufLen) {
		//	Copy necesary data to complete local buffer
		SSIZE_T need = TS_PACKET_SIZE - _local->length();
		offset		 = (need > bufLen) ? bufLen : need;
		_local->append( buf->buffer(), offset );
		if (_local->length() >= TS_PACKET_SIZE) {
			//	Parse local buffer
			SSIZE_T parsed = parse( _local, 0 );
			if (parsed == static_cast<SSIZE_T>(_local->length())) {
				_local->resize( 0 );
			}
			else if (parsed) {
				assert(true);
			}
		}
	}

	//	Parse rest of buffer
	if (offset < bufLen) {
		SSIZE_T parsed = parse( buf, offset );
		SSIZE_T rest   = bufLen-parsed;
		if (rest) {
			_local->append( buf->buffer()+parsed, rest );
		}
		//printf( "[TSDemuxer] Parsed buffer: offset=%ld, parsed=%ld, rest=%ld\n", offset, parsed, rest );
	}
}

//	input parser: Parse a TS packet Documentation at iso13818-1.pdf
SSIZE_T TSDemuxer::parse( Buffer *buf, SSIZE_T parsed ) {
	BYTE *ptr	= (BYTE *)buf->buffer();
	SIZE_T size = buf->length();

	//printf( "[TSDemuxer] Begin parse: offset=%ld, bufLen=%ld, local=%d\n", parsed, size, (_local == buf) );			
	
	while (parsed < size) {
		{	//	Find TS SYNC byte
			SSIZE_T begin=parsed;
			while (ptr[parsed] != TS_SYNC && parsed < size) {
				parsed++;
			}
			if (parsed - begin) {
				Buffer show( buf->buffer()+begin, parsed+10, false );
				printf( "[TSDemuxer] Warning: Sync lost offset=%ld, size=%ld, count=%ld, isLocal=%d, data=%s\n",
					begin, size, parsed-begin, (_local == buf), show.asHexa().c_str() );
			}
		}

		//	is the begin of TS packet!
		if (parsed < size) {
			int len	 = size - parsed;
			BYTE *ts = ptr + parsed;

			//	is ths TS complete?
			if (len >= TS_PACKET_SIZE) {
				ID pid = TS_PID(ts);

				//	Check for Transport Error Indicator (TES), payload exist, and null packets!!
				if (!TS_HAS_ERROR(ts) && TS_HAS_PAYLOAD(ts) && pid != TS_PID_NULL) {
					int payloadOffset = TS_HEAD_SIZE;
					
					//	Adaptation field exists?
					if (TS_HAS_ADAPTATION(ts)) {
						//	Only calculate payload offset if adaptation field exist
						payloadOffset += TSA_LEN(ts);
					}

					//	Check payload offset
					if (payloadOffset < TS_PACKET_SIZE) {
						bool startFlag = TS_START(ts);
						
						//	Find filter (and check continuity bit)
						_mutex.lock();
						PSIFilter *filter = checkContinuity( pid, startFlag, TS_CONTINUITY(ts) );
						if (filter) {
							BYTE *tsPayload	 = ts+payloadOffset;
							int tsPayloadLen = TS_PACKET_SIZE-payloadOffset;

							//	Begin of a section?
							if (startFlag) {
								//	Get pointer field, skip them in payload and len
								BYTE pointerField = tsPayload[0];
								tsPayload++;
								tsPayloadLen--;
								
								//	Check pointer field
								if (!pointerField || pointerField < tsPayloadLen) {
									if (pointerField) {
										//	Append last block of a section
										filter->pushData( tsPayload, pointerField );

										//	Skip data marked via pointer field
										tsPayload	 += pointerField;
										tsPayloadLen -= pointerField;
									}
									//	TODO: Can start more than one section/pes packet

									//	Start a new section
									filter->startData( tsPayload, tsPayloadLen );
								}
								else {
									printf( "[TSDemuxer] Warning: Pointer field invalid pointer=%d, tsPayloadLen=%d\n", pointerField, tsPayloadLen );
								}
							}
							else {
								//	Add payload to current section
								filter->pushData( tsPayload, tsPayloadLen );
							}

							if (TS_PRIORITY(ts)) {	//	TODO: Priority not processed
								printf( "[TSDemuxer] Warning: Priority not processed\n" );
							}
						}
						_mutex.unlock();
					}
					else {
						printf( "[TSDemuxer] Warning: Transport stream payload not valid\n" );
					}
				}

				parsed += TS_PACKET_SIZE;				
			}
			else {
				//	break loop
				break;
			}
		}
	}

	//printf( "[TSDemuxer] End parse: parsed=%ld\n", parsed );
	
	return parsed;
}

}

