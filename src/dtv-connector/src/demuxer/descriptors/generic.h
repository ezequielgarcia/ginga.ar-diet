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

#include "../descriptors.h"
#include "../../service/types.h"
#include <util/buffer.h>
#include <util/any.h>
#include <vector>

namespace tuner {
namespace desc {

//	Service name descriptor
typedef std::string ServiceName;

//	Service list descriptor
typedef std::pair<ID,service::Type> Service;
typedef std::vector<Service> ServiceListDescriptor;

//	Service descriptor
struct ServiceInfo {
	service::Type type;
	std::string provider;
	std::string name;
};
typedef struct ServiceInfo ServiceDescriptor;
void show( const ServiceDescriptor &desc );

//	Parental rating descriptor
struct ParentalRating {
	std::string language;
	BYTE rating;
};
typedef std::vector<struct ParentalRating> ParentalRatingDescriptor;
void show( const ParentalRatingDescriptor &desc );

//	Short event descriptor
struct ShortEvent {
    std::string language;
    std::string event;
    std::string text;
};
typedef struct ShortEvent ShortEventDescriptor;
void show( const ShortEventDescriptor &desc );

//	Content descriptor
struct Content {
	BYTE content;
	BYTE user;
};
typedef std::vector<struct Content> ContentDescriptor;
void show( const ContentDescriptor &desc );

//	Video Decode Descriptor
struct VideoDecodeStruct {
	bool stillPictureFlag;
	bool sequenceEndCodeFlag;
	BYTE videoEncodeFormat;
};
typedef struct VideoDecodeStruct VideoDecodeDescriptor;
void show( const VideoDecodeDescriptor &desc );


//	Application Signalling Descriptor
struct ApplicationSignallingElementStruct {
	WORD type;
	BYTE version;
};
typedef struct ApplicationSignallingElementStruct ApplicationSignallingElementInfo;
typedef std::vector<ApplicationSignallingElementInfo> ApplicationSignallingDescriptor;
void show( const ApplicationSignallingDescriptor &desc );


//	System Management Descriptor
struct SystemManagementStruct {
	WORD systemID;
	Buffer info;
};
typedef struct SystemManagementStruct SystemManagementDescriptor;
void show( const SystemManagementDescriptor &desc );


//	Terrestrial Delivery System Descriptor
struct TerrestrialDeliverySystemStruct {
	WORD area;
	BYTE interval;
	BYTE mode;
	std::vector<WORD> frequencies;
};
typedef struct TerrestrialDeliverySystemStruct TerrestrialDeliverySystemDescriptor;
void show( const TerrestrialDeliverySystemDescriptor &desc );


//	Partial Reception Descriptor
typedef std::vector<WORD> PartialReceptionDescriptor;
void show( const PartialReceptionDescriptor &desc );


//	Association Tag Descriptor
struct AssociationTagStruct {
	WORD tag;
	WORD use;
};
typedef AssociationTagStruct AssociationTagDescriptor;
void show( const AssociationTagDescriptor &desc );


//	Data Component Descriptor
struct DataComponentStruct {
	WORD codingMethodID;
	Buffer info;	
};
typedef DataComponentStruct DataComponentDescriptor;
void show( const DataComponentDescriptor &desc );

//	Transport Stream Information Descriptor
struct TransmissionTypeStruct {
	BYTE type;
	std::vector<WORD> services;
};
typedef struct TransmissionTypeStruct TransmissionTypeInfo;

struct TransportStreamInformationStruct {
	BYTE remoteControlKeyID;
	std::string name;
	std::vector<TransmissionTypeInfo> transmissions;
};
typedef TransportStreamInformationStruct TransportStreamInformationDescriptor;
void show( const TransportStreamInformationDescriptor &desc );

//	Logo Transmission descriptor
#define LOGO_TYPE1 0x01
#define LOGO_TYPE2 0x02
#define LOGO_TYPE3 0x03
struct LogoTransmissionStruct {
	BYTE type;
	Any data;
};
struct LogoTransmissionType1Struct {
	WORD id;
	WORD version;
	WORD downloadID;
};
typedef struct LogoTransmissionType1Struct LogoTransmissionType1;
typedef WORD                               LogoTransmissionType2;
typedef std::string                        LogoTransmissionType3;
typedef struct LogoTransmissionStruct      LogoTransmissionDescriptor;

void show( const LogoTransmissionDescriptor &desc );

//	Local Time Offset Descriptor
struct LocalTimeOffsetStruct {
	DWORD countryCode;
	BYTE countryRegion;
	bool localTimeOffsetPolarity;
	WORD localTimeOffset;
	QWORD timeOfChange;
	WORD nextTimeOffset;
};
typedef struct LocalTimeOffsetStruct LocalTimeOffset;
typedef std::vector<LocalTimeOffset> LocalTimeOffsetDescriptor;
void show( const LocalTimeOffsetDescriptor &desc );

}
}
