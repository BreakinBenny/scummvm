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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTES_SXFILE_H
#define WINTERMUTES_SXFILE_H


#include "engines/wintermute/base/base_scriptable.h"
#include "common/stream.h"

namespace Wintermute {

class SXFile : public BaseScriptable {
public:
	DECLARE_PERSISTENT(SXFile, BaseScriptable)
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
	SXFile(BaseGame *inGame, ScStack *Stack);
	~SXFile() override;
private:
	Common::SeekableReadStream *_readFile;
	Common::WriteStream *_writeFile;
	int32 _mode; // 0..none, 1..read, 2..write, 3..append
	bool _textMode;
	void close();
	void cleanup();
	uint32 getPos();
	uint32 getLength();
	bool setPos(uint32 pos, int whence = SEEK_SET);
	char *_filename;
	Common::WriteStream *openForWrite(const Common::String &filename, bool binary);
	Common::WriteStream *openForAppend(const Common::String &filename, bool binary);
};

} // End of namespace Wintermute

#endif
