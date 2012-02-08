/******************************************************************************
This file is part of the declarative environment of middleware Ginga (Ginga-NCL)

Copyright 2011 Fernando Vasconcelos - MICROTROL - ARGENTINA


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
fvasconcelos@microtrol.com.ar
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
http://www.microtrol.com.ar
*******************************************************************************/

#include <system/io/InputManager.h>
#include<connector/connector/handler/ttshandler.h>

#include "../../../../include/LuaPlayer.h"

connector::TextToSpeechHandler textToSpeech; 

LUALIB_API int l_speech (lua_State *L) {
	//Comenzamos a recibir los argunmentos de lua
	//Recibimos el speech
	const char *speech = luaL_checkstring(L, 1);
	bool queue=false;
	//Chequeamos que el proximo argumento sea booleano
	if(lua_isboolean(L, 2)){
		queue = (bool)lua_toboolean(L, 2);
	}
	
	//Chequeamos que el proximo argumento sea un int
	unsigned int delay = luaL_checkinteger(L,3); 
		
	connector::TextToSpeechData* textToSpeechData = new connector::TextToSpeechData(speech, queue, delay);
	textToSpeech.send(InputManager::getInstance()->getConnector(), connector::SPEECH, textToSpeechData);	
	delete textToSpeechData;
	return 0;
}

LUALIB_API int l_stopSpeech (lua_State *L){
	textToSpeech.send(InputManager::getInstance()->getConnector(), connector::STOP_TTS, NULL);	
	return 0;
}

LUALIB_API int l_audioOn(lua_State *L){
	textToSpeech.send(InputManager::getInstance()->getConnector(), connector::AUDIO_ON, NULL);	
	return 0;
}

LUALIB_API int l_audioOff(lua_State *L){
	textToSpeech.send(InputManager::getInstance()->getConnector(), connector::AUDIO_OFF, NULL);	
	return 0;
}

static const struct luaL_Reg meths[] = {
	{ "speech"	  ,	l_speech      },
	{ "stopSpeech",	l_stopSpeech  },
	{ "audioOn"   ,	l_audioOn	  },
	{ "audioOff"  ,	l_audioOff	  },
	{ NULL        , NULL	       } /* sentinel */
};

LUALIB_API int luaopen_tts(lua_State *L){
	luaL_register(L, "tts", meths);
	//Consigo el puntero al conector
	return 0;
}

LUALIB_API int luaclose_tts (lua_State* L){
	// Detener el speech al salir
	textToSpeech.send(InputManager::getInstance()->getConnector(), connector::STOP_TTS, NULL);	
	return 0;
}
