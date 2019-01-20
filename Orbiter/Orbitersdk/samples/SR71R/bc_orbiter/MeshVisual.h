//	MeshVisual - bco Orbiter Library
//	Copyright(C) 2015  Blake Christensen
//
//	This program is free software : you can redistribute it and / or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation, either version 3 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program.If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "Visual.h"
#include "Tools.h"

namespace bc_orbiter
{
	/**	MeshVisual
	Base class for a visual element that changes by altering the mesh coordinates.
	*/
	class MeshVisual : public Visual
	{
	public:
		MeshVisual(const NTVERTEX* verts, UINT group) : Visual(verts, group)
		{ }

		/**	Draw
		Called to update the visual.  Implementing class needs to define how the visual
		will be manipulated.
		*/
		virtual void Draw(DEVMESHHANDLE devMesh)
		{
			if (NULL == devMesh)
			{
				return;
			}

			auto translate = GetTranslate();
			auto angle = GetAngle();

			GROUPEDITSPEC change;
			NTVERTEX delta[4];

			TransformXY2d(GetVerts(), delta, 4, translate, angle);

			change.flags = GRPEDIT_VTXCRD;
			change.nVtx = 4;
			change.vIdx = NULL; //Just use the mesh order
			change.Vtx = delta;
			oapiEditMeshGroup(devMesh, GetGroup(), &change);
		}
	};
}