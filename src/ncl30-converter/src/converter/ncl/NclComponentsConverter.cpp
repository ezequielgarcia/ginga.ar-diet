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

#include "../../../include/ncl/NclComponentsConverter.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace converter {
namespace ncl {
	NclComponentsConverter::NclComponentsConverter(
		    DocumentParser *documentParser) :
		    	    NclComponentsParser(documentParser) {


	}

	void NclComponentsConverter::addPortToContext(
		    void *parentObject, void *childObject) {

		if (((Entity*)parentObject)->instanceOf("ContextNode")) {
			((ContextNode*)parentObject)->addPort((Port*)childObject);
		}
	}

	void NclComponentsConverter::addPropertyToContext(
		    void *parentObject, void *childObject) {

		if (((Entity*)parentObject)->instanceOf("ContextNode")) {
			((ContextNode*)parentObject)->addAnchor((Anchor*)childObject);

		} else if (((Entity*)parentObject)->instanceOf("ReferNode")) {
			((ReferNode*)parentObject)->addAnchor((Anchor*)childObject);
		}
	}

	void NclComponentsConverter::addContextToContext(
		    void *parentObject, void *childObject) {

		if (((Entity*)parentObject)->instanceOf("ContextNode")) {
			//adicionar composicao aa composicao
			addNodeToContext(
				    (ContextNode*)parentObject, (NodeEntity*)childObject);
		}
	}

	void NclComponentsConverter::addMediaToContext(
		    void *parentObject, void *childObject) {

		if (((Entity*)parentObject)->instanceOf("ContextNode")) {
			//adicionar composicao aa composicao
			addNodeToContext(
				    (ContextNode*)parentObject, (NodeEntity*)childObject);
		}
	}

	void NclComponentsConverter::addLinkToContext(
		    void *parentObject, void *childObject) {

		int min;
		int max;
		Role* role;
		vector<Role*>::iterator i;

		if (((Entity*)parentObject)->instanceOf("ContextNode")) {
			vector<Role*>* roles;

			roles = ((Link*)childObject)->getConnector()->getRoles();
			if (roles != NULL) {
				i = roles->begin();
				while (i != roles->end()) {
					role = *i;
					//if (role->instanteOf("CardinalityRole") {
					min = role->getMinCon();
					max = role->getMaxCon();

					/*} else {
						min = 1;
						max = 1;
					}*/

					if (((Link*)childObject)->getNumRoleBinds(role) <
						    (unsigned int)min) {

						cout << "NclComponentsConverter::addLinkToContext";
						cout << " role " << role->getLabel().c_str();
						cout << " with less than minimum binds." << endl;
						return;

					} else if (max > 0 && ((Link*)childObject)->
						    getNumRoleBinds(role) > (unsigned int)max) {

						cout << "NclComponentsConverter::addLinkToContext";
						cout << " role " << role->getLabel().c_str();
						cout << " with more than maximum binds." << endl;
						delete roles;
						return;
					}
					++i;
				}
				delete roles;
			}

			((ContextNode*)parentObject)->addLink((Link*)childObject);
		}
	}

	void NclComponentsConverter::addNodeToContext(
		    ContextNode *contextNode, NodeEntity *node) {

		//adicionar um no' de midia aa composicao
		contextNode->addNode(node);
	}

	void NclComponentsConverter::addAnchorToMedia(
		    ContentNode *contentNode, Anchor *anchor) {

		if (contentNode->getAnchor(anchor->getId()) != NULL) {
			wclog << "There is another interface (area, attribute, etc.)";
			wclog << " with the same id (" << anchor->getId().c_str();
			wclog << ") defined for the" << contentNode->getId().c_str();
			wclog << " media element." << endl;

		} else {
			contentNode->addAnchor(anchor);
		}
	}

	void NclComponentsConverter::addAreaToMedia(
		    void *parentObject, void *childObject) {

		addAnchorToMedia((ContentNode*)parentObject, (Anchor*)childObject);
	}

	void NclComponentsConverter::addPropertyToMedia(
		    void *parentObject, void *childObject) {

		addAnchorToMedia((ContentNode*)parentObject, (Anchor*)childObject);
	}

	void NclComponentsConverter::addSwitchToContext(
		    void *parentObject, void *childObject) {

		((ContextNode*)parentObject)->addNode((Node*)childObject);
	}

	void *NclComponentsConverter::createContext(
		     DOMElement *parentElement, void *objGrandParent) {

		NclDocument *document;
		string id, attValue;
		Node *node;
		Entity *referNode;
		ContextNode *context;
		GenericDescriptor *descriptor;

		if (!parentElement->hasAttribute(XMLString::transcode("id"))) {
			wclog << "A context element was declared";
			wclog << " without an id attribute." << endl;
			return NULL;
		}

		id = XMLString::transcode(parentElement->getAttribute(
			    XMLString::transcode("id")) );

		node = ((NclDocumentConverter*)getDocumentParser())->getNode(id);
		if (node != NULL) {
			wclog << "There is another node element previously declared with";
			wclog << " the same " << id.c_str() << " id." << endl;
			return NULL;
		}

		if (parentElement->hasAttribute(XMLString::transcode("refer"))) {
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("refer")));

			try {
				referNode = (ContextNode*)((NclDocumentConverter*)
					    getDocumentParser() )->getNode(attValue);

				if (referNode == NULL) {
					document = (NclDocument*)(getDocumentParser()->
						    getObject("return", "document"));

					referNode = (ContextNode*)(document->getNode(attValue));
					if (referNode == NULL) {
						referNode = (Entity*)(new reuse::ReferredNode(
								attValue, (void*)parentElement));
					}
				}
			}
			catch (...) {
				wclog << "The context element refers to " << attValue.c_str();
				wclog << " object, which is not a context element." << endl;
				return NULL;
			}

			node = new ReferNode(id);
			((ReferNode*)node)->setReferredEntity(referNode);

			//((NclDocumentConverter*)getDocumentParser())->
				     //addNode((NodeEntity*)node);

			return node;
		}

		//retornar nova composicao ncm a partir do elemento xml que a
		//representa em NCL
		context = new ContextNode(id);

		if (parentElement->hasAttribute(XMLString::transcode("descriptor"))) {
			// adicionar um descritor a um objeto de midia
			attValue = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("descriptor")));

			document = (NclDocument*)(getDocumentParser()->getObject(
				    "return", "document"));

			descriptor = document->getDescriptor(attValue);
			if (descriptor != NULL) {
				context->setDescriptor(descriptor);

			} else {
				wclog << "The context element with " << id.c_str();
				wclog << " id attribute refers to a descriptor (";
				wclog << attValue.c_str() << ") that" << " does not exist.";
				wclog << endl;
				return NULL;
			}
		}

		return context;
	}

	void *NclComponentsConverter::posCompileContext(
		    DOMElement *parentElement, void *parentObject) {

		wclog << "posCompileContext" << endl;
		DOMNodeList *elementNodeList;
		int i, size;
		DOMNode *node;
		DOMElement *element;
		string elementTagName;
		void *elementObject;

		elementNodeList = parentElement->getChildNodes();
		size = elementNodeList->getLength();

		for (i = 0; i < size; i++) {
			node = elementNodeList->item(i);
			if (node->getNodeType() == DOMNode::ELEMENT_NODE) {
				element = (DOMElement*)node;
				elementTagName = XMLString::transcode(element->getTagName());
				if (XMLString::compareIString(elementTagName.c_str(),
					    "context") == 0) {

					elementObject = ((NclDocumentConverter*)
						    getDocumentParser())->getNode(XMLString::transcode(
								    element->getAttribute(XMLString::transcode(
								    	    "id"))));

					try {
						if (((NodeEntity*)elementObject)->instanceOf(
							    "ContextNode")) {

							posCompileContext(element, elementObject);
						}

					} catch(...) {
						//treating error with the <NodeEntity*> casting
					}

				} else if (XMLString::compareIString(elementTagName.c_str(),
					    "switch") == 0) {

					elementObject = ((NclDocumentConverter*)
						    getDocumentParser())->getNode(XMLString::transcode(
						    	    element->getAttribute(XMLString::transcode(
						    	    	    "id"))));

					try {
						if (((NodeEntity*)elementObject)->
							    instanceOf("SwitchNode")) {

							getPresentationControlParser()->posCompileSwitch(
								    element, elementObject);
						}

					} catch(...) {
						//treating error with the <NodeEntity*> casting
					}
				}
			}
		}

		return NclComponentsParser::posCompileContext(
			    parentElement, parentObject);
	}

	void *NclComponentsConverter::createMedia(
		    DOMElement *parentElement, void *objGrandParent) {

		string attValue, id;
		NclDocument *document;
		Node *node;
		Entity *referNode;
		GenericDescriptor *descriptor;
		XMLCh* tmpStr;
		char* tmpChar;

		tmpStr = XMLString::transcode("id");
		if (!parentElement->hasAttribute(tmpStr)) {
			wclog << "Error: a media element was declared without an id";
			wclog << " attribute." << endl;
			XMLString::release(&tmpStr);
			return NULL;
		}

		tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
		id = string(tmpChar);
		XMLString::release(&tmpChar);
		XMLString::release(&tmpStr);
		
		node = ((NclDocumentConverter*)getDocumentParser())->getNode(id);
		if (node != NULL) {
			wclog << "There is another node element previously declared";
			wclog << " with the same " << id.c_str() << " id." << endl;
			return NULL;
		}
      
		tmpStr = XMLString::transcode("refer");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			attValue = string(tmpChar);
			XMLString::release(&tmpChar);

			try {
				referNode = (ContentNode*)((NclDocumentConverter*)
					    getDocumentParser())->getNode(attValue);

				if (referNode == NULL) {
					//TODO: verificar se faz a mesma coisa da linha anterior
					document = (NclDocument*)(getDocumentParser()->
						    getObject("return", "document"));

					referNode = (ContentNode*)document->getNode(attValue);
					if (referNode == NULL) {
						referNode = new reuse::ReferredNode(
								attValue, (void*)parentElement);
					}
				}

			} catch (...) {
				wclog << "The media element refers to ";
				wclog << attValue.c_str();
				wclog << " object, which is not a media element." << endl;
				return NULL;
			}

			node = new ReferNode(id);
			((ReferNode*)node)->setReferredEntity(referNode);
			XMLCh* attribute = XMLString::transcode("instance");
			if (parentElement->hasAttribute(attribute)) {
				char* attValueChar = XMLString::transcode(parentElement->getAttribute(attribute));
				attValue = string(attValueChar);
				XMLString::release(&attValueChar);

				((ReferNode*)node)->setInstanceType(attValue);
			}
			XMLString::release(&attribute);

			return node;
		}
		XMLString::release(&tmpStr);

		node = new ContentNode(id, NULL, "");

		// type of media object
		tmpStr = XMLString::transcode("type");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			((ContentNode*)node)->setNodeType(tmpChar);
			XMLString::release(&tmpChar);
		}
		XMLString::release(&tmpStr);

		tmpStr = XMLString::transcode("src");
		if (parentElement->hasAttribute(tmpStr)) {
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			string src = string(tmpChar);
			XMLString::release(&tmpChar);

			if (src != "") {
				if (isAbsolutePath(src)) {
					((ContentNode*)node)->setContent(
						     new AbsoluteReferenceContent(src));

				} else {
					((ContentNode*)node)->setContent(
							new AbsoluteReferenceContent(
									getDocumentParser()->getDocumentPath() +
											"/" + src));
				}
			}
		}
		XMLString::release(&tmpStr);

		//testar duracao implicita do no' de midia
	    /*
	     if (parentElement.hasAttribute("implicitDur")) {
	     String durStr = parentElement.getAttribute("implicitDur");
	     durStr = durStr.substring(0, durStr.length() - 1);
	     //converter duracao para double
	     long dur = (long)((new Double(durStr)).doubleValue() * 1000);
	     //recuperar ancora lambda do no' de midia
	     ILambdaAnchor anchor = (ILambdaAnchor)node.getAnchor(0);
	     //cast
	     IIntervalRegion region = (IIntervalRegion)anchor.getRegion();
	     //atribuir duracao implicita
	     region.setEnd(dur);
	     }
	     */

		tmpStr = XMLString::transcode("descriptor");
		if (parentElement->hasAttribute(tmpStr)) {
			// adicionar um descritor a um objeto de midia
			tmpChar = XMLString::transcode(parentElement->getAttribute(tmpStr));
			attValue = string(tmpChar);
			XMLString::release(&tmpChar);

			document = (NclDocument*)getDocumentParser()->getObject(
				    "return", "document");

			descriptor = document->getDescriptor(attValue);
			if (descriptor != NULL) {
				((ContentNode*)node)->setDescriptor(descriptor);

			} else {
				wclog << "The media element with " << id.c_str();
				wclog << " id attribute refers to a descriptor (";
				wclog << attValue.c_str() << ") that does not exist." << endl;
				delete node;
				node = NULL;
				return NULL;
			}
		}
		XMLString::release(&tmpStr);

		//((NclDocumentConverter*)getDocumentParser())->addNode((NodeEntity*)node);

		//retornar no' de midia
		return node;
	}
}
}
}
}
}
