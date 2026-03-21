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

CLASS_DECLARATION( jkSimpleForcePower, jkForceHeal )
END_CLASS


/*
================
jkForceHeal::Event_DoForcePower
================
*/
void jkForceHeal::Event_DoForcePower( void ) {
	gameLocal.DPrintf ("Event_DoForcePower Force Heal\n");

	owner->GiveHealthPool(25);
}