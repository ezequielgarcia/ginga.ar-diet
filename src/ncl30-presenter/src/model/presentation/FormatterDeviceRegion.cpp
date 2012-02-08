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

#include "../../../include/model/FormatterDeviceRegion.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace presentation {
	FormatterDeviceRegion::FormatterDeviceRegion(
		    string id) : LayoutRegion(id) {

		this->id = id;
		regions = new vector<LayoutRegion*>;
		typeSet.insert("FormatterDeviceRegion");
	}

	void FormatterDeviceRegion::addRegion(LayoutRegion *region) {
		vector<LayoutRegion*>::iterator i;
		int zIndex;

		if (regionSet.count(region) == 0) {
			regionSet.insert(region);
			zIndex = region->getZIndexValue();

			i = regions->begin();
			while (i != regions->end()) {
				if (zIndex <= (*i)->getZIndexValue()) {
					break;
				}
				++i;
			}

			regions->insert(i, region);
			region->setParent(this);
		}
	}

	LayoutRegion *FormatterDeviceRegion::cloneRegion() {
		LayoutRegion *cloneRegion;
		vector<LayoutRegion*> *childRegions;

		cloneRegion = new FormatterDeviceRegion(id);

		cloneRegion->setTitle(getTitle());
		cloneRegion->setLeft(left, false);
		cloneRegion->setTop(top, false);
		cloneRegion->setWidth(width, false);
		cloneRegion->setHeight(height, false);
		cloneRegion->setDecorated(false);
		cloneRegion->setMovable(false);
		cloneRegion->setResizable(false);

		childRegions = getRegions();
		vector<LayoutRegion*>::iterator it;
		for (it = childRegions->begin(); it != childRegions->end(); ++it) {
			cloneRegion->addRegion(*it);
		}

		return cloneRegion;
	}

	int FormatterDeviceRegion::compareWidthSize(string w) {
		int newW;

		newW = atoi(w.c_str());
		if (newW == width) {
			return 0;

		} else if (newW > width) {
			return 1;

		} else {
			return -1;
		}
	}

	int FormatterDeviceRegion::compareHeightSize(string h) {
		int newH;

		newH = atoi(h.c_str());
		if (newH == height) {
			return 0;

		} else if (newH > height) {
			return 1;

		} else {
			return -1;
		}
	}

	short FormatterDeviceRegion::getBackgroundColor() {
		return -1;
	}

	double FormatterDeviceRegion::getBottom() {
		return NaN();
	}

	double FormatterDeviceRegion::getHeight() {
		return height;
	}

	double FormatterDeviceRegion::getLeft() {
		return left;
	}

	double FormatterDeviceRegion::getRight() {
		return NaN();
	}

	LayoutRegion *FormatterDeviceRegion::getRegion(string id) {
		int i, size;
		LayoutRegion *region;

		size = regions->size();
		for (i = 0; i < size; i++) {
			region = (LayoutRegion*)((*regions)[i]);
			if (region->getId() == id) {
				return region;
			}
		}
		return NULL;
	}

	LayoutRegion *FormatterDeviceRegion::getRegionRecursively(string id) {
		int i, size;
		LayoutRegion *region, *auxRegion;

		size = regions->size();
		for (i = 0; i < size; i++) {
			region = (LayoutRegion*)((*regions)[i]);
			if (region->getId() == id) {
				return region;
			}
			auxRegion = region->getRegionRecursively(id);
			if (auxRegion != NULL) {
				return auxRegion;
			}
		}
		return NULL;
	}

	vector<LayoutRegion*> *FormatterDeviceRegion::getRegions() {
		return regions;
	}

	string FormatterDeviceRegion::getTitle() {
		return "";
	}

	double FormatterDeviceRegion::getTop() {
		return top;
	}

	double FormatterDeviceRegion::getWidth() {
		return width;
	}

	int FormatterDeviceRegion::getZIndex() {
		return -1;
	}

	int FormatterDeviceRegion::getZIndexValue() {
		int zIndex;

		zIndex = getZIndex();
		if (zIndex != -1) {
			return zIndex;

		} else {
			return 0;
		}
	}

	bool FormatterDeviceRegion::isBottomPercentual() {
		return false;
	}

	bool FormatterDeviceRegion::isHeightPercentual() {
		return false;
	}

	bool FormatterDeviceRegion::isLeftPercentual() {
		return false;
	}

	bool FormatterDeviceRegion::isRightPercentual() {
		return false;
	}

	bool FormatterDeviceRegion::isTopPercentual() {
		return false;
	}

	bool FormatterDeviceRegion::isWidthPercentual() {
		return false;
	}

	string FormatterDeviceRegion::toString() {
		string str;
		int i, size;
		LayoutRegion *region;

		str = "id: " + getId() + '\n';
		size = regions->size();
		for (i = 0; i < size; i++) {
			region = ((LayoutRegion*)(*regions)[i]);
			str = str + region->toString();
		}
		return str + '\n';
	}

	bool FormatterDeviceRegion::removeRegion(LayoutRegion *region) {
		vector<LayoutRegion*>::iterator it;

		if (regionSet.count(region) != 0) {
			for (it=regions->begin(); it!=regions->end(); ++it) {
				if (*it == region) {
					regions->erase(it);
					regionSet.erase(regionSet.find(region));
					return true;
				}
			}
		}
		return false;
	}

	void FormatterDeviceRegion::removeRegions() {
		regions->clear();
		regionSet.clear();
	}

	void FormatterDeviceRegion::setBackgroundColor(Color *newBackgroundColor) {

	}

	bool FormatterDeviceRegion::setBottom(double newBottom, bool isPercentual) {
		return false;
	}

	bool FormatterDeviceRegion::setHeight(double newHeight, bool isPercentual) {
		this->height = (int)newHeight;
		return true;
	}

	bool FormatterDeviceRegion::setLeft(double newLeft, bool isPercentual) {
		this->left = (int)newLeft;
		return true;
	}

	bool FormatterDeviceRegion::setRight(double newRight, bool isPercentual) {
		return false;
	}

	void FormatterDeviceRegion::setTitle(string newTitle) {
	}

	bool FormatterDeviceRegion::setTop(double newTop, bool isPercentual) {
		this->top = (int)newTop;
		return true;
	}

	bool FormatterDeviceRegion::setWidth(double newWidth, bool isPercentual) {
		this->width = (int)newWidth;
		return true;
	}

	void FormatterDeviceRegion::setZIndex(int newZIndex) {

	}

	vector<LayoutRegion*> *FormatterDeviceRegion::getRegionsSortedByZIndex() {
		vector<LayoutRegion*>* sortedRegions;
		/*vector<LayoutRegion*>::iterator componentRegions;
		LayoutRegion* ncmRegion;
		LayoutRegion* auxRegion;
		vector<LayoutRegion*>::iterator i;
		int j, size, zIndexValue;

		sortedRegions = new vector<LayoutRegion*>;

		componentRegions = regions->begin();
		while (componentRegions != regions->end()) {
			ncmRegion = (*componentRegions);
			zIndexValue = ncmRegion->getZIndexValue();

			size = sortedRegions->size();
			i = sortedRegions->begin();
			for (j = 0; j < size; j++) {
				auxRegion = *i;
				if (i == sortedRegions->end() ||
						zIndexValue <= (auxRegion)->
					    getZIndexValue()) {

					break;
				}
				++i;
			}
			sortedRegions->insert(i, ncmRegion);
			++componentRegions;
		}*/

		sortedRegions = new vector<LayoutRegion*>(*regions);
		return sortedRegions;
	}

	vector<LayoutRegion*> *FormatterDeviceRegion::getRegionsOverRegion(
		    LayoutRegion *region) {

		vector<LayoutRegion*> *allRegions;
		vector<LayoutRegion*> *frontRegions;
		LayoutRegion *childRegion;

		frontRegions = new vector<LayoutRegion*>;
		allRegions = getRegionsSortedByZIndex();
		vector<LayoutRegion*>::iterator it;
		for (it = allRegions->begin(); it != allRegions->end(); ++it) {
			childRegion = *it;
			if (childRegion->getZIndexValue() > region->getZIndexValue()) {
				frontRegions->insert(frontRegions->begin(), childRegion);
			}
		}
		delete allRegions;
		allRegions = NULL;

		return frontRegions;
	}

	LayoutRegion *FormatterDeviceRegion::getParent() {
		return NULL;
	}

	void FormatterDeviceRegion::setParent(LayoutRegion *parent) {

	}

	int FormatterDeviceRegion::getTopInPixels() {
		return top;
	}

	int FormatterDeviceRegion::getBottomInPixels() {
		return top + height;
	}

	int FormatterDeviceRegion::getRightInPixels() {
		return left + width;
	}

	int FormatterDeviceRegion::getLeftInPixels() {
		return left;
	}

	int FormatterDeviceRegion::getHeightInPixels() {
		return height;
	}

	int FormatterDeviceRegion::getWidthInPixels() {
		return width;
	}

	bool FormatterDeviceRegion::isMovable() {
		return false;
	}

	bool FormatterDeviceRegion::isResizable() {
		return false;
	}

	bool FormatterDeviceRegion::isDecorated() {
		return false;
	}

	void FormatterDeviceRegion::setMovable(bool movable) {
	}

	void FormatterDeviceRegion::setResizable(bool resizable) {
	}

	void FormatterDeviceRegion::setDecorated(bool decorated) {
	}

	void FormatterDeviceRegion::resetTop() {
	}

	void FormatterDeviceRegion::resetBottom() {
	}

	void FormatterDeviceRegion::resetLeft() {
	}

	void FormatterDeviceRegion::resetHeight() {
	}

	void FormatterDeviceRegion::resetWidth() {
	}

	void FormatterDeviceRegion::resetZIndex() {
	}

	void FormatterDeviceRegion::resetDecorated() {
	}

	void FormatterDeviceRegion::resetMovable() {
	}

	void FormatterDeviceRegion::resetResizable() {
	}

	int FormatterDeviceRegion::getAbsoluteLeft() {
		return left;
	}

	int FormatterDeviceRegion::getAbsoluteTop() {
		return top;
	}

	void FormatterDeviceRegion::dispose() {
		removeRegions();
		//regions = NULL;
	}

	string FormatterDeviceRegion::getId() {
		return id;
	}

	void FormatterDeviceRegion::setId(string id) {
		this->id = id;
	}

	Entity *FormatterDeviceRegion::getDataEntity() {
		return NULL;
	}

	int FormatterDeviceRegion::compareTo(void *arg0) {
		return 0;
	}
}
}
}
}
}
}
}
