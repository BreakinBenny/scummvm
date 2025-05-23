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

#ifndef AWE_DETECTION_H
#define AWE_DETECTION_H

#include "engines/advancedDetector.h"

namespace Awe {

enum AweDebugChannels {
	kDebugScript = 1,
	kDebugBank,
	kDebugVideo,
	kDebugSound,
	kDebugInfo,
	kDebugPak,
	kDebugResource
};

enum DataType {
	DT_DOS,
	DT_AMIGA,
	DT_ATARI,
	DT_15TH_EDITION,
	DT_20TH_EDITION,
	DT_WIN31,
	DT_3DO,
	DT_ATARI_DEMO // ST Action Issue44 Disk28
};

struct AweGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;

	int _gameType;
};

extern const PlainGameDescriptor aweGames[];

extern const AweGameDescription gameDescriptions[];

#define GAMEOPTION_COPY_PROTECTION		GUIO_GAMEOPTIONS1

} // namespace Awe

class AweMetaEngineDetection : public AdvancedMetaEngineDetection<Awe::AweGameDescription> {
	static const DebugChannelDef debugFlagList[];

public:
	AweMetaEngineDetection();
	~AweMetaEngineDetection() override {}

	const char *getName() const override {
		return "awe";
	}

	const char *getEngineName() const override {
		return "Awe";
	}

	const char *getOriginalCopyright() const override {
		return "Out of This World (C) 1991 by Delphine Software International";
	}

	const DebugChannelDef *getDebugChannels() const override {
		return debugFlagList;
	}
};

#endif
