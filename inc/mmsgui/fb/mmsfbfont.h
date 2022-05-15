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

#ifndef MMSFBFONT_H_
#define MMSFBFONT_H_

#include "mmsgui/fb/mmsfbbase.h"
#include "mmsgui/fb/mmsfbbuffer.h"

#ifdef __HAVE_GLU__
#define MMSFBFONT_GLYPH_MAX_MESHES	512
#endif

//! descibes a loaded glyph
typedef struct {
	//! character code
	unsigned int	character;
	//! pointer to bitmap data
	unsigned char	*buffer;
	//! pitch in byte of one row in the bitmap buffer
	int 			pitch;
	//! x-offset
	int				left;
	//! y-offset
	int				top;
	//! width in pixel of the glyph
	int 			width;
	//! height in pixel of the glyph
	int 			height;
	//! width in pixel of the whole character
	int				advanceX;
#ifdef __HAVE_OPENGL__
#ifndef __HAVE_GLU__
	//! OpenGL texture for this glyph, we use bitmaps from freetype
	//! note: text rendering based on textures needs a lot of memory and is not the fastest way
	GLuint	texture;
#else
	//! OpenGL primitives for this glyph, we convert outlines from freetype using GLU tesselator
	//! note: text rendering based on primitives can be more than two times faster
	MMSFBBuffer	*meshes;
	MMSFBBuffer	*outline;
#endif
#endif
} MMSFBFont_Glyph;

//! Font rendering class.
/*!
\author Jens Schneider
*/
class MMSFBFont {
    private:
        //! true if initialized
        bool 		initialized;

        //! to make it thread-safe
        MMSMutex  	Lock;

#ifdef __HAVE_DIRECTFB__
        //! pointer to the directfb font
    	void 		*dfbfont;
#endif

    	//! static pointer to the freetype library
        static void *ft_library;

        //! pointer to the loaded freetype face
        void	*ft_face;

        //! font file
        string 	filename;

    	//! defines mapping between filename and font id
		typedef std::map<std::string, unsigned int> MMSFBFONT_MAP;

		//! static index
		static MMSFBFONT_MAP index;

		//! static index position
		static unsigned int index_pos;

		//! id of font
		unsigned int font_id;

#if (defined(__HAVE_OPENGL__) && defined(__HAVE_GLU__))
        //! scale coefficient
        float	scale_coeff;
#endif

    	//! ascender
    	int 	ascender;

    	//! descender
    	int		descender;

    	//! real height of one line
    	int 	height;

    	//! maps a character id to a already loaded glyph (see glyphpool)
    	std::map<unsigned int, MMSFBFont_Glyph> charmap;

    	//! reference counter for final release of freetype library
    	static unsigned int numReferences;

        void lock();
        void unlock();

        void *loadFTGlyph(unsigned int character);
        bool setupFTGlyph(unsigned int character, void *ftg, MMSFBFont_Glyph *glyph);

    public:
        MMSFBFont(string filename, int w, int h);
        virtual ~MMSFBFont();

        bool isInitialized();

        bool getStringWidth(string text, int len, int *width);
        bool getHeight(int *height);

        bool getAscender(int *ascender);
        bool getDescender(int *descender);

        bool getScaleCoeff(float *scale_coeff);

        bool getGlyph(unsigned int character, MMSFBFont_Glyph *glyph);

    	friend class MMSFBSurface;
    	friend class MMSFBBackEndInterface;
};

#define MMSFBFONT_GET_UNICODE_CHAR(text, len) \
	for (int cnt = 0; cnt < len; cnt++) { \
		unsigned char c = text[cnt]; \
		unsigned int character; \
		if(c >= 0xf0) /* 11110000 -> 4 bytes */ { \
			if(len < (cnt + 3)) { DEBUGMSG("MMSFBFONT", "invalid unicode string"); break; } \
			character = (unsigned int)((c & 0x07 /* 00000111 */) << 18); \
			character |= ((text[++cnt] & 0x3f /* 00111111 */) << 12); \
			character |= ((text[++cnt] & 0x3f) << 6); \
			character |= (text[++cnt] & 0x3f); \
		} else if(c >= 0xe0)  /* 11100000 -> 3 bytes */ { \
			if(len < (cnt + 2)) { DEBUGMSG("MMSFBFONT", "invalid unicode string"); break; } \
			character = (unsigned int)((c & 0x0f /* 00001111 */) << 12); \
			character |= ((text[++cnt] & 0x3f) << 6); \
			character |= (text[++cnt] & 0x3f); \
		} else if(c >= 0xc0)  /* 11000000 -> 2 bytes */ { \
			if(len < (cnt + 1)) { DEBUGMSG("MMSFBFONT", "invalid unicode string"); break; } \
			character = (unsigned int)(((c & 0x1f /* 00011111 */) << 6) | (text[++cnt] & 0x3f)); \
		} else  /* 1 byte */ { \
			character = (unsigned int)c; \
		}

#endif /*MMSFBFONT_H_*/
