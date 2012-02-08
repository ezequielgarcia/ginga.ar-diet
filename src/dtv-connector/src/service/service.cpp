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
#include "service.h"
#include "../demuxer/ts.h"
#include "../demuxer/descriptors.h"
#include "../demuxer/descriptors/generic.h"
#include "../demuxer/psi/nit.h"
#include "../demuxer/psi/sdt.h"
#include "../demuxer/psi/pmt.h"
#include <util/string.h>
#include <boost/foreach.hpp>
#include <stdio.h>

namespace tuner {

Service::Service( void )
{
	_id          = 0;
	_pid         = TS_PID_NULL;
	_pcrPID      = TS_PID_NULL;
	_type        = service::type::digital_television;
	_eitSchedule = false;
	_eitPresent  = false;
	_status      = service::status::undefined;
	_state       = service::state::present;
	_caMode      = false;
	_remote      = 0;
}

Service::Service( ID programID, ID pid )
{
	_id          = programID;
	_pid         = pid;
	_pcrPID      = TS_PID_NULL;
	_type        = service::type::digital_television;
	_eitSchedule = false;
	_eitPresent  = false;	
	_status      = service::status::undefined;
	_state       = service::state::present;	
	_caMode      = false;
	_remote      = 0;
}

Service::~Service( void )
{
}

ID Service::id() const {
	return _id;
}

ID Service::pid() const {
	return _pid;
}

service::Type Service::type() const {
	return _type;
}

const std::string &Service::provider() const {
	return _provider;
}

const std::string &Service::name() const {
	return _name;
}

const std::string &Service::networkName() const {
	return _nitName;
}

const std::vector<Pmt::ElementaryInfo> &Service::elements() const {
	return _elements;
}

int Service::remote() const {
	return _remote;
}

const std::string &Service::tsName() const {
	return _tsName;
}

std::string Service::logicalChannel() const {
	//	ABNT NBR 15603-2 Annex G and H
	return util::format( "%02d.%01d%01d",
		_remote, (_id & 0x0018) >> 3, (_id & 0x0007)+1 );
}

ID Service::pcrPID() const {
	return _pcrPID;
}

service::status::type Service::status() const {
	return _status;
}

service::state::type Service::state() const {
	return _state;
}

void Service::state( service::state::type st ) {
	_state = st;
}

void Service::show() const {
	printf( "[Service] Service: ID=%x, type=%02x, name=%s, provider=%s\n", _id, _type, _name.c_str(), _provider.c_str() );
	printf( "[Service]\t pid=%x, pcrPID=%x, elements=%d\n", _pid, _pcrPID, _elements.size() );
	printf( "[Service]\t TS: name=%s, remote=%d, logicalChannel=%s\n", _tsName.c_str(), _remote, logicalChannel().c_str() );
	printf( "[Service]\t Network: name=%s\n", _nitName.c_str() );
	printf( "[Service]\t status=%x, EIT flags(schedule=%d,present=%d), caMode=%d\n", _status, _eitSchedule, _eitPresent, _caMode );
}

//	Process tables
bool Service::process( Pmt *pmt ) {
	bool changed=false;
	
	if (pmt->programID() == _id &&
		pmt->pid() == _pid)
	{
		_pcrPID   = pmt->pcrPID();
		_elements = pmt->elements();
		_state    = service::state::complete;
		changed   = true;
	}
	
	return changed;
}

bool Service::process( Nit *nit ) {
	bool changed = false;
	
	if (nit->actual()) {
		//	Process NIT network descriptors
		changed |= process( nit->descriptors() );
	
		//	Process NIT networks
		BOOST_FOREACH( Nit::ElementaryInfo info, nit->elements() ) {
			//	Process NIT service descriptor
			changed |= process( info.descriptors );
		}
	}

	return changed;
}

bool Service::process( Sdt *sdt ) {
	bool changed = false;
	
	//	SDT is for actual network?
	if (sdt->actual()) {
		//	Process SDT services
		const std::vector<Sdt::Service> &services = sdt->services();
		BOOST_FOREACH( Sdt::Service srv, services ) {
			if (srv.serviceID == _id) {
				//	Update information
				_eitSchedule = srv.eitSchedule;
				_eitPresent  = srv.eitPresentFollowing;				
				_status      = srv.status;
				_caMode      = srv.caMode;

				//	Process descriptors!
				changed |= process( srv.descriptors );
			}
		}
	}

	return changed;
}

bool Service::process( desc::MapOfDescriptors &descriptors ) {
	bool changed = false;
	
	BOOST_FOREACH( desc::DescriptorItem itDesc, descriptors ) {
		util::BYTE tag = itDesc.first;

		switch (tag) {
			case desc::ident::network_name: {
				//	Network Name descriptor (0x40)
				_nitName = itDesc.second.get<std::string>();
				changed  = true;
				break;
			}
			case desc::ident::service_list: {
				//	Service List Descriptor (0x41)
				const desc::ServiceListDescriptor &serviceList = itDesc.second.get<desc::ServiceListDescriptor>();
				BOOST_FOREACH( desc::Service srv, serviceList ) {
					if (srv.first == _id) {
 						_type   = srv.second;
						changed = true;
					}
				}
				break;
			}
			case desc::ident::service: {
				//	Service Descriptor (0x48)
				const desc::ServiceDescriptor &srvDesc = itDesc.second.get<desc::ServiceDescriptor>();
				_type     = srvDesc.type;
				_name     = srvDesc.name;
				_provider = srvDesc.provider;
				changed   = true;
				break;
			}
			case desc::ident::ts_information: {
				//	TS Information Descriptor
				const desc::TransportStreamInformationDescriptor &desc = itDesc.second.get<desc::TransportStreamInformationDescriptor>();
				_remote = desc.remoteControlKeyID;
				_tsName = desc.name;
				changed = true;
				break;
			}
			default:
				printf( "[Service] Warning: descriptor not processsed: %x\n", tag );
		};
	}

	return changed;
}

}
