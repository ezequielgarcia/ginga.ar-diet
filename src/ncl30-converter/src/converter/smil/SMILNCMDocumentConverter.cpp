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

#include "../../../include/smil/SMILNCMDocumentConverter.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace converter {
namespace smil {
	SmilNcmDocumentConverter::SmilNcmDocumentConverter(
			IDeviceLayout* deviceLayout) {

		this->deviceLayout = deviceLayout;
		this->autoId       = 1;
	}

	string SmilNcmDocumentConverter::getNextId() {
		string id = "_autoId", num;
		int n = autoId++;

		while(n) {
			num = (char)('0'+n%10) + num;
			n /= 10;
		}

		return id + num;
	}

	void *SmilNcmDocumentConverter::parseRootElement(DOMElement *rootElement) {
		string tagName;
		tagName = XMLString::transcode(rootElement->getTagName());

		wclog << "SmilNcmDocumentConverter::parseRootElement" << endl;

		if (tagName == "smil") {
			return parseSmil(rootElement, NULL);

		} else {
			wclog << "SmilNcmDocumentParser::parseRootElement Warning!";
			wclog << " Trying to parse a non NCL document. rootElement";
			wclog << "->getTagName = '" << tagName.c_str() << "'" << endl;
			return NULL;
		}
	}

	void *SmilNcmDocumentConverter::parseSmil(DOMElement *parentElement,
			                                 void *objGrandParent) {
		void *parentObject = NULL;
		DOMNodeList *elementNodeList;
		int i, size;
		DOMNode *node;
		void *elementObject = NULL;
		bool parseHeadOk = false;

		wclog << "SmilNcmDocumentConverter::parseSmil" << endl;

		parentObject = createSmil(parentElement, objGrandParent);
		if (parentObject == NULL) {
			wclog << "SmilNcmDocumentConverter::parseSmil return NULL" << endl;
			return NULL;
		}

		elementNodeList = parentElement->getChildNodes();
		size = elementNodeList->getLength();

		for (i = 0; i < size; i++) {
			node = elementNodeList->item(i);
			if (node->getNodeType()==DOMNode::ELEMENT_NODE &&
				    XMLString::compareIString(((DOMElement*)node)->
				    	    getTagName(), XMLString::transcode("head") )==0) {

				elementObject = parseHead((DOMElement*)node, parentObject);
				parseHeadOk = true;
				if (elementObject != NULL) {
					//addHeadToSmil(parentObject, elementObject);
					break;
				}
			}
		}

		if(!parseHeadOk) {
			parseHead(NULL, parentObject);
			parseHeadOk = true;
		}

		for (i = 0; i < size; i++) {
			node = elementNodeList->item(i);
			if (node->getNodeType()==DOMNode::ELEMENT_NODE &&
				    XMLString::compareIString(((DOMElement*)node)->
				    	    getTagName(), XMLString::transcode("body") )==0) {

				elementObject = parseBody((DOMElement*)node, parentObject);
		      	if (elementObject != NULL) {
					//posCompileBody((DOMElement*)node, elementObject);
					//addBodyToSmil(parentObject, elementObject);
					break;
				}
			}
		}

		return parentObject;
	}

	void *SmilNcmDocumentConverter::createSmil(DOMElement *parentElement,
			                                  void *objGrandParent) {
		string docName;
		NclDocument *document;

		if(parentElement->hasAttribute(XMLString::transcode("id"))) {
			docName = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("id")));
		}

		if (docName == "") {
			docName = "smilDocument";
		}

		document = new NclDocument(docName);
		addObject("return", "document", document);
		return document;
	}

	void *SmilNcmDocumentConverter::parseHead(DOMElement *parentElement,
			                                 void *objGrandParent) {
		NclDocument *document;
		RegionBase *rBase;
		LayoutRegion *region;

		wclog << "SmilNcmDocumentConverter::parseHead" << endl;

		document = (NclDocument*)getObject("return", "document");

		rBase = new RegionBase("__rgBase", deviceLayout);
		region = new LayoutRegion("_fullScreen");
		region->setLeft(0, false);
		region->setTop(0, false);
		region->setWidth(100, true);
		region->setHeight(100, true);
		rBase->addRegion(region);
		document->addRegionBase( rBase );

		DescriptorBase *dBase;
		Descriptor *descriptor;
		dBase = new DescriptorBase("_dsBase");
		descriptor = new Descriptor("_dsFullScreen");
		descriptor->setRegion(region);
		dBase->addDescriptor(descriptor);
		document->setDescriptorBase( dBase );

		ConnectorBase *connBase;
		CausalConnector *conn;

		connBase = new ConnectorBase("_connBase");

		ConditionExpression *onEnd = new SimpleCondition("onEnd");
		Action *start = new SimpleAction("start");
		((Role*)start)->setMaxCon(Role::UNBOUNDED);
		conn = new CausalConnector("onEndStart", onEnd, start);
		connBase->addConnector(conn);

		ConditionExpression *onBegin = new SimpleCondition("onBegin");;
		start = new SimpleAction("start");
		((Role*)start)->setMaxCon(Role::UNBOUNDED);
		conn = new CausalConnector("onBeginStart", onBegin, start);
		connBase->addConnector(conn);
		document->setConnectorBase(connBase);

		return NULL;
	}

	void *SmilNcmDocumentConverter::createHead(DOMElement *parentElement,
			                                  void *objGrandParent) {
		return parentElement;
	}

	void *SmilNcmDocumentConverter::parseBody(DOMElement *parentElement,
			                                 void *objGrandParent) {
		void *parentObject = NULL;
		DOMNodeList *elementNodeList;
		int i, size;
		DOMNode *node;
		void *elementObject = NULL;
		bool firstChildNode = true;

		wclog << "SmilNcmDocumentConverter::parseBody" << endl;

		parentObject = createBody(parentElement, objGrandParent);
		if (parentObject == NULL) {
			wclog << "SmilNcmDocumentConverter::parseBody return NULL" << endl;
			return NULL;
		}

		elementNodeList = parentElement->getChildNodes();
		size = elementNodeList->getLength();

		for (i = 0; i < size; i++) {
			node = elementNodeList->item(i);
			if (node->getNodeType()==DOMNode::ELEMENT_NODE) {

				elementObject = NULL;

				if(XMLString::compareIString(((DOMElement*)node)->
					      getTagName(), XMLString::transcode("seq") )==0) {
					elementObject = parseSeq((DOMElement*)node, parentObject);
				}
				else if(XMLString::compareIString(((DOMElement*)node)->
					      getTagName(), XMLString::transcode("par") )==0) {
					elementObject = parsePar((DOMElement*)node, parentObject);
				}
				else if(XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("text") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("img") )==0 ||
	               XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("audio") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("video") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("ref") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("textstream") )==0 ||
	                   XMLString::compareIString(((DOMElement*)node)->
					      getTagName(), XMLString::transcode("animation") )==0
				  ) {

					wclog << "SmilNcmDocumentConverter::parseBody foundMedia"
					      << endl;

					elementObject = parseMedia((DOMElement*)node, parentObject);
				}

				if (elementObject != NULL) {
					((ContextNode*)parentObject)->
					                  addNode((Node*)elementObject);

					if(firstChildNode) {
						wclog << "SmilNcmDocumentConverter::parseBody port";
						string pId =
							"p" + ((ContextNode*)parentObject)->getId();
						Node *portNode = (Node*)elementObject;

						wclog << "...pId = " << pId.c_str();

						NodeEntity *portNodeEntity;
						portNodeEntity = (NodeEntity*)
						                  portNode->getDataEntity();

						if(portNode==NULL)
							wclog << " ... portNode=NULL!!!";
						if(portNodeEntity->getAnchor(0)==NULL)
							wclog << " ... anchor=NULL!!!";
						wclog << endl;

						Port *port = new Port(pId, portNode,
								portNodeEntity->getAnchor(0));
						((ContextNode*)parentObject)->addPort(port);
						firstChildNode = false;
					}
					wclog <<
					"SmilNcmDocumentConverter::parseBody elementObject!=NULL"
					      << endl;
				}

			}

		}

		wclog << "SmilNcmDocumentConverter::parseBody final" << endl;

		return posCompileBody(parentElement, parentObject);
	}

	void *SmilNcmDocumentConverter::createBody(DOMElement *parentElement,
			                                  void *objGrandParent) {
		// criar composicao a partir do elemento body do documento ncl
		// fazer uso do nome da composicao body que foi atribuido pelo
		// compilador
		NclDocument *document;
		ContextNode *context;

		document = (NclDocument*) getObject("return", "document");

		if (!parentElement->hasAttribute(XMLString::transcode("id"))) {
			parentElement->setAttribute(XMLString::transcode("id"),
				    XMLString::transcode(document->getId().c_str()));

			context=(ContextNode*)createContext(parentElement, objGrandParent);

			parentElement->removeAttribute(XMLString::transcode("id"));

		} else {
			context=(ContextNode*)createContext(parentElement, objGrandParent);

		}

		return context;
	}

	void *SmilNcmDocumentConverter::posCompileBody(DOMElement *parentElement,
												  void *parentObject) {
		wclog << "SmilNcmDocumentConverter::posCompileBody" << endl;
		NclDocument *document;
		document = (NclDocument*) getObject("return", "document");

		if(document==NULL) wclog << "document NULL!!!" << endl;

		document->setBody( (ContextNode*)parentObject );

		vector<Node*> *nodes = ((CompositeNode*)parentObject)->getNodes();

		Connector*conn=document->getConnectorBase()->getConnector("onEndStart");

		wclog << "SmilNcmDocumentConverter::posCompileBody connectorId="
		      << conn->getId().c_str() << endl;

		for(int i=0; i<(int)nodes->size()-1; i++) {
			Node *cond = (*nodes)[i];
			Node *act = (*nodes)[i+1];
			CausalLink *link = new CausalLink(getNextId(), conn);

			wclog << "SmilNcmDocumentConverter::posCompileBody i=" << i
			      << " condId = " << cond->getId().c_str() << " actId = "
			      << act->getId().c_str() << endl;

			/*
			anchorNodeEntity = (NodeEntity*)cond->getDataEntity();
			InterfacePoint *interfCond = anchorNodeEntity->getAnchor(0);
			link->bind(cond, interfCond, NULL, "onEnd");

			anchorNodeEntity = (NodeEntity*)act->getDataEntity();
			InterfacePoint *interfAct = anchorNodeEntity->getAnchor(0);
			link->bind(act, interfAct, NULL, "start");
			*/

			InterfacePoint *interfCond = cond->getAnchor(0);
			link->bind(cond, interfCond, NULL, "onEnd");

			InterfacePoint *interfAct = act->getAnchor(0);
			link->bind(act, interfAct, NULL, "start");

			((ContextNode*)parentObject)->addLink(link);
		}

		return parentObject;
	}

	void *SmilNcmDocumentConverter::createContext(
			DOMElement *parentElement,
			void *objGrandParent) {

		string id, attValue;
		ContextNode *context;

		if (!parentElement->hasAttribute(XMLString::transcode("id"))) {
			/*
			wclog << "A context element was declared";
		for (i = 0; i < size; i++) {
			node = elementNodeList->item(i);
			if (node->getNodeType()==DOMNode::ELEMENT_NODE &&
				    XMLString::compareIString(((DOMElement*)node)->
				    	    getTagName(), XMLString::transcode("body") )==0) {

				elementObject = parseBody((DOMElement*)node, parentObject);
		      	if (elementObject != NULL) {
					//posCompileBody((DOMElement*)node, elementObject);
					//addBodyToSmil(parentObject, elementObject);
					break;
				}
			}
		}                          void *objGrandParent) {
		string id;
		ContentNode *media;



			wclog << " without an id attribute." << endl;
			return NULL;
			*/
			id = getNextId();
		}
		else {
			id = XMLString::transcode(parentElement->getAttribute(
					XMLString::transcode("id")) );
		}

		/*
		node = ((NclDocumentConverter*)getDocumentParser())->getNode(id);
		if (node != NULL) {
			wclog << "There is another node element previously declared with";
			wclog << " the same " << id.c_str() << " id." << endl;
			return NULL;
		}
		*/

		//retornar nova composicao ncm a partir do elemento xml que a
		//representa em NCL
		context = new ContextNode(id);

		return context;
	}

	void *SmilNcmDocumentConverter::parseMedia(DOMElement *parentElement,
			                                 void *objGrandParent) {
		void *parentObject = NULL;

		wclog << "SmilNcmDocumentConverter::parseMedia" << endl;

		parentObject = createMedia(parentElement, objGrandParent);
		return parentObject;
	}

	void *SmilNcmDocumentConverter::createMedia(DOMElement *parentElement,
			                                   void *objGrandParent) {
		string id;
		ContentNode *media;
		GenericDescriptor *descriptor;
		NclDocument *document;

		if (!parentElement->hasAttribute(XMLString::transcode("id"))) {
			id = getNextId();
		}
		else {
			id = XMLString::transcode(parentElement->getAttribute(
					XMLString::transcode("id")) );
		}
		media = new ContentNode(id, NULL, "");

		if (parentElement->hasAttribute(XMLString::transcode("src"))) {
			string src = XMLString::transcode(parentElement->getAttribute(
				    XMLString::transcode("src")));

			if (src != "") {
				if (isAbsolutePath(src)) {
					((ContentNode*)media)->setContent(
						     new AbsoluteReferenceContent(src));

				} else {
					((ContentNode*)media)->setContent(
							new AbsoluteReferenceContent(
									getDocumentPath() +
											"/" + src));
				}
			}
		}

		if( XMLString::compareIString(
				XMLString::transcode(parentElement->getTagName()),
				"audio")
		  ) {
			document = (NclDocument*) getObject("return", "document");
			descriptor = document->getDescriptor("_dsFullScreen");
			media->setDescriptor(descriptor);
		}
		return media;
	}

	void *SmilNcmDocumentConverter::parseSeq(DOMElement *parentElement,
			                                void *objGrandParent) {
		void *parentObject = NULL;
		DOMNodeList *elementNodeList;
		int i, size;
		DOMNode *node;
		void *elementObject = NULL;
		bool firstChildNode = true;

		wclog << "SmilNcmDocumentConverter::parseSeq" << endl;

		parentObject = createContext(parentElement, objGrandParent);
		if (parentObject == NULL) {
			wclog << "SmilNcmDocumentConverter::parseSeq return NULL" << endl;
			return NULL;
		}

		elementNodeList = parentElement->getChildNodes();
		size = elementNodeList->getLength();

		for (i = 0; i < size; i++) {
			node = elementNodeList->item(i);

			if (node->getNodeType()==DOMNode::ELEMENT_NODE) {
				elementObject = NULL;

				if(XMLString::compareIString(((DOMElement*)node)->
					      getTagName(), XMLString::transcode("seq") )==0) {
					elementObject = parseSeq((DOMElement*)node, parentObject);
				}
				else if(XMLString::compareIString(((DOMElement*)node)->
					      getTagName(), XMLString::transcode("par") )==0) {
					elementObject = parsePar((DOMElement*)node, parentObject);
				}
				else if(XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("text") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("img") )==0 ||
	               XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("audio") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("video") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("ref") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("textstream") )==0 ||
	               XMLString::compareIString(((DOMElement*)node)->
					  getTagName(), XMLString::transcode("animation") )==0

				  ) {

					wclog << "SmilNcmDocumentConverter::parseSeq foundMedia"
					      << endl;

					elementObject = parseMedia((DOMElement*)node, parentObject);
				}
				if (elementObject != NULL) {
					((ContextNode*)parentObject)->
        		                   addNode((Node*)elementObject);

					if(firstChildNode) {
						wclog << "SmilNcmDocumentConverter::parseSeq port";
						string pId =
							"p" + ((ContextNode*)parentObject)->getId();
						Node *portNode = (Node*)elementObject;

						wclog << "...pId = " << pId.c_str();

						NodeEntity *portNodeEntity;
						portNodeEntity = (NodeEntity*)
						                  portNode->getDataEntity();

						if(portNode==NULL)
							wclog << " ... portNode=NULL!!!";
						if(portNodeEntity->getAnchor(0)==NULL)
							wclog << " ... anchor=NULL!!!";
						wclog << endl;

						Port *port = new Port(pId, portNode,
								portNodeEntity->getAnchor(0));
						((ContextNode*)parentObject)->addPort(port);
						firstChildNode = false;
					}
					wclog <<
					"SmilNcmDocumentConverter::parseBody elementObject!=NULL"
					      << endl;
				} else {
					wclog << "SmilNcmDocumentConverter::parseBody";
					wclog << " elementObject == NULL" << endl;
				}
			}
		}

		//posCompile: adding links
		Node *cond, *act;
		CausalLink *link;
		vector<Node*> *nodes = ((CompositeNode*)parentObject)->getNodes();
		NclDocument *document = (NclDocument*) getObject("return", "document");
		Connector*conn=document->getConnectorBase()->getConnector("onEndStart");

		for(int i=0; i<(int)nodes->size()-1; i++) {
			cond = (*nodes)[i];
			act = (*nodes)[i+1];
			link = new CausalLink(getNextId(), conn);

			wclog << "SmilNcmDocumentConverter::posCompileSeq i=" << i
			      << " condId = " << cond->getId().c_str() << " actId = "
			      << act->getId().c_str() << endl;

			InterfacePoint *interfCond = cond->getAnchor(0);
			link->bind(cond, interfCond, NULL, "onEnd");

			InterfacePoint *interfAct = act->getAnchor(0);
			link->bind(act, interfAct, NULL, "start");


			((ContextNode*)parentObject)->addLink(link);
		}

		return parentObject;

	}

	void *SmilNcmDocumentConverter::parsePar(DOMElement *parentElement,
			                                void *objGrandParent) {
		void *parentObject = NULL;
		DOMNodeList *elementNodeList;
		int i, size;
		DOMNode *node;
		void *elementObject = NULL;
		int pCount = 1;

		wclog << "SmilNcmDocumentConverter::parsePar" << endl;

		parentObject = createContext(parentElement, objGrandParent);
		if (parentObject == NULL) {
			wclog << "SmilNcmDocumentConverter::parsePar return NULL" << endl;
			return NULL;
		}

		elementNodeList = parentElement->getChildNodes();
		size = elementNodeList->getLength();

		for (i = 0; i < size; i++) {
			node = elementNodeList->item(i);

			if (node->getNodeType()==DOMNode::ELEMENT_NODE) {
				elementObject = NULL;

				if(XMLString::compareIString(((DOMElement*)node)->
					      getTagName(), XMLString::transcode("seq") )==0) {
					elementObject = parseSeq((DOMElement*)node, parentObject);
				}
				else if(XMLString::compareIString(((DOMElement*)node)->
					      getTagName(), XMLString::transcode("par") )==0) {
					elementObject = parsePar((DOMElement*)node, parentObject);
				}
				else if(XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("text") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("img") )==0 ||
	               XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("audio") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("video") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("ref") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("textstream") )==0 ||
                   XMLString::compareIString(((DOMElement*)node)->
				      getTagName(), XMLString::transcode("animation") )==0
				  ) {

					wclog << "SmilNcmDocumentConverter::parsePar foundMedia"
					      << endl;

					elementObject = parseMedia((DOMElement*)node, parentObject);
				}
				if (elementObject != NULL) {
					((ContextNode*)parentObject)->
	    		                   addNode((Node*)elementObject);

					wclog << "SmilNcmDocumentConverter::parsePar port";

					string nCount;
					int n = pCount++;
					while(n) {
						nCount = (char)(n%10+'0') + nCount;
						n /= 10;
					}

					string pId ="p"+ ((ContextNode*)parentObject)->getId() + nCount;
					Node *portNode = (Node*)elementObject;

					wclog << "...pId = " << pId.c_str();

					NodeEntity *portNodeEntity;
					portNodeEntity = (NodeEntity*)
					                  portNode->getDataEntity();

					if(portNode==NULL)
						wclog << " ... portNode=NULL!!!";
					if(portNodeEntity->getAnchor(0)==NULL)
						wclog << " ... anchor=NULL!!!";
					wclog << endl;

					Port *port = new Port(pId, portNode,
							              portNodeEntity->getAnchor(0));
					((ContextNode*)parentObject)->addPort(port);

					wclog <<
					"SmilNcmDocumentConverter::parseBody elementObject!=NULL"
					      << endl;
				}
			}
		}

		return parentObject;
	}

}
}
}
}
}
