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

#ifndef MMSFILESEARCH_H_
#define MMSFILESEARCH_H_

/**
 * @file mmsfilesearch.h
 *
 * Header file for MMSFileSearch class.
 *
 * @ingroup mmstools
 */

#include "mmstools/mmserror.h"

#include <list>
#include <vector>

extern "C" {
#include <sys/types.h>
#include <dirent.h>
}

MMS_CREATEERROR(MMSFileSearchError);

#define MMSFILESEARCH_DEEPESTDIRENTRY "<DEEPESTDIRENTRY>;"
#define MMSFILESEARCH_DEEPESTDIRENTRY_OF_FILE "<DEEPESTDIRENTRYOFFILE>;"

/**
 * Structure containing results of file searches.
 */
typedef struct _mmsfile_entry {
	bool   isdir;		/**< Entry is a directory.                                  */
	string name;		/**< Name of file entry.                                    */
	string basename;	/**< Basename of file entry (filename excluding directory). */
	string path;		/**< Path of file entry (directory excluding filename).     */
} MMSFILE_ENTRY;

/**
 * List of file search result entries.
 */
typedef list<MMSFILE_ENTRY *> MMSFILEENTRY_LIST;

/**
 * Options to configure file search.
 */
typedef enum {
	MMSFILESEARCH_NONE = 0,         	/**< Use default options. */
	MMSFILESEARCH_DEEPESTDIR,       	/**<  */
	MMSFILESEARCH_DEEPESTDIR_OF_FILE	/**<  */
} MMSFILESEARCH_OPTION;

/**
 * Helper class to search for files or directories.
 *
 * This class searches for given files or directories providing
 * support for recursive search, case (in)sensitivity and masking.
 */
class MMSFileSearch {
	public:
		/**
		 * Constructor.
		 *
		 * @param directory			Root directory to start the search.
		 * @param mask				Filename mask (i.e. "foo*"). You may specify multiple masks
		 * 							separated by colons (i.e. "foo*;*bar").
		 * @param recursive			Search recursively through subdirectories.
		 * @param caseinsensitive	If true search case insensitive.
		 * @param getdirs			Add directories in search results.
		 */
		MMSFileSearch(string directory, string mask, bool recursive=true, bool caseinsensitive=false, bool getdirs = false);

		/**
		 * Set #recursive parameter, which enables recursive searching through subdirectories.
		 *
		 * @param recursive		If set to true, recursive searching is enabled.
		 */
		void setRecursive(bool recursive);

		/**
		 * Set root directory (#directory) for searching.
		 *
		 * @param directory		This is the top-level directory for the executed search.
		 */
		void setDirectory(string directory);

		/**
		 * Set filename mask to search for.
		 *
		 * @param mask	Filename mask to search for (i.e. foo*.*).
		 *
		 * TODO: This method should be renamed to setMask().
		 */
		void setString(string mask);

		/**
		 * Configures the search to match results using case sensitivity.
		 *
		 * @param caseinsensitive	If true, results are matched without case sensitivity.
		 */
		void setCaseInsensitive(bool caseinsensitive);

		/**
		 * Start searching for files.
		 *
		 * @return	List of file entries matching your search criteria.
		 */
		MMSFILEENTRY_LIST execute();

	private:
		bool recursive;					/**< Search recursively? 					*/
		bool caseinsensitive;			/**< Search without using case sensitivity? */
		bool getdirs;					/**< Add directories to search results?		*/
		string directory;				/**< Top-level directory to search for.		*/
		string mask;					/**< Filename mask to search for.			*/
		vector<string>  singlemask;		/**< Separated filename masks if #mask containes more than one mask. */
		DIR *dirhandle;					/**< Internal directory handle used when searching. */

		/**
		 * Does the filename match the given #mask?
		 *
		 * @param entry		Filename to check.
		 *
		 * @return True if filename matches the given #mask.
		 */
		bool match(char *entry);

		/**
		 * Helper method to scan a directory for files.
		 *
		 * @param result		Pointer to file entry list.
		 * @param dirhandle		Directory handle to use for search.
		 * @param cwd			Current working directory.
		 */
		void scanDir(list<MMSFILE_ENTRY *> *result,DIR *dirhandle, string cwd);

		/**
		 * Split a filemask in multiple submask.
		 *
		 * You may specify multiple masks separated by colons.
		 * This method splits the #mask into a vector of masks (#singlemask).
		 */
		void separateMask();

		/**
		 * Options to use for searching.
		 */
		MMSFILESEARCH_OPTION option;
};
#endif /*MMSFILESEARCH_H_*/
