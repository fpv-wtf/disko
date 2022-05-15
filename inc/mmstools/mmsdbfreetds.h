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

#ifndef MMSDBFREETDS_H_
#define MMSDBFREETDS_H_

#ifdef __ENABLE_FREETDS__

#include "mmstools/base.h"
#include "mmstools/mmserror.h"
#include "mmstools/mmsrecordset.h"
#include "mmstools/datasource.h"
#include "mmstools/interfaces/immsdb.h"

#include <sql.h>
#include <map>
#include <sqlext.h>

/**
 * @file  mmsdbfreetds.h
 *
 * @brief Header file for freetds database functions.
 *
 * @ingroup mmstools
 */

#define DIAG_TYPE_ENV   1
#define DIAG_TYPE_DBC   2
#define DIAG_TYPE_STMT  3

#define FREETDS_SP_EXEC_CMD		"CALL "

/**
 * Implementation of database access using FreeTDS (http://www.freetds.org).
 *
 * This allows usage of Microsoft SQL Server and Sybase database backends.
 * MMSDBFreeTDS implementes the IMMSDB interface, which should be used in
 * your application to provide simple database backend exchange by only
 * changing the configuration in your XML-configuration file.
 *
 * @see MMSDBMySQL
 * @see MMSDBSQLite
 */
class MMSDBFreeTDS : public IMMSDB {
    public:
		/**
		 * Constructor which only saves a reference to the given DataSource object.
		 *
		 *  @param	datasource	Database configuration to use.
		 *
		 *  @throws	MMSError	datasource was NULL
		 *
    	 * @see ~MMSDBFreeTDS()
		 */
    	MMSDBFreeTDS(DataSource *datasource);

    	/**
    	 * Destructor which disconnects from the database.
    	 *
    	 * @see MMSDBFreeTDS()
    	 * @see disconnect()
    	 */
        virtual ~MMSDBFreeTDS();

        /**
         * Opens connection to the database.
         *
         * The database settings have to be given to the constructor before.
         *
         * @throws MMSError		Connection to SQL database could not be established
         * 						(for detailed information use MMSError::getMessage())
         *
         * @see MMSDBFreeTDS()
         * @see disconnect()
         */
        void connect();

        /**
         * Close connection to database if a connection was established before.
         *
         * @see connect()
         */
        void disconnect();

        /**
         * Method not implemented since the FreeTDS implementation does not
         * support transactions.
         */
        void startTransaction() {};

        /**
         * Method not implemented since the FreeTDS implementation does not
         * support transactions.
         */
        void commitTransaction() {};

        /**
         * Method not implemented since the FreeTDS implementation does not
         * support transactions.
         */
        void rollbackTransaction() {};


        /**
         * This function executes the given database query and puts the results in MMSRecordSet.
         *
         * This method is used for select statements.
         *
         * @param 	statement 	buffer with database query
         * @param 	rs			recordset containing result of query
         *
         * @return Returns the number of affected rows.
         *
         * @throws MMSError		SQL query could not be executed
         * 						(for detailed information use MMSError::getMessage())
         *
         * @see MMSRecordSet
         */
        int query(string statement, MMSRecordSet *rs);

        /**
         * This function executes given database query.
         *
         * This method is used for insert, update and delete statements.
         *
         * @param 	statement 	buffer with database query
         *
         * @throws MMSError		SQL query could not be executed
         * 						(for detailed information use MMSError::getMessage())
         *
         * @return Returns the number of affected rows
         */
        int query(string statement);

        /**
         * This function executes given stored procedure and puts the results in MMSRecordSet.
         *
         * This method is used for insert, update and delete statements
         *
         * @param spName 	name of stored procedure
         * @param argList	arguments for stored procedure
         * @param rs		recordset containing result
         *
         * @return Returns the number of affected rows
         */
        int executeSP(string spName, MMSDB_SP_ARGLIST argList, MMSRecordSet *rs);

        /**
         * This function executes a stored procedure.
         *
         * This method is used for insert, update and delete statements
         *
         * @param spName 	name of stored procedure
         * @param argList	arguments for stored procedure
         *
         * @return Returns the number of affected rows
         */
        int executeSP(string spName, MMSDB_SP_ARGLIST argList);

        /**
         * This method is not supported by the FreeTDS implementation.
         *
         * @return Returns always 0.
         */
        int getLastInsertedID() { return 0; }

    private:
        SQLHDBC 	*dbhandle;		/**< Database handle retrieved in connect(). */
        SQLHENV 	henv;			/**< Database environment handle retrieved in connect(). */
   	 	DataSource  *datasource;	/**< Database configuration. */

        /**
         * Internal function to execute a given database query.
         *
         * This method is used by the public query() methods.
         *
         * @param 	statement 		buffer with database query
         * @param	hstmt			reference to SQL statement handle
         * @param	finishStatement	if false query cursor stays open i.e. to call SqlFetch() afterwards
         *
         * @throws MMSError		SQL query could not be executed
         * 						(for detailed information use MMSError::getMessage())
         *
         * @return Returns the number of affected rows
         */
        int query(string statement, SQLHSTMT &hstmt, bool finishStatement = true);
};

#endif /* __ENABLE_FREETDS__ */

#endif /*MMSDBFREETDS_H_*/
