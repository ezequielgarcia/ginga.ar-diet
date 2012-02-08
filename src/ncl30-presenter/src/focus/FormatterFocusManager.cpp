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

#include "../../include/focus/FormatterFocusManager.h"
#include "generated/config.h"

#if HAVE_COMPSUPPORT
#include "cm/IComponentManager.h"
using namespace ::br::pucrio::telemidia::ginga::core::cm;
#else
#include <system/io/InputManager.h>
#endif

#include <multidevice/services/IDeviceDomain.h>
using namespace ::br::pucrio::telemidia::ginga::core::multidevice;

#include "../../include/emconverter/FormatterConverter.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::emconverter;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace focus {
#if HAVE_COMPSUPPORT
	static IComponentManager* cm = IComponentManager::getCMInstance();
#endif

	IInputManager* FormatterFocusManager::im     = NULL;

	FormatterFocusManager::FormatterFocusManager(
		    PlayerAdapterManager* playerManager,
		    IFormatterMultiDevice* multiDevice,
		    void* converter) {

		string strValue;

		if (im == NULL) {
#if HAVE_COMPSUPPORT
			im = ((InputManagerCreator*)(cm->getObject("InputManager")))();
#else
			im = InputManager::getInstance();
#endif
		}

		focusTable     = new map<string, set<ExecutionObject*>*>;
		focusSequence  = new vector<string>;
		currentFocus   = "";
		objectToSelect = "";
		isHandler      = false;
		parentManager  = NULL;

		strValue = PresentationContext::getInstance()->getPropertyValue(
				ContextBase::DEFAULT_FOCUS_BORDER_COLOR);

		if (strValue == "") {
			defaultFocusBorderColor = new Color("blue");

		} else {
			defaultFocusBorderColor = new Color(strValue);
		}

		strValue = PresentationContext::getInstance()->getPropertyValue(
				ContextBase::DEFAULT_FOCUS_BORDER_WIDTH);

		if (strValue == "") {
			defaultFocusBorderWidth = -3;

		} else {
			defaultFocusBorderWidth = stof(strValue);
		}

		strValue = PresentationContext::getInstance()->getPropertyValue(
				ContextBase::DEFAULT_SEL_BORDER_COLOR);

		if (strValue == "") {
			defaultSelBorderColor = new Color("green");

		} else {
			defaultSelBorderColor = new Color(strValue);
		}

		this->selectedObject = NULL;
		this->playerManager  = playerManager;
		this->multiDevice    = multiDevice;
		this->converter      = converter;

		pthread_mutex_init(&mutexFocus, NULL);
		pthread_mutex_init(&mutexTable, NULL);
/*
		cout << "FormatterFocusManager::FormatterFocusManager(" << this;
		cout << ")" << endl;*/
	}

	FormatterFocusManager::~FormatterFocusManager() {
		pthread_mutex_lock(&mutexFocus);
		pthread_mutex_lock(&mutexTable);

		/*cout << "FormatterFocusManager::~FormatterFocusManager(";
		cout << this << ")";
		cout << "currentFocus is '" << currentFocus << "' and ";
		cout << "parentCurrentFocus is '" << parentManager;
		cout << "'" << endl;*/

		unregister();
		if (parentManager != NULL) {
			if (isHandler && !parentManager->setKeyHandler(true)) {
				parentManager->registerNavigationKeys();
			}
		}
		selectedObject = NULL;

		if (defaultFocusBorderColor != NULL) {
			delete defaultFocusBorderColor;
			defaultFocusBorderColor = NULL;
		}

		if (defaultSelBorderColor != NULL) {
			delete defaultSelBorderColor;
			defaultSelBorderColor = NULL;
		}

		delete focusSequence;
		focusSequence = NULL;

		delete focusTable;
		focusTable = NULL;

		playerManager = NULL;
		pthread_mutex_unlock(&mutexFocus);
		pthread_mutex_destroy(&mutexFocus);
		pthread_mutex_unlock(&mutexTable);
		pthread_mutex_destroy(&mutexTable);
	}

	void FormatterFocusManager::setParent(FormatterFocusManager* parent) {
		parentManager = parent;
	}

	bool FormatterFocusManager::setKeyHandler(bool isHandler) {
		cout << "FormatterFocusManager::setKeyHandler(" << this << ")";
		cout << " isHandler '" << isHandler << "'";
		cout << " this->isHandler '" << this->isHandler << "'";
		cout << " parentManager '" << parentManager << "'";
		cout << endl;

		if (this->isHandler == isHandler) {
			return false;
		}

		this->isHandler = isHandler;
		setHandlingObjects(isHandler);
		if (isHandler) {
			registerNavigationKeys();

		} else {
			unregister();
		}

		return isHandler;
	}

	void FormatterFocusManager::setStandByState(bool standBy) {
		if (!isHandler) {
			return;
		}

		if (standBy) {
			if (parentManager != NULL) {
				parentManager->unregister();
			}
			unregister();

		} else {
			if (parentManager != NULL) {
				parentManager->registerBackKeys();
			}
			registerNavigationKeys();
		}
	}

	void FormatterFocusManager::setHandlingObjects(bool isHandling) {
		((FormatterConverter*)converter)->setHandlingStatus(isHandling);
	}

	ExecutionObject* FormatterFocusManager::getObjectFromFocusIndex(
			string focusIndex) {

		map<string, set<ExecutionObject*>*>::iterator i;
		set<ExecutionObject*>::iterator j;
		CascadingDescriptor* desc;
		bool visible;

		i = focusTable->find(focusIndex);
		if (i == focusTable->end()) {
			return NULL;
		}

		j = i->second->begin();
		while (j != i->second->end()) {
			desc = (*j)->getDescriptor();
			if (desc != NULL && desc->getFormatterRegion() != NULL) {
				visible = desc->getFormatterRegion()->isVisible();
				if (visible) {
					return *j;
				}
			}
			++j;
		}

		delete i->second;
		focusTable->erase(i);

		return NULL;
	}

	void FormatterFocusManager::insertObject(
			ExecutionObject* obj, string focusIndex) {

		string auxIndex;
		map<string, set<ExecutionObject*>*>::iterator i;
		vector<string>::iterator j;
		set<ExecutionObject*>* objs;

		i = focusTable->find(focusIndex);
		if (i != focusTable->end()) {
			objs = i->second;

		} else {
			objs = new set<ExecutionObject*>;
			(*focusTable)[focusIndex] = objs;
		}

		objs->insert(obj);

		// put new index in alphabetical order
		for (j = focusSequence->begin(); j != focusSequence->end(); ++j) {
			auxIndex = *j;
			if (focusIndex.compare(auxIndex) < 0) {
				break;

			} else if (focusIndex.compare(auxIndex) == 0) {
				return;
			}
		}

		focusSequence->insert(j, focusIndex);
	}

	void FormatterFocusManager::removeObject(
			ExecutionObject* obj, string focusIndex) {

		map<string, set<ExecutionObject*>*>::iterator i;
		set<ExecutionObject*>::iterator j;
		vector<string>::iterator k;
		set<ExecutionObject*>* objs;

		i = focusTable->find(focusIndex);
		if (i != focusTable->end()) {
			objs = i->second;
			j = objs->find(obj);
			if (j != objs->end()) {
				objs->erase(j);
			}
			if (objs->empty()) {
				delete objs;
				focusTable->erase(i);
			}
		}

		k = focusSequence->begin();
		while (k != focusSequence->end()) {
			if (*k == focusIndex) {
				focusSequence->erase(k);
				k = focusSequence->begin();

			} else {
				++k;
			}
		}
	}

	void FormatterFocusManager::setKeyMaster(string focusIndex) {
		ExecutionObject* nextObject = NULL;
		ExecutionObject* currentObject = NULL;
		CascadingDescriptor* currentDescriptor = NULL;
		CascadingDescriptor* nextDescriptor = NULL;
		FormatterPlayerAdapter* player = NULL;
		LayoutRegion* ncmRegion;
		bool isId = false;

		pthread_mutex_lock(&mutexTable);
		pthread_mutex_lock(&mutexFocus);

		if (focusIndex == "" && selectedObject != NULL) {
			objectToSelect = "";
			selectedObject->setHandler(false);
			selectedObject->getDescriptor()->
					getFormatterRegion()->setSelection(false);

			recoveryDefaultState(selectedObject);

			player = playerManager->getPlayer(selectedObject);
			exitSelection(player);
			pthread_mutex_unlock(&mutexTable);
			pthread_mutex_unlock(&mutexFocus);
			return;
		}

		nextObject = getObjectFromFocusIndex(focusIndex);
		if (nextObject == NULL) {
			nextObject = ((FormatterConverter*)converter)->getObjectFromNodeId(
					focusIndex);

			if (nextObject == NULL) {
				cout << "FormatterFocusManager::setKeyMaster can't set '";
				cout << focusIndex << "' as master: object is not available.";
				cout << endl;

				objectToSelect = focusIndex;
				pthread_mutex_unlock(&mutexTable);
				pthread_mutex_unlock(&mutexFocus);
				return;
			}

			isId = true;
		}

		nextDescriptor = nextObject->getDescriptor();
		if (nextDescriptor == NULL) {
			cout << "FormatterFocusManager::setKeyMaster can't set '";
			cout << focusIndex << "' as master: NULL descriptor";
			cout << endl;

			if (isId) {
				objectToSelect = focusIndex;
			}

			pthread_mutex_unlock(&mutexTable);
			pthread_mutex_unlock(&mutexFocus);
			return;
		}

		currentObject = getObjectFromFocusIndex(currentFocus);
		if (currentObject != NULL && currentObject != nextObject) {
			pthread_mutex_unlock(&mutexTable);
			//recovering focused object
			currentDescriptor = currentObject->getDescriptor();
			if (currentDescriptor != NULL) {
				ncmRegion = currentDescriptor->getRegion();
				if (currentDescriptor->getFormatterRegion() != NULL) {
					currentDescriptor->getFormatterRegion()->setFocus(false);
				}
			}
			recoveryDefaultState(currentObject);

		} else {
			pthread_mutex_unlock(&mutexTable);
		}

		if (!isId) {
			currentFocus = focusIndex;

		} else {
			currentFocus = nextDescriptor->getFocusIndex();
		}

		if (nextDescriptor->getFormatterRegion() == NULL) {
			::usleep(100000);
		}

		if (nextDescriptor->getFormatterRegion() != NULL) {
			if (nextDescriptor->getFormatterRegion()->isVisible() &&
					nextDescriptor->getFormatterRegion()->setSelection(true)) {

				//recovering old selected object
				if (selectedObject != NULL && selectedObject != nextObject) {
					selectedObject->setHandler(false);
					selectedObject->getDescriptor()->
							getFormatterRegion()->setSelection(false);

					recoveryDefaultState(selectedObject);
				}

				//selecting new object
				selectedObject = nextObject;
				selectedObject->setHandler(true);
				player = playerManager->getPlayer(selectedObject);

				enterSelection(player);
				nextObject->selectionEvent(
						CodeMap::KEY_NULL, player->getMediaTime());

			} else {
				cout << "FormatterFocusManager::setKeyMaster can't ";
				cout << " select '" << currentFocus << "'" << endl;
				if (isId) {
					objectToSelect = focusIndex;
				}
			}

		} else {
			cout << "FormatterFocusManager::setKeyMaster can't select '";
			cout << focusIndex << "' region == NULL" << endl;
			if (isId) {
				objectToSelect = focusIndex;
			}
		}

		pthread_mutex_unlock(&mutexFocus);
		multiDevice->updatePassiveDevices();
	}

	void FormatterFocusManager::setFocus(string focusIndex) {
		ExecutionObject* nextObject = NULL;
		ExecutionObject* currentObject = NULL;
		CascadingDescriptor* currentDescriptor = NULL;
		CascadingDescriptor* nextDescriptor = NULL;

		if (focusIndex == currentFocus && currentFocus != "") {
			cout << "FormatterFocusManager::setFocus unnecessary currentFocus";
			cout << " = '" << currentFocus << "' focusIndex = '" << focusIndex;
			cout << "'" << endl;
			return;
		}

		pthread_mutex_lock(&mutexTable);
		pthread_mutex_lock(&mutexFocus);

		nextObject = getObjectFromFocusIndex(focusIndex);
		if (nextObject == NULL) {

			cout << "FormatterFocusManager::setFocus Warning! index '";
			cout << focusIndex << "' is not visible." << endl;

			pthread_mutex_unlock(&mutexFocus);
			pthread_mutex_unlock(&mutexTable);
			multiDevice->updatePassiveDevices();
			return;
		}

		currentObject = getObjectFromFocusIndex(currentFocus);
		if (currentObject != NULL) {
			pthread_mutex_unlock(&mutexTable);
			currentDescriptor = currentObject->getDescriptor();

		} else {
			currentDescriptor = NULL;
			cout << "FormatterFocusManager::setFocus index '";
			cout << focusIndex << "' is not in focus Table." << endl;
			pthread_mutex_unlock(&mutexTable);
		}

		currentFocus = focusIndex;
		PresentationContext::getInstance()->setPropertyValue(
				"service.currentFocus", currentFocus);

		if (currentDescriptor != NULL) {
			recoveryDefaultState(currentObject);
			currentDescriptor->getFormatterRegion()->setFocus(false);

		} else if (currentObject != NULL) {
			cout << "FormatterFocusManager::setFocus Warning can't recover '";
			cout << currentObject->getId() << "'" << endl;
		}

		nextDescriptor = nextObject->getDescriptor();
		if (nextDescriptor != NULL) {
			setFocus(nextDescriptor);
		}

		pthread_mutex_unlock(&mutexFocus);
		multiDevice->updatePassiveDevices();
	}

	void FormatterFocusManager::setFocus(CascadingDescriptor* descriptor) {
		double borderAlpha;
		Color* focusColor = NULL;
		Color* selColor = NULL;
		int borderWidth = -3;
		int width;
		FormatterRegion* region = NULL;

		if (descriptor != NULL) {
			focusColor = descriptor->getFocusBorderColor();
			borderWidth = descriptor->getFocusBorderWidth();
			selColor = descriptor->getSelBorderColor();
			region = descriptor->getFormatterRegion();
		}

		if (focusColor == NULL) {
			focusColor = defaultFocusBorderColor;
		}

		borderAlpha = descriptor->getFocusBorderTransparency();
		if (!isNaN(borderAlpha)) {
			focusColor = new Color(focusColor->getR(), focusColor->getG(),
					focusColor->getB(), (int)(borderAlpha * 255));
		}

		width = borderWidth;

		if (selColor == NULL) {
			selColor = defaultSelBorderColor;
		}

		if (!isNaN(borderAlpha)) {
			selColor = new Color(selColor->getR(), selColor->getG(),
					selColor->getB(), (int)(borderAlpha * 255));
		}

		if (region != NULL) {
			region->setFocusInfo(focusColor, width,
					descriptor->getFocusSrc(), selColor, width,
					descriptor->getSelectionSrc());

			region->setFocus(true);
		}

		if (focusColor != NULL && focusColor != defaultFocusBorderColor) {
			if (descriptor == NULL || (descriptor != NULL &&
					descriptor->getFocusBorderColor() != focusColor)) {

				delete focusColor;
				focusColor = NULL;
			}
		}

		if (selColor != NULL && selColor != defaultSelBorderColor) {
			if (descriptor == NULL || (descriptor != NULL &&
					descriptor->getSelBorderColor() != selColor)) {

				delete selColor;
				selColor = NULL;
			}
		}
	}

/*
	void FormatterFocusManager::recoveryDefaultState(ExecutionObject* object) {
		if (object == NULL || object->getDescriptor() == NULL ||
				object->getDescriptor()->getFormatterRegion() == NULL) {

			multiDevice->updatePassiveDevices();
			return;
		}

		FormatterPlayerAdapter* player;
		player = playerManager->getPlayer(object);
		if (player != NULL) {
			ISurface* renderedSurface;
			renderedSurface = player->getObjectDisplay();
			if (renderedSurface != NULL) {
				object->getDescriptor()->getFormatterRegion()->
						renderSurface(renderedSurface);

				player->flip();

			} else {
				cout << "FormatterFocusManager::recoveryDefaultState ";
				cout << "Warning! null renderedsurface for '";
				cout << object->getId() << "'" << endl;
			}
		}
		multiDevice->updatePassiveDevices();
	}
*/

   void FormatterFocusManager::recoveryDefaultState(ExecutionObject* object) {
		if (object == NULL || object->getDescriptor() == NULL ||
				object->getDescriptor()->getFormatterRegion() == NULL) {

			multiDevice->updatePassiveDevices();
			return;
		}

		FormatterPlayerAdapter* player;
		player = playerManager->getPlayer(object);
		if (player != NULL) {
			IPlayer* media_player = player->getPlayer();
			media_player->refreshContent();
		}
		multiDevice->updatePassiveDevices();
	}

	void FormatterFocusManager::showObject(ExecutionObject* object) {
		CascadingDescriptor* descriptor;
		string focusIndex, auxIndex;
		string paramValue;
		FormatterPlayerAdapter* player;
		vector<string>::iterator i;
		bool isId = false;

		if (object == NULL) {
			cout << "FormatterFocusManager::showObject Warning! object ";
			cout << "is null." << endl;
			multiDevice->updatePassiveDevices();
			return;
		}

		descriptor = object->getDescriptor();
		if (descriptor == NULL) {
			cout << "FormatterFocusManager::showObject Warning! ";
			cout << " descriptor is null." << endl;
			multiDevice->updatePassiveDevices();
			return;
		}

		focusIndex = descriptor->getFocusIndex();
		if (focusIndex == "") {
			isId = true;
			focusIndex = object->getDataObject()->getDataEntity()->getId();

		} else {
			pthread_mutex_lock(&mutexTable);
			insertObject(object, focusIndex);
			pthread_mutex_unlock(&mutexTable);
		}

		if (currentFocus == "") {
			registerNavigationKeys();

			paramValue = PresentationContext::getInstance()->getPropertyValue(
					"service.currentKeyMaster");

			if (paramValue == focusIndex || focusIndex == objectToSelect) {
				objectToSelect = "";
				setKeyMaster(focusIndex);

			} else if (!isId) {
				setFocus(focusIndex);
			}

		} else {
			paramValue = PresentationContext::getInstance()->
					getPropertyValue("service.currentFocus");

			if (paramValue != "" &&
				    paramValue == focusIndex &&
				    !isId &&
				    descriptor->getFormatterRegion()->isVisible()) {

				/*if (focusTable->count(currentFocus) != 0) {
					currentObject = (*focusTable)[currentFocus];
					currentObject->getDescriptor()->
						    getFormatterRegion()->setFocus(false);

					recoveryDefaultState(currentObject);
				}

			 	currentFocus = focusIndex;*/
			 	setFocus(focusIndex);
			}

			paramValue = PresentationContext::getInstance()->getPropertyValue(
					"service.currentKeyMaster");

			if ((paramValue == focusIndex || focusIndex == objectToSelect) &&
					descriptor->getFormatterRegion()->isVisible()) {

				// first set as current focus
				/*if (focusTable->count(currentFocus) != 0) {
					currentObject = (*focusTable)[currentFocus];
					currentObject->getDescriptor()->
						    getFormatterRegion()->setFocus(false);

					recoveryDefaultState(currentObject);
				}

				currentFocus = focusIndex;
				PresentationContext::getInstance()->setPropertyValue(
						"service.currentFocus", currentFocus);*/

				objectToSelect = "";

				if (!isId) {
					setFocus(focusIndex);
				}

				// then set as selected
				if (descriptor->getFormatterRegion()->setSelection(true)) {
					// unselect the previous selected object, if exists
					if (selectedObject != NULL) {
						selectedObject->setHandler(false);
						selectedObject->getDescriptor()->
							    getFormatterRegion()->setSelection(false);

						recoveryDefaultState(selectedObject);
					}

					selectedObject = object;
					selectedObject->setHandler(true);
					player = playerManager->getPlayer(selectedObject);
					enterSelection(player);
				}
			}
    	}
		multiDevice->updatePassiveDevices();
	}

	void FormatterFocusManager::hideObject(ExecutionObject* object) {
		string focusIndex;
		FormatterRegion* region;
		FormatterPlayerAdapter* player;
		map<string, set<ExecutionObject*>*>::iterator i;

		if (object == NULL || object->getDescriptor() == NULL ||
				object->getDescriptor()->getFormatterRegion() == NULL) {

			multiDevice->updatePassiveDevices();
			return;
		}

		region = object->getDescriptor()->getFormatterRegion();
		focusIndex = object->getDescriptor()->getFocusIndex();
		if (focusIndex != "") {
			pthread_mutex_lock(&mutexTable);
			removeObject(object, focusIndex);
			pthread_mutex_unlock(&mutexTable);

			if (region->getFocusState() == FormatterRegion::SELECTED &&
					selectedObject == object) {

				player = playerManager->getPlayer(selectedObject);
				exitSelection(player);
				//region->setSelection(false);
				//recoveryDefaultState(selectedObject);
				selectedObject = NULL;
			}

			if (currentFocus == focusIndex) {
				//region->setFocus(false);
				//recoveryDefaultState(object);

				pthread_mutex_lock(&mutexTable);
				if (focusSequence->empty()) {
					pthread_mutex_unlock(&mutexTable);
					currentFocus = "";
					unregister();

				} else {
					string ix = *(focusSequence->begin());
					i = focusTable->find(ix);
					while (i == focusTable->end()) {
						focusSequence->erase(focusSequence->begin());
						if (focusSequence->empty()) {
							pthread_mutex_unlock(&mutexTable);
							currentFocus = "";
							unregister();
							return;
						}
						ix = *(focusSequence->begin());
						i = focusTable->find(ix);
					}
					pthread_mutex_unlock(&mutexTable);
					setFocus(ix);
				}
			}
		}
		multiDevice->updatePassiveDevices();
	}

	bool FormatterFocusManager::keyCodeOk(ExecutionObject* currentObject) {
		FormatterPlayerAdapter* player;
		bool isHandling = false;
/*
		cout << "FormatterFocusManager::keyCodeOk(" << this << ")";
		cout << " this->isHandler '" << this->isHandler << "'";
		cout << " parentManager '" << parentManager << "'";
		cout << " lastHandler '" << lastHandler << "'";
		cout << endl;
*/
		if (parentManager != NULL) {
			parentManager->unregister();
		}

		if (currentObject != selectedObject) {
			if (selectedObject != NULL) {
				cout << "FormatterFocusManager::keyCodeOk Warning! ";
				cout << "selecting an object with another selected." << endl;
				selectedObject->setHandler(false);
			}
		}

		selectedObject = currentObject;
		selectedObject->setHandler(true);
		player = playerManager->getPlayer(selectedObject);

		changeSettingState("service.currentKeyMaster", "start");
		isHandling = enterSelection(player);

		if (selectedObject != NULL) {
			selectedObject->selectionEvent(
					CodeMap::KEY_NULL, player->getMediaTime());
		}

		changeSettingState("service.currentKeyMaster", "stop");

		multiDevice->updatePassiveDevices();
		return isHandling;
	}

	bool FormatterFocusManager::keyCodeBack() {
		CascadingDescriptor* selectedDescriptor;
		FormatterRegion* fr;
		FormatterPlayerAdapter* player;
		string ix;
/*
		cout << "FormatterFocusManager::keyCodeBack(" << this << ")";
		cout << " this->isHandler '" << this->isHandler << "'";
		cout << " parentManager '" << parentManager << "'";
		cout << " lastHandler '" << lastHandler << "'";
		cout << endl;
*/
		if (parentManager != NULL) {
			parentManager->registerBackKeys();
		}

		if (selectedObject == NULL) {
			multiDevice->updatePassiveDevices();
			cout << "FormatterFocusManager::keyCodeBack NULL selObject";
			cout << endl;
			return false;
		}

		selectedObject->setHandler(false);
		selectedDescriptor = selectedObject->getDescriptor();
		if (selectedDescriptor == NULL) {
			multiDevice->updatePassiveDevices();
			cout << "FormatterFocusManager::keyCodeBack NULL selDescriptor";
			cout << endl;
			return false;
		}

		ix = selectedDescriptor->getFocusIndex();
		fr = selectedDescriptor->getFormatterRegion();
		if (fr == NULL) {
			multiDevice->updatePassiveDevices();
			cout << "FormatterFocusManager::keyCodeBack NULL formatterRegion";
			cout << endl;
			return false;
		}

		fr->setSelection(false);

		pthread_mutex_lock(&mutexFocus);
		if (ix == currentFocus) {
			fr->setFocus(true);
		}

		player = playerManager->getPlayer(selectedObject);
		changeSettingState("service.currentKeyMaster", "start");
		exitSelection(player);
		changeSettingState("service.currentKeyMaster", "stop");
		selectedObject = NULL;
		pthread_mutex_unlock(&mutexFocus);

		multiDevice->updatePassiveDevices();
		return false;
	}

	bool FormatterFocusManager::enterSelection(FormatterPlayerAdapter* player) {
		bool newHandler = false;
		string keyMaster;

		registerBackKeys();

		if (player != NULL && selectedObject != NULL) {
			keyMaster = (selectedObject->getDataObject()->getDataEntity()->
					getId());

			PresentationContext::getInstance()->setPropertyValue(
					"service.currentKeyMaster", keyMaster);

			newHandler = player->setKeyHandler(true);
			multiDevice->updatePassiveDevices();
		}

		return newHandler;
	}

	void FormatterFocusManager::exitSelection(FormatterPlayerAdapter* player) {
		unregister();

		if (player != NULL) {
			player->setKeyHandler(false);

			PresentationContext::getInstance()->setPropertyValue(
					"service.currentKeyMaster", "");

		}

		registerNavigationKeys();

		if (player != NULL) {
			multiDevice->updatePassiveDevices();
		}
	}

	void FormatterFocusManager::registerNavigationKeys() {
		set<int>* evs;

		evs = new set<int>;
		evs->insert(CodeMap::KEY_CURSOR_DOWN);
		evs->insert(CodeMap::KEY_CURSOR_LEFT);
		evs->insert(CodeMap::KEY_CURSOR_RIGHT);
		evs->insert(CodeMap::KEY_CURSOR_UP);

		evs->insert(CodeMap::KEY_ENTER);
		evs->insert(CodeMap::KEY_OK);

		evs->insert(CodeMap::KEY_MENU);
		evs->insert(CodeMap::KEY_INFO);

		evs->insert(CodeMap::KEY_RED);
		evs->insert(CodeMap::KEY_GREEN);
		evs->insert(CodeMap::KEY_YELLOW);
		evs->insert(CodeMap::KEY_BLUE);

		if (im != NULL) {
			im->addInputEventListener(this, evs);
		}
		else{
		  evs->clear();
		  delete evs;
		}
	}

	void FormatterFocusManager::registerBackKeys() {
		set<int>* evs;

		evs = new set<int>;
		evs->insert(CodeMap::KEY_BACKSPACE);
		evs->insert(CodeMap::KEY_BACK);
		if (im != NULL) {
			im->addInputEventListener(this, evs);
		}
	}

	void FormatterFocusManager::unregister() {
		if (im != NULL) {
			im->removeInputEventListener(this);
		}
	}

	void FormatterFocusManager::setDefaultFocusBorderColor(Color* color) {
		if (defaultFocusBorderColor != NULL) {
			delete defaultFocusBorderColor;
		}
		defaultFocusBorderColor = color;
	}

	void FormatterFocusManager::setDefaultFocusBorderWidth(int width) {
		defaultFocusBorderWidth = width;
	}

	void FormatterFocusManager::setDefaultSelBorderColor(Color* color) {
		if (defaultSelBorderColor != NULL) {
			delete defaultSelBorderColor;
		}
		defaultSelBorderColor = color;
	}

	void FormatterFocusManager::changeSettingState(string name, string act) {
		set<ExecutionObject*>* settingObjects;
		set<ExecutionObject*>::iterator i;
		vector<FormatterEvent*>* events;
		vector<FormatterEvent*>::iterator j;
		PropertyAnchor* property;
		string pName, keyM;

		settingObjects = ((FormatterConverter*)
				converter)->getSettingNodeObjects();

		/*cout << "FormatterFocusManager::changeSettingState number of ";
		cout << "settings objects: '" << settingObjects->size() << "'";
		cout << endl;*/

		i = settingObjects->begin();
		while (i != settingObjects->end()) {
			events = (*i)->getEvents();
			if (events != NULL) {
				j = events->begin();
				while (j != events->end()) {
					if ((*j)->instanceOf("AttributionEvent")) {
						property = ((AttributionEvent*)(*j))->getAnchor();
						pName = property->getPropertyName();
						if (pName == name) {
							if (act == "start") {
								(*j)->start();

							} else if (act == "stop") {
								if (pName == "service.currentFocus") {
									((AttributionEvent*)(*j))->setValue(
											currentFocus);

								} else if (pName ==
										"service.currentKeyMaster") {

									if (selectedObject != NULL) {
										keyM = (selectedObject->getDataObject()
												->getDataEntity()->getId());

										((AttributionEvent*)(*j))->setValue(
												keyM);
									}
								}

								(*j)->stop();
							}
						}
					}
					++j;
				}
				delete events;
			}
			++i;
		}

		delete settingObjects;
		multiDevice->updatePassiveDevices();
	}

	bool FormatterFocusManager::userEventReceived(IInputEvent* userEvent) {
		ExecutionObject* currentObject;
		CascadingDescriptor* currentDescriptor;
		FormatterRegion* fr;
		string nextIndex;
		map<string, set<ExecutionObject*>*>::iterator i;

		const int code = userEvent->getKeyCode();

		if (!isHandler) {
			return true;
		}

		pthread_mutex_lock(&mutexTable);
		i = focusTable->find(currentFocus);
		if (i == focusTable->end()) {
			if (currentFocus != "") {
				cout << "FormatterFocusManager::userEventReceived ";
				cout << "currentFocus not found which is '" << currentFocus;
				cout << "'" << endl;
			}

			if (!focusSequence->empty()) {
				nextIndex = (*focusSequence)[0];
				pthread_mutex_unlock(&mutexTable);
				setFocus(nextIndex);

			} else {
				pthread_mutex_unlock(&mutexTable);
			}
			multiDevice->updatePassiveDevices();
			return true;
		}

		currentObject = getObjectFromFocusIndex(currentFocus);
		if (currentObject == NULL) {
			cout << "FormatterFocusManager::userEventReceived ";
			cout << "Warning! object == NULL" << endl;
			cout << "'" << endl;

			pthread_mutex_unlock(&mutexTable);
			multiDevice->updatePassiveDevices();
			return true;
		}
		pthread_mutex_unlock(&mutexTable);

		currentDescriptor = currentObject->getDescriptor();
		if (currentDescriptor == NULL) {
			cout << "FormatterFocusManager::userEventReceived ";
			cout << "Warning! descriptor == NULL" << endl;
			cout << "'" << endl;
			multiDevice->updatePassiveDevices();
			return true;
		}

		nextIndex = "";
		if (selectedObject != NULL) {
			if (code == CodeMap::KEY_BACKSPACE || code == CodeMap::KEY_BACK) {
				multiDevice->updatePassiveDevices();
				return keyCodeBack();
			}

		} else if (code == CodeMap::KEY_CURSOR_UP) {
			nextIndex = currentDescriptor->getMoveUp();

		} else if (code == CodeMap::KEY_CURSOR_DOWN) {
			nextIndex = currentDescriptor->getMoveDown();

		} else if (code == CodeMap::KEY_CURSOR_LEFT) {
			nextIndex = currentDescriptor->getMoveLeft();

		} else if (code == CodeMap::KEY_CURSOR_RIGHT) {
			nextIndex = currentDescriptor->getMoveRight();

		} else if (code == CodeMap::KEY_ENTER || code == CodeMap::KEY_OK) {
			userEvent->setKeyCode(CodeMap::KEY_NULL);
			fr = currentDescriptor->getFormatterRegion();
			if (fr != NULL && fr->isVisible() && fr->setSelection(true)) {
				if (!keyCodeOk(currentObject)) {
					if (im != NULL) {
						im->postEvent(CodeMap::KEY_BACK);
					}
				}

			} else {
				cout << "FormatterFocusManager::userEventReceived cant ";
				cout << " select '" << currentFocus << "'" << endl;
			}

			multiDevice->updatePassiveDevices();
			return false;
		}

		if (nextIndex != "") {
			changeSettingState("service.currentFocus", "start");
			setFocus(nextIndex);
			changeSettingState("service.currentFocus", "stop");
		}

		multiDevice->updatePassiveDevices();
		return true;
	}
}
}
}
}
}
}
