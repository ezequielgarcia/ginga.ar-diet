/*
<one line to give the program's name and a brief idea of what it does.>
Copyright (C) <year>  <name of author>
  Copyright 2010 LIFIA - Facultad de Informatica - Univ. Nacional de La Plata


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "generated/config.h"
#include "../../include/DummyConnector.h"

/*
#include <system/io/interface/content/video/DFBVideoProvider.h>
#include <system/io/interface/content/audio/FusionSoundAudioProvider.h>
*/

#include <system/io/interface/content/IContinuousMediaProvider.h>
#include <util/mcr.h>

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace player {

DummyConnector::DummyConnector(std::string mrl, bool hasVisual) {
/*
	this->hasVisual = hasVisual;
	 if (hasVisual) {
		this->provider = new DFBVideoProvider(mrl.c_str());
	} else {
		this->provider = new FusionSoundAudioProvider(mrl.c_str());
	}
*/
	surface = NULL;
	//this->surface = createFrame();
	//this->scopeEndTime = getTotalMediaTime();
}
	
DummyConnector::~DummyConnector(){
	//DEL(surface)
	//DEL(provider)
}


void DummyConnector::play(bool isMainAV, system::io::IProviderListener* listener){
	if (provider && !isMainAV){
		provider->playOver(surface, hasVisual, listener);
	}//TODO
}
	
	
void DummyConnector::setSoundLevel(float level){
	if (provider) {
		provider->setSoundLevel(level);
	}
}
	
void DummyConnector::stop(){
	if (provider) {
		provider->stop();
	}
}
	
void DummyConnector::pause(){
	if (provider) {
		provider->pause();
	}
}
	
void DummyConnector::resume(){
	if (provider) {	
		provider->resume(surface, hasVisual);
	}
}
	
bool DummyConnector::getHasVisual(){
	return hasVisual;
}
	
system::io::ISurface* DummyConnector::createFrame() {
	if (surface != NULL) {
		cout << "AVPlayer::createFrame Warning! surface != NULL";
		cout << endl;
		delete surface;
		surface = NULL;
	}
	if (provider) {	
		surface = provider->getPerfectSurface();
	}
	return surface;
}
	
void DummyConnector::getOriginalResolution(int* height, int* width) {
	if (provider) {
		provider->getOriginalResolution(height, width);
	}
}
	
double DummyConnector::getTotalMediaTime() {
	return (provider) ? provider->getTotalMediaTime() : 0;
}
	
double DummyConnector::getMediaTime() {
	return (provider)  ? provider->getMediaTime() : 0;
}

void DummyConnector::setMediaTime(double pos) {
	if (provider) {	
		provider->setMediaTime(pos);
	}
}

void DummyConnector::setVoutWindow(string id, int x, int y, int w, int h) {
	//provider->setVoutWindow(id, x, y, w, h);
	cout << "XineVideoProvider::checkVideoResizeEvent ";
	cout << "width = " << w << " height = " << h;
	cout << " x = " << x << " y = " << y << endl;
}

}
}
}
}
}
}
