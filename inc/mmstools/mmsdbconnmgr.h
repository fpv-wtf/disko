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

#ifndef MMSDBCONNMGR_H_
#define MMSDBCONNMGR_H_

#include "mmstools/interfaces/immsdb.h"

/**
 * @file mmsdbconnmgr.h
 *
 * Header file for MMSDBConnMgr class.
 *
 * @ingroup mmstools
 */

#define DBMS_SQLITE3	"SQLITE3"	/**< Use sqlite3 as database management system. */
#define DBMS_FREETDS	"FREETDS"	/**< Use freetds as database management system. */
#define DBMS_MYSQL 		"MYSQL"		/**< Use mysql as database management system.   */

/**
 * Database connection manager class.
 *
 * This is the base class to connect to a configured database.
 */
class MMSDBConnMgr {
	public:
		/**
		 * Constructor for database connection managers.
		 *
		 * @param	datasource	DataSource object, containing database settings
		 */
		MMSDBConnMgr(DataSource *datasource);

		/**
		 * Retrieve an interface to a database connection.
		 *
		 * @returns	database connection interface
		 */
		IMMSDB *getConnection();

	private:
		DataSource *datasource;		/**< Contains database settings. */
};

#endif /*MMSDBCONNMGR_H_*/
