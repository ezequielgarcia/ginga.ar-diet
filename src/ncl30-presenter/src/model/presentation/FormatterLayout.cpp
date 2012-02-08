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

#include "../../../include/model/FormatterLayout.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace presentation {
	FormatterLayout::FormatterLayout(int x, int y, int w, int h) {
		regionMap     = new map<string, set<FormatterRegion*>*>;
		regionZIndex  = new map<string, int>;
		sortedRegions = new vector<string>;
		objectMap     = new map<FormatterRegion*, ExecutionObject*>;
		deviceRegion  = NULL;
		childs        = NULL;

		typeSet.insert("FormatterLayout");
		pthread_mutex_init(&mutex, NULL);
		createDeviceRegion(x, y, w, h);
	}

	FormatterLayout::~FormatterLayout() {
		clear();
		lock();
		if (regionMap != NULL) {
			delete regionMap;
			regionMap = NULL;
		}

		if (regionZIndex != NULL) {
			delete regionZIndex;
			regionZIndex = NULL;
		}

		if (objectMap != NULL) {
			delete objectMap;
			objectMap = NULL;
		}

		if (childs != NULL) {
			delete childs;
			childs = NULL;
		}
		unlock();
	}

	void FormatterLayout::addChild(string objectId, FormatterLayout* child) {
		if (childs == NULL) {
			childs = new map<string, FormatterLayout*>;
		}

		(*childs)[objectId] = child;
	}

	FormatterLayout* FormatterLayout::getChild(string objectId) {
		map<string, FormatterLayout*>::iterator i;

		if (childs == NULL) {
			return NULL;
		}

		i = childs->find(objectId);
		if (i != childs->end()) {
			return i->second;
		}

		return NULL;
	}

	string FormatterLayout::getBitMapRegionId() {
		if (deviceRegion != NULL) {
			return deviceRegion->getOutputMapRegionId();
		}
		return "";
	}

	LayoutRegion* FormatterLayout::getNcmRegion(string regionId) {
		if (deviceRegion != NULL) {
			return deviceRegion->getRegionRecursively(regionId);
		}
		return NULL;
	}

	void FormatterLayout::printRegionMap() {
		map<string, set<FormatterRegion*>*>::iterator i;

		cout << "FormatterLayout::printRegionMap: | '";

		i = regionMap->begin();
		while (i != regionMap->end()) {
			cout << i->first << "' ";
			++i;
		}
		cout << "|" << endl;
	}

	void FormatterLayout::printObjectMap() {
		map<FormatterRegion*, ExecutionObject*>::iterator i;

		cout << "FormatterLayout::printObjectMap: | '";

		i = objectMap->begin();
		while (i != objectMap->end()) {
			cout << i->second->getId() << "' ";
			++i;
		}
		cout << "|" << endl;
	}

	void FormatterLayout::createDeviceRegion(int x, int y, int w, int h) {
		deviceRegion = new FormatterDeviceRegion("defaultScreenFormatter");
		deviceRegion->setLeft(x, false);
		deviceRegion->setTop(y, false);
		deviceRegion->setWidth(w, false);
		deviceRegion->setHeight(h, false);
	}

	ExecutionObject* FormatterLayout::getObject(int x, int y) {
		map<string, set<FormatterRegion*>*>::iterator i;

		FormatterRegion* formRegion = NULL;
		set<FormatterRegion*>* formRegions = NULL;
		set<FormatterRegion*>::iterator j;

		map<FormatterRegion*, ExecutionObject*>::iterator k;

		LayoutRegion* currentRegion = NULL;
		LayoutRegion* region        = NULL;
		ExecutionObject* object     = NULL;

		lock();
		i = regionMap->begin();
		while (i != regionMap->end()) {
			formRegions = i->second;
			j = formRegions->begin();
			while (j != formRegions->end()) {
				formRegion = *j;
				k = objectMap->find(formRegion);
				if (formRegion->intersects(x, y) && k != objectMap->end()) {
					if (object == NULL) {
						region = k->first->getLayoutRegion();
						object = k->second;

					} else {
						currentRegion = formRegion->getLayoutRegion();
						if (currentRegion != NULL && region != NULL) {
							if (currentRegion->getZIndex() >
									region->getZIndex()) {

								region = currentRegion;
								object = k->second;
							}
						}
					}
				}
				++j;
			}
			++i;
		}
		unlock();
		return object;
	}

	bool FormatterLayout::getScreenShot(IWindow* region) {
		map<string, set<FormatterRegion*>*>::iterator i;
		set<FormatterRegion*>::iterator j;
		vector<string>::iterator k;
		map<FormatterRegion*, ExecutionObject*>::iterator l;
		map<string, FormatterLayout*>::iterator m;

		set<FormatterRegion*>* formRegions = NULL;
		FormatterRegion* formatterRegion;
		IWindow* formWindow;

		lock();
		k = sortedRegions->begin();
		while (k != sortedRegions->end()) {
			i = regionMap->find(*k);
			if (i != regionMap->end()) {
				formRegions = i->second;
				j = formRegions->begin();
				while (j != formRegions->end()) {
					formatterRegion = (*j);
					formWindow = formatterRegion->getODContentPane();
					if (childs != NULL) {
						l = objectMap->find(formatterRegion);
						if (l != objectMap->end()) {
							m = childs->find(l->second->getId());
							if (m != childs->end()) {
								m->second->getScreenShot(formWindow);
								formWindow->validate();
								unlock();
								return false;
							}
						}
					}
					region->blit(formWindow);
					++j;
				}
			}
			++k;
		}

		unlock();
		return true;
	}

	void FormatterLayout::prepareFormatterRegion(
			ExecutionObject* object, ISurface* renderedSurface) {

		CascadingDescriptor* descriptor;
		FormatterRegion* region;

		LayoutRegion *layoutRegion, *parent, *grandParent;
		string regionId, mapId;

		vector<LayoutRegion*>* childs;
		vector<LayoutRegion*>::iterator i;
		set<FormatterRegion*>* formRegions;
		int devClass, z;

		if (object == NULL || object->getDescriptor() == NULL ||
			    object->getDescriptor()->getFormatterRegion() == NULL) {

			/*cout << "FormatterLayout::prepareFormatterRegion Warning! Can't";
			cout << " prepare FormatterRegion for '";
			if (object != NULL) {
				cout << object->getId();

			} else {
				cout << "NULL";
			}

			cout << "' object" << endl;*/
			return;
		}

		descriptor = object->getDescriptor();
		region = descriptor->getFormatterRegion();
		layoutRegion = region->getOriginalRegion();

		//every presented object has a region root
		//the formatter device region
		parent = layoutRegion;
		grandParent = layoutRegion->getParent();
		if (grandParent != NULL) {
			while (grandParent->getParent() != NULL) {
				parent = grandParent;
				grandParent = grandParent->getParent();
			}
		}

		if (grandParent != deviceRegion && grandParent != NULL) {
			childs = grandParent->getRegions();
			i = childs->begin();
			/*cout << endl << endl;
			cout << "FormatterLayout::prepareFormatterRegion grandParent '";
			cout << grandParent->getId() << "' regions: ";*/
			while (i != childs->end()) {
				/*cout << "'" << (*i)->getId() << "'(" << *i << ") ";*/
				deviceRegion->addRegion(*i);
				(*i)->setParent(deviceRegion);
				++i;
			}
			//cout << endl << endl;

			devClass = grandParent->getDeviceClass();
			if (devClass >= 0) {
				mapId = grandParent->getOutputMapRegionId();
				deviceRegion->setDeviceClass(devClass, mapId);

				/*cout << endl << endl;
				cout << "FormatterLayout::prepareFormatterRegion deviceRegion";
				cout << " id = '" << deviceRegion->getId();
				cout << "' bitmap = '" << deviceRegion->getOutputMapRegionId();
				cout << "' with class = ";
				cout << "'" << deviceRegion->getDeviceClass();
				cout << "' grandParent ";
				cout << " id = '" << grandParent->getId();
				cout << "' bitmap = '" << grandParent->getOutputMapRegionId();
				cout << "' with class = ";
				cout << "'" << grandParent->getDeviceClass();
				cout << "' parent ";
				cout << " id = '" << parent->getId();
				cout << "' bitmap = '" << parent->getOutputMapRegionId();
				cout << "' with class = ";
				cout << "'" << parent->getDeviceClass() << "'";
				cout << endl << endl;*/
			}

			deviceRegion->addRegion(parent);
			parent->setParent(deviceRegion);

		} else if (grandParent == NULL) {
			cout << endl << endl;
			cout << "FormatterLayout::prepareFormatterRegion parent is ";
			cout << "NULL for layout '" << layoutRegion->getId() << "'(";
			cout << layoutRegion << ")";
			cout << endl;
			cout << endl << endl;
		}

		regionId = layoutRegion->getId();

		/*cout << endl << endl;
		cout << "FormatterLayout::prepareFormatterRegion call prepareOD";
		cout << " for '" << object->getId() << "'" << endl;
		cout << endl << endl;*/
		lock();
		if (regionMap->count(regionId)==0) {
			formRegions = new set<FormatterRegion*>;
			(*regionMap)[regionId] = formRegions;
			z = layoutRegion->getZIndex();
			(*regionZIndex)[regionId] = z;
			sortRegion(regionId, z);

		} else {
			formRegions = (*regionMap)[regionId];
		}
		formRegions->insert(region);
		region->prepareOutputDisplay(renderedSurface);

		(*objectMap)[region] = object;
		unlock();
	}

	void FormatterLayout::sortRegion(string regionId, int zIndex) {
		vector<string>::iterator i;
		map<string, int>::iterator j;

		i = sortedRegions->begin();
		while (i != sortedRegions->end()) {
			j = regionZIndex->find(*i);
			if (j != regionZIndex->end()) {
				if (zIndex <= j->second) {
					break;
				}
			}
			++i;
		}

		sortedRegions->insert(i, regionId);
	}

	void FormatterLayout::showObject(ExecutionObject* object) {
		FormatterRegion* region;

		if (object == NULL || object->getDescriptor() == NULL ||
			    object->getDescriptor()->getFormatterRegion() == NULL) {

			/*cout << "FormatterLayout::showObject can't find region for '";
			if (object != NULL) {
				cout << object->getId();

			} else {
				cout << "NULL";
			}
			cout << "'" << endl;*/
			return;
		}

		/*cout << "FormatterLayout::showObject call showContent for '";
		cout << object->getId() << "' devW = '";
		cout << deviceRegion->getWidthInPixels() << "' devH = '";
		cout << deviceRegion->getHeightInPixels() << "'" << endl;*/
		region = object->getDescriptor()->getFormatterRegion();
		region->showContent();
	}

	void FormatterLayout::hideObject(ExecutionObject* object) {
		FormatterRegion* region = NULL;
		LayoutRegion* layoutRegion;
		string regionId;

		set<FormatterRegion*>* formRegions;
		set<FormatterRegion*>::iterator i;
		map<string, set<FormatterRegion*>*>::iterator j;
		map<FormatterRegion*, ExecutionObject*>::iterator k;
		map<string, int>::iterator l;
		vector<string>::iterator m;

		if (object != NULL && object->getDescriptor() != NULL) {
			region = object->getDescriptor()->getFormatterRegion();
		}

		if (region == NULL) {
			return;
		}

		region = object->getDescriptor()->getFormatterRegion();
		region->hideContent();
		layoutRegion = region->getLayoutRegion();
		regionId = layoutRegion->getId();
		lock();
		if (regionMap != NULL && regionMap->count(regionId) != 0) {
			formRegions = (*regionMap)[regionId];
			i = formRegions->find(region);
			if (i != formRegions->end()) {
				formRegions->erase(i);
				if (formRegions->empty()) {
					j = regionMap->find(regionId);
					if (j != regionMap->end()) {
						regionMap->erase(j);
						delete formRegions;

						l = regionZIndex->find(regionId);
						if (l != regionZIndex->end()) {
							regionZIndex->erase(l);
						}

						m = sortedRegions->begin();
						while (m != sortedRegions->end()) {
							if (regionId == *m) {
								sortedRegions->erase(m);
								break;
							}
							++m;
						}
					}
				}

				k = objectMap->find(region);
				if (k != objectMap->end()) {
					objectMap->erase(k);
				}
			}
		}
		unlock();

		/*map<string, vector<FormatterRegion*>*>::iterator i;
		i=regionMap->begin();
		while(i!=regionMap->end()) {
			if (i->first == regionId) {
				formRegions = i->second;

				if (formRegions->size() == 1) {
					formRegions->clear();

				} else {
					vector<FormatterRegion*>::iterator j;
					j=formRegions->begin();
					while (j!=formRegions->end()) {
						if (*j == region) {
							formRegions->erase(j);
							j = formRegions->begin();

						} else {
							++j;
						}
					}
				}

				regionMap->erase(i);
				i = regionMap->begin();

			} else {
				++i;
			}
		}*/
	}

	set<FormatterRegion*>* FormatterLayout::
		    getFormatterRegionsFromNcmRegion(string regionId) {

		set<FormatterRegion*>* regions;

		lock();
		if (regionMap->count(regionId) == 0) {
			unlock();
			return NULL;
		}

		regions = new set<FormatterRegion*>(*((*regionMap)[regionId]));
		unlock();
		return regions;
	}

	void FormatterLayout::clear() {
		map<string, set<FormatterRegion*>*>::iterator i;
		set<FormatterRegion*>* formRegions;

		lock();
		for (i=regionMap->begin(); i!=regionMap->end(); ++i) {
			formRegions = i->second;
			formRegions->clear();
			delete formRegions;
		}

		regionMap->clear();
		regionZIndex->clear();
		objectMap->clear();
		sortedRegions->clear();
		unlock();
		deviceRegion->removeRegions();
	}

	void FormatterLayout::lock() {
		pthread_mutex_lock(&mutex);
	}

	void FormatterLayout::unlock() {
		pthread_mutex_unlock(&mutex);
	}
}
}
}
}
}
}
}
