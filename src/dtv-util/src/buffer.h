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
#pragma once

#include <string>
#include <stdio.h>
#include "types.h"

namespace util {

	class Buffer {
	public:
		//	Constructors
		Buffer( void );
		Buffer( int blockSize );
		Buffer( const Buffer &other );
		Buffer( const char *buf, int len, bool owner=true );
		virtual ~Buffer( void );

		//	Operators
		Buffer &operator=( const Buffer &other );
		bool operator==( const Buffer &other ) const;

		//	Getters
		int length() const;
		int capacity() const;
		int blockSize() const;
		char *buffer() const;
		operator const char*( void );
		char &operator[]( int pos );
		std::string asHexa( void ) const;
		template<class Operand> inline void format( Operand &oper )const;

		//	Operations
		void resize( int len );
		void append( const char *buf, int len );
		void copy( const char *buf, int len );
		void copy( int pos, const char *buf, int len );
		void assign( const char *buf, int len );
		void swap( Buffer &other );

	protected:
		void clear( void );
		void makeSpace( int len, bool copy );

	private:
		char *_buf;
		bool  _owner;
		int   _length;
		int   _bufLen;
		int   _blockSize;
	};

	template<class Operand>
	inline void Buffer::format( Operand &oper ) const {
		for (int i=0; i<_length; ++i) {
			oper( _buf[i] );
		}
	}

}

