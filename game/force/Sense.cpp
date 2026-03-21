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

CLASS_DECLARATION( jkSimpleForcePower, jkForceSense )
END_CLASS


/*
================
jkForceSense::Event_DoForcePower
================
*/
void jkForceSense::Event_DoForcePower( void ) {
	gameLocal.DPrintf ("Event_DoForcePower\n");

	int i, listedEntities;
	idEntity *entityList[ MAX_GENTITIES ];
	int pushRadius = 400;

	int forceLevel = 1;

	listedEntities = gameLocal.EntitiesWithinRadius( GetPhysics()->GetOrigin(), pushRadius, entityList, MAX_GENTITIES );

	int pushStrength = 1000;
	gameLocal.Printf("Push\n");
	idVec3 origin = GetPhysics()->GetOrigin();
	idVec3 offset( 0.0f, 40.0f, 0.0f );
	// Collect entities within radius. Doom 3 doesn't have a direct RadiusList helper in public SDK,
	// so we use clip.Contents or iterate entities and test distance (simple but works for small maps).

	for( i = 0; i < listedEntities; i++ ) {
			idEntity *ent = entityList[ i ];
			if ( !ent || ent == owner ) continue;
			if ( ent->IsHidden() ) continue;
			if ( ent->IsType( idWorldspawn::Type) ) continue;
			if ( ent->IsType( idLight::Type) ) continue;
			ent->GetName();
			gameLocal.DPrintf(ent->GetName());
			// basic distance check
			idVec3 eorg = ent->GetPhysics()->GetOrigin();
			float dist = ( eorg - origin ).Length();
			if ( dist > pushRadius ) continue;

			// Direction and strength falloff (optional)
			idVec3 dir = eorg - (origin + offset);
			if ( dir.Length() == 0 ) {
				dir = this->GetPhysics()->GetAxis()[0];
			} else {
				dir.Normalize();
			}

			float falloff = 1.0f - ( dist / pushRadius );
			float impulse = pushStrength * falloff;


			idPhysics *phys = ent->GetPhysics();
			if ( !phys ) {
				continue;
				/*
				idVec3 vel = phys->GetLinearVelocity();
				vel += dir * impulse;
				phys->SetLinearVelocity( vel );
				*/
			}
			ent->ForcePowerResponse(this, this, dir, "push", 1, INVALID_JOINT);
			//idMover_Binary *binary = dynamic_cast<idMover_Binary*>( ent );
			if ( ent->IsType( idMover_Binary::Type ) ) {
				//continue;
				ent->Signal( SIG_TRIGGER );
				ent->ProcessEvent( &EV_Activate, owner);
				continue;
			}
			// Handle NPCs/enemies (idAI or subclasses)
			idAI *ai = dynamic_cast<idAI*>( ent );
			if ( ent->IsType( idAI::Type ) ) {
		}
		
		idAFEntity_Base *rag = dynamic_cast<idAFEntity_Base*>( ent );
		if ( rag && rag->IsActiveAF() ) {
			// Add impulses to all articulated bodies
			for ( int j = 0; j < rag->GetAFPhysics()->GetNumBodies(); j++ ) {
				rag->GetAFPhysics()->GetBody( j )->AddForce( dir, dir * impulse * 0.5f );
			}
			continue;
		}
			

			// Damage small amount to ragdollable entities or apply other effects
		if ( ent->fl.takedamage ) {
			// Create damage structure
			int dmg = (int)( 10.0f * falloff );
			if ( dmg > 0 ) {
					idEntity *inflictor = this;
					ent->Damage( this, inflictor, dir, "push_power", dmg, 0 );
				}
			}
		}
}