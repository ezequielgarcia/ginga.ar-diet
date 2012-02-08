/******************************************************************************
Este arquivo eh parte da implementacao do ambiente declarativo do middleware
Ginga (Ginga-NCL).

Direitos Autorais Reservados (c) 1989-2007 PUC-Rio/Laboratorio TeleMidia

Este programa eh software livre; voce pode redistribui-lo e/ou modificah-lo sob
os termos da Licenca Publica Geral GNU versao 2 conforme publicada pela Free
Software Foundation.

Este programa eh distribuido na expectativa de que seja util, porem, SEM
NENHUMA GARANTIA; nem mesmo a garantia implicita de COMERCIABILIDADE OU
ADEQUACAO A UMA FINALIDADE ESPECIFICA. Consulte a Licenca Publica Geral do
GNU versao 2 para mais detalhes.

Voce deve ter recebido uma copia da Licenca Publica Geral do GNU versao 2 junto
com este programa; se nao, escreva para a Free Software Foundation, Inc., no
endereco 59 Temple Street, Suite 330, Boston, MA 02111-1307 USA.

Para maiores informacoes:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
******************************************************************************
This file is part of the declarative environment of middleware Ginga (Ginga-NCL)

Copyright: 1989-2007 PUC-RIO/LABORATORIO TELEMIDIA, All Rights Reserved.
  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata


This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License version 2 for more
details.

You should have received a copy of the GNU General Public License version 2
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

For further information contact:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
*******************************************************************************/

#include "../../../../include/io/interface/input/CodeMap.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <directfb.h>
#ifdef __cplusplus
}
#endif

using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace system {
namespace io {
	const int CodeMap::KEY_NULL              = DIKS_NULL;
	const int CodeMap::KEY_0                 = DIKS_0;
	const int CodeMap::KEY_1                 = DIKS_1;
	const int CodeMap::KEY_2                 = DIKS_2;
	const int CodeMap::KEY_3                 = DIKS_3;
	const int CodeMap::KEY_4                 = DIKS_4;
	const int CodeMap::KEY_5                 = DIKS_5;
	const int CodeMap::KEY_6                 = DIKS_6;
	const int CodeMap::KEY_7                 = DIKS_7;
	const int CodeMap::KEY_8                 = DIKS_8;
	const int CodeMap::KEY_9                 = DIKS_9;

	const int CodeMap::KEY_SMALL_A           = DIKS_SMALL_A;
	const int CodeMap::KEY_SMALL_B           = DIKS_SMALL_B;
	const int CodeMap::KEY_SMALL_C           = DIKS_SMALL_C;
	const int CodeMap::KEY_SMALL_D           = DIKS_SMALL_D;
	const int CodeMap::KEY_SMALL_E           = DIKS_SMALL_E;
	const int CodeMap::KEY_SMALL_F           = DIKS_SMALL_F;
	const int CodeMap::KEY_SMALL_G           = DIKS_SMALL_G;
	const int CodeMap::KEY_SMALL_H           = DIKS_SMALL_H;
	const int CodeMap::KEY_SMALL_I           = DIKS_SMALL_I;
	const int CodeMap::KEY_SMALL_J           = DIKS_SMALL_J;
	const int CodeMap::KEY_SMALL_K           = DIKS_SMALL_K;
	const int CodeMap::KEY_SMALL_L           = DIKS_SMALL_L;
	const int CodeMap::KEY_SMALL_M           = DIKS_SMALL_M;
	const int CodeMap::KEY_SMALL_N           = DIKS_SMALL_N;
	const int CodeMap::KEY_SMALL_O           = DIKS_SMALL_O;
	const int CodeMap::KEY_SMALL_P           = DIKS_SMALL_P;
	const int CodeMap::KEY_SMALL_Q           = DIKS_SMALL_Q;
	const int CodeMap::KEY_SMALL_R           = DIKS_SMALL_R;
	const int CodeMap::KEY_SMALL_S           = DIKS_SMALL_S;
	const int CodeMap::KEY_SMALL_T           = DIKS_SMALL_T;
	const int CodeMap::KEY_SMALL_U           = DIKS_SMALL_U;
	const int CodeMap::KEY_SMALL_V           = DIKS_SMALL_V;
	const int CodeMap::KEY_SMALL_W           = DIKS_SMALL_W;
	const int CodeMap::KEY_SMALL_X           = DIKS_SMALL_X;
	const int CodeMap::KEY_SMALL_Y           = DIKS_SMALL_Y;
	const int CodeMap::KEY_SMALL_Z           = DIKS_SMALL_Z;

	const int CodeMap::KEY_CAPITAL_A         = DIKS_CAPITAL_A;
	const int CodeMap::KEY_CAPITAL_B         = DIKS_CAPITAL_B;
	const int CodeMap::KEY_CAPITAL_C         = DIKS_CAPITAL_C;
	const int CodeMap::KEY_CAPITAL_D         = DIKS_CAPITAL_D;
	const int CodeMap::KEY_CAPITAL_E         = DIKS_CAPITAL_E;
	const int CodeMap::KEY_CAPITAL_F         = DIKS_CAPITAL_F;
	const int CodeMap::KEY_CAPITAL_G         = DIKS_CAPITAL_G;
	const int CodeMap::KEY_CAPITAL_H         = DIKS_CAPITAL_H;
	const int CodeMap::KEY_CAPITAL_I         = DIKS_CAPITAL_I;
	const int CodeMap::KEY_CAPITAL_J         = DIKS_CAPITAL_J;
	const int CodeMap::KEY_CAPITAL_K         = DIKS_CAPITAL_K;
	const int CodeMap::KEY_CAPITAL_L         = DIKS_CAPITAL_L;
	const int CodeMap::KEY_CAPITAL_M         = DIKS_CAPITAL_M;
	const int CodeMap::KEY_CAPITAL_N         = DIKS_CAPITAL_N;
	const int CodeMap::KEY_CAPITAL_O         = DIKS_CAPITAL_O;
	const int CodeMap::KEY_CAPITAL_P         = DIKS_CAPITAL_P;
	const int CodeMap::KEY_CAPITAL_Q         = DIKS_CAPITAL_Q;
	const int CodeMap::KEY_CAPITAL_R         = DIKS_CAPITAL_R;
	const int CodeMap::KEY_CAPITAL_S         = DIKS_CAPITAL_S;
	const int CodeMap::KEY_CAPITAL_T         = DIKS_CAPITAL_T;
	const int CodeMap::KEY_CAPITAL_U         = DIKS_CAPITAL_U;
	const int CodeMap::KEY_CAPITAL_V         = DIKS_CAPITAL_V;
	const int CodeMap::KEY_CAPITAL_W         = DIKS_CAPITAL_W;
	const int CodeMap::KEY_CAPITAL_X         = DIKS_CAPITAL_X;
	const int CodeMap::KEY_CAPITAL_Y         = DIKS_CAPITAL_Y;
	const int CodeMap::KEY_CAPITAL_Z         = DIKS_CAPITAL_Z;

	const int CodeMap::KEY_PAGE_DOWN         = DIKS_PAGE_DOWN;
	const int CodeMap::KEY_PAGE_UP           = DIKS_PAGE_UP;

	const int CodeMap::KEY_F1                = DIKS_F1;
	const int CodeMap::KEY_F2                = DIKS_F2;
	const int CodeMap::KEY_F3                = DIKS_F3;
	const int CodeMap::KEY_F4                = DIKS_F4;
	const int CodeMap::KEY_F5                = DIKS_F5;
	const int CodeMap::KEY_F6                = DIKS_F6;
	const int CodeMap::KEY_F7                = DIKS_F7;
	const int CodeMap::KEY_F8                = DIKS_F8;
	const int CodeMap::KEY_F9                = DIKS_F9;
	const int CodeMap::KEY_F10               = DIKS_F10;
	const int CodeMap::KEY_F11               = DIKS_F11;
	const int CodeMap::KEY_F12               = DIKS_F12;

	const int CodeMap::KEY_PLUS_SIGN         = DIKS_PLUS_SIGN;
	const int CodeMap::KEY_MINUS_SIGN        = DIKS_MINUS_SIGN;

	const int CodeMap::KEY_ASTERISK          = DIKS_ASTERISK;
	const int CodeMap::KEY_NUMBER_SIGN       = DIKS_NUMBER_SIGN;

	const int CodeMap::KEY_PERIOD            = DIKS_PERIOD;

	const int CodeMap::KEY_SUPER             = DIKS_SUPER;
	const int CodeMap::KEY_PRINTSCREEN       = DIKS_PRINT;
	const int CodeMap::KEY_MENU              = DIKS_MENU;
	const int CodeMap::KEY_INFO              = DIKS_INFO;
	const int CodeMap::KEY_EPG               = DIKS_EPG;

	const int CodeMap::KEY_CURSOR_DOWN       = DIKS_CURSOR_DOWN;
	const int CodeMap::KEY_CURSOR_LEFT       = DIKS_CURSOR_LEFT;
	const int CodeMap::KEY_CURSOR_RIGHT      = DIKS_CURSOR_RIGHT;
	const int CodeMap::KEY_CURSOR_UP         = DIKS_CURSOR_UP;

	const int CodeMap::KEY_CHANNEL_DOWN      = DIKS_CHANNEL_DOWN;
	const int CodeMap::KEY_CHANNEL_UP        = DIKS_CHANNEL_UP;

	const int CodeMap::KEY_VOLUME_DOWN       = DIKS_VOLUME_DOWN;
	const int CodeMap::KEY_VOLUME_UP         = DIKS_VOLUME_UP;

	const int CodeMap::KEY_ENTER             = DIKS_ENTER;
	const int CodeMap::KEY_OK                = DIKS_OK;

	const int CodeMap::KEY_RED               = DIKS_RED;
	const int CodeMap::KEY_GREEN             = DIKS_GREEN;
	const int CodeMap::KEY_YELLOW            = DIKS_YELLOW;
	const int CodeMap::KEY_BLUE              = DIKS_BLUE;

	const int CodeMap::KEY_BACKSPACE         = DIKS_BACKSPACE;
	const int CodeMap::KEY_BACK              = DIKS_BACK;
	const int CodeMap::KEY_ESCAPE            = DIKS_ESCAPE;
	const int CodeMap::KEY_EXIT              = DIKS_EXIT;

	const int CodeMap::KEY_POWER             = DIKS_POWER;
	const int CodeMap::KEY_REWIND            = DIKS_REWIND;
	const int CodeMap::KEY_STOP              = DIKS_STOP;
	const int CodeMap::KEY_EJECT             = DIKS_EJECT;
	const int CodeMap::KEY_PLAY              = DIKS_PLAY;
	const int CodeMap::KEY_RECORD            = DIKS_RECORD;
	const int CodeMap::KEY_PAUSE             = DIKS_PAUSE;

	const int CodeMap::KEY_GREATER_THAN_SIGN = DIKS_GREATER_THAN_SIGN;
	const int CodeMap::KEY_LESS_THAN_SIGN    = DIKS_LESS_THAN_SIGN;

	const int CodeMap::KEY_TAP = DIKS_CUSTOM0;

	CodeMap::CodeMap() {
		keyMap = new map<string, int>;
		(*keyMap)["NO_CODE"] = KEY_NULL;
		(*keyMap)["0"] = KEY_0;
		(*keyMap)["1"] = KEY_1;
		(*keyMap)["2"] = KEY_2;
		(*keyMap)["3"] = KEY_3;
		(*keyMap)["4"] = KEY_4;
		(*keyMap)["5"] = KEY_5;
		(*keyMap)["6"] = KEY_6;
		(*keyMap)["7"] = KEY_7;
		(*keyMap)["8"] = KEY_8;
		(*keyMap)["9"] = KEY_9;

		(*keyMap)["a"] = KEY_SMALL_A;
		(*keyMap)["b"] = KEY_SMALL_B;
		(*keyMap)["c"] = KEY_SMALL_C;
		(*keyMap)["d"] = KEY_SMALL_D;
		(*keyMap)["e"] = KEY_SMALL_E;
		(*keyMap)["f"] = KEY_SMALL_F;
		(*keyMap)["g"] = KEY_SMALL_G;
		(*keyMap)["h"] = KEY_SMALL_H;
		(*keyMap)["i"] = KEY_SMALL_I;
		(*keyMap)["j"] = KEY_SMALL_J;
		(*keyMap)["k"] = KEY_SMALL_K;
		(*keyMap)["l"] = KEY_SMALL_L;
		(*keyMap)["m"] = KEY_SMALL_M;
		(*keyMap)["n"] = KEY_SMALL_N;
		(*keyMap)["o"] = KEY_SMALL_O;
		(*keyMap)["p"] = KEY_SMALL_P;
		(*keyMap)["q"] = KEY_SMALL_Q;
		(*keyMap)["r"] = KEY_SMALL_R;
		(*keyMap)["s"] = KEY_SMALL_S;
		(*keyMap)["t"] = KEY_SMALL_T;
		(*keyMap)["u"] = KEY_SMALL_U;
		(*keyMap)["v"] = KEY_SMALL_V;
		(*keyMap)["w"] = KEY_SMALL_W;
		(*keyMap)["x"] = KEY_SMALL_X;
		(*keyMap)["y"] = KEY_SMALL_Y;
		(*keyMap)["z"] = KEY_SMALL_Z;

		(*keyMap)["A"] = KEY_CAPITAL_A;
		(*keyMap)["B"] = KEY_CAPITAL_B;
		(*keyMap)["C"] = KEY_CAPITAL_C;
		(*keyMap)["D"] = KEY_CAPITAL_D;
		(*keyMap)["E"] = KEY_CAPITAL_E;
		(*keyMap)["F"] = KEY_CAPITAL_F;
		(*keyMap)["G"] = KEY_CAPITAL_G;
		(*keyMap)["H"] = KEY_CAPITAL_H;
		(*keyMap)["I"] = KEY_CAPITAL_I;
		(*keyMap)["J"] = KEY_CAPITAL_J;
		(*keyMap)["K"] = KEY_CAPITAL_K;
		(*keyMap)["L"] = KEY_CAPITAL_L;
		(*keyMap)["M"] = KEY_CAPITAL_M;
		(*keyMap)["N"] = KEY_CAPITAL_N;
		(*keyMap)["O"] = KEY_CAPITAL_O;
		(*keyMap)["P"] = KEY_CAPITAL_P;
		(*keyMap)["Q"] = KEY_CAPITAL_Q;
		(*keyMap)["R"] = KEY_CAPITAL_R;
		(*keyMap)["S"] = KEY_CAPITAL_S;
		(*keyMap)["T"] = KEY_CAPITAL_T;
		(*keyMap)["U"] = KEY_CAPITAL_U;
		(*keyMap)["V"] = KEY_CAPITAL_V;
		(*keyMap)["W"] = KEY_CAPITAL_W;
		(*keyMap)["X"] = KEY_CAPITAL_X;
		(*keyMap)["Y"] = KEY_CAPITAL_Y;
		(*keyMap)["Z"] = KEY_CAPITAL_Z;

		(*keyMap)["*"] = KEY_ASTERISK;
		(*keyMap)["#"] = KEY_NUMBER_SIGN;

		(*keyMap)["."] = KEY_PERIOD;

		(*keyMap)["MENU"]  = KEY_MENU;
		(*keyMap)["INFO"]  = KEY_INFO;
		(*keyMap)["GUIDE"] = KEY_EPG;

		(*keyMap)["CURSOR_DOWN"]  = KEY_CURSOR_DOWN;
		(*keyMap)["CURSOR_LEFT"]  = KEY_CURSOR_LEFT;
		(*keyMap)["CURSOR_RIGHT"] = KEY_CURSOR_RIGHT;
		(*keyMap)["CURSOR_UP"]    = KEY_CURSOR_UP;

		(*keyMap)["CHANNEL_DOWN"] = KEY_CHANNEL_DOWN;
		(*keyMap)["CHANNEL_UP"]   = KEY_CHANNEL_UP;

		(*keyMap)["VOLUME_DOWN"]  = KEY_VOLUME_DOWN;
		(*keyMap)["VOLUME_UP"]    = KEY_VOLUME_UP;

		(*keyMap)["ENTER"]  = KEY_ENTER;
		(*keyMap)["OK"]     = KEY_OK;
		(*keyMap)["TAP"]    = KEY_TAP;

		(*keyMap)["F1"]     = KEY_F1; /*added for use event.key in lua*/
		(*keyMap)["F2"]     = KEY_F2;
		(*keyMap)["F3"]     = KEY_F3;
		(*keyMap)["F4"]     = KEY_F4;
		(*keyMap)["F5"]     = KEY_F5;
		(*keyMap)["F6"]     = KEY_F6;
		(*keyMap)["F7"]     = KEY_F7;

		(*keyMap)["RED"]    = KEY_RED;
		(*keyMap)["GREEN"]  = KEY_GREEN;
		(*keyMap)["YELLOW"] = KEY_YELLOW;
		(*keyMap)["BLUE"]   = KEY_BLUE;

		(*keyMap)["BACK"]   = KEY_BACK;
		(*keyMap)["EXIT"]   = KEY_EXIT;

		(*keyMap)["POWER"]  = KEY_POWER;
		(*keyMap)["REWIND"] = KEY_REWIND;
		(*keyMap)["STOP"]   = KEY_STOP;
		(*keyMap)["EJECT"]  = KEY_EJECT;
		(*keyMap)["PLAY"]   = KEY_PLAY;
		(*keyMap)["RECORD"] = KEY_RECORD;
		(*keyMap)["PAUSE"]  = KEY_PAUSE;

		valueMap = new map<int, string>;

        map<string,int>::iterator it;
        for (it=keyMap->begin(); it!=keyMap->end(); it++)
		    (*valueMap)[it->second] = it->first;

	}

	int CodeMap::getCode(string codeStr) {
		if (keyMap->count(codeStr) == 0) {
			return KEY_NULL;
		}

		return (*keyMap)[codeStr];
	}

	string CodeMap::getValue(int code) {
		map<int, string>::iterator i;

		i = valueMap->find(code);
		if (i != valueMap->end()) {
			return i->second;
		}

		return "";
	}

	CodeMap* CodeMap::_instance = 0;

	CodeMap* CodeMap::getInstance() {
		if (CodeMap::_instance == NULL) {
			CodeMap::_instance = new CodeMap();
		}
		return CodeMap::_instance;
	}

	map<string, int>* CodeMap::cloneKeyMap() {
		return new map<string, int>(*keyMap);
	}

	void CodeMap::valuesToKeys (const std::set<int> &keys, std::set<util::key::type> &result)
	{
		std::set<int>::const_iterator it;
		result.clear();
		for (it = keys.begin(); it != keys.end(); ++it) {
			util::key::type key = util::key::null;
			switch (*it) {
				case KEY_NULL: key = (util::key::null); break;
				case KEY_0: key = (util::key::number_0); break;
				case KEY_1: key = (util::key::number_1); break;
				case KEY_2: key = (util::key::number_2); break;
				case KEY_3: key = (util::key::number_3); break;
				case KEY_4: key = (util::key::number_4); break;
				case KEY_5: key = (util::key::number_5); break;
				case KEY_6: key = (util::key::number_6); break;
				case KEY_7: key = (util::key::number_7); break;
				case KEY_8: key = (util::key::number_8); break;
				case KEY_9: key = (util::key::number_9); break;
				
				case KEY_SMALL_A: key = (util::key::small_a); break;
				case KEY_SMALL_B: key = (util::key::small_b); break;
				case KEY_SMALL_C: key = (util::key::small_c); break;
				case KEY_SMALL_D: key = (util::key::small_d); break;
				case KEY_SMALL_E: key = (util::key::small_e); break;
				case KEY_SMALL_F: key = (util::key::small_f); break;
				case KEY_SMALL_G: key = (util::key::small_g); break;
				case KEY_SMALL_H: key = (util::key::small_h); break;
				case KEY_SMALL_I: key = (util::key::small_i); break;
				case KEY_SMALL_J: key = (util::key::small_j); break;
				case KEY_SMALL_K: key = (util::key::small_k); break;
				case KEY_SMALL_L: key = (util::key::small_l); break;
				case KEY_SMALL_M: key = (util::key::small_m); break;
				case KEY_SMALL_N: key = (util::key::small_n); break;
				case KEY_SMALL_O: key = (util::key::small_o); break;
				case KEY_SMALL_P: key = (util::key::small_p); break;
				case KEY_SMALL_Q: key = (util::key::small_q); break;
				case KEY_SMALL_R: key = (util::key::small_r); break;
				case KEY_SMALL_S: key = (util::key::small_s); break;
				case KEY_SMALL_T: key = (util::key::small_t); break;
				case KEY_SMALL_U: key = (util::key::small_u); break;
				case KEY_SMALL_V: key = (util::key::small_v); break;
				case KEY_SMALL_W: key = (util::key::small_w); break;
				case KEY_SMALL_X: key = (util::key::small_x); break;
				case KEY_SMALL_Y: key = (util::key::small_y); break;
				case KEY_SMALL_Z: key = (util::key::small_z); break;
				
				case KEY_CAPITAL_A: key = (util::key::capital_a); break;
				case KEY_CAPITAL_B: key = (util::key::capital_b); break;
				case KEY_CAPITAL_C: key = (util::key::capital_c); break;
				case KEY_CAPITAL_D: key = (util::key::capital_d); break;
				case KEY_CAPITAL_E: key = (util::key::capital_e); break;
				case KEY_CAPITAL_F: key = (util::key::capital_f); break;
				case KEY_CAPITAL_G: key = (util::key::capital_g); break;
				case KEY_CAPITAL_H: key = (util::key::capital_h); break;
				case KEY_CAPITAL_I: key = (util::key::capital_i); break;
				case KEY_CAPITAL_J: key = (util::key::capital_j); break;
				case KEY_CAPITAL_K: key = (util::key::capital_k); break;
				case KEY_CAPITAL_L: key = (util::key::capital_l); break;
				case KEY_CAPITAL_M: key = (util::key::capital_m); break;
				case KEY_CAPITAL_N: key = (util::key::capital_n); break;
				case KEY_CAPITAL_O: key = (util::key::capital_o); break;
				case KEY_CAPITAL_P: key = (util::key::capital_p); break;
				case KEY_CAPITAL_Q: key = (util::key::capital_q); break;
				case KEY_CAPITAL_R: key = (util::key::capital_r); break;
				case KEY_CAPITAL_S: key = (util::key::capital_s); break;
				case KEY_CAPITAL_T: key = (util::key::capital_t); break;
				case KEY_CAPITAL_U: key = (util::key::capital_u); break;
				case KEY_CAPITAL_V: key = (util::key::capital_v); break;
				case KEY_CAPITAL_W: key = (util::key::capital_w); break;
				case KEY_CAPITAL_X: key = (util::key::capital_x); break;
				case KEY_CAPITAL_Y: key = (util::key::capital_y); break;
				case KEY_CAPITAL_Z: key = (util::key::capital_z); break;
				
				case KEY_PAGE_DOWN: key = (util::key::page_down); break;
				case KEY_PAGE_UP  : key = (util::key::page_up); break;
				
				case KEY_F1 : key = (util::key::f1); break;
				case KEY_F2 : key = (util::key::f2); break;
				case KEY_F3 : key = (util::key::f3); break;
				case KEY_F4 : key = (util::key::f4); break;
				case KEY_F5 : key = (util::key::f5); break;
				case KEY_F6 : key = (util::key::f6); break;
				case KEY_F7 : key = (util::key::f7); break;
				case KEY_F8 : key = (util::key::f8); break;
				case KEY_F9 : key = (util::key::f9); break;
				case KEY_F10: key = (util::key::f10); break;
				case KEY_F11: key = (util::key::f11); break;
				case KEY_F12: key = (util::key::f12); break;
				
				case KEY_PLUS_SIGN : key = (util::key::plus_sign); break;
				case KEY_MINUS_SIGN: key = (util::key::minus_sign); break;
				
				case KEY_ASTERISK   : key = (util::key::asterisk); break;
				case KEY_NUMBER_SIGN: key = (util::key::number_sign); break;
				
				case KEY_PERIOD: key = (util::key::period); break;
				
				case KEY_PRINTSCREEN: key = (util::key::printscreen); break;
				case KEY_SUPER: key = (util::key::super); break;
				case KEY_MENU : key = (util::key::menu); break;
				case KEY_INFO : key = (util::key::info); break;
				case KEY_EPG  : key = (util::key::epg); break;
				
				case KEY_CURSOR_DOWN : key = (util::key::cursor_down); break;
				case KEY_CURSOR_LEFT : key = (util::key::cursor_left); break;
				case KEY_CURSOR_RIGHT: key = (util::key::cursor_right); break;
				case KEY_CURSOR_UP   : key = (util::key::cursor_up); break;
				
				case KEY_CHANNEL_DOWN: key = (util::key::channel_down); break;
				case KEY_CHANNEL_UP  : key = (util::key::channel_up); break;
				
				case KEY_VOLUME_DOWN: key = (util::key::volume_down); break;
				case KEY_VOLUME_UP  : key = (util::key::volume_up); break;
				
				case KEY_ENTER: key = (util::key::enter); break;
				case KEY_OK   : key = (util::key::ok); break;
				
				case KEY_RED   : key = (util::key::red); break;
				case KEY_GREEN : key = (util::key::green); break;
				case KEY_YELLOW: key = (util::key::yellow); break;
				case KEY_BLUE  : key = (util::key::blue); break;
				
				case KEY_BACKSPACE: key = (util::key::backspace); break;
				case KEY_BACK  : key = (util::key::back); break;
				case KEY_ESCAPE: key = (util::key::escape); break;
				case KEY_EXIT  : key = (util::key::exit); break;
				
				case KEY_POWER : key = (util::key::power); break;
				case KEY_REWIND: key = (util::key::rewind); break;
				case KEY_STOP  : key = (util::key::stop); break;
				case KEY_EJECT : key = (util::key::eject); break;
				case KEY_PLAY  : key = (util::key::play); break;
				case KEY_RECORD: key = (util::key::record); break;
				case KEY_PAUSE : key = (util::key::pause); break;
				
				case KEY_GREATER_THAN_SIGN: key = (util::key::greater_than_sign); break;
				case KEY_LESS_THAN_SIGN   : key = (util::key::less_than_sign); break;
				
				case KEY_TAP: key = (util::key::tap); break;
				default:
					key = (util::key::null);
					break;
			}
			result.insert(key);
		}
	}
}
}
}
}
}
}
}
