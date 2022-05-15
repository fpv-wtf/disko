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

#ifdef __ENABLE_FREETDS__

#include "mmstools/mmsdbfreetds.h"
#include "mmstools/mmstools.h"
#include <stdlib.h>
#include <string.h>

/**
 * @file  mmsdbfreetds.cpp
 *
 * @brief Source file for freetds database functions.
 *
 * @ingroup mmstools
 */

MMSDBFreeTDS::MMSDBFreeTDS(DataSource *_datasource) :
	dbhandle(NULL),
	henv(SQL_NULL_HENV),
	IMMSDB(_datasource) {

	if(!this->datasource) {
		throw MMSError(0, "Cannot instantiate MMSDBFreeTDS without datasource");
	}
}

MMSDBFreeTDS::~MMSDBFreeTDS() {
	this->disconnect();
}

/**
 * @brief Gets and creates a databases error message.
 *
 * @param 	rc 		return code of last called sql function
 * @param 	htype 	type of handle for which diagnostics should be run
 * @param 	handle 	database connection handle
 *
 * @return last error message.
 */
string errmsg(SQLRETURN rc, SQLSMALLINT htype, SQLHANDLE handle) {
	SQLCHAR szSqlState[6],szErrorMsg[SQL_MAX_MESSAGE_LENGTH];
	SQLINTEGER pfNativeError;
	SQLSMALLINT pcbErrorMsg;

	rc = SQLGetDiagRec(htype, handle,1,
			(SQLCHAR *)&szSqlState,
			(SQLINTEGER *)&pfNativeError,
			(SQLCHAR *)&szErrorMsg,
			SQL_MAX_MESSAGE_LENGTH-1,
			(SQLSMALLINT *)&pcbErrorMsg);

	string msg = string((char*)szSqlState) + "|" + string((char*)szErrorMsg);
	DEBUGMSG("MMSFREETDS", msg);

	return msg;
}

void MMSDBFreeTDS::connect() {
	int rc;
	char connection_string[256] = "";

	if(this->connected) {
		DEBUGMSG("MMSFREETDS", "already connected");
		return;
	}

	if((rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &this->henv)) != SQL_SUCCESS) {
		throw MMSError(rc, "SQLAllocHandle() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");
	}

	if((rc = SQLSetEnvAttr(this->henv,SQL_ATTR_ODBC_VERSION,(SQLPOINTER)SQL_OV_ODBC3,0)) != SQL_SUCCESS) {
		SQLFreeHandle(SQL_HANDLE_ENV, this->henv);
		throw MMSError(rc, "SQLSetEnvAttr() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");
	}

	if((rc = SQLAllocHandle(SQL_HANDLE_DBC, this->henv, this->dbhandle)) != SQL_SUCCESS) {
		SQLFreeHandle(SQL_HANDLE_ENV, this->henv);
		throw MMSError(rc, "SQLAllocHandle() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");
	}

	snprintf(connection_string,sizeof(connection_string),
			"Server=%s;UID=%s;PWD=%s;Database=%s;Port=%d;TDS_Version=8.0;",
			datasource->getAddress().c_str(),
			datasource->getUser().c_str(),
			datasource->getPassword().c_str(),
			datasource->getDatabaseName().c_str(),
			datasource->getPort());

	DEBUGMSG("MMSFREETDS", "try to connect to database: %s", connection_string);

	if((rc = SQLDriverConnect(*(this->dbhandle), NULL, (SQLCHAR *)connection_string, SQL_NTS,
							(SQLCHAR *)connection_string,sizeof(connection_string),NULL, SQL_DRIVER_COMPLETE)) != SQL_SUCCESS) {
		SQLFreeHandle(SQL_HANDLE_DBC, *(this->dbhandle));
		this->dbhandle = NULL;
		SQLFreeHandle(SQL_HANDLE_ENV, this->henv);
		throw MMSError(rc, "SQLDriverConnect() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");
	}

	this->connected = true;

	DEBUGMSG("MMSFREETDS", "connect to database successful");
}

void MMSDBFreeTDS::disconnect() {
	if(this->connected) {
		if(this->dbhandle) {
			SQLDisconnect(this->dbhandle);
			SQLFreeConnect(this->dbhandle);
			SQLFreeHandle(SQL_HANDLE_DBC, this->dbhandle);
		}

		if(this->henv != SQL_NULL_HENV) {
			SQLFreeHandle(SQL_HANDLE_ENV, this->henv);
		}

		this->connected = false;
	}
}

int MMSDBFreeTDS::query(string statement, MMSRecordSet *rs) {
	int 			rc		= 0;
	SQLHSTMT		hstmt	= SQL_NULL_HSTMT;
	SQLSMALLINT 	columns = 0;
	MMSRecordSet 	*myrs	= (MMSRecordSet *)rs;

	myrs->reset();
	myrs->setRecordNum(0);

	query(statement, hstmt, false);

	int ret, rows=0;

	while (SQL_SUCCEEDED(ret = SQLFetch(hstmt))) {
		SQLUSMALLINT i;

		/* add a new row to resultset*/
		myrs->addRow();
		rows++;

		/* Loop through the columns */
		for (i = 1; i <= columns; i++) {
			SQLLEN indicator;
			char buf[512];
			SQLCHAR colname[32];
			SQLSMALLINT coltype;
			SQLSMALLINT colnamelen;
			SQLSMALLINT nullable;
			SQLSMALLINT scale;
			SQLULEN collen[1024];

			SQLDescribeCol (hstmt, i, colname, sizeof (colname),
					&colnamelen, &coltype, &collen[i], &scale, &nullable);

			/* retrieve column data as a string */
			ret = SQLGetData(hstmt, i, SQL_C_CHAR, buf, sizeof(buf), &indicator);
			if (SQL_SUCCEEDED(ret)) {
				/* Handle null columns */
				if (indicator == SQL_NULL_DATA) strcpy(buf, "NULL");

				(*myrs)[(char*)colname]=(char *)buf;
			}
		}
	}

	//rewind
	rs->setRecordNum(0);

	SQLCloseCursor(hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, hstmt);

	DEBUGMSG("MMSFREETDS", iToStr(rows) + " rows returned.");

	return rows;
}

int MMSDBFreeTDS::query(string statement) {
	SQLHSTMT hstmt = SQL_NULL_HSTMT;

	return query(statement, hstmt);
}

int MMSDBFreeTDS::query(string statement, SQLHSTMT &hstmt, bool finishStatement) {
	int rc = 0;
	SQLSMALLINT columns=0;

	// start the query
	if((rc = SQLAllocHandle(SQL_HANDLE_STMT, *this->dbhandle, &hstmt)) != SQL_SUCCESS) {
		throw MMSError(rc, "SQLAllocHandle() failed. [" + errmsg(rc,DIAG_TYPE_ENV, this->henv) +"]");
	}

	DEBUGMSG("MMSFREETDS", statement);

	// start a stored procedure
	if((rc = SQLExecDirect(hstmt, (SQLCHAR *) statement.c_str(), SQL_NTS)) != SQL_SUCCESS) {
		throw MMSError(rc, "Execution of query failed. [" + errmsg(rc, DIAG_TYPE_STMT, hstmt) +"]");
	}

	SQLNumResultCols(hstmt, &columns);

	if(finishStatement) {
		SQLCloseCursor(hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
	}

	return columns;
}

int MMSDBFreeTDS::executeSP(string spName, MMSDB_SP_ARGLIST argList, MMSRecordSet *rs) {
	string query;
	MMSDB_SP_ARGLIST::iterator it;

	//Build my query
	query="{" + string(FREETDS_SP_EXEC_CMD) + spName + " (";

	for(it = argList.begin();it!=argList.end();it++) {
		if(it!=argList.begin())
		query += ",";

		query += it->first + "=" + it->second;
	}

	query+=")}";

	return this->query(query, rs);
}

int MMSDBFreeTDS::executeSP(string spName, MMSDB_SP_ARGLIST argList) {
	string query;
	MMSDB_SP_ARGLIST::iterator it;

	//Build my query
	query="{" + string(FREETDS_SP_EXEC_CMD) + spName + " (";

	for(it = argList.begin();it!=argList.end();it++) {
		if(it!=argList.begin())
		query += ",";

		query += it->first + "=" + it->second;
	}

	query+=")}";

	return this->query(query);
}

#endif /*__ENABLE_FREETDS__*/
