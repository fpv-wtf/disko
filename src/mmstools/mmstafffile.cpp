/***************************************************************************
 *   Copyright (C) 2005-2007 Stefan Schwarzer, Jens Schneider,             *
 *                           Matthias Hardt, Guido Madaus                  *
 *                                                                         *
 *   Copyright (C) 2007-2008 BerLinux Solutions GbR                        *
 *                           Stefan Schwarzer & Guido Madaus               *
 *                                                                         *
 *   Copyright (C) 2009      BerLinux Solutions GmbH                       *
 *                                                                         *
 *   Authors:                                                              *
 *      Stefan Schwarzer   <stefan.schwarzer@diskohq.org>,                 *
 *      Matthias Hardt     <matthias.hardt@diskohq.org>,                   *
 *      Jens Schneider     <pupeider@gmx.de>,                              *
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

#include "mmstools/mmstafffile.h"
#include "mmstools/tools.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <png.h>



MMSTaffFile::MMSTaffFile(string taff_filename, TAFF_DESCRIPTION *taff_desc,
			             string external_filename, MMSTAFF_EXTERNAL_TYPE external_type,
			             bool ignore_blank_values, bool trace, bool print_warnings,
			             bool force_rewrite_taff, bool auto_rewrite_taff) {
	this->taff_filename = taff_filename;
	this->taff_desc = taff_desc;
	this->taff_buf = NULL;
	this->taff_buf_size = 0;
	this->taff_buf_pos = 0;
	this->loaded = false;
	this->correct_version = false;
	this->external_filename = external_filename;
	this->external_type = external_type;
	this->ignore_blank_values = ignore_blank_values;
	this->trace = trace;
	this->print_warnings = print_warnings;
	this->destination_pixelformat = MMSTAFF_PF_ARGB;
	this->destination_premultiplied = true;
	this->mirror_size = 0;
	this->correct_version = false;
	this->current_tag = -1;
	this->current_tag_pos = 0;

	if (!this->taff_desc)
		if (this->external_type == MMSTAFF_EXTERNAL_TYPE_IMAGE)
			this->taff_desc = &mmstaff_image_taff_description;

	if ((this->taff_filename == "")&&(this->external_filename == ""))
		return;

	// read the file into taff_buf
	bool ret = false;
	if (!force_rewrite_taff)
		ret = readFile();
	if (!ret)
		// failed or rewrite, try to convert from external_filename
		if ((force_rewrite_taff)||(auto_rewrite_taff))
			if (convertExternal2TAFF())
				// auto read in
				readFile();
}

MMSTaffFile::~MMSTaffFile() {
	if (this->taff_buf)
		free(this->taff_buf);
}

bool MMSTaffFile::writeBuffer(MMSFile *mmsfile, void *ptr, size_t *ritems, size_t size, size_t nitems, bool *write_status) {
	if (!mmsfile) {
		if (write_status) *write_status = false;
		return false;
	}

	if (!mmsfile->writeBuffer(ptr, ritems, size, nitems)) {
		// error while writing file
		printf("TAFF: Error while writing to file %s\n", mmsfile->getName().c_str());
		if (write_status) *write_status = false;
		return false;
	}

	return true;
}

bool MMSTaffFile::readPNG(const char *filename, void **buf, int *width, int *height, int *pitch, int *size) {
	FILE 			*fp;
	char			png_sig[8];
    png_structp     png_ptr = NULL;
    png_infop       info_ptr = NULL;
    png_infop       end_info_ptr = NULL;
    png_bytep       *row_pointers = NULL;

    // check if file does exist and if it is an png format
    *buf = NULL;
    fp = fopen(filename, "rb");
    if (!fp)
    	return false;
    if (fread(png_sig, 1, sizeof(png_sig), fp)!=sizeof(png_sig)) {
        fclose(fp);
    	return false;
    }
#if PNG_LIBPNG_VER_MINOR == 2
    if (!png_check_sig((png_byte*)png_sig, sizeof(png_sig))) {
#else
   	if (png_sig_cmp((png_byte*)png_sig, 0, sizeof(png_sig)) != 0) {
#endif
        fclose(fp);
    	return false;
    }

    // init png structs and abend handler
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        fclose(fp);
    	return false;
    }
    png_set_sig_bytes(png_ptr, sizeof(png_sig));

    if(setjmp(png_jmpbuf(png_ptr))) {
    	// abend from libpng
        printf("png read error\n");
    	png_destroy_read_struct(&png_ptr, (info_ptr)?&info_ptr:NULL, (end_info_ptr)?&end_info_ptr:NULL);
        if (row_pointers) free(row_pointers);
    	if (*buf) free(*buf);
        *buf = NULL;
        fclose(fp);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
    	png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
    	return false;
    }

    end_info_ptr = png_create_info_struct(png_ptr);
    if (!end_info_ptr) {
    	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
    	return false;
    }

    // read png infos
    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);
    png_uint_32 w;
    png_uint_32 h;
    int bit_depth;
    int color_type;
    png_get_IHDR(png_ptr, info_ptr, &w, &h, &bit_depth, &color_type, NULL, NULL, NULL);

    // check the png format
    if (((bit_depth != 8)&&(bit_depth != 16)) || (color_type != PNG_COLOR_TYPE_RGB_ALPHA)) {
    	// we only support ARGB png images
    	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);
        fclose(fp);
    	return false;
    }
    *width = w;
    *height = h;
    *pitch = 4 * w;
    *size = *pitch * h;

    // set input transformations
    if (bit_depth == 16)
    	png_set_strip_16(png_ptr);
    png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);
    png_set_bgr(png_ptr);
    png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    // allocate memory for row pointers
    row_pointers = (png_bytep*)malloc(*height * sizeof(png_bytep));
    if (!row_pointers) {
    	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);
        fclose(fp);
    	return false;
    }

    // allocate memory for image data
    if (this->mirror_size > *height) this->mirror_size = *height;
    *buf = malloc((*size) + (*pitch) * this->mirror_size);
    if (!*buf) {
    	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);
        free(row_pointers);
        fclose(fp);
    	return false;
    }
    char *b = (char*)*buf;
    for (int i = 0; i < *height; i++) {
    	row_pointers[i]=(png_byte*)b;
    	b+=*pitch;
    }

    // read the image data
    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, end_info_ptr);

    // all right, freeing helpers
	png_destroy_read_struct(&png_ptr, &info_ptr, &end_info_ptr);
    free(row_pointers);
    fclose(fp);

    // at this point we have ARGB (MMSTAFF_PF_ARGB) pixels ********
    // so check now if i have to convert it

    // should i pre-multiply with alpha channel?
    if (this->destination_premultiplied) {
		unsigned int *src = (unsigned int*)*buf;
	    for (int i = *width * *height; i > 0; i--) {
	    	register unsigned int s = *src;
	        register unsigned int sa = s >> 24;
	        if (sa != 0xff)
	        	// source alpha is > 0x00 and < 0xff
		        *src = ((((s & 0x00ff00ff) * sa) >> 8) & 0x00ff00ff) |
		               ((((s & 0x0000ff00) * sa) >> 8) & 0x0000ff00) |
		               ((((s & 0xff000000))));
	        src++;
	    }
    }

    // should create a mirror effect?
    if (this->mirror_size > 0) {
    	// yes
		unsigned int *dst = (unsigned int*)*buf;
		dst+=*width * *height;
		unsigned int *src = dst - *width;
		unsigned int alpha = 170;
		unsigned int alphaX = 0x050 / this->mirror_size;
		if (0x050 % this->mirror_size >= (this->mirror_size >> 1))
			alphaX++;
		if (alphaX == 0) alphaX = 1;
    	for (int i = 0; i < this->mirror_size; i++) {
    		for (int j = 0; j < *width; j++) {
    			register unsigned int s = *src;
    			register unsigned int sa = s >> 24;
    			*dst = (s & 0x00ffffff) | (((sa > alpha)?(sa-alpha):0)<<24);
    			dst++;
    			src++;
    		}
    		src-=(*width) << 1;
    		alpha+=alphaX;
    	}

    	// set new values
    	*height = (*height) + this->mirror_size;
    	*size = (*pitch) * (*height);
    }

    // have to convert the pixelformat?
    switch (this->destination_pixelformat) {
    case MMSTAFF_PF_AiRGB: {
    		// invert the alpha channel
			unsigned int *src = (unsigned int*)*buf;
		    for (int i = *width * *height; i > 0; i--) {
		    	register unsigned int s = *src;
		    	register unsigned int a = s;
		    	a = ~a;
		    	a = a & 0xff000000;
		    	s = s & 0x00ffffff;
		    	s = s | a;
		    	*src = s;
		    	src++;
		    }
    	}
   		break;
    case MMSTAFF_PF_AYUV: {
    		// convert RGB to YUV color space
			unsigned int *src = (unsigned int*)*buf;
		    for (int i = *width * *height; i > 0; i--) {
		    	register unsigned int s = *src;
		    	register int r = (s >> 16) & 0xff;
		    	register int g = (s >> 8) & 0xff;
		    	register int b = s  & 0xff;
		    	s = s & 0xff000000;											//A
		    	if (s) {
			    	s = s | (((((66*r+129*g+25*b+128)>>8)+16) & 0xff) << 16);	//Y
			    	s = s | (((((-38*r-74*g+112*b+128)>>8)+128) & 0xff) << 8);	//U (Cb)
			    	s = s | ((((112*r-94*g-18*b+128)>>8)+128) & 0xff);			//V (Cr)
		    	}
		    	*src = s;
		    	src++;
		    }
    	}
    	break;
    case MMSTAFF_PF_ARGB4444: {
			// divide ARGB data into halves
			*pitch = (*pitch) >> 1;
			*size = (*size) >> 1;
			void *newbuf = malloc(*size);
			if (!newbuf) {
				free(*buf);
				*buf = NULL;
				return false;
			}

			// convert it
			unsigned int *src = (unsigned int*)*buf;
			unsigned short int *dst = (unsigned short int*)newbuf;
		    for (int i = *width * *height; i > 0; i--) {
		    	register unsigned int s = *src;
		    	register unsigned short int d;
		    	d =   ((s & 0xf0000000) >> 16)
					| ((s & 0x00f00000) >> 12)
					| ((s & 0x0000f000) >> 8)
					| ((s & 0x000000f0) >> 4);
		    	*dst = d;
		    	src++;
		    	dst++;
		    }

			// switch buffers
			free(*buf);
			*buf = newbuf;
		}
		break;
    case MMSTAFF_PF_RGB16: {
			// convert into RGB16 format (remove alpha channel)
			*pitch = (*pitch) >> 1;
			*size = (*size) >> 1;
			void *newbuf = malloc(*size);
			if (!newbuf) {
				free(*buf);
				*buf = NULL;
				return false;
			}

			// convert it
			unsigned int *src = (unsigned int*)*buf;
			unsigned short int *dst = (unsigned short int*)newbuf;
		    for (int i = *width * *height; i > 0; i--) {
		    	// get src
		    	register unsigned int SRC = *src;
		    	register unsigned int A = SRC >> 24;
		    	register unsigned int SA= 0x100 - A;
		    	register unsigned short int d;

		    	// set background color to black
				unsigned int r = 0;
				unsigned int g = 0;
				unsigned int b = 0;

				// invert src alpha
			    r = SA * r;
			    g = SA * g;
			    b = (SA * b) >> 5;

			    // add src to dst
			    r += (A*(SRC & 0xf80000)) >> 19;
				g += (A*(SRC & 0xfc00)) >> 5;
				b += (A*(SRC & 0xf8)) >> 8;
				d =   ((r & 0xffe000)   ? 0xf800 : ((r >> 8) << 11))
			    	| ((g & 0xfff80000) ? 0x07e0 : ((g >> 13) << 5))
			    	| ((b & 0xff00)     ? 0x1f 	 : (b >> 3));
		    	*dst = d;
		    	src++;
		    	dst++;
		    }

			// switch buffers
			free(*buf);
			*buf = newbuf;
		}
		break;
	default: break;
    }

	return true;
}

bool MMSTaffFile::convertXML2TAFF_throughDoc(int depth, void *void_node, MMSFile *taff_file) {

	bool wok = true;
	xmlNode *node = (xmlNode*) void_node;
	xmlNode *cur_node;
	if (depth==0)
		/* work with root */
		cur_node = node;
	else
		/* iterate through childs */
		cur_node = node->children;

	while (cur_node) {

		int tagid = 0;
		bool tag_found = false;

		while (this->taff_desc->tagtable[tagid].name) {
			if (xmlStrcmp(cur_node->name, (const xmlChar *)this->taff_desc->tagtable[tagid].name)==0) {
				/* tag found in XML and tagtable, check the type? */
				if (this->taff_desc->tagtable[tagid].typeattr)
				{
					/* searching the typeattr and type */
					for (xmlAttr *cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {
						if (xmlStrcmp(cur_attr->name, (const xmlChar *)this->taff_desc->tagtable[tagid].typeattr)==0) {
							xmlChar *attrVal = xmlGetProp(cur_node, cur_attr->name);
					    	if (attrVal) {
								if (xmlStrcmp(attrVal, (const xmlChar *)this->taff_desc->tagtable[tagid].type)==0) {
									tag_found = true;
									break;
								}
								xmlFree(attrVal);
					    	}
						}
					}
				}
				else
					tag_found = true;

				if (tag_found)
					break;
			}
			tagid++;
		}

		if (tag_found) {

			if (this->trace)
				printf("Tag \"%s\" found, ID=%d\n", cur_node->name, tagid);

			/* writing the new tag */
			if (taff_file) {
				size_t ritems;
				unsigned char wb[2];
				wb[0]=MMSTAFF_TAGTABLE_TYPE_TAG;
				wb[1]=(unsigned char)tagid;
				writeBuffer(taff_file, wb, &ritems, 1, 2, &wok);
			}

			/* get variables */
			for (xmlAttr *cur_attr = cur_node->properties; cur_attr; cur_attr = cur_attr->next) {

				int attrid = 0;
				TAFF_ATTRDESC *attr = this->taff_desc->tagtable[tagid].attr;
				bool attr_found = false;

				if (!attr) continue;

				xmlChar *attrVal = xmlGetProp(cur_node, cur_attr->name);
		    	if (!attrVal) continue;

				while (attr[attrid].name)
				{
					if (xmlStrcmp(cur_attr->name, (const xmlChar *)attr[attrid].name)==0) {
						attr_found = true;
						break;
					}
					attrid++;
				}

				if (attr_found) {

					/* now check the type of the variable */
					string validvals = "";
					bool badval = false;
					bool int_val_set = false;
					int	 int_val;
					switch (attr[attrid].type) {
					case TAFF_ATTRTYPE_NONE:
					case TAFF_ATTRTYPE_STRING:
					case TAFF_ATTRTYPE_BINDATA:
						break;
					case TAFF_ATTRTYPE_NE_STRING:
						badval = (!*attrVal);
						if (badval) {
							validvals = "any characters, but not empty";
							attr_found = false;
						}
						break;
					case TAFF_ATTRTYPE_BOOL:
						badval = ((xmlStrcmp(attrVal, (xmlChar *)"true"))&&(xmlStrcmp(attrVal, (xmlChar *)"false")));
						if (badval) {
							validvals = "\"true\", \"false\"";
							attr_found = false;
						}
						else {
							int_val_set = true;
							if (xmlStrcmp(attrVal, (xmlChar *)"true")==0)
								int_val = -1;
							else
								int_val = 0;
						}
						break;
					case TAFF_ATTRTYPE_UCHAR:
					case TAFF_ATTRTYPE_UCHAR100:
						badval = ((xmlStrlen(attrVal) < 1)||(xmlStrlen(attrVal) > 3));
						if (!badval) {
							char iv[3+1];
							int_val = atoi((char*)attrVal);
							sprintf(iv, "%d", int_val);
							badval = (xmlStrcmp(attrVal, (xmlChar *)iv));
							if (!badval) {
								if (attr[attrid].type == TAFF_ATTRTYPE_UCHAR100)
									badval = (int_val<0||int_val>100);
								else
									badval = (int_val<0||int_val>255);
							}
							int_val_set = !badval;
						}
						if (badval) {
							if (attr[attrid].type == TAFF_ATTRTYPE_UCHAR100)
								validvals = "\"0\"..\"100\"";
							else
								validvals = "\"0\"..\"255\"";
							attr_found = false;
						}
						break;
					case TAFF_ATTRTYPE_INT:
						char iv[11+1];
						int_val = atoi((char*)attrVal);
						sprintf(iv, "%d", int_val);
						badval = (xmlStrcmp(attrVal, (xmlChar *)iv));
						int_val_set = !badval;
						if (badval) {
							validvals = "\"-2147483648\"..\"2147483647\"";
							attr_found = false;
						}
						break;
					case TAFF_ATTRTYPE_STATE:
						badval = ((xmlStrcmp(attrVal, (xmlChar *)"true"))&&(xmlStrcmp(attrVal, (xmlChar *)"false"))
								&&(xmlStrcmp(attrVal, (xmlChar *)"auto")));
						if (badval) {
							validvals = "\"true\", \"false\", \"auto\"";
							attr_found = false;
						}
						else {
							int_val_set = true;
							if (xmlStrcmp(attrVal, (xmlChar *)"true")==0)
								int_val = -1;
							else
							if (xmlStrcmp(attrVal, (xmlChar *)"auto")==0)
								int_val = 0x01010101;
							else
								int_val = 0;
						}
						break;
					}

					/* check if the value is blank and i have to ignore it */
					if (this->ignore_blank_values)
						if (!*attrVal)
							continue;

					if (!attr_found) {
						printf("Error: Value \"%s\" is invalid for the attribute \"%s\" of tag \"%s\", line %d of file %s\n       valid values: %s\n",
										attrVal, cur_attr->name, cur_node->name, cur_node->line, external_filename.c_str(), validvals.c_str());
						return false;
					}


					/* all right */
					if (this->trace)
						printf(" Attribute \"%s\" found, ID=%d, value=\"%s\"\n", cur_attr->name, attrid, attrVal);

					/* write attribute value */
					if (taff_file) {
						if (!int_val_set) {
							/* get the length of the value INCLUDING the 0x00 because of fast read & parse of the TAFF */
							int attrvallen = xmlStrlen(attrVal) + 1;

							size_t ritems;
							unsigned char wb[3];
							wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
							wb[1]=(unsigned char)attrid;
							if (attrvallen >= 0xff) {
								/* in this case set 0xff as mark and append the full integer */
								wb[2]=0xff;
								writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
								writeBuffer(taff_file, &attrvallen, &ritems, 1, sizeof(int), &wok);
							}
							else {
								/* in this case write only one byte length */
								wb[2]=(unsigned char)attrvallen;
								writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
							}
							writeBuffer(taff_file, attrVal, &ritems, 1, attrvallen, &wok);
						}
						else {
							/* writing the value as INT and not as STRING */
							size_t ritems;
							unsigned char wb[3];
							wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
							wb[1]=(unsigned char)attrid;
							wb[2]=sizeof(int);
							writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
							writeBuffer(taff_file, &int_val, &ritems, 1, sizeof(int), &wok);
						}
					}
				}
				else {
					/* attribute is not defined, so we cannot found an ID of it */
					/* we store it with ID 0xff and its real name */
					if (this->trace)
						printf(" Attribute \"%s\" found without ID, value=\"%s\"\n", cur_attr->name, attrVal);

					if (this->print_warnings)
						printf("Warning: Attribute \"%s\" is not defined for tag \"%s\", line %d of file %s\n         We store it with the real name.\n",
								cur_attr->name, cur_node->name, cur_node->line, external_filename.c_str());

					/* write attribute value */
					if (taff_file) {
						/* get the length of the value INCLUDING the 0x00 because of fast read & parse of the TAFF */
						int attrnamlen = xmlStrlen(cur_attr->name) + 1;
						int attrvallen = xmlStrlen(attrVal) + 1;

						size_t ritems;
						unsigned char wb[3];
						wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
						wb[1]=MMSTAFF_ATTR_WITHOUT_ID;
						writeBuffer(taff_file, wb, &ritems, 1, 2, &wok);
						writeBuffer(taff_file, &attrnamlen, &ritems, 1, sizeof(int), &wok);
						writeBuffer(taff_file, (char*)cur_attr->name, &ritems, 1, attrnamlen, &wok);
						if (attrvallen >= 0xff) {
							/* in this case set 0xff as mark and append the full integer */
							wb[0]=0xff;
							writeBuffer(taff_file, wb, &ritems, 1, 1, &wok);
							writeBuffer(taff_file, &attrvallen, &ritems, 1, sizeof(int), &wok);
						}
						else {
							/* in this case write only one byte length */
							wb[0]=(unsigned char)attrvallen;
							writeBuffer(taff_file, wb, &ritems, 1, 1, &wok);
						}
						writeBuffer(taff_file, attrVal, &ritems, 1, attrvallen, &wok);
					}
				}

				xmlFree(attrVal);
			}

			/* call recursive to find childs */
			if (!convertXML2TAFF_throughDoc(depth+1, cur_node, taff_file))
				return false;

			/* writing the close tag */
			if (taff_file) {
				size_t ritems;
				unsigned char wb[2];
				wb[0]=MMSTAFF_TAGTABLE_TYPE_CLOSETAG;
				wb[1]=(unsigned char)tagid;
				writeBuffer(taff_file, wb, &ritems, 1, 2, &wok);
			}
		}
		else {
			if (xmlStrcmp(cur_node->name, (const xmlChar *)"text")&&xmlStrcmp(cur_node->name, (const xmlChar *)"comment")) {
				printf("Error: Tag \"%s\" is not defined, line %d of file %s\n", cur_node->name, cur_node->line, external_filename.c_str());
				return false;
			}
		}

		if (depth==0)
			break;
		else
			cur_node = cur_node->next;
	}

	// return with write status
	return wok;
}

bool MMSTaffFile::convertXML2TAFF() {

	bool   rc = false;
	xmlDoc *parser = NULL;

	LIBXML_TEST_VERSION

	/* check input parameters */
	if (!this->taff_desc) return false;
	if (this->external_filename=="") return false;


	/* read the XML file */
	parser = xmlReadFile(this->external_filename.c_str(),
			NULL,
			XML_PARSE_PEDANTIC |
			XML_PARSE_NOBLANKS | XML_PARSE_NONET | XML_PARSE_NODICT |
			XML_PARSE_NOXINCNODE
#if LIBXML_VERSION >= 20700
			| XML_PARSE_NOBASEFIX
#endif
			);

	if (parser) {

		/* open binary destination file */
		MMSFile *taff_file = NULL;
		if (this->taff_filename!="") {
			taff_file = new MMSFile(this->taff_filename.c_str(), MMSFM_WRITE);
			size_t ritems;
			bool wok = true;
			writeBuffer(taff_file, (void*)TAFF_IDENT, &ritems, 1, strlen(TAFF_IDENT), &wok);
			writeBuffer(taff_file, &(this->taff_desc->type), &ritems, 1, sizeof(this->taff_desc->type), &wok);
			writeBuffer(taff_file, &(this->taff_desc->version), &ritems, 1, sizeof(this->taff_desc->version), &wok);
			if (!wok) {
				// write error, close file and free
				delete taff_file;
			    xmlFreeDoc(parser);

		    	// reset the file
				taff_file = new MMSFile(this->taff_filename.c_str(), MMSFM_WRITE);
				if (taff_file) delete taff_file;

			    return false;
			}
		}

		// get the first element
		xmlNode* node = xmlDocGetRootElement(parser);

		// through the doc
		rc = convertXML2TAFF_throughDoc(0, node, taff_file);

		// close file and free
		if (taff_file)
			delete taff_file;
	    xmlFreeDoc(parser);

	    if (!rc) {
	    	// failed, reset the file
			taff_file = new MMSFile(this->taff_filename.c_str(), MMSFM_WRITE);
			if (taff_file) delete taff_file;
	    }
	}
	else {
		printf("Error: cannot read external file %s\n", this->external_filename.c_str());
	}

	return rc;
}

bool MMSTaffFile::convertIMAGE2TAFF() {
	bool   	rc = false;
	void	*png_buf;
	int 	png_width;
	int 	png_height;
	int     png_pitch;
	int     png_size;

	/* check input parameters */
	if (!this->taff_desc) return false;
	if (this->external_filename=="") return false;

	/* read the whole PNG image */
	if (readPNG(this->external_filename.c_str(), &png_buf, &png_width, &png_height, &png_pitch, &png_size)) {
		/* open binary destination file */
		MMSFile *taff_file = NULL;
		bool wok = true;
		if (this->taff_filename!="") {
			taff_file = new MMSFile(this->taff_filename.c_str(), MMSFM_WRITE);
			size_t ritems;
			writeBuffer(taff_file, (void*)TAFF_IDENT, &ritems, 1, strlen(TAFF_IDENT), &wok);
			writeBuffer(taff_file, &(this->taff_desc->type), &ritems, 1, sizeof(this->taff_desc->type), &wok);
			writeBuffer(taff_file, &(this->taff_desc->version), &ritems, 1, sizeof(this->taff_desc->version), &wok);

			/* write the tag */
			unsigned char wb[3];
			wb[0]=MMSTAFF_TAGTABLE_TYPE_TAG;
			wb[1]=MMSTAFF_IMAGE_TAGTABLE_TAG_RAWIMAGE;
			writeBuffer(taff_file, wb, &ritems, 1, 2, &wok);

			/* write attributes: width */
			wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
			wb[1]=MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_width;
			wb[2]=sizeof(int);
			writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
			writeBuffer(taff_file, &png_width, &ritems, 1, sizeof(int), &wok);

			/* write attributes: height */
			wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
			wb[1]=MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_height;
			wb[2]=sizeof(int);
			writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
			writeBuffer(taff_file, &png_height, &ritems, 1, sizeof(int), &wok);

			/* write attributes: pitch */
			wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
			wb[1]=MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_pitch;
			wb[2]=sizeof(int);
			writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
			writeBuffer(taff_file, &png_pitch, &ritems, 1, sizeof(int), &wok);

			/* write attributes: size */
			wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
			wb[1]=MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_size;
			wb[2]=sizeof(int);
			writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
			writeBuffer(taff_file, &png_size, &ritems, 1, sizeof(int), &wok);

			/* write attributes: pixelformat */
			wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
			wb[1]=MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_pixelformat;
			wb[2]=sizeof(int);
			writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
			int pf = (int)this->destination_pixelformat;
			writeBuffer(taff_file, &pf, &ritems, 1, sizeof(int), &wok);

			/* write attributes: premultiplied */
			wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
			wb[1]=MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_premultiplied;
			wb[2]=sizeof(bool);
			writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
			bool pm = (this->destination_premultiplied);
			writeBuffer(taff_file, &pm, &ritems, 1, sizeof(bool), &wok);

			/* write attributes: mirror_size */
			wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
			wb[1]=MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_mirror_size;
			wb[2]=sizeof(int);
			writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
			int ms = (int)this->mirror_size;
			writeBuffer(taff_file, &ms, &ritems, 1, sizeof(int), &wok);

			/* write attributes: data */
			wb[0]=MMSTAFF_TAGTABLE_TYPE_ATTR;
			wb[1]=MMSTAFF_IMAGE_RAWIMAGE_ATTR::MMSTAFF_IMAGE_RAWIMAGE_ATTR_IDS_data;
			wb[2]=0xff;
			writeBuffer(taff_file, wb, &ritems, 1, 3, &wok);
			writeBuffer(taff_file, &png_size, &ritems, 1, sizeof(int), &wok);
			writeBuffer(taff_file, png_buf, &ritems, 1, png_size, &wok);

			/* write the close tag */
			wb[0]=MMSTAFF_TAGTABLE_TYPE_CLOSETAG;
			wb[1]=MMSTAFF_IMAGE_TAGTABLE_TAG_RAWIMAGE;
			writeBuffer(taff_file, wb, &ritems, 1, 2, &wok);
		}

		// set rc
		rc = wok;

		// close file and free
		if (taff_file)
			delete taff_file;
		free(png_buf);

	    if (!rc) {
	    	// failed, reset the file
			taff_file = new MMSFile(this->taff_filename.c_str(), MMSFM_WRITE);
			if (taff_file) delete taff_file;
	    }
	}
	else {
		printf("Error: cannot read external file %s\n", this->external_filename.c_str());
	}

	return rc;
}

bool MMSTaffFile::convertExternal2TAFF() {
	switch (this->external_type) {
	case MMSTAFF_EXTERNAL_TYPE_XML:
		return convertXML2TAFF();
	case MMSTAFF_EXTERNAL_TYPE_IMAGE:
		return convertIMAGE2TAFF();
	}
	return false;
}

bool MMSTaffFile::convertTAFF2XML_throughDoc(int depth, int tagid, MMSFile *external_file) {
	size_t ritems;
	char wb[8*1024];

	TAFF_TAGTABLE *tagt = &(this->taff_desc->tagtable[tagid]);
	TAFF_ATTRDESC *attr = tagt->attr;

	if (this->trace)
		printf("Tag \"%s\" found, ID=%d\n", tagt->name, tagid);

	/* write tag */
	if (external_file) {
		*wb = '\n';
		memset(&wb[1], ' ', depth*4);
		sprintf(&wb[1+depth*4], "<%s", tagt->name);
		writeBuffer(external_file, wb, &ritems, 1, strlen(wb));
	}

	/* write attributes */
	char *attrval_str;
	int  attrval_int;
	char *attr_name;
	int  attrid;
	while ((attrid = getNextAttribute(&attrval_str, &attrval_int, &attr_name)) >= 0) {
		if (attrid != MMSTAFF_ATTR_WITHOUT_ID) {
			string attrval;
			switch (attr[attrid].type) {
			case TAFF_ATTRTYPE_BOOL:
				if (attrval_int)
					attrval = "true";
				else
					attrval = "false";
				break;
			case TAFF_ATTRTYPE_UCHAR:
			case TAFF_ATTRTYPE_UCHAR100:
			case TAFF_ATTRTYPE_INT:
				attrval = iToStr(attrval_int);
				break;
			case TAFF_ATTRTYPE_STATE:
				if ((attrval_int & 0xff) == 0x01)
					attrval = "auto";
				else
				if (attrval_int)
					attrval = "true";
				else
					attrval = "false";
				break;
			default:
				attrval = attrval_str;
				break;
			}

			if (this->trace)
				printf(" Attribute \"%s\" found, ID=%d, value=\"%s\"\n", attr[attrid].name, attrid, attrval.c_str());

			if (external_file) {
				*wb = '\n';
				memset(&wb[1], ' ', depth*4+4);
				sprintf(&wb[1+depth*4+4], "%s = \"%s\"", attr[attrid].name, attrval.c_str());
				writeBuffer(external_file, wb, &ritems, 1, strlen(wb));
			}
		}
		else {
			if (this->trace)
				printf(" Attribute \"%s\" found without ID, value=\"%s\"\n", attr_name, attrval_str);

			if (external_file) {
				*wb = '\n';
				memset(&wb[1], ' ', depth*4+4);
				sprintf(&wb[1+depth*4+4], "%s = \"%s\"", attr_name, attrval_str);
				writeBuffer(external_file, wb, &ritems, 1, strlen(wb));
			}
		}
	}

	/* close tag */
	if (external_file) {
		sprintf(wb, ">\n");
		writeBuffer(external_file, wb, &ritems, 1, strlen(wb));
	}

	/* through my child tags */
	while (1) {
		bool eof;
		int tid = getNextTag(eof);
		if (tid < 0) {
			/* close tag */
			if (external_file) {
				memset(wb, ' ', depth*4);
				sprintf(&wb[depth*4], "</%s>\n", tagt->name);
				writeBuffer(external_file, wb, &ritems, 1, strlen(wb));
			}
			return true;
		}
		if (convertTAFF2XML_throughDoc(depth+1, tid, external_file)==false)
			return false;
	}
}

bool MMSTaffFile::convertTAFF2XML() {
	if (!this->loaded) return false;

	/* get root tag */
	int tagid = getFirstTag();
	if (tagid < 0) return false;

	/* open binary destination file */
	MMSFile *external_file = NULL;
	if (this->external_filename!="")
		external_file = new MMSFile(this->external_filename.c_str(), MMSFM_WRITE);

	/* start with root */
	bool rc = convertTAFF2XML_throughDoc(0, tagid, external_file);

	if (external_file)
		delete external_file;

	return rc;
}

bool MMSTaffFile::convertTAFF2External() {
	switch (this->external_type) {
	case MMSTAFF_EXTERNAL_TYPE_XML:
		return convertTAFF2XML();
	case MMSTAFF_EXTERNAL_TYPE_IMAGE:
		printf("TAFF: Currently we cannot convert taff to image\n");
		return false;
	}
	return false;
}

bool MMSTaffFile::readFile() {
	if (this->taff_buf) {
		free(this->taff_buf);
		this->taff_buf = NULL;
	}
	this->loaded = false;

	if (!this->taff_desc) return false;
	if (this->taff_filename=="") return false;

	// load the file
	MMSFile *taff_file = new MMSFile(this->taff_filename.c_str(), MMSFM_READ, false);
	if (!taff_file) return false;
	size_t ritems;
	char taff_ident[32];
	if (!taff_file->readBuffer((void*)taff_ident, &ritems, 1, strlen(TAFF_IDENT))) {
		// read error
		this->taff_buf = NULL;
		delete taff_file;
		return false;
	}
	if (ritems == 0) {
		// file is empty
		printf("TAFF: File is empty (%s)\n", this->taff_filename.c_str());
		this->taff_buf = NULL;
		delete taff_file;
		return false;
	}
	if (memcmp(taff_ident, TAFF_IDENT, strlen(TAFF_IDENT))!=0) {
		// the first bytes of the file are different from TAFF_IDENT
		printf("TAFF: TAFF_IDENT mismatch (%s)\n", this->taff_filename.c_str());
		this->taff_buf = NULL;
		delete taff_file;
		return false;
	}
	if (!taff_file->readBufferEx((void**)&(this->taff_buf), &ritems)) {
		// read error
		this->taff_buf = NULL;
		delete taff_file;
		return false;
	}
	delete taff_file;

	if (ritems < 40) {
		// wrong size
		free(this->taff_buf);
		this->taff_buf = NULL;
		return false;
	}

	// check the version of the file
	this->correct_version = false;
	if (strcmp((char*)this->taff_buf, (char*)&(this->taff_desc->type))) {
		// wrong type
		printf("TAFF: Wrong TAFF type (%s)\n", this->taff_filename.c_str());
		free(this->taff_buf);
		this->taff_buf = NULL;
		return false;
	}
	if (memcmp(this->taff_buf+sizeof(this->taff_desc->type), &(this->taff_desc->version), sizeof(this->taff_desc->version))) {
		// wrong version
		free(this->taff_buf);
		this->taff_buf = NULL;
		return false;
	}
	this->correct_version = true;

	// compare the modification time of the taff and external file
	if (this->external_filename!="") {
        struct stat statbuf1;
        struct stat statbuf2;
        if (stat(this->taff_filename.c_str(), &statbuf1)!=0) {
    		free(this->taff_buf);
    		this->taff_buf = NULL;
    		return false;
    	}
        if (stat(this->external_filename.c_str(), &statbuf2)==0) {
			if (statbuf2.st_mtime <= time(NULL)) {
				// ok, external file created in the past
				if (statbuf2.st_mtime >= statbuf1.st_mtime) {
					// external file has been modified, therefore the taff file maybe not up-to-date
					free(this->taff_buf);
					this->taff_buf = NULL;
					return false;
				}
			}
        }
	}

	// all right
	this->taff_buf_size = ritems;
	getFirstTag();
	this->loaded = true;
	return true;
}

bool MMSTaffFile::isLoaded() {
	return this->loaded;
}

bool MMSTaffFile::checkVersion() {
	return this->correct_version;
}

void MMSTaffFile::setExternal(string external_filename, MMSTAFF_EXTERNAL_TYPE external_type) {
	this->external_filename = external_filename;
	this->external_type = external_type;
}

void MMSTaffFile::setTrace(bool trace) {
	this->trace = trace;
}

void MMSTaffFile::setPrintWarnings(bool print_warnings) {
	this->print_warnings = print_warnings;
}

void MMSTaffFile::setDestinationPixelFormat(MMSTAFF_PF pixelformat, bool premultiplied) {
	this->destination_pixelformat = pixelformat;
	this->destination_premultiplied = premultiplied;
}

void MMSTaffFile::setMirrorEffect(int size) {
	this->mirror_size = size;
}

int MMSTaffFile::getFirstTag() {
	this->taff_buf_pos = sizeof(this->taff_desc->type) + sizeof(this->taff_desc->version);
	this->current_tag = -1;
	this->current_tag_pos = 0;

	if (this->taff_buf[this->taff_buf_pos] == MMSTAFF_TAGTABLE_TYPE_TAG) {
		bool eof;
		return getNextTag(eof);
	}

	return this->current_tag;
}

int MMSTaffFile::getNextTag(bool &eof) {
	/* searching for next tag */
	eof = false;
	while (this->taff_buf_pos < this->taff_buf_size) {
		switch (this->taff_buf[this->taff_buf_pos]) {
		case MMSTAFF_TAGTABLE_TYPE_TAG:
			this->current_tag = this->taff_buf[this->taff_buf_pos+1];
			this->current_tag_pos = this->taff_buf_pos;
			this->taff_buf_pos+=2;
			return this->current_tag;
		case MMSTAFF_TAGTABLE_TYPE_ATTR: {
				this->taff_buf_pos+=2;
				int len;

				/* check if name of attribute is stored instead of id */
				if (this->taff_buf[this->taff_buf_pos-1] == MMSTAFF_ATTR_WITHOUT_ID) {
					len = MMSTAFF_INT32_FROM_UCHAR_STREAM(&this->taff_buf[this->taff_buf_pos]);
					this->taff_buf_pos+=sizeof(int);
					this->taff_buf_pos+=len;
				}

				/* get the length of the value */
				len = (int)this->taff_buf[this->taff_buf_pos];
				this->taff_buf_pos++;
				if (len >= 0xff) {
					len = MMSTAFF_INT32_FROM_UCHAR_STREAM(&this->taff_buf[this->taff_buf_pos]);
					this->taff_buf_pos+=sizeof(int);
				}
				this->taff_buf_pos+=len;
			}
			break;
		case MMSTAFF_TAGTABLE_TYPE_CLOSETAG:
			this->current_tag = -1;
			this->taff_buf_pos+=2;
			eof = false;
			return this->current_tag;
		default:
			this->current_tag = -1;
			this->current_tag_pos = 0;
			eof = true;
			return this->current_tag;
		}
	}
	this->current_tag = -1;
	this->current_tag_pos = 0;
	eof = true;
	return this->current_tag;
}

int MMSTaffFile::getCurrentTag() {
	return this->current_tag;
}

MMSTaffFile *MMSTaffFile::copyCurrentTag() {
	MMSTaffFile *mytafff = NULL;
	int tag_cnt, closetag_cnt;

	if (!this->current_tag_pos)
		return NULL;

	/* save buffer positions */
	int	saved_taff_buf_pos = this->taff_buf_pos;
	int	saved_current_tag = this->current_tag;
	int	saved_current_tag_pos = this->current_tag_pos;

	/* go to the position after the current tag */
	this->taff_buf_pos = this->current_tag_pos;

	/* searching the close tag of this tag */
	tag_cnt = 0;
	closetag_cnt = 0;
	do {
		bool eof;
		if (getNextTag(eof) < 0) {
			if (eof) break;
			closetag_cnt++;
		}
		else
			tag_cnt++;
	} while (tag_cnt > closetag_cnt);

	/* all right? */
	if (tag_cnt == closetag_cnt) {
		/* yes, allocate memory and copy buffer */
		mytafff = new MMSTaffFile("", this->taff_desc, "", this->external_type,
								  this->ignore_blank_values, this->trace, false);
		if (mytafff) {
			int len = this->taff_buf_pos - saved_current_tag_pos;
			mytafff->taff_buf_size = sizeof(this->taff_desc->type) + sizeof(this->taff_desc->version) + len;
			mytafff->taff_buf = (unsigned char *)malloc(mytafff->taff_buf_size);
			if (mytafff->taff_buf) {
				/* copy & init */
				memcpy(mytafff->taff_buf, this->taff_buf, sizeof(this->taff_desc->type) + sizeof(this->taff_desc->version));
				memcpy(&(mytafff->taff_buf[sizeof(this->taff_desc->type) + sizeof(this->taff_desc->version)]),
					   &(this->taff_buf[saved_current_tag_pos]), len);
				mytafff->getFirstTag();
				mytafff->loaded = true;
				mytafff->correct_version = true;
			}
			else {
				/* out of memory */
				delete mytafff;
				mytafff = NULL;
			}
		}
	}

	/* restore the old buffer positions */
	this->taff_buf_pos = saved_taff_buf_pos;
	this->current_tag = saved_current_tag;
	this->current_tag_pos = saved_current_tag_pos;

	return mytafff;
}


int MMSTaffFile::getFirstAttribute(char **value_str, int *value_int, char **name) {
	if (!this->current_tag_pos)
		return -1;

	/* go to the position after the current tag */
	this->taff_buf_pos = this->current_tag_pos;
	this->taff_buf_pos+=2;

	/* get the attribute */
	if (this->taff_buf[this->taff_buf_pos] == MMSTAFF_TAGTABLE_TYPE_ATTR)
		return getNextAttribute(value_str, value_int, name);

	return -1;
}

int MMSTaffFile::getNextAttribute(char **value_str, int *value_int, char **name) {
	/* searching for next attribute */
	do {
		switch (this->taff_buf[this->taff_buf_pos]) {
		case MMSTAFF_TAGTABLE_TYPE_ATTR: {
				int attrid = (int)this->taff_buf[this->taff_buf_pos+1];
			    int len;
				this->taff_buf_pos+=2;

				/* check if name of attribute is stored instead of id */
				if (attrid == MMSTAFF_ATTR_WITHOUT_ID) {
					len = MMSTAFF_INT32_FROM_UCHAR_STREAM(&this->taff_buf[this->taff_buf_pos]);
					this->taff_buf_pos+=sizeof(int);
					if (name)
						*name = (char*)&this->taff_buf[this->taff_buf_pos];
					this->taff_buf_pos+=len;
				}
				else
					if (name) *name=NULL;

				/* get the length of the value */
				len = (int)this->taff_buf[this->taff_buf_pos];
				this->taff_buf_pos++;
				if (len >= 0xff) {
					len = MMSTAFF_INT32_FROM_UCHAR_STREAM(&this->taff_buf[this->taff_buf_pos]);
					this->taff_buf_pos+=sizeof(int);
				}

				/* check the type of value and set the return values */
				if (attrid != MMSTAFF_ATTR_WITHOUT_ID) {
					TAFF_ATTRDESC *attr = this->taff_desc->tagtable[current_tag].attr;
					switch (attr[attrid].type) {
					case TAFF_ATTRTYPE_BOOL:
					case TAFF_ATTRTYPE_UCHAR:
					case TAFF_ATTRTYPE_UCHAR100:
					case TAFF_ATTRTYPE_INT:
					case TAFF_ATTRTYPE_STATE:
						*value_str = NULL;
						*value_int = MMSTAFF_INT32_FROM_UCHAR_STREAM(&this->taff_buf[this->taff_buf_pos]);
						break;
					default:
						*value_str = (char*)&this->taff_buf[this->taff_buf_pos];
						break;
					}
				}
				else
				if (name) {
					*value_str = (char*)&this->taff_buf[this->taff_buf_pos];
				}
				this->taff_buf_pos+=len;

				if (!((attrid == MMSTAFF_ATTR_WITHOUT_ID)&&(!name)))
					/* return attribute ID */
					return attrid;

				/* attribute has no ID and name is not set, go to the next attribute */
				break;
			}
			break;
		default:
			return -1;
		}
	} while (this->taff_buf_pos < this->taff_buf_size);
	return -1;
}

bool MMSTaffFile::getAttribute(int id, char **value_str, int *value_int) {
	char *attr_name;
	int attrid = getFirstAttribute(value_str, value_int, &attr_name);
	while (attrid >= 0) {
		if (attrid == id)
			return true;
		attrid = getNextAttribute(value_str, value_int, &attr_name);
	}
	return false;
}

char *MMSTaffFile::getAttributeString(int id) {
	char *value_str = NULL;
	int  value_int;
	if (getAttribute(id, &value_str, &value_int))
		if (value_str)
			return value_str;
	return NULL;
}


TAFF_ATTRDESC MMSTAFF_IMAGE_RAWIMAGE_ATTR_I[]	= MMSTAFF_IMAGE_RAWIMAGE_ATTR_INIT;

TAFF_TAGTABLE mmstaff_image_taff_tagtable[] = {
	{	"rawimage",		NULL, 	NULL,			MMSTAFF_IMAGE_RAWIMAGE_ATTR_I	},
	{	NULL, 			NULL, 	NULL,			NULL							}
};

TAFF_DESCRIPTION mmstaff_image_taff_description = { "mmstaff_image", 1, mmstaff_image_taff_tagtable };


