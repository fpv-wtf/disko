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

#ifdef __HAVE_MMSCRYPT__
#ifndef MMSCRYPT_H_
#define MMSCRYPT_H_

#include <openssl/evp.h>
#include "mmstools/mmserror.h"

/**
 * @file mmscrypt.h
 *
 * Header file for MMSCrypt class.
 *
 * @ingroup mmstools
 */

MMS_CREATEERROR(MMSCryptError);

#define MMSCRYPT_DEFAULT_KEY_FILENAME "./.key"

/**
 * Basic crypthographic methods based on OpenSSL functionality.
 *
 * This class simply provides helper methods for de- and encrypting buffers.
 *
 * @note To use MMSCrypt you have to build disko with 'enable_crypt=y'
 *       which depends on having OpenSSL development libraries installed.
 */
class MMSCrypt {
	public:
		/**
		 * Constructor for accessing cryptographic functions.
		 *
		 * @param	keyfile		filename of user key
		 *
		 * @exception MMSCryptError		keyfile could not be opened or created
		 */
		MMSCrypt(string keyfile = MMSCRYPT_DEFAULT_KEY_FILENAME);

		/**
		 * Destructor of MMSCrypt class.
		 *
		 * Frees all used resources.
		 */
		~MMSCrypt();

		/**
		 * Encrypts a given buffer.
		 *
		 * @param	in			buffer to encrypt
		 * @param	size		size of buffer (in) to encrypt
		 * @param	useMMSCtx	if set to true private disko context will be used, otherwise use user key
		 *
		 * @returns	Encrypted buffer if the call was successful.
		 *
		 * @exception MMSCryptError		An error occured while encrypting
		 * 								(call MMSError::getMessage() for a detailed error message.
		 *
		 * @see decrypt()
		 */
		unsigned char* encrypt(unsigned char *in, unsigned int size = 0, bool useMMSCtx = false);

		/**
		 * Decrypts a given buffer.
		 *
		 * @param	in			buffer to decrypt
		 * @param	size		size of buffer to decrypt
		 * @param	useMMSCtx	if set to true private disko context will be used, otherwise use user key
		 *
		 * @return	Decrypted buffer if the call was successful.
		 *
		 * @exception MMSCryptError		Not enough memory for decrypting the message.
		 *
		 * @see encrypt()
		 */
		unsigned char* decrypt(unsigned char *in, unsigned int size = 0, bool useMMSCtx = false);

	private:
        EVP_CIPHER_CTX 	mmsCtx,		/**< private disko cipher context */
        				userCtx;	/**< user cipher context */

        /**
         * Creates an SSL key that will be saved in the given file.
         *
         * @param	keyfile		save encrypted key to this file
         *
         * @note	The memory for the returned key has to be freed.
         *
         * @return  Unencrypted key (NULL if error occured).
         *
         * @see		getUserKey()
         */
        unsigned char* createUserKey(string keyfile);

        /**
         * Returns an SSL key that was stored in the given file.
         * If the file doesn't exist, a new key will be generated
         * and saved.
         *
         * @param	keyfile		read encrypted key from this file
         *
         * @note	The memory for the returned key has to be freed.
         *
         * @return  Unencrypted key (NULL if error occured).
         *
         * @see		createUserKey()
         *
         * @exception MMSCryptError 	File could not be opened.
         */
        unsigned char* getUserKey(string keyfile);
};

#endif /* MMSCRYPT_H_ */
#endif /* __HAVE_MMSCRYPT__ */
