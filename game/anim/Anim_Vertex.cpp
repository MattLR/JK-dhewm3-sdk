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

#include "Anim_Vertex.h"
#include "idlib/geometry/JointTransform.h"
#include "Game_local.h"
#include "renderer/ModelManager.h"

/*
========================================

dnVertexAnimDef

========================================
*/

/*
=====================
dnVertexAnimDef::~dnVertexAnimDef
=====================
*/
dnVertexAnimDef::~dnVertexAnimDef() {
	for ( int i = 0; i < frameCommands.Num(); i++ ) {
		idAnim::FreeFrameCommand(frameCommands[i].cmd);
	}
	frameCommands.Clear();
}

/*
=====================
SkeletalCommandName

Frame commands that look up joints on the entity's skeleton, which vertex models don't have.
Returns the name if the command can't work on a vertex anim, otherwise NULL.
=====================
*/
static const char* SkeletalCommandName( const frameCommand_t& cmd ) {
	switch (cmd.type) {
		case FC_CREATEMISSILE:			return "create_missile";
		case FC_LAUNCHMISSILE:			return "launch_missile";
		case FC_FIREMISSILEATTARGET:	return "fire_missile_at_target";
		case FC_MUZZLEFLASH:			return (cmd.string && cmd.string->Length()) ? "muzzle_flash <joint>" : NULL;
		default:						return NULL;
	}
}

/*
========================================

dnDeclVertexAnim

Decl layout (frame commands are optional, and use the stock modelDef syntax):

vertexmodeldef zombie {
	model "models/md3/zombie.md3"

	anim idle 0 40 15
	anim walk 41 80 24 {
		frame 5  footstep
		frame 15 sound_body snd_step
	}
}

Frame numbers in the block are 1 based and relative to the start of the anim,
exactly like a modelDef (frame 1 == start_frame).
========================================
*/

/*
=====================
dnDeclVertexAnim::DefaultDefinition
=====================
*/
dnDeclVertexAnim::dnDeclVertexAnim() {
}

/*
=====================
dnDeclVertexAnim::DefaultDefinition
=====================
*/
dnDeclVertexAnim::~dnDeclVertexAnim() {
	FreeData();
}

/*
=====================
dnDeclVertexAnim::DefaultDefinition
=====================
*/
const char* dnDeclVertexAnim::DefaultDefinition() const {
	// FIXME
	return "{ }";
	//return "{\n\t\"model\" : \"path/to/model.md3\"\n\t\"anim <name> <start_frame> <end_frame> <fps>\"\n}\n";
}

/*
=====================
dnDeclVertexAnim::ParseAnim

Same checks as idDeclModelDef::ParseAnim. Warns and returns false on error.
=====================
*/
bool dnDeclVertexAnim::ParseAnim( idLexer& src, dnVertexAnimDef& def ) {
	idToken token;

	if ( !src.ReadToken(&token) ) {
		src.Warning("Unexpected end of file");
		return false;
	}
	def.name = token;

	if ( FindAnim(def.name.c_str()) ) {
		src.Warning("Duplicate anim '%s'", def.name.c_str());
		return false;
	}

	def.startFrame = src.ParseInt();
	def.endFrame = src.ParseInt();
	def.fps = src.ParseFloat();	// 0 is fine, it holds start_frame

	if ( def.startFrame < 0 || def.endFrame < def.startFrame ) {
		src.Warning("Anim '%s': invalid frame range %d to %d", def.name.c_str(), def.startFrame, def.endFrame);
		return false;
	}
	if ( def.fps < 0.0f ) {
		src.Warning("Anim '%s': invalid fps %f", def.name.c_str(), def.fps);
		return false;
	}

	// parse any frame commands
	if ( src.CheckTokenString("{") ) {
		while (1) {
			if (!src.ReadToken(&token)) {
				src.Warning("Unexpected end of file");
				return false;
			}
			if ( token == "}" ) {
				break;
			}
			else if ( token == "frame" ) {
				// make sure we don't have any line breaks while reading the frame command so the error line # will be correct
				if (!src.ReadTokenOnLine(&token)) {
					src.Warning("Missing frame # after 'frame'");
					return false;
				}
				if ( token.type == TT_PUNCTUATION && token == "-" ) {
					src.Warning("Invalid frame # after 'frame'");
					return false;
				} else if ( token.type != TT_NUMBER || (token.subtype & TT_FLOAT) ) {
					src.Warning("expected integer value, found '%s'", token.c_str());
					return false;
				}

				const int framenum = token.GetIntValue();
				if ( framenum < 1 || framenum > def.NumFrames() ) {
					src.Warning("Frame %d out of range", framenum);
					return false;
				}

				dnVertexFrameCmd_t entry;
				entry.frame = framenum - 1;	// frame numbers are 1 based in the decl, 0 based internally

				// no modelDef: joint names can't be checked
				const char* err = idAnim::ParseFrameCommand(NULL, src, entry.cmd);
				if ( err ) {
					src.Warning("%s", err);
					idAnim::FreeFrameCommand(entry.cmd);
					return false;
				}

				const char* skeletal = SkeletalCommandName(entry.cmd);
				if ( skeletal ) {
					src.Warning("'%s' needs skeletal joints and is not supported on vertex animations", skeletal);
					idAnim::FreeFrameCommand(entry.cmd);
					return false;
				}

				def.frameCommands.Append(entry);
			} else {
				src.Warning("Unknown command '%s'", token.c_str());
				return false;
			}
		}
	}

	return true;
}

/*
=====================
dnDeclVertexAnim::Parse
=====================
*/
bool dnDeclVertexAnim::Parse( const char* text, const int textLength ) {
	idLexer src;
	idToken token;

	src.LoadMemory(text, textLength, GetFileName(), GetLineNum());
	src.SetFlags(DECL_LEXER_FLAGS);
	src.SkipUntilString("{");

	while (1) {
		if ( !src.ReadToken(&token) ) {
			break;
		}

		if ( token == "}" ) {
			break;
		}

		if ( token == "model" ) {
			if ( !src.ReadToken(&token) ) {
				src.Warning("Unexpected end of file");
				MakeDefault();
				return false;
			}

			// same as 'mesh' in a modelDef
			idRenderModel* m = renderModelManager->FindModel(token);
			if ( !m ) {
				src.Warning("Model '%s' not found", token.c_str());
				MakeDefault();
				return false;
			}
			if ( m->IsDefaultModel() ) {
				src.Warning("Model '%s' defaulted", token.c_str());
				MakeDefault();
				return false;
			}
			model = token;
		} else if ( token == "anim")  {
			if ( model.IsEmpty() ) {
				src.Warning("Must specify model before defining anims");
				MakeDefault();
				return false;
			}

			dnVertexAnimDef* def = new dnVertexAnimDef;
			if ( !ParseAnim(src, *def) ) {
				delete def;
				MakeDefault();
				return false;
			}
			anims.Append(def);
		} else {
			src.Warning("Unknown token: %s", token.c_str());
			MakeDefault();
			return false;
		}
	}
	return true;
}

/*
=====================
dnDeclVertexAnim::FindAnim
=====================
*/
const dnVertexAnimDef* dnDeclVertexAnim::FindAnim( const char* name ) const {
	for (int i = 0; i < anims.Num(); i++) {
		if (anims[i]->name.Icmp(name) == 0) {
			return anims[i];
		}
	}
	return nullptr;
}

/*
=====================
dnDeclVertexAnim::FreeData
=====================
*/
void dnDeclVertexAnim::FreeData() {
	model.Clear();
	anims.Clear();
}

/*
=====================
dnDeclVertexAnim::GetModel
=====================
*/
const idStr& dnDeclVertexAnim::GetModel() const {
	return model;
}

/*
=====================
dnDeclVertexAnim::GetAnims
=====================
*/
const idList<dnVertexAnimDef*>& dnDeclVertexAnim::GetAnims() const {
	return anims;
}

/*
========================================

dnVertexAnimator

========================================
*/

/*
=====================
dnVertexAnimator::dnVertexAnimator
=====================
*/
dnVertexAnimator::dnVertexAnimator() {
	//FreeData();
}

/*
=====================
idAnimator::~idAnimator
=====================
*/
dnVertexAnimator::~dnVertexAnimator() {
	//FreeData();
}

/*
=====================
dnVertexAnimator::Save
=====================
*/
void dnVertexAnimator::Save( idSaveGame *savefile ) const {
	savefile->WriteVertexDef( vertexAnim );
	savefile->WriteString( currentAnimName);
}

/*
=====================
dnVertexAnimator::Restore
=====================
*/
void dnVertexAnimator::Restore( idRestoreGame *savefile ) {
	savefile->ReadVertexDef( vertexAnim );
	savefile->ReadString( currentAnimName);
	renderModel = renderModelManager->FindModel(vertexAnim->GetModel());
	PlayAnim(currentAnimName, 0);
}

/*
=====================
dnVertexAnimator::SetModel
=====================
*/
idRenderModel* dnVertexAnimator::SetModel( const char* modelDecl ) {
	vertexAnim = (const dnDeclVertexAnim *)declManager->FindType(DECL_VERTEXMODELDEF, modelDecl, false);
	if (!vertexAnim) {
		gameLocal.Warning("dnVertexAnimator::SetModel: Failed to load vertex model decl %s\n", modelDecl);
		return nullptr;
	}

	currentAnim = nullptr;
	currentAnimName.Clear();

	start_frame = 0;
	end_frame = 0;
	currentFrame = 0;
	lerpFrame = 0;
	lastFrame = 0;
	backlerp = 0.0f;
	lastEventFrame = 0.0f;

	isLooping = false;
	playSerial++;

	lastTime = gameLocal.time;

	renderModel = renderModelManager->FindModel(vertexAnim->GetModel());
	return renderModel;
}

/*
=====================
dnVertexAnimator::PlayAnim
=====================
*/
void dnVertexAnimator::PlayAnim( const char* animName, bool loop ) {

	//Dynamix Test - tag names
	//for (int i = 0; i < 8; i++) {
	//common->DPrintf("Tag #%d name: %s\n", i, renderModel->GetJointName((jointHandle_t)i));
	//}

	if ( vertexAnim == nullptr ) {
		return;
	}

	const dnVertexAnimDef* def = vertexAnim->FindAnim(animName);
	if ( def == nullptr ) {
		common->Warning("Failed to find vertex animation %s\n", animName);
		return;
	}

	// Looping this anim already
	if ( currentAnim == def && isLooping && loop) {
		return;
	}

	currentAnim = def;
	currentAnimName = animName;
	playSerial++;	// lets a running frame command notice that it changed the anim

	lastTime = gameLocal.time;

	isLooping = loop;
	start_frame = def->startFrame;
	end_frame = def->endFrame;
	fps = def->fps;
	currentFrame = (float)start_frame;

	// events fire for frames in (lastEventFrame, currentFrame], so start one before to fire frame 1
	lastEventFrame = (float)start_frame - 1.0f;
}

/*
=====================
dnVertexAnimator::FireFrameCommands

Runs the frame commands for every frame f with from < f <= to (absolute frame numbers).
Returns false if a command changed the animation (PlayAnim / ClearAllAnims / SetModel),
in which case the caller must stop using its local state.
=====================
*/
bool dnVertexAnimator::FireFrameCommands( float from, float to ) {
	if ( currentAnim == nullptr || !currentAnim->HasFrameCommands() ) {
		return true;
	}
	if ( entity == nullptr ) {
		return true;	// nothing to run commands on
	}

	const int serial = playSerial;
	const dnVertexAnimDef* def = currentAnim;

	for ( int i = 0; i < def->frameCommands.Num(); i++ ) {
		const dnVertexFrameCmd_t& fc = def->frameCommands[i];
		const float f = (float)(def->startFrame + fc.frame);

		if ( f > from && f <= to ) {
			idAnim::ExecuteFrameCommand(NULL, def->name.c_str(), fc.frame, entity, fc.cmd);

			// the command may have started a different anim (or deleted the entity's anim state)
			if ( serial != playSerial ) {
				return false;
			}
		}
	}
	return true;
}

/*
=====================
dnVertexAnimator::Update
=====================
*/

void dnVertexAnimator::Update( void ) {
	if ( vertexAnim == nullptr || currentAnim == nullptr ) {
		gameLocal.Error("Testing Animator broken");
		return;
	}

	// Get the current time and calculate the delta time in seconds
	int currentTime = gameLocal.time;
	int deltaTime = currentTime - lastTime;
	lastTime = currentTime;
	if ( deltaTime < 0 ) {
		deltaTime = 0;
	} else if ( deltaTime > 100 ) {
		// Dynamix changing this
		deltaTime = 100;	// never skip/flood events after a hitch or load
	}
	float deltaTimeSeconds = deltaTime / 1000.0f;

	currentFrame += deltaTimeSeconds * fps;

	bool valid = true;
	const float span = (float)(end_frame - start_frame);

	if ( currentFrame >= end_frame ) {
		if (isLooping && span > 0.0f) {
			// finish out this cycle, then wrap keeping the overshoot
			valid = FireFrameCommands(lastEventFrame, (float)end_frame);
			if (valid) {
				float over = currentFrame - (float)end_frame;
				currentFrame = (float)start_frame + fmodf(over, span);
				lastEventFrame = (float)start_frame - 1.0f;	// so the start frame fires again
			}
		}
		else {
			currentFrame = (float)end_frame;
		}
	}

	if ( valid ) {
		valid = FireFrameCommands(lastEventFrame, currentFrame);
		if ( valid ) {
			lastEventFrame = currentFrame;
		}
	}

	if ( !valid ) {
		// a command started a new anim; PlayAnim already set up the state, just refresh lerp values
		if ( currentAnim == nullptr ) {
			return;
		}
	}

	// lerp values
	if ( currentFrame >= end_frame ) {
		currentFrame = (float)end_frame;
		lerpFrame = end_frame;
		lastFrame = end_frame;
	} else {
		lerpFrame = (int)currentFrame + 1;
		lastFrame = lerpFrame - 1;
	}

	if ( lastFrame <= start_frame ) {
		lastFrame = start_frame;
	}

	backlerp = (float)lerpFrame - currentFrame;
}

/*
=====================
dnVertexAnimator::IsAnimDone
=====================
*/
bool dnVertexAnimator::IsAnimDone( void ) {
	if ( isLooping ) {
		return false;
	}
	return currentFrame >= end_frame;
}

/*
=====================
dnVertexAnimator::ClearAllAnims
=====================
*/
void dnVertexAnimator::ClearAllAnims( void )  {
	currentAnim = nullptr;
	playSerial++;
}

#ifdef _CENG
/*
=====================
dnVertexAnimator::GetJointHandle
=====================
*/

jointHandle_t dnVertexAnimator::GetJointHandle( const char* name ) {
	//return INVALID_JOINT;
	
	if ( renderModel == nullptr ) {
		return INVALID_JOINT;
	}

	return renderModel->GetJointHandle( name );
	
}

/*
=====================
dnVertexAnimator::GetJointTransform
=====================
*/

bool dnVertexAnimator::GetJointTransform( jointHandle_t jointHandle, idVec3& offset, idMat3& axis ) {
	const idJointQuat *pose;

	pose = GetDefaultPose();
	offset.Zero();
	axis.Identity();

	offset = pose[jointHandle].t;
	axis = pose[jointHandle].q.ToMat3();

	return true;
	
	/*
	offset.Zero();
	axis.Identity();

	if (renderModel == nullptr)
		return false;

	if (jointHandle == INVALID_JOINT)
		return false;

	md3Tag_t* tag = renderModel->GetTag(jointHandle, currentFrame);
	if (tag == nullptr)
		return false;

	offset = tag->origin;
	axis = idMat3(tag->axis[0], tag->axis[1], tag->axis[2]);

	return true;
	*/
}

/*
=====================
idDeclModelDef::GetDefaultPose
=====================
*/
const idJointQuat *dnVertexAnimator::GetDefaultPose( void ) const {
	return renderModel->GetDefaultPose();
}
#endif