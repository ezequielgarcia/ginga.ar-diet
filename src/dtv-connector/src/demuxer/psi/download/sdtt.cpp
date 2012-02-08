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
#include "sdtt.h"
#include <boost/foreach.hpp>
#include <stdio.h>

namespace tuner {

Sdtt::Sdtt( Version version, ID modelID, ID tsID, ID netID, ID serviceID, std::vector<ElementaryInfo> elements )
{
	_version   = version;
	_modelID   = modelID;
	_tsID      = tsID;
	_nitID     = netID;
	_serviceID = serviceID;
	_elements  = elements;
}

Sdtt::~Sdtt( void )
{
}

//  Getters
Version Sdtt::version() const {
	return _version;
}

ID Sdtt::modelID() const {
	return _modelID;
}

ID Sdtt::tsID() const {
	return _tsID;
}

ID Sdtt::nitID() const {
	return _nitID;
}

ID Sdtt::serviceID() const {
	return _serviceID;
}

const std::vector<Sdtt::ElementaryInfo> &Sdtt::elements() const {
	return _elements;
}

void Sdtt::show() const {
	printf( "[Sdtt] SDTT: version=%d, tsID=%x, nitID=%x, modelID=%04x, serviceID=%x\n", _version, _tsID, _nitID, _modelID, _serviceID );
	
	BOOST_FOREACH( Sdtt::ElementaryInfo info, _elements ) {
		printf( "[Sdtt]\tgroup=%x, targetVer=%x, newver=%x, downloadLevel=%x, versionIndicator=%x, timeShift=%x\n",
			info.group, info.targetVersion, info.newVersion, info.downloadLevel, info.versionIndicator, info.scheduleTimeShiftInfo );

		//	Show scheduleInfo
		printf( "[Sdtt]\tSchedule Info:\n" );
		BOOST_FOREACH( Sdtt::ScheduleElementaryInfo &sched, info.schedules ) {
			printf( "[Sdtt]\t\tstartTime=%llx, duration=%lx\n", sched.startTime, sched.duration );
		}

		//	Show DownloadContentDescriptor
		printf( "[Sdtt]\tContents descriptors:\n" );
		BOOST_FOREACH( const DownloadContentDescriptor &desc, info.contents ) {
			printf( "[Sdtt]\t\treboot=%d, addOn=%d, componentSize=%ld, downloadID=%lx, timeoutDII=%ld, leakRate=%08lx, componentTag=%02x, lang=%s, text=%s\n",
				desc.reboot, desc.addOn, desc.componentSize, desc.downloadID,
				desc.timeoutDII, desc.leakRate, desc.componentTag, desc.lang.c_str(), desc.text.c_str() );

			if (desc.hasCompatibility) {
				//	Show Compatibility descriptor
				BOOST_FOREACH( const dsmcc::compatibility::Descriptor &comp, desc.compatibilities ) {
					printf( "[Sdtt]\t\t\tCompatibility: type=%02x, model=%04x, version=%04x, specifier=%08lx\n",
						comp.type, comp.model, comp.version, comp.specifier );
					printf( "[Sdtt]\t\t\tIdentification field: type=%02x, data=%08lx, marker=%02x, model=%02x, group=%02x, version=%04x\n",
						SPECIFIER_TYPE(comp.specifier), SPECIFIER_DATA(comp.specifier),
						MAKER_ID(comp.model), MODEL_ID(comp.model),
						GROUP_ID(comp.version), VERSION_ID(comp.version) );
				}
			}

			if (desc.hasModuleInfo) {
				//	Show Modules
				BOOST_FOREACH( const dsmcc::module::Type &module, desc.modules ) {
					printf( "[Sdtt]\t\t\tModules: id=%x, size=%ld, version=%x, info=%d\n",
						module.id, module.size, module.version, module.info.length() );
				}
			}
		}
	} 
}

}
