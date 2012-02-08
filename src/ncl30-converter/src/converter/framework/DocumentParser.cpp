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

#include "../../../include/framework/DocumentParser.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace converter {
namespace framework {
	DocumentParser::DocumentParser() {
		genericTable = new map<string, void*>;
		//initialize();
		//setDependencies();
		documentTree = NULL;
		xmlParser = NULL;
	}

	DocumentParser::~DocumentParser() {
		if (genericTable != NULL) {
			genericTable->clear();
			map<string, void*>* table;
			map<string, void*>::iterator i;
			i = genericTable->begin();
			while (i != genericTable->end()) {
				table = (map<string, void*>*)(i->second);
				if (table != NULL) {
					table->clear();
					delete table;
					table = NULL;
				}
				++i;
			}

			delete genericTable;
			genericTable = NULL;
		}

		if (documentTree != NULL) {
			delete documentTree;
			documentTree = NULL;
		}

		if (xmlParser != NULL){
            delete xmlParser;
            xmlParser = NULL;
		}
	}

	void* DocumentParser::parse(string uri) {
		DOMElement* rootElement;

		if (!isXmlStr(uri)) {
			if (SOFLAG == 1) {
				string::size_type pos;
				while (true) {
					pos = uri.find_first_of("\\");
					if (pos == string::npos)
						break;
					uri.replace(pos,1,"/");
				}
			}

			uri = absoluteFile(getDocumentPath(), uri);
		}

		if (xmlParser == NULL)
            xmlParser = new XMLParsing();

        if (documentTree != NULL) {
			delete documentTree;
			documentTree = NULL;
		}

		documentTree = xmlParser->parse(uri);

		if (documentTree==NULL) {
			wclog << "Error when parsing " << uri.c_str();
			wclog << " (file not found)" << endl;
			return NULL;
		}

		//elemento raiz
		rootElement = (DOMElement*)documentTree->getDocumentElement();


		void* retValue = parse(rootElement, uri);

		if (documentTree != NULL) {
			delete documentTree;
			documentTree = NULL;
		}
		return retValue;
	}

	void* DocumentParser::parse(DOMElement* rootElement, string uri) {
		if (!isXmlStr(uri)) {
			if (SOFLAG == 1) {
				string::size_type pos;
				while (true) {
					pos = uri.find_first_of("\\");
					if (pos == string::npos)
						break;
					uri.replace(pos,1,"/");
				}
			}
		}

		documentUri = uri;
		/*base_uri = uri_alloc((char*)(uri.c_str()), uri.length());
		wclog << "uri: " << uri << endl;
		wclog << "uri_all_path: " << uri_all_path(base_uri) << endl;*/
		//setDocumentPath( hncm::util::getPath(uri_all_path(base_uri)) + "/" );
		setDocumentPath(getPath(uri) + "/" );
		wclog << "PATH = [" << documentPath.c_str() << "]" << endl;

		//TODO: generalizar isso para qualquer numero de NCLs
		setDocumentPath(documentPath);

		//uri_free(base_uri);

		return parseRootElement(rootElement);
	}

	void DocumentParser::setDependencies() {

	}

	string DocumentParser::getAbsolutePath(string path) {
		//uri_t* newUrl = NULL;

		if (SOFLAG == 1) {
			string::size_type pos;
			while (true) {
				pos = path.find_first_of("\\");
				if (pos == string::npos)
					break;
				path.replace(pos,1,"/");
			}
		}

		/*try {
			newUrl = uri_alloc((char*)(path.c_str()), path.length());
		}
		catch (...) {
			try {
				wclog << "path: " << path << endl;
				path = documentPath + path;
				wclog << "doc + path: " << path << endl;
				newUrl = uri_alloc((char*)(path.c_str()), path.length());
			}
			catch (...) {

			}
		}*/
		path = documentPath + path;
		if (path.find_first_of("/") != 0)
			path = getCurrentPath() + path;

		/*string ret = uri_all_path(newUrl);*/
		return path.substr( 0, path.find_last_of('/') );
	}

	void DocumentParser::setAttributeAbsPath(
		    DOMElement* element, string attribute) {

		XMLCh *attributeXML = XMLString::transcode(attribute.c_str());
		if ( element->hasAttribute(attributeXML) ) {
			string path = XMLString::transcode(
				    element->getAttribute(attributeXML) );

			XMLCh *pathXML = XMLString::transcode(
				    getAbsolutePath(path).c_str() );

			element->setAttribute(attributeXML, pathXML);
		}
	}

	string DocumentParser::getDocumentPath() {
		return documentPath;
	}

	void DocumentParser::setDocumentPath(string path) {
		if (SOFLAG == 1) {
			string::size_type pos;
			while (true) {
				pos = path.find_first_of("\\");
				if (pos == string::npos)
					break;
				path.replace(pos,1,"/");
			}
		}
		documentPath = path;

		util::setDocumentPath(path);
	}

	string DocumentParser::getDocumentUri() {
	    return documentUri;
	}

	DOMDocument *DocumentParser::getDocumentTree() {
		return documentTree;
	}

	void DocumentParser::addObject(
		    string tableName, string key, void* value) {

		map<string,void*>* table;
		map<string,void*>::iterator i;
		for (i=genericTable->begin();i!=genericTable->end();++i) {
			if (i->first == tableName) {
				table = (map<string, void*>*)(i->second);
				(*table)[key] = value;
				return;
			}
		}

		table = new map<string,void*>;
		(*table)[key]=value;
		(*genericTable)[tableName] = table;
	}

	void* DocumentParser::getObject(string tableName, string key) {
		map<string,void*>* table = NULL;
		map<string,void*>::iterator i;
		for (i=genericTable->begin(); i!=genericTable->end(); i++) {
			if(i->first == tableName) {
				table = (map<string,void*>*)i->second;
				break;
			}
		}
		if(table==NULL) {
		    return NULL;
		}
		else {
			for(i=table->begin(); i!=table->end(); i++) {
				if(i->first == key) {
					return i->second;
				}
			}
			return NULL;
		}
	}

	map<string,void*>* DocumentParser::getTable(string tableName) {
		if (genericTable->find(tableName) != genericTable->end())
			return (map<string,void*>*)((*genericTable)[tableName]);
		else
			return NULL;
	}

	void DocumentParser::removeObject(string tableName, string key) {
		map<string,void*>* table = NULL;
		map<string,void*>::iterator i;
		for (i=genericTable->begin(); i!=genericTable->end(); i++) {
			if(i->first == tableName) {
				table = (map<string,void*>*)i->second;
				break;
			}
		}
		if(table==NULL) {
			wclog << "Erro no removeObject da DocumentParser" << endl;
		}
		else {
			for (i=table->begin(); i!=table->end(); i++) {
				if(i->first == key) {
					//delete i->second;
					table->erase(i);
					return;
				}
			}
		}
		wclog << "Erro no removeObject da DocumentParser ... chave ";
		wclog << key.c_str() << "nao encontrada!" << endl;
	}

	void DocumentParser::addObjectGrouped(
		    string tableName, string key, void* value) {

		map<string, void*> *table = NULL;
		vector<void*> *vec = NULL;
		map<string, void*>::iterator i;

		for(i=genericTable->begin(); i!=genericTable->end(); i++) {
			if(i->first == tableName) {
				table = (map<string, void*> *)i->second;
				break;
			}
		}

		if(table!=NULL) {
			for(i=table->begin(); i!=table->end(); i++) {
				if(i->first==key) {
					vec = (vector<void*>*)i->second;
					break;
				}
			}
			if(vec!=NULL) {
				vec->push_back(value);
			}
			else {
				vec = new vector<void*>;
				vec->push_back(value);
				(*table)[key] = vec;
			}
		}
		else {
			vec = new vector<void*>;
			table = new map<string, void*>;
			vec->push_back(value);
			(*table)[key] = vec;
			(*genericTable)[tableName] = table;
		}
	}

	bool DocumentParser::importDocument(
		    DocumentParser* parser, string docLocation) {

		string uri;

		if (!isAbsolutePath(docLocation)) {
			uri = absoluteFile(getDocumentPath(), docLocation);

		} else {
			uri = docLocation;
		}

		parser->parse(uri);

		return true;
	}
}
}
}
}
}
