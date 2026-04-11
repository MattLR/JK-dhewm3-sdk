/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code ("Doom 3 Source Code").

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __GAME_VEHICLE2_H__
#define __GAME_VEHICLE2_H__

#include "physics/Physics_RigidBody.h"
#include "physics/Force_Constant.h"
#include "script/Script_Thread.h"
#include "gamesys/Event.h"
#include "Entity.h"
#include "Player.h"
#include "Projectile.h"

class jkRBVehicleTest : public idAnimatedEntity {
public:
	CLASS_PROTOTYPE( jkRBVehicleTest );

	
							jkRBVehicleTest( void );
	virtual					~jkRBVehicleTest( void );

	void					Spawn( void );
    void					Use( idPlayer *player );
    virtual void			Think( void );

	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	//virtual void			GetImpactInfo( idEntity *ent, int id, const idVec3 &point, impactInfo_t *info );
	//virtual void			ApplyImpulse( idEntity *ent, int id, const idVec3 &point, const idVec3 &impulse );
	//virtual void			AddForce( idEntity *ent, int id, const idVec3 &point, const idVec3 &force );
	virtual bool			Collide( const trace_t &collision, const idVec3 &velocity );
	//virtual bool			GetPhysicsToVisualTransform( idVec3 &origin, idMat3 &axis );
	virtual void			FreeModelDef( void );
    virtual void			Killed( idEntity *inflictor, idEntity *attacker, int damage, const idVec3 &dir, int location );
	//virtual void			WriteToSnapshot( idBitMsgDelta &msg ) const;
	//virtual void			ReadFromSnapshot( const idBitMsgDelta &msg );

    void					SetClipModel( void );
	void					SetCombatModel( void );
	idClipModel *			GetCombatModel( void ) const;
							// contents of combatModel can be set to 0 or re-enabled (mp)
	void					SetCombatContents( bool enable );
	virtual void			LinkCombat( void );
	virtual void			UnlinkCombat( void );

	//int						BodyForClipModelId( int id ) const;

protected:

	idPhysics_RigidBody		physicsObj;				// physics object
	idClipModel *			combatModel;	// render model for hit detection
	int						combatModelContents;
	idVec3					spawnOrigin;	// spawn origin
	idMat3					spawnAxis;		// rotation axis used when spawned
	int						nextSoundTime;	// next time this can make a sound
    idPlayer *				player;
	jointHandle_t			eyesJoint;
	//jointHandle_t			steeringWheelJoint;
	//float					wheelRadius;
	float					steerAngle;
	float					steerSpeed;
	//const idDeclParticle *	dustSmoke;
    float					GetSteerAngle( void );
};


#endif /* !__GAME_VEHICLE2_H__ */