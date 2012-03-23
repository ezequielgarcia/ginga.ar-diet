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

#include <boost/filesystem.hpp>

#include "generated/config.h"
#include "system/util/Color.h"
#include "system/io/interface/content/text/IFontProvider.h"
#include "system/io/interface/output/ISurface.h"

#include "../../include/TextPlayer.h"

#include "../../include/PlayersComponentSupport.h"

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace player {
	TextPlayer::TextPlayer() : Player("") {
   		initializePlayer();
   	}

	TextPlayer::~TextPlayer() {
		wclog << "TextPlayer::~TextPlayer" << endl;

   		if (font != NULL) {
   			delete font;
   			font = NULL;
   		}

   		if (fontColor != NULL) {
   			delete fontColor;
   			fontColor = NULL;
   		}

   		if (bgColor != NULL) {
   			delete bgColor;
   			bgColor = NULL;
   		}
		/*
		 * TODO: is some class deleting this
		 * if (renderSurface != NULL) {
			delete renderSurface;
			renderSurface = NULL;
		}*/
   	}

	void TextPlayer::initializePlayer() {
		this->bgColor = NULL;
		this->fontHeight = 0;
		this->currentLine = 0;
		this->currentColumn = 0;
		this->tabSize = 0;
		this->font = NULL;
		this->fontUri = "";
		this->fontColor = NULL;
		this->fontSize = 12;
		this->isBold = false;
		this->style = "Normal";
		this->surface = new DFBSurface();

		setFont(PREFIX_PATH "/player/fonts/vera.ttf");
		this->setFontFamily(PREFIX_PATH "/player/fonts/vera");
		
		if (this->surface != NULL) {
			this->surface->setCaps(
					this->surface->getCap("ALPHACHANNEL"));
		}
	}

	int TextPlayer::write(
			ISurface* s,
			string text,
			string fontUri,
			int fontSize, IColor* fontColor) {

		string aux;
		aux = PREFIX_PATH "/player/fonts/decker.ttf";
		if (fontSize < 1 || s == NULL || text == "") {
			return 0;
		}

		if (fontUri == "" || !fileExists(fontUri)) {
			if (!fileExists(aux)) {
				cout << "TextPlayer::write Warning! File not found: '";
				cout << fontUri.c_str() << "'" << endl;
				return 0;
			}

		} else {
			aux = fontUri;
		}

		IFontProvider* font = NULL;
		int width;

		font = new DFBFontProvider(aux.c_str(), fontSize);

		if (fontColor == NULL) {
			fontColor = new Color("black");
		}

		if (font != NULL) {
			s->setColor(fontColor);
			s->setFont(font);

			width = font->getStringWidth(text.c_str(), strlen((const char*)(text.c_str())));

			font->playOver((void*)s, text.c_str());
			delete font;
			font = NULL;
		}

		delete fontColor;
		fontColor = NULL;

		return width;
	}
	
	void TextPlayer::setFontFamily(std::string aFont) {
		this->fontFamily = aFont;
	}

	bool TextPlayer::setFont(string someUri) {

		if(boost::filesystem::exists(someUri)) {
			this->fontUri = someUri;

			delete font;
			font = new DFBFontProvider(this->fontUri.c_str(), fontSize);

			
			if (font->getContent() == NULL) {
				cout << "TextPlayer::setFont Warning! Can't create Font "<< this->fontUri << endl;
				return false;
			}
			fontHeight = font->getHeight();
		}else{
			cout << "TextPlayer::setFont warning! file does not exist: "<< someUri << endl;
			return false;
		}
		
		return true;
	}

	void TextPlayer::setFontSize(int size) {
		fontSize = size;
		setFont(fontUri);
	}

	int TextPlayer::getFontSize() {
		return fontSize;
	}

	int TextPlayer::getFontHeight() {
		return fontHeight;
	}

	bool TextPlayer::getBold(){
		return isBold;
	}

	void TextPlayer::setBold(bool bold){
		isBold = bold;
		setFont(fontFamily + getFontProperties());
	}

	string TextPlayer::getStyle(){
		return style;
	}

	void TextPlayer::setStyle(string newStyle){
		style = newStyle;
		setFont(fontFamily + getFontProperties());
	}
	
	std::string TextPlayer::getFontProperties(){
		if (isBold and style=="Normal")
			return "-Bold.ttf";
		else if (isBold and style!="Normal")
			return "-Bold" + style + ".ttf";
		else if (style != "Normal")
			return "-" + style + ".ttf";
		else return ".ttf";
	}

	void TextPlayer::setBgColor(int red, int green, int blue, int alpha) {
		if (this->bgColor != NULL) {
			delete this->bgColor;
			this->bgColor = NULL;
		}

		this->bgColor = new Color(red, green, blue, alpha);
		if (this->surface != NULL) {
			surface->setBgColor(bgColor);
		}
	}

	void TextPlayer::setColor(int red, int green, int blue, int alpha) {
		if (this->fontColor != NULL) {
			delete this->fontColor;
			this->fontColor = NULL;
		}

		this->fontColor = new Color(red, green, blue, alpha);
	}

	void TextPlayer::setTabSize(int size) {
		this->tabSize = size;
	}

	int TextPlayer::getTabSize() {
		return tabSize;
	}

  	void TextPlayer::drawText(string text, short align) {

		string uri;
		int textWidth, surWidth, surHeight;

		unsigned int maxToDraw;
		string::size_type splitPos, len;
		unsigned int widthAverage;
		bool space;
		int maxAdvance, oldTextWidth;
		string splited;

		if (font == NULL) {
			cout << "TextPlayer::drawText Warning! can't set font" << endl;
			return;

		} else {
			surface->setFont(font);
		}

		if (fontColor == NULL) {
			fontColor = new Color("black");
		}

		surface->setColor(fontColor);
		if (font != NULL && surface != NULL && surface->getContent() != NULL) {
			surface->getSize(&surWidth, &surHeight);
			textWidth = font->getStringWidth(
					text.c_str(), strlen((const char*)(text.c_str())));

            if ( (align & 0x00000008) and (currentLine == 0))
                currentLine += fontHeight;

			if ((currentColumn + textWidth) > surWidth) {
				space = false;
				maxAdvance = font->getMaxAdvance();

				widthAverage = (int)(textWidth / text.length());
				if (textWidth > surWidth)
                    maxToDraw = (int)(((surWidth) / widthAverage) * 0.85);
                else
                    maxToDraw = (int) (surWidth - currentColumn)/widthAverage;

				len = text.length();
				splited = text.substr(0, maxToDraw);
				splitPos = splited.find_last_of(" ");

				if (splitPos == std::string::npos) {
					splitPos = maxToDraw;
                    splited = text.substr(splitPos, len - splitPos);
				} else {
					splitPos++;
					splited = text.substr(splitPos, len - splitPos);
					space = true;
				}

				text = text.substr(0, splitPos);

				textWidth = font->getStringWidth(text.c_str());
				while (textWidth > surWidth) {
					if (space) {
						splited = " " + splited;
					}

					len = text.length();
					splitPos = text.find_last_of(" ");
					if (splitPos == std::string::npos) {
						splited = text[len] + splited;
						text = text.substr(0, len - 1);
						space = false;

					} else {
						splitPos++;
						splited = text.substr(
								splitPos, len - splitPos) + splited;

						text = text.substr(0, splitPos);
						space = true;
					}

					oldTextWidth = textWidth;
					textWidth = font->getStringWidth(text.c_str());

					if (oldTextWidth == textWidth) {
						break;
					}
				}

				font->playOver(
						(void*)surface, text.c_str(),
						currentColumn, currentLine, align);

				/*if (align == A_TOP_CENTER) {
					DFBCHECK(surface->getSurface()->DrawString(
					    surface->getSurface(),
					    text.c_str(),
					    -1,
					    (int)(surWidth / 2),
					    currentLine, (DFBSurfaceTextFlags)(align)));

				} else if (align == A_CENTER) {
					DFBCHECK(surface->getSurface()->DrawString(
					    surface->getSurface(),
					    text.c_str(),
					    -1,
					    (int)(surWidth / 2),
					    (int)((surHeight + currentLine) / 2),
					    (DFBSurfaceTextFlags)(align)));

				} else if (align == A_BOTTOM_CENTER) {
					DFBCHECK(surface->getSurface()->DrawString(
					    surface->getSurface(),
					    text.c_str(),
					    -1,
					    (int)(surWidth / 2),
					    (int)((surHeight - currentLine) / 2),
					    (DFBSurfaceTextFlags)(align)));

				} else {
					DFBCHECK(surface->getSurface()->DrawString(
					    surface->getSurface(),
					    text.c_str(),
					    -1,
					    currentColumn,
					    currentLine, (DFBSurfaceTextFlags)(align)));
				}*/

				breakLine();
				drawText(splited, align);

			} else {

				font->playOver(
						(void*)surface, text.c_str(),
						currentColumn, currentLine, align);
/*
				if (align == A_TOP_CENTER) {

					DFBCHECK(surface->getSurface()->DrawString(
					    surface->getSurface(),
					    text.c_str(),
					    -1,
					    (int)(surWidth / 2),
					    currentLine, (DFBSurfaceTextFlags)(align)));

				} else if (align == A_CENTER) {
					DFBCHECK(surface->getSurface()->DrawString(
					    surface->getSurface(),
					    text.c_str(),
					    -1,
					    (int)(surWidth / 2),
					    (int)((surHeight + currentLine) / 2),
					    (DFBSurfaceTextFlags)(align)));

				} else if (align == A_BOTTOM_CENTER) {
					DFBCHECK(surface->getSurface()->DrawString(
					    surface->getSurface(),
					    text.c_str(),
					    -1,
					    (int)(surWidth / 2),
					    (int)((surHeight - currentLine) / 2),
					    (DFBSurfaceTextFlags)(align)));

				} else {
					DFBCHECK(surface->getSurface()->DrawString(
					    surface->getSurface(),
					    text.c_str(),
					    -1,
					    currentColumn,
					    currentLine, (DFBSurfaceTextFlags)(align)));
				}
*/
				currentColumn += textWidth;
			}

		} else {
			cout << "TextPlayer::drawText Warning! FontProvider(" << font;
			cout << ") or Surface(" << surface << ") = NULL";
			cout << endl;
		}
	}

	void TextPlayer::drawTextLn(string text, short align) {
		drawText(text, align);
		breakLine();
	}

	void TextPlayer::tab() {
		currentColumn = currentColumn + (tabSize * 12);
	}

	void TextPlayer::breakLine() {
		int w, h;

		surface->getSize(&w, &h);
		if ((currentLine + fontHeight) > h) {
			cout << "TextPlayer::breakLine() Warning! Exceed surface bounds";
			cout << " currentLine = '" << currentLine << "'";
			cout << " fontHeight = '" << fontHeight << "'";
			cout << " surH = '" << h << "'" << endl;
			currentColumn = 0;

		} else {
			currentLine = currentLine + (int)(0.9 * fontHeight);
			currentColumn = 0;
		}
	}

	int TextPlayer::getCurrentColumn() {
		return this->currentColumn;
	}

	int TextPlayer::getCurrentLine() {
		return this->currentLine;
	}

	void TextPlayer::setPropertyValue(
			string name, string value, double duration, double by) {

		//TODO: set font url, font size, font color, ...
		Player::setPropertyValue(name, value, duration, by);
	}
}
}
}
}
}
}

extern "C" int renderText(
		::br::pucrio::telemidia::ginga::core::system::io::ISurface* s,
		string text, string fontUri, int fontSize, IColor* fontColor) {

	return ::br::pucrio::telemidia::ginga::core::player::TextPlayer::write(
			s, text, fontUri, fontSize, fontColor);
}

