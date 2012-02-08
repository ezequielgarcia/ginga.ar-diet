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

#include "../../../include/model/CascadingDescriptor.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace presentation {
	CascadingDescriptor::CascadingDescriptor(
		    GenericDescriptor* firstDescriptor) {

		initializeCascadingDescriptor();

		if (firstDescriptor != NULL) {
			cascade(firstDescriptor);
		}
	}

	CascadingDescriptor::CascadingDescriptor(CascadingDescriptor* descriptor) {

		initializeCascadingDescriptor();

		int i, size;

		if (descriptor != NULL) {
			size = ((CascadingDescriptor*)descriptor)->descriptors->size();
			for (i = 0; i < size; i++) {
				cascade((GenericDescriptor*)((*((CascadingDescriptor*)
					    descriptor)->descriptors)[i]));
			}
			size = ((CascadingDescriptor*)descriptor)->unsolvedDescriptors->size();
			for (i = 0; i < size; i++) {
				cascade((GenericDescriptor*)((*((CascadingDescriptor*)
					    descriptor)->unsolvedDescriptors)[i]));
			}
		}
	}

	CascadingDescriptor::~CascadingDescriptor() {
		focusBorderColor = NULL;
		selBorderColor = NULL;

		if (formatterRegion != NULL) {
			delete formatterRegion;
			formatterRegion = NULL;
		}

		if (inputTransitions != NULL) {
			delete inputTransitions;
			inputTransitions = NULL;
		}

		if (outputTransitions != NULL) {
			delete outputTransitions;
			outputTransitions = NULL;
		}

		if (parameters != NULL) {
			delete parameters;
			parameters = NULL;
		}

		if (descriptors != NULL) {
			delete descriptors;
			descriptors = NULL;
		}

		if (unsolvedDescriptors != NULL) {
			delete unsolvedDescriptors;
			unsolvedDescriptors = NULL;
		}
	}

	void CascadingDescriptor::initializeCascadingDescriptor() {
		typeSet.insert("CascadingDescriptor");

		id = "";
		explicitDuration = NaN();
		playerName = "";
		repetitions = 0;
		freeze = false;
		region = NULL;
		formatterRegion = NULL;

		focusIndex = "";
		moveUp = "";
		moveDown = "";
		moveLeft = "";
		moveRight = "";

		focusSrc = "";
		selectionSrc = "";
		focusBorderColor = NULL;
		selBorderColor = NULL;
		focusBorderWidth = 0;
		focusBorderTransparency = NaN();
		inputTransitions = new vector<Transition*>;
		outputTransitions = new vector<Transition*>;
		parameters = new map<string, string>;
		descriptors = new vector<GenericDescriptor*>;
		unsolvedDescriptors = new vector<GenericDescriptor*>;
	}

	bool CascadingDescriptor::instanceOf(string s) {
		if (typeSet.empty())
			return false;
		else
			return (typeSet.find(s) != typeSet.end());
	}

	string CascadingDescriptor::getId() {
		return id;
	}

	void CascadingDescriptor::cascadeDescriptor(Descriptor* descriptor) {
		if (descriptor->getPlayerName() != "")
			playerName = descriptor->getPlayerName();

		region = descriptor->getRegion();
		if (!isNaN(descriptor->getExplicitDuration())) {
			explicitDuration = descriptor->getExplicitDuration();
		}

		this->freeze = descriptor->isFreeze();

		if (descriptor->getKeyNavigation() != NULL) {
			KeyNavigation* keyNavigation;
			keyNavigation = (KeyNavigation*)(
				    descriptor->getKeyNavigation());

			if (keyNavigation->getFocusIndex() != "") {
				focusIndex = keyNavigation->getFocusIndex();
			}

			if (keyNavigation->getMoveUp() != "") {
				moveUp = keyNavigation->getMoveUp();
			}

			if (keyNavigation->getMoveDown() != "") {
				moveDown = keyNavigation->getMoveDown();
			}

			if (keyNavigation->getMoveLeft() != "") {
				moveLeft = keyNavigation->getMoveLeft();
			}

			if (keyNavigation->getMoveRight() != "") {
				moveRight = keyNavigation->getMoveRight();
			}
		}

		if (descriptor->getFocusDecoration() != NULL) {
			FocusDecoration* focusDecoration;
			focusDecoration = (FocusDecoration*)(
				    descriptor->getFocusDecoration());

			if (focusDecoration->getFocusBorderColor() != NULL) {
				focusBorderColor = focusDecoration->getFocusBorderColor();
			}

			if (focusDecoration->getSelBorderColor() != NULL) {
				selBorderColor = focusDecoration->getSelBorderColor();
			}

			focusBorderWidth = focusDecoration->getFocusBorderWidth();

			if (!(isNaN(focusDecoration->
				    getFocusBorderTransparency()))) {

				focusBorderTransparency = focusDecoration->
					    getFocusBorderTransparency();
			}

			if (focusDecoration->getFocusSrc() != "") {
				focusSrc = focusDecoration->getFocusSrc();
			}

			if (focusDecoration->getFocusSelSrc() != "") {
				selectionSrc = focusDecoration->getFocusSelSrc();
			}
		}

		vector<Transition*>* transitions = NULL;
		transitions = descriptor->getInputTransitions();
		if (transitions != NULL) {
			if (this->inputTransitions == NULL) {
				this->inputTransitions = new vector<Transition*>;
			}

			inputTransitions->insert(
				    inputTransitions->end(),
				    transitions->begin(),
				    transitions->end());
		}

		transitions = descriptor->getOutputTransitions();
		if (transitions != NULL) {
			if (this->outputTransitions == NULL) {
				this->outputTransitions = new vector<Transition*>;
			}

			outputTransitions->insert(
				    outputTransitions->end(),
				    transitions->begin(),
				    transitions->end());
		}

		Parameter* param;
		vector<Parameter*>* paramsMap;
		vector<Parameter*>::iterator it;

		paramsMap = descriptor->getParameters();
		if (paramsMap != NULL) {
			for (it = paramsMap->begin(); it != paramsMap->end(); ++it) {
				param = *it;
				(*parameters)[param->getName()] = param->getValue();
			}
			delete paramsMap;
			paramsMap = NULL;
		}
	}

	bool CascadingDescriptor::isLastDescriptor(
		    GenericDescriptor* descriptor) {

		if ((descriptors->size() > 0) &&
			    (descriptor->getId() == ((GenericDescriptor*)
			        (*descriptors)[descriptors->size() - 1])->getId()))

			return true;
		else
			return false;
	}

	void CascadingDescriptor::cascade(GenericDescriptor *descriptor) {
		GenericDescriptor* preferredDescriptor;

		preferredDescriptor = (GenericDescriptor*)
			    (descriptor->getDataEntity());

		if ((preferredDescriptor == NULL) ||
			    preferredDescriptor->instanceOf("CascadingDescriptor"))

			return;

		if (isLastDescriptor(preferredDescriptor))
			return;

		descriptors->push_back(preferredDescriptor);
		if (id == "")
			id = preferredDescriptor->getId();
		else
			id = id + "+" + preferredDescriptor->getId();

		if (preferredDescriptor->instanceOf("Descriptor") &&
			    unsolvedDescriptors->empty()) {

			cascadeDescriptor((Descriptor*)preferredDescriptor);

		} else {
			unsolvedDescriptors->push_back(preferredDescriptor);
		}
	}

	GenericDescriptor* CascadingDescriptor::
		    getFirstUnsolvedDescriptor() {

		if (unsolvedDescriptors->size() > 0)
			return (GenericDescriptor*)
				    (*(unsolvedDescriptors->begin()));
		else
			return NULL;
	}

	vector<GenericDescriptor*>* CascadingDescriptor::getUnsolvedDescriptors() {
		return unsolvedDescriptors;
	}

	void CascadingDescriptor::cascadeUnsolvedDescriptor() {
		if (unsolvedDescriptors->empty())
			return;

		GenericDescriptor* genericDescriptor, *descriptor;
		DescriptorSwitch* descAlternatives;
		GenericDescriptor* auxDescriptor;

		genericDescriptor = (GenericDescriptor*)
			    ((*unsolvedDescriptors)[0]);

		if (genericDescriptor->instanceOf("DescriptorSwitch")) {
			descAlternatives = (DescriptorSwitch*)genericDescriptor;
			auxDescriptor = descAlternatives->getSelectedDescriptor();
			descriptor = (GenericDescriptor*)
				    auxDescriptor->getDataEntity();

		} else {
			descriptor = (Descriptor*)genericDescriptor;
		}

		unsolvedDescriptors->erase(unsolvedDescriptors->begin());

		if (isLastDescriptor(descriptor)) {
			return;
		}

		cascadeDescriptor((Descriptor*)descriptor);
	}

	double CascadingDescriptor::getExplicitDuration() {
		return (this->explicitDuration);
	}

	bool CascadingDescriptor::getFreeze() {
		return freeze;
	}

	string CascadingDescriptor::getPlayerName() {
		return playerName;
	}

	LayoutRegion* CascadingDescriptor::getRegion() {
		return region;
	}

	FormatterRegion* CascadingDescriptor::getFormatterRegion() {
		return formatterRegion;
	}

	void CascadingDescriptor::setFormatterRegion(FormatterRegion* region) {
		if (this->formatterRegion != NULL && region != formatterRegion) {
			//TODO: is it occours? is it a memory leak?
			cout << "CascadingDescriptor::setFormatterRegion Warning!";
			cout << " is it a memory leak?" << endl;
		}
		formatterRegion = region;
	}

	void CascadingDescriptor::setFormatterLayout(void* formatterLayout) {
		if (region != NULL) {
			if (this->formatterRegion != NULL) {
				cout << "CascadingDescriptor::setFormatterRegion Warning!";
				cout << " memory leak!" << endl;
			}
  			formatterRegion = new FormatterRegion(id, this, formatterLayout);
		}
	}

	long CascadingDescriptor::getRepetitions() {
		return repetitions;
	}

	vector<Parameter*>* CascadingDescriptor::getParameters() {
		vector<Parameter*>* params;
		params = new vector<Parameter*>;
		Parameter* parameter;

		map<string, string>::iterator it;

		for (it = parameters->begin(); it != parameters->end(); ++it) {
			parameter = new Parameter(it->first, it->second);
			params->push_back(parameter);
		}
		return params;
	}

	string CascadingDescriptor::getParameterValue(string paramName) {
		string::size_type pos;
		string paramValue;
		double value;

		if (parameters == NULL || parameters->count(paramName) == 0) {
			return "";
		}

		paramValue = (*parameters)[paramName];

		if (paramValue == "") {
			return paramValue;
		}

		pos = paramValue.find_last_of("%");
		if (pos != std::string::npos && pos == paramValue.length() - 1) {
			paramValue = paramValue.substr(0, paramValue.length() - 1);
			if (isNumeric((void*)(paramValue.c_str()))) {
				value = stof(paramValue) / 100;
				paramValue = itos(value);
			}
		}

		return paramValue;
	}

	vector<GenericDescriptor*>* CascadingDescriptor::getNcmDescriptors() {
		return this->descriptors;
	}

	Color* CascadingDescriptor::getFocusBorderColor() {
		return focusBorderColor;
	}

	double CascadingDescriptor::getFocusBorderTransparency() {
		return focusBorderTransparency;
	}

	int CascadingDescriptor::getFocusBorderWidth() {
		return focusBorderWidth;
	}

	string CascadingDescriptor::getFocusIndex() {
		return focusIndex;
	}

	string CascadingDescriptor::getFocusSrc() {
		return focusSrc;
	}

	string CascadingDescriptor::getSelectionSrc() {
		return selectionSrc;
	}

	string CascadingDescriptor::getMoveDown() {
		return moveDown;
	}

	string CascadingDescriptor::getMoveLeft() {
		return moveLeft;
	}

	string CascadingDescriptor::getMoveRight() {
		return moveRight;
	}

	string CascadingDescriptor::getMoveUp() {
		return moveUp;
	}

	Color* CascadingDescriptor::getSelBorderColor() {
		return selBorderColor;
	}

	vector<Transition*>* CascadingDescriptor::getInputTransitions() {
		return this->inputTransitions;
	}

	vector<Transition*>* CascadingDescriptor::getOutputTransitions() {
		return this->outputTransitions;
	}
}
}
}
}
}
}
}
