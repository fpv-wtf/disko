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

#ifndef DATASOURCE_H_
#define DATASOURCE_H_

using namespace std;

#include <string>

/**
 * @file datasource.h
 *
 * Header file for DataSource class.
 *
 * @ingroup mmstools
 */

/**
 * Configures database settings.
 *
 * This class defines common settings for databases.
 * Be aware that not all databases need user/password or ip connection settings, so they may be ignored.
 */
class DataSource {

    private:
    	string       dbms;		/**< Defines the database management system to use ("SQLITE3", "FREETDS" or "MYSQL"). */
    	string       address;	/**< Defines the database ip address. */
        unsigned int port;		/**< Defines the database port. */
        string       dbName;	/**< Defines the database name. */
        string       user;		/**< Defines the username to access the database. */
        string       password;	/**< Defines the password to access the database. */

    public:
        /**
         * Constructor for DataSource object.
         *
         * @note If dbms is an empty string, the default database
         * management system (SQLITE3) will be used.
         *
         * @param	dbms		database management system
         * 						(currently either "SQLITE3", "FREETDS" or "MYSQL")
         * @param	dbName		database name
         * @param	address		database ip address
         * @param	port		database port
         * @param	user		username to access database
         * @param	password	password to access database
         */
    	DataSource(const string dbms,
    			   const string dbName,
    			   const string address    = "",
    			   const unsigned int port = 0,
    			   const string user       = "",
    			   const string password   = "");

    	/**
    	 * Copy-constructor for DataSource object.
    	 *
    	 * @param	d			DataSource object to copy
    	 */
    	DataSource(const DataSource& d);

    	/**
    	 * Destructor for DataSource object.
    	 */
    	~DataSource();

    	/**
    	 * Sets database management system.
    	 *
    	 * @note If an empty string is provided, the default system
    	 * (SQLITE3) will be used.
    	 *
    	 * @param	dbms		database management system
    	 * 						(currently either "SQLITE3", "FREETDS" or "MYSQL"
    	 *
    	 * @see getDBMS()
    	 */
        void setDBMS(const string dbms);

        /**
         * Gets database management system.
         *
         * @note If it returns an empty string, sqlite3 will be used.
         *
         * @returns	Either "", "SQLITE3", "FREETDS" or "MYSQL"
         *
         * @see setDBMS()
         */
        const string getDBMS();

        /**
         * Sets the ip address for the database connection.
         *
         * @param	address		database ip address
         *
         * @see getAddress()
         * @see setPort()
         * @see getPort()
         */
        void setAddress(const string address);

        /**
         * Gets the ip address for the database connection.
         *
         * @returns	ip address of database connection
         *
         * @see setAddress()
         * @see setPort()
         * @see getPort()
         */
        const string getAddress();

        /**
         * Sets the port for the database connection.
         *
         * @param	port	database port
         *
         * @see getPort()
         * @see setAddress()
         * @see getAddress()
         */
        void setPort(const unsigned int port);

        /**
         * Gets the port for the database connection.
         *
         * @returns database connection port
         *
         * @see setPort()
         * @see setAddress()
         * @see getAddress()
         */
        const unsigned int getPort();

        /**
         * Sets the database name.
         *
         * @param	dbName	database name
         *
         * @see getDatabaseName()
         */
        void setDatabaseName(const string dbName);

        /**
         * Gets the database name.
         *
         * @returns	database name
         *
         * @see setDatabaseName()
         */
        const string getDatabaseName();

        /**
         * Sets the user for database access.
         *
         * @param	user	user for database access
         *
         * @see getUser()
         * @see setPassword()
         * @see getPassword()
         */
        void setUser(const string user);

        /**
         * Gets the user for database access.
         *
         * @returns	user for database access
         *
         * @see setUser()
         * @see setPassword()
         * @see getPassword()
         */
        const string getUser();

        /**
         * Sets the password for database access.
         *
         * @param	password	password for database access
         *
         * @see getPassword()
         * @see setUser()
         * @see getUser()
         */
        void setPassword(const string password);

        /**
         * Gets the password for database access.
         *
         * @returns password for database access
         *
         * @see setPassword()
         * @see setUser()
         * @see getUser()
         */
        const string getPassword();
};

#endif /*DATASOURCE_H_*/
