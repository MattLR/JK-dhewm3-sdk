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

#pragma once

#include "idlib/Dict.h"
#include "renderer/Model.h"
#include "anim/Anim.h"

class idEntity;
class idSaveGame;
class idRestoreGame;

/*
==============================================================================================

	dnVertexFrameCmd_t / dnVertexAnimDef

	One parsed "anim" entry from a vertexmodeldef decl.
	Frame commands use the stock Doom 3 frameCommand_t, so every stock command
	(sound, event, trigger, fx, melee, footstep ...) works the same way as in a modelDef.

==============================================================================================
*/
struct dnVertexFrameCmd_t {
	int					frame;		// 0 based, relative to the start of the anim (the .def uses 1 based)
	frameCommand_t		cmd;
};

class dnVertexAnimDef {
public:
	idStr							name;
	int								startFrame;
	int								endFrame;
	float							fps;
	idList<dnVertexFrameCmd_t>		frameCommands;		// kept sorted by frame

									dnVertexAnimDef() : startFrame( 0 ), endFrame( 0 ), fps( 24.0f ) {}
									~dnVertexAnimDef();

	int								NumFrames() const { return endFrame - startFrame + 1; }
	bool							HasFrameCommands() const { return frameCommands.Num() > 0; }
};


/*
==============================================================================================

	dnDeclVertexAnim

==============================================================================================
*/
class dnDeclVertexAnim : public idDecl {
public:
								dnDeclVertexAnim();
								~dnDeclVertexAnim();

	// Fill in missing functions  - Dynamix FIXME
	//virtual size_t				Size( void ) const;							
	virtual const char * 		DefaultDefinition() const override;
	virtual bool 				Parse( const char* text, const int textLength ) override;
	virtual void 				FreeData( void ) override;

	//void						Touch( void ) const;

	const dnVertexAnimDef*			FindAnim(const char* name) const;


	// Add methods to access and manipulate the animation data
	const idStr & 				GetModel() const;
	const idList<dnVertexAnimDef*>&	GetAnims() const;

	idDict 						animData; // Store animation data here

private:
	bool							ParseAnim( idLexer& src, dnVertexAnimDef& def );

	idStr 							model; // MD3 model path
	idList<dnVertexAnimDef*>		anims; // owned
};

/*
==============================================================================================

	dnVertexAnimator

==============================================================================================
*/
class dnVertexAnimator {
public:
	// Fill in missing functions - FIXME Dynamix
								dnVertexAnimator();
								~dnVertexAnimator();


	void						Save( idSaveGame *savefile ) const;					// archives object for save game file
	void						Restore( idRestoreGame *savefile );					// unarchives object from save game file


	idRenderModel*				SetModel( const char* modelDecl );
	void						SetEntity( idEntity* ent ) { entity = ent; }	// needed for frame commands to fire

	void						PlayAnim( const char* animName, bool loop );
	int							GetCurrentFrame() const { return lerpFrame; }
	int							GetLastFrame() const { return lastFrame; }
	float						GetBacklerp() const { return backlerp; }
	void						Update ( void );
	bool						IsAnimDone( void );
	void						ClearAllAnims( void ) ;
	
	#ifdef _CENG
	jointHandle_t				GetJointHandle( const char* name );
	bool						GetJointTransform( jointHandle_t jointHandle, idVec3& offset, idMat3& axis );
	const idJointQuat *			GetDefaultPose( void ) const;
	#endif

private:
	bool						FireFrameCommands(float from, float to);

	const dnVertexAnimDef*		currentAnim = nullptr;
	bool						isLooping = false;

	int							lastTime = 0;
	int							start_frame = 0;
	int							end_frame = 0;
	float						fps = 24.0f;

	int							lerpFrame = 0;
	float						currentFrame = 0.0f;
	int							lastFrame = 0;
	float						backlerp = 0.0f;

	float						lastEventFrame = 0.0f;	// frame position events have been fired up to
	int							playSerial = 0;			// bumped by PlayAnim so events can detect a re-entrant anim change

	idStr						currentAnimName;

	const dnDeclVertexAnim*		vertexAnim;
	idRenderModel*				renderModel = nullptr;
	idEntity*					entity = nullptr;
};