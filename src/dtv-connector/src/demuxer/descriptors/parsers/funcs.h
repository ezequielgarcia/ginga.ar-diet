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

#include "../demuxer.h"
#include "../../ts.h"

using namespace util;

#define DESC_LANGUAGE(var,ptr,offset)			\
	var.assign( (char *)(ptr+offset), 3 ); offset+=3;

#define DESC_NAME(var,ptr,offset) \
    { \
        BYTE len = RB(ptr,offset); \
        var.assign( (char *)(ptr+offset), len ); \
        offset += len; \
    }

#define DESC_STRING(var,len,ptr,offset)			\
    { \
        var.assign( (char *)(ptr+offset), len ); \
        offset += len; \
    }

namespace tuner {
namespace desc {

bool fnc000Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc001Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc002Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc003Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc004Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc005Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc006Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc007Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc008Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc009Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc00aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc00bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc00cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc00dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc00eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc00fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc010Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc011Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc012Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc013Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc014Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc015Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc016Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc017Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc018Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc019Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc01aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc01bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc01cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc01dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc01eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc01fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc020Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc021Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc022Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc023Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc024Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc025Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc026Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc027Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc028Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc029Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc02aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc02bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc02cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc02dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc02eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc02fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc030Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc031Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc032Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc033Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc034Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc035Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc036Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc037Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc038Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc039Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc03aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc03bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc03cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc03dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc03eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc03fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc040Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc041Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc042Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc043Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc044Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc045Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc046Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc047Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc048Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc049Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc04aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc04bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc04cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc04dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc04eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc04fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc050Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc051Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc052Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc053Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc054Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc055Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc056Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc057Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc058Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc059Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc05aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc05bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc05cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc05dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc05eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc05fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc060Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc061Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc062Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc063Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc064Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc065Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc066Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc067Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc068Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc069Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc06aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc06bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc06cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc06dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc06eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc06fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc070Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc071Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc072Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc073Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc074Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc075Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc076Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc077Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc078Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc079Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc07aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc07bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc07cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc07dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc07eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc07fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc080Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc081Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc082Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc083Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc084Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc085Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc086Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc087Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc088Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc089Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc08aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc08bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc08cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc08dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc08eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc08fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc090Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc091Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc092Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc093Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc094Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc095Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc096Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc097Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc098Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc099Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc09aParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc09bParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc09cParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc09dParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc09eParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc09fParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a0Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a1Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a2Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a3Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a4Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a5Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a6Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a7Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a8Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0a9Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0aaParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0abParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0acParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0adParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0aeParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0afParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b0Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b1Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b2Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b3Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b4Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b5Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b6Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b7Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b8Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0b9Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0baParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0bbParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0bcParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0bdParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0beParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0bfParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c0Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c1Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c2Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c3Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c4Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c5Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c6Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c7Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c8Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0c9Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0caParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0cbParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0ccParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0cdParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0ceParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0cfParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d0Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d1Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d2Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d3Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d4Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d5Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d6Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d7Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d8Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0d9Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0daParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0dbParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0dcParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0ddParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0deParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0dfParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e0Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e1Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e2Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e3Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e4Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e5Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e6Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e7Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e8Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0e9Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0eaParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0ebParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0ecParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0edParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0eeParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0efParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f0Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f1Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f2Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f3Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f4Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f5Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f6Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f7Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f8Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0f9Parser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0faParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0fbParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0fcParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0fdParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0feParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );
bool fnc0ffParser( MapOfDescriptors &descriptors, BYTE *descPayload, SIZE_T len );

}
}
