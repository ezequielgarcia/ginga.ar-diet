/*******************************************************************************

  Copyright (C) 2010, 2011 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of DTV-util implementation.

    DTV-util is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 2 of the License.

    DTV-util is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de DTV-util.

    DTV-util es Software Libre: Ud. puede redistribuirlo y/o modificarlo
  bajo los términos de la Licencia Pública General Reducida GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    DTV-util se distribuye esperando que resulte de utilidad, pero SIN NINGUNA
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública
  General Reducida GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General Reducida GNU
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#ifndef KEYDEFS_H
#define KEYDEFS_H

namespace util {
namespace key {

	/**
	 * @brief Enumeration for key identification.
	 */
	enum type {
		null = 0,

		number_0,
		number_1,
		number_2,
		number_3,
		number_4,
		number_5,
		number_6,
		number_7,
		number_8,
		number_9,

		small_a,
		small_b,
		small_c,
		small_d,
		small_e,
		small_f,
		small_g,
		small_h,
		small_i,
		small_j,
		small_k,
		small_l,
		small_m,
		small_n,
		small_o,
		small_p,
		small_q,
		small_r,
		small_s,
		small_t,
		small_u,
		small_v,
		small_w,
		small_x,
		small_y,
		small_z,

		capital_a,
		capital_b,
		capital_c,
		capital_d,
		capital_e,
		capital_f,
		capital_g,
		capital_h,
		capital_i,
		capital_j,
		capital_k,
		capital_l,
		capital_m,
		capital_n,
		capital_o,
		capital_p,
		capital_q,
		capital_r,
		capital_s,
		capital_t,
		capital_u,
		capital_v,
		capital_w,
		capital_x,
		capital_y,
		capital_z,

		page_down,
		page_up,

		f1,
		f2,
		f3,
		f4,
		f5,
		f6,
		f7,
		f8,
		f9,
		f10,
		f11,
		f12,

		plus_sign,
		minus_sign,

		asterisk,
		number_sign,

		period,

		super,
		printscreen,
		menu,
		info,
		epg,

		cursor_down,
		cursor_left,
		cursor_right,
		cursor_up,

		channel_down,
		channel_up,

		volume_down,
		volume_up,

		enter,
		ok,

		red,
		green,
		yellow,
		blue,

		backspace,
		back,
		escape,
		exit,

		power,
		rewind,
		stop,
		eject,
		play,
		record,
		pause,

		greater_than_sign,
		less_than_sign,

		tap
	};

}
}

#endif /* KEYDEFS_H */

