/*******************************************************************************

  Copyright (C) 2010, 2011 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata

********************************************************************************

  This file is part of DTV implementation.

    DTV is free software: you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 2 of the License.

    DTV is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License along with
  this program. If not, see <http://www.gnu.org/licenses/>.

********************************************************************************

  Este archivo es parte de la implementación de DTV.

    DTV es Software Libre: Ud. puede redistribuirlo y/o modificarlo
  bajo los términos de la Licencia Pública General Reducida GNU como es publicada por la
  Free Software Foundation, según la versión 2 de la licencia.

    DTV se distribuye esperando que resulte de utilidad, pero SIN NINGUNA
  GARANTÍA; ni siquiera la garantía implícita de COMERCIALIZACIÓN o ADECUACIÓN
  PARA ALGÚN PROPÓSITO PARTICULAR. Para más detalles, revise la Licencia Pública
  General Reducida GNU.

    Ud. debería haber recibido una copia de la Licencia Pública General Reducida GNU
  junto a este programa. Si no, puede verla en <http://www.gnu.org/licenses/>.

*******************************************************************************/
#include "buffer.h"
#include "types.h"
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <assert.h>

#define MAKE_SPACE(l,c) if (l > _bufLen) { makeSpace( l, c ); }

namespace util {

Buffer::Buffer( void )
{
	_buf       = NULL;
	_bufLen    = 0;
	_length    = 0;
	_blockSize = 0;
	_owner     = true;
}

Buffer::Buffer( const Buffer &other )
{
	//	Always do buffer owner
	_buf       = NULL;
	_bufLen    = 0;
	_length    = 0;
	_blockSize = other._blockSize;
	_owner     = true;
	
	if (other._length) {
		copy( other._buf, other._length );		
	}
	else {
		MAKE_SPACE( 1, false );
	}
}

Buffer::Buffer( int blockSize )
{
	_buf       = NULL;
	_bufLen    = 0;
	_length    = 0;
	_blockSize = blockSize;
	_owner     = true;
	MAKE_SPACE( 1, false );	//	Force buffer allocation
}

Buffer::Buffer( const char *buf, int len, bool owner/*=true*/ )
{
	_buf       = NULL;
	_bufLen    = 0;
	_length    = 0;
	_blockSize = 0;
	_owner     = true;
	if (owner) {
		copy( buf, len );
	}
	else {
		assign( buf, len );
	}
}

Buffer::~Buffer( void )
{
	if (_owner) {
		free(_buf);
	}
}

//	Operators
Buffer &Buffer::operator=( const Buffer &other ) {
	_blockSize = other._blockSize;
	copy( other._buf, other._length );		
	return *this;
}

bool Buffer::operator==( const Buffer &other ) const {
	return _length == other._length && !memcmp(_buf,other._buf,_length);
}

//	Getters
int Buffer::length() const {
	return _length;
}

int Buffer::capacity() const {
	return _bufLen;
}

int Buffer::blockSize() const {
	return _blockSize;
}

char *Buffer::buffer() const {
	return _buf;
}

char &Buffer::operator[]( int pos ) {
	if (pos < _length) {
		return _buf[pos];
	}
	throw std::overflow_error( "Invalid index" );
}

Buffer::operator const char *( void ) {
	return _buf;
}

//	Operations
void Buffer::resize( int len ) {
	if (len > 0) {
		MAKE_SPACE( len, true );
	}
	_length = len;
}

void Buffer::append( const char *buf, int len ) {
	copy( _length, buf, len );
}

void Buffer::copy( const char *buf, int len ) {
	MAKE_SPACE( len, false );
	memcpy(_buf,buf,len);
	_length = len;
}

void Buffer::copy( int pos, const char *buf, int len ) {
	if (pos >= 0) {
		int s = pos + len;
		MAKE_SPACE( s, true );
		memcpy( _buf+pos, buf, len );
		if (s > _length) {
			_length = s;
		}
	}
}

void Buffer::assign( const char *buf, int len ) {
	if (_owner) {
		free(_buf);
	}
	_buf    = (char *)buf;
	_bufLen = _length = len;
	_owner  = false;
	_blockSize = 0;
}

void Buffer::swap( Buffer &other ) {
	std::swap(_buf,other._buf);
	std::swap(_bufLen,other._bufLen);
	std::swap(_length,other._length);
	std::swap(_blockSize,other._blockSize);
	std::swap(_owner,other._owner);
}

class ConvertToHex {
public:
	ConvertToHex( std::string &result ) : _result(result) {}

	ConvertToHex &operator=( const ConvertToHex &other ) {
		memcpy(_temp,other._temp,16);
		_result = other._result;
		return *this;
	}

	void operator()( char ch ) {
		_temp[0] = 0;
		DWORD x=0x00000000FF & ch;
		sprintf( _temp, "%02lX", x );
		if (_result.length()) {
			_result += ":";
		}
		_result += _temp;
	}

	std::string &_result;
private:
	char _temp[16];
};

std::string Buffer::asHexa( void ) const {
	std::string result;
	ConvertToHex oper( result );
	format( oper );
	return result;
}

//	Aux
void Buffer::makeSpace( int len, bool copy ) {
	//	Get bytes to allocate
	if (_blockSize) {
		int blocks = (len < _blockSize) ? 1 : (len / _blockSize) + 1;
		_bufLen = blocks*_blockSize;
	}
	else {
		_bufLen = len;
	}

	if (copy && _length) {
		//	Translate memory
		char *tmp = (char *)malloc( _bufLen );
		memcpy(tmp,_buf,_length);
		if (_owner) {
			free(_buf);
		}
		_buf = tmp;
	}
	else if (_owner) {
		_buf = (char *)realloc( _buf, _bufLen );
	}
	else {
		_buf = (char *)malloc( _bufLen );
	}
}

}
