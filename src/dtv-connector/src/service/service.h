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

#include "types.h"
#include "../demuxer/psi/pmt.h"
#include "../demuxer/descriptors.h"
#include <boost/function.hpp>

namespace tuner {

class Pmt;
class Nit;
class Sdt;

class Service {
public:
	Service( void );
	Service( ID programID, ID pid );
	virtual ~Service( void );

	//	Service information
	ID id() const;
	ID pid() const;
	service::Type type() const;
	const std::string &provider() const;
	const std::string &name() const;
	const std::string &networkName() const;
	const std::vector<Pmt::ElementaryInfo> &elements() const;
	ID pcrPID() const;
	service::status::type status() const;
	service::state::type state() const;
	void state( service::state::type );

	//	TS information
	int remote() const;
	const std::string &tsName() const;

	std::string logicalChannel() const;
	void show() const;

	//	Process of Tables
	bool process( Pmt *pmt );
	bool process( Nit *nit );
	bool process( Sdt *sdt );
	bool process( desc::MapOfDescriptors &descriptors );

private:
	ID _id;                //	Service ID
	ID _pid;               //	PID of service
	ID _pcrPID;            //	PCR pid
	service::Type _type;   //	Type of service
	std::string _provider; //	Name of provider
	std::string _name;     //	Name of service
	std::string _tsName;   //	TS name
	std::string _nitName;  //	Network name
	int _remote;           //	Remote control
	bool _eitSchedule;
	bool _eitPresent;
	service::status::type _status;
	service::state::type _state;
	bool _caMode;
	std::vector<Pmt::ElementaryInfo> _elements;
};

}
