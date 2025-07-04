/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/tokenizer.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/surface.h"
#include "graphics/fonts/ttf.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/text/text.h"
#include "zvision/text/truetype_font.h"

namespace ZVision {

TextStyleState::TextStyleState() {
	_fontname = "Arial";
	_blue = 255;
	_green = 255;
	_red = 255;
	_bold = false;
#if 0
	_newline = false;
	_escapement = 0;
#endif
	_italic = false;
	_justification = TEXT_JUSTIFY_LEFT;
	_size = 12;
#if 0
	_skipcolor = false;
#endif
	_strikeout = false;
	_underline = false;
	_statebox = 0;
	_sharp = false;
}

TextChange TextStyleState::parseStyle(const Common::String &str, int16 len) {
	Common::String buf = Common::String(str.c_str(), len);

	uint retval = TEXT_CHANGE_NONE;

	Common::StringTokenizer tokenizer(buf, " ");
	Common::String token;

	while (!tokenizer.empty()) {
		token = tokenizer.nextToken();

		if (token.matchString("font", true)) {
			token = tokenizer.nextToken();
			if (token[0] == '"') {
				Common::String _tmp = Common::String(token.c_str() + 1);

				while (token.lastChar() != '"' && !tokenizer.empty()) {
					token = tokenizer.nextToken();
					_tmp += " " + token;
				}

				if (_tmp.lastChar() == '"')
					_tmp.deleteLastChar();

				_fontname = _tmp;
			} else {
				if (!tokenizer.empty())
					_fontname = token;
			}
			retval |= TEXT_CHANGE_FONT_TYPE;

		} else if (token.matchString("blue", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				int32 tmp = atoi(token.c_str());
				if (_blue != tmp) {
					_blue = tmp;
					retval |= TEXT_CHANGE_FONT_STYLE;
				}
			}
		} else if (token.matchString("red", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				int32 tmp = atoi(token.c_str());
				if (_red != tmp) {
					_red = tmp;
					retval |= TEXT_CHANGE_FONT_STYLE;
				}
			}
		} else if (token.matchString("green", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				int32 tmp = atoi(token.c_str());
				if (_green != tmp) {
					_green = tmp;
					retval |= TEXT_CHANGE_FONT_STYLE;
				}
			}
		} else if (token.matchString("newline", true)) {
#if 0
			if ((retval & TXT_RET_NEWLN) == 0)
				_newline = 0;

			_newline++;
#endif
			retval |= TEXT_CHANGE_NEWLINE;
		} else if (token.matchString("point", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				int32 tmp = atoi(token.c_str());
				if (_size != tmp) {
					_size = tmp;
					retval |= TEXT_CHANGE_FONT_TYPE;
				}
			}
		} else if (token.matchString("escapement", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
#if 0
				int32 tmp = atoi(token.c_str());
				_escapement = tmp;
#endif
			}
		} else if (token.matchString("italic", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("on", true)) {
					if (_italic != true) {
						_italic = true;
						retval |= TEXT_CHANGE_FONT_STYLE;
					}
				} else if (token.matchString("off", true)) {
					if (_italic != false) {
						_italic = false;
						retval |= TEXT_CHANGE_FONT_STYLE;
					}
				}
			}
		} else if (token.matchString("underline", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("on", true)) {
					if (_underline != true) {
						_underline = true;
						retval |= TEXT_CHANGE_FONT_STYLE;
					}
				} else if (token.matchString("off", true)) {
					if (_underline != false) {
						_underline = false;
						retval |= TEXT_CHANGE_FONT_STYLE;
					}
				}
			}
		} else if (token.matchString("strikeout", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("on", true)) {
					if (_strikeout != true) {
						_strikeout = true;
						retval |= TEXT_CHANGE_FONT_STYLE;
					}
				} else if (token.matchString("off", true)) {
					if (_strikeout != false) {
						_strikeout = false;
						retval |= TEXT_CHANGE_FONT_STYLE;
					}
				}
			}
		} else if (token.matchString("bold", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("on", true)) {
					if (_bold != true) {
						_bold = true;
						retval |= TEXT_CHANGE_FONT_STYLE;
					}
				} else if (token.matchString("off", true)) {
					if (_bold != false) {
						_bold = false;
						retval |= TEXT_CHANGE_FONT_STYLE;
					}
				}
			}
		} else if (token.matchString("skipcolor", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
#if 0
				if (token.matchString("on", true)) {
					_skipcolor = true;
				} else if (token.matchString("off", true)) {
					_skipcolor = false;
				}
#endif
			}
		} else if (token.matchString("image", true)) {
			// Not used
		} else if (token.matchString("statebox", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				_statebox = atoi(token.c_str());
				retval |= TEXT_CHANGE_HAS_STATE_BOX;
			}
		} else if (token.matchString("justify", true)) {
			if (!tokenizer.empty()) {
				token = tokenizer.nextToken();
				if (token.matchString("center", true))
					_justification = TEXT_JUSTIFY_CENTER;
				else if (token.matchString("left", true))
					_justification = TEXT_JUSTIFY_LEFT;
				else if (token.matchString("right", true))
					_justification = TEXT_JUSTIFY_RIGHT;
			}
		}
	}
	return (TextChange)retval;
}

void TextStyleState::readAllStyles(const Common::String &txt) {
	int16 startTextPosition = -1;
	int16 endTextPosition = -1;

	for (uint16 i = 0; i < txt.size(); i++) {
		if (txt[i] == '<')
			startTextPosition = i;
		else if (txt[i] == '>') {
			endTextPosition = i;
			if (startTextPosition != -1) {
				if ((endTextPosition - startTextPosition - 1) > 0) {
					parseStyle(Common::String(txt.c_str() + startTextPosition + 1), endTextPosition - startTextPosition - 1);
				}
			}
		}

	}
}

void TextStyleState::updateFontWithTextState(StyledTTFont &font) {
	uint tempStyle = 0;

	if (_bold) {
		tempStyle |= StyledTTFont::TTF_STYLE_BOLD;
	}
	if (_italic) {
		tempStyle |= StyledTTFont::TTF_STYLE_ITALIC;
	}
	if (_underline) {
		tempStyle |= StyledTTFont::TTF_STYLE_UNDERLINE;
	}
	if (_strikeout) {
		tempStyle |= StyledTTFont::TTF_STYLE_STRIKETHROUGH;
	}
	if (_sharp) {
		tempStyle |= StyledTTFont::TTF_STYLE_SHARP;
	}

	font.loadFont(_fontname, _size, tempStyle);
}

void TextRenderer::drawTextWithJustification(const Common::String &text, StyledTTFont &font, uint32 color, Graphics::Surface &dest, int lineY, TextJustification justify) {
	switch (justify) {
	case TEXT_JUSTIFY_LEFT :
		font.drawString(&dest, text, 0, lineY, dest.w, color, Graphics::kTextAlignLeft);
		break;
	case TEXT_JUSTIFY_CENTER :
		font.drawString(&dest, text, 0, lineY, dest.w, color, Graphics::kTextAlignCenter);
		break;
	case TEXT_JUSTIFY_RIGHT :
		font.drawString(&dest, text, 0, lineY, dest.w, color, Graphics::kTextAlignRight);
		break;
	}
}

int32 TextRenderer::drawText(const Common::String &text, TextStyleState &state, Graphics::Surface &dest) {
	StyledTTFont font(_engine);
	state.updateFontWithTextState(font);

	uint32 color = _engine->_resourcePixelFormat.RGBToColor(state._red, state._green, state._blue);
	drawTextWithJustification(text, font, color, dest, 0, state._justification);

	return font.getStringWidth(text);
}

struct TextSurface {
	TextSurface(Graphics::Surface *surface, Common::Point surfaceOffset, uint lineNumber)
		: _surface(surface),
		  _surfaceOffset(surfaceOffset),
		  _lineNumber(lineNumber) {
	}

	Graphics::Surface *_surface;
	Common::Point _surfaceOffset;
	uint _lineNumber;
};

void TextRenderer::drawTextWithWordWrapping(const Common::String &text, Graphics::Surface &dest, bool blackFrame) {
	Common::Array<TextSurface> textSurfaces;
	Common::Array<uint> lineWidths;
	Common::Array<TextJustification> lineJustifications;

	// Create the initial text state
	TextStyleState currentState;

	// Create an empty font and bind it to the state
	StyledTTFont font(_engine);
	currentState.updateFontWithTextState(font);

	Common::String currentSentence; // Not a true 'grammatical' sentence. Rather, it's just a collection of words
	Common::String currentWord;
	int sentenceWidth = 0;
	int wordWidth = 0;
	int lineWidth = 0;
	int lineHeight = font.getFontHeight();

	uint currentLineNumber = 0u;

	uint numSpaces = 0u;
	int spaceWidth = 0;

	// The pixel offset to the currentSentence
	Common::Point sentencePixelOffset;

	uint i = 0u;
	uint stringlen = text.size();

	// Parse entirety of supplied text
	while (i < stringlen) {
		// Style tag encountered?
		if (text[i] == '<') {
			// Flush the currentWord to the currentSentence
			currentSentence += currentWord;
			sentenceWidth += wordWidth;

			// Reset the word variables
			currentWord.clear();
			wordWidth = 0;

			// Parse the style tag
			uint startTextPosition = i;
			while (i < stringlen && text[i] != '>') {
				++i;
			}
			uint endTextPosition = i;

			uint32 textColor = currentState.getTextColor(_engine);

			uint stateChanges = 0u;
			if ((endTextPosition - startTextPosition - 1) > 0) {
				stateChanges = currentState.parseStyle(Common::String(text.c_str() + startTextPosition + 1), endTextPosition - startTextPosition - 1);
			}

			if (stateChanges & (TEXT_CHANGE_FONT_TYPE | TEXT_CHANGE_FONT_STYLE)) {
				// Use the last state to render out the current sentence
				// Styles apply to the text 'after' them
				if (!currentSentence.empty()) {
					textSurfaces.push_back(TextSurface(font.renderSolidText(currentSentence, textColor), sentencePixelOffset, currentLineNumber));

					lineWidth += sentenceWidth;
					sentencePixelOffset.x += sentenceWidth;

					// Reset the sentence variables
					currentSentence.clear();
					sentenceWidth = 0;
				}

				// Update the current state with the style information
				currentState.updateFontWithTextState(font);

				lineHeight = MAX(lineHeight, font.getFontHeight());
				spaceWidth = font.getCharWidth(' ');
			}
			if (stateChanges & TEXT_CHANGE_NEWLINE) {
				// If the current sentence has content, render it out
				if (!currentSentence.empty()) {
					textSurfaces.push_back(TextSurface(font.renderSolidText(currentSentence, textColor), sentencePixelOffset, currentLineNumber));
				}

				// Set line width
				lineWidths.push_back(lineWidth + sentenceWidth - (numSpaces * spaceWidth));

				currentSentence.clear();
				sentenceWidth = 0;

				// Update the offsets
				sentencePixelOffset.x = 0u;
				sentencePixelOffset.y += lineHeight;

				// Reset the line variables
				lineHeight = font.getFontHeight();
				lineWidth = 0;
				++currentLineNumber;
				lineJustifications.push_back(currentState._justification);
			}
			if (stateChanges & TEXT_CHANGE_HAS_STATE_BOX) {
				Common::String temp = Common::String::format("%d", _engine->getScriptManager()->getStateValue(currentState._statebox));
				wordWidth += font.getStringWidth(temp);

				// If the word causes the line to overflow, render the sentence and start a new line
				if (lineWidth + sentenceWidth + wordWidth > dest.w) {
					textSurfaces.push_back(TextSurface(font.renderSolidText(currentSentence, textColor), sentencePixelOffset, currentLineNumber));

					// Set line width
					lineWidths.push_back(lineWidth + sentenceWidth - (numSpaces * spaceWidth));

					currentSentence.clear();
					sentenceWidth = 0;

					// Update the offsets
					sentencePixelOffset.x = 0u;
					sentencePixelOffset.y += lineHeight;

					// Reset the line variables
					lineHeight = font.getFontHeight();
					lineWidth = 0;
					++currentLineNumber;
					lineJustifications.push_back(currentState._justification);
				}
			}
		} else {
			currentWord += text[i];
			wordWidth += font.getCharWidth(text[i]);

			if (text[i] == ' ') {
				// When we hit the first space, flush the current word to the sentence
				if (!currentWord.empty()) {
					currentSentence += currentWord;
					sentenceWidth += wordWidth;

					currentWord.clear();
					wordWidth = 0;
				}

				// We track the number of spaces so we can disregard their width in lineWidth calculations
				++numSpaces;
			} else {
				// If the word causes the line to overflow, render the sentence and start a new line
				if (lineWidth + sentenceWidth + wordWidth > dest.w) {
					// Only render out content
					if (!currentSentence.empty()) {
						textSurfaces.push_back(TextSurface(font.renderSolidText(currentSentence, currentState.getTextColor(_engine)), sentencePixelOffset, currentLineNumber));
					}

					// Set line width
					lineWidths.push_back(lineWidth + sentenceWidth - (numSpaces * spaceWidth));

					currentSentence.clear();
					sentenceWidth = 0;

					// Update the offsets
					sentencePixelOffset.x = 0u;
					sentencePixelOffset.y += lineHeight;

					// Reset the line variables
					lineHeight = font.getFontHeight();
					lineWidth = 0;
					++currentLineNumber;
					lineJustifications.push_back(currentState._justification);
				}

				numSpaces = 0u;
			}
		}

		i++;
	}

	// Render out any remaining words/sentences
	if (!currentWord.empty() || !currentSentence.empty()) {
		currentSentence += currentWord;
		sentenceWidth += wordWidth;

		textSurfaces.push_back(TextSurface(font.renderSolidText(currentSentence, currentState.getTextColor(_engine)), sentencePixelOffset, currentLineNumber));
	}

	lineWidths.push_back(lineWidth + sentenceWidth);
	lineJustifications.push_back(currentState._justification);

	for (Common::Array<TextSurface>::iterator iter = textSurfaces.begin(); iter != textSurfaces.end(); ++iter) {
		Common::Rect empty;
		int16 Xpos = iter->_surfaceOffset.x;
		switch (lineJustifications[iter->_lineNumber]) {
		case TEXT_JUSTIFY_LEFT :
			break;
		case TEXT_JUSTIFY_CENTER :
			Xpos += ((dest.w - lineWidths[iter->_lineNumber]) / 2);
			break;
		case TEXT_JUSTIFY_RIGHT :
			Xpos += dest.w - lineWidths[iter->_lineNumber];
			break;
		}
		if (blackFrame)
			_engine->getRenderManager()->blitSurfaceToSurface(*iter->_surface, empty, dest, Xpos, iter->_surfaceOffset.y);
		else
			_engine->getRenderManager()->blitSurfaceToSurface(*iter->_surface, empty, dest, Xpos, iter->_surfaceOffset.y, 0);
		// Release memory
		iter->_surface->free();
		delete iter->_surface;
	}
}

Common::U32String readWideLine(Common::SeekableReadStream &stream) {
	Common::U32String asciiString;

	while (true) {
		uint32 value = stream.readUint16LE();
		if (stream.eos())
			break;
		// Check for CRLF
		if (value == 0x0A0D) {
			// Read in the extra NULL char
			stream.readByte(); // \0
			// End of the line. Break
			break;
		}

		asciiString += value;
	}
	return asciiString;
}

} // End of namespace ZVision
