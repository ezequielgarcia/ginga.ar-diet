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
using namespace ::br::pucrio::telemidia::ginga::ncl::model::presentation;

#include "../../../include/model/FormatterRegion.h"
#include "../../../include/model/FormatterLayout.h"

#include "generated/config.h"

#if HAVE_COMPSUPPORT
#include "cm/IComponentManager.h"
using namespace ::br::pucrio::telemidia::ginga::core::cm;
#else
#include <system/io/interface/output/dfb/DFBWindow.h>
#endif

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace ncl {
namespace model {
namespace presentation {
#if HAVE_COMPSUPPORT
	static IComponentManager* cm = IComponentManager::getCMInstance();
#endif
	FormatterRegion::FormatterRegion(
		    string objectId, void* descriptor, void* layoutManager) {

		this->layoutManager = layoutManager;
		this->objectId = objectId;
		this->descriptor = descriptor;

		originalRegion = NULL;
		if (descriptor != NULL) {
			originalRegion = ((CascadingDescriptor*)descriptor)->getRegion();
		}

		if (originalRegion != NULL) {
			ncmRegion = originalRegion->cloneRegion();

		} else {
			ncmRegion = NULL;
		}

		this->outputDisplay = NULL;
		this->focusState = FormatterRegion::UNSELECTED;
		this->focusBorderColor = new Color("white");
		this->focusBorderWidth = 0;
		this->focusComponentSrc = "";
		this->selBorderColor = new Color("red");
		this->selBorderWidth = 0;
		this->selComponentSrc = "";
		this->chromaKey = NULL;
		this->transitionIn = "";
		this->transitionOut = "";
		this->abortTransitionIn = false;
		this->abortTransitionOut = false;
		this->regionVisible = true;

		pthread_mutex_init(&mutex, NULL);
		pthread_mutex_init(&mutexT, NULL);
		pthread_mutex_init(&mutexFI, NULL);

		// TODO: look for descriptor parameters overriding region attributes
		string value;
		value = ((CascadingDescriptor*)descriptor)->getParameterValue(
			    "transparency");

		this->setTransparency(value);

		value = ((CascadingDescriptor*)descriptor)->getParameterValue(
			    "background");

		this->setBackgroundColor(util::trim(value));

		value = ((CascadingDescriptor*)descriptor)->getParameterValue("fit");

		this->setFit(value);

		value = ((CascadingDescriptor*)descriptor)->
			    getParameterValue("scroll");

		this->setScroll(value);

		value = ((CascadingDescriptor*)descriptor)->getParameterValue(
			    "chromakey");

		this->setChromaKey(value);

		value = ((CascadingDescriptor*)descriptor)->getParameterValue(
			    "rgbChromakey");

		if (value == "") {
			value = ((CascadingDescriptor*)descriptor)->getParameterValue(
				    "x-rgbChromakey");
		}

		this->setRgbChromaKey(value);

		//TODO: methods setTransIn and setTransOut
		value = ((CascadingDescriptor*)descriptor)->getParameterValue(
			    "transitionIn");

		if (value != "") {
			transitionIn = value;
		}

		value = ((CascadingDescriptor*)descriptor)->getParameterValue(
			    "transitionOut");

		if (value != "") {
			transitionOut = value;
		}

		//TODO: fit and scroll
	}

	FormatterRegion::~FormatterRegion() {
		lockFocusInfo();
		layoutManager = NULL;
		descriptor = NULL;

		originalRegion = NULL;
		if (ncmRegion != NULL) {
			delete ncmRegion;
			ncmRegion = NULL;
		}

		lock();
		if (outputDisplay != NULL) {
			delete outputDisplay;
			outputDisplay = NULL;
		}
		unlock();

		if (focusBorderColor != NULL) {
			delete focusBorderColor;
			focusBorderColor = NULL;
		}

		if (selBorderColor != NULL) {
			delete selBorderColor;
			selBorderColor = NULL;
		}

		if (bgColor != NULL) {
			delete bgColor;
			bgColor = NULL;
		}

		if (chromaKey != NULL) {
			delete chromaKey;
			chromaKey = NULL;
		}

		pthread_mutex_destroy(&mutex);
		lockTransition();
		unlockTransition();
		pthread_mutex_destroy(&mutexT);
		unlockFocusInfo();
		pthread_mutex_destroy(&mutexFI);
	}

	void* FormatterRegion::getLayoutManager() {
		return layoutManager;
	}

	void FormatterRegion::meetComponent(
		    int width,
		    int height,
		    int prefWidth,
		    int prefHeight,
		    ISurface* component) {

		int finalH, finalW;

		if (prefWidth == 0 || prefHeight == 0) {
			return;
		}

		finalH = (prefHeight * width) / prefWidth;
		if (finalH <= height) {
			finalW = width;

		} else {
			finalH = height;
			finalW = (prefWidth * height) / prefHeight;
		}
		//component->setSize(finalW, finalH);
	}

	void FormatterRegion::sliceComponent(
		    int width,
		    int height,
		    int prefWidth,
		    int prefHeight,
		    ISurface* component) {

		int finalH, finalW;

		if (prefWidth == 0 || prefHeight == 0) {
			return;
		}

		finalH = (prefHeight * width) / prefWidth;
		if (finalH > height) {
			finalW = width;

		} else {
			finalH = height;
			finalW = (prefWidth * height) / prefHeight;
		}

		//component->setSize(finalW, finalH);
	}

	void FormatterRegion::updateCurrentComponentSize() {
		//int prefWidth, prefHeight, width, height;

		//sizeRegion();

		switch (fit) {
			case Descriptor::FIT_HIDDEN:
				/*currentComponent.setSize(
					(int)currentComponent.getPreferredSize().getWidth(),
					(int)currentComponent.getPreferredSize().getHeight());*/
				break;

			case Descriptor::FIT_MEET:
/*				prefWidth = (int)currentComponent.
					    getPreferredSize().getWidth();

				prefHeight = (int)currentComponent.
					    getPreferredSize().getHeight();

				width = outputDisplay.getWidth();
				height = outputDisplay.getHeight();
				meetComponent(
					    width,
					    height,
					    prefWidth,
					    prefHeight,
					    currentComponent);*/

				break;

			case Descriptor::FIT_MEETBEST:
				/*prefWidth = (int)currentComponent.
					    getPreferredSize().getWidth();

				prefHeight = (int)currentComponent.
					    getPreferredSize().getHeight();

				width = outputDisplay.getWidth();
				height = outputDisplay.getHeight();

				// the scale factor must not overtake 100% (2 times)
				if ((2 * prefWidth) >= width &&	(2 * prefHeight) >= height) {
					meetComponent(
						    width,
						    height,
						    prefWidth,
						    prefHeight,
						    currentComponent);

				}*/

				break;

			case Descriptor::FIT_SLICE:
				/*prefWidth = (int)currentComponent->
					    getPreferredSize().getWidth();

				prefHeight = (int)currentComponent->
					    getPreferredSize().getHeight();

				width = outputDisplay.getWidth();
				height = outputDisplay.getHeight();
				sliceComponent(
					    width,
					    height,
					    prefWidth,
					    prefHeight,
					    currentComponent);*/

				break;

			case Descriptor::FIT_FILL:
			default:
				/*currentComponent->setSize(
						outputDisplay.getWidth(),
						outputDisplay.getHeight());*/
				break;
		}
	}

	void FormatterRegion::updateCurrentComponentLocation() {
		//currentComponent->setLocation(0, 0);
	}

	void FormatterRegion::updateRegionBounds() {
		sizeRegion();
		if (outputDisplay != NULL) {
			//updateCurrentComponentSize();
    		//updateCurrentComponentLocation();
			if (focusState == FormatterRegion::UNSELECTED) {
				unselect();

			} else { //is focused (at least)
				// if is focused and selected
				if (focusState == FormatterRegion::SELECTED) {
					setSelection(true);

				} else if (focusState == FormatterRegion::FOCUSED) {
					setFocus(true);
				}
			}
		}
	}

	void FormatterRegion::sizeRegion() {
		int left = 0;
		int top = 0;
		int width = 0;
		int height = 0;

		if (ncmRegion != NULL) {
			left = ncmRegion->getLeftInPixels();
			top = ncmRegion->getTopInPixels();
			width = ncmRegion->getWidthInPixels();
			height = ncmRegion->getHeightInPixels();
		}

		/*wclog << "FormatterRegion::sizeRegion windowAdd = '" << outputDisplay;
		wclog << "' x = '" << left;
		wclog << "' y = '" << top;
		wclog << "' w = '" << width;
		wclog << "' h = '" << height << "'" << endl;*/

		if (left < 0)
			left = 0;

		if (top < 0)
			top = 0;

		if (width <= 0)
			width = 1;

		if (height <= 0)
			height = 1;

		lock();
		if (outputDisplay != NULL) {
			outputDisplay->setBounds(left, top, width, height);
		}
		unlock();
	}

	bool FormatterRegion::intersects(int x, int y) {
		if (ncmRegion != NULL) {
			return ncmRegion->intersects(x, y);
		}

		return false;
	}

	LayoutRegion* FormatterRegion::getLayoutRegion() {
		return ncmRegion;
	}

	LayoutRegion* FormatterRegion::getOriginalRegion() {
		return originalRegion;
	}

	IWindow* FormatterRegion::getODContentPane() {
		return outputDisplay;
	}

	void FormatterRegion::prepareOutputDisplay(ISurface* renderedSurface) {
		//cout << "FormatterRegion::prepareOutputDisplay" << endl;
		lock();
		if (outputDisplay == NULL) {
			string title;
			int left = 0;
			int top = 0;
			int width = 0;
			int height = 0;

			if (ncmRegion == NULL) {
				title = objectId;

			} else {
				if (ncmRegion->getTitle() == "") {
					title = objectId;

				} else {
					title = ncmRegion->getTitle();
				}

				left = ncmRegion->getAbsoluteLeft();
				top = ncmRegion->getAbsoluteTop();
				width = ncmRegion->getWidthInPixels();
				height = ncmRegion->getHeightInPixels();
			}
/*
			cout << "FormatterRegion::prepareOutputDisplay";
			cout << " regionId = " << ncmRegion->getId();

			cout << " xip = " << ncmRegion->getLeftInPixels();
			cout << " x = " << ncmRegion->getLeft();
			cout << " ax = " << ncmRegion->getAbsoluteLeft();
			cout << " yip = " << ncmRegion->getTopInPixels();
			cout << " y = " << ncmRegion->getTop();
			cout << " ay = " << ncmRegion->getAbsoluteTop();
			cout << " wip = " << width;
			cout << " w = " << ncmRegion->getWidth();
			cout << " hip = " << height;
			cout << " h = " << ncmRegion->getHeight();
			cout << endl;

			LayoutRegion *parentRegion, *grandParent;
			parentRegion = ncmRegion->getParent();
			cout << "FormatterRegion::prepareOutputDisplay";
			cout << " parentId = " << parentRegion->getId();
			cout << " ax = " << par
entRegion->getAbsoluteLeft();
			cout << " xip = " << parentRegion->getLeftInPixels();
			cout << " ay = " << parentRegion->getAbsoluteTop();
			cout << " yip = " << parentRegion->getTopInPixels();
			cout << " wip = " << parentRegion->getWidthInPixels();
			cout << " w = " << parentRegion->getWidth();
			cout << " hip = " << parentRegion->getHeightInPixels();
			cout << " h = " << parentRegion->getHeight();
			cout << endl;
			grandParent = parentRegion->getParent();
			while (grandParent != NULL) {
				parentRegion = grandParent;
				grandParent = grandParent->getParent();
			}
			cout << "FormatterRegion::prepareOutputDisplay";
			cout << " grandParentId = " << parentRegion->getId();
			cout << " ax = " << parentRegion->getAbsoluteLeft();
			cout << " ay = " << parentRegion->getAbsoluteTop();
			cout << " wip = " << parentRegion->getWidthInPixels();
			cout << " w = " << parentRegion->getWidth();
			cout << " hip = " << parentRegion->getHeightInPixels();
			cout << " h = " << parentRegion->getHeight();
			cout << endl;
*/
			if (left < 0)
				left = 0;

			if (top < 0)
				top = 0;

			if (width <= 0)
				width = 1;

			if (height <= 0)
				height = 1;

#if HAVE_COMPSUPPORT
			outputDisplay = ((WindowCreator*)(cm->getObject("Window")))(
					left, top, width, height);
#else
			outputDisplay = new DFBWindow(left, top, width, height);
#endif
			if (bgColor != NULL) {
				outputDisplay->setBackgroundColor(
					    bgColor->getR(),
					    bgColor->getG(),
					    bgColor->getB(),
					    bgColor->getAlpha());
			}

			outputDisplay->setTransparencyValue((int)(transparency * 255));

			if (renderedSurface->getCaps() &
					outputDisplay->getCap("ALPHACHANNEL")) {

				outputDisplay->addCaps(
						outputDisplay->getCap("ALPHACHANNEL"));
			}

			if (chromaKey != NULL) {
				outputDisplay->setCaps(outputDisplay->getCap("NOSTRUCTURE"));
				outputDisplay->draw();
				outputDisplay->setColorKey(
					    chromaKey->getR(),
					    chromaKey->getG(),
					    chromaKey->getB());

			} else {
				outputDisplay->draw();
			}

			if (scroll != Descriptor::SCROLL_NONE) {
				//int vertPolicy, horzPolicy;
				switch (scroll) {
					case Descriptor::SCROLL_HORIZONTAL:
						/*vertPolicy = JScrollPane.
							    VERTICAL_SCROLLBAR_NEVER;

						horzPolicy = JScrollPane.
							    HORIZONTAL_SCROLLBAR_ALWAYS;*/
						break;

					case Descriptor::SCROLL_VERTICAL:
						/*vertPolicy = JScrollPane.
							    VERTICAL_SCROLLBAR_ALWAYS;

						horzPolicy = JScrollPane.
							    HORIZONTAL_SCROLLBAR_NEVER;*/

						break;

					case Descriptor::SCROLL_BOTH:
						/*vertPolicy = JScrollPane.
							    VERTICAL_SCROLLBAR_ALWAYS;

						horzPolicy = JScrollPane.
							    HORIZONTAL_SCROLLBAR_ALWAYS;*/

						break;

					case Descriptor::SCROLL_AUTOMATIC:
						/*vertPolicy = JScrollPane.
							    VERTICAL_SCROLLBAR_AS_NEEDED;

						horzPolicy = JScrollPane.
							    HORIZONTAL_SCROLLBAR_AS_NEEDED;*/
						break;

					default:
						/*vertPolicy = JScrollPane.
							    VERTICAL_SCROLLBAR_NEVER;

						horzPolicy = JScrollPane.
							    HORIZONTAL_SCROLLBAR_NEVER;*/
						break;
				}

				/*scrollPane = new JScrollPane(vertPolicy, horzPolicy);
				scrollPane.getViewport().setBackground(bgColor);
				contentPane.add(scrollPane);*/
			}

		} else {
			cout << "FormatterRegion::prepareOutputDisplay Warning!";
			cout << "window != NULL" << endl;
		}

		/*if (component != null) {
			updateCurrentComponentSize();
			updateCurrentComponentLocation();
		}
		if (scrollPane != null) {
			scrollPane.setViewportView(currentComponent);
		}
		else {
			contentPane.add(currentComponent);
		}*/
		unlock();
		renderSurface(renderedSurface);
	}

	void FormatterRegion::renderSurface(ISurface* renderedSurface) {
		lock();
		if (renderedSurface != NULL && outputDisplay != NULL) {
			if (renderedSurface->setParent((void*)outputDisplay)) {
				outputDisplay->renderFrom(renderedSurface);
			}
		}
		unlock();
	}

	void FormatterRegion::showContent() {
		string value;
		CascadingDescriptor* desc;;
		/*struct transT* t;
		pthread_t threadId_;*/

		lockTransition();
		desc = ((CascadingDescriptor*)descriptor);
		value = desc->getParameterValue("visible");
		abortTransitionIn = false;
		abortTransitionOut = true;
		if (value == "" || value != "false") {
			vector<Transition*>* transitions;
			transitions = desc->getInputTransitions();

			if (!transitions->empty()) {
				Transition* transition;
				int transitionType;
				unsigned int i;
				for (i = 0; i < transitions->size(); i++) {
					transition = (*transitions)[i];
					transitionType = transition->getType();
					if (transitionType == Transition::TYPE_FADE) {
						toFront();

						fade(transition, true);
						unlockTransition();
						/*t = new struct transT;
						t->fr = this;
						t->t = transition;

						//show with fade transition type
						pthread_create(
								&threadId_,
								0, FormatterRegion::fadeT, (void*)t);

						pthread_detach(threadId_);*/
						return;

					} else if (transitionType == Transition::TYPE_BARWIPE) {
						toFront();

						barWipe(transition, true);
						unlockTransition();
						/*t = new struct transT;
						t->fr = this;
						t->t = transition;

						//show with barwipe transition type
						pthread_create(
								&threadId_,
								0, FormatterRegion::barWipeT, (void*)t);

						pthread_detach(threadId_);*/
						return;
					}
				}
			}

			unlockTransition();
			if(this->regionVisible){
				toFront();
				setRegionVisibility(true);
			}
			/*cout << "FormatterRegion::showContent '" << desc->getId();
			cout << "'" << endl;*/

		} else {
			unlockTransition();
		}
		/*if(this->regionVisible == false)
			this->setRegionVisibility(false);*/
	}

	void FormatterRegion::hideContent() {
		abortTransitionIn = true;
		abortTransitionOut = false;
		lockTransition();
		if (this->isVisible()) {
			vector<Transition*>* transitions;
			transitions = ((CascadingDescriptor*)descriptor)->
				    getOutputTransitions();

			if (!transitions->empty()) {
				unsigned int i;
				Transition* transition;
				int transitionType;

				for (i = 0; i < transitions->size(); i++) {
					transition = (*transitions)[i];
					transitionType = transition->getType();
					if (transitionType == Transition::TYPE_FADE) {
						//hide with transition type
						fade(transition, false);
						unlockTransition();
						return;

					} else if (transitionType == Transition::TYPE_BARWIPE) {
						//hide with transition type
						barWipe(transition, false);
						unlockTransition();
						return;
					}
				}
			}
			setRegionVisibility(false);
			abortTransitionIn = true;
			abortTransitionOut = true;
			disposeOutputDisplay();
			unlockTransition();

		} else {
			if (outputDisplay == NULL) {
				cout << "FormatterRegion::hideContent(" << this << ")";
				cout << " display is NULL" << endl;

			} else {
				abortTransitionIn = true;
				abortTransitionOut = true;
				disposeOutputDisplay();
			}
			unlockTransition();
		}
	}

	void FormatterRegion::setRegionVisibility(bool visible) {
		bool isVisible = this->isVisible();
		lock();
		if (outputDisplay != NULL) {
			if(visible != isVisible){
				if (!visible) {
					outputDisplay->hide();
				} else {
					outputDisplay->show();
				}
			}
		}
		else
			this->regionVisible = visible;
		unlock();
	}

	void FormatterRegion::disposeOutputDisplay() {
		lock();
		if (outputDisplay != NULL) {
			outputDisplay->hide();
			delete outputDisplay;
			outputDisplay = NULL;
		}
		unlock();
	}

	void FormatterRegion::toFront() {
		if (outputDisplay != NULL) {
			outputDisplay->raiseToTop();
			if (ncmRegion != NULL) {
				bringChildrenToFront(ncmRegion);
			}
			bringSiblingToFront(this);
		}
	}

	void FormatterRegion::bringChildrenToFront(LayoutRegion* parentRegion) {
		vector<LayoutRegion*>* regions = NULL;
		vector<LayoutRegion*>::iterator i;
		set<FormatterRegion*>* formRegions = NULL;
		set<FormatterRegion*>::iterator j;
		LayoutRegion* layoutRegion;
		FormatterRegion* region;

		if (parentRegion != NULL) {
			regions = parentRegion->getRegionsSortedByZIndex();
		}

		/*cout << endl << endl << endl;
		cout << "DEBUG REGIONS SORTED BY ZINDEX parentID: ";
		cout << parentRegion->getId() << endl;
		i = regions->begin();
		while (i != regions->end()) {
			cout << "region: " << (*i)->getId();
			cout << "zindex: " << (*i)->getZIndex();
			++i;
		}
		cout << endl << endl << endl;*/

		i = regions->begin();
		while (i != regions->end()) {
			layoutRegion = *i;

			if (layoutRegion != NULL) {
				bringChildrenToFront(layoutRegion);
				formRegions = ((FormatterLayout*)layoutManager)->
					    getFormatterRegionsFromNcmRegion(
					    	    layoutRegion->getId());

				if (formRegions != NULL) {
					j = formRegions->begin();
					while (j != formRegions->end()) {
						region = *j;
						if (region != NULL) {
							region->toFront();

						} else {
							cout << "FormatterRegion::";
							cout << "bringChildrenToFront";
							cout << " Warning! region == NULL";
						}

						++j;
					}

					delete formRegions;
					formRegions = NULL;
				}
			}
			++i;
		}

		delete regions;
		regions = NULL;
	}

	void FormatterRegion::traverseFormatterRegions(
		    LayoutRegion *region, LayoutRegion *baseRegion) {

		LayoutRegion* auxRegion;
		set<FormatterRegion*>* formRegions;
		FormatterRegion *formRegion;
		set<FormatterRegion*>::iterator it;

		formRegions = ((FormatterLayout*)layoutManager)->
			    getFormatterRegionsFromNcmRegion(region->getId());

		if (formRegions != NULL) {
			it = formRegions->begin();
			while (it != formRegions->end()) {
				formRegion = *it;
				if (formRegion != NULL) {
					auxRegion = formRegion->getLayoutRegion();
					if (ncmRegion != NULL) {
						if (ncmRegion->intersects(auxRegion) &&
								ncmRegion != auxRegion) {

							formRegion->toFront();
						}
					}

				} else {
					cout << "FormatterRegion::traverseFormatterRegion";
					cout << " Warning! formRegion == NULL" << endl;
				}
				/*
				cout << "FormatterRegion::traverseFormatterRegion toFront = ";
				cout << "'" << formRegion->getLayoutRegion()->getId();
				cout << "'" << endl;
				*/
				++it;
			}
			delete formRegions;
			formRegions = NULL;

		} else {
			bringHideWindowToFront(baseRegion, region);
		}
	}

	void FormatterRegion::bringHideWindowToFront(
		    LayoutRegion *baseRegion, LayoutRegion *hideRegion) {

		vector<LayoutRegion*> *regions;
		LayoutRegion *region;
		vector<LayoutRegion*>::iterator it;

		if (ncmRegion->intersects(hideRegion) && ncmRegion != hideRegion) {
			regions = hideRegion->getRegions();
			if (regions != NULL) {
				for (it = regions->begin(); it != regions->end(); ++it) {
					region = *it;
					traverseFormatterRegions(region, baseRegion);
				}
			}
			delete regions;
			regions = NULL;
		}
	}

	void FormatterRegion::bringSiblingToFront(FormatterRegion *region) {
		IWindow *regionRect;
		LayoutRegion *layoutRegion, *parentRegion, *baseRegion, *siblingRegion;
		vector<LayoutRegion*> *regions;
		vector<LayoutRegion*>::iterator it;

		regionRect = region->getRegionRectangle();
		layoutRegion = region->getOriginalRegion();
		if (layoutRegion == NULL) {
			return;
		}
		parentRegion = layoutRegion->getParent();
		baseRegion = layoutRegion;

		while (parentRegion != NULL) {
			regions = parentRegion->getRegionsOverRegion(baseRegion);
			for (it=regions->begin(); it!=regions->end(); ++it) {
				siblingRegion = *it;
				traverseFormatterRegions(siblingRegion, layoutRegion);
			}
			baseRegion = parentRegion;
			delete regions;
			regions = NULL;
			parentRegion = parentRegion->getParent();
		}
	}

	void FormatterRegion::windowGainedFocus() {
		if (ncmRegion != NULL) {
			bringChildrenToFront(ncmRegion);
		}
		bringSiblingToFront(this);
		lock();
		if (outputDisplay != NULL) {
			outputDisplay->validate();
		}
		unlock();
	}

	IWindow* FormatterRegion::getRegionRectangle() {
		//TODO: check if is this method usefull
		return NULL;
	}

	void FormatterRegion::setGhostRegion(bool ghost) {
		lock();
		if (outputDisplay != NULL) {
			outputDisplay->setGhostWindow(ghost);
		}
		unlock();
	}

	bool FormatterRegion::isVisible() {
		bool opaque = false;

		lock();
		if (outputDisplay != NULL) {
			opaque = outputDisplay->isVisible();
		}
		unlock();
		return opaque;
	}

	short FormatterRegion::getFocusState() {
		return focusState;
	}

	bool FormatterRegion::setSelection(bool selOn) {
		if (selOn && focusState == FormatterRegion::SELECTED) {
			return false;
		}

		if (selOn) {
			focusState = FormatterRegion::SELECTED;
			if (selComponentSrc != "") {
				ISurface* selSurface = NULL;
				selSurface = FocusSourceManager::getComponent(selComponentSrc);

				lock();
				if (selSurface != NULL && outputDisplay != NULL) {
					outputDisplay->renderFrom(selSurface);
					delete selSurface;
					selSurface = NULL;
				}
				unlock();
			}

			lock();
			if (outputDisplay != NULL) {
				lockFocusInfo();
				if (selComponentSrc == "") {
					outputDisplay->validate();
				}
				outputDisplay->setBorder(selBorderColor, selBorderWidth);
				unlockFocusInfo();
			}
			unlock();

		} else {
			unselect();
		}

		return selOn;
	}

	void FormatterRegion::setFocus(bool focusOn) {
		if (focusOn) {
			focusState = FormatterRegion::FOCUSED;
/*if (outputDisplay != NULL) {
	outputDisplay->clear();
}*/

/*			if (borderWidth > 0) {
				outputDisplay->setBounds(
						outputDisplay->getX() - borderWidth,
						outputDisplay->getY() - borderWidth,
						outputDisplay->getW() + (2 * borderWidth),
						outputDisplay->getH() + (2 * borderWidth));

				currentComponent.setLocation(
  					currentComponent.getX() + borderWidth,
  					currentComponent.getY() + borderWidth);

			} else if (borderWidth < 0) {
				currentComponent.setSize(
  					currentComponent.getWidth() + (2 * borderWidth),
    				currentComponent.getHeight() + (2* borderWidth));

	  			currentComponent.setLocation(
	  				currentComponent.getX() - borderWidth,
	  				currentComponent.getY() - borderWidth);
			}*/

			if (focusComponentSrc != "") {
				ISurface* focusSurface = NULL;
				focusSurface = FocusSourceManager::getComponent(
					    focusComponentSrc);

				lock();
				if (focusSurface != NULL && outputDisplay != NULL) {
					outputDisplay->renderFrom(focusSurface);
					delete focusSurface;
					focusSurface = NULL;
				}
				unlock();
			}

			lock();
			if (outputDisplay != NULL) {
				lockFocusInfo();
				if (focusComponentSrc == "") {
					outputDisplay->validate();
				}
				outputDisplay->setBorder(focusBorderColor, focusBorderWidth);
				unlockFocusInfo();
			}
			unlock();

		} else {
			unselect();
  		}
	}

	void FormatterRegion::unselect() {
		focusState = FormatterRegion::UNSELECTED;

		lock();
		if (outputDisplay != NULL) {
			outputDisplay->validate();
		}
		unlock();

/*if (outputDisplay != NULL) {
	outputDisplay->clear();
}*/

/*		cout << "FormatterRegion::unselect(" << this << ")" << endl;
		lock();
		if (outputDisplay == NULL) {
			unlock();
			return;
		}

		if (bgColor != NULL) {
			outputDisplay->setBackgroundColor(
				    bgColor->getR(),
				    bgColor->getG(),
				    bgColor->getB());

		} else {
			outputDisplay->setBackgroundColor(0, 0, 0);
		}
*/
		/*currentComponent.setLocation(
					currentComponent.getX() - borderWidth,
					currentComponent.getY() - borderWidth);
		}
		else if (borderWidth < 0){
			currentComponent.setLocation(
					currentComponent.getX() + borderWidth,
					currentComponent.getY() + borderWidth);

			currentComponent.setSize(
					currentComponent.getWidth() - (2 * borderWidth),
  				currentComponent.getHeight() - (2* borderWidth));
		}*/

/*
		outputDisplay->showContentSurface();
		outputDisplay->validate();
		unlock();
*/
	}

	void FormatterRegion::setFocusInfo(
		    Color* focusBorderColor,
		    int focusBorderWidth,
		    string focusComponentSrc,
		    Color* selBorderColor,
		    int selBorderWidth,
		    string selComponentSrc) {

		lockFocusInfo();
		if (this->focusBorderColor != NULL) {
			delete this->focusBorderColor;
			this->focusBorderColor = NULL;
		}

		this->focusBorderColor = new Color(
				focusBorderColor->getR(),
				focusBorderColor->getG(),
				focusBorderColor->getB());

		this->focusBorderWidth = focusBorderWidth;
		this->focusComponentSrc = focusComponentSrc;

		if (this->selBorderColor != NULL) {
			delete this->selBorderColor;
			this->selBorderColor = NULL;
		}

		this->selBorderColor = new Color(
				selBorderColor->getR(),
				selBorderColor->getG(),
				selBorderColor->getB());

		this->selBorderWidth = selBorderWidth;
		this->selComponentSrc = selComponentSrc;

		unlockFocusInfo();
	}

	string FormatterRegion::getFocusIndex() {
		return ((CascadingDescriptor*)descriptor)->getFocusIndex();
	}

	Color* FormatterRegion::getBackgroundColor() {
		return bgColor;
	}

	void FormatterRegion::barWipe(Transition* transition, bool isShowEffect) {
		int i, factor=1, x, y, width, height;
		double time, initTime;
		int transitionSubType, transparencyValue, initValue, endValue;
		double transitionDur, startProgress, endProgress;
		short transitionDir;

		transitionDur = transition->getDur();
		transitionSubType = transition->getSubtype();
		transitionDir = transition->getDirection();
		startProgress = transition->getStartProgress();
		endProgress = transition->getEndProgress();

		lock();
		if (outputDisplay == NULL) {
			cout << "FormatterRegion::barWipe(" << this << ")";
			cout << "Warning! return cause ";
			cout << "abortIn = '" << abortTransitionIn << "' and ";
			cout << "abortOut = '" << abortTransitionOut << "' and ";
			cout << "display = '" << outputDisplay << "' and ";
			cout << "isShow = '" << isShowEffect << "'" << endl;
			unlock();
			return;
		}

		transparencyValue = outputDisplay->getTransparencyValue();

		x = outputDisplay->getX();
		y = outputDisplay->getY();
		width = outputDisplay->getW();
		height = outputDisplay->getH();
		unlock();

		//outputDisplay->setStretch(false);
		initTime = getCurrentTimeMillis();

		//cout << transition->getStartProgress() << endl << endl;

		if (transitionSubType == Transition::SUBTYPE_BARWIPE_LEFTTORIGHT) {
			if (isShowEffect) {
				lock();
				if (outputDisplay != NULL) {
					outputDisplay->setCurrentTransparency(transparencyValue);
					outputDisplay->resize(1, height);
				}
				unlock();

				initValue = width*startProgress;
				i = initValue + 1;
				endValue = width*endProgress;
			} else {
				initValue = width*endProgress;
				endValue = width*startProgress;
				i = initValue-1;
			}

			while (true) {
				time = getCurrentTimeMillis();
				if (time >= initTime+transitionDur)
					break;
				lock();
				if (outputDisplay != NULL) {
					if (transitionDir == Transition::DIRECTION_REVERSE) {
						outputDisplay->setBounds(
							x + (width - i), y, i, height);

					} else {
						outputDisplay->resize(i, height);
					}
				} else {
					unlock();
					return;
				}
				unlock();

				i = getNextStepValue(
						initValue,
						endValue, factor, time, initTime, transitionDur, 0);

				lock();
				if (outputDisplay != NULL) {
					outputDisplay->validate();
				}
				unlock();

				if ((abortTransitionIn && isShowEffect) ||
						(abortTransitionOut && !isShowEffect)) {

					lock();
					if (outputDisplay != NULL) {
						outputDisplay->setBounds(x, y, width, height);
						unlock();
						setRegionVisibility(isShowEffect);
						//outputDisplay->setStretch(true);
						if (!isShowEffect) {
							disposeOutputDisplay();
						}

					} else {
						unlock();
					}
					return;
				}
			}

		} else if (transitionSubType ==
				Transition::SUBTYPE_BARWIPE_TOPTOBOTTOM) {

			if (isShowEffect) {
				lock();
				if (outputDisplay != NULL) {
					outputDisplay->setCurrentTransparency(transparencyValue);
					outputDisplay->resize(width, 1);
				}
				unlock();
				initValue = height*startProgress;
				i = initValue + 1;
				endValue = height*endProgress;

			} else {
				initValue = height*endProgress;
				endValue = height*startProgress;
				i = initValue-1;
			}

			cout << "i = " << i << " startProgress = ";
			cout << startProgress << " endProgress = " << endProgress << endl;
			cout << " initValue = " << initValue << " endValue = " << endValue;
			cout << endl;

			while (true) {
				time = getCurrentTimeMillis();
				if(time >= initTime + transitionDur)
					break;
				lock();
				if (outputDisplay != NULL) {
					if (transitionDir == Transition::DIRECTION_REVERSE) {
						outputDisplay->setBounds(
								x, y + (height - i), width, i);

					} else {
						outputDisplay->resize(width, i);
					}

				} else {
					unlock();
					return;
				}
				unlock();

				i = getNextStepValue(
						initValue,
						endValue,
						factor, time, initTime, transitionDur, 0);

				lock();
				if (outputDisplay != NULL) {
					outputDisplay->validate();
				}
				unlock();

				if ((abortTransitionIn && isShowEffect) ||
						(abortTransitionOut && !isShowEffect)) {

					lock();
					if (outputDisplay != NULL) {
						outputDisplay->setBounds(x, y, width, height);
						unlock();
						//outputDisplay->setStretch(true);
						if (!isShowEffect) {
							disposeOutputDisplay();
						}
					} else {
						unlock();
					}
					setRegionVisibility(isShowEffect);
					return;
				}
			}
		}

		if (!isShowEffect) {
			disposeOutputDisplay();

		} else {
			lock();
			if (outputDisplay != NULL) {
				//outputDisplay->setStretch(true);
				outputDisplay->setBounds(x, y, width, height);
				outputDisplay->validate();
			}
			unlock();
		}
	}

	void* FormatterRegion::barWipeT(void* ptr) {
		struct transT* trans;
		Transition* t;
		FormatterRegion* fr;

		trans = (struct transT*)ptr;
		fr = trans->fr;
		t = trans->t;

		delete trans;

		fr->barWipe(t, true);
		return NULL;
	}

	void FormatterRegion::fade(Transition* transition, bool isShowEffect) {
		int i, factor=1;
		double time, initTime, initValue, endValue, startProgress, endProgress;
		int transparencyValue;
		double transitionDur;

		lock();
		if (outputDisplay == NULL) {
			cout << "FormatterRegion::fade(" << this << ")";
			cout << "Warning! return cause ";
			cout << "abortIn = '" << abortTransitionIn << "' and ";
			cout << "abortOut = '" << abortTransitionOut << "' and ";
			cout << "display = '" << outputDisplay << "' and ";
			cout << "isShow = '" << isShowEffect << "'" << endl;
			unlock();
			return;
		}

		transparencyValue = outputDisplay->getTransparencyValue();
		unlock();
		transitionDur = transition->getDur();
		startProgress = transition->getStartProgress();
		endProgress = transition->getEndProgress();

		if (isShowEffect) {
			initValue = transparencyValue*startProgress;
			endValue = transparencyValue*endProgress;
			i = initValue+1;

		} else {
			initValue = transparencyValue*endProgress;
			endValue = transparencyValue*startProgress;
			i = initValue+1; // TODO: confirm if + or -
		}

		initTime = getCurrentTimeMillis();
		while (true) {
			time = getCurrentTimeMillis();
			if(time >= initTime + transitionDur)
				break;
			lock();
			if (outputDisplay != NULL) {
				outputDisplay->setCurrentTransparency(i);

			} else {
				unlock();
				return;
			}
			unlock();

			i = getNextStepValue(
					initValue,
					endValue,
					factor,
					time, initTime, transitionDur, 0);

			if ((abortTransitionIn && isShowEffect) ||
					(abortTransitionOut && !isShowEffect)) {

				break;
			}
		}

		if (!isShowEffect) {
			disposeOutputDisplay();

		} else {
			setRegionVisibility(isShowEffect);
		}
	}

	void* FormatterRegion::fadeT(void* ptr) {
		struct transT* trans;
		Transition* t;
		FormatterRegion* fr;

		trans = (struct transT*)ptr;
		fr = trans->fr;
		t = trans->t;

		delete trans;

		fr->fade(t, true);
		return NULL;
	}

	/*void changeCurrentComponent(Component newComponent) {
		if (newComponent != null && currentComponent != null &&
				outputDisplay != null) {
			newComponent.setBounds(currentComponent.getBounds());
			outputDisplay.remove(currentComponent);
			outputDisplay.add(newComponent);
			currentComponent = newComponent;
			outputDisplay.validate();
		}
	}*/

	void FormatterRegion::lock() {
		pthread_mutex_lock(&mutex);
	}

	void FormatterRegion::unlock() {
		pthread_mutex_unlock(&mutex);
	}

	void FormatterRegion::lockTransition() {
		pthread_mutex_lock(&mutexT);
	}

	void FormatterRegion::unlockTransition() {
		pthread_mutex_unlock(&mutexT);
	}

	void FormatterRegion::lockFocusInfo() {
		pthread_mutex_lock(&mutexFI);
	}

	void FormatterRegion::unlockFocusInfo() {
		pthread_mutex_unlock(&mutexFI);
	}

	void FormatterRegion::setTransparency(string strTrans){
		float trans;
		if(strTrans == "")
			trans = 1.0;
		else trans = stof(strTrans);

		setTransparency(trans);

	}

	float FormatterRegion::getTransparency(){
		return this->transparency;
	}

	void FormatterRegion::setTransparency(float transparency){
		lock();
		if (transparency < 0.0 || transparency > 1.0) {
			this->transparency = 1.0;
		} else
			this->transparency = transparency;

		/*cout << "FormatterRegion::setTransparency : calling with value ";
		cout << transparency << endl;*/

		if (outputDisplay != NULL) {
			outputDisplay->setCurrentTransparency(
					(int)(this->transparency * 255));
		}

		unlock();
	}

	void FormatterRegion::setBackgroundColor(string color) {
		if (util::trim(color) != "") {
			if (color == "transparent") {
				this->bgColor = NULL;
			} else {
				this->bgColor = new Color(color, (int)(transparency * 255));
			}
		} else {
			this->bgColor = NULL;
		}
	}

	void FormatterRegion::setBackgroundColor(Color *color) {
		this->bgColor = color;
	}

	void FormatterRegion::setChromaKey(string value) {
		if (value != "") {
			if (value == "black") {
				this->chromaKey = new Color(0, 0, 0, 0);
			} else {
				this->chromaKey = new Color(value);
			}
		}
	}

	void FormatterRegion::setRgbChromaKey(string value) {
		if (value != "") {
			vector<string>* params;

			params = split(value, ",");
			if (params->size() == 3) {
				this->chromaKey = new Color(
						(int)stof((*params)[0]),
						(int)stof((*params)[1]),
						(int)stof((*params)[2]));
			}
			delete params;
		}
	}

	void FormatterRegion::setFit(string value) {
		if (value != "") {
			setFit(DescriptorUtil::getFitCode(value));

		} else {
			setFit(Descriptor::FIT_FILL);
		}
	}

	void FormatterRegion::setFit(short value) {
		if (value < 0) {
			fit = Descriptor::FIT_FILL;

		} else {
			fit = value;
		}
	}

	void FormatterRegion::setScroll(string value) {
		if (value != "") {
			setScroll(DescriptorUtil::getScrollCode(value));

		} else {
			setScroll(Descriptor::SCROLL_NONE);
		}
	}

	void FormatterRegion::setScroll(short value) {
		if (value < 0) {
			scroll = Descriptor::SCROLL_NONE;

		} else {
			scroll = value;
		}
	}
}
}
}
}
}
}
}
