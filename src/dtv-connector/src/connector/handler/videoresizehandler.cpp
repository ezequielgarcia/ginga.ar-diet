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
#include "videoresizehandler.h"
#include "../connector.h"
#include <util/buffer.h>
#include <boost/foreach.hpp>

namespace connector {

VideoResizeHandler::VideoResizeHandler( void )
{
}

VideoResizeHandler::~VideoResizeHandler( void )
{
}

#define FLOAT_SIZE sizeof(float)
#define AV_DESCRIPTOR_BUF_SIZE ((FLOAT_SIZE*4)+4)

void VideoResizeHandler::send( Connector *conn, AVDescriptor* descriptor ) {
	char buf[AV_DESCRIPTOR_BUF_SIZE];
	char *ptr = (char *)MSG_PAYLOAD(buf);
	int offset	= 0;

	float value = descriptor->getX();
	memcpy(ptr+offset, &value, FLOAT_SIZE );
	offset+=FLOAT_SIZE;

	
	value = descriptor->getY();
	memcpy(ptr+offset, &value, FLOAT_SIZE );
	offset+=FLOAT_SIZE;
	
	value = descriptor->getWidth();
	memcpy(ptr+offset, &value, FLOAT_SIZE );
	offset+=FLOAT_SIZE;

	value =descriptor->getHeight();
	memcpy(ptr+offset, &value, FLOAT_SIZE );
	offset += FLOAT_SIZE;
	
	MAKE_MSG(buf,messages::videoResize,offset+MESSAGE_HEADER);

	util::Buffer msg( buf, offset+MESSAGE_HEADER, false );
	conn->send( &msg );
}

void VideoResizeHandler::onVideoResizeEvent( const VideoResizeEventCallback &callback ) {
	_onVideoResizeEvent = callback;
}

void VideoResizeHandler::process( util::Buffer *msg ) {
	if (!_onVideoResizeEvent.empty()) {
		AVDescriptor msgData;
		util::BYTE *payload  		= MSG_PAYLOAD(msg->buffer());		
		float value 			= 0;

		memcpy( &value, payload, FLOAT_SIZE );
		msgData.setX(value);		
		payload += FLOAT_SIZE;
		
		memcpy( &value, payload, FLOAT_SIZE );
		msgData.setY(value);
		payload += FLOAT_SIZE;
		
		memcpy( &value, payload, FLOAT_SIZE );
		msgData.setWidth(value);
		payload += FLOAT_SIZE;
		
		memcpy( &value, payload, FLOAT_SIZE );
		msgData.setHeight(value);

		_onVideoResizeEvent( &msgData );
	}
}

/******************* class AVDescriptor *******************/

AVDescriptor::AVDescriptor( void ){
}

AVDescriptor::AVDescriptor(float x, float y, float width, float height ){
	this->x = x;
	this->y = y;
	this->width = width;
	this->height = height;
}

AVDescriptor::~AVDescriptor( void ){
}

/* Getters */
float AVDescriptor::getX(){
	return x;
}

float AVDescriptor::getY(){
	return y;
}

float AVDescriptor::getWidth(){
	return width;
}

float AVDescriptor::getHeight(){
	return height;
}

/* Setters */
void AVDescriptor::setX(float x){
	this->x = x;
}

void AVDescriptor::setY(float y){
	this->y = y;
}

void AVDescriptor::setWidth(float width){
	this->width = width;
}

void AVDescriptor::setHeight(float height){
	this->height = height;
}


}
