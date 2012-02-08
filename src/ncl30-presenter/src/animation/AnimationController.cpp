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
    Copyright 2011 Fernando Vasconcelos - Microtrol - Rosario, Argentina


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

#include "../../include/animation/AnimationController.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace animation {

	void AnimationController::startAnimation(
			ExecutionObject* execObj,
			FormatterPlayerAdapter* player,
			AttributionEvent* event,
			string value,
			Animation* anim){

		AnimationController *animControl = new AnimationController(
				execObj, player, event, value, anim);

		animControl->start();
	}

	AnimationController::AnimationController() {

	}

	AnimationController::AnimationController(
			ExecutionObject* execObj,
			FormatterPlayerAdapter* player,
			AttributionEvent* event,
			string value,
			Animation* anim){

		this->execObj    = execObj;
		this->player     = player;
		this->event      = event;
		this->animation  = anim;

		value   	     = trim(value);
		params 		 	 = split(value, ",");

		this->propName   = (event->getAnchor())->getPropertyName();

		this->duration	 = stof(anim->getDuration());
		this->stepSize 	 = (int) stof(anim->getBy());

		this->targetValues 		= new vector<double>();
		this->strTargetValues 	= new vector <string>();

	}

	AnimationController::~AnimationController() {
		cout << "AnimationController::~AnimationController : destructor called";
		cout << endl;

		delete this->params;

		this->targetValues->clear();
		delete this->targetValues;

		this->strTargetValues->clear();
		delete this->strTargetValues;
	}

	/**
	 * WARNING: this method calls "delete this" when the animation is done.
	 * NOTA: Se soluciono el bug de animacion. La variable done no estaba 
	 * inicializada por lo que muchas veces era tomada como TRUE.
	 */
	void AnimationController::run() {
		bool done = false;
		// if the property name is a group, this function will ungroup them.
		if (!ungroupProperty()) {
			cout << "AnimationController::run : [WARNING] the animating can ";
			cout << "not be started. The parameters passed by the NCL program ";
			cout << "are wrong." << endl;

			return;
		}

		if (loadInitValues() && loadTargetValues()) {
			this->initTime = getCurrentTimeMillis();
			//the animation can be performed
			while (!done) {
				done = animeStep();
			}

		} else {
			cout << "AnimationController::run() : problem with loadInitValues ";
			cout << "or loadTargeValues (false returned)"<<endl;
		}

		this->event->stop();
		delete this;
	}

	bool AnimationController::animeStep() {
		double time;
		int factor = 1;
		unsigned int i;
		vector<double>* nextValues = new vector<double>;

		time = getCurrentTimeMillis();

		//cout << "AnimationController::animeStep : Next Values = ";
		for (i = 0; i < initValues.size(); i++) {
			nextValues->push_back(
					getNextStepValue(
							this->initValues[i],
							(*this->targetValues)[i],
							factor,
							time,
							this->initTime,
							(this->duration * 1000),
							stepSize)
					);
			//cout << (*nextValues)[i] << " ";
		}
		//cout << endl;

		if (time >= (this->initTime + (this->duration * 1000))) {
			nextValues->clear();
			delete nextValues;
			if (isExecObjAnim) {
				if (execObj->setPropertyValue(
						this->propName, this->targetValues)) {

					return true;
				}

			} else {
				//TODO: change FormatterPlayerAdapter to receive a group of
				//properties
				if (player->setPropertyValue(event,
						itos((*targetValues)[0]), (Animation*)NULL)) {

					return true;
				}
			}

		} else {
			if (isExecObjAnim) {
				execObj->setPropertyValue(this->propName, nextValues);

			} else {
				player->setPropertyValue(
						event, itos((*nextValues)[0]), (Animation*)NULL);
			}
			nextValues->clear();
			delete nextValues;
		}

		return false;
	}


	bool AnimationController::loadInitValues() {
		double propValue;
		unsigned int i;

		this->initValues.clear();
		if (isExecObjAnim) {
			for (i = 0; i < this->propertySingleNames.size(); i++) {
				propValue = execObj->getPropertyValueAsDouble(
						this->propertySingleNames[i]);

				/*cout << "AnimationController::loadInitValues (execObj): ";
				cout << propertySingleNames[i] << " value = '";
				cout << propValue << endl;*/

				this->initValues.push_back(propValue);
			}

		} else {
			for (i = 0; i < this->propertySingleNames.size(); i++) {
				propValue = stof(player->getPropertyValue(this->event));

				/*cout << "AnimationController::loadInitValues (player): ";
				cout << propertySingleNames[i] << " value = '";
				cout << propValue << endl;*/

				this->initValues.push_back(propValue);
			}
		}
		return true;
	}

	bool AnimationController::loadTargetValues() {
		CascadingDescriptor* descriptor = NULL;
		FormatterRegion* region = NULL;
		unsigned int i;

		this->targetValues->clear();

		if (isExecObjAnim) {
			descriptor = this->execObj->getDescriptor();

			if (descriptor == NULL ||
					descriptor->getFormatterRegion() == NULL) {

				cout << "AnimationController::loadTargetValues : load target";
				cout << " could not be performed. Descriptor or ";
				cout << "formatterRegion is NULL." << endl;

				return false;
			}

			region = descriptor->getFormatterRegion();
			this->initRegion = region->getLayoutRegion();

			/*cout << "AnimationController::loadTargetValues : The animation ";
			cout << "should start with values -";
			cout << " top: "<<initRegion->getTopInPixels();
			cout << " left: "<<initRegion->getLeftInPixels();
			cout << " width: "<<initRegion->getWidthInPixels();
			cout << " height:"<<initRegion->getHeightInPixels();
			cout << endl;*/

			this->targetRegion = initRegion->copyRegion();

			updateTargetRegion();
		}

		for (i = 0; i < propertySingleNames.size(); i++) {
			this->targetValues->push_back(
					getSinglePropertyTarget(i));
		}
		return true;
	}

	bool AnimationController::ungroupProperty() {
		int i;

		if (this->propName == "") {
			return false;
		}

		this->isExecObjAnim = false;

		this->targetValues->clear();
		this->propertySingleNames.clear();

		if (this->propName == "size") {

			this->isExecObjAnim = true;

			if (this->params->size() == 2) {
				this->propertySingleNames.push_back("width");
				this->propertySingleNames.push_back("height");

				this->strTargetValues->push_back((*params)[0]);
				this->strTargetValues->push_back((*params)[1]);

			} else {
				return false;
			}

		} else if (this->propName == "location") {

			this->isExecObjAnim = true;

			if (this->params->size() == 2) {
				this->propertySingleNames.push_back("left");
				this->propertySingleNames.push_back("top");

				this->strTargetValues->push_back((*params)[0]);
				this->strTargetValues->push_back((*params)[1]);

			} else {
				return false;
			}

		} else if (this->propName == "bounds") {
			this->isExecObjAnim = true;

			if (this->params->size() == 4) {
				this->propertySingleNames.push_back("left");
				this->propertySingleNames.push_back("top");
				this->propertySingleNames.push_back("width");
				this->propertySingleNames.push_back("height");

				for (i = 0; i < 4; i++) {
					this->strTargetValues->push_back((*params)[i]);
				}

			} else {
				return false;
			}

		} else {
			this->isExecObjAnim = isExecutionObjectProperty(propName);

			if (this->params->size() == 1) {
				this->propertySingleNames.push_back(propName);
				this->strTargetValues->push_back((*params)[0]);

			} else {
				return false;
			}
		}

		return true;
	}

	bool AnimationController::isExecutionObjectProperty(string name) {
		if (name == "bounds" || name == "location" || name == "size"
				|| propName == "left" || propName == "top"
				|| propName == "bottom" || propName == "right"
				|| propName == "width" || propName == "height"
				|| propName == "transparency") {

			return true;
		}

		return false;
	}

	//TODO: should return false if any problem occurs
	bool AnimationController::updateTargetRegion() {
		bool percent;
		string param, value;
		unsigned int i;

		for (i = 0; i < strTargetValues->size(); i++) {
			param   = propertySingleNames[i];
			value   = (*strTargetValues)[i];
			percent = isPercentualValue(value);

			if (percent) {
				if (param == "left") {
					targetRegion->setTargetLeft(
							getPercentualValue(value), true);

				} else if (param == "top") {
					targetRegion->setTargetTop(
							getPercentualValue(value), true);

				} else if (param == "width") {
					targetRegion->setTargetWidth(
							getPercentualValue(value), true);

				} else if (param == "height") {
					targetRegion->setTargetHeight(
							getPercentualValue(value), true);

				} else if (param == "bottom") {
					targetRegion->setTargetBottom(
							getPercentualValue(value), true);

				} else if (param == "right") {
					targetRegion->setTargetRight(
							getPercentualValue(value), true);
				}

			} else {
				if (param == "left") {
					targetRegion->setTargetLeft(
							(double)(stof(value)), false);

				} else if (param == "top") {
					targetRegion->setTargetTop(
							(double)(stof(value)), false);

				} else if (param == "width") {
					targetRegion->setTargetWidth(
							(double)(stof(value)), false);

				} else if (param == "height") {
					targetRegion->setTargetHeight(
							(double)(stof(value)), false);

				} else if (param == "bottom") {
					targetRegion->setTargetBottom(
							(double)(stof(value)), false);

				} else if (param == "right") {
					targetRegion->setTargetRight(
							(double)(stof(value)), false);
				}
			}
		}

		return true;
	}

	double AnimationController::getSinglePropertyTarget(int i) {
		string propName = propertySingleNames[i];
		double target = 0;

		if (propName == "left") {
			target = targetRegion->getAbsoluteLeft();

		} else if (propName == "top") {
			target = targetRegion->getAbsoluteTop();

		} else if (propName == "width") {
			target = targetRegion->getWidthInPixels();

		} else if (propName == "height") {
			target = targetRegion->getHeightInPixels();

		} else {
			target = (double)(stof((*strTargetValues)[i]));
		}

		return target;
	}
}
}
}
}
}
}
