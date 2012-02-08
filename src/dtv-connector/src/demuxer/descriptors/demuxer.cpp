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
#include "demuxer.h"
#include "parsers/funcs.h"
#include <util/buffer.h>
#include <stdio.h>

#define USER_PRIVATE_DESCRIPTOR  64
#define DESC_TAG(ptr)            ((ptr)[0])
#define DESC_SIZE(ptr)           ((ptr)[1])
#define DESC_HEADER              2


namespace tuner {
namespace desc {

Demuxer::MapOfDemuxers Demuxer::_demuxers;

Demuxer::Demuxer( void )
{
    if (_demuxers.empty()) {
        loadStaticDemuxer();
    }
}

Demuxer::~Demuxer()
{
}

//  Operations
int Demuxer::parse( MapOfDescriptors &descriptors, BYTE *descPayload ) {
    int offset = 0;

    //  Get descriptor length
    WORD len = GET_DESCRIPTOR_LENGTH(descPayload);
    offset += 2;

	//	printf( "[desc::Demuxer] begin parse: len=%d\n", len );

    //  Find demuxer by descriptor tag
    BYTE *ptr;
    bool result;
    while (offset < len+2) {
		result=false;
        ptr = descPayload+offset;
        MapOfDemuxers::iterator it=_demuxers.find( DESC_TAG(ptr) );
        if (it != _demuxers.end()) {
            //  Found demuxer, parse!
            FncDescriptorDemuxer demuxer = (*it).second;
            result = (*demuxer)( descriptors, ptr+DESC_HEADER, DESC_SIZE(ptr) );
        }
		
        if (!result) {
			util::Buffer text( (char *)ptr, DESC_SIZE(ptr)+DESC_HEADER, false );
            printf( "[desc::Demuxer] Warning: Descriptor of tag=%x, len=%d not parsed: %s\n",
				DESC_TAG(ptr), DESC_SIZE(ptr), text.asHexa().c_str() );
        }
        offset += DESC_SIZE(ptr) + DESC_HEADER;
    }
    return offset;
}

void Demuxer::addDemuxer( BYTE tag, Demuxer::FncDescriptorDemuxer demuxer ) {
    if (tag >= USER_PRIVATE_DESCRIPTOR) {
        _demuxers[tag] = demuxer;
    }
}

void Demuxer::loadStaticDemuxer( void ) {
    //  0-1 -> reserved
    //  2   -> video
    //  3   -> audio
    //  4   -> hierarchy
    //  5 -> registration
    //  6 -> data stream alignment
    //  7 -> target background grid
    //  8 -> Video window
    //  9 -> CA
    // 10 -> ISO-639 languaje
    // 11 -> System clock
    // 12 -> Multiplex buffer utilization
    // 13 -> Copyright
    // 14 -> Maximum bitrate
    // 15 -> Private data indicator
    // 16 -> Smoothing buffer
    // 17 -> STD
    // 18 -> IBP
    // 19/26 -> Defined in ISO/IEC 13818-6
    // 27 -> MPEG-4 video
    // 28 -> MPEG-4 audio
    // 29 -> IOD
    // 30 -> SL
    // 31 -> FMC
    // 32 -> External ES ID
    // 33 -> MuxCode
    // 34 -> Fmx Buffer size
    // 35 -> Multiplex Buffer
    // 36-63 -> ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved
    // 64-255 -> User private
    // _demuxers[0x000] = &fnc000Parser;
    // _demuxers[0x001] = &fnc001Parser;
    // _demuxers[0x002] = &fnc002Parser;
    // _demuxers[0x003] = &fnc003Parser;
    // _demuxers[0x004] = &fnc004Parser;
    // _demuxers[0x005] = &fnc005Parser;
    // _demuxers[0x006] = &fnc006Parser;
    // _demuxers[0x007] = &fnc007Parser;
    // _demuxers[0x008] = &fnc008Parser;
    // _demuxers[0x009] = &fnc009Parser;
    // _demuxers[0x00a] = &fnc00aParser;
    // _demuxers[0x00b] = &fnc00bParser;
    // _demuxers[0x00c] = &fnc00cParser;
    // _demuxers[0x00d] = &fnc00dParser;
    // _demuxers[0x00e] = &fnc00eParser;
    // _demuxers[0x00f] = &fnc00fParser;
    // _demuxers[0x010] = &fnc010Parser;
    // _demuxers[0x011] = &fnc011Parser;
    // _demuxers[0x012] = &fnc012Parser;
    _demuxers[0x013] = &fnc013Parser;
    _demuxers[0x014] = &fnc014Parser;
    // _demuxers[0x015] = &fnc015Parser;
    // _demuxers[0x016] = &fnc016Parser;
    // _demuxers[0x017] = &fnc017Parser;
    // _demuxers[0x018] = &fnc018Parser;
    // _demuxers[0x019] = &fnc019Parser;
    // _demuxers[0x01a] = &fnc01aParser;
    // _demuxers[0x01b] = &fnc01bParser;
    // _demuxers[0x01c] = &fnc01cParser;
    // _demuxers[0x01d] = &fnc01dParser;
    // _demuxers[0x01e] = &fnc01eParser;
    // _demuxers[0x01f] = &fnc01fParser;
    // _demuxers[0x020] = &fnc020Parser;
    // _demuxers[0x021] = &fnc021Parser;
    // _demuxers[0x022] = &fnc022Parser;
    // _demuxers[0x023] = &fnc023Parser;
    // _demuxers[0x024] = &fnc024Parser;
    // _demuxers[0x025] = &fnc025Parser;
    // _demuxers[0x026] = &fnc026Parser;
    // _demuxers[0x027] = &fnc027Parser;
    // _demuxers[0x028] = &fnc028Parser;
    // _demuxers[0x029] = &fnc029Parser;
    // _demuxers[0x02a] = &fnc02aParser;
    // _demuxers[0x02b] = &fnc02bParser;
    // _demuxers[0x02c] = &fnc02cParser;
    // _demuxers[0x02d] = &fnc02dParser;
    // _demuxers[0x02e] = &fnc02eParser;
    // _demuxers[0x02f] = &fnc02fParser;
    // _demuxers[0x030] = &fnc030Parser;
    // _demuxers[0x031] = &fnc031Parser;
    // _demuxers[0x032] = &fnc032Parser;
    // _demuxers[0x033] = &fnc033Parser;
    // _demuxers[0x034] = &fnc034Parser;
    // _demuxers[0x035] = &fnc035Parser;
    // _demuxers[0x036] = &fnc036Parser;
    // _demuxers[0x037] = &fnc037Parser;
    // _demuxers[0x038] = &fnc038Parser;
    // _demuxers[0x039] = &fnc039Parser;
    // _demuxers[0x03a] = &fnc03aParser;
    // _demuxers[0x03b] = &fnc03bParser;
    // _demuxers[0x03c] = &fnc03cParser;
    // _demuxers[0x03d] = &fnc03dParser;
    // _demuxers[0x03e] = &fnc03eParser;
    // _demuxers[0x03f] = &fnc03fParser;
	_demuxers[0x040] = &fnc040Parser;
    _demuxers[0x041] = &fnc041Parser;
    // _demuxers[0x042] = &fnc042Parser;
    // _demuxers[0x043] = &fnc043Parser;
    // _demuxers[0x044] = &fnc044Parser;
    // _demuxers[0x045] = &fnc045Parser;
    // _demuxers[0x046] = &fnc046Parser;
    // _demuxers[0x047] = &fnc047Parser;
    _demuxers[0x048] = &fnc048Parser;
    // _demuxers[0x049] = &fnc049Parser;
    // _demuxers[0x04a] = &fnc04aParser;
    // _demuxers[0x04b] = &fnc04bParser;
    // _demuxers[0x04c] = &fnc04cParser;
    // _demuxers[0x04d] = &fnc04dParser;
    // _demuxers[0x04e] = &fnc04eParser;
    // _demuxers[0x04f] = &fnc04fParser;
    // _demuxers[0x050] = &fnc050Parser;
    // _demuxers[0x051] = &fnc051Parser;
    _demuxers[0x052] = &fnc052Parser;
    // _demuxers[0x053] = &fnc053Parser;
    // _demuxers[0x054] = &fnc054Parser;
    // _demuxers[0x055] = &fnc055Parser;
    // _demuxers[0x056] = &fnc056Parser;
    // _demuxers[0x057] = &fnc057Parser;
    _demuxers[0x058] = &fnc058Parser;
    // _demuxers[0x059] = &fnc059Parser;
    // _demuxers[0x05a] = &fnc05aParser;
    // _demuxers[0x05b] = &fnc05bParser;
    // _demuxers[0x05c] = &fnc05cParser;
    // _demuxers[0x05d] = &fnc05dParser;
    // _demuxers[0x05e] = &fnc05eParser;
    // _demuxers[0x05f] = &fnc05fParser;
    // _demuxers[0x060] = &fnc060Parser;
    // _demuxers[0x061] = &fnc061Parser;
    // _demuxers[0x062] = &fnc062Parser;
    // _demuxers[0x063] = &fnc063Parser;
    // _demuxers[0x064] = &fnc064Parser;
    // _demuxers[0x065] = &fnc065Parser;
    // _demuxers[0x066] = &fnc066Parser;
    // _demuxers[0x067] = &fnc067Parser;
    // _demuxers[0x068] = &fnc068Parser;
    // _demuxers[0x069] = &fnc069Parser;
    // _demuxers[0x06a] = &fnc06aParser;
    // _demuxers[0x06b] = &fnc06bParser;
    // _demuxers[0x06c] = &fnc06cParser;
    // _demuxers[0x06d] = &fnc06dParser;
    // _demuxers[0x06e] = &fnc06eParser;
    _demuxers[0x06f] = &fnc06fParser;
    // _demuxers[0x070] = &fnc070Parser;
    // _demuxers[0x071] = &fnc071Parser;
    // _demuxers[0x072] = &fnc072Parser;
    // _demuxers[0x073] = &fnc073Parser;
    // _demuxers[0x074] = &fnc074Parser;
    // _demuxers[0x075] = &fnc075Parser;
    // _demuxers[0x076] = &fnc076Parser;
    // _demuxers[0x077] = &fnc077Parser;
    // _demuxers[0x078] = &fnc078Parser;
    // _demuxers[0x079] = &fnc079Parser;
    // _demuxers[0x07a] = &fnc07aParser;
    // _demuxers[0x07b] = &fnc07bParser;
    // _demuxers[0x07c] = &fnc07cParser;
    // _demuxers[0x07d] = &fnc07dParser;
    // _demuxers[0x07e] = &fnc07eParser;
    // _demuxers[0x07f] = &fnc07fParser;
    // _demuxers[0x080] = &fnc080Parser;
    // _demuxers[0x081] = &fnc081Parser;
    // _demuxers[0x082] = &fnc082Parser;
    // _demuxers[0x083] = &fnc083Parser;
    // _demuxers[0x084] = &fnc084Parser;
    // _demuxers[0x085] = &fnc085Parser;
    // _demuxers[0x086] = &fnc086Parser;
    // _demuxers[0x087] = &fnc087Parser;
    // _demuxers[0x088] = &fnc088Parser;
    // _demuxers[0x089] = &fnc089Parser;
    // _demuxers[0x08a] = &fnc08aParser;
    // _demuxers[0x08b] = &fnc08bParser;
    // _demuxers[0x08c] = &fnc08cParser;
    // _demuxers[0x08d] = &fnc08dParser;
    // _demuxers[0x08e] = &fnc08eParser;
    // _demuxers[0x08f] = &fnc08fParser;
    // _demuxers[0x090] = &fnc090Parser;
    // _demuxers[0x091] = &fnc091Parser;
    // _demuxers[0x092] = &fnc092Parser;
    // _demuxers[0x093] = &fnc093Parser;
    // _demuxers[0x094] = &fnc094Parser;
    // _demuxers[0x095] = &fnc095Parser;
    // _demuxers[0x096] = &fnc096Parser;
    // _demuxers[0x097] = &fnc097Parser;
    // _demuxers[0x098] = &fnc098Parser;
    // _demuxers[0x099] = &fnc099Parser;
    // _demuxers[0x09a] = &fnc09aParser;
    // _demuxers[0x09b] = &fnc09bParser;
    // _demuxers[0x09c] = &fnc09cParser;
    // _demuxers[0x09d] = &fnc09dParser;
    // _demuxers[0x09e] = &fnc09eParser;
    // _demuxers[0x09f] = &fnc09fParser;
    // _demuxers[0x0a0] = &fnc0a0Parser;
    // _demuxers[0x0a1] = &fnc0a1Parser;
    // _demuxers[0x0a2] = &fnc0a2Parser;
    // _demuxers[0x0a3] = &fnc0a3Parser;
    // _demuxers[0x0a4] = &fnc0a4Parser;
    // _demuxers[0x0a5] = &fnc0a5Parser;
    // _demuxers[0x0a6] = &fnc0a6Parser;
    // _demuxers[0x0a7] = &fnc0a7Parser;
    // _demuxers[0x0a8] = &fnc0a8Parser;
    // _demuxers[0x0a9] = &fnc0a9Parser;
    // _demuxers[0x0aa] = &fnc0aaParser;
    // _demuxers[0x0ab] = &fnc0abParser;
    // _demuxers[0x0ac] = &fnc0acParser;
    // _demuxers[0x0ad] = &fnc0adParser;
    // _demuxers[0x0ae] = &fnc0aeParser;
    // _demuxers[0x0af] = &fnc0afParser;
    // _demuxers[0x0b0] = &fnc0b0Parser;
    // _demuxers[0x0b1] = &fnc0b1Parser;
    // _demuxers[0x0b2] = &fnc0b2Parser;
    // _demuxers[0x0b3] = &fnc0b3Parser;
    // _demuxers[0x0b4] = &fnc0b4Parser;
    // _demuxers[0x0b5] = &fnc0b5Parser;
    // _demuxers[0x0b6] = &fnc0b6Parser;
    // _demuxers[0x0b7] = &fnc0b7Parser;
    // _demuxers[0x0b8] = &fnc0b8Parser;
    // _demuxers[0x0b9] = &fnc0b9Parser;
    // _demuxers[0x0ba] = &fnc0baParser;
    // _demuxers[0x0bb] = &fnc0bbParser;
    // _demuxers[0x0bc] = &fnc0bcParser;
    // _demuxers[0x0bd] = &fnc0bdParser;
    // _demuxers[0x0be] = &fnc0beParser;
    // _demuxers[0x0bf] = &fnc0bfParser;
    // _demuxers[0x0c0] = &fnc0c0Parser;
    // _demuxers[0x0c1] = &fnc0c1Parser;
    // _demuxers[0x0c2] = &fnc0c2Parser;
    // _demuxers[0x0c3] = &fnc0c3Parser;
    // _demuxers[0x0c4] = &fnc0c4Parser;
    // _demuxers[0x0c5] = &fnc0c5Parser;
    // _demuxers[0x0c6] = &fnc0c6Parser;
    // _demuxers[0x0c7] = &fnc0c7Parser;
    _demuxers[0x0c8] = &fnc0c8Parser;
    // _demuxers[0x0c9] = &fnc0c9Parser;
    // _demuxers[0x0ca] = &fnc0caParser;
    // _demuxers[0x0cb] = &fnc0cbParser;
    // _demuxers[0x0cc] = &fnc0ccParser;
    _demuxers[0x0cd] = &fnc0cdParser;
    // _demuxers[0x0ce] = &fnc0ceParser;
    _demuxers[0x0cf] = &fnc0cfParser;
    // _demuxers[0x0d0] = &fnc0d0Parser;
    // _demuxers[0x0d1] = &fnc0d1Parser;
    // _demuxers[0x0d2] = &fnc0d2Parser;
    // _demuxers[0x0d3] = &fnc0d3Parser;
    // _demuxers[0x0d4] = &fnc0d4Parser;
    // _demuxers[0x0d5] = &fnc0d5Parser;
    // _demuxers[0x0d6] = &fnc0d6Parser;
    // _demuxers[0x0d7] = &fnc0d7Parser;
    // _demuxers[0x0d8] = &fnc0d8Parser;
    // _demuxers[0x0d9] = &fnc0d9Parser;
    // _demuxers[0x0da] = &fnc0daParser;
    // _demuxers[0x0db] = &fnc0dbParser;
    // _demuxers[0x0dc] = &fnc0dcParser;
    // _demuxers[0x0dd] = &fnc0ddParser;
    // _demuxers[0x0de] = &fnc0deParser;
    // _demuxers[0x0df] = &fnc0dfParser;
    // _demuxers[0x0e0] = &fnc0e0Parser;
    // _demuxers[0x0e1] = &fnc0e1Parser;
    // _demuxers[0x0e2] = &fnc0e2Parser;
    // _demuxers[0x0e3] = &fnc0e3Parser;
    // _demuxers[0x0e4] = &fnc0e4Parser;
    // _demuxers[0x0e5] = &fnc0e5Parser;
    // _demuxers[0x0e6] = &fnc0e6Parser;
    // _demuxers[0x0e7] = &fnc0e7Parser;
    // _demuxers[0x0e8] = &fnc0e8Parser;
    // _demuxers[0x0e9] = &fnc0e9Parser;
    // _demuxers[0x0ea] = &fnc0eaParser;
    // _demuxers[0x0eb] = &fnc0ebParser;
    // _demuxers[0x0ec] = &fnc0ecParser;
    // _demuxers[0x0ed] = &fnc0edParser;
    // _demuxers[0x0ee] = &fnc0eeParser;
    // _demuxers[0x0ef] = &fnc0efParser;
    // _demuxers[0x0f0] = &fnc0f0Parser;
    // _demuxers[0x0f1] = &fnc0f1Parser;
    // _demuxers[0x0f2] = &fnc0f2Parser;
    // _demuxers[0x0f3] = &fnc0f3Parser;
    // _demuxers[0x0f4] = &fnc0f4Parser;
    // _demuxers[0x0f5] = &fnc0f5Parser;
    // _demuxers[0x0f6] = &fnc0f6Parser;
    // _demuxers[0x0f7] = &fnc0f7Parser;
    // _demuxers[0x0f8] = &fnc0f8Parser;
    // _demuxers[0x0f9] = &fnc0f9Parser;
    _demuxers[0x0fa] = &fnc0faParser;
    _demuxers[0x0fb] = &fnc0fbParser;
    // _demuxers[0x0fc] = &fnc0fcParser;
    _demuxers[0x0fd] = &fnc0fdParser;
    _demuxers[0x0fe] = &fnc0feParser;
    // _demuxers[0x0ff] = &fnc0ffParser;
}

}
}
