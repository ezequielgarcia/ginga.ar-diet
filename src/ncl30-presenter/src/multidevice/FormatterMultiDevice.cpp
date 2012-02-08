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

#include "../../include/multidevice/FormatterMultiDevice.h"
#include "../../include/multidevice/FormatterBaseDevice.h"
#include "../../include/multidevice/FormatterPassiveDevice.h"
#include "../../include/multidevice/FormatterActiveDevice.h"
#include "../../include/multidevice/FMDComponentSupport.h"

#include "system/io/interface/input/CodeMap.h"

#include "ncl/layout/DeviceLayout.h"
using namespace ::br::pucrio::telemidia::ncl::layout;

#include "../../include/adaptation/context/PresentationContext.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::adaptation::context;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace multidevice {
	ILocalDeviceManager* FormatterMultiDevice::dm   = NULL;
	IInputManager* FormatterMultiDevice::im         = NULL;
	IRemoteDeviceManager* FormatterMultiDevice::rdm = NULL;

	pthread_mutex_t FormatterMultiDevice::mutex;

	FormatterMultiDevice::FormatterMultiDevice(
			IDeviceLayout* deviceLayout, int x, int y, int w, int h) {

		this->xOffset          = x;
		this->yOffset          = y;
		this->defaultWidth     = w;
		this->defaultHeight    = h;
		this->deviceClass      = -1;
		this->hasRemoteDevices = false;
		this->deviceLayout     = deviceLayout;
		this->layoutManager    = new map<int, FormatterLayout*>;
		this->activeBaseUri    = "";
		this->activeUris       = NULL;
		this->bitMapScreen     = NULL;
		this->serialized       = NULL;

		if (im == NULL) {
			pthread_mutex_init(&FormatterMultiDevice::mutex, NULL);

#if HAVE_COMPSUPPORT
			dm = ((LocalDeviceManagerCreator*)(
					cm->getObject("LocalDeviceManager")))();
#else
			dm = LocalDeviceManager::getInstance();
#endif

			if (defaultWidth == 0) {
				defaultWidth = dm->getDeviceWidth();
			}

			if (defaultHeight == 0) {
				defaultHeight = dm->getDeviceHeight();
			}

#if HAVE_COMPSUPPORT
			im = ((InputManagerCreator*)(cm->getObject("InputManager")))();
#else
			im = InputManager::getInstance();
#endif

			im->setAxisBoundaries(defaultWidth, defaultHeight, 0);
			im->setAxisValues(
					(int)(dm->getDeviceWidth() / 2),
					(int)(dm->getDeviceHeight() / 2), 0);
		}

#if HAVE_COMPSUPPORT
		printScreen = ((WindowCreator*)(cm->getObject("Window")))(
				0, 0, defaultWidth, defaultHeight);
#else
		printScreen = new DFBWindow(0, 0, defaultWidth, defaultHeight);
#endif

		printScreen->setCaps(printScreen->getCap("ALPHACHANNEL"));
		printScreen->draw();

	}

	FormatterMultiDevice::~FormatterMultiDevice() {
		im->removeInputEventListener(this);
                delete printScreen;
	}

	void FormatterMultiDevice::setBackgroundImage(string uri) {
		dm->setBackgroundImage(uri);
	}

	void* FormatterMultiDevice::getMainLayout() {
		return mainLayout;
	}

	void* FormatterMultiDevice::getFormatterLayout(int devClass) {
		map<int, FormatterLayout*>::iterator i;

		i = layoutManager->find(devClass);
		if (i != layoutManager->end()) {
			return i->second;
		}

		return NULL;
	}

	string FormatterMultiDevice::getScreenShot() {
		return serializeScreen(deviceClass, printScreen);
	}

	string FormatterMultiDevice::serializeScreen(
			int devClass, IWindow* mapWindow) {

		string fileUri = "";
		FormatterLayout* formatterLayout;
		map<int, FormatterLayout*>::iterator i;
		int quality = 100;
		int dumpW = defaultWidth;
		int dumpH = defaultHeight;

		i = layoutManager->find(devClass);
		if (i != layoutManager->end()) {
			formatterLayout = i->second;
			mapWindow->clearContent();
			if (formatterLayout->getScreenShot(mapWindow)) {
				if (devClass == 1) {
					quality = 45;
					dumpW   = 480 / 1.8;
					dumpH   = 320 / 1.8;
				}
				fileUri = mapWindow->getDumpFileUri(quality, dumpW, dumpH);
			}
		}

		return fileUri;
	}

	void FormatterMultiDevice::postMediaContent(int destDevClass) {
		string fileUri;

		/*cout << "FormatterMultiDevice::postMediaContent to class '";
		cout << destDevClass << "'";
		cout << endl;*/

		pthread_mutex_lock(&FormatterMultiDevice::mutex);
		if (destDevClass == IDeviceDomain::CT_PASSIVE) {
			fileUri = serializeScreen(destDevClass, serialized);
			if (fileUri != "" && fileExists(fileUri)) {
				rdm->postMediaContent(destDevClass, fileUri);
				if (bitMapScreen != NULL) {
					bitMapScreen->clearContent();
					bitMapScreen->stretchBlit(serialized);
					bitMapScreen->show();
					bitMapScreen->validate();
					//renderFromUri(bitMapScreen, fileUri);
				}
			}

		} else if (destDevClass == IDeviceDomain::CT_ACTIVE) {
			if (!activeUris->empty()) {

			}
		}
		pthread_mutex_unlock(&FormatterMultiDevice::mutex);
	}

	FormatterLayout* FormatterMultiDevice::getFormatterLayout(
			CascadingDescriptor* descriptor) {

		map<int, FormatterLayout*>::iterator i;
		FormatterLayout* layout;
		LayoutRegion* region;
		int devClass;

		region = descriptor->getRegion();
		if (region == NULL) {
			cout << "FormatterMultiDevice::getFormatterLayout region is NULL";
			cout << endl;
			return NULL;
		}

		devClass = region->getDeviceClass();
		i = layoutManager->find(devClass);

		if (i == layoutManager->end()) {
			if (devClass == IDeviceDomain::CT_PASSIVE) {
				layout = new FormatterLayout(
						0, 0, DV_QVGA_WIDTH, DV_QVGA_HEIGHT);

				(*layoutManager)[devClass] = layout;
				return layout;
			}

			cout << "FormatterMultiDevice::getFormatterLayout NOT FOUND for ";
			cout << "class '" << devClass << "'" << endl;
			return NULL;

		} else {
			if (devClass == IDeviceDomain::CT_PASSIVE) {
				/*cout << "FormatterMultiDevice::getFormatterLayout FOUND";
				cout << " CT_PASSIVE in LAYOUTMANAGER" << endl;*/
			}
			return i->second;
		}
	}

	void FormatterMultiDevice::prepareFormatterRegion(
			ExecutionObject* executionObject,
			ISurface* renderedSurface) {

		FormatterLayout* layout;
		CascadingDescriptor* descriptor;
		string regionId;

		map<int, FormatterLayout*>::iterator i;
		LayoutRegion* bitMapRegion;

		descriptor = executionObject->getDescriptor();
		if (descriptor != NULL) {
			layout = getFormatterLayout(descriptor);
			if (layout != NULL) {
				layout->prepareFormatterRegion(
						executionObject, renderedSurface);

				if (bitMapScreen != NULL) {
					/*cout << endl;
					cout << "FormatterMultiDevice::prepareFormatterRegion ";
					cout << "bitMapScreen != NULL" << endl;*/
					return;
				}

				regionId = layout->getBitMapRegionId();
				/*cout << endl;
				cout << "FormatterMultiDevice::prepareFormatterRegion map '";
				cout << regionId << "'" << endl;*/

				if (regionId == "") {
					return;
				}

				i = layoutManager->find(deviceClass);
				if (i == layoutManager->end()) {
					/*cout << endl;
					cout << "FormatterMultiDevice::prepareFormatterRegion ";
					cout << "CANT FIND devClass '" << deviceClass << "'";
					cout << endl;*/
					return;
				}

				layout = i->second;
				bitMapRegion = layout->getNcmRegion(regionId);

				if (bitMapRegion == NULL) {
					/*cout << endl;
					cout << "FormatterMultiDevice::prepareFormatterRegion ";
					cout << "CANT FIND bitMapRegion(" << bitMapRegion << ")";
					cout << " for id '" << regionId << "' devClass = '";
					cout << deviceClass << "'" << endl;
					cout << endl;*/
					return;
				}

#if HAVE_COMPSUPPORT
				bitMapScreen = ((WindowCreator*)(cm->getObject("Window")))(
						bitMapRegion->getAbsoluteLeft(),
						bitMapRegion->getAbsoluteTop(),
						bitMapRegion->getWidthInPixels(),
						bitMapRegion->getHeightInPixels());
#else
				bitMapScreen = new DFBWindow(
						bitMapRegion->getAbsoluteLeft(),
						bitMapRegion->getAbsoluteTop(),
						bitMapRegion->getWidthInPixels(),
						bitMapRegion->getHeightInPixels());
#endif

				/*cout << endl << endl;
				cout << "FormatterMultiDevice::prepareFormatterRegion '";
				cout << regionId << "' left = '";
				cout << bitMapRegion->getLeftInPixels();
				cout << "' top = '" << bitMapRegion->getTopInPixels();
				cout << "' width = '" << bitMapRegion->getWidthInPixels();
				cout << "' height = '" << bitMapRegion->getHeightInPixels();
				cout << endl << endl;*/

				bitMapScreen->setCaps(bitMapScreen->getCap("ALPHACHANNEL"));
				bitMapScreen->draw();
			}
		}
	}

	void FormatterMultiDevice::showObject(ExecutionObject* executionObject) {
		FormatterLayout* layout;
		CascadingDescriptor* descriptor;
		FormatterRegion* fRegion;
		LayoutRegion* region;
		int devClass;
		string fileUri;
		string url;
		string relativePath;
		Content* content;

		/*INCLSectionProcessor* nsp = NULL;
		vector<StreamData*>* streams;*/

		descriptor = executionObject->getDescriptor();
		if (descriptor != NULL) {
			region = descriptor->getRegion();
			layout = getFormatterLayout(descriptor);
			if (region != NULL && layout != NULL) {
				devClass = region->getDeviceClass();

				/*cout << "FormatterMultiDevice::showObject '";
				cout << executionObject->getId() << "' class '";
				cout << devClass << "'" << endl;*/

				fRegion = descriptor->getFormatterRegion();
				if (devClass != IDeviceDomain::CT_BASE) {
					if (fRegion != NULL) {
						fRegion->setGhostRegion(true);
					}
				}

				if (devClass != IDeviceDomain::CT_ACTIVE) {
					layout->showObject(executionObject);
				}

				if (hasRemoteDevices) {
					if (devClass == IDeviceDomain::CT_PASSIVE) {
						postMediaContent(devClass);

					} else if (devClass == IDeviceDomain::CT_ACTIVE) {
						//cout << "activeBaseUri: "<<activeBaseUri<<endl;
						//cout << "activeUris: "<<activeUris<<endl;

						content = ((NodeEntity*)(
								executionObject->getDataObject()->
										getDataEntity()))->getContent();

						relativePath = "";

						if (content != NULL && content->instanceOf(
								"ReferenceContent")) {

							url = ((ReferenceContent*)content)->
									getCompleteReferenceUrl();

							cout << "FormatterMultiDevice::showObject ";
							cout << "executionObject.url = '" << url;
							cout << "'" << endl;

							cout << "FormatterMultiDevice::showObject ";
							cout << "executionObject.activeBaseUri = '";
							cout << activeBaseUri << "'" << endl;

							relativePath = url.substr(
									activeBaseUri.size(),
									url.size() - activeBaseUri.size());

							cout << "FormatterMultiDevice::showObject ";
							cout << "executionObject.RP = '";
							cout << relativePath << "'" << endl;
						}

						rdm->postEvent(devClass,
								IDeviceDomain::FT_PRESENTATIONEVENT,
								(char*)("start::" + relativePath).c_str(),
								("start::" + relativePath).size());

						/**streams = nsp->createNCLSections(
								"0x01.0x01",
								executionObject->getId(),
								activeBaseUri,
								activeUris,
								NULL);

						rdm->postNclMetadata(devClass, streams);

						fileUri = "start::" + executionObject->getId();*
						rdm->postEvent(
								devClass,
								IDeviceDomain::FT_PRESENTATIONEVENT,
								(char*)(fileUri.c_str()),
								fileUri.length());*/
					}
				}
			}
		}
	}

	void FormatterMultiDevice::hideObject(ExecutionObject* executionObject) {
		FormatterLayout* layout;
		CascadingDescriptor* descriptor;
		LayoutRegion* region;
		int devClass;
		string fileUri;

		descriptor = executionObject->getDescriptor();
		if (descriptor != NULL) {
			region = descriptor->getRegion();
			layout = getFormatterLayout(descriptor);
			if (region != NULL && layout != NULL) {
				devClass = region->getDeviceClass();
				if (devClass != IDeviceDomain::CT_ACTIVE) {
					/*cout << "FormatterMultiDevice::hideObject '";
					cout << executionObject->getId() << "' class '";
					cout << devClass << "'" << endl;*/


					layout->hideObject(executionObject);
				}

				if (hasRemoteDevices) {
						if (devClass == IDeviceDomain::CT_PASSIVE) {
							postMediaContent(devClass);

						} else if (devClass == IDeviceDomain::CT_ACTIVE) {
							//TODO: TAB FORMAT
							Content* content;
							content = ((NodeEntity*)(executionObject->getDataObject()->
							          getDataEntity()))->getContent();
							string relativePath = "";

							if (content != NULL && content->instanceOf("ReferenceContent")) {
								string url = ((ReferenceContent*)content)->getCompleteReferenceUrl();

								cout<<"FormatterMultiDevice::showObject executionObject.url="<<url<<endl;
								cout<<"FormatterMultiDevice::showObject executionObject.activeBaseUri="<<activeBaseUri<<endl;
								relativePath = url.substr(activeBaseUri.size(),url.size()-activeBaseUri.size());

								cout<<"FormatterMultiDevice::showObject executionObject.RP="<<relativePath<<endl;
							}

							rdm->postEvent(devClass,
									IDeviceDomain::FT_PRESENTATIONEVENT,
									(char*)("stop::" +relativePath).c_str(),
									("stop::" +relativePath).size());
							/*
							fileUri = "stop::" + executionObject->getId();
							rdm->postEvent(
									devClass,
									IDeviceDomain::FT_PRESENTATIONEVENT,
									(char*)(fileUri.c_str()),
									fileUri.length()); */
						}
				}
			}
		}
	}

	void FormatterMultiDevice::renderFromUri(IWindow* win, string uri) {
		ISurface* s;
		IPlayer* img;

#if HAVE_COMPSUPPORT
		img = ((PlayerCreator*)(cm->getObject(
				"ImagePlayer")))(uri.c_str(), true);
#else
		img = new ImagePlayer(uri.c_str());
#endif

		s = img->getSurface();

		win->setColorKey(0, 0, 0);
		win->clearContent();
		win->renderFrom(s);
		win->show();
		win->validate();

		delete img;
	}

	void FormatterMultiDevice::tapObject(int devClass, int x, int y) {
		FormatterLayout* layout;
		ExecutionObject* object;

		if (layoutManager->count(devClass) != 0) {
			layout = (*layoutManager)[devClass];

			object = layout->getObject(x, y);
			if (object != NULL && object->getDescriptor() != NULL &&
					object->getDescriptor()->getFormatterRegion() != NULL &&
					object->getDescriptor()->getFormatterRegion()->isVisible()) {

				cout << "FormatterMultiDevice::tapObject '";
				cout << object->getId() << "'" << endl;
				object->selectionEvent(CodeMap::KEY_NULL, 0);

			} else {
				cout << "FormatterMultiDevice::tapObject can't ";
				cout << "find object at '" << x << "' and '";
				cout << y << "' coords" << endl;
			}

		} else {
			cout << "FormatterMultiDevice::tapObject can't find layout of '";
			cout << devClass << "' device class" << endl;
		}
	}

	bool FormatterMultiDevice::newDeviceConnected(
			int newDevClass,
			int w,
			int h) {

		bool isNewClass = false;
		/*INCLSectionProcessor* nsp = NULL;
		vector<StreamData*>* streams;*/

		cout << "FormatterMultiDevice::newDeviceConnected class '";
		cout << newDevClass << "', w = '" << w << "', h = '" << h << "'";
		cout << endl;

		PresentationContext::getInstance()->incPropertyValue(
				ContextBase::SYSTEM_DEVNUMBER + "(" + itos(newDevClass) + ")");

		if (!hasRemoteDevices) {
			hasRemoteDevices = true;
		}

		if (layoutManager->count(newDevClass) == 0) {
			(*layoutManager)[newDevClass] = new FormatterLayout(0, 0, w, h);
			isNewClass = true;
		}

		if (newDevClass == IDeviceDomain::CT_ACTIVE) {
			cout << "FormatterMulDevice::newDeviceConnected class = ";
			cout << IDeviceDomain::CT_ACTIVE << endl;

			/*streams = nsp->createNCLSections(
					"0x01.0x01",
					"nclApp",
					activeBaseUri,
					activeUris,
					NULL);

			rdm->postNclMetadata(newDevClass, streams);*/

		} else {
			postMediaContent(newDevClass);
		}

		return isNewClass;
	}

	bool FormatterMultiDevice::receiveRemoteEvent(
			int remoteDevClass,
			int eventType,
			string eventContent) {

		vector<string>* params;
		int eventCode;

		/*cout << "FormatterActiveDevice::receiveRemoteEvent from class '";
		cout << remoteDevClass << "', eventType '" << eventType << "', ";
		cout << "eventContent = '" << eventContent << "'" << endl;*/

		if (remoteDevClass == IDeviceDomain::CT_PASSIVE &&
				eventType == IDeviceDomain::FT_SELECTIONEVENT) {

			if (eventContent.find(",") != std::string::npos) {
				params = split(eventContent, ",");
				if (params != NULL) {
					if (params->size() == 3) {
						string strCode, strX, strY;
						strCode = (*params)[0];
						eventCode = CodeMap::getInstance()->getCode(strCode);
						if (eventCode == CodeMap::KEY_TAP) {
							strX    = (*params)[1];
							strY    = (*params)[2];

							tapObject(
									IDeviceDomain::CT_PASSIVE,
									(int)stof(strX),
									(int)stof(strY));

						} else if (eventCode != CodeMap::KEY_NULL) {
							im->postEvent(eventCode);
						}
					}
					delete params;
				}

			} else {
				eventCode = CodeMap::getInstance()->getCode(eventContent);
				if (eventCode != CodeMap::KEY_NULL) {
					im->postEvent(eventCode);
				}
			}
		}

		return true;
	}

	void FormatterMultiDevice::addActiveUris(
			string baseUri, vector<string>* uris) {

		if (activeUris != NULL) {
			delete activeUris;
		}

		activeUris    = uris;
		activeBaseUri = baseUri;
		cout << "FormatterMultiDevice::addActiveUris activeBaseUri="<<baseUri<<endl;
	}

	void FormatterMultiDevice::updatePassiveDevices() {
		postMediaContent(IDeviceDomain::CT_PASSIVE);
	}

	void FormatterMultiDevice::updateStatus(
			short code, string parameter, short type) {

		switch(code) {
			case IPlayer::PL_NOTIFY_UPDATECONTENT:
				if (type == IPlayer::TYPE_PASSIVEDEVICE) {
					FormatterMultiDevice::updatePassiveDevices();
				}

			default:
				break;
		}
	}
}
}
}
}
}
}

extern "C" ::br::pucrio::telemidia::ginga::ncl::multidevice::
		IFormatterMultiDevice* createFormatterMultiDevice(
				IDeviceLayout* deviceLayout,
				int devClass,
				int x,
				int y,
				int w,
				int h) {

	if (devClass == IDeviceDomain::CT_BASE) {
		return (new ::br::pucrio::telemidia::ginga::ncl::multidevice::
				FormatterBaseDevice(deviceLayout, x, y, w, h));

	} else if (devClass == IDeviceDomain::CT_PASSIVE) {
		return (new ::br::pucrio::telemidia::ginga::ncl::multidevice::
				FormatterPassiveDevice(deviceLayout, x, y, w, h));

	} else if (devClass == IDeviceDomain::CT_ACTIVE) {
		return (new ::br::pucrio::telemidia::ginga::ncl::multidevice::
				FormatterActiveDevice(deviceLayout, x, y, w, h));
	}

	return NULL;
}

extern "C" void destroyFormatterMultiDevice(
		::br::pucrio::telemidia::ginga::ncl::multidevice::
				IFormatterMultiDevice* fmd) {

	delete fmd;
}
