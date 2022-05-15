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

#ifndef MMSERROR_H_
#define MMSERROR_H_

/**
 * @file mmserror.h
 *
 * Header file for MMSError class.
 *
 * @ingroup mmstools
 */

using namespace std;

#include <string>

/**
 * Base class for exception handling in disko.
 *
 * Derive your exception class from this one to extend exception handling.
 */
class MMSError {
    public:
		/**
		 * Constructor.
		 *
		 * @param code		error code (currently not used)
		 * @param message	error message which describes the exception more detailed
		 */
        MMSError(int code, string message) : message(message), code(code) {};

        /**
         * Destructor.
         */
        ~MMSError() {};

        /**
         * Retrieve the error message saved in #message.
         *
         * @return String containing the error messages of this exception object.
         */
        string getMessage();

        /**
         * Retrieve the error code saved in #code.
         *
         * @return Error code of this exception object.
         */
        int getCode();

    private:
        string 	message;	/**< Explanation of the error that caused this exception object to be thrown. */
        int 	code;		/**< Error code (currently not used). */
};

/**
 * Macro which defines a new error type.
 *
 * You may use this shorthand define to derive your own exception class,
 * i.e. MMS_CREATEERROR(FooClass).
 */
#define MMS_CREATEERROR(error)  class error : public MMSError { public: error(int code, string message) : MMSError(code,message) {};}

#endif /*MMSERROR_H_*/
