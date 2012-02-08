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

#define CONNECTOR_VERSION 0.0.1.0

#define PSI_TIMEOUT_BASE      4

//	DVB
#define DVB_MIN_SIGNAL        0x6000
#define DVB_MAX_SNR           0x0FFF
#define DVB_BANDWIDTH         6
#define DVB_CHANNELS          55
#define DVB_FIRST_FREQUENCY   473142857

//	Connector connect timeout
#define CONNECTOR_CONNECT_TIMEOUT           10000
#define CONNECTOR_CONNECT_TIMEOUT_RETRY     1000
#define CONNECTOR_CONNECT_TIMEOUT_MAX_RETRY 10

//	Connector keep alive
#define CONNECTOR_KEEP_ALIVE_INTERVAL       500
#define CONNECTOR_KEEP_ALIVE_MAX_RETRY      5

//	Ginga applications
#define GINGA_DEFAULT_PROCESS "/root/ginga/sbin/ginga"
#define GINGA_TIMEOUT         10000
