/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2012 BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <jens.schneider@diskohq.org>,                   *
 *      Guido Madaus       <guido.madaus@diskohq.org>,                     *
 *      Patrick Helterhoff <patrick.helterhoff@diskohq.org>,               *
 *      René Bählkow       <rene.baehlkow@diskohq.org>                     *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License version 2.1 as published by the Free Software Foundation.     *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 **************************************************************************/

#ifndef MMS3DSCENE_H_
#define MMS3DSCENE_H_

#include "mmsgui/3d/mms3dpolygonmesh.h"
#include "mmsgui/3d/mms3dobject.h"

class MMS3DScene {
private:

	typedef enum {
		OBJ_NOTSET = -1,
		OBJ_SIZE = 256
	} OBJ;

	//! objects
	MMS3D_OBJECT objbuf[OBJ_SIZE];
	MMS3D_OBJECT *objects[OBJ_SIZE + 1];
	int objects_cnt;

	MMS3DPolygonMesh	mms3dpm;

	//! stores base matrix and matrix operations
	MMS3DMatrixStack	matrixStack;

	//! children objects
	vector<MMS3DObject*> children;

private:
	int newObject(MMS3DObject *object);

	MMS3D_OBJECT *getObject(int object);

	bool getResultMatrix(MMSMatrix result);

public:

	MMS3DScene();

	bool setPrimitives(string id, MMS_VERTEX_ARRAY *vertices, MMS_VERTEX_ARRAY *normals,
					   MMS_VERTEX_ARRAY *texcoords, MMS_INDEX_ARRAY *indices);

	void getMeshArrays(MMS_VERTEX_ARRAY ***varrays, MMS_INDEX_ARRAY ***iarrays);

	void getObjects(MMS3D_OBJECT ***objects);

	void setBaseMatrix(MMSMatrix matrix);

	void reset();

	bool scale(float sx, float sy, float sz);

	bool translate(float tx, float ty, float tz);

	bool rotate(float angle, float x, float y, float z);

	bool genMatrices();

	friend class MMS3DObject;
	friend class MMS3DRectangle;
	friend class MMS3DSphere;
	friend class MMS3DTorus;
	friend class MMS3DCylinder;
	friend class MMS3DPrimitives;
};

#endif /* MMS3DSCENE_H_ */

