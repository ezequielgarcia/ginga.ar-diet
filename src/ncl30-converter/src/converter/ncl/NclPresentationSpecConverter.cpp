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

#include "../../../include/ncl/NclPresentationSpecConverter.h"
#include "../../../include/ncl/NclImportConverter.h"
#include "../../../include/ncl/NclPresentationControlConverter.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace converter {
namespace ncl {
	NclPresentationSpecConverter::NclPresentationSpecConverter(
		    DocumentParser *documentParser) :
		    	    NclPresentationSpecificationParser(documentParser) {


	}

	void NclPresentationSpecConverter::addDescriptorToDescriptorBase(
		    void *parentObject, void *childObject) {

		((DescriptorBase*) parentObject)->addDescriptor(
			    (GenericDescriptor*) childObject);
	}

	void NclPresentationSpecConverter::addDescriptorSwitchToDescriptorBase(
		    void *parentObject, void *childObject) {

		((DescriptorBase*) parentObject)->addDescriptor(
			    (GenericDescriptor*) childObject);
	}

	void NclPresentationSpecConverter::addDescriptorParamToDescriptor(
		     void *parentObject, void *childObject) {

		// cast para descritor
		Descriptor *descriptor = (Descriptor*) parentObject;
		DOMElement*param = (DOMElement*) childObject;
		// recuperar nome e valor da variavel
		string paramName = XMLString::transcode(param->getAttribute(
			    XMLString::transcode("name")));

		string paramValue = XMLString::transcode(param->getAttribute(
			    XMLString::transcode("value")));

		// adicionar variavel ao descritor
		Parameter *descParam = new Parameter(paramName, paramValue);
		descriptor->addParameter(descParam);
	}

	void NclPresentationSpecConverter::addImportBaseToDescriptorBase(
		    void *parentObject, void *childObject) {

		string baseAlias, baseLocation;
		NclDocumentConverter* compiler;
		NclDocument *importedDocument, *thisDocument;
		DescriptorBase *descriptorBase;
		map<int, RegionBase*>* regionBases;
		map<int, RegionBase*>::iterator i;
		RuleBase *ruleBase;

		// get the external base alias and location
		baseAlias = XMLString::transcode( ((DOMElement*) childObject)->
			    getAttribute(XMLString::transcode("alias")) );

		baseLocation = XMLString::transcode( ((DOMElement*) childObject)->
			    getAttribute(XMLString::transcode("documentURI")) );

		compiler = (NclDocumentConverter*)getDocumentParser();
		importedDocument = compiler->importDocument(baseLocation);
		if (importedDocument == NULL) {
			return;
		}

		descriptorBase = importedDocument->getDescriptorBase();
		if (descriptorBase == NULL) {
			return;
		}

		// insert the imported base into the document descriptor base
		try {
			((DescriptorBase*) parentObject)->addBase(
				    descriptorBase, baseAlias, baseLocation);

		} catch (...) {

		}

		// importing descriptor bases implies importing region, rule, and cost
		// function bases in order to maintain reference consistency
		thisDocument = (NclDocument*) getDocumentParser()->
			    getObject("return", "document");

		regionBases = importedDocument->getRegionBases();
		if (regionBases != NULL && !regionBases->empty()) {
			i = regionBases->begin();
			while (i != regionBases->end()) {
				thisDocument->getRegionBase(0)->addBase(
							i->second, baseAlias, baseLocation);

				++i;
			}
		}

		ruleBase = importedDocument->getRuleBase();
		if (ruleBase != NULL) {
			try {
				thisDocument->getRuleBase()->addBase(
					    ruleBase, baseAlias, baseLocation);

			} catch (...) {

			}
		}
	}

	void *NclPresentationSpecConverter::createDescriptorParam(
		    DOMElement *parentElement, void *objGrandParent) {

		return parentElement;
	}

	void *NclPresentationSpecConverter::createDescriptorBind(
		    DOMElement *parentElement, void *objGrandParent) {

		// ver componentCompilers para comentarios
		return parentElement;
	}

	void *NclPresentationSpecConverter::createDescriptorBase(
		    DOMElement *parentElement, void *objGrandParent) {

		DescriptorBase *descBase;
		// criar nova base de conectores com id gerado a partir do nome de seu
		// elemento
		descBase = new DescriptorBase(XMLString::transcode(
			    parentElement->getAttribute(XMLString::transcode("id"))));

		return descBase;
	}

	void* NclPresentationSpecConverter::createDescriptor(
		    DOMElement *parentElement, void *objGrandParent) {

		Descriptor *descriptor;
		NclDocument *document;
		LayoutRegion* region;

		long duration;

		KeyNavigation* keyNavigation;
		string src;
		FocusDecoration* focusDecoration;
		Color* color;
		string attValue;
		vector<string>* transIds;
		unsigned int i;
		TransitionBase* transitionBase;
		Transition* transition;
		
		XMLCh* tmpStr;
		char* tmpChar;

		// cria descritor
		tmpStr = XMLString::transcode("id");
		tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
		descriptor = new Descriptor(tmpChar);
		
		XMLString::release(&tmpStr);
		XMLString::release(&tmpChar);

		document = (NclDocument*)getDocumentParser()->
			    getObject("return", "document");

		// atributo region
		tmpStr = XMLString::transcode("region");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			region = document->getRegion(tmpChar);
			
			XMLString::release(&tmpChar);

			if (region != NULL) {
				descriptor->setRegion(region);
			}
		}
		
		XMLString::release(&tmpStr);

		// atributo explicitDur
		tmpStr = XMLString::transcode("explicitDur");
		if (parentElement->hasAttribute(tmpStr)) {

			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			string durStr = string(tmpChar);
			XMLString::release(&tmpChar);

			duration = (long)(stof(durStr.substr(0, durStr.length() - 1))
				    * 1000);

			descriptor->setExplicitDuration(duration);
		}
		XMLString::release(&tmpStr);
		
		tmpStr = XMLString::transcode("freeze");
		if (parentElement->hasAttribute(tmpStr)) {
			string freeze;
			
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			freeze = string(tmpChar);
			XMLString::release(&tmpChar);

			if (freeze == "true") {
				descriptor->setFreeze(true);

			} else {
				descriptor->setFreeze(false);
			}
		}
		XMLString::release(&tmpStr);

		// atributo player
		tmpStr = XMLString::transcode("player");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			descriptor->setPlayerName(tmpChar);
			XMLString::release(&tmpChar);
		}
		XMLString::release(&tmpStr);

		// key navigation attributes
		keyNavigation = new KeyNavigation();
		descriptor->setKeyNavigation(keyNavigation);
		tmpStr = XMLString::transcode("focusIndex");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			keyNavigation->setFocusIndex(tmpChar);
			XMLString::release(&tmpChar);
			
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("moveUp");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			keyNavigation->setMoveUp(tmpChar);
			XMLString::release(&tmpChar);
		}
		XMLString::release(&tmpChar);

		tmpStr = XMLString::transcode("moveDown");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			keyNavigation->setMoveDown(tmpChar);
			XMLString::release(&tmpChar);
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("moveLeft");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			keyNavigation->setMoveLeft(tmpChar);
			XMLString::release(&tmpChar);
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("moveRight");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			keyNavigation->setMoveRight(tmpChar);
			XMLString::release(&tmpChar);
		}
		XMLString::release(&tmpStr);

		focusDecoration = new FocusDecoration();
		descriptor->setFocusDecoration(focusDecoration);
		tmpStr = XMLString::transcode("focusSrc");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			src = string(tmpChar);
			XMLString::release(&tmpChar);

			if (isAbsolutePath(src)) {
				focusDecoration->setFocusSrc(src);

	    	} else {
				focusDecoration->setFocusSrc(
					getDocumentParser()->getDocumentPath() + src);
	    	}
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("focusBorderColor");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			color = new Color(string(tmpChar));
			XMLString::release(&tmpChar);

			focusDecoration->setFocusBorderColor(color);
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("focusBorderWidth");
		if (parentElement->hasAttribute(tmpStr)) {

			int w;
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			w = (int)stof(string(tmpChar));
			XMLString::release(&tmpChar);

			focusDecoration->setFocusBorderWidth(w);
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("focusBorderTransparency");
		if (parentElement->hasAttribute(tmpStr)) {

			double alpha;
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			alpha = (double)stof(string(tmpChar));
			XMLString::release(&tmpChar);
			

			focusDecoration->setFocusBorderTransparency(alpha);
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("focusSelSrc");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			src = string(tmpChar);
			XMLString::release(&tmpChar);

			if (isAbsolutePath(src)) {
				focusDecoration->setFocusSelSrc(src);

			} else {
				focusDecoration->setFocusSelSrc(
					    getDocumentParser()->getDocumentPath() + src);
			}
		}
		XMLString::release(&tmpStr);
		
		tmpStr = XMLString::transcode("selBorderColor");
		if (parentElement->hasAttribute(tmpStr)) {

			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			color = new Color(string(tmpChar));
			XMLString::release(&tmpChar);
			
			focusDecoration->setSelBorderColor(color);
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("transIn");
		if (parentElement->hasAttribute(tmpStr)) {
			transitionBase = document->getTransitionBase();
			if (transitionBase != NULL) {
				string trimValue, value;
				tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
				attValue = string(tmpChar);
				XMLString::release(&tmpChar);

				transIds = split(attValue, ";");
				if (!transIds->empty()) {
					vector<string>::iterator i;
					i = transIds->begin();
					int j = 0;
					while (i != transIds->end()) {
						value = (*i);
						trimValue = trim(value);
						*i = trimValue;
						transition = transitionBase->getTransition(trimValue);
						if (transition != NULL) {
							descriptor->addInputTransition(transition, j);
						}
						++i;
						j++;
					}
				}

				delete transIds;
				transIds = NULL;
			}
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("transOut");
		if (parentElement->hasAttribute(tmpStr)) {
			transitionBase = document->getTransitionBase();
			if (transitionBase != NULL) {
				string trimValue, value;
				tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
				attValue = string(tmpChar);
				XMLString::release(&tmpChar);

				transIds = split(attValue, ";");
				if (!transIds->empty()) {
					for (i = 0; i < transIds->size(); i++) {
						value = (*transIds)[i];
						trimValue = trim(value);
						(*transIds)[i] = trimValue;
						transition = transitionBase->getTransition(trimValue);
						if (transition != NULL) {
							descriptor->addOutputTransition(transition, i);
						}
					}
				}

				delete transIds;
				transIds = NULL;
			}
		}
		XMLString::release(&tmpStr);
		
		return descriptor;
	}
}
}
}
}
}
