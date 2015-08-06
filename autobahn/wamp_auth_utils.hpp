///////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) Tavendo GmbH
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
//
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef WAMP_AUTH_UTILS_HPP
#define WAMP_AUTH_UTILS_HPP

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exception>

//
// Execpetion thrown when something gets wrong
// creating the derived auth key.....
//
class derived_key_error : public std::exception
{
  virtual const char* what() const throw()
  {
    return "Error occured when calulcate a derived key";
  }
};


//////////////////////////////////////////////////////
// - using openssl crypto lib
// see openssl at : https://www.openssl.org
//////////////////////////////////////////////////////


#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>

#include <stdint.h>


/*!
 * base64 encoding
 *
 * \param data The data to be encoded
 * \return A encoded string
 */
inline std::string base_64_encode(const std::string & data )
{
    BIO *bio, *b64;
    BUF_MEM *pBuf;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    BIO_write(bio, (const unsigned char *) data.c_str(), data.size());
    (void)BIO_flush(bio);
    
    BIO_get_mem_ptr(bio, &pBuf);
    (void)BIO_set_close(bio, BIO_NOCLOSE);
    
    std::string str_out;
    str_out.assign( pBuf->data, pBuf->length );

    BIO_free_all(bio);

    return str_out;
}


/*!
 * create a derived key from a password/secret
 *
 * \param  passwd A secret string to make a derived key for
 * \param  salt A random salt added to the key
 * \param  iterations A number of intertions used to create the derived key
 * \param  keylen The length of the derived key returned.
 * \return a PBKDF2-sha256 derived key
 */
inline std::string derive_key(
        const std::string & passwd,
        const std::string & salt,
        int iterations,
        int keylen
        )
{

    int passwdLen = passwd.size();
    const char * pwd = passwd.c_str();

    int saltLen = salt.size();
    unsigned char * salt_value = (unsigned char * ) salt.c_str();

    std::string str_out;
    str_out.resize( keylen );


    unsigned char * out = (unsigned char *) str_out.c_str();


    int result = PKCS5_PBKDF2_HMAC(
            pwd, passwdLen,
            salt_value, saltLen,
            iterations,
            EVP_sha256(),
            keylen, out);

    if ( result != 0 )
    {
        return base_64_encode( str_out );
    }
    else
    {
        throw derived_key_error();
    }
}


/*!
 * make a keyed-hash from a key using the HMAC-sha256 and a challenge
 *
 * \param key The key to make a digest for
 * \param challenge Some data mixin - identify the specific digest
 * \return a base64 encoded digest
 */
inline std::string compute_wcs(
        const std::string & key,
        const std::string & challenge )
{

    unsigned int len = 32;
    unsigned char hash[32];

    HMAC_CTX hmac;
    HMAC_CTX_init(&hmac);
    HMAC_Init_ex(&hmac, key.data(), key.length(), EVP_sha256(), NULL);
    HMAC_Update(&hmac, ( unsigned char* ) challenge.data(), challenge.length());
    HMAC_Final(&hmac, hash, &len);
    HMAC_CTX_cleanup(&hmac);


    std::string str_out;
    str_out.assign( ( char * ) &hash , 32 );

    return base_64_encode( str_out );
}



/*!
 *  Generates a new random secret for use with WAMP-CRA.
 *  The secret generated is a random character sequence drawn from
 *
 *  - upper and lower case latin letters
 *  - digits
 *  -
 *
 * \param length The length of the secret to generate.
 * \param challenge Some data mixin - identify the specific digest
 * \return The generated secret. The length of the generated is ``length`` octets.
 */
inline std::string generate_wcs(int length=14){

    //
    // The characters from which to generate the secret.
    //
    static const char WCS_SECRET_CHARSET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    std::string s;
    for (int i = 0; i < length; ++i) {
        s.push_back( WCS_SECRET_CHARSET[ rand() % (sizeof(WCS_SECRET_CHARSET) - 1) ] );
    }

    return s;
}


#endif //WAMP_AUTH_UTILS_HPP
