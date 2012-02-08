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

#include "../../../include/ncl/NclInterfacesConverter.h"
#include "../../../include/ncl/NclDocumentConverter.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace converter {
namespace ncl {
	NclInterfacesConverter::NclInterfacesConverter(
		    DocumentParser *documentParser) :
		    	    NclInterfacesParser(documentParser) {


	}

	void *NclInterfacesConverter::createPort(
		    DOMElement* parentElement, void *objGrandParent) {

		string id, attValue;
		Node *portNode;

		CompositeNode *context = (CompositeNode*) objGrandParent;
		XMLCh* component;
		char* attribName;

		component = XMLString::transcode("id");
		if (!parentElement->hasAttribute(component)) {
			wclog << "Error: a port element was declared without";
			wclog << " an id attribute." << endl;
			return NULL;
		}
		
		attribName = XMLString::transcode(parentElement->getAttribute(component));
		id = string(attribName);
		XMLString::release(&attribName);
		XMLString::release(&component);
		
		if (context->getPort(id) != NULL) {
			wclog << "Error: a port already exists with the same ";
			wclog << id.c_str() << " id in " << context->getId().c_str();
			wclog << " context" << endl;
			return NULL;
		}

		component = XMLString::transcode("component");
		if (!parentElement->hasAttribute(component)) {
			wclog << "Error: " << id.c_str() << " port must refer";
			wclog << " a context component using component attribute.";
			wclog << endl;
			return NULL;
		}
    
		attribName = XMLString::transcode(parentElement->getAttribute(component));
		attValue = string(attribName);
		XMLString::release(&attribName);
		XMLString::release(&component);

		portNode = context->getNode(attValue);
		if(portNode == NULL) {
			wclog << "Composition does not contain the referenced ";
			wclog << attValue.c_str() << " component." << endl;

			return NULL;
		}

		NodeEntity *portNodeEntity;
		portNodeEntity = (NodeEntity*) portNode->getDataEntity();
		InterfacePoint *portInterfacePoint;
		Port *port = NULL;

		component = XMLString::transcode("interface");
		if (!parentElement->hasAttribute(component)) {
			// port element nao tem o atributo port, logo pegar a
			// ancora lambda do docment node->
			portInterfacePoint = portNodeEntity->getAnchor(0);

		} else {
			// tem-se o atributo port
			// tentar pegar a ancora do document node
			attribName =XMLString::transcode(parentElement->getAttribute(component));
			attValue = string(attribName);
			

			portInterfacePoint = portNodeEntity->getAnchor(attValue);
			if (portInterfacePoint == NULL && portNodeEntity->instanceOf(
				    "CompositeNode")) {

				// the interface may refer to a composition port and instead of
				// an anchor
				portInterfacePoint = ((CompositeNode*) portNodeEntity)->
					    getPort(attValue);
			}
		}

		if (portInterfacePoint == NULL) {
			wclog << "The referenced " << portNode->getId().c_str();
			wclog << " component does not contain the referenced ";
			wclog << attribName;

			wclog << " interface" << endl;
			return NULL;
		}

		XMLString::release(&attribName);
		XMLString::release(&component);
		port = new Port(id, portNode, portInterfacePoint);
		return port;
	}

	SpatialAnchor *NclInterfacesConverter::createSpatialAnchor(
		    DOMElement *areaElement) {

		SpatialAnchor *anchor = NULL;
		string coords, shape;

		if (areaElement->hasAttribute(XMLString::transcode("coords"))) {
			coords = XMLString::transcode(areaElement->getAttribute(
				    XMLString::transcode("coords")));

			if (areaElement->hasAttribute(XMLString::transcode("shape"))) {
				shape = XMLString::transcode(areaElement->getAttribute(
					    XMLString::transcode("shape")));

			} else {
				shape = "rect";
			}

			if (shape=="rect" || shape=="default") {
				long int x1, y1, x2, y2;
				sscanf(coords.c_str(), "%ld,%ld,%ld,%ld", &x1, &y1, &x2, &y2);
				anchor = new RectangleSpatialAnchor(XMLString::transcode(
					    areaElement->getAttribute(XMLString::transcode("id"))),
					    x1, y1, x2 - x1, y2 - y1);

			} else if (shape=="circle") {
				// TODO
			} else if (shape=="poly") {
				// TODO
			}
		}
		return anchor;
	}

	IntervalAnchor *NclInterfacesConverter::createTemporalAnchor(
		    DOMElement *areaElement) {

		IntervalAnchor *anchor = NULL;
		string begin, end;
		double begVal, endVal;
		short firstSyntax, lastSyntax;

		if (areaElement->hasAttribute(XMLString::transcode("begin")) ||
			    areaElement->hasAttribute(XMLString::transcode("end"))) {

			if (areaElement->hasAttribute(XMLString::transcode("begin"))) {
				begin = XMLString::transcode(areaElement->getAttribute(
					    XMLString::transcode("begin")));

				if (begin[begin.length() - 1] == 's') {
					begin = begin.substr(0, begin.length() - 1);
				}
				begVal = stof(begin) * 1000;

			} else {
				begVal = 0;
			}

			if (areaElement->hasAttribute(XMLString::transcode("end"))) {
				end = XMLString::transcode(areaElement->getAttribute(
					    XMLString::transcode("end")));

				if (end[end.length() - 1] == 's') {
					end = end.substr(0, end.length() - 1);
				}
				endVal = stof(end) * 1000;

			} else {
				endVal = IntervalAnchor::OBJECT_DURATION;
			}
			anchor = new RelativeTimeIntervalAnchor(XMLString::transcode(
				    areaElement->getAttribute(XMLString::transcode("id"))),
				    begVal,
				    endVal);
		}

		// region delimeted through sample identifications
		if (areaElement->hasAttribute(XMLString::transcode("first")) ||
			    areaElement->hasAttribute(XMLString::transcode("last"))) {

			begVal      = 0;
			endVal      = IntervalAnchor::OBJECT_DURATION;
			firstSyntax = ContentAnchor::CAT_NPT;
			lastSyntax  = ContentAnchor::CAT_NPT;

			if (areaElement->hasAttribute(XMLString::transcode("first"))) {
				begin = XMLString::transcode(areaElement->getAttribute(
					    XMLString::transcode("first")));

				if (begin.find("s") != std::string::npos) {
					firstSyntax = ContentAnchor::CAT_SAMPLES;
					begVal = stof(begin.substr(0, begin.length() - 1));

				} else if (begin.find("f") != std::string::npos) {
					firstSyntax = ContentAnchor::CAT_FRAMES;
					begVal = stof(begin.substr(0, begin.length() - 1));

				} else if (begin.find("npt") != std::string::npos) {
					firstSyntax = ContentAnchor::CAT_NPT;
					begVal = stof(begin.substr(0, begin.length() - 3));
				}
			}

			if (areaElement->hasAttribute(XMLString::transcode("last"))) {
				end = XMLString::transcode(areaElement->getAttribute(
					    XMLString::transcode("last")));

				if (begin.find("s") != std::string::npos) {
					lastSyntax = ContentAnchor::CAT_SAMPLES;
					endVal = stof(end.substr(0, end.length() - 1));

				} else if (begin.find("f") != std::string::npos) {
					lastSyntax = ContentAnchor::CAT_FRAMES;
					endVal = stof(end.substr(0, end.length() - 1));

				} else if (begin.find("npt") != std::string::npos) {
					lastSyntax = ContentAnchor::CAT_NPT;
					endVal = stof(end.substr(0, end.length() - 3));
				}
			}

			anchor = new SampleIntervalAnchor(XMLString::transcode(
				    areaElement->getAttribute(XMLString::transcode("id"))),
				    begVal,
				    endVal);

			((SampleIntervalAnchor*)anchor)->setValueSyntax(
					firstSyntax, lastSyntax);

		}
		return anchor;
	}

	void *NclInterfacesConverter::createProperty(
		    DOMElement *parentElement, void *objGrandParent) {

		string attributeName, attributeValue;
		PropertyAnchor *anchor;

		if (!parentElement->hasAttribute(XMLString::transcode("name"))) {
			wclog << "Error: a property element (";
			wclog << XMLString::transcode(parentElement->getTagName());
			wclog << ") was declared without a name attribute." << endl;
			return NULL;
		}

		attributeName = XMLString::transcode(parentElement->getAttribute(
			    XMLString::transcode("name")));

		anchor = new PropertyAnchor(attributeName);
		if (parentElement->hasAttribute(XMLString::transcode("value"))) {
			attributeValue = XMLString::transcode(parentElement->
				    getAttribute(XMLString::transcode("value")));

			anchor->setPropertyValue(attributeValue);
		}
		return anchor;
	}

	void *NclInterfacesConverter::createArea(
		    DOMElement *parentElement, void *objGrandParent) {

		string anchorId;
		string position, anchorLabel;
		Anchor *anchor;

		if (!parentElement->hasAttribute(XMLString::transcode("id"))) {
			cout << "A media interface element (";
			cout << XMLString::transcode(parentElement->getTagName());
			cout << ") was declared without an id attribute" << endl;
			return NULL;
		}

		anchorId = XMLString::transcode(parentElement->getAttribute(
			    XMLString::transcode("id")));

		anchor = NULL;

		if (parentElement->hasAttribute(XMLString::transcode("begin")) ||
			    parentElement->hasAttribute(XMLString::transcode("end")) ||
			    parentElement->hasAttribute(XMLString::transcode("first")) ||
			    parentElement->hasAttribute(XMLString::transcode("last"))) {

	    	anchor = createTemporalAnchor(parentElement);

		// ancora textual
		} else if (parentElement->hasAttribute(XMLString::transcode("text"))) {
			position = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("position")));

			anchor = new TextAnchor(anchorId, XMLString::transcode(
				    parentElement->getAttribute(
				    	     XMLString::transcode("text"))),
				    	     atoi(position.c_str()));

		} else if (parentElement->hasAttribute(
			     XMLString::transcode("coords"))) {

			anchor = createSpatialAnchor(parentElement);

		} else if (parentElement->hasAttribute(
			    XMLString::transcode("label"))) {

			anchorLabel = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("label")));

			anchor = new LabeledAnchor(anchorId, anchorLabel);

		} else {
			anchor = new LabeledAnchor(anchorId, anchorId);
		}

		if (anchor == NULL) {
			cout << "NclInterfacesConverter::createArea Error" << endl;
			return NULL;
		}

		return anchor;
	}

	void *NclInterfacesConverter::createMapping(
		    DOMElement *parentElement, void *objGrandParent) {

		DOMElement *switchElement;
		SwitchNode *switchNode;
		NodeEntity *mappingNodeEntity;
		Node *mappingNode;
		InterfacePoint *interfacePoint;
		Port *port;

		switchElement = (DOMElement*)parentElement->getParentNode()->
			    getParentNode();

		switchNode = (SwitchNode*)((NclDocumentConverter*)getDocumentParser())
			    ->getNode(XMLString::transcode(
			    	    switchElement->getAttribute(
			    	    	    XMLString::transcode("id"))));

		mappingNode = switchNode->getNode(XMLString::transcode(
			    parentElement->getAttribute(
			    	   XMLString::transcode("component"))));

		if (mappingNode == NULL) {
			wclog << "Error: a mapping element points to a node (";
			wclog << XMLString::transcode(parentElement->getAttribute(XMLString::transcode("component")));
			wclog << " not contained by the " << switchNode->getId().c_str();
			wclog << " switch" << endl;
			return NULL;
		}

		mappingNodeEntity = (NodeEntity*) mappingNode->getDataEntity();
		if (parentElement->hasAttribute(XMLString::transcode("interface"))) {
			// tem-se o atributo, tentar pegar a ancora do document node
			interfacePoint = mappingNodeEntity->getAnchor(XMLString::transcode(
				      parentElement->getAttribute(
				      	    XMLString::transcode("interface"))));

			if (interfacePoint == NULL) {
				// ou o document node era um terminal node e nao
				// possuia a ancora (erro), ou port indicava uma porta em uma
				// composicao
				if (mappingNodeEntity->instanceOf("CompositeNode")) {
					interfacePoint = ((CompositeNode*)mappingNodeEntity)->
						    getPort(XMLString::transcode(parentElement->
						    	    getAttribute(XMLString::transcode(
						    	    	    "interface"))));
				}
			}

		} else {
			// port element nao tem o atributo port, logo pegar a
			// ancora lambda do no->
			interfacePoint = mappingNodeEntity->getAnchor(0);
		}

		if (interfacePoint == NULL) {
			wclog << "Error: a mapping element points to a node interface (";
			wclog << XMLString::transcode(parentElement->getAttribute(XMLString::transcode("interface")));
			wclog << " not contained by the " << mappingNode->getId().c_str();
			wclog << " node." << endl;
			return NULL;
		}

		port = new Port(((SwitchPort*) objGrandParent)->getId(), mappingNode,
			     interfacePoint);

		return port;
	}

	void *NclInterfacesConverter::createSwitchPort(
		     DOMElement *parentElement, void *objGrandParent) {

		SwitchNode *switchNode;
		SwitchPort *switchPort;
		string id;

		switchNode = (SwitchNode*) objGrandParent;

		if (!parentElement->hasAttribute(XMLString::transcode("id"))) {
			wclog << "Error: the switch port element was declared without an";
			wclog << " id attribute." << endl;
			return NULL;
		}

		id = XMLString::transcode(parentElement->getAttribute(
			    XMLString::transcode("id")));

		if (switchNode->getPort(id) != NULL) {
			wclog << "Error: a port already exists with the same ";
			wclog << id.c_str() << " id in ";
			wclog << switchNode->getId().c_str() << " context" << endl;
			return NULL;
		}

		switchPort = new SwitchPort(id, switchNode);
		return switchPort;
	}

	void NclInterfacesConverter::addMappingToSwitchPort(
		    void *parentObject, void *childObject) {

		((SwitchPort*) parentObject)->addPort((Port*) childObject);
	}
}
}
}
}
}
