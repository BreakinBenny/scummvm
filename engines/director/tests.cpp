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

#include "common/config-manager.h"
#include "common/system.h"
#include "common/compression/deflate.h"

#include "common/memstream.h"
#include "common/macresman.h"
#include "common/formats/cue.h"

#include "graphics/fonts/macfont.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"

#include "engines/util.h"

#include "director/director.h"
#include "director/archive.h"
#include "director/movie.h"
#include "director/picture.h"
#include "director/window.h"

#include "image/pict.h"

namespace Director {

//////////////////////
// Graphics tests
//////////////////////
void Window::testFontScaling() {
	int x = 10;
	int y = 10;
	int w = g_system->getWidth();
	int h = g_system->getHeight();

	_vm->setPalette(CastMemberID(kClutSystemMac, -1));

	Graphics::ManagedSurface surface;

	surface.create(w, h, _wm->_pixelformat);
	surface.clear(_wm->_colorWhite);

	Graphics::MacFont origFont(Graphics::kMacFontNewYork, 18);

	const Graphics::MacFONTFont *font1 = (const Graphics::MacFONTFont *)_wm->_fontMan->getFont(origFont);

	Graphics::MacFONTFont::testBlit(font1, &surface, 0xff, x, y + 200, 500);

	Graphics::MacFont bigFont(Graphics::kMacFontSystem, 12);

	font1 = (const Graphics::MacFONTFont *)_wm->_fontMan->getFont(bigFont);

	Graphics::MacFONTFont::testBlit(font1, &surface, 0xff, x, y + 50 + 170, w - 10);

	const char *text = "d";

	for (int i = 9; i <= 20; i++) {
		Graphics::MacFont macFont(Graphics::kMacFontNewYork, i);

		const Graphics::Font *font = _wm->_fontMan->getFont(macFont);

		int width = font->getStringWidth(text);

		Common::Rect bbox = font->getBoundingBox(text, x, y, w);
		surface.frameRect(bbox, 15);

		font->drawString(&surface, text, x, y, width, 0xFF);

		x += width + 1;
	}

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			int y1 = 80 + i * 7;
			int x1 = 80 + j * 7;

			for (x = x1; x < x1 + 6; x++)
				for (y = y1; y < y1 + 6; y++)
					if (_wm->_pixelformat.bytesPerPixel == 1)
						*((byte *)surface.getBasePtr(x, y)) = _vm->transformColor(i * 16 + j);
					else
						*((uint32 *)surface.getBasePtr(x, y)) = _vm->transformColor(i * 16 + j);
		}
	}

	x = 10;
	for (int i = 0; i < kNumBuiltinTiles; i++) {
		Picture *tile = g_director->getTile(i);
		surface.blitFrom(tile->_surface, Common::Point(x, 250));

		x += tile->_surface.w + 10;
	}

	Common::String filename("blend2.pic");
	Common::Path path = findPath(filename);
	Common::File in;
	in.open(path);

	if (in.isOpen()) {
		Image::PICTDecoder k;
		k.loadStream(in);

		Graphics::Surface *res = k.getSurface()->convertTo(_wm->_pixelformat, k.getPalette().data(), k.getPalette().size(), _wm->getPalette(), _wm->getPaletteSize(), Graphics::kDitherNaive);
		surface.blitFrom(*res, Common::Point(400, 280));
		delete res;

		in.close();
	} else {
		warning("b_importFileInto(): Cannot open file %s", path.toString().c_str());
	}

	g_system->copyRectToScreen(surface.getPixels(), surface.pitch, 0, 0, w, h); // testing fonts

	Common::Event event;

	while (true) {
		if (g_director->pollEvent(event))
			if (event.type == Common::EVENT_QUIT)
				break;

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void Window::testFonts() {
	Common::Path fontName("Helvetica");

	Common::MacResManager *fontFile = new Common::MacResManager();
	if (!fontFile->open(fontName))
		error("testFonts(): Could not open %s as a resource fork", fontName.toString(Common::Path::kNativeSeparator).c_str());

	Common::MacResIDArray fonds = fontFile->getResIDArray(MKTAG('F','O','N','D'));
	if (fonds.size() > 0) {
		for (auto &iterator : fonds) {
			Common::SeekableReadStream *stream = fontFile->getResource(MKTAG('F', 'O', 'N', 'D'), iterator);
			Common::String name = fontFile->getResName(MKTAG('F', 'O', 'N', 'D'), iterator);

			debug("Font: %s", name.c_str());

			Graphics::MacFontFamily font(name);
			font.load(*stream);
		}
	}

	delete fontFile;
}

//////////////////////
// Movie iteration
//////////////////////
Common::HashMap<Common::String, Movie *> *Window::scanMovies(const Common::Path &folder) {
	Common::FSNode directory(folder);
	Common::FSList movies;
	const char *sharedMMMname;

	if (_vm->getPlatform() == Common::kPlatformWindows)
		sharedMMMname = "SHARDCST.MMM";
	else
		sharedMMMname = "Shared Cast";


	Common::HashMap<Common::String, Movie *> *nameMap = new Common::HashMap<Common::String, Movie *>();
	if (!directory.getChildren(movies, Common::FSNode::kListFilesOnly))
		return nameMap;

	if (!movies.empty()) {
		for (Common::FSList::const_iterator i = movies.begin(); i != movies.end(); ++i) {
			debugC(2, kDebugLoading, "File: %s", i->getName().c_str());

			if (Common::matchString(i->getName().c_str(), sharedMMMname, true)) {
				debugC(2, kDebugLoading, "Shared cast detected: %s", i->getName().c_str());
				continue;
			}

			warning("name: %s", i->getName().c_str());
			Archive *arc = _vm->openArchive(i->getPathInArchive());
			Movie *m = new Movie(this);
			m->setArchive(arc);
			nameMap->setVal(m->getMacName(), m);

			debugC(2, kDebugLoading, "Movie name: \"%s\"", m->getMacName().c_str());
		}
	}

	return nameMap;
}

void Window::enqueueAllMovies() {
	Common::FSNode dir(ConfMan.getPath("path"));
	Common::FSList files;
	if (!dir.getChildren(files, Common::FSNode::kListFilesOnly)) {
		warning("DirectorEngine::enqueueAllMovies(): Failed inquiring file list");
		return;
	}

	for (Common::FSList::const_iterator file = files.begin(); file != files.end(); ++file)
		_movieQueue.push_back((*file).getName());

	Common::sort(_movieQueue.begin(), _movieQueue.end());

	debug(1, "=========> Enqueued %d movies", _movieQueue.size());
}

MovieReference Window::getNextMovieFromQueue() {
	MovieReference res;

	if (_movieQueue.empty())
		return res;

	res.movie = _movieQueue.front();

	debug(0, "=======================================");
	debug(0, "=========> Next movie is %s", res.movie.c_str());
	debug(0, "=======================================");

	_movieQueue.remove_at(0);

	return res;
}

const byte testMovie[] = {
	0x1F, 0x8B, 0x08, 0x08, 0x00, 0xD1, 0x10, 0x5F, 0x00, 0x03, 0x74, 0x65,
	0x73, 0x74, 0x6D, 0x6F, 0x76, 0x69, 0x65, 0x66, 0x69, 0x78, 0x65, 0x64,
	0x33, 0x00, 0xB5, 0x54, 0xCD, 0x6B, 0x53, 0x41, 0x10, 0x9F, 0xF7, 0x91,
	0xE6, 0x35, 0x89, 0xE6, 0xD5, 0xC4, 0xD8, 0xDA, 0x22, 0x2F, 0x50, 0x24,
	0x14, 0x35, 0x60, 0x2E, 0x2A, 0x45, 0x2C, 0xC1, 0x90, 0x22, 0xD1, 0x98,
	0xB4, 0x35, 0x2A, 0x48, 0x0B, 0x49, 0xB4, 0x35, 0x5F, 0x84, 0xA0, 0xF5,
	0x16, 0xAF, 0x9E, 0x84, 0x5E, 0x5A, 0xF1, 0x20, 0x78, 0x2E, 0x2D, 0xDE,
	0xBC, 0xF5, 0xE8, 0xA1, 0xFE, 0x19, 0x06, 0xF1, 0xA0, 0xA2, 0x17, 0x11,
	0x49, 0xFC, 0xED, 0xBE, 0xDD, 0xF7, 0xD2, 0xF2, 0xE8, 0x45, 0x5C, 0xDE,
	0xEC, 0xCC, 0xFE, 0x76, 0x76, 0x76, 0xDE, 0xEC, 0xCC, 0x14, 0xE6, 0x33,
	0x25, 0xA2, 0x91, 0xA9, 0xDC, 0xD2, 0xE5, 0xD4, 0x6A, 0x7D, 0xA5, 0x45,
	0x44, 0xE3, 0x20, 0x05, 0x74, 0x8E, 0x0E, 0x8D, 0x3A, 0xDF, 0xD7, 0x2E,
	0x41, 0x23, 0x8A, 0x65, 0x02, 0x34, 0x35, 0x10, 0x03, 0xB2, 0xBF, 0x20,
	0x6C, 0x71, 0x65, 0xC5, 0x3E, 0x33, 0x64, 0x33, 0xC4, 0xB0, 0xF0, 0xCE,
	0x76, 0xDE, 0xB1, 0x23, 0xEE, 0x00, 0xB6, 0x7B, 0xE3, 0xFA, 0xDD, 0x19,
	0x88, 0x7F, 0x80, 0xD7, 0xE4, 0x7D, 0x4B, 0x77, 0xD2, 0x19, 0xB0, 0x3C,
	0x91, 0xDE, 0x92, 0x58, 0x7A, 0xAE, 0xC8, 0xF4, 0x0C, 0x60, 0x9F, 0x25,
	0x56, 0x6D, 0x57, 0x2A, 0x8E, 0x93, 0x21, 0x3E, 0x1B, 0x1E, 0x98, 0xE9,
	0x81, 0x8D, 0x7A, 0x60, 0xC7, 0x8B, 0xCD, 0x76, 0xD9, 0xF6, 0x59, 0xFF,
	0xE0, 0xFA, 0x92, 0x99, 0x07, 0xBB, 0x4D, 0xE4, 0xCB, 0xB8, 0x58, 0x31,
	0x0D, 0x76, 0x15, 0xD8, 0x9B, 0x23, 0x7C, 0x19, 0xF1, 0xC0, 0xC6, 0x60,
	0x2F, 0x67, 0x03, 0x22, 0x5E, 0xDE, 0x67, 0x27, 0x3C, 0xB0, 0x13, 0x87,
	0x31, 0x16, 0x7F, 0xC4, 0xA5, 0x83, 0xD5, 0x34, 0x7C, 0xFE, 0x25, 0xB7,
	0x8A, 0x0B, 0xA5, 0x05, 0xB0, 0x0B, 0xF0, 0xAF, 0x27, 0x31, 0xA1, 0x37,
	0x01, 0x6C, 0xFA, 0x88, 0x7B, 0x8F, 0x79, 0x60, 0x31, 0x0F, 0xEC, 0x94,
	0x07, 0x36, 0xE9, 0x81, 0x8D, 0x7B, 0x60, 0xA7, 0x99, 0xE3, 0xFF, 0x44,
	0xF6, 0x30, 0xC3, 0x3B, 0x71, 0xC3, 0xB1, 0x4D, 0x1F, 0x0D, 0xB6, 0xDE,
	0xFF, 0xCD, 0x64, 0x25, 0xAE, 0xF4, 0x68, 0x4C, 0x1D, 0x84, 0x68, 0xD0,
	0xC7, 0x32, 0x2B, 0x48, 0xEE, 0x70, 0x81, 0x02, 0xCE, 0x7A, 0xF3, 0x79,
	0xEF, 0xC0, 0x1E, 0x1D, 0x38, 0x63, 0x40, 0x77, 0xE8, 0x1E, 0x8C, 0xF9,
	0x8D, 0xD7, 0x98, 0x07, 0xF6, 0x0B, 0xC0, 0x1F, 0x85, 0xCD, 0xFB, 0xFB,
	0xA4, 0x32, 0xBE, 0xB5, 0x45, 0x1A, 0xE3, 0xD5, 0x2A, 0xE9, 0x8C, 0xA7,
	0x52, 0x4E, 0x8E, 0x87, 0x78, 0x04, 0x58, 0x0D, 0xF9, 0x05, 0x8F, 0x88,
	0xB7, 0xF2, 0xE1, 0xFD, 0x64, 0x8E, 0x07, 0x98, 0x2C, 0x72, 0x51, 0x67,
	0x9F, 0xA8, 0x87, 0xA0, 0x2D, 0xF3, 0x7C, 0x0C, 0x0B, 0x39, 0x67, 0x47,
	0x55, 0x97, 0x39, 0x39, 0x2C, 0x9B, 0xB6, 0xDC, 0x79, 0xFC, 0x3F, 0xE4,
	0x56, 0x30, 0x49, 0xD9, 0x75, 0x65, 0x34, 0xDB, 0xE8, 0xF7, 0x6F, 0x6E,
	0x6B, 0xAF, 0xE6, 0xBE, 0xF6, 0xFB, 0x85, 0x6C, 0xD2, 0x80, 0x7C, 0xEF,
	0xD6, 0x37, 0x32, 0x97, 0x89, 0x36, 0xAD, 0x06, 0x19, 0x56, 0xFE, 0xEC,
	0x23, 0x32, 0x92, 0x41, 0xA7, 0xA6, 0xF3, 0xFA, 0x7D, 0x7A, 0x49, 0x7B,
	0x6A, 0x4B, 0xBB, 0x86, 0xB8, 0xAA, 0x70, 0x58, 0xE3, 0x3D, 0x63, 0x17,
	0x64, 0xB1, 0xE0, 0xEA, 0x09, 0x68, 0x58, 0x22, 0xD2, 0x7B, 0x7C, 0xFE,
	0x89, 0xFF, 0x55, 0x1E, 0xBE, 0x83, 0x34, 0x1B, 0xFB, 0xF1, 0xBE, 0x4A,
	0xAA, 0xFB, 0x14, 0x6E, 0xCD, 0x3A, 0x43, 0xE5, 0x14, 0x05, 0xC1, 0xBE,
	0xDB, 0x37, 0x28, 0x02, 0x3A, 0xE9, 0xD4, 0xCB, 0x19, 0xA6, 0xAA, 0x89,
	0x23, 0x4E, 0x2F, 0x23, 0xE0, 0x26, 0x3D, 0x61, 0xFF, 0x49, 0xA6, 0x53,
	0x33, 0x11, 0x3B, 0x0D, 0xA0, 0xBE, 0x4C, 0xDF, 0xE9, 0x8B, 0xB2, 0x41,
	0xCA, 0xA7, 0xAE, 0xA2, 0xFA, 0x86, 0xBA, 0x03, 0x7F, 0x4D, 0x39, 0xBA,
	0x7C, 0xF6, 0x0F, 0x21, 0xF2, 0x7E, 0xD4, 0x1F, 0x4D, 0x4A, 0xF2, 0xAF,
	0x75, 0xAC, 0xF3, 0xD6, 0x5A, 0x27, 0x50, 0x7E, 0x56, 0x5B, 0x69, 0x40,
	0xAC, 0x57, 0x7C, 0x8B, 0x8D, 0xD5, 0xF5, 0x2B, 0x70, 0xDC, 0xED, 0x31,
	0x9C, 0x98, 0x75, 0x05, 0xA1, 0x8F, 0xD2, 0x45, 0x04, 0x6B, 0x06, 0xFE,
	0x25, 0xC8, 0xDF, 0xED, 0xE2, 0x26, 0x85, 0x1E, 0x20, 0xAA, 0x01, 0x2A,
	0x01, 0x9B, 0x25, 0xB3, 0x42, 0x6A, 0x17, 0x7F, 0xCE, 0xFC, 0x7C, 0x41,
	0x6F, 0xDD, 0x4E, 0xC0, 0x33, 0x8F, 0x51, 0x2C, 0x5B, 0xA9, 0xD5, 0x9A,
	0xD6, 0xD3, 0x66, 0xBB, 0x56, 0x8E, 0x8B, 0x6E, 0x6D, 0x62, 0x47, 0x23,
	0x59, 0x9F, 0x32, 0xAF, 0xE8, 0x2F, 0xED, 0x2A, 0x34, 0x9B, 0x24, 0x06,
	0x00, 0x00
};

void Window::runTests() {
	Common::MemoryReadStream *movie = new Common::MemoryReadStream(testMovie, ARRAYSIZE(testMovie));
	Common::SeekableReadStream *stream = Common::wrapCompressedReadStream(movie);

	const char *cueTest =
"PERFORMER \"Bloc Party\"\n"
"TITLE \"Silent Alarm\"\n"
"FILE \"Bloc Party - Silent Alarm.flac\" WAVE\n"
   "TRACK 01 AUDIO\n"
      "TITLE \"Like Eating Glass\"\n"
      "PERFORMER \"Bloc Party\"\n"
      "INDEX 00 00:00:00\n"
      "INDEX 01 03:22:70\n"
   "TRACK 02 AUDIO\n"
      "TITLE \"Helicopter\"\n"
      "PERFORMER \"Bloc Party\"\n"
      "INDEX 00 07:42:69\n"
      "INDEX 01 07:44:69\n"
"";


	Common::CueSheet cue(cueTest);

	initGraphics(640, 480);

	Archive *mainArchive = new RIFXArchive();
	g_director->setMainArchive(mainArchive);
	g_director->_allSeenResFiles.setVal("test.dir", mainArchive);
	if (!mainArchive->openStream(stream, 0)) {
		error("DirectorEngine::runTests(): Bad movie data");
	}
	_currentMovie = new Movie(this);
	_currentMovie->setArchive(mainArchive);
	_currentMovie->loadArchive();

	if (debugChannelSet(-1, kDebugText)) {
		testFontScaling();
		testFonts();
	}

	g_lingo->runTests();
}

} // End of namespace Director
