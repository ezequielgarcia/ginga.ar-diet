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

#ifndef _FormatterMediator_H_
#define _FormatterMediator_H_

/****************************/
//#ifdef linux
	#define SOFLAG 1
//#endif

#ifdef WIN32
	#define SOFLAG 2
#endif

/****************************/

#include "player/INCLPlayer.h"
#include "player/Player.h"
using namespace ::br::pucrio::telemidia::ginga::core::player;

#include "ncl/components/ContextNode.h"
#include "ncl/components/CompositeNode.h"
#include "ncl/components/ContentNode.h"
#include "ncl/components/Node.h"
#include "ncl/components/NodeEntity.h"
using namespace ::br::pucrio::telemidia::ncl::components;

#include "ncl/interfaces/Anchor.h"
#include "ncl/interfaces/PropertyAnchor.h"
#include "ncl/interfaces/Port.h"
#include "ncl/interfaces/SwitchPort.h"
#include "ncl/interfaces/InterfacePoint.h"
using namespace ::br::pucrio::telemidia::ncl::interfaces;

#include "ncl/switches/Rule.h"
using namespace ::br::pucrio::telemidia::ncl::switches;

#include "ncl/descriptor/GenericDescriptor.h"
using namespace ::br::pucrio::telemidia::ncl::descriptor;

#include "ncl/link/Bind.h"
#include "ncl/link/CausalLink.h"
#include "ncl/link/Link.h"
#include "ncl/link/LinkComposition.h"
using namespace ::br::pucrio::telemidia::ncl::link;

#include "ncl/connectors/EventUtil.h"
#include "ncl/connectors/SimpleAction.h"
#include "ncl/connectors/Connector.h"
using namespace ::br::pucrio::telemidia::ncl::connectors;

#include "ncl/layout/LayoutRegion.h"
using namespace ::br::pucrio::telemidia::ncl::layout;

#include "ncl/reuse/ReferNode.h"
using namespace ::br::pucrio::telemidia::ncl::reuse;

#include "system/util/functions.h"
using namespace ::util;

#include "ncl/Base.h"
#include "ncl/NclDocument.h"
#include "ncl/connectors/ConnectorBase.h"
#include "ncl/descriptor/DescriptorBase.h"
#include "ncl/layout/RegionBase.h"
#include "ncl/switches/RuleBase.h"
using namespace ::br::pucrio::telemidia::ncl;

#include "ncl/transition/Transition.h"
#include "ncl/transition/TransitionBase.h"
using namespace ::br::pucrio::telemidia::ncl::transition;

#include "model/CompositeExecutionObject.h"
#include "model/ExecutionObject.h"
#include "model/NodeNesting.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::components;

#include "model/FormatterEvent.h"
#include "model/AttributionEvent.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::event;

#include "model/LinkActionListener.h"
#include "model/LinkAssignmentAction.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::link;

#include "adaptation/context/RuleAdapter.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::adaptation::context;

#include "emconverter/ObjectCreationForbiddenException.h"
#include "emconverter/FormatterConverter.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::emconverter;

#include "prefetch/IPrefetchManager.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::prefetch;

#include "FormatterScheduler.h"
#include "privatebase/PrivateBaseManager.h"

#include "editingcommandevents.h"

#include <map>
#include <set>
#include <vector>
#include <string>
using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
  class FormatterMediator : public INCLPlayer, public Player,
		/*public INclEditListener,*/ public IFormatterSchedulerListener {

	private:

		NclPlayerData* data;
		string currentFile;
		NclDocument* currentDocument;

		static PrivateBaseManager* privateBaseManager;
		static IPrefetchManager* pm;
		map<string, FormatterEvent*>* documentEvents;
		map<string, vector<FormatterEvent*>*>* documentEntryEvents;
		FormatterScheduler* scheduler;
		RuleAdapter* ruleAdapter;
		FormatterConverter* compiler;
		PlayerAdapterManager* playerManager;
		ITimeBaseProvider* timeBaseProvider;
		vector<string>* compileErrors;
		IDeviceLayout* deviceLayout;
		IFormatterMultiDevice* multiDevice;
		bool isEmbedded;

	public:
		FormatterMediator(NclPlayerData* data);
		virtual ~FormatterMediator();

		static void release();
		void refreshContent(){};
		void printData(NclPlayerData* data);
		void setStandByState(bool standBy);
		void setTimeBaseProvider(ITimeBaseProvider* timeBaseProvider);

		void setBackgroundImage(string uri);
		void setParentLayout(void* parentLayout);
		string getScreenShot();

		vector<string> *getCompileErrors();
		void* setCurrentDocument(string fName);

	private:
		virtual void* addDocument(string fName);
		bool removeDocument(string documentId);
		ContextNode* getDocumentContext(string documentId);

	public:
		void setDepthLevel(int level);
		int getDepthLevel();

	private:
		vector<FormatterEvent*>* processDocument(
				string documentId, string interfaceId);

		vector<FormatterEvent*>* getDocumentEntryEvent(string documentId);
		bool compileDocument(string documentId, string interfaceId);
		bool prepareDocument(string documentId);

		void solveRemoteDescriptorsUris(string docLocation,
					vector<GenericDescriptor*>* descs, bool isRemoteDoc);

		void solveRemoteNodesUris(
				string docLocation, vector<Node*>* nodes, bool isRemoteDoc);

		void solveRemoteNclDeps(string docLocation, bool isRemoteDoc);

		void solveRemoteLuaDeps(
				string docLocation, string src, bool isRemoteDoc);

		string solveRemoteSourceUri(string docLocation, string src);

		bool startDocument(string documentId, string interfaceId);
		bool stopDocument(string documentId);
		bool pauseDocument(string documentId);
		bool resumeDocument(string documentId);
		void presentationCompleted(IFormatterEvent* documentEvent);

	public:
		//bool nclEdit(string nclEditApi);
		//bool editingCommand(string commandTag, string privateDataPayload);
		bool editingCommand(::br::pucrio::telemidia::ginga::ncl::StartDocumentEC* ec);
		bool editingCommand(::br::pucrio::telemidia::ginga::ncl::StopDocumentEC* ec);
		bool editingCommand(::br::pucrio::telemidia::ginga::ncl::AddLinkEC* ec);
		bool editingCommand(::br::pucrio::telemidia::ginga::ncl::SetPropertyValueEC* ec);
		bool editingCommand(::br::pucrio::telemidia::ginga::ncl::AddNodeEC* ec);
		bool editingCommand(::br::pucrio::telemidia::ginga::ncl::AddInterfaceEC* ec);

	private:
		LayoutRegion* addRegion(
				string documentId,
				string regionBaseId,
				string regionId,
				string xmlRegion);

		LayoutRegion* removeRegion(
				string documentId,
				string regionBaseId,
				string regionId);

		RegionBase* addRegionBase(string documentId, string xmlRegionBase);
		RegionBase* removeRegionBase(
				string documentId, string regionBaseId);

		Rule* addRule(string documentId, string xmlRule);
		Rule* removeRule(string documentId, string ruleId);
		RuleBase* addRuleBase(string documentId, string xmlRuleBase);
		RuleBase* removeRuleBase(string documentId, string ruleBaseId);
		Transition* addTransition(string documentId, string xmlTransition);
		Transition* removeTransition(
				string documentId, string transitionId);

		TransitionBase* addTransitionBase(
				string documentId, string xmlTransitionBase);

		TransitionBase* removeTransitionBase(
				string documentId, string transitionBaseId);

		Connector* addConnector(string documentId, string xmlConnector);
		Connector* removeConnector(string documentId, string connectorId);
		ConnectorBase* addConnectorBase(
				string documentId, string xmlConnectorBase);

		ConnectorBase* removeConnectorBase(
				string documentId, string connectorBaseId);

		GenericDescriptor* addDescriptor(
				string documentId, string xmlDescriptor);

		GenericDescriptor* removeDescriptor(
				string documentId, string descriptorId);

		DescriptorBase* addDescriptorBase(
				string documentId, string xmlDescriptorBase);

		DescriptorBase* removeDescriptorBase(
				string documentId, string descriptorBaseId);

		Base* addImportBase(
				string documentId, string docBaseId, string xmlImportBase);

		Base* removeImportBase(
				string documentId, string docBaseId, string documentURI);

		NclDocument* addImportedDocumentBase(
				string documentId, string xmlImportedDocumentBase);

		NclDocument* removeImportedDocumentBase(
				string documentId, string importedDocumentBaseId);

		NclDocument* addImportNCL(string documentId, string xmlImportNCL);
		NclDocument* removeImportNCL(string documentId, string documentURI);

		void processInsertedReferNode(ReferNode* referNode);
		void processInsertedComposition(CompositeNode* composition);

		Node* addNode(
				string documentId, string compositeId, string xmlNode);

		Node* removeNode(
				string documentId, string compositeId, string nodeId);

		InterfacePoint* addInterface(
				string documentId, string nodeId, string xmlInterface);

		void removeInterfaceMappings(
				Node* node,
				InterfacePoint* interfacePoint,
				CompositeNode* composition);

		void removeInterfaceLinks(
				Node* node,
				InterfacePoint* interfacePoint,
				LinkComposition* composition);

		void removeInterface(
				Node* node, InterfacePoint* interfacePoint);

		InterfacePoint* removeInterface(
				string documentId, string nodeId, string interfaceId);

		Link* addLink(
				string documentId, string compositeId, string xmlLink);

		void removeLink(LinkComposition* composition, Link* link);
		Link* removeLink(
				string documentId, string compositeId, string linkId);

		bool setPropertyValue(
				string documentId,
				string nodeId,
				string propertyId,
				string value);

		void pushEPGEventToEPGFactory(map<string, string> t);

	public:
		void setNotifyContentUpdate(bool notify){};
		void addListener(IPlayerListener* listener);
		void removeListener(IPlayerListener* listener);
		void notifyListeners(
				short code, string paremeter, short type);

		void setSurface(io::ISurface* surface);
		io::ISurface* getSurface();
		void flip();
		double getMediaTime();
		void setMediaTime(double newTime);
		bool setKeyHandler(bool isHandler);
		void setScope(
				string scope, short type, double begin=-1, double end=-1);

		void play();
		void stop();
		void abort();
		void pause();
		void resume();

		string getPropertyValue(string name);
		void setPropertyValue(
				string name,
				string value, double duration=-1, double by=-1);

		void setReferenceTimePlayer(IPlayer* player);
		void addTimeReferPlayer(IPlayer* referPlayer);
		void removeTimeReferPlayer(IPlayer* referPlayer);
		void notifyReferPlayers(int transition);
		void timebaseObjectTransitionCallback(int transition);
		void setTimeBasePlayer(IPlayer* timeBasePlayer);
		bool hasPresented();
		void setPresented(bool presented);
		bool isVisible();
		void setVisible(bool visible);
		bool immediatelyStart();
		void setImmediatelyStart(bool immediatelyStartVal);
		void forceNaturalEnd();
		bool isForcedNaturalEnd();
		bool setOutWindow(io::IWindow* w);

		/*Exclusive for ChannelPlayer*/
		IPlayer* getSelectedPlayer() {return NULL;};
		void setPlayerMap(map<string, IPlayer*>* objs){};
		map<string, IPlayer*>* getPlayerMap() {return NULL;};
		IPlayer* getPlayer(string objectId) {return NULL;};
		void select(IPlayer* selObject){};

		/*Exclusive for Application Players*/
		void setCurrentScope(string scopeId);

		string getActiveUris(vector<string>* uris);
		string getDepUris(vector<string>* uris, int targetDev=0);

	private:
		string getDepUrisFromNodes(
				vector<string>* uris, vector<Node*>* nodes, int targetDev=0);

		string getDepUriFromNode(
				vector<string>* uris, Node* node, int targetDev=0);

		string getBaseUri(string baseA, string baseB);

	public:
		void timeShift(string direction);
  };
}
}
}
}
}

#endif //_FormatterMediator_H_
