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

#include "../include/FormatterMediator.h"

#include "generated/config.h"

#if HAVE_COMPSUPPORT
#include "cm/IComponentManager.h"
using namespace ::br::pucrio::telemidia::ginga::core::cm;
#else
#include <system/io/LocalDeviceManager.h>
#include <system/io/GingaLocatorFactory.h>
#include "../include/prefetch/PrefetchManager.h"
#include "../include/multidevice/FormatterBaseDevice.h"
#endif

#include "../include/gfx/GingaNclGfx.h"
#include "../include/editingcommandevents.h"

#include <system/io/IGingaLocatorFactory.h>
#include <multidevice/services/IDeviceDomain.h>



namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
   // using namespace ::br::pucrio::telemidia::ginga::lssm;
	static bool handlerFormatter = true;

#if HAVE_COMPSUPPORT
	static IComponentManager* cm = IComponentManager::getCMInstance();
#endif

	PrivateBaseManager* FormatterMediator::privateBaseManager = NULL;
	IPrefetchManager* FormatterMediator::pm = NULL;

	FormatterMediator::FormatterMediator(NclPlayerData* pData) : Player("") {

		string deviceName = "systemScreen(" + itos(pData->devClass) + ")";

		this->data            = pData;
		this->currentFile     = "";
		this->currentDocument = NULL;

		printData(data);

		if (pm == NULL) {
#if HAVE_COMPSUPPORT
			pm = ((PrefetchManagerCreator*)(
					cm->getObject("PrefetchManager")))();
#else
			pm = PrefetchManager::getInstance();
#endif
		}

		deviceLayout = new DeviceLayout(deviceName);
		deviceLayout->addDevice(deviceName, data->x, data->y, data->w, data->h);
		if (data->enableGfx) {
			GingaNclGfx::show(data->x, data->y, data->w, data->h);
		}

#if HAVE_COMPSUPPORT
		multiDevice = ((FormatterMultiDeviceCreator*)(cm->getObject(
				"FormatterMultiDevice")))(
						deviceLayout,
						IDeviceDomain::CT_BASE,
						data->x, data->y, data->w, data->h);

#else
		multiDevice = new FormatterBaseDevice(
				deviceLayout,
				data->x, data->y, data->w, data->h);
#endif

		playerManager   = new PlayerAdapterManager(data);
		compileErrors   = NULL;

		isEmbedded      = data->parentDocId != "";
		ruleAdapter     = new RuleAdapter();
		compiler        = new FormatterConverter(ruleAdapter);

		scheduler       = new FormatterScheduler(
			    playerManager, ruleAdapter, multiDevice, compiler);

		scheduler->addSchedulerListener(this);
		compiler->setScheduler(scheduler);

		if (data->focusManager != NULL) {
			scheduler->getFocusManager()->setParent(
					(FormatterFocusManager*)(data->focusManager));
		}

		data->focusManager = scheduler->getFocusManager();

		if (handlerFormatter) {
			handlerFormatter = false;
			scheduler->setKeyHandler(true);
		}

		documentEvents      = new map<string, FormatterEvent*>;
		documentEntryEvents = new map<string, vector<FormatterEvent*>*>;

		if (privateBaseManager == NULL) {
			privateBaseManager = (PrivateBaseManager*)(
					PrivateBaseManager::getInstance());
		}

		privateBaseManager->createPrivateBase(data->baseId);
		playerManager->setNclEditListener((INclEditListener*)this);

		cout << "[FormatterMediator] created. "<< endl;
	}

	FormatterMediator::~FormatterMediator() {
		string docId = "";
		ExecutionObject* bodyObject;

		//PresentationContext::getInstance()->save();
		cout << "[FormatterMediator] released."<< endl;

		if (scheduler != NULL) {
			scheduler->removeSchedulerListener(this);
		}

		if (currentDocument != NULL) {
			docId = currentDocument->getId();
			if (compiler != NULL) {
				bodyObject = compiler->getObjectFromNodeId(docId);
				if (bodyObject != NULL) {
					compiler->removeExecutionObject(bodyObject);
				}
			}
			removeDocument(docId);
			currentDocument = NULL;
		}

		if (playerManager != NULL) {
			delete playerManager;
			playerManager = NULL;
		}

		if (documentEvents != NULL) {
			//TODO: check if we can delete events too
			delete documentEvents;
			documentEvents = NULL;
		}

		if (documentEntryEvents != NULL) {
			//TODO: check if we can delete events too
			delete documentEntryEvents;
			documentEntryEvents = NULL;
		}

		if (ruleAdapter != NULL) {
			delete ruleAdapter;
			ruleAdapter = NULL;
		}

		if (scheduler != NULL) {
			delete scheduler;
			scheduler = NULL;
		}

		if (compiler != NULL) {
			delete compiler;
			compiler = NULL;
		}

		if (data != NULL) {
			delete data;
			data = NULL;
		}

		if (compileErrors != NULL) {
			delete compileErrors;
			compileErrors = NULL;
		}
		if (multiDevice != NULL) {
                        delete multiDevice;
                        multiDevice = NULL;
                }
        }

	void FormatterMediator::release() {
		if (pm != NULL) {
			pm->release();
			delete pm;
			pm = NULL;
		}
	}

	void FormatterMediator::printData(NclPlayerData* data) {
	/*	cout << "FormatterMediator::printData(" << this << ")";
		cout << " baseId: " << data->baseId;
		cout << " playerId: " << data->playerId;
		cout << " devClass: " << data->devClass<<endl;
		cout << " parentDocId: " << data->parentDocId;
		cout << " docId: " << data->docId;
		cout << endl;*/
		
		cout <<"[FormatterMediator] screen information,";
		cout << " x: " << data->x;
		cout << " y: " << data->y;
		cout << " w: " << data->w;
		cout << " h: " << data->h;
		cout << endl;
	}

	void FormatterMediator::setStandByState(bool standBy) {
		setVisible(!standBy);
		if (standBy) {
			pause();

		} else {
			resume();
		}

		scheduler->setStandByState(standBy);
		playerManager->setStandByState(standBy);
	}

	void FormatterMediator::setTimeBaseProvider(
			ITimeBaseProvider* timeBaseProvider) {

		playerManager->setTimeBaseProvider(timeBaseProvider);
	}

	void FormatterMediator::setBackgroundImage(string uri) {
		cout << endl << endl;
		cout << "FormatterMediator::setBackgroundImage '";
		cout << uri << "'" << endl;
		cout << endl << endl;

		multiDevice->setBackgroundImage(uri);
	}

	void FormatterMediator::setParentLayout(void* parentLayout) {
		FormatterLayout* mainLayout;

		cout << "FormatterMediator::setParentLayout in '" << data->playerId;
		cout << "'" << endl;

		if (multiDevice != NULL && parentLayout != NULL &&
				multiDevice->getMainLayout() != NULL) {

			mainLayout = (FormatterLayout*)(multiDevice->getMainLayout());
			((FormatterLayout*)parentLayout)->addChild(
					data->baseId, mainLayout);
		}
	}

	string FormatterMediator::getScreenShot() {
		return multiDevice->getScreenShot();
	}

	vector<string>* FormatterMediator::getCompileErrors() {
		if (compileErrors == NULL) {
			return new vector<string>;

		} else {
			return compileErrors;
		}
	}

	void* FormatterMediator::setCurrentDocument(string fName) {
		vector<string>* uris;
		string baseUri;

		if (currentDocument != NULL) {
			cout << "FormatterMediator::setCurrentDocument currentDocument ";
			cout << "!= NULL";
			cout << endl;
			return NULL;
		}

		if (isAbsolutePath(fName)) {
			currentFile = fName;

		} else {
			currentFile = getCurrentPath() + fName;
		}

		if (currentFile.length() > 7 &&
				currentFile.substr(0, 7) == "http://") {

			if (pm == NULL) {
				if (data->enableGfx) {
					GingaNclGfx::release();
				}
				cout << "FormatterMediator::setCurrentDocument ";
				cout << "Warning! Can't solve remote NCL '" << currentFile;
				cout << "': NULL prefecthManager";
				cout << endl;
				return NULL;
			}

			if (!pm->hasIChannel()) {
				cout << "FormatterMediator::setCurrentDocument ";
				cout << "Warning! Can't solve remote NCL '" << currentFile;
				cout << "': no channels found in prefecthManager";
				cout << endl;
				return NULL;
			}

			currentFile = pm->createDocumentPrefetcher(currentFile);
			solveRemoteNclDeps(currentFile, true);

		} else {
			solveRemoteNclDeps(currentFile, false);
		}

		currentDocument = (NclDocument*)addDocument(currentFile);

		if (data->enableGfx) {
			GingaNclGfx::release();
		}

		if (currentDocument != NULL) {
			uris        = new vector<string>;
			baseUri     = getActiveUris(uris);

			std::string::size_type pos = currentFile.rfind("/");
			if (pos != string::npos) {
				baseUri = currentFile.substr(0, pos+1);

			} else {
				baseUri = "";
			}

			multiDevice->addActiveUris(baseUri, uris);
		}

		return currentDocument;
	}

	void* FormatterMediator::addDocument(string docLocation) {
		NclDocument* addedDoc = NULL;

		if (isEmbedded) {
			addedDoc = privateBaseManager->embedDocument(
					data->baseId,
					data->parentDocId,
					data->nodeId,
					docLocation,
					deviceLayout);

		} else {
			addedDoc = privateBaseManager->addDocument(
					data->baseId, docLocation, deviceLayout);
		}

		if (addedDoc == NULL) {
			cout << "FormatterMediator::addDocument Warning!";
			cout << " Can't add document for '";
			cout << docLocation << "'";
			cout << endl;
			return NULL;
		}

		data->docId = addedDoc->getId();

		if (prepareDocument(addedDoc->getId())) {
			return addedDoc;
		}

		cout << "FormatterMediator::addDocument Warning!";
		cout << " Can't prepare document for '";
		cout << docLocation << "'";
		cout << endl;

		return NULL;
	}

	bool FormatterMediator::removeDocument(string documentId) {
		NclDocument* document;

		if (documentEvents->count(documentId) != 0) {
			stopDocument(documentId);
		}

		if (isEmbedded && documentId == data->docId) {
			document = privateBaseManager->removeEmbeddedDocument(
					data->baseId,
					data->parentDocId,
					data->nodeId);

		} else {
			document = privateBaseManager->removeDocument(
					data->baseId, documentId);
		}

		if (document != NULL) {
			delete document;
			return true;

		} else {
			return false;
		}
	}

	ContextNode* FormatterMediator::getDocumentContext(string documentId) {
		NclDocument* nclDocument;


        map<string, FormatterEvent*>::iterator it = documentEvents->begin();
        for ( ; it != documentEvents->end() ; ++it)
            std::cout << "    " << (*it).first << std::endl;

		if (documentEvents->count(documentId) != 0) {
			return NULL;
		}

		if (isEmbedded && documentId == data->docId) {
			nclDocument = privateBaseManager->getEmbeddedDocument(
					data->baseId, data->parentDocId, data->nodeId);

		} else {
			nclDocument = privateBaseManager->getDocument(
					data->baseId, documentId);
		}

		if (nclDocument == NULL) {
			return NULL;
		}

		return nclDocument->getBody();
	}

	void FormatterMediator::setDepthLevel(int level) {
		compiler->setDepthLevel(level);
	}

	int FormatterMediator::getDepthLevel() {
		return compiler->getDepthLevel();
	}

	vector<FormatterEvent*>* FormatterMediator::processDocument(
		    string documentId, string interfaceId) {

		vector<FormatterEvent*>* entryEvents;
		vector<Port*>* ports;
		int i, size;
		ContextNode* context;
		Port* port;
		NodeNesting* contextPerspective;
		FormatterEvent* event;

		port = NULL;

		// look for the entry point perspective
		context = getDocumentContext(documentId);
		if (context == NULL) {
			// document has no body
			cout << "FormatterMediator::processDocument warning! Doc '";
			cout << documentId;
			cout << "': without body!" << endl;
			return NULL;
		}

		ports = new vector<Port*>;
		if (interfaceId == "") {
			size = context->getNumPorts();
			for (i = 0; i < size; i++) {
				port = context->getPort(i);
				if (port != NULL && port->getEndInterfacePoint() != NULL &&
					    port->getEndInterfacePoint()->
					    	    instanceOf("ContentAnchor")) {

					ports->push_back(port);
				}
			}

		} else {
			port = context->getPort(interfaceId);
			if (port != NULL) {
				ports->push_back(port);
			}
		}

		if (ports->empty()) {
			// interfaceId not found
			cout << "FormatterMediator::processDocument warning! Doc '";
			cout << documentId;
			cout << "': without interfaces" << endl;
			delete ports;
			return NULL;
		}

		contextPerspective = new NodeNesting(
			    privateBaseManager->getPrivateBase(data->baseId));

		contextPerspective->insertAnchorNode(context);

		entryEvents = new vector<FormatterEvent*>;
		size = ports->size();
		for (i = 0; i < size; i++) {
			port = (*ports)[i];
			event = compiler->insertContext(contextPerspective, port);
			if (event != NULL) {
				entryEvents->push_back(event);
			}
		}

		delete ports;

		if (entryEvents->empty()) {
			cout << "FormatterMediator::processDocument warning! Doc '";
			cout << documentId;
			cout << "': without entry events" << endl;
			return NULL;
		}

		return entryEvents;
	}

	vector<FormatterEvent*>* FormatterMediator::getDocumentEntryEvent(
		    string documentId) {

		if (documentEntryEvents->count(documentId) != 0) {
			return (*documentEntryEvents)[documentId];

		} else {
			return NULL;
		}
	}

	bool FormatterMediator::compileDocument(
			string documentId, string interfaceId) {

		vector<FormatterEvent*>* entryEvents;
		FormatterEvent* event;
		ExecutionObject* executionObject;
		CompositeExecutionObject* parentObject;
		FormatterEvent* documentEvent;

		if (documentEvents->count(documentId) != 0) {
			return true;
		}

		entryEvents = processDocument(documentId, interfaceId);
		if (entryEvents == NULL) {
			return false;
		}

		event = (*entryEvents)[0];
		executionObject = (ExecutionObject*)(event->getExecutionObject());
		parentObject = (CompositeExecutionObject*)(executionObject->
			    getParentObject());

		if (parentObject != NULL) {
			while (parentObject->getParentObject() != NULL) {
				executionObject = parentObject;
				parentObject    = (CompositeExecutionObject*)(parentObject->
					    getParentObject());
			}
			documentEvent = executionObject->getWholeContentPresentationEvent();

		} else {
			documentEvent = event;
		}

		(*documentEvents)[documentId]      = documentEvent;
		(*documentEntryEvents)[documentId] = entryEvents;

		return true;
	}

	bool FormatterMediator::prepareDocument(string documentId) {
		NclDocument* doc;
		string src, docLocation;
		bool isRemoteDoc;

		DescriptorBase* db;
		vector<GenericDescriptor*>* descs;

		ContextNode* body;
		vector<Node*>* nodes;

		body = getDocumentContext(documentId);
		if (body == NULL || pm == NULL) {
			// document has no body
			cout << "FormatterMediator::prepareDocument warning! Doc '";
			cout << documentId;
			cout << "': without body!" << endl;
			return false;
		}

		if (isEmbedded && documentId == data->docId) {
			docLocation = privateBaseManager->getEmbeddedDocumentLocation(
					data->baseId, data->parentDocId, data->nodeId);

		} else {
			docLocation = privateBaseManager->getDocumentLocation(
					data->baseId, documentId);
		}

		if (docLocation == "") {
			return false;
		}

		isRemoteDoc = pm->hasRemoteLocation(docLocation);

		//solving remote descriptors URIs
		if (isEmbedded && documentId == data->docId) {
			doc = privateBaseManager->getEmbeddedDocument(
					data->baseId, data->parentDocId, data->nodeId);

		} else {
			doc = privateBaseManager->getDocument(data->baseId, documentId);
		}

		if (doc != NULL) {
			db = doc->getDescriptorBase();
			if (db != NULL) {
				descs = db->getDescriptors();
				if (descs != NULL) {
					solveRemoteDescriptorsUris(docLocation, descs, isRemoteDoc);
				}
			}
		}

		//solving remote nodes URIs
		//cout << "FormatterMediator::prepareDocument '" << docLocation;
		//cout << "'" << endl;
		nodes = body->getNodes();
		if (nodes != NULL) {
			solveRemoteNodesUris(docLocation, nodes, isRemoteDoc);
		}

		if (pm != NULL) {
			pm->getScheduledContents();
		}

		return true;
	}

	void FormatterMediator::solveRemoteDescriptorsUris(string docLocation,
			vector<GenericDescriptor*>* descs, bool isRemoteDoc) {

		string src;
		GenericDescriptor* desc;
		vector<GenericDescriptor*>::iterator i;
		FocusDecoration* fd;

		i = descs->begin();
		while (i != descs->end()) {
			desc = *i;
			if (desc->instanceOf("Descriptor")) {
				fd = ((Descriptor*)desc)->getFocusDecoration();

			} else {
				fd = NULL;
			}

			if (fd != NULL) {
				src = fd->getFocusSelSrc();
				//cout << "FormatterMediator::solveRemoteDescriptorsUris old ";
				//cout << "focusSelSrc='" << src << "'" << endl;
				if (src != "") {
					src = solveRemoteSourceUri(docLocation, src);
					fd->setFocusSelSrc(src);
				}
				//cout << "FormatterMediator::solveRemoteDescriptorsUris new ";
				//cout << "focusSelSrc='" << src << "'" << endl;

				src = fd->getFocusSrc();
				//cout << "FormatterMediator::solveRemoteDescriptorsUris old ";
				//cout << "focusSrc='" << src << "'" << endl;
				if (src != "") {
					src = solveRemoteSourceUri(docLocation, src);
					fd->setFocusSrc(src);
				}
				//cout << "FormatterMediator::solveRemoteDescriptorsUris new ";
				//cout << "focusSrc='" << src << "'" << endl;
			}
			++i;
		}
	}

	void FormatterMediator::solveRemoteNodesUris(
			string docLocation, vector<Node*>* nodes, bool isRemoteDoc) {

		CompositeNode* node;
		ContextNode* parent;
		vector<Node*>* childs;
		vector<Node*>::iterator i;
		Content* content;
		string src, mime;
		string::size_type mimePos;

		i = nodes->begin();
		while (i != nodes->end()) {
			if ((*i)->instanceOf("ContextNode")) {
				parent = (ContextNode*)(*i);
				childs = parent->getNodes();
				if (childs != NULL) {
					solveRemoteNodesUris(docLocation, childs, isRemoteDoc);
				}

			} else if ((*i)->instanceOf("CompositeNode")) {
				node = (CompositeNode*)(*i);
				childs = node->getNodes();
				if (childs != NULL) {
					solveRemoteNodesUris(docLocation, childs, isRemoteDoc);
				}

			} else {
				content = ((NodeEntity*)((*i)->getDataEntity()))->getContent();
				if (content != NULL && content->instanceOf(
						"ReferenceContent")) {

					src = ((ReferenceContent*)content)->
							getCompleteReferenceUrl();

					if (src != "" && src.find(".") != std::string::npos) {
						mimePos = src.find_last_of(".") + 1;
						mime = src.substr(mimePos, src.length() - mimePos);

						if (mime.length() == 2 ||
								mime.length() > 3 ||
								mime == "com" || mime == "org" ||
								mime == "ht" || mime == "htm" ||
								mime == "html" || mime == "xhtml") {

							if (src.substr(0, 1) == "/") {
								src = updatePath(src);
							}

						} else if (mime == "lua") {
							src = updatePath(solveRemoteSourceUri(
									docLocation, updatePath(src)));

							solveRemoteLuaDeps(docLocation, src, isRemoteDoc);

						} else {
							src = updatePath(solveRemoteSourceUri(
									docLocation, src));
						}

						((ReferenceContent*)content)->setReference(src);
					}
				}
			}
			++i;
		}
	}

	void FormatterMediator::solveRemoteNclDeps(
			string docLocation, bool isRemoteDoc) {

		string docRoot, nclDep;
		ifstream fis;

		docRoot = docLocation.substr(0, docLocation.find_last_of("/"));
		fis.open(docLocation.c_str(), ifstream::in);
		if (!fis.is_open()) {
			cout << "FormatterMediator::solveRemoteNclDeps Warning!";
			cout << " Can't solve ncl";
			cout << " deps on '" << docLocation << "'" << endl;
			return;
		}

		while (fis.good()) {
			fis >> nclDep;
			if (nclDep.find("documentURI") != std::string::npos) {
				while (nclDep.find("\"") == std::string::npos && fis.good()) {
					fis >> nclDep;
				}

				if (nclDep.find("\"") != std::string::npos &&
						nclDep.find_first_of("\"") !=
							nclDep.find_last_of("\"")) {

					nclDep = nclDep.substr(
							nclDep.find_first_of("\"") + 1,
							nclDep.find_last_of("\"") -
							(nclDep.find_first_of("\"") + 1));

					if ((!isRemoteDoc &&
							nclDep.find("http://") == std::string::npos) ||
							nclDep == "") {

						continue;
					}

					nclDep = solveRemoteSourceUri(docLocation, nclDep);
					if (pm != NULL) {
						pm->getScheduledContent(nclDep);

					} else {
						cout << "FormatterMediator::solveRemoteNclDeps";
						cout << " Warning! ";
						cout << "PrefetchManager is NULL" << endl;
						break;
					}
				}
			}
		}

		fis.close();
	}

	void FormatterMediator::solveRemoteLuaDeps(
			string docLocation, string src, bool isRemoteDoc) {

		string clientLuaDepsSrc, clientLuaDepsRoot, luaDep;
		ifstream fis;

		if (!isRemoteDoc && src.find("http://") == std::string::npos) {
			return;
		}

		clientLuaDepsSrc = src.substr(0, src.find_last_of(".")) + ".deps";
		clientLuaDepsSrc = solveRemoteSourceUri(docLocation, clientLuaDepsSrc);
		clientLuaDepsRoot = clientLuaDepsSrc.substr(
				0, clientLuaDepsSrc.find_last_of("/"));

		if (pm != NULL) {
			pm->getScheduledContent(clientLuaDepsSrc);

		} else {
			cout << "FormatterMediator::solveRemoteLuaDeps";
			cout << " Warning! PrefetchManager ";
			cout << "is NULL" << endl;
		}

		fis.open(clientLuaDepsSrc.c_str(), ifstream::in);
		if (!fis.is_open()) {
			cout << "FormatterMediator::solveRemoteLuaDeps";
			cout << " Warning! Can't solve lua";
			cout << " deps on '" << clientLuaDepsSrc << "'" << endl;
			return;
		}

		while (fis.good()) {
			fis >> luaDep;
			luaDep = clientLuaDepsRoot + "/" + luaDep;
			solveRemoteSourceUri(docLocation, luaDep);
		}

		fis.close();
	}

	string FormatterMediator::solveRemoteSourceUri(
			string localDocUri, string src) {

		return pm->createSourcePrefetcher(localDocUri, src);
	}

	bool FormatterMediator::startDocument(
			string documentId, string interfaceId) {

		vector<FormatterEvent*>* entryEvents;
		FormatterEvent* documentEvent;

		cout << "FormatterMediator::startDocument docId: '" << documentId;
		cout << "', interfaceId: '" << interfaceId << "'." << endl;
		if (compileDocument(documentId, interfaceId)) {
			if (documentEvents->count(documentId) != 0) {
				if (data->enableGfx) {
					GingaNclGfx::release();
				}
				documentEvent = (*documentEvents)[documentId];
				entryEvents   = (*documentEntryEvents)[documentId];
				scheduler->startDocument(documentEvent, entryEvents);
				//cout << "FormatterMediator::startDocument return true";
				//cout << endl;
				return true;
			}
		}

		cout << "FormatterMediator::startDocument return false" << endl;
		if (data->enableGfx) {
			GingaNclGfx::release();
		}
		return false;
	}

	 bool FormatterMediator::stopDocument(string documentId) {
		FormatterEvent* documentEvent;

		if (pm != NULL) {
			pm->releaseContents();
		}

		if (documentEvents->count(documentId) == 0) {
			return false;
		}

		documentEvent = (*documentEvents)[documentId];
		scheduler->stopDocument(documentEvent);
		if (documentEvents != NULL && documentEvents->count(documentId) != 0) {
			documentEvents->erase(documentEvents->find(documentId));
		}

		if (documentEntryEvents != NULL) {
			if (documentEntryEvents->count(documentId) != 0) {
				documentEntryEvents->erase(
						documentEntryEvents->find(documentId));
			}
		}

		return true;
	}

	bool FormatterMediator::pauseDocument(string documentId) {
		FormatterEvent* documentEvent;

		if (documentEvents->count(documentId) == 0) {
			return false;
		}

		cout << "FormatterMediator::pauseDocument '" << documentId << "'";
		cout << endl;
		documentEvent = (*documentEvents)[documentId];
		scheduler->pauseDocument(documentEvent);
		return true;
	}

	bool FormatterMediator::resumeDocument(string documentId) {
		FormatterEvent* documentEvent;

		if (documentEvents->count(documentId) == 0) {
			return false;
		}

		cout << "FormatterMediator::resumeDocument '" << documentId << "'";
		cout << endl;
		documentEvent = (*documentEvents)[documentId];
		scheduler->resumeDocument(documentEvent);
		return true;
	}

	void FormatterMediator::presentationCompleted(
			IFormatterEvent* documentEvent) {

		/*string documentId = "";
		ExecutionObject* eventObject;
		Node* dataObject;*/

		cout << "FormatterMediator::presentationCompleted for '";
		cout << data->playerId;
		cout << "'" << endl;

		if (scheduler != NULL) {
			scheduler->removeSchedulerListener(this);
		}

/*		eventObject = ((ExecutionObject*)documentEvent->
			    getExecutionObject());

		if (eventObject != NULL) {
			dataObject = eventObject->getDataObject();

			if (dataObject != NULL) {
				documentId = dataObject->getId();
			}
		}*/

//		PresentationContext::getInstance()->save();

		if (pm != NULL) {
			pm->releaseContents();
		}

		documentEvent->removeEventListener((IEventListener*)this);

		/*
		if (documentEvents->count(documentId) != 0) {
			if (documentEvents != NULL && !documentEvents->empty()) {
				if (documentEvents->count(documentId) != 0) {
					documentEvents->erase(documentEvents->find(documentId));
				}
			}

			if (documentEntryEvents != NULL && !documentEntryEvents->empty()) {
				if (documentEntryEvents->count(documentId) != 0) {
					documentEntryEvents->erase(
							documentEntryEvents->find(documentId));
				}
			}
		}*/
		documentEvents->clear();
		documentEntryEvents->clear();


		Player::notifyListeners(
				PL_NOTIFY_STOP, data->playerId, TYPE_PRESENTATION);

	/*	if (scheduler != NULL) {
			delete scheduler;
			scheduler = NULL;
		} */
	}

/*	bool FormatterMediator::nclEdit(string nclEditApi) {
		string commandTag = trim(
				nclEditApi.substr(0, nclEditApi.find_first_of(",")));

		return editingCommand(commandTag, nclEditApi.substr(
				nclEditApi.find_first_of(",") + 1,
				nclEditApi.length() - (nclEditApi.find_first_of(",") + 1)));
	}
*/
    bool FormatterMediator::editingCommand(StartDocumentEC* ec)
    {
        cout << "[FormattterMediatopr] StartDocument" << endl;
        cout << "Document id = " << ec->getDocumentId() << endl;
        cout << "Interface id = " << ec->getInterfaceId() << endl;
	bool res;
        if (res = startDocument(ec->getDocumentId(), ec->getInterfaceId())) {
	    setKeyHandler(true);
        }
	else {
            cout << "FormatterMediator::editingCommand warning!";
            cout << " can't startDocument" << endl;
        }
        return res;
    }

    bool FormatterMediator::editingCommand(StopDocumentEC* ec)
    {
        return stopDocument(ec->getDocumentId());
    }

    bool FormatterMediator::editingCommand(AddLinkEC* ec)
    {
        Link* l = addLink(ec->getDocumentId(), ec->getCompositeId(), ec->getXmlLink());
        return l != NULL;
    }

    bool FormatterMediator::editingCommand(SetPropertyValueEC* ec)
    {
	return setPropertyValue(ec->getDocumentId(), ec->getNodeId(), ec->getPropertyId(), ec->getPropertyValue());
    }

    bool FormatterMediator::editingCommand(AddNodeEC* ec){
        Node* n = addNode(ec->getDocumentId(), ec->getCompositeId(), ec->getXmlNode());
        if (n != NULL) {
		cout << "FormatterMediator::editingCommand node added '";
		cout << n->getId() << "'" << endl;
	}
	else {
	    cout << "FormatterMediator::editingCommand warning! ";
            cout << "cant addnode doc='";
            cout << ec->getDocumentId() << "' compositeId='" << ec->getCompositeId();
            cout << endl;
	}
	return n != NULL;
    }

    bool FormatterMediator::editingCommand( AddInterfaceEC* ec)
    {
        InterfacePoint* intPoint;
        intPoint = addInterface(ec->getDocumentId(), ec->getNodeId(), ec->getXmlInterface());
        cout << "FormatterMediator::editingCommand addInterf doc='";
        cout << ec->getDocumentId() << "' nodeId='" << ec->getNodeId() << "' xml='";
        cout << ec->getXmlInterface() << "'" << endl;

        if (intPoint != NULL) {
            cout << "FormatterMediator::editingCommand ";
            cout << "interface added '";
            cout << intPoint->getId() << "'" << endl;

        } else {
            cout << "FormatterMediator::editingCommand warning! ";
            cout << "cant addInterf doc='";
            cout << ec->getDocumentId() << "' nodeId='" << ec->getNodeId() << "' xml='";
            cout << ec->getXmlInterface() << "'" << endl;
        }
        return intPoint != NULL;
    }
/*
	bool FormatterMediator::editingCommand(
			string commandTag, string privateDataPayload) {

		vector<string>* args;
		vector<string>::iterator i;
		string arg, uri, ior, docUri, docIor, uName, docId;
		IGingaLocatorFactory* glf = NULL;

#if HAVE_COMPSUPPORT
		glf = ((GingaLocatorFactoryCreator*)(cm->getObject(
				"GingaLocatorFactory")))();
#else
		glf = GingaLocatorFactory::getInstance();
#endif

		args = split(privateDataPayload, ",");
		i = args->begin();
		arg = *i;
		++i;

		cout << "FormatterMediator::editingCommand(" << commandTag << "): '";
		cout << args->size() << "'" << endl;

		//parse command
		if (commandTag == EC_START_DOCUMENT) {
			cout << "FormatterMediator::editingCommand startDocument" << endl;
			string interfaceId;
			docId = *i;
			++i;

			if (args->size() == 2) {
				if (startDocument(docId, "")) {
					setKeyHandler(true);
				}

			} else if (args->size() == 3) {
				interfaceId = *i;
				if (startDocument(docId, interfaceId)) {
					setKeyHandler(true);
				}

			} else {
				cout << "FormatterMediator::editingCommand warning!";
				cout << " can't startDocument";
				cout << " wrong number of params: " << args->size() << endl;
			}

		} else if (commandTag == EC_STOP_DOCUMENT) {
			stopDocument(*i);

		} else if (commandTag == EC_PAUSE_DOCUMENT) {
			pauseDocument(*i);

		} else if (commandTag == EC_RESUME_DOCUMENT) {
			resumeDocument(*i);

		} else if (commandTag == EC_ADD_REGION) {
			cout << "FormatterMediator::editingCommand addRegion" << endl;
			string regionId, regionBaseId, xmlRegion;
			LayoutRegion* region;

			if (args->size() == 5) {
				docId = *i;
				++i;
				regionBaseId = *i;
				++i;
				regionId = *i;
				++i;
				xmlRegion = *i;
				region = addRegion(docId, regionBaseId, regionId, xmlRegion);
				if (region != NULL) {
					cout << "FormatterMediator::editingCommand region added '";
					cout << region->getId() << "'" << endl;

				} else {
					cout << "FormatterMediator::editingCommand warning! ";
					cout << "cant addregion doc='";
					cout << docId << "' regionId='" << regionId << "' xml='";
					cout << xmlRegion << "'" << endl;
				}

			} else {
				cout << "FormatterMediator::editingCommand warning! ";
				cout << "can't addRegion";
				cout << " wrong number of params: " << args->size() << endl;
			}

		} else if (commandTag == EC_REMOVE_REGION) {
			string regionId, regionBaseId;

			if (args->size() == 4) {
				regionBaseId = *i;
				++i;
				docId = *i;
				++i;
				regionId = *i;
				removeRegion(docId, regionBaseId, regionId);
			}

		} else if (commandTag == EC_ADD_REGION_BASE) {
			string xmlRegionBase;
			RegionBase* regionBase;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlRegionBase = *i;
				regionBase = addRegionBase(docId, xmlRegionBase);
				if (regionBase != NULL) {
					cout << "FormatterMediator::editingCommand ";
					cout << "regionBase added '";
					cout << regionBase->getId() << "'" << endl;

				} else {
					cout << "FormatterMediator::editingCommand warning! ";
					cout << "cant addrgbase doc='";
					cout << docId << "' xml='" << xmlRegionBase << "'";
					cout << endl;
				}

			} else {
				cout << "FormatterMediator::editingCommand warning! ";
				cout << "can't addRegionBase";
				cout << " wrong number of params: " << args->size() << endl;
			}

		} else if (commandTag == EC_REMOVE_REGION_BASE) {
			string baseId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				baseId = *i;
				removeRegionBase(docId, baseId);
			}

		} else if (commandTag == EC_ADD_RULE) {
			string xmlRule;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlRule = *i;
				addRule(docId, xmlRule);
			}

		} else if (commandTag == EC_REMOVE_RULE) {
			string ruleId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				ruleId = *i;
				removeRule(docId, ruleId);
			}

		} else if (commandTag == EC_ADD_RULE_BASE) {
			string xmlRuleBase;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlRuleBase = *i;
				addRuleBase(docId, xmlRuleBase);
			}

		} else if (commandTag == EC_REMOVE_RULE_BASE) {
			string baseId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				baseId = *i;
				removeRuleBase(docId, baseId);
			}

		} else if (commandTag == EC_ADD_CONNECTOR) {
			string xmlConnector;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlConnector = *i;
				addConnector(docId, xmlConnector);
			}

		} else if (commandTag == EC_REMOVE_CONNECTOR) {
			string connectorId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				connectorId = *i;
				removeConnector(docId, connectorId);
			}

		} else if (commandTag == EC_ADD_CONNECTOR_BASE) {
			cout << "FormatterMediator::editingCommand addConnectorBase";
			cout << endl;
			string xmlConnectorBase;
			ConnectorBase* connBase;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlConnectorBase = *i;
				connBase = addConnectorBase(docId, xmlConnectorBase);
				if (connBase != NULL) {
					cout << "FormatterMediator::editingCommand ";
					cout << "connBase added '";
					cout << connBase->getId() << "'" << endl;
				} else {
					cout << "FormatterMediator::editingCommand warning! ";
					cout << "cant addconnBs doc='";
					cout << docId << "' xmlConnBs='" << xmlConnectorBase;
					cout << "'" << endl;
				}

			} else {
				cout << "FormatterMediator::editingCommand warning!";
				cout << " can't addConnBase";
				cout << " wrong number of params: " << args->size() << endl;
			}

		} else if (commandTag == EC_REMOVE_CONNECTOR_BASE) {
			string baseId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				baseId = *i;
				removeConnectorBase(docId, baseId);
			}

		} else if (commandTag == EC_ADD_DESCRIPTOR) {
			cout << "FormatterMediator::editingCommand addDescriptor" << endl;
			string xmlDescriptor;
			GenericDescriptor* desc;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlDescriptor = *i;
				desc = addDescriptor(docId, xmlDescriptor);
				if (desc != NULL) {
					cout << "FormatterMediator::editingCommand ";
					cout << "descriptor added '";
					cout << desc->getId() << "'" << endl;

				} else {
					cout << "FormatterMediator::editingCommand warning! ";
					cout << "cant addDesc doc='";
					cout << docId << "' xmlDesc='" << xmlDescriptor;
					cout << "'" << endl;
				}

			} else {
				cout << "FormatterMediator::editingCommand warning!";
				cout << " can't addDesc";
				cout << " wrong number of params: " << args->size() << endl;
			}

		} else if (commandTag == EC_REMOVE_DESCRIPTOR) {
			string descId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				descId = *i;
				removeDescriptor(docId, descId);
			}

		} else if (commandTag == EC_ADD_DESCRIPTOR_SWITCH) {
			string xmlDesc;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlDesc = *i;
				addDescriptor(docId, xmlDesc);
			}

		} else if (commandTag == EC_REMOVE_DESCRIPTOR_SWITCH) {
			string descId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				descId = *i;
				removeDescriptor(docId, descId);
			}

		} else if (commandTag == EC_ADD_DESCRIPTOR_BASE) {
			cout << "FormatterMediator::editingCommand addDescriptorBase";
			cout << endl;
			string xmlDescBase;
			DescriptorBase* dBase;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlDescBase = *i;
				dBase = addDescriptorBase(docId, xmlDescBase);
				if (dBase != NULL) {
					cout << "FormatterMediator::editingCommand dBase added '";
					cout << dBase->getId() << "'" << endl;
				} else {
					cout << "FormatterMediator::editingCommand warning! ";
					cout << "cant adddBase doc='";
					cout << docId << "' xmlDescBase='";
					cout << xmlDescBase << "'" << endl;
				}

			} else {
				cout << "FormatterMediator::editingCommand warning! ";
				cout << "can't addDescBase";
				cout << " wrong number of params: " << args->size() << endl;
			}

		} else if (commandTag == EC_REMOVE_DESCRIPTOR_BASE) {
			string baseId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				baseId = *i;
				removeDescriptorBase(docId, baseId);
			}

		} else if (commandTag == EC_ADD_TRANSITION) {
			string xmlTrans;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlTrans = *i;
				addTransition(docId, xmlTrans);
			}

		} else if (commandTag == EC_REMOVE_TRANSITION) {
			string transId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				transId = *i;
				removeTransition(docId, transId);
			}

		} else if (commandTag == EC_ADD_TRANSITION_BASE) {
			string xmlTransBase;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlTransBase = *i;
				addTransitionBase(docId, xmlTransBase);
			}

		} else if (commandTag == EC_REMOVE_TRANSITION_BASE) {
			string baseId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				baseId = *i;
				removeTransitionBase(docId, baseId);
			}

		} else if (commandTag == EC_ADD_IMPORT_BASE) {
			string docBaseId, xmlImportBase;

			if (args->size() == 4) {
				docId = *i;
				++i;
				docBaseId = *i;
				++i;
				xmlImportBase = *i;
				addImportBase(docId, docBaseId, xmlImportBase);
			}

		} else if (commandTag == EC_REMOVE_IMPORT_BASE) {
			string docBaseId;

			if (args->size() == 4) {
				docId = *i;
				++i;
				docBaseId = *i;
				++i;
				docUri = *i;
				removeImportBase(docId, docBaseId, docUri);
			}

		} else if (commandTag == EC_ADD_IMPORTED_DOCUMENT_BASE) {
			string xmlImportedBase;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlImportedBase = *i;
				addImportedDocumentBase(docId, xmlImportedBase);
			}

		} else if (commandTag == EC_REMOVE_IMPORTED_DOCUMENT_BASE) {
			string baseId;

			if (args->size() == 3) {
				docId = *i;
				++i;
				baseId = *i;
				removeImportedDocumentBase(docId, baseId);
			}

		} else if (commandTag == EC_ADD_IMPORT_NCL) {
			string xmlImportNcl;

			if (args->size() == 3) {
				docId = *i;
				++i;
				xmlImportNcl = *i;
				addImportNCL(docId, xmlImportNcl);
			}

		} else if (commandTag == EC_REMOVE_IMPORT_NCL) {
			if (args->size() == 3) {
				docId = *i;
				++i;
				docUri = *i;
				removeImportNCL(docId, docUri);
			}

		} else if (commandTag == EC_ADD_NODE) {
			cout << "FormatterMediator::editingCommand addNode" << endl;
			string compositeId, xmlNode;
			Node* n;

			if (args->size() == 4) {
				docId = *i;
				++i;
				compositeId = *i;
				++i;
				xmlNode = *i;
				n = addNode(docId, compositeId, xmlNode);
				if (n != NULL) {
					cout << "FormatterMediator::editingCommand node added '";
					cout << n->getId() << "'" << endl;
				} else {
					cout << "FormatterMediator::editingCommand warning! ";
					cout << "cant addnode doc='";
					cout << docId << "' compositeId='" << compositeId;
					cout << "' xmlNode='";
					cout << xmlNode << "'" << endl;
				}

			} else {
				cout << "FormatterMediator::editingCommand warning!";
				cout << "can't addNode";
				cout << " wrong number of params: " << args->size() << endl;
			}

		} else if (commandTag == EC_REMOVE_NODE) {
			string compositeId, nodeId;

			if (args->size() == 4) {
				docId = *i;
				++i;
				compositeId = *i;
				++i;
				nodeId = *i;
				removeNode(docId, compositeId, nodeId);
			}

		} else if (commandTag == EC_ADD_INTERFACE) {
			cout << "FormatterMediator::editingCommand addInterface" << endl;
			string nodeId, xmlInterface;
			InterfacePoint* intPoint;

			if (args->size() == 4) {
				docId = *i;
				++i;
				nodeId = *i;
				++i;
				xmlInterface = *i;
				intPoint = addInterface(docId, nodeId, xmlInterface);
				cout << "FormatterMediator::editingCommand addInterf doc='";
				cout << docId << "' nodeId='" << nodeId << "' xml='";
				cout << xmlInterface << "'" << endl;

				if (intPoint != NULL) {
					cout << "FormatterMediator::editingCommand ";
					cout << "interface added '";
					cout << intPoint->getId() << "'" << endl;

				} else {
					cout << "FormatterMediator::editingCommand warning! ";
					cout << "cant addInterf doc='";
					cout << docId << "' nodeId='" << nodeId << "' xml='";
					cout << xmlInterface << "'" << endl;
				}

			} else {
				cout << "FormatterMediator::editingCommand warning!";
				cout << " can't addInterface";
				cout << " wrong number of params: " << args->size() << endl;
			}

		} else if (commandTag == EC_REMOVE_INTERFACE) {
			string nodeId, interfaceId;

			if (args->size() == 4) {
				docId = *i;
				++i;
				nodeId = *i;
				++i;
				interfaceId = *i;
				removeInterface(docId, nodeId, interfaceId);
			}

		} else if (commandTag == EC_ADD_LINK) {
			std::cout << __FILE__ << __LINE__ << ": commandTag == EC_ADD_LINK!\n";
			string compositeId, xmlLink;

			if (args->size() == 4) {
				docId = *i;
				++i;
				compositeId = *i;
				++i;
				xmlLink = *i;
				std::cout << __FILE__ << __LINE__ << ": docId = " << docId << ", compositeId = " << compositeId << ", xmlLink = " << xmlLink << "\n";
				addLink(docId, compositeId, xmlLink);
			}

		} else if (commandTag == EC_REMOVE_LINK) {
			string compositeId, linkId;

			if (args->size() == 4) {
				docId = *i;
				++i;
				compositeId = *i;
				++i;
				linkId = *i;
				removeLink(docId, compositeId, linkId);
			}

		} else if (commandTag == EC_SET_PROPERTY_VALUE) {
			string nodeId, propId, value;

			if (args->size() == 5) {
				docId = *i;
				++i;
				nodeId = *i;
				++i;
				propId = *i;
				++i;
				value = *i;

				setPropertyValue(docId, nodeId, propId, value);
			}
		}

		delete args;
		args = NULL;
		return true;
	}
*/
	LayoutRegion* FormatterMediator::addRegion(
		    string documentId,
		    string regionBaseId, string regionId, string xmlRegion) {

		return privateBaseManager->addRegion(
				data->baseId,
				documentId, regionBaseId, regionId, xmlRegion);
	}

	LayoutRegion* FormatterMediator::removeRegion(
			string documentId, string regionBaseId, string regionId) {

		return privateBaseManager->removeRegion(
				data->baseId, documentId, regionBaseId, regionId);
	}

	RegionBase* FormatterMediator::addRegionBase(
		    string documentId, string xmlRegionBase) {

		return privateBaseManager->addRegionBase(
				data->baseId, documentId, xmlRegionBase);
	}

	RegionBase* FormatterMediator::removeRegionBase(
		    string documentId, string regionBaseId) {

		return privateBaseManager->removeRegionBase(
				data->baseId, documentId, regionBaseId);
	}

	Rule* FormatterMediator::addRule(string documentId, string xmlRule) {
		return privateBaseManager->addRule(
				data->baseId, documentId, xmlRule);
	}

	Rule* FormatterMediator::removeRule(string documentId, string ruleId) {
		return privateBaseManager->removeRule(
				data->baseId, documentId, ruleId);
	}

	RuleBase* FormatterMediator::addRuleBase(
			string documentId, string xmlRuleBase) {

		return privateBaseManager->addRuleBase(
				data->baseId, documentId, xmlRuleBase);
	}

	RuleBase* FormatterMediator::removeRuleBase(
			string documentId, string ruleBaseId) {

		return privateBaseManager->removeRuleBase(
				data->baseId, documentId, ruleBaseId);
	}

	Transition* FormatterMediator::addTransition(
		    string documentId, string xmlTransition) {

		return privateBaseManager->addTransition(
				data->baseId, documentId, xmlTransition);
	}

	Transition* FormatterMediator::removeTransition(
		    string documentId, string transitionId) {

		return privateBaseManager->removeTransition(
				data->baseId, documentId, transitionId);
	}

	TransitionBase* FormatterMediator::addTransitionBase(
		    string documentId, string xmlTransitionBase) {

		return privateBaseManager->addTransitionBase(
				data->baseId, documentId, xmlTransitionBase);
	}

	TransitionBase* FormatterMediator::removeTransitionBase(
		    string documentId, string transitionBaseId) {

		return privateBaseManager->removeTransitionBase(
				data->baseId, documentId, transitionBaseId);
	}

	Connector* FormatterMediator::addConnector(
			string documentId, string xmlConnector) {

		return privateBaseManager->addConnector(
				data->baseId, documentId, xmlConnector);
	}

	Connector* FormatterMediator::removeConnector(
		    string documentId, string connectorId) {

		return privateBaseManager->removeConnector(
				data->baseId, documentId, connectorId);
	}

	ConnectorBase* FormatterMediator::addConnectorBase(
		    string documentId, string xmlConnectorBase) {

		return privateBaseManager->addConnectorBase(
				data->baseId, documentId, xmlConnectorBase);
	}

	ConnectorBase* FormatterMediator::removeConnectorBase(
		    string documentId, string connectorBaseId) {

		return privateBaseManager->removeConnectorBase(
				data->baseId, documentId, connectorBaseId);
	}

	GenericDescriptor* FormatterMediator::addDescriptor(
		    string documentId, string xmlDescriptor) {

		return privateBaseManager->addDescriptor(
				data->baseId, documentId, xmlDescriptor);
	}

	GenericDescriptor* FormatterMediator::removeDescriptor(
		    string documentId, string descriptorId) {

		return privateBaseManager->removeDescriptor(
				data->baseId, documentId, descriptorId);
	}

	DescriptorBase* FormatterMediator::addDescriptorBase(
		    string documentId, string xmlDescriptorBase) {

		return privateBaseManager->addDescriptorBase(
				data->baseId, documentId, xmlDescriptorBase);
	}

	DescriptorBase* FormatterMediator::removeDescriptorBase(
		    string documentId, string descriptorBaseId) {

		return privateBaseManager->removeDescriptorBase(
				data->baseId, documentId, descriptorBaseId);
	}

	Base* FormatterMediator::addImportBase(
		    string documentId, string docBaseId, string xmlImportBase) {

		return privateBaseManager->addImportBase(
				data->baseId, documentId, docBaseId, xmlImportBase);
	}

	Base* FormatterMediator::removeImportBase(
		    string documentId, string docBaseId, string documentURI) {

		return privateBaseManager->removeImportBase(
				data->baseId, documentId, docBaseId, documentURI);
	}

	NclDocument* FormatterMediator::addImportedDocumentBase(
		    string documentId, string xmlImportedDocumentBase) {

		return privateBaseManager->addImportedDocumentBase(
				data->baseId, documentId, xmlImportedDocumentBase);
	}

	NclDocument* FormatterMediator::removeImportedDocumentBase(
		    string documentId, string importedDocumentBaseId) {

		return privateBaseManager->removeImportedDocumentBase(
				data->baseId, documentId, importedDocumentBaseId);
	}

	NclDocument* FormatterMediator::addImportNCL(
		    string documentId, string xmlImportNCL) {

		return privateBaseManager->addImportNCL(
				data->baseId, documentId, xmlImportNCL);
	}

	NclDocument* FormatterMediator::removeImportNCL(
		    string documentId, string documentURI) {

		return privateBaseManager->removeImportNCL(
				data->baseId, documentId, documentURI);
	}

	void FormatterMediator::processInsertedReferNode(ReferNode* referNode) {
		NodeEntity* nodeEntity;
		ExecutionObject* executionObject;
		CompositeExecutionObject* parentObject;
		int depthLevel;
		string instanceType;

		nodeEntity = (NodeEntity*)(referNode->getDataEntity());
		instanceType = referNode->getInstanceType();
		if (nodeEntity->instanceOf("ContentNode") &&
				instanceType == "instSame") {

			executionObject = compiler->hasExecutionObject(
					nodeEntity, NULL);
			if (executionObject != NULL) {
				parentObject = compiler->addSameInstance(
					    executionObject, referNode);

				if (parentObject != NULL) {
					depthLevel = compiler->getDepthLevel();
					if (depthLevel > 0) {
						depthLevel = depthLevel - 1;
					}

					compiler->compileExecutionObjectLinks(
						    executionObject,
						    referNode,
						    parentObject,
						    depthLevel);
				}

			} else {
				cout << "FormatterMediator::processInsertedReferNode referred object";
				cout << " is NULL for '";
				cout << nodeEntity->getId() << "' instType = '";
				cout << instanceType << "'" << endl;
			}

		} else {
			cout << "FormatterMediator::processInsertedReferNode referred object";
			cout << " for '" << nodeEntity->getId() << "' instType = '";
			cout << instanceType << "'" << endl;
		}
	}

	void FormatterMediator::processInsertedComposition(CompositeNode* composition) {
		vector<Node*>* nodes;

		nodes = composition->getNodes();
		if (nodes != NULL) {
			vector<Node*>::iterator i;
			i = nodes->begin();
			while (i != nodes->begin()) {
				if ((*i)->instanceOf("ReferNode")) {
					processInsertedReferNode((ReferNode*)(*i));

				} else if ((*i)->instanceOf("CompositeNode")) {
					processInsertedComposition((CompositeNode*)(*i));
				}
			}
		}
	}

	Node* FormatterMediator::addNode(
		    string documentId, string compositeId, string xmlNode) {

		Node* node;

		node = privateBaseManager->addNode(
				data->baseId, documentId, compositeId, xmlNode);

		if (node == NULL) {
			return NULL;
		}

		if (node->instanceOf("ReferNode")) {
			processInsertedReferNode((ReferNode*)node);

		} else if (node->instanceOf("CompositeNode")) {
			// look for child nodes with refer and newInstance=false
			processInsertedComposition((CompositeNode*)node);
		}

		return node;
	}

	Node* FormatterMediator::removeNode(
		    string documentId, string compositeId, string nodeId) {

		NclDocument* document;

		document = privateBaseManager->getDocument(
				data->baseId, documentId);

		if (document == NULL) {
			// document does not exist
			return NULL;
		}

		Node* parentNode;

		parentNode = document->getNode(compositeId);
		if (parentNode == NULL || !(parentNode->instanceOf("ContextNode"))) {
			// composite node (compositeId) does exist or is not a context node
			return NULL;
		}

		Node* node;
		CompositeNode* compositeNode;

		compositeNode = (ContextNode*)parentNode;
		node = compositeNode->getNode(nodeId);
		if (node == NULL) {
			// node (nodeId) is not a compositeId child node
			return NULL;
		}

		ExecutionObject* executionObject;
		vector<Anchor*>* nodeInterfaces;
		vector<Anchor*>* anchors;
		vector<Anchor*>::iterator it;
		vector<Port*>* ports;
		vector<Port*>::iterator j;
		int i, size;
		InterfacePoint* nodeInterface;

		// remove all node interfaces
		nodeInterfaces = new vector<Anchor*>;

		anchors = node->getAnchors();
		if (anchors != NULL) {
			it = anchors->begin();
			while (it != anchors->end()) {
				nodeInterfaces->push_back(*it);
				++it;
			}
		}

		if (node->instanceOf("CompositeNode")) {
			ports = ((CompositeNode*)node)->getPorts();
			if (ports != NULL) {
				j = ports->begin();
				while (j != ports->end()) {
					nodeInterfaces->push_back((Anchor*)(*j));
				}
			}
		}

		size = nodeInterfaces->size();
		for (i = 0; i < size; i++) {
			nodeInterface = (*nodeInterfaces)[i];
			removeInterface(node, nodeInterface);
		}

		// remove the execution object
		executionObject = compiler->hasExecutionObject(
				node, NULL);

		if (executionObject != NULL) {
			if (node->instanceOf("ReferNode") &&
				    ((ReferNode*)node)->getInstanceType() != "new") {

				// remove the object entry
				compiler->removeExecutionObject(
					    executionObject, (ReferNode*)node);

			} else {
				// remove the whole execution object
				if (executionObject->getMainEvent() != NULL) {
					scheduler->stopEvent(executionObject->getMainEvent());
				}
				compiler->removeExecutionObject(
						executionObject);
			}
		}

		compositeNode->removeNode(node);
		return node;
	}

	InterfacePoint* FormatterMediator::addInterface(
		    string documentId, string nodeId, string xmlInterface) {

		return privateBaseManager->addInterface(
				data->baseId, documentId, nodeId, xmlInterface);
	}

	void FormatterMediator::removeInterfaceMappings(
		    Node* node,
		    InterfacePoint* interfacePoint,
		    CompositeNode* composition) {

		vector<Port*>* portsToBeRemoved;
		vector<Port*>* ports;
		vector<Port*>* mappings;
		vector<Port*>::iterator j, k;

		int i, size;
		Port* port;
		Port* mapping;

		portsToBeRemoved = new vector<Port*>;

		ports = composition->getPorts();
		if (ports != NULL) {
			j = ports->begin();
			while (j != ports->end()) {
				port = *j;

				if (port->instanceOf("SwitchPort")) {
					mappings = ((SwitchPort*)port)->getPorts();
					if (mappings != NULL) {
						k = mappings->begin();
						while (k != mappings->end()) {
							mapping = (Port*)(*k);
							if (mapping->getNode() == node &&
								    mapping->getInterfacePoint() ==
								    interfacePoint) {

								portsToBeRemoved->push_back(port);
								break;
							}
							++k;
						}
					}

				} else if (port->getNode() == node &&
							port->getInterfacePoint() == interfacePoint) {

					portsToBeRemoved->push_back(port);
				}
				++j;
			}
		}

		size = portsToBeRemoved->size();
		for (i = 0; i < size; i++) {
			port = (Port*)((*portsToBeRemoved)[i]);
			removeInterface(composition, port);
		}
	}

	void FormatterMediator::removeInterfaceLinks(
		    Node* node,
		    InterfacePoint* interfacePoint,
		    LinkComposition* composition) {

		vector<Link*>* linksToBeRemoved;
		vector<Link*>* links;
		vector<Link*>::iterator j;
		vector<Bind*>* binds;
		vector<Bind*>::iterator k;

		Link* link;
		Bind* bind;
		int i, size;

		linksToBeRemoved = new vector<Link*>;
		links = composition->getLinks();
		if (links != NULL) {
			j = links->begin();
			while (j != links->end()) {
				link = (Link*)(*j);
				// verify if node and interface point participate in link
				binds = link->getBinds();
				if (binds != NULL) {
					k = binds->begin();
					while (k != binds->end()) {
						bind = (Bind*)(*k);
						if (bind->getNode() == node &&
								bind->getInterfacePoint() == interfacePoint) {

							linksToBeRemoved->push_back(link);
							break;
						}
						++k;
					}
				}
				++j;
			}
		}

		size = linksToBeRemoved->size();
		for (i = 0; i < size; i++) {
			link = (Link*)((*linksToBeRemoved)[i]);
			removeLink(composition, link);
		}
	}

	void FormatterMediator::removeInterface(
		    Node* node, InterfacePoint* interfacePoint) {

		CompositeNode* parentNode;

		parentNode = (CompositeNode*)(node->getParentComposition());
		removeInterfaceMappings(node, interfacePoint, parentNode);

		if (parentNode != NULL && parentNode->instanceOf("LinkComposition")) {
			removeInterfaceLinks(
				    node, interfacePoint, (LinkComposition*)parentNode);
		}

		if (interfacePoint->instanceOf("Anchor")) {
			node->removeAnchor((Anchor*)interfacePoint);

		} else {
			((CompositeNode*)node)->removePort((Port*)interfacePoint);
			//TODO verify if a special treatment is necessary for switch ports
		}
	}

	InterfacePoint* FormatterMediator::removeInterface(
		    string documentId, string nodeId, string interfaceId) {

		NclDocument* document;
		Node* node;
		InterfacePoint* interfacePoint;

		document = privateBaseManager->getDocument(
				data->baseId, documentId);

		if (document == NULL) {
			// document does not exist
			return NULL;
		}

		node = document->getNode(nodeId);
		if (node == NULL) {
			// node (nodeId) does not exist
			return NULL;
		}

		interfacePoint = node->getAnchor(interfaceId);
		if (interfacePoint == NULL && node->instanceOf("CompositeNode")) {
			interfacePoint = ((CompositeNode*)node)->getPort(interfaceId);
		}

		if (interfacePoint == NULL) {
			// interface (interfaceId) does not exist or
			// does not pertain to node
			return NULL;
		}

		removeInterface(node, interfacePoint);
		return interfacePoint;
	}

	Link* FormatterMediator::addLink(
		    string documentId, string compositeId, string xmlLink) {

		Link* link;
		NclDocument* document;
		ContextNode* contextNode;

		link = privateBaseManager->addLink(
				data->baseId, documentId, compositeId, xmlLink);

		if (link != NULL) {
			document = privateBaseManager->getDocument(
					data->baseId, documentId);

			contextNode = (ContextNode*)(document->getNode(compositeId));
			if (link->instanceOf("CausalLink")) {
				std::cout << __FILE__ << __LINE__ << ": link->instanceOf(CausalLink) es true...\n";
				compiler->addCausalLink(
						contextNode, (CausalLink*)link);
			} else {
				std::cout << __FILE__ << __LINE__ << ": link->instanceOf(CausalLink) es false!\n";
			}
		}
		return link;
	}

	void FormatterMediator::removeLink(LinkComposition* composition, Link* link) {
		CompositeExecutionObject* compositeObject;

		if (composition->instanceOf("CompositeNode")) {
			compositeObject = (CompositeExecutionObject*)(compiler->
					hasExecutionObject((CompositeNode*)composition, NULL));

			if (compositeObject != NULL) {
				compositeObject->removeNcmLink(link);
			}
		}
		composition->removeLink(link);
	}

	Link* FormatterMediator::removeLink(
		    string documentId, string compositeId, string linkId) {

		NclDocument* document;
		Node* node;
		ContextNode* contextNode;
		Link* link;

		document = privateBaseManager->getDocument(
				data->baseId, documentId);

		if (document == NULL) {
			// document does not exist
			return NULL;
		}

		node = document->getNode(compositeId);
		if (node == NULL || !(node->instanceOf("ContextNode"))) {
			// composite node (compositeId) does exist or is not a context node
			return NULL;
		}

		contextNode = (ContextNode*)node;
		link = contextNode->getLink(linkId);
		if (link == NULL) {
			// link (linkId) is not a nodeId child link
			return NULL;
		}

		removeLink(contextNode, link);
		return link;
	}

	bool FormatterMediator::setPropertyValue(
		    string documentId, string nodeId, string propertyId, string value) {

		NclDocument* document;
		Node* node;
		Anchor* anchor;
		NodeNesting* perspective;
		ExecutionObject* executionObject;
		FormatterEvent* event;
		LinkAssignmentAction* setAction;

		document = privateBaseManager->getDocument(
				data->baseId, documentId);

		if (document == NULL) {
			// document does not exist
			return false;
		}

		node = document->getNode(nodeId);
		if (node == NULL) {
			// node (nodeId) does exist
			return false;
		}

		anchor = node->getAnchor(propertyId);
		if (anchor == NULL or !(anchor->instanceOf("PropertyAnchor"))) {
			//property inexistente or interface (interfaceId) is not a property
			cout << "FormatterMediator::setPropertyValue()";
			cout << " Warning! Trying to set a NULL property";
			cout << " for '" << node->getId() << "'" << endl;
			return false;
		}

		perspective = new NodeNesting(node->getPerspective());
		try {
			executionObject = compiler->getExecutionObject(
				    perspective, NULL, compiler->getDepthLevel());

		} catch (ObjectCreationForbiddenException* exc) {
			return false;
		}

		event = compiler->getEvent(
			    executionObject, anchor, EventUtil::EVT_ATTRIBUTION, "");

		if (event == NULL || !(event->instanceOf("AttributionEvent"))) {
			return false;
		}

		setAction = new LinkAssignmentAction(
			    (AttributionEvent*)event,
				SimpleAction::ACT_SET,
				value);

		((LinkActionListener*)scheduler)->runAction(setAction);
		return true;
	}

	void FormatterMediator::pushEPGEventToEPGFactory(map<string, string> t) {
		playerManager->pushEPGEventToEPGFactory(t);
	}

	void FormatterMediator::addListener(IPlayerListener* listener) {
		Player::addListener(listener);
	}

	void FormatterMediator::removeListener(IPlayerListener* listener) {
		Player::removeListener(listener);
	}

	void FormatterMediator::notifyListeners(
			short code, string paremeter, short type) {

		Player::notifyListeners(code, paremeter, type);
	}

	void FormatterMediator::setSurface(ISurface* surface) {
		Player::setSurface(surface);
	}

	ISurface* FormatterMediator::getSurface() {
		ISurface* s;

		s = Player::getSurface();
		s->clearContent();
		s->flip();

		return s;
	}

	void FormatterMediator::flip() {
		if (data->devClass == IDeviceDomain::CT_PASSIVE) {
			multiDevice->updatePassiveDevices();
		}
	}

	void FormatterMediator::setMediaTime(double newTime) {
		Player::setMediaTime(newTime);
	}

	double FormatterMediator::getMediaTime() {
		return Player::getMediaTime();
	}

	bool FormatterMediator::setKeyHandler(bool isHandler) {
		scheduler->setKeyHandler(isHandler);
		return isHandler;
	}

	void FormatterMediator::setScope(
			string scope, short type, double initTime, double endTime) {

		Player::setScope(scope, type, initTime, endTime);
		cout << "FormatterMediator::setScope '" << scope << "' type '" << type;
		cout << "' initTime '" << initTime << "' endTime '" << endTime;
		cout << endl;
	}

	void FormatterMediator::play() {
		if (currentDocument != NULL) {
			Player::play();
			startDocument(currentDocument->getId(), scope);

		} else {
			cout << "FormatterMediator::play()";
			cout << " Warning! Trying to start a NULL doc";
			cout << " for '" << data->playerId << "'" << endl;
		}
	}

	void FormatterMediator::stop() {
		if (currentDocument != NULL) {
			Player::stop();
			stopDocument(currentDocument->getId());
		}
	}

	void FormatterMediator::abort() {
		stop();
	}

	void FormatterMediator::pause() {
		if (currentDocument != NULL) {
			Player::pause();
			pauseDocument(currentDocument->getId());
		}
	}

	void FormatterMediator::resume() {
		if (currentDocument != NULL) {
			Player::resume();
			resumeDocument(currentDocument->getId());
		}
	}

	string FormatterMediator::getPropertyValue(string name) {
		return Player::getPropertyValue(name);
	}

	void FormatterMediator::setPropertyValue(
			string name, string value, double duration, double by) {

		Player::setPropertyValue(name, value, duration, by);
	}

	void FormatterMediator::setReferenceTimePlayer(IPlayer* player) {
		Player::setReferenceTimePlayer(player);
	}

	void FormatterMediator::addTimeReferPlayer(IPlayer* referPlayer) {
		Player::addTimeReferPlayer(referPlayer);
	}

	void FormatterMediator::removeTimeReferPlayer(IPlayer* referPlayer) {
		Player::removeTimeReferPlayer(referPlayer);
	}

	void FormatterMediator::notifyReferPlayers(int transition) {
		Player::notifyReferPlayers(transition);
	}

	void FormatterMediator::timebaseObjectTransitionCallback(int transition) {
		Player::timebaseObjectTransitionCallback(transition);
	}

	void FormatterMediator::setTimeBasePlayer(IPlayer* timeBasePlayer) {
		Player::setTimeBasePlayer(timeBasePlayer);
	}

	bool FormatterMediator::hasPresented() {
		return Player::hasPresented();
	}

	void FormatterMediator::setPresented(bool presented) {
		Player::setPresented(presented);
	}

	bool FormatterMediator::isVisible() {
		return Player::isVisible();
	}

	void FormatterMediator::setVisible(bool visible) {
		set<ExecutionObject*>::iterator i;
		set<ExecutionObject*>* objects;
		ExecutionObject* object;
		string strVisible        = "true";
		FormatterEvent* event    = NULL;
		PropertyAnchor* property = NULL;
		bool fakeEvent           = false;

		if (!visible) {
			strVisible = "false";
		}

		objects = compiler->getRunningObjects();
		i = objects->begin();
		while (i != objects->end()) {
			object = *i;
			event = object->getEventFromAnchorId("visible");
			if (event == NULL) {
				property = new PropertyAnchor("visible");
				property->setPropertyValue(strVisible);
				event = new AttributionEvent("visible", object, property);
				fakeEvent = true;

			} else {
				fakeEvent = false;
			}

			if (event->instanceOf("AttributionEvent")) {
				playerManager->setVisible(
						object->getId(),
						strVisible,
						(AttributionEvent*)event);
			}

			if (fakeEvent) {
				delete event;
				delete property;
			}
			++i;
		}
		Player::setVisible(visible);

		delete objects;
	}

	bool FormatterMediator::immediatelyStart() {
		return Player::immediatelyStart();
	}

	void FormatterMediator::setImmediatelyStart(bool immediatelyStartVal) {
		Player::setImmediatelyStart(immediatelyStartVal);
	}

	void FormatterMediator::forceNaturalEnd() {
		Player::forceNaturalEnd();
	}

	bool FormatterMediator::isForcedNaturalEnd() {
		return Player::isForcedNaturalEnd();
	}

	bool FormatterMediator::setOutWindow(io::IWindow* w) {
		return Player::setOutWindow(w);
	}

	void FormatterMediator::setCurrentScope(string scopeId) {
		Player::setCurrentScope(scopeId);
	}

	string FormatterMediator::getActiveUris(vector<string>* uris) {
		return getDepUris(uris, IDeviceDomain::CT_ACTIVE);
	}

	string FormatterMediator::getDepUris(vector<string>* uris, int targetDev) {
		CompositeNode* node;
		ContextNode* parent;
		ContextNode* body;
		vector<Node*>* nodes;
		vector<Node*>* childs;
		vector<Node*>::iterator i;
		string baseUri = "", childBaseUri = "";

		if (currentDocument == NULL) {
			return "";
		}

		body = currentDocument->getBody();
		nodes = body->getNodes();
		if (nodes == NULL) {
			return "";
		}

		i = nodes->begin();
		while (i != nodes->end()) {
			if ((*i)->instanceOf("ContextNode")) {
				parent = (ContextNode*)(*i);
				childs = parent->getNodes();
				if (childs != NULL) {
					childBaseUri = getDepUrisFromNodes(uris, childs, targetDev);
					baseUri = getBaseUri(childBaseUri, baseUri);
				}

			} else if ((*i)->instanceOf("CompositeNode")) {
				node = (CompositeNode*)(*i);
				childs = node->getNodes();
				if (childs != NULL) {
					childBaseUri = getDepUrisFromNodes(uris, childs, targetDev);
					baseUri = getBaseUri(childBaseUri, baseUri);
				}

			} else {
				childBaseUri = getDepUriFromNode(uris, *i, targetDev);
				baseUri = getBaseUri(childBaseUri, baseUri);
			}
			++i;
		}

		return baseUri;
	}

	string FormatterMediator::getDepUrisFromNodes(
			vector<string>* uris, vector<Node*>* nodes, int targetDev) {

		CompositeNode* node;
		ContextNode* parent;
		vector<Node*>* childs;
		vector<Node*>::iterator i;
		string baseUri = "", childBaseUri = "";

		i = nodes->begin();
		while (i != nodes->end()) {
			if ((*i)->instanceOf("ContextNode")) {
				parent = (ContextNode*)(*i);
				childs = parent->getNodes();
				if (childs != NULL) {
					childBaseUri = getDepUrisFromNodes(uris, childs, targetDev);
					baseUri = getBaseUri(childBaseUri, baseUri);
				}

			} else if ((*i)->instanceOf("CompositeNode")) {
				node = (CompositeNode*)(*i);
				childs = node->getNodes();
				if (childs != NULL) {
					childBaseUri = getDepUrisFromNodes(uris, childs, targetDev);
					baseUri = getBaseUri(childBaseUri, baseUri);
				}

			} else {
				childBaseUri = getDepUriFromNode(uris, *i, targetDev);
				baseUri = getBaseUri(childBaseUri, baseUri);
			}
			++i;
		}

		return baseUri;
	}

	string FormatterMediator::getDepUriFromNode(
			vector<string>* uris, Node* node, int targetDev) {

		GenericDescriptor* descriptor;
		LayoutRegion* ncmRegion;
		NodeEntity* nodeEntity;
		Content* content;
		string src = "";

		content = ((NodeEntity*)((node)->getDataEntity()))->getContent();
		if (content != NULL && content->instanceOf("ReferenceContent")) {
			src = ((ReferenceContent*)content)->getCompleteReferenceUrl();
			if (src != "") {
				if (targetDev == 0) {
					uris->push_back(src);

				} else {
					nodeEntity = ((NodeEntity*)node->getDataEntity());
					descriptor = nodeEntity->getDescriptor();
					
					if (descriptor != NULL && !(descriptor->instanceOf("DescriptorSwitch"))) {
						//TODO: find descriptor switch urls
						ncmRegion  = ((Descriptor*)descriptor)->getRegion();
						if (ncmRegion != NULL &&
								ncmRegion->getDeviceClass() == targetDev) {

							uris->push_back(src);
						}
					}
				}
			}
		}

		return src;
	}

	string FormatterMediator::getBaseUri(string baseA, string baseB) {
		string base = "";

		if (baseA == "") {
			base = baseB;

		} else if (baseB != "") {
			if (baseB.length() < baseA.length()) {
				base = baseB;
			}

		} else {
			base = baseA;
		}

		return base;
	}

	void FormatterMediator::timeShift(string direction) {
		playerManager->timeShift(direction);
	}
}
}
}
}
}

using namespace ::br::pucrio::telemidia::ginga::ncl;

extern "C" INCLPlayer* createNCLPlayer(NclPlayerData* data) {
	return (new FormatterMediator(data));
}

extern "C" void destroyNCLPlayer(INCLPlayer* p) {
	delete p;
}
