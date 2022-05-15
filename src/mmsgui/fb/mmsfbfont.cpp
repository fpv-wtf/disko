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

#include "mmsgui/fb/mmsfbfont.h"
#include "mmsgui/fb/mmsfb.h"

#include "mmsgui/ft/mmsfttesselator.h"

#include <ft2build.h>
#include FT_GLYPH_H

#include <math.h>

// static variables
pthread_mutex_t	globalLock = PTHREAD_MUTEX_INITIALIZER;
void *MMSFBFont::ft_library = NULL;
unsigned int MMSFBFont::numReferences = 0;

MMSFBFont::MMSFBFONT_MAP MMSFBFont::index;
unsigned int MMSFBFont::index_pos = 0;


#define INITCHECK  if(!this->isInitialized()){MMSFB_SetError(0,"MMSFBFont is not initialized");return false;}

MMSFBFont::MMSFBFont(string filename, int w, int h) :
	initialized(false),
#ifdef __HAVE_DIRECTFB__
	dfbfont(NULL),
#endif
	ft_face(NULL),
	filename(filename),
	ascender(0),
	descender(0),
	height(0) {

	// get index
	pthread_mutex_lock(&globalLock);
	MMSFBFONT_MAP::iterator it = this->index.find(this->filename);
	if (it == this->index.end()) {
		// new id
		this->index_pos++;
		this->font_id = this->index_pos;
		this->index.insert(make_pair(this->filename, this->font_id));
	}
	else {
		// use existing id
		this->font_id = it->second;
	}
	pthread_mutex_unlock(&globalLock);


    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__

		// create the dfb font
		DFBResult   		dfbres;
		DFBFontDescription 	desc;
		if (w > 0) {
			desc.flags = DFDESC_WIDTH;
			desc.width = w;
		}
		if (h > 0) {
			desc.flags = DFDESC_HEIGHT;
			desc.height = h;
		}
		if ((dfbres=mmsfb->dfb->CreateFont(mmsfb->dfb, this->filename.c_str(), &desc, (IDirectFBFont**)&this->dfbfont)) != DFB_OK) {
			MMSFB_SetError(dfbres, "IDirectFB::CreateFont(" + this->filename + ") failed");
			return;
		}
		this->initialized = true;
#endif
    }
    else {
		// init freetype library
		pthread_mutex_lock(&globalLock);
    	if (!ft_library) {
    		if (FT_Init_FreeType((FT_Library*)&this->ft_library)) {
    			MMSFB_SetError(0, "FT_Init_FreeType() failed");
    			this->ft_library = NULL;
				pthread_mutex_unlock(&globalLock);
    			return;
			}
		}

    	this->numReferences++;
		pthread_mutex_unlock(&globalLock);

    	// load the face
    	if (FT_New_Face((FT_Library)this->ft_library, this->filename.c_str(), 0, (FT_Face*)&this->ft_face)) {
    		this->ft_face = NULL;
			MMSFB_SetError(0, "FT_New_Face(" + this->filename + ") failed");
			return;
    	}

    	// select the charmap
    	if (FT_Select_Charmap((FT_Face)this->ft_face, ft_encoding_unicode)) {
    		FT_Done_Face((FT_Face)this->ft_face);
    		this->ft_face = NULL;
			MMSFB_SetError(0, "FT_Select_Charmap(ft_encoding_unicode) for " + this->filename + " failed");
			return;
    	}

    	// check requested width and height
    	if (w < 0) w = 0;
    	if (h < 0) h = 0;

#if (defined(__HAVE_OPENGL__) && defined(__HAVE_GLU__))
		if (mmsfb->bei) {
			// we create base meshes and scale up/down to glyph's destination size
			int rw = w;
			int rh = h;
			if (rw && rh) {
				float ratio = (float)rw / (float)rh;
				h = 200;
				w = h * ratio;
				if (w == h) w = 0;
				this->scale_coeff = (float)rh / (float)h;
			}
			else
			if (rh) {
				w = 0;
				h = 200;
				this->scale_coeff = (float)rh / (float)h;
			}
			else {
				w = 200;
				h = 0;
				this->scale_coeff = (float)rw / (float)w;
			}
//    		printf("coeff = %f\n", this->scale_coeff);
		}
#endif

        // set the font size
    	if (FT_Set_Char_Size((FT_Face)this->ft_face, w << 6, h << 6, 0, 0)) {
    		FT_Done_Face((FT_Face)this->ft_face);
    		this->ft_face = NULL;
			MMSFB_SetError(0, "FT_Set_Char_Size(" + iToStr(w << 6) + "," + iToStr(h << 6) + ") for " + this->filename + " failed");
			return;
    	}

    	// try to load a first glyph
    	if (FT_Load_Glyph((FT_Face)this->ft_face, FT_Get_Char_Index((FT_Face)this->ft_face, '0'), FT_LOAD_RENDER)) {
    		FT_Done_Face((FT_Face)this->ft_face);
    		this->ft_face = NULL;
			MMSFB_SetError(0, "FT_Load_Glyph('0') for " + this->filename + " failed");
			return;
    	}

    	if (((FT_Face)this->ft_face)->glyph->format != FT_GLYPH_FORMAT_BITMAP) {
    		FT_Done_Face((FT_Face)this->ft_face);
    		this->ft_face = NULL;
			MMSFB_SetError(0, "Glyph format is not FT_GLYPH_FORMAT_BITMAP for " + this->filename);
			return;
    	}

    	this->ascender = ((FT_Face)this->ft_face)->size->metrics.ascender / 64;
    	this->descender = abs(((FT_Face)this->ft_face)->size->metrics.descender / 64);
    	this->height = this->ascender + this->descender + 1;

/*
printf("asc = %d\n", this->ascender);
printf("des = %d\n", this->descender);
printf("height = %d\n", this->height);
*/

    	this->initialized = true;
    }
}

MMSFBFont::~MMSFBFont() {
	lock();

	for (std::map<unsigned int, MMSFBFont_Glyph>::iterator it = this->charmap.begin();
			it != this->charmap.end(); ++it) {

		MMSFBFont_Glyph *glyph = &it->second;

		if (glyph->buffer) {
			free(glyph->buffer);
			glyph->buffer = NULL;
		}

#ifdef  __HAVE_OPENGL__
#ifndef __HAVE_GLU__
		// release texture
		if (mmsfb->bei)
			if (glyph->texture)
				mmsfb->bei->deleteTexture(glyph->texture);
#else
		// release mesh memory
		if (glyph->meshes) {
			delete glyph->meshes;
			glyph->meshes = NULL;
		}

		// release outline memory
		if (glyph->outline) {
			delete glyph->outline;
			glyph->outline = NULL;
		}
#endif
#endif

	}
	this->charmap.clear();

	if(mmsfb->backend != MMSFB_BE_DFB) {
		if(this->ft_face) {
			FT_Done_Face((FT_Face)this->ft_face);
			this->ft_face = NULL;
		}
	}

	unlock();

	pthread_mutex_lock(&globalLock);
	this->numReferences--;

	if(mmsfb->backend != MMSFB_BE_DFB) {
		if(this->ft_library && this->numReferences == 0) {
			FT_Done_FreeType((FT_Library)this->ft_library);
			this->ft_library = NULL;
		}
	}
	pthread_mutex_unlock(&globalLock);
}

bool MMSFBFont::isInitialized() {
    return this->initialized;
}

void MMSFBFont::lock() {
	this->Lock.lock();
}

void MMSFBFont::unlock() {
	this->Lock.unlock();
}
/*
void showGlyphAttributes(FT_GlyphSlot glyph) {
	FT_Glyph_Metrics *metrics = &glyph->metrics;
    FT_Bitmap  *bitmap = &glyph->bitmap;
	FT_Outline *outline = &glyph->outline;

	printf("glyph***\n");

	printf("format = %d\n", glyph->format);

	printf("metrics***\n");
	printf("  width        = %d, %.02f\n", metrics->width, (float)metrics->width / 64);
	printf("  height       = %d, %.02f\n", metrics->height, (float)metrics->height / 64);
	printf("  horiBearingX = %d, %.02f\n", metrics->horiBearingX, (float)metrics->horiBearingX / 64);
	printf("  horiBearingY = %d, %.02f\n", metrics->horiBearingY, (float)metrics->horiBearingY / 64);
	printf("  horiAdvance  = %d, %.02f\n", metrics->horiAdvance, (float)metrics->horiAdvance / 64);
	printf("  vertBearingX = %d, %.02f\n", metrics->vertBearingX, (float)metrics->vertBearingX / 64);
	printf("  vertBearingY = %d, %.02f\n", metrics->vertBearingY, (float)metrics->vertBearingY / 64);
	printf("  vertAdvance  = %d, %.02f\n", metrics->vertAdvance, (float)metrics->vertAdvance / 64);
	printf("***metrics\n");

	printf("advance.x    = %d, %.02f\n", glyph->advance.x, (float)glyph->advance.x / 64);
	printf("advance.y    = %d, %.02f\n", glyph->advance.y, (float)glyph->advance.y / 64);

	printf("bitmap***\n");
	printf("  pitch  = %d\n", bitmap->pitch);
	printf("  width  = %d\n", bitmap->width);
	printf("  height = %d\n", bitmap->rows);
	printf("***bitmap\n");
	printf("bitmap_left = %d\n", glyph->bitmap_left);
	printf("bitmap_top  = %d\n", glyph->bitmap_top);

	printf("outline***\n");
	printf("  n_contours = %d\n", outline->n_contours);
	printf("  n_points   = %d\n", outline->n_points);
	for (int i = 0; i < outline->n_points; i++) {
		printf("    %d, %d\n   >%.02f, %.02f\n", outline->points[i].x, outline->points[i].y,
											  (float)outline->points[i].x / 64, (float)outline->points[i].y / 64);
	}
	printf("  flags = %d\n", outline->flags);
	printf("***outline\n");

	printf("lsb_delta    = %d\n", glyph->lsb_delta);
	printf("rsb_delta    = %d\n", glyph->rsb_delta);

	printf("***glyph\n");
}
*/

void *MMSFBFont::loadFTGlyph(unsigned int character) {
	FT_GlyphSlot g = NULL;

	// load glyph but do NOT render a bitmap
	if (!FT_Load_Glyph((FT_Face)this->ft_face,
			FT_Get_Char_Index((FT_Face)this->ft_face, (FT_ULong)character), FT_LOAD_DEFAULT
//			FT_Get_Char_Index((FT_Face)this->ft_face, (FT_ULong)character), FT_LOAD_DEFAULT | FT_LOAD_FORCE_AUTOHINT
//		| FT_LOAD_TARGET_LIGHT
//		| FT_LOAD_TARGET_MONO
//		| FT_LOAD_TARGET_LCD
//		| FT_LOAD_TARGET_LCD_V
		)) {
		g = ((FT_Face)this->ft_face)->glyph;
	} else {
		MMSFB_SetError(0, "FT_Load_Glyph(,,FT_LOAD_DEFAULT) failed for " + this->filename);
	}

/*TEST CODE
	if (!FT_Load_Glyph((FT_Face)this->ft_face,
		FT_Get_Char_Index((FT_Face)this->ft_face, (FT_ULong)character), FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT)) {
		g = ((FT_Face)this->ft_face)->glyph;
	} else {
		MMSFB_SetError(0, "FT_Load_Glyph(,,FT_LOAD_RENDER) failed for " + this->filename);
	}
	showGlyphAttributes(g);
	exit(0);
*/

	if (g) {
#ifdef __HAVE_OPENGL__
		if (!mmsfb->bei) {
#else
		if (1) {
#endif
			// OpenGL is not initialized, we need a bitmap from freetype
			if (g->format != FT_GLYPH_FORMAT_BITMAP) {
				if (FT_Render_Glyph(g, FT_RENDER_MODE_NORMAL)) {
					// failed to load glyph
					MMSFB_SetError(0, "FT_Render_Glyph(,FT_RENDER_MODE_NORMAL) failed for " + this->filename);
					return NULL;
				}
			}

			if (g->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
				// failed to load glyph
				MMSFB_SetError(0, "glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY for " + this->filename);
				return NULL;
			}
		}
		else {
#ifndef __HAVE_GLU__
			// OpenGL is initialized but GLU is missing, we need a bitmap from freetype
			if (g->format != FT_GLYPH_FORMAT_BITMAP) {
				if (FT_Render_Glyph(g, FT_RENDER_MODE_NORMAL)) {
					// failed to load glyph
					MMSFB_SetError(0, "FT_Render_Glyph(,FT_RENDER_MODE_NORMAL) failed for " + this->filename);
					return NULL;
				}
			}

			if (g->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY) {
				// failed to load glyph
				MMSFB_SetError(0, "glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY for " + this->filename);
				return NULL;
			}
#endif
		}

		// successfully loaded
		return g;
	}
	else {
		// failed to load glyph
		return NULL;
	}
}


bool MMSFBFont::setupFTGlyph(unsigned int character, void *ftg, MMSFBFont_Glyph *glyph) {
	if (!ftg || !glyph) return false;
	FT_GlyphSlot g = (FT_GlyphSlot)ftg;

	glyph->character = character;

#ifdef __HAVE_OPENGL__
	if (!mmsfb->bei) {
#else
	if (1) {
#endif
		// OpenGL is not initialized, setup glyph for software rendering
		glyph->left		= g->bitmap_left;
		glyph->top		= g->bitmap_top;
		glyph->width	= g->bitmap.width;
		glyph->height	= g->bitmap.rows;
		glyph->advanceX	= g->advance.x / 64;
		glyph->pitch	= g->bitmap.pitch;

#ifdef __HAVE_OPENGL__
#ifdef __HAVE_GLU__
		glyph->meshes = NULL;
		glyph->outline = NULL;
#else
		glyph->texture = 0;
#endif
#endif
		glyph->buffer	= (unsigned char*)calloc(1, glyph->pitch * glyph->height);
		memcpy(glyph->buffer, g->bitmap.buffer, glyph->pitch * glyph->height);

		if (MMSFBBase_rotate180) {
			// rotate glyph by 180°
			rotateUCharBuffer180(glyph->buffer, glyph->pitch, glyph->width, glyph->height);
		}

		return true;
	}

#ifndef __HAVE_GLU__
#ifdef  __HAVE_OPENGL__
	// OpenGL is initialized but GLU is missing, we need a bitmap from freetype
	glyph->left		= g->bitmap_left;
	glyph->top		= g->bitmap_top;
	glyph->width	= g->bitmap.width;
	glyph->height	= g->bitmap.rows;
	glyph->advanceX	= g->advance.x / 64;

	/*
	printf("left %d width %d advanceX %d top %d height %d fonth %d - %d\n",
			glyph->left, glyph->width, glyph->advanceX, glyph->top, glyph->height, this->height, g->advance.y / 64);
	*/

	if (!g->bitmap.pitch) {
		// glyph has no bitmap (e.g. space char)
		glyph->pitch = 0;
		glyph->buffer = NULL;
		glyph->texture = 0;
		return true;
	}

	// add glyph to charmap, we use a pitch which is a multiple of 4 needed e.g. for OGL textures
	if(mmsfb->bei && (g->bitmap.pitch & 3)) {
		glyph->pitch = (g->bitmap.pitch & ~3) + 4;
		glyph->buffer = (unsigned char*)calloc(1, glyph->pitch * glyph->height);
		unsigned char *src = g->bitmap.buffer;
		unsigned char *dst = glyph->buffer;
		for(int i = 0; i < glyph->height; i++) {
			memcpy(dst, src, glyph->pitch);
			src += g->bitmap.pitch;
			dst += glyph->pitch;
		}
	}
	else {
		glyph->pitch  = g->bitmap.pitch;
		glyph->buffer = (unsigned char*)calloc(1, glyph->pitch * glyph->height);
		memcpy(glyph->buffer, g->bitmap.buffer, glyph->pitch * glyph->height);
	}

	if (MMSFBBase_rotate180) {
		// rotate glyph by 180°
		rotateUCharBuffer180(glyph->buffer, glyph->pitch, glyph->width, glyph->height);
	}

	// create a texture for this glyph
	glyph->texture = 0;
	mmsfb->bei->createAlphaTexture(&glyph->texture, glyph->buffer,
							glyph->pitch, glyph->height);

	return true;
#endif

#else

	// OpenGL is initialized and GLU is available, we create meshes based on freetype outlines

bool with_outline = true;

	// init glyph basics
	glyph->buffer	= NULL;
	glyph->pitch	= 0;
/*	glyph->left		= (float)g->metrics.horiBearingX * this->scale_coeff;
	if (glyph->left>= 0) glyph->left = (glyph->left + 32) / 64; else glyph->left = (glyph->left - 32) / 64;
	glyph->top		= (float)g->metrics.horiBearingY * this->scale_coeff;
	if (glyph->top >= 0) glyph->top = (glyph->top + 32) / 64; else glyph->top = (glyph->top - 32) / 64;
	glyph->width	= ((float)g->metrics.width * this->scale_coeff + 32) / 64;
	glyph->height	= ((float)g->metrics.height * this->scale_coeff + 32) / 64;
	glyph->advanceX	= ((float)g->advance.x * this->scale_coeff + 32) / 64;
*/

	glyph->left		= g->metrics.horiBearingX / 64;
	glyph->top		= g->metrics.horiBearingY / 64;
	glyph->width	= g->metrics.width / 64;
	glyph->height	= g->metrics.height / 64;
	glyph->advanceX	= g->advance.x / 64;

	/*
	printf("left %d width %d advanceX %d top %d height %d fonth %d - %d\n",
			glyph->left, glyph->width, glyph->advanceX, glyph->top, glyph->height, this->height, g->advance.y / 64);
	*/

	// init glyph mesh and outline description
	glyph->meshes = NULL;
	glyph->outline = NULL;

	// my mesh id
	unsigned int subkey_mesh = glyph->character;
	glyph->meshes = new MMSFBBuffer(this->font_id, subkey_mesh);

	if (!with_outline) {
		// without outline
		if (glyph->meshes->isInitialized()) {
			// meshes already initialized
//			printf("MMSFBFont: meshes already initialized\n");
			return true;
		}
	}
	else {
		// my outline id
		unsigned int subkey_outline = 0x80000000 | glyph->character;
		glyph->outline = new MMSFBBuffer(this->font_id, subkey_outline);

		if (glyph->meshes->isInitialized() && glyph->outline->isInitialized()) {
			// meshes and outline already initialized
//			printf("MMSFBFont: meshes and outline already initialized\n");
			return true;
		}
	}

	// init tesselator
	MMSFTTesselator *ftv = new MMSFTTesselator(g);

	if (!glyph->meshes->isInitialized()) {
//		printf("MMSFBFont: have to generate meshes\n");

		ftv->generateGlyph();
		const MMSFTGlyph *ftglyph = ftv->getGlyph();
		if (!ftglyph) {
			// glyph not generated
			MMSFB_SetError(0, "MMSFTTesselator::generateGlyph() failed");
			delete ftv;
			return false;
		}

		if (!ftglyph->getMeshCount()) {
			// no meshes available, but o.k. (e.g. space char)
			delete ftv;
			return true;
		}

		// count max meshes
		unsigned short int max_meshes = 0;
		for (unsigned int m = 0; m < ftglyph->getMeshCount(); m++) {
			if (!ftglyph->getMesh(m)) continue;
			max_meshes++;
			if (max_meshes >= MMSFBFONT_GLYPH_MAX_MESHES) {
				printf("MMSFBFONT_GLYPH_MAX_MESHES(%u) reached, %u needed\n", MMSFBFONT_GLYPH_MAX_MESHES, ftglyph->getMeshCount());
			}
		}

		if (!max_meshes) {
			// no meshes available
			MMSFB_SetError(0, "no meshes available");
			delete ftv;
			return false;
		}

		// allocate base buffer for vertices and indices
		// we do not need to clear because all fields will be set separately
		MMSFBBuffer::INDEX_BUFFER index_buffer;
		MMSFBBuffer::VERTEX_BUFFER vertex_buffer;
		index_buffer.num_arrays = 0;
		index_buffer.max_arrays = max_meshes;
		index_buffer.arrays = (MMS_INDEX_ARRAY*)malloc(sizeof(MMS_INDEX_ARRAY) * index_buffer.max_arrays);
		vertex_buffer.num_arrays = 0;
		vertex_buffer.max_arrays = max_meshes;
		vertex_buffer.arrays = (MMS_VERTEX_ARRAY*)malloc(sizeof(MMS_VERTEX_ARRAY) * vertex_buffer.max_arrays);

		// for all meshes
		for (unsigned int m = 0; m < ftglyph->getMeshCount(); m++) {
			// prepare access to vertices and indices of glyph
			if (index_buffer.num_arrays >= max_meshes) {
				printf("max_meshes(%u) reached\n", max_meshes);
				break;
			}
			MMS_INDEX_ARRAY  *indices  = &index_buffer.arrays[index_buffer.num_arrays];
			MMS_VERTEX_ARRAY *vertices = &vertex_buffer.arrays[vertex_buffer.num_arrays];

			// get access to polygon data
			const MMSFTMesh *ftmesh = ftglyph->getMesh(m);
			if (!ftmesh) continue;

			// prepare indices
			// note: no need to allocate index buffer, because vertices are correctly sorted
			switch (ftmesh->getMeshType()) {
			case GL_TRIANGLES:
				initIndexArray(indices, MMS_INDEX_ARRAY_TYPE_TRIANGLES);
				break;
			case GL_TRIANGLE_STRIP:
				initIndexArray(indices, MMS_INDEX_ARRAY_TYPE_TRIANGLE_STRIP);
				break;
			case GL_TRIANGLE_FAN:
				initIndexArray(indices, MMS_INDEX_ARRAY_TYPE_TRIANGLE_FAN);
				break;
			default:
				// unsupported type
				printf("MMSFBFont: unsupported mesh type %u\n", ftmesh->getMeshType());
				delete ftv;
				return false;
			}

#ifndef __HAVE_OGL_HALF_FLOAT__
			// prepare vertices using normal 32bit floating point values
			initVertexArray(vertices, 2, ftmesh->getVertexCount(), MMS_VERTEX_DATA_TYPE_FLOAT);
#else
			// prepare vertices using 16bit half floating point values
			initVertexArray(vertices, 2, ftmesh->getVertexCount(), MMS_VERTEX_DATA_TYPE_HALF_FLOAT);
#endif
			// for all vertices in the polygon
			for (unsigned int v = 0; v < ftmesh->getVertexCount(); v++) {
				const MMSFTVertex &vertex = ftmesh->getVertex(v);
				MMS_VA_SET_VERTEX_2v(vertices, v,
									 (float)(vertex.X() - g->metrics.horiBearingX) / 64,
									 (float)(g->metrics.horiBearingY - vertex.Y()) / 64);
			}

			// next mesh
			index_buffer.num_arrays++;
			vertex_buffer.num_arrays++;
		}

		glyph->meshes->initBuffer(index_buffer, vertex_buffer);
	}

	if (with_outline && ftv->getContourCount() > 0) {
		if (!glyph->outline->isInitialized()) {
//			printf("MMSFBFont: have to generate outline\n");

			// add outline primitives
			unsigned short int max_outlines = ftv->getContourCount();

			// allocate base buffer for vertices and indices
			// we do not need to clear because all fields will be set separately
			MMSFBBuffer::INDEX_BUFFER index_buffer;
			MMSFBBuffer::VERTEX_BUFFER vertex_buffer;
			index_buffer.num_arrays = 0;
			index_buffer.max_arrays = max_outlines;
			index_buffer.arrays = (MMS_INDEX_ARRAY*)malloc(sizeof(MMS_INDEX_ARRAY) * index_buffer.max_arrays);
			vertex_buffer.num_arrays = 0;
			vertex_buffer.max_arrays = max_outlines;
			vertex_buffer.arrays = (MMS_VERTEX_ARRAY*)malloc(sizeof(MMS_VERTEX_ARRAY) * vertex_buffer.max_arrays);

			// for all contours (outlines)
			for (unsigned int c = 0; c < ftv->getContourCount(); c++) {
				// prepare access to vertices and indices of glyph
				if (index_buffer.num_arrays >= max_outlines) {
					printf("max_outlines(%u) reached\n", max_outlines);
					break;
				}
				MMS_INDEX_ARRAY  *indices  = &index_buffer.arrays[index_buffer.num_arrays];
				MMS_VERTEX_ARRAY *vertices = &vertex_buffer.arrays[vertex_buffer.num_arrays];

				// get access to contour data
				const MMSFTContour *ftcontour = ftv->getContour(c);
				if (!ftcontour) continue;

				// prepare indices
				// note: no need to allocate index buffer, because vertices are correctly sorted
				initIndexArray(indices, MMS_INDEX_ARRAY_TYPE_LINE_LOOP);

#ifndef __HAVE_OGL_HALF_FLOAT__
				// prepare vertices using normal 32bit floating point values
				initVertexArray(vertices, 2, ftcontour->getVertexCount(), MMS_VERTEX_DATA_TYPE_FLOAT);
#else
				// prepare vertices using 16bit half floating point values
				initVertexArray(vertices, 2, ftcontour->getVertexCount(), MMS_VERTEX_DATA_TYPE_HALF_FLOAT);
#endif
				// for all vertices in the polygon
				for (unsigned int v = 0; v < ftcontour->getVertexCount(); v++) {
					const MMSFTVertex &vertex = ftcontour->Vertex(v);
					MMS_VA_SET_VERTEX_2v(vertices, v,
										 (float)(vertex.X() - g->metrics.horiBearingX) / 64,
										 (float)(g->metrics.horiBearingY - vertex.Y()) / 64);
				}

				// next outline
				index_buffer.num_arrays++;
				vertex_buffer.num_arrays++;
			}

			glyph->outline->initBuffer(index_buffer, vertex_buffer);
		}
	}


// Test mit dreiecken zieht strom!!! dazu kommt noch das Skalierungsproblem...
/*	if (with_outline && ftv->getContourCount() > 0) {
		if (!glyph->outline->isInitialized()) {

			glyph->top		+= 0;
			glyph->width	+= 2;
			glyph->height	+= 2;
			glyph->advanceX	+= 2;


			// add outline primitives
			unsigned short int max_outlines = ftv->getContourCount();

			// allocate base buffer for vertices and indices
			// we do not need to clear because all fields will be set separately
			MMSFBBuffer::INDEX_BUFFER index_buffer;
			MMSFBBuffer::VERTEX_BUFFER vertex_buffer;
			index_buffer.num_arrays = 0;
			index_buffer.max_arrays = max_outlines;
			index_buffer.arrays = (MMS_INDEX_ARRAY*)malloc(sizeof(MMS_INDEX_ARRAY) * index_buffer.max_arrays);
			vertex_buffer.num_arrays = 0;
			vertex_buffer.max_arrays = max_outlines;
			vertex_buffer.arrays = (MMS_VERTEX_ARRAY*)malloc(sizeof(MMS_VERTEX_ARRAY) * vertex_buffer.max_arrays);

			// for all contours (outlines)
			for (unsigned int c = 0; c < ftv->getContourCount(); c++) {
				// prepare access to vertices and indices of glyph
				if (index_buffer.num_arrays >= max_outlines) {
					printf("max_outlines(%u) reached\n", max_outlines);
					break;
				}
				MMS_INDEX_ARRAY  *indices  = &index_buffer.arrays[index_buffer.num_arrays];
				MMS_VERTEX_ARRAY *vertices = &vertex_buffer.arrays[vertex_buffer.num_arrays];

				// get access to contour data
				const MMSFTContour *ftcontour = ftv->getContour(c);
				if (!ftcontour) continue;

				// prepare indices
				// note: no need to allocate index buffer, because vertices are correctly sorted
				initIndexArray(indices, MMS_INDEX_ARRAY_TYPE_TRIANGLES);

				// prepare vertices using normal 32bit floating point values
				initVertexArray(vertices, 2, ftcontour->getVertexCount() * 6, MMS_VERTEX_DATA_TYPE_FLOAT);
printf("*********************\n");
				for (unsigned int v = 0; v < ftcontour->getVertexCount(); v++) {
					const MMSFTVertex &vertex1 = ftcontour->Vertex(v);
					const MMSFTVertex &outset1 = ftcontour->Outset(v);
					const MMSFTVertex &vertex2 = ftcontour->Vertex((v+1 < ftcontour->getVertexCount()) ? v+1 : 0);
					const MMSFTVertex &outset2 = ftcontour->Outset((v+1 < ftcontour->getVertexCount()) ? v+1 : 0);
printf("v: %f,%f\n", vertex1.X(), vertex1.Y());
printf("o: %f,%f\n", outset1.X(), outset1.Y());

					float fac=3.0f;

					float *vdata = (float *)vertices->data;


					vdata[v*6 * vertices->eSize + 0] = (float)(vertex1.X() - g->metrics.horiBearingX) / 64;
					vdata[v*6 * vertices->eSize + 1] = (float)(g->metrics.horiBearingY - vertex1.Y()) / 64;

					vdata[v*6 * vertices->eSize + 2] = (float)(vertex1.X() + outset1.X() * fac - g->metrics.horiBearingX) / 64;
					vdata[v*6 * vertices->eSize + 3] = (float)(g->metrics.horiBearingY - vertex1.Y() - outset1.Y() * fac) / 64;

					vdata[v*6 * vertices->eSize + 4] = (float)(vertex2.X() - g->metrics.horiBearingX) / 64;
					vdata[v*6 * vertices->eSize + 5] = (float)(g->metrics.horiBearingY - vertex2.Y()) / 64;

					vdata[v*6 * vertices->eSize + 6] = (float)(vertex1.X() + outset1.X() * fac - g->metrics.horiBearingX) / 64;
					vdata[v*6 * vertices->eSize + 7] = (float)(g->metrics.horiBearingY - vertex1.Y() - outset1.Y() * fac) / 64;

					vdata[v*6 * vertices->eSize + 8] = (float)(vertex2.X() + outset2.X() * fac - g->metrics.horiBearingX) / 64;
					vdata[v*6 * vertices->eSize + 9] = (float)(g->metrics.horiBearingY - vertex2.Y() - outset2.Y() * fac) / 64;

					vdata[v*6 * vertices->eSize + 10]= (float)(vertex2.X() - g->metrics.horiBearingX) / 64;
					vdata[v*6 * vertices->eSize + 11]= (float)(g->metrics.horiBearingY - vertex2.Y()) / 64;
				}

				// next outline
				index_buffer.num_arrays++;
				vertex_buffer.num_arrays++;
			}

			glyph->outline->initBuffer(index_buffer, vertex_buffer);
		}
	}
*/

    // all is successfully done
	delete ftv;
	return true;

#endif

	return false;
}

bool MMSFBFont::getGlyph(unsigned int character, MMSFBFont_Glyph *glyph) {
	if (!glyph) {
		return false;
	}

    if (mmsfb->backend == MMSFB_BE_DFB) {
#ifdef  __HAVE_DIRECTFB__
#endif
    }
    else {
    	if(!this->ft_face) {
    		return false;
    	}

    	bool ret = false;

    	lock();

    	// check if requested character is already loaded
    	std::map<unsigned int, MMSFBFont_Glyph>::iterator it;
    	it = this->charmap.find(character);
    	if (it == this->charmap.end()) {
    		// no, have to load it
			FT_GlyphSlot g;
			if (!(g = (FT_GlyphSlot)loadFTGlyph(character))) {
				// failed to load glyph
				unlock();
				return false;
			}

			// setup glyph values
			if (!setupFTGlyph(character, g, glyph)) {
				// failed to setup glyph
				unlock();
				return false;
			}

			// add to charmap
			this->charmap.insert(std::make_pair(character, *glyph));
			ret = true;
    	}
    	else {
    		// already loaded
    		*glyph = it->second;
			ret = true;
    	}

    	unlock();

    	return ret;
    }

    return false;
}


bool MMSFBFont::getStringWidth(string text, int len, int *width) {
    // check if initialized
    INITCHECK;

    // reset width
    if (!width) return false;
	*width = 0;

	// get the length of the string
	if (len < 0) len = text.size();
	if (!len) return true;

    // get the width of the whole string
#ifdef  __HAVE_DIRECTFB__
    if (this->dfbfont) {
		if (((IDirectFBFont*)this->dfbfont)->GetStringWidth((IDirectFBFont*)this->dfbfont, text.c_str(), len, width) != DFB_OK)
			return false;
		return true;
    } else
#endif
    {
    	MMSFBFONT_GET_UNICODE_CHAR(text, len) {
    		MMSFBFont_Glyph glyph;
    		if (!getGlyph(character, &glyph)) break;

#if (defined(__HAVE_OPENGL__) && defined(__HAVE_GLU__))
			if (mmsfb->bei) {
				// have to calculate advanceX because of scale coefficient
				(*width)+= (int)((float)glyph.advanceX * this->scale_coeff + 0.5f);
			}
			else
#endif
				(*width)+= glyph.advanceX;
    	} }
    	return true;
    }
    return false;
}

bool MMSFBFont::getHeight(int *height) {
    // check if initialized
    INITCHECK;

    // get the height of the font
#ifdef  __HAVE_DIRECTFB__
	if (this->dfbfont) {
		if (((IDirectFBFont*)this->dfbfont)->GetHeight((IDirectFBFont*)this->dfbfont, height) != DFB_OK)
			return false;
		return true;
    } else
#endif
	{
#if (defined(__HAVE_OPENGL__) && defined(__HAVE_GLU__))
		if (mmsfb->bei) {
			// have to calculate height because of scale coefficient
			int asc = (int)((float)this->ascender * this->scale_coeff + 0.5f);
			int des = (int)((float)this->descender * this->scale_coeff + 0.5f);
			*height = asc + des + 1;
		}
		else
#endif
			*height = this->height;

    	return true;
    }
    return false;
}

bool MMSFBFont::getAscender(int *ascender) {
    // check if initialized
    INITCHECK;

    // get the ascender of the font
#ifdef __HAVE_DIRECTFB__
    if (this->dfbfont) {
	} else
#endif
	{
#if (defined(__HAVE_OPENGL__) && defined(__HAVE_GLU__))
		if (mmsfb->bei) {
			// have to calculate ascender because of scale coefficient
			*ascender = (int)((float)this->ascender * this->scale_coeff + 0.5f);
		}
		else
#endif
			*ascender = this->ascender;

		return true;
	}
	return false;
}

bool MMSFBFont::getDescender(int *descender) {
    // check if initialized
    INITCHECK;

    // get the descender of the font
#ifdef __HAVE_DIRECTFB__
    if (this->dfbfont) {
	}
	else
#endif
	{
#if (defined(__HAVE_OPENGL__) && defined(__HAVE_GLU__))
		if (mmsfb->bei) {
			// have to calculate descender because of scale coefficient
			*descender = (int)((float)this->descender * this->scale_coeff + 0.5f);
		}
		else
#endif
			*descender = this->descender;

		return true;
	}
	return false;
}

bool MMSFBFont::getScaleCoeff(float *scale_coeff) {
    // check if initialized
    INITCHECK;

#if (defined(__HAVE_OPENGL__) && defined(__HAVE_GLU__))
	if (mmsfb->bei) {
		*scale_coeff = this->scale_coeff;
		return true;
	}
#endif

	return false;
}





