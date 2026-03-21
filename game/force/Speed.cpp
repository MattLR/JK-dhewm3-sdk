#include "../Force.h"

#include "sys/platform.h"
#include "framework/DeclEntityDef.h"
#include "framework/DeclSkin.h"

#include "gamesys/SysCvar.h"
#include "ai/AI.h"
#include "Player.h"
#include "Trigger.h"
#include "SmokeParticles.h"
#include "WorldSpawn.h"
#include "Mover.h"

CLASS_DECLARATION( jkSimpleForcePower, jkForceSpeed )
END_CLASS


/*
================
jkForceSpeed::Event_DoForcePower
10 seconds, 1: 1.33, 2: 200. 3: 500
================
*/
void jkForceSpeed::Event_DoForcePower( void ) {
	gameLocal.DPrintf ("Event_DoForcePower Speed\n");

	switch (owner->forceLevels[0]) {
		case 0:
			gameLocal.DPrintf ("DoForcePower Speed level 0\n");
				if ( owner->UseForcePoints(50) ) {
					struct statusEffect s = {0, 1.33, gameLocal.time + 10000};
					owner->statusEffects.Append(s);
					owner->hud->SetStateString( "activeForceBG", Icon() );
				}
			break;
		case 1:
			gameLocal.DPrintf ("DoForcePower Speed level 1\n");
				if ( owner->UseForcePoints(50) ) {
					struct statusEffect s = {0, 1.33, gameLocal.time + 10000};
					owner->statusEffects.Append(s);
					owner->hud->SetStateString( "activeForceBG", Icon() );
				}
			break;
		case 3:
			gameLocal.DPrintf ("DoForcePower Speed level 3\n");
				if ( owner->UseForcePoints(50) ) {
					struct statusEffect s = {0, 1.33, gameLocal.time + 10000};
					owner->statusEffects.Append(s);
					owner->hud->SetStateString( "activeForceBG", Icon() );
				}
			break;
		default:
			gameLocal.DPrintf ("DoForcePower Speed default\n");

	}
}