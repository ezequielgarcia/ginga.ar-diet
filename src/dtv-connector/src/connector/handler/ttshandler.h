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

#ifndef _TTSHANDLER_H_
#define _TTSHANDLER_H_

#include <string>

#include "messagehandler.h"
#include <boost/function.hpp>
using namespace std;

namespace connector {

	//No de exceder los 10 elementos
	typedef enum {SPEECH	= 0x01,
				  STOP_TTS  = 0x02,
				  AUDIO_ON  = 0x03,
				  AUDIO_OFF = 0x04
				} MessageType;

	class TextToSpeechData;

	class Connector;	

	class TextToSpeechHandler : public MessageHandler {
	public:	
			TextToSpeechHandler (void);	
			virtual ~TextToSpeechHandler (void);
		

        	void send(Connector *conn, TextToSpeechData* data);
        	void send(Connector *conn, MessageType, TextToSpeechData* data);

			virtual void process( util::Buffer *msg );
	
    	    //typedef void (*TextToSpeechCallbackT)(string, bool, unsigned int);
       		typedef boost::function<void (string, bool, unsigned int )> TextToSpeechCallbackT; 
       		typedef boost::function<void (void)> StopTextToSpeechCallbackT; 
       		typedef boost::function<void (void)> AudioOnCallbackT; 
       		typedef boost::function<void (void)> AudioOffCallbackT; 
       		 
       		void onTextToSpeechHandler(const TextToSpeechCallbackT &callback);
			void onStopTextToSpeechHandler(const StopTextToSpeechCallbackT &callback);
			void onAudioOnHandler(const AudioOnCallbackT &callback);
			void onAudioOffHandler(const AudioOffCallbackT &callback);
	private:
	
        	TextToSpeechCallbackT		 _onTextToSpeech;
			StopTextToSpeechCallbackT	 _onStopTextToSpeech;
			AudioOnCallbackT			 _onAudioOn;
			AudioOffCallbackT			 _onAudioOff;
	}; 

	class TextToSpeechData {
	public:
			TextToSpeechData();
			TextToSpeechData(string speech, bool queue=false, unsigned int delay=0);
			virtual ~TextToSpeechData();
		
			void set_speech(string);
			string get_speech();

			void set_queue(bool);
			bool get_queue();
			
			void set_delay(unsigned int);
			unsigned int get_delay();
		
	private:
			string _strSpeech;
			bool   _blnQueue;
			unsigned int _delay;
	};


}






#endif /* _TTSHANDLER_H_ */






