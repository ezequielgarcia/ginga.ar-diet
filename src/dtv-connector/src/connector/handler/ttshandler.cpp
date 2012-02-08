/*******************************************************************************

  Copyright (C) 2011 Fernando Vasconcelos - MICROTROL - Rosario, Argentina

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

********************************************************************************/

#include "ttshandler.h"
#include "../connector.h"

namespace connector {

TextToSpeechHandler::TextToSpeechHandler(void) {
}

TextToSpeechHandler::~TextToSpeechHandler(void) {
}


void TextToSpeechHandler::send(Connector *conn, MessageType messageType, TextToSpeechData *data) {
	
	char* buf;	
	unsigned int messageSize = 0; 
	
	switch(messageType){
		case SPEECH:
			//Checkeo que data no sea NULL
			if(data != NULL) {

				 buf = (char*)malloc(MESSAGE_HEADER+
				         			 data->get_speech().size()+
									 sizeof(data->get_queue())+
									 sizeof(data->get_delay()));

				//Guardo el tipo de mensaje 
				buf[MESSAGE_HEADER]=messageType;

				//Guardo el boleano
				buf[MESSAGE_HEADER+1]=data->get_queue();
	
				//Guardo el delay
				unsigned int delay = data->get_delay();
				buf[MESSAGE_HEADER+2]= (delay >> 24) & 0xFF;
				buf[MESSAGE_HEADER+3]= (delay >> 16) & 0xFF;
				buf[MESSAGE_HEADER+4]= (delay >> 8 ) & 0xFF;
				buf[MESSAGE_HEADER+5]= (delay      ) & 0xFF;

				//Se guarda el texto que se va a reproducir
				const unsigned speech_idx = MESSAGE_HEADER + 6;
				string speech = data->get_speech();
				const unsigned int speech_size = speech.size();
				char *speech_ptr = buf + speech_idx;
				strcpy(speech_ptr, speech.c_str());	
				
				//Almaceno el tamaño del mensaje
				messageSize = speech_idx+speech_size+1;
	
			} else { 

				buf = (char*)malloc(MESSAGE_HEADER + sizeof(MessageType));
				//Almaceno el tamaño del mensaje
				messageSize = MESSAGE_HEADER;

			}
			break;

		case STOP_TTS:
			buf =(char*)malloc(MESSAGE_HEADER + sizeof(MessageType));
			//Guardo el tipo de mensaje
			buf[MESSAGE_HEADER] = messageType;
			//Almaceno el tamaño del mensaje
			messageSize = MESSAGE_HEADER+1;
			break;

		case AUDIO_ON:
			buf =(char*)malloc(MESSAGE_HEADER + sizeof(MessageType));
			//Guardo el tipo de mensaje
			buf[MESSAGE_HEADER] = messageType;
			//Almaceno el tamaño del mensaje
			messageSize = MESSAGE_HEADER+1;
			break;

		case AUDIO_OFF:
			buf =(char*)malloc(MESSAGE_HEADER + sizeof(MessageType));
			//Guardo el tipo de mensaje
			buf[MESSAGE_HEADER] = messageType;
			//Almaceno el tamaño del mensaje
			messageSize = MESSAGE_HEADER+1;
			break;
			
		default:
			break;
	
	}
	
	
	MAKE_MSG(buf, messages::textToSpeech, messageSize);
	util::Buffer msg(buf, messageSize, false);
	conn->send(&msg);

	//Libero buff	
	free(buf);
}

void TextToSpeechHandler::send(Connector *conn, TextToSpeechData  *data) {
	char buf[MESSAGE_HEADER+data->get_speech().size()+sizeof(data->get_queue())+sizeof(data->get_delay())]; //MESSAGE_HEADER + TAMANO DEL MENSAJE 
			
	//Guardo el boleano
	buf[MESSAGE_HEADER]=data->get_queue();
	
	//Guardo el delay
	unsigned int delay = data->get_delay();
	buf[MESSAGE_HEADER+1]= (delay >> 24) & 0xFF;
	buf[MESSAGE_HEADER+2]= (delay >> 16) & 0xFF;
	buf[MESSAGE_HEADER+3]= (delay >> 8 ) & 0xFF;
	buf[MESSAGE_HEADER+4]= (delay      ) & 0xFF;

	//Se guarda el texto que se va a reproducir
	const unsigned speech_idx = MESSAGE_HEADER + 5;
	string speech = data->get_speech();
	const unsigned int speech_size = speech.size();
	char *speech_ptr = buf + speech_idx;
	strcpy(speech_ptr, speech.c_str());	
    MAKE_MSG(buf, messages::textToSpeech, speech_idx+speech_size+1);
	util::Buffer msg(buf, speech_idx+speech_size+1, false);
	conn->send(&msg);	
}

void TextToSpeechHandler::process(::util::Buffer *msg) {
	
   	util::BYTE *payload = MSG_PAYLOAD(msg->buffer());
	
	unsigned int offset = 0;

	//Obtengo el tipo de mensaje
	MessageType messageType = (MessageType)RB(payload, offset);

	switch(messageType){
		case SPEECH:
			{
			//Obtengo el booleano queue
			util::BYTE queue = RB(payload, offset);
			//Obtengo el parametro del delay	
			util::DWORD delay = RDW(payload, offset);
			//Obtengo el speech
			//TODO: Agregar validacion del tamaño del speech	
			string speech((char*)payload+offset);

			//Verifico si hay una funcion de callback y la llamo
			if (!_onTextToSpeech.empty()  /*&TextToSpeechCallbackT*/ ) {
			_onTextToSpeech(speech, queue, delay); 	
			}
			}
			break;

		case STOP_TTS:
			if (!_onStopTextToSpeech.empty()  /*&StopTextToSpeechCallbackT*/ ) {
				_onStopTextToSpeech(); 	
			}
			break;		

		case AUDIO_ON:
			if (!_onAudioOn.empty() /*&AudioOnCallbackT*/ ) {
				printf("[DEBUG] %s, calling _onAudioOn()\n", __func__);
				_onAudioOn();
			}	
			break;	

		case AUDIO_OFF:
			if (!_onAudioOff.empty() /*&AudioOffCallbackT*/ ) {
				printf("[DEBUG] %s, calling _onAudioOff()\n", __func__);
				_onAudioOff();
			}	
			break;
		
		default:
			break;
	}
	
}

void TextToSpeechHandler::onTextToSpeechHandler(const TextToSpeechCallbackT &callback) {
	_onTextToSpeech = callback;
}

void TextToSpeechHandler::onStopTextToSpeechHandler(const StopTextToSpeechCallbackT &callback) {
	_onStopTextToSpeech = callback;
}

void TextToSpeechHandler::onAudioOnHandler(const AudioOnCallbackT &callback) {
	_onAudioOn = callback;
}

void TextToSpeechHandler::onAudioOffHandler(const AudioOffCallbackT &callback) {
	_onAudioOff = callback;
}


TextToSpeechData::TextToSpeechData(){
}

TextToSpeechData::TextToSpeechData(string speech, bool queue, unsigned int delay){
	_strSpeech = speech;
	_blnQueue = queue;
	_delay = delay;
} 

TextToSpeechData::~TextToSpeechData(){
}

void TextToSpeechData::set_speech(string speech){
	_strSpeech = speech;
}

string TextToSpeechData::get_speech(){
	return _strSpeech; 
}

void TextToSpeechData::set_queue(bool queue){
	_blnQueue = queue;
}

bool TextToSpeechData::get_queue(){
	return _blnQueue; 
}

void TextToSpeechData::set_delay(unsigned int delay){
	_delay = delay;
}

unsigned int TextToSpeechData::get_delay(){
	return _delay; 
}

}
