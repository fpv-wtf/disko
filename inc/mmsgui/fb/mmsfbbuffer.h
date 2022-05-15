/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009-2011 BerLinux Solutions GmbH                       *
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

#ifndef MMSFBBUFFER_H_
#define MMSFBBUFFER_H_

#include "mmstools/base.h"
#include "mmstools/mmstypes.h"
#include "mmsgui/fb/mmsfbgl.h"
#include <stdlib.h>

class MMSFBBuffer {
	public:
		//! list of index arrays
		typedef struct {
			//! index data
			MMS_INDEX_ARRAY	*arrays;
			//! maximum number of arrays
			unsigned short int	max_arrays;
			//! arrays which are initialized
			unsigned short int	num_arrays;
		} INDEX_BUFFER;

    	//! list of vertex arrays
    	typedef struct {
			//! vertex data
			MMS_VERTEX_ARRAY	*arrays;
			//! maximum number of arrays
			unsigned short int	max_arrays;
			//! arrays which are initialized
			unsigned short int	num_arrays;
		} VERTEX_BUFFER;

    	//! buffer types
    	typedef enum {
    		//! buffer not initialized
    		BUFFER_TYPE_NOTSET = 0,
    		//! index and vertex buffer
    		BUFFER_TYPE_INDEX_VERTEX
    	} BUFFER_TYPE;

		//! contains a OpenGL index buffer object with additional description
		typedef struct {
			//! OpenGL's buffer object
			unsigned int		bo;
			//! index data description
			MMS_INDEX_BUFFER	*buffers;
			//! number of buffers
			unsigned short int	num_buffers;
		} INDEX_BUFFER_OBJECT;

		//! contains a OpenGL vertex buffer object with additional description
		typedef struct {
			//! OpenGL's buffer object
			unsigned int		bo;
			//! vertex data description
			MMS_VERTEX_BUFFER	*buffers;
			//! number of buffers
			unsigned short int	num_buffers;
		} VERTEX_BUFFER_OBJECT;

    	//! extkey description
    	class EXTKEY {
			public:
				//! is extkey initialized?
				bool initialized;

				//! use count
				unsigned int	use_count;

				//! own key
				unsigned int	key;

#ifdef __HAVE_OPENGL__
				//! OpenGL's buffer object which contains indices
				unsigned int	ibo;
				unsigned int	ibo_size;
				unsigned int	ibo_used;

				//! OpenGL's buffer object which contains vertices
				unsigned int	vbo;
				unsigned int	vbo_size;
				unsigned int	vbo_used;
#endif

				EXTKEY(unsigned int key);
				~EXTKEY();
				bool allocVertexArray(unsigned int size);
				bool reserveIndexArray(unsigned int requested_size, unsigned int *offset);
				bool reserveVertexArray(unsigned int requested_size, unsigned int *offset);
    	};

    	//! buffer description
    	class BUFFER {
			public:
				//! is buffer initialized?
				bool initialized;

				//! use count
				unsigned int	use_count;

				//! type of buffer
				BUFFER_TYPE		type;

				//! index data
				INDEX_BUFFER	index_buffer;

				//! vertex data
				VERTEX_BUFFER	vertex_buffer;

#ifdef __HAVE_OPENGL__
				//! OpenGL's buffer object which contains indices
				INDEX_BUFFER_OBJECT		index_bo;

				//! OpenGL's buffer object which contains vertices
				VERTEX_BUFFER_OBJECT	vertex_bo;
#endif

				BUFFER();
				~BUFFER();
				void initIndexBuffer(EXTKEY *extkey, INDEX_BUFFER index_buffer);
				void initVertexBuffer(EXTKEY *extkey, VERTEX_BUFFER vertex_buffer);
				bool getBuffers(MMSFBBuffer::INDEX_BUFFER **index_buffer, MMSFBBuffer::VERTEX_BUFFER **vertex_buffer);
#ifdef __HAVE_OPENGL__
				bool getBufferObjects(MMSFBBuffer::INDEX_BUFFER_OBJECT **index_bo, MMSFBBuffer::VERTEX_BUFFER_OBJECT **vertex_bo);
#endif
    	};

	private:

		//! external ID of buffer (64bit, extkey + subkey)
		unsigned long long ext_id;

    	//! defines mapping between extkey of buffer and content
		typedef std::map<unsigned int, MMSFBBuffer::EXTKEY*> EXTKEY_INDEX;

		//! static key index
		static EXTKEY_INDEX extkey_index;

		//! pointer to extkey content
		EXTKEY *extkey;

		//! defines mapping between external ID of buffer and content
		typedef std::map<unsigned long long, MMSFBBuffer::BUFFER*> BUFFER_INDEX;

		//! static buffer index
		static BUFFER_INDEX buffer_index;

		//! pointer to buffer content
		BUFFER *buffer;

	public:
        MMSFBBuffer(unsigned int extkey, unsigned int subkey);
        ~MMSFBBuffer();
        bool isInitialized();
        bool getExtKey(MMSFBBuffer::EXTKEY **extkey);
        bool initBuffer(INDEX_BUFFER index_buffer, VERTEX_BUFFER vertex_buffer);
        bool getBuffer(MMSFBBuffer::BUFFER **buffer);
};

#endif /* MMSFBBUFFER_H_ */



