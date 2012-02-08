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

#ifndef DUMMYCONNECTOR_H
#define DUMMYCONNECTOR_H

#include <iostream>
#include <system/io/interface/content/IContinuousMediaProvider.h>
#include <system/io/interface/content/IProviderListener.h>
#include <system/io/interface/output/ISurface.h>

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace player {

class DummyConnector
{
public:
	DummyConnector(std::string mrl, bool hasVisual);
	virtual ~DummyConnector();
	double getTotalMediaTime();
	void setSoundLevel(float level);
	bool getHasVisual();
	void play(bool isMainAV, system::io::IProviderListener* listener);
	void stop();
	void pause();
	void resume();
	system::io::ISurface* createFrame();
	void getOriginalResolution(int* height, int* width);
	system::io::ISurface* surface;
	double getMediaTime();
	void setMediaTime(double pos);
	void setVoutWindow(string id, int x, int y, int w, int h);

private:	
	system::io::IContinuousMediaProvider* provider;
	bool hasVisual;
		
};

}
}
}
}
}
}



#endif // DUMMYCONNECTOR_H
