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

#include "../include/CommonCoreManager.h"
#include "generated/config.h"

#if HAVE_COMPSUPPORT
#include "cm/IComponentManager.h"
using namespace ::br::pucrio::telemidia::ginga::core::cm;
#else
#include <system/io/interface/output/dfb/DFBWindow.h>
#include <player/AVPlayer.h>
#include <player/ImagePlayer.h>
#endif

#if HAVE_TUNER
#include <tuner/ITuner.h>
using namespace ::br::pucrio::telemidia::ginga::core::tuning;
#endif

#if HAVE_TSPARSER
#include <tsparser/IDemuxer.h>
#include <tsparser/ITSFilter.h>
using namespace ::br::pucrio::telemidia::ginga::core::tsparser;
#endif

#if HAVE_DSMCC
#include <Mdataprocessing/IDataProcessor.h>
using namespace ::br::pucrio::telemidia::ginga::core::dataprocessing;

#include <dataprocessing/dsmcc/carousel/object/IObjectListener.h>
using namespace ::br::pucrio::telemidia::ginga::core::dataprocessing::carousel;
#endif

#include <system/io/interface/output/IWindow.h>
using namespace ::br::pucrio::telemidia::ginga::core::system::io;

#include <player/IPlayer.h>
//#include "player/IProgramAV.h"
using namespace ::br::pucrio::telemidia::ginga::core::player;

#if HAVE_TUNER && HAVE_TSPARSER && HAVE_DSMCC
#include "../include/DataWrapperListener.h"
#endif

#include <pthread.h>
#include <string>
#include <iostream>
using namespace std;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace lssm {
#if HAVE_COMPSUPPORT
	static IComponentManager* cm = IComponentManager::getCMInstance();
#endif
	CommonCoreManager::CommonCoreManager(IPresentationEngineManager* pem) {
		tuningWindow    = NULL;
		tuner           = NULL;
		demuxer         = NULL;
		ccUser          = NULL;
		nptProvider     = NULL;
		ocDelay         = 0;
		this->pem       = pem;

#if HAVE_TUNER && HAVE_TSPARSER && HAVE_DSMCC
#if HAVE_COMPSUPPORT
		tuner = ((TunerCreator*)(cm->getObject("Tuner")))();
#else
		tuner = new Tuner();
#endif

		pem->setIsLocalNcl(false, tuner);

#if HAVE_COMPSUPPORT
		demuxer = ((demCreator*)(cm->getObject("Demuxer")))((ITuner*)tuner);
#else
		demuxer = new Demuxer((Tuner*) tuner);
#endif

#if HAVE_COMPSUPPORT
		dataProcessor = ((dpCreator*)(cm->getObject("DataProcessor")))();
#else
		dataProcessor = new DataProcessor();
#endif

		ccUser = pem->getDsmccListener();

		// Add PEM as a listener of SEs and OCs
		((IDataProcessor*)dataProcessor)->addEPGListener(
				(IEPGListener*)((DataWrapperListener*)ccUser));

		((IDataProcessor*)dataProcessor)->addSEListener(
				"gingaEditingCommands", (IStreamEventListener*)(
						(DataWrapperListener*)ccUser));

		((IDataProcessor*)dataProcessor)->addObjectListener(
				(IObjectListener*)((DataWrapperListener*)ccUser));

		((IDataProcessor*)dataProcessor)->setServiceDomainListener(
				(IServiceDomainListener*)((DataWrapperListener*)ccUser));


#endif //HAVE_TUNER && HAVE_TSPARSER && HAVE_DSMCC
	}

	void CommonCoreManager::setOCDelay(double ocDelay) {
		this->ocDelay = ocDelay;
	}

	void CommonCoreManager::removeOCFilterAfterMount(bool removeIt) {
#if HAVE_TUNER && HAVE_TSPARSER && HAVE_DSMCC
		((IDataProcessor*)dataProcessor)->removeOCFilterAfterMount(removeIt);
#endif
	}

	void CommonCoreManager::startPresentation() {
		int aPid, vPid;

#if HAVE_TUNER && HAVE_TSPARSER && HAVE_DSMCC

		NclPlayerData* data   = NULL;
		StcWrapper* sw        = NULL;
		INetworkInterface* ni = NULL;
		ITSFilter* mavFilter  = NULL;
		IWindow* wmv          = NULL;
		ISurface* s           = NULL;
		ISurface* smv         = NULL;
		//IPlayer* ma           = NULL;
		IPlayer* mv           = NULL;
		string mrlv           = "sbtvd-ts://dvr0.ts";
		//string mrla           = "sbtvd-ts://dvr1.ts";

		data = pem->createNclPlayerData();

		if (fileExists(DATA_PREFIX_PATH "/etc/ginga/tuner/tuning.png")) {
#if HAVE_COMPSUPPORT
			tuningWindow = ((WindowCreator*)(cm->getObject("Window")))(
					data->x, data->y, data->w, data->h);

			s = ((ImageRenderer*)(cm->getObject("ImageRenderer")))(
					(char*)(DATA_PREFIX_PATH "/etc/ginga/tuner/tuning.png"));

#else
			tuningWindow = new DFBWindow(data->x, data->y, data->w, data->h);
			s = ImagePlayer::renderImage(
					DATA_PREFIX_PATH "/etc/ginga/tuner/tuning.png");
#endif

			((IWindow*)tuningWindow)->setCaps(
					((IWindow*)tuningWindow)->getCap("ALPHACHANNEL"));

			((IWindow*)tuningWindow)->draw();
			((IWindow*)tuningWindow)->show();
			((IWindow*)tuningWindow)->lowerToBottom();
			((IWindow*)tuningWindow)->renderFrom(s);

			delete s;
			s = NULL;
		}

#if HAVE_COMPSUPPORT
		wmv = ((WindowCreator*)(cm->getObject("Window")))(
				data->x, data->y, data->w, data->h);

#else
		wmv = new DFBWindow(data->x, data->y, data->w, data->h);
#endif

		wmv->draw();

		delete data;

		((ITuner*)tuner)->tune();
		((IDemuxer*)demuxer)->waitProgramInformation();

		if (ocDelay == 0) {
			((IDataProcessor*)dataProcessor)->
				createStreamTypeSectionFilter(
						STREAM_TYPE_DSMCC_TYPE_B, (IDemuxer*)demuxer);
		}

		pem->setCurrentPrivateBaseId(((IDemuxer*)demuxer)->getTSId());

		if (ocDelay > 0) {
			((IDataProcessor*)dataProcessor)->
					createStreamTypeSectionFilter(
							STREAM_TYPE_DSMCC_TYPE_B, (IDemuxer*)demuxer);

			::usleep((int)(ocDelay * 1000));
		}

#if HAVE_COMPSUPPORT
		mavFilter = ((filterCreator*)(cm->getObject("PipeFilter")))(0);

#else
		mavFilter = new PipeFilter(0);
#endif

		//mavFilter->addDestination(1);

		((IDemuxer*)demuxer)->addPesFilter(PFT_DEFAULTTS, mavFilter);

		// Create Main AV
		cout << "lssm-ccm::sp creating av player" << endl;

#if HAVE_COMPSUPPORT
		mv = ((PlayerCreator*)(cm->getObject("AVPlayer")))(
				mrlv.c_str(), true);

		//ma = ((PlayerCreator*)(cm->getObject("AVPlayer")))(
		//		mrla.c_str(), false);

#else
		mv = new AVPlayer(mrlv.c_str(), true);
		//ma = new AVPlayer(mrla.c_str(), true);
#endif

		//ma->setOutWindow(NULL);
		mv->setOutWindow(wmv);

		cout << "lssm-ccp::sp create av ok" << endl;

		vPid = ((IDemuxer*)demuxer)->getDefaultMainVideoPid();
		aPid = ((IDemuxer*)demuxer)->getDefaultMainAudioPid();

		((IDemuxer*)demuxer)->waitBuffers();
		while (smv == NULL) {
			smv = mv->getSurface();
			::usleep(100000);
		}

		ni = ((ITuner*)tuner)->getCurrentInterface();
		if (ni != NULL && ni->getCaps() & DPC_CAN_DECODESTC) {
			cout << "lssm-ccp::sp using stc hardware!" << endl;
			((IDataProcessor*)dataProcessor)->setSTCProvider(ni);

		} else {
			cout << "lssm-ccp::sp using stc wrapper!" << endl;
			sw = new StcWrapper(mv);
			((IDataProcessor*)dataProcessor)->setSTCProvider(sw);
		}

		nptProvider = ((IDataProcessor*)dataProcessor)->getNPTProvider();
		if (nptProvider != NULL) {
			pem->setTimeBaseProvider((ITimeBaseProvider*)nptProvider);
		}

		/*((IDataProcessor*)dataProcessor)->
				createStreamTypeSectionFilter(
						STREAM_TYPE_DSMCC_TYPE_C, (IDemuxer*)demuxer);*/

		((IDataProcessor*)dataProcessor)->
				createStreamTypeSectionFilter(
						STREAM_TYPE_DSMCC_TYPE_D, (IDemuxer*)demuxer);

		/*((IDataProcessor*)dataProcessor)->
				createPidSectionFilter(
						SDT_PID, (IDemuxer*)demuxer); //SDT

		((IDataProcessor*)dataProcessor)->
				createPidSectionFilter(
						EIT_PID, (IDemuxer*)demuxer); //EIT

		((IDataProcessor*)dataProcessor)->
				createPidSectionFilter(
						CDT_PID, (IDemuxer*)demuxer); //CDT*/

		wmv->show();

		if (tuningWindow != NULL) {
			((IWindow*)tuningWindow)->clearContent();
			((IWindow*)tuningWindow)->hide();
			delete ((IWindow*)tuningWindow);
			tuningWindow = NULL;
		}

		pthread_mutex_t mutex;
		pthread_cond_t condition;

		cout << "lssm ccm::sp cond_wait" << endl;
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&condition, NULL);

		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&condition, &mutex);
		pthread_mutex_unlock(&mutex);

		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&condition);

		cout << "lssm ccm::sp all done!" << endl;
#endif //TUNER...
		
		// Activate PEM Connector
		//pem->startConnector();

	}

        void CommonCoreManager::waitPresentationEnd()
        {
                pthread_mutex_t mutex;
                pthread_cond_t condition;

                cout << "lssm ccm::sp cond_wait" << endl;
                pthread_mutex_init(&mutex, NULL);
                pthread_cond_init(&condition, NULL);

                pthread_mutex_lock(&mutex);
                pthread_cond_wait(&condition, &mutex);
                pthread_mutex_unlock(&mutex);

                pthread_mutex_destroy(&mutex);
                pthread_cond_destroy(&condition);

                cout << "lssm ccm::sp all done!" << endl;
        }
}
}
}
}
}

extern "C" ::br::pucrio::telemidia::ginga::lssm::ICommonCoreManager*
		createCCM(::br::pucrio::telemidia::ginga::lssm::
				IPresentationEngineManager* pem) {

	return new ::br::pucrio::telemidia::ginga::lssm::CommonCoreManager(pem);
}

extern "C" void destroyCCM(
		::br::pucrio::telemidia::ginga::lssm::ICommonCoreManager* ccm) {

	delete ccm;
}
