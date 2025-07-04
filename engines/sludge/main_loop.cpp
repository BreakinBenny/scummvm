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

#include "sludge/event.h"
#include "sludge/function.h"
#include "sludge/graphics.h"
#include "sludge/people.h"
#include "sludge/saveload.h"
#include "sludge/sound.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/timing.h"

namespace Sludge {

extern VariableStack *noStack;

int main_loop(Common::String filename) {

	if (!initSludge(filename)) {
		return 0;
	}

	g_sludge->_gfxMan->initGfx();

	startNewFunctionNum(0, 0, NULL, noStack);

	g_sludge->_evtMan->startGame();
	g_sludge->_timer->init();

	int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	if (saveSlot != -1)
		g_sludge->loadNow = g_sludge->getSaveStateName(saveSlot);

	while (!g_sludge->_evtMan->quit()) {
		g_sludge->_evtMan->checkInput();
		g_sludge->_peopleMan->walkAllPeople();
		if (g_sludge->_evtMan->handleInput()) {
			runAllFunctions();
			handleSaveLoad();
		}
		sludgeDisplay();
		g_sludge->_soundMan->handleSoundLists();
		g_sludge->_timer->waitFrame();
	}

	killSludge();

	// Load next game
	if (!g_sludge->launchNext.empty()) {
		Common::String name = g_sludge->launchNext;
		g_sludge->launchNext.clear();
		main_loop(name);
	}

	return (0);
}

}
 // End of namespace Sludge
