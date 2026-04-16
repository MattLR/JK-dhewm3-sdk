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

/*
==============================================================================================

	dnDeclVertexAnim

==============================================================================================
*/
class dnDeclVertexAnim : public idDecl {
public:
	idDict animData; // Store animation data here

	virtual const char* DefaultDefinition() const override;
	virtual bool Parse(const char* text, const int textLength) override;
	virtual void FreeData() override;

	const idDict* FindAnim(const char* name) const;

	// Add methods to access and manipulate the animation data
	const idStr& GetModel() const;
	const idList<idDict>& GetAnims() const;

private:
	idStr model; // MD3 model path
	idList<idDict> anims; // List of animations
};

/*
==============================================================================================

	dnVertexAnimator

==============================================================================================
*/
class dnVertexAnimator {
public:
	idRenderModel*					SetModel(const char* modelDecl);
	void							PlayAnim(const char* animName, bool loop);
	int								GetCurrentFrame() const { return currentFrame; }
	int								GetLastFrame() const { return lastFrame; }
	float							GetBacklerp() const { return backlerp; }
	void							Update(void);
	bool							IsAnimDone(void);
	void							ClearAllAnims(void);
	//Stuff to fix later Dynamix 
	//jointHandle_t					GetJointHandle(const char* name);
	//bool							GetJointTransform(jointHandle_t jointHandle, idVec3& offset, idMat3& axis);
private:
	const idDict*					currentAnim = nullptr;
	bool							isLooping = false;

	int								lastTime = 0;
	int								start_frame = 0;
	int								end_frame = 0;

	float							currentFrame= 0;
	int								lastFrame = 0;
	float							backlerp = 0;

	idStr							currentAnimName;

	const dnDeclVertexAnim*			vertexAnim;
	idRenderModel*					renderModel = nullptr;
};