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

#include <assert.h>
#include "../../include/AVPlayer.h"
#include "../../include/PlayersComponentSupport.h"
#include <system/util/functions.h>
#include <util/mcr.h>
#include <sys/types.h>
#include <fcntl.h>

using namespace ::util;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace player {
	static const unsigned int buffSize = 188 * 1024;
	static const char dataBuffer[buffSize] = "";

	AVPlayer::AVPlayer(string mrl, bool hasVisual) : Thread(), Player(mrl) {
		

		this->provider    = NULL;
		this->mainAV      = false;
		this->buffered    = false;
		this->isRemoteMrl = false;

		this->status      = STOP;
		this->running     = false;
		this->hasVisual   = hasVisual;
		this->soundLevel  = 1.0;
		this->win         = NULL;
		loadMrl(mrl);

	}

	void AVPlayer::loadMrl(string mrl) {
		string::size_type pos;
		if (mrl.length() > 11 && mrl.substr(0, 11) == "sbtvd-ts://") {
			this->mainAV = true;
			pos = mrl.find("#");
			if (pos != std::string::npos) {
				this->mrl = mrl.substr(pos + 1, mrl.length() - (pos + 1));

			} else {
				this->mrl = mrl.substr(11, mrl.length() - 11);
			}

		} else if (fileExists(mrl)) {
			this->provider = new DummyConnector(mrl.c_str(), hasVisual);
			this->surface = this->provider->createFrame();
			this->scopeEndTime = getTotalMediaTime();

		} else {
			cout << "AVPlayer::AVPlayer Warning! File not Found: '";
			cout << mrl << "'" << endl;
			this->provider = NULL;
			this->surface = NULL;
		}
	}

	AVPlayer::~AVPlayer() {
		cout << "AVPlayer::~AVPlayer(" << mrl << ")" << endl;
		if (status != STOP) {
			stop();
		}

		unlockConditionSatisfied();
		lock();
		if (this->provider != NULL) {
			release();
		}
		unlock();
	}

	void AVPlayer::finished() {
		cout << "AVPlayer::finished" << endl;
	}

	double AVPlayer::getEndTime() {
		return scopeEndTime;
	}

	bool AVPlayer::getHasVisual() {
		return this->hasVisual;
	}

	void AVPlayer::setSoundLevel(float level) {
		this->soundLevel = level;
		if (provider != NULL) {
			provider->setSoundLevel(soundLevel);
		}
	}

	ISurface* AVPlayer::createFrame() {
		assert(false);
		return NULL;
		/*
		//wclog << "AVPlayer::createFrame()" << endl;
		lock();
		if (surface != NULL) {
			cout << "AVPlayer::createFrame Warning! surface != NULL";
			cout << endl;
			delete surface;
		}
		surface = provider->getPerfectSurface();
		unlock();
		return surface;
		*/
	}

	void AVPlayer::getOriginalResolution(int* height, int* width) {
		if (provider != NULL) {
			provider->getOriginalResolution(height, width);
		}
	}

	double AVPlayer::getTotalMediaTime() {
		if (provider != NULL) {
			return provider->getTotalMediaTime();
		}

		this->wakeUp();
		return 0;
	}

	void AVPlayer::timeShift(string direction) {
/*		if (provider != NULL) {
			if (direction == "forward") {
				provider->setMediaTime(provider->getMediaTime() + 10);

			} else if (direction == "backward") {
				provider->setMediaTime(provider->getMediaTime() - 10);
			}
		}*/
	}

	double AVPlayer::getMediaTime() {
		if (provider == NULL) {
			this->wakeUp();
			return -1;
		}

		return provider->getMediaTime();
	}

	void AVPlayer::setMediaTime(double pos) {
		if (status == PLAY) {
			status = PAUSE;
			this->wakeUp();
			provider->setMediaTime(pos);
			status = PLAY;
			running = true;
			Thread::start();

		} else if (provider != NULL) {
			provider->setMediaTime(pos);
		}
	}

	void AVPlayer::setStopTime(double pos) {
		if (status == PLAY) {
			status = PAUSE;
			this->wakeUp();
			scopeEndTime = pos;
			status = PLAY;
			running = true;
			Thread::start();

		} else {
			scopeEndTime = pos;
		}
	}

	double AVPlayer::getStopTime() {
	     return scopeEndTime;
	}

	void AVPlayer::setScope(
			string scope, short type, double begin, double end) {

		Player::setScope(scope, type, begin, end);
		if (type == TYPE_PRESENTATION) {
			if (scopeInitTime > 0) {
				setMediaTime(scopeInitTime);
			}

			if (scopeEndTime > 0) {
				setStopTime(scopeEndTime);
			}
		}
	}

	void AVPlayer::play() {
		if (provider == NULL) {
			cout << "AVPlayer::play() can't play, provider is NULL" << endl;
			this->wakeUp();
			return;
		}

		Player::play();
		if (!isRemoteMrl) {
			cout << "void AVPlayer::play(" << mrl << ")" << endl;
			provider->play(false, this);
		}

		if (!running) {
			running = true;
			Thread::start();
		}
	}

	void AVPlayer::pause() {
		status = PAUSE;
		if (provider == NULL) {
			this->wakeUp();
			return;
		}

		provider->pause();
		this->wakeUp();
	}

	void AVPlayer::stop() {
		Player::stop();
		if (provider == NULL) {
			this->wakeUp();
			return;
		}
		provider->stop();
		this->wakeUp();
	}

	void AVPlayer::resume() {
		setSoundLevel(soundLevel);

		Player::play();
		if (!isRemoteMrl) {
			cout << "void AVPlayer::play(" << mrl << ")" << endl;
			provider->resume();
		}

		if (!running) {
			running = true;
			Thread::start();
		}
	}

	void AVPlayer::setPropertyValue(
			string name, string value, double duration, double by) {

		float fValue = 1.0;
		
		//TODO: animation, set volume, brightness, ...
		if (name == "soundLevel") {
			if (value != "") {
				fValue = stof(value);
			}
			setSoundLevel(fValue);
		} else if (name == "src") {
			if (value != "") {
				loadMrl(value);	
			}		
		}

		Player::setPropertyValue(name, value, duration, by);
	}

	void AVPlayer::addListener(IPlayerListener* listener) {
		Player::addListener(listener);
	}

	void AVPlayer::release() {
		if (provider == NULL) {
			return;
		}

		provider->stop();
		DEL(provider)

	}

	void AVPlayer::setMrl(const char* mrl) {
		this->mrl = mrl;
	}

	string AVPlayer::getMrl() {
		return mrl;
	}

	bool AVPlayer::isPlaying() {
		if (provider == NULL) {
			return false;
		}

		if ((getMediaTime() <= 0 && status != PAUSE) ||
			    (getMediaTime() >= getStopTime() &&
			    getStopTime() > 0)) {

			return false;

		} else {
			return true;
		}
	}

	void AVPlayer::setVoutWindow(int x, int y, int w, int h) {
		if (hasVisual) {
			provider->setVoutWindow("", x, y, w, h);
		}
	}


	bool AVPlayer::isRunning() {
		return running;
	}


	void AVPlayer::run() {
		double dur;
		double currentTime;
		double timeRemain;

		lock();
		if (mainAV) {
			cout << "AVPlayer::run mainAV mrl: '" << mrl << "'" << endl;
			running = true;

			if (this->win != NULL && provider->surface->getParent() == NULL) {
				provider->surface->setParent((void*)win);
			}

			provider->play(true, NULL);//aca llamar al setVoutWindow
			buffered = true;
			waitForUnlockCondition();

		} else {
			if (!isInfinity(scopeEndTime) &&
					scopeEndTime > 0 && scopeEndTime < getTotalMediaTime()) {

				dur = getStopTime();

			} else {
				dur = getTotalMediaTime();
			}

			::usleep(850000);
			currentTime = getMediaTime();
			if (currentTime > 0) {
				while (dur > (currentTime + 0.1)) {

					if (status != PLAY) {
						break;
					}

					timeRemain = (dur - currentTime) * 1000;
					if (notifyContentUpdate) {
						if (timeRemain > 250) {
							notifyListeners(
									PL_NOTIFY_UPDATECONTENT,
									"", TYPE_PASSIVEDEVICE);
						}

						if (!this->usleep(65)) { // 15 fps
							break;
						}

					} else if (!this->usleep(timeRemain)) {
						break;
					}

					currentTime = getMediaTime();
					if (currentTime <= 0) {
						break;
					}
				}
			}
		}

		if (status != PAUSE) {
			presented = true;
		}

		if (provider != NULL) {
			provider->stop();
		}

		if (status != STOP && status != PAUSE) {
			status = STOP;
			running = false;
			unlock();
			notifyListeners(PL_NOTIFY_STOP, "");

		} else {
			status = STOP;
			running = false;
			unlock();
		}

	}
}
}
}
}
}
}

using namespace ::br::pucrio::telemidia::ginga::core::player;

static pthread_mutex_t avpm;
static bool avpmInit = false;

extern "C" IPlayer* createAVPlayer(const char* mrl, bool hasVisual) {
	AVPlayer* player;
	if (!avpmInit) {
		avpmInit = true;
		pthread_mutex_init(&avpm, NULL);
	}

	pthread_mutex_lock(&avpm);
	player = new AVPlayer(mrl, hasVisual);
	pthread_mutex_unlock(&avpm);

	return player;
}

extern "C" void destroyAVPlayer(
		::br::pucrio::telemidia::ginga::core::player::IPlayer* p) {

	delete p;
}
