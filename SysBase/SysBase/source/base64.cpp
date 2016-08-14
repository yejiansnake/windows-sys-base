/* /////////////////////////////////////////////////////////////////////////
 * File:        b64.c
 *
 * Purpose:     Implementation file for the b64 library
 *
 * Created:     18th October 2004
 * Updated:     18th November 2009
 *
 * Home:        http://synesis.com.au/software/
 *
 * Copyright (c) 2004-2009, Matthew Wilson and Synesis Software
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer. 
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name(s) of Matthew Wilson and Synesis Software nor the names of
 *   any contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * ////////////////////////////////////////////////////////////////////// */

/* /////////////////////////////////////////////////////////////////////////
 * Version information
 */

#ifndef B64_DOCUMENTATION_SKIP_SECTION
# define B64_VER_B64_H_B64_MAJOR    1
# define B64_VER_B64_H_B64_MINOR    6
# define B64_VER_B64_H_B64_REVISION 1
# define B64_VER_B64_H_B64_EDIT     31
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */

/** \def B64_VER_MAJOR
 * The major version number of b64
 */

/** \def B64_VER_MINOR
 * The minor version number of b64
 */

/** \def B64_VER_REVISION
 * The revision version number of b64
 */

/** \def B64_VER
 * The current composite version number of b64
 */

#ifndef B64_DOCUMENTATION_SKIP_SECTION
# define B64_VER_1_0_1          0x01000100
# define B64_VER_1_0_2          0x01000200
# define B64_VER_1_0_3          0x01000300
# define B64_VER_1_1_1          0x01010100
# define B64_VER_1_1_2          0x01010200
# define B64_VER_1_1_3          0x01010300
# define B64_VER_1_2_1          0x01020100
# define B64_VER_1_2_2          0x01020200
# define B64_VER_1_2_3          0x01020300
# define B64_VER_1_2_4          0x01020400
# define B64_VER_1_2_5          0x01020500
# define B64_VER_1_2_6          0x01020600
# define B64_VER_1_2_7          0x01020700
# define B64_VER_1_3_1          0x010301ff
# define B64_VER_1_3_2          0x010302ff
# define B64_VER_1_3_3          0x010303ff
# define B64_VER_1_3_4          0x010304ff
# define B64_VER_1_4_1_ALPHA_1  0x01040101
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */

#define B64_VER_MAJOR       1
#define B64_VER_MINOR       4
#define B64_VER_REVISION    1
#define B64_VER             0x01040101

//////////////////////////////////////////////////////////////////////////

/** \file b64.c Implementation file for the b64 library
 */

/* /////////////////////////////////////////////////////////////////////////
 * Version information
 */

#ifndef B64_DOCUMENTATION_SKIP_SECTION
# define B64_VER_C_B64_MAJOR    1
# define B64_VER_C_B64_MINOR    3
# define B64_VER_C_B64_REVISION 3
# define B64_VER_C_B64_EDIT     20
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#include "../SysBase_Interface.h"

#if defined(B64_VARIANT_TEST) && \
    !defined(B64_NO_USE_XCONTRACT)
# define B64_USING_XCONTRACT_
#endif /* B64_VARIANT_TEST && !B64_NO_USE_XCONTRACT */

#ifdef B64_USING_XCONTRACT_
# include <xcontract/xcontract.h>
#else /* ? B64_USING_XCONTRACT_ */
# include <assert.h>
#endif /* B64_USING_XCONTRACT_ */
#include <string.h>

/* /////////////////////////////////////////////////////////////////////////
 * Contract Enforcements
 */

#ifdef B64_USING_XCONTRACT_
# define B64_ENFORCE_PRECONDITION(expr, msg)        XCONTRACT_ENFORCE_PRECONDITION_PARAMETERS_2((expr), (msg))
# define B64_ENFORCE_ASSUMPTION(expr)               XCONTRACT_ENFORCE_ASSUMPTION_2((expr), "design invariant violated")
#else /* ? B64_USING_XCONTRACT_ */
# define B64_ENFORCE_PRECONDITION(expr, msg)        assert((expr))
# define B64_ENFORCE_ASSUMPTION(expr)               assert((expr))
#endif /* B64_USING_XCONTRACT_ */

/* /////////////////////////////////////////////////////////////////////////
 * Constants and definitions
 */

#ifndef B64_DOCUMENTATION_SKIP_SECTION
# define NUM_PLAIN_DATA_BYTES        (3)
# define NUM_ENCODED_DATA_BYTES      (4)
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Macros
 */

#ifndef NUM_ELEMENTS
# define NUM_ELEMENTS(x)        (sizeof(x) / sizeof(x[0]))
#endif /* !NUM_ELEMENTS */

/* /////////////////////////////////////////////////////////////////////////
 * Warnings
 */

#if defined(_MSC_VER) && \
    _MSC_VER < 1000
# pragma warning(disable : 4705)
#endif /* _MSC_VER < 1000 */

 /* /////////////////////////////////////////////////////////////////////////
 * Typedefs
 */

/** Return codes (from b64_encode2() / b64_decode2())
 */
enum B64_RC
{
        B64_RC_OK                   =   0   /*!< Operation was successful. */
    ,   B64_RC_INSUFFICIENT_BUFFER  =   1   /*!< The given translation buffer was not of sufficient size. */
    ,   B64_RC_TRUNCATED_INPUT      =   2   /*!< The input did not represent a fully formed stream of octet couplings. */
    ,   B64_RC_DATA_ERROR           =   3   /*!< Invalid data. */
#ifndef B64_DOCUMENTATION_SKIP_SECTION
    ,   B64_max_RC_value
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */
};

#ifndef __cplusplus
typedef enum B64_RC B64_RC;
#endif /* !__cplusplus */

/** Coding behaviour modification flags (for b64_encode2() / b64_decode2())
 */
enum B64_FLAGS
{
        B64_F_LINE_LEN_USE_PARAM    =   0x0000  /*!< Uses the lineLen parameter to b64_encode2(). Ignored by b64_decode2(). */
    ,   B64_F_LINE_LEN_INFINITE     =   0x0001  /*!< Ignores the lineLen parameter to b64_encode2(). Line length is infinite. Ignored by b64_decode2(). */
    ,   B64_F_LINE_LEN_64           =   0x0002  /*!< Ignores the lineLen parameter to b64_encode2(). Line length is 64. Ignored by b64_decode2(). */
    ,   B64_F_LINE_LEN_76           =   0x0003  /*!< Ignores the lineLen parameter to b64_encode2(). Line length is 76. Ignored by b64_decode2(). */
    ,   B64_F_LINE_LEN_MASK         =   0x000f  /*!< Mask for testing line length flags to b64_encode2(). Ignored by b64_encode2(). */
    ,   B64_F_STOP_ON_NOTHING       =   0x0000  /*!< Decoding ignores all invalid characters in the input data. Ignored by b64_encode2(). */
    ,   B64_F_STOP_ON_UNKNOWN_CHAR  =   0x0100  /*!< Causes decoding to break if any non-Base-64 [a-zA-Z0-9=+/], non-whitespace character is encountered. Ignored by b64_encode2(). */
    ,   B64_F_STOP_ON_UNEXPECTED_WS =   0x0200  /*!< Causes decoding to break if any unexpected whitespace is encountered. Ignored by b64_encode2(). */
    ,   B64_F_STOP_ON_BAD_CHAR      =   0x0300  /*!< Causes decoding to break if any non-Base-64 [a-zA-Z0-9=+/] character is encountered. Ignored by b64_encode2(). */
};

/* /////////////////////////////////////////////////////////////////////////
 * Functions
 */


/** Encodes a block of binary data into Base-64
 *
 * \param src Pointer to the block to be encoded. May not be NULL, except when
 *   \c dest is NULL, in which case it is ignored.
 * \param srcSize Length of block to be encoded
 * \param dest Pointer to the buffer into which the result is to be written. May
 *   be NULL, in which case the function returns the required length
 * \param destLen Length of the buffer into which the result is to be written. Must
 *   be at least as large as that indicated by the return value from
 *   \link b64::b64_encode b64_encode(NULL, srcSize, NULL, 0)\endlink.
 *
 * \return 0 if the size of the buffer was insufficient, or the length of the
 * converted buffer was longer than \c destLen
 *
 * \note The function returns the required length if \c dest is NULL
 *
 * \note The function returns the required length if \c dest is NULL. The returned size
 *   might be larger than the actual required size, but will never be smaller.
 *
 * \note Threading: The function is fully re-entrant.
 *
 * \see b64::encode()
 */
size_t b64_encode(
    void const* src
,   size_t      srcSize
,   char* dest
,   size_t      destLen
);

/** Encodes a block of binary data into Base-64
 *
 * \param src Pointer to the block to be encoded. May not be NULL, except when
 *   \c dest is NULL, in which case it is ignored.
 * \param srcSize Length of block to be encoded
 * \param dest Pointer to the buffer into which the result is to be written. May
 *   be NULL, in which case the function returns the required length
 * \param destLen Length of the buffer into which the result is to be written. Must
 *   be at least as large as that indicated by the return value from
 *   \link b64::b64_encode2 b64_encode2(NULL, srcSize, NULL, 0, flags, lineLen, rc)\endlink.
 * \param flags A combination of the B64_FLAGS enumeration, that moderate the
 *   behaviour of the function
 * \param lineLen If the flags parameter contains B64_F_LINE_LEN_USE_PARAM, then
 *   this parameter represents the length of the lines into which the encoded form is split,
 *   with a hard line break ('\\r\\n'). If this value is 0, then the line is not
 *   split. If it is <0, then the RFC-1113 recommended line length of 64 is used
 * \param rc The return code representing the status of the operation. May be NULL.
 *
 * \return 0 if the size of the buffer was insufficient, or the length of the
 *   converted buffer was longer than \c destLen
 *
 * \note The function returns the required length if \c dest is NULL. The returned size
 *   might be larger than the actual required size, but will never be smaller.
 *
 * \note Threading: The function is fully re-entrant.
 *
 * \see b64::encode()
 */
size_t b64_encode2(
    void const* src
,   size_t      srcSize
,   char* dest
,   size_t      destLen
,   unsigned    flags
,   int         lineLen /* = 0 */
,   B64_RC*     rc     /* = NULL */
);

/** Decodes a sequence of Base-64 into a block of binary data
 *
 * \param src Pointer to the Base-64 block to be decoded. May not be NULL, except when
 *   \c dest is NULL, in which case it is ignored. If \c dest is NULL, and \c src is
 *   <b>not</b> NULL, then the returned value is calculated exactly, otherwise a value
 *   is returned that is guaranteed to be large enough to hold the decoded block.
 *
 * \param srcLen Length of block to be encoded. Must be an integral of 4, the Base-64
 *   encoding quantum, otherwise the Base-64 block is assumed to be invalid
 * \param dest Pointer to the buffer into which the result is to be written. May
 *   be NULL, in which case the function returns the required length
 * \param destSize Length of the buffer into which the result is to be written. Must
 *   be at least as large as that indicated by the return value from
 *   \c b64_decode(src, srcSize, NULL, 0), even in the case where the encoded form
 *   contains a number of characters that will be ignored, resulting in a lower total
 *   length of converted form.
 *
 * \return 0 if the size of the buffer was insufficient, or the length of the
 *   converted buffer was longer than \c destSize
 *
 * \note The function returns the required length if \c dest is NULL. The returned size
 *   might be larger than the actual required size, but will never be smaller.
 *
 * \note \anchor anchor__4_characters The behaviour of both
 * \link b64::b64_encode2 b64_encode2()\endlink
 * and
 * \link b64::b64_decode2 b64_decode2()\endlink
 * are undefined if the line length is not a multiple of 4.
 *
 * \note Threading: The function is fully re-entrant.
 *
 * \see b64::decode()
 */
size_t b64_decode(
    char const*   src
,   size_t              srcLen
,   void*               dest
,   size_t              destSize
);

/** Decodes a sequence of Base-64 into a block of binary data
 *
 * \param src Pointer to the Base-64 block to be decoded. May not be NULL, except when
 * \c dest is NULL, in which case it is ignored. If \c dest is NULL, and \c src is
 * <b>not</b> NULL, then the returned value is calculated exactly, otherwise a value
 * is returned that is guaranteed to be large enough to hold the decoded block.
 *
 * \param srcLen Length of block to be encoded. Must be an integral of 4, the Base-64
 *   encoding quantum, otherwise the Base-64 block is assumed to be invalid
 * \param dest Pointer to the buffer into which the result is to be written. May
 *   be NULL, in which case the function returns the required length
 * \param destSize Length of the buffer into which the result is to be written. Must
 *   be at least as large as that indicated by the return value from
 *   \c b64_decode(src, srcSize, NULL, 0), even in the case where the encoded form
 *   contains a number of characters that will be ignored, resulting in a lower total
 *   length of converted form.
 * \param flags A combination of the B64_FLAGS enumeration, that moderate the
 *   behaviour of the function.
 * \param rc The return code representing the status of the operation. May be NULL.
 * \param badChar If the flags parameter does not contain B64_F_STOP_ON_NOTHING, this
 *   parameter specifies the address of a pointer that will be set to point to any
 *   character in the sequence that stops the parsing, as dictated by the flags
 *   parameter. May be NULL.
 *
 * \return 0 if the size of the buffer was insufficient, or the length of the
 * converted buffer was longer than \c destSize, or a bad character stopped parsing.
 *
 * \note The function returns the required length if \c dest is NULL. The returned size
 *   might be larger than the actual required size, but will never be smaller.
 *
 * \note The behaviour of both
 * \link b64::b64_encode2 b64_encode2()\endlink
 * and
 * \link b64::b64_decode2 b64_decode2()\endlink
 * are undefined if the line length is not a multiple of 4.
 *
 * \note Threading: The function is fully re-entrant.
 *
 * \see b64::decode()
 */
size_t b64_decode2(
    char const*   src
,   size_t              srcLen
,   void*               dest
,   size_t              destSize
,   unsigned            flags
,   char const**  badChar /* = NULL */
,   B64_RC*             rc      /* = NULL */
);

 //////////////////////////////////////////////////////////////////////////
 

 /** Encodes a block of binary data into Base-64
 *
 * \param src Pointer to the block to be encoded. May not be NULL, except when
 *   \c dest is NULL, in which case it is ignored.
 * \param srcSize Length of block to be encoded
 * \param dest Pointer to the buffer into which the result is to be written. May
 *   be NULL, in which case the function returns the required length
 * \param destLen Length of the buffer into which the result is to be written. Must
 *   be at least as large as that indicated by the return value from
 *   \link b64::b64_encode b64_encode(NULL, srcSize, NULL, 0)\endlink.
 *
 * \return 0 if the size of the buffer was insufficient, or the length of the
 * converted buffer was longer than \c destLen
 *
 * \note The function returns the required length if \c dest is NULL
 *
 * \note The function returns the required length if \c dest is NULL. The returned size
 *   might be larger than the actual required size, but will never be smaller.
 *
 * \note Threading: The function is fully re-entrant.
 *
 * \see b64::encode()
 */
size_t urlsafe_b64_encode(
    void const* src
,   size_t      srcSize
,   char* dest
,   size_t      destLen
);

/** Encodes a block of binary data into Base-64
 *
 * \param src Pointer to the block to be encoded. May not be NULL, except when
 *   \c dest is NULL, in which case it is ignored.
 * \param srcSize Length of block to be encoded
 * \param dest Pointer to the buffer into which the result is to be written. May
 *   be NULL, in which case the function returns the required length
 * \param destLen Length of the buffer into which the result is to be written. Must
 *   be at least as large as that indicated by the return value from
 *   \link b64::b64_encode2 b64_encode2(NULL, srcSize, NULL, 0, flags, lineLen, rc)\endlink.
 * \param flags A combination of the B64_FLAGS enumeration, that moderate the
 *   behaviour of the function
 * \param lineLen If the flags parameter contains B64_F_LINE_LEN_USE_PARAM, then
 *   this parameter represents the length of the lines into which the encoded form is split,
 *   with a hard line break ('\\r\\n'). If this value is 0, then the line is not
 *   split. If it is <0, then the RFC-1113 recommended line length of 64 is used
 * \param rc The return code representing the status of the operation. May be NULL.
 *
 * \return 0 if the size of the buffer was insufficient, or the length of the
 *   converted buffer was longer than \c destLen
 *
 * \note The function returns the required length if \c dest is NULL. The returned size
 *   might be larger than the actual required size, but will never be smaller.
 *
 * \note Threading: The function is fully re-entrant.
 *
 * \see b64::encode()
 */
size_t urlsafe_b64_encode2(
    void const* src
,   size_t      srcSize
,   char* dest
,   size_t      destLen
,   unsigned    flags
,   int         lineLen /* = 0 */
,   B64_RC*     rc     /* = NULL */
);

/** Decodes a sequence of Base-64 into a block of binary data
 *
 * \param src Pointer to the Base-64 block to be decoded. May not be NULL, except when
 *   \c dest is NULL, in which case it is ignored. If \c dest is NULL, and \c src is
 *   <b>not</b> NULL, then the returned value is calculated exactly, otherwise a value
 *   is returned that is guaranteed to be large enough to hold the decoded block.
 *
 * \param srcLen Length of block to be encoded. Must be an integral of 4, the Base-64
 *   encoding quantum, otherwise the Base-64 block is assumed to be invalid
 * \param dest Pointer to the buffer into which the result is to be written. May
 *   be NULL, in which case the function returns the required length
 * \param destSize Length of the buffer into which the result is to be written. Must
 *   be at least as large as that indicated by the return value from
 *   \c b64_decode(src, srcSize, NULL, 0), even in the case where the encoded form
 *   contains a number of characters that will be ignored, resulting in a lower total
 *   length of converted form.
 *
 * \return 0 if the size of the buffer was insufficient, or the length of the
 *   converted buffer was longer than \c destSize
 *
 * \note The function returns the required length if \c dest is NULL. The returned size
 *   might be larger than the actual required size, but will never be smaller.
 *
 * \note \anchor anchor__4_characters The behaviour of both
 * \link b64::b64_encode2 b64_encode2()\endlink
 * and
 * \link b64::b64_decode2 b64_decode2()\endlink
 * are undefined if the line length is not a multiple of 4.
 *
 * \note Threading: The function is fully re-entrant.
 *
 * \see b64::decode()
 */
size_t urlsafe_b64_decode(
    char const*   src
,   size_t              srcLen
,   void*               dest
,   size_t              destSize
);

/** Decodes a sequence of Base-64 into a block of binary data
 *
 * \param src Pointer to the Base-64 block to be decoded. May not be NULL, except when
 * \c dest is NULL, in which case it is ignored. If \c dest is NULL, and \c src is
 * <b>not</b> NULL, then the returned value is calculated exactly, otherwise a value
 * is returned that is guaranteed to be large enough to hold the decoded block.
 *
 * \param srcLen Length of block to be encoded. Must be an integral of 4, the Base-64
 *   encoding quantum, otherwise the Base-64 block is assumed to be invalid
 * \param dest Pointer to the buffer into which the result is to be written. May
 *   be NULL, in which case the function returns the required length
 * \param destSize Length of the buffer into which the result is to be written. Must
 *   be at least as large as that indicated by the return value from
 *   \c b64_decode(src, srcSize, NULL, 0), even in the case where the encoded form
 *   contains a number of characters that will be ignored, resulting in a lower total
 *   length of converted form.
 * \param flags A combination of the B64_FLAGS enumeration, that moderate the
 *   behaviour of the function.
 * \param rc The return code representing the status of the operation. May be NULL.
 * \param badChar If the flags parameter does not contain B64_F_STOP_ON_NOTHING, this
 *   parameter specifies the address of a pointer that will be set to point to any
 *   character in the sequence that stops the parsing, as dictated by the flags
 *   parameter. May be NULL.
 *
 * \return 0 if the size of the buffer was insufficient, or the length of the
 * converted buffer was longer than \c destSize, or a bad character stopped parsing.
 *
 * \note The function returns the required length if \c dest is NULL. The returned size
 *   might be larger than the actual required size, but will never be smaller.
 *
 * \note The behaviour of both
 * \link b64::b64_encode2 b64_encode2()\endlink
 * and
 * \link b64::b64_decode2 b64_decode2()\endlink
 * are undefined if the line length is not a multiple of 4.
 *
 * \note Threading: The function is fully re-entrant.
 *
 * \see b64::decode()
 */
size_t urlsafe_b64_decode2(
    char const*   src
,   size_t              srcLen
,   void*               dest
,   size_t              destSize
,   unsigned            flags
,   char const**  badChar /* = NULL */
,   B64_RC*             rc      /* = NULL */
);


/* /////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

#define b64_strchr_     strchr

/** This function reads in 3 bytes at a time, and translates them into 4
 * characters.
 */
size_t b64_encode_(
    unsigned char const*    src
,   size_t                  srcSize
,   char* const       dest
,   size_t                  destLen
,   unsigned                lineLen
,   B64_RC*                 rc
)
{
	static const char b64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t total = ((srcSize + (NUM_PLAIN_DATA_BYTES - 1)) / NUM_PLAIN_DATA_BYTES) * NUM_ENCODED_DATA_BYTES;

    B64_ENFORCE_PRECONDITION(NULL != rc, "pointer to return code may not be NULL");
    *rc = B64_RC_OK;

    if(lineLen > 0)
    {
        size_t numLines = (total + (lineLen - 1)) / lineLen;

        total += 2 * (0 == numLines ? 0u : (numLines - 1));
    }

    if(NULL == dest)
    {
        return total;
    }
    else if(destLen < total)
    {
        *rc = B64_RC_INSUFFICIENT_BUFFER;

        return 0;
    }
    else
    {
        char* p   =   dest;
        char* end =   dest + destLen;
        size_t      len =   0;

        for(; NUM_PLAIN_DATA_BYTES <= srcSize; srcSize -= NUM_PLAIN_DATA_BYTES)
        {
            char characters[NUM_ENCODED_DATA_BYTES];

            /* 
             * 
             * |       0       |       1       |       2       |
             *
             * |               |               |               |
             * |       |       |       |       |       |       |
             * |   |   |   |   |   |   |   |   |   |   |   |   |
             * | | | | | | | | | | | | | | | | | | | | | | | | |
             * 
             * |     0     |     1     |     2     |     3     |
             * 
             */

            /* characters[0] is the 6 left-most bits of src[0] */
            characters[0] = (char)((src[0] & 0xfc) >> 2);
            /* characters[0] is the right-most 2 bits of src[0] and the left-most 4 bits of src[1] */
            characters[1] = (char)(((src[0] & 0x03) << 4) + ((src[1] & 0xf0) >> 4));
            /* characters[0] is the right-most 4 bits of src[1] and the 2 left-most bits of src[2] */
            characters[2] = (char)(((src[1] & 0x0f) << 2) + ((src[2] & 0xc0) >> 6));
            /* characters[3] is the right-most 6 bits of src[2] */
            characters[3] = (char)(src[2] & 0x3f);

#ifndef __WATCOMC__
            B64_ENFORCE_ASSUMPTION(characters[0] >= 0 && characters[0] < 64);
            B64_ENFORCE_ASSUMPTION(characters[1] >= 0 && characters[1] < 64);
            B64_ENFORCE_ASSUMPTION(characters[2] >= 0 && characters[2] < 64);
            B64_ENFORCE_ASSUMPTION(characters[3] >= 0 && characters[3] < 64);
#endif /* __WATCOMC__ */

            src += NUM_PLAIN_DATA_BYTES;
            *p++ = b64_chars[(unsigned char)characters[0]];
            B64_ENFORCE_ASSUMPTION(NULL != b64_strchr_(b64_chars, *(p-1)));
            ++len;
            B64_ENFORCE_ASSUMPTION(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[1]];
            B64_ENFORCE_ASSUMPTION(NULL != b64_strchr_(b64_chars, *(p-1)));
            ++len;
            B64_ENFORCE_ASSUMPTION(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[2]];
            B64_ENFORCE_ASSUMPTION(NULL != b64_strchr_(b64_chars, *(p-1)));
            ++len;
            B64_ENFORCE_ASSUMPTION(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[3]];
            B64_ENFORCE_ASSUMPTION(NULL != b64_strchr_(b64_chars, *(p-1)));

            if( ++len == lineLen &&
                p != end)
            {
                *p++ = '\r';
                *p++ = '\n';
                len = 0;
            }
        }

        if(0 != srcSize)
        {
            /* Deal with the overspill, by boosting it up to three bytes (using 0s)
             * and then appending '=' for any missing characters.
             *
             * This is done into a temporary buffer, so we can call ourselves and
             * have the output continue to be written direct to the destination.
             */

            unsigned char   dummy[NUM_PLAIN_DATA_BYTES];
            size_t          i;

            for(i = 0; i != srcSize; ++i)
            {
                dummy[i] = *src++;
            }

            for(; i != NUM_PLAIN_DATA_BYTES; ++i)
            {
                dummy[i] = '\0';
            }

            b64_encode_(&dummy[0], NUM_PLAIN_DATA_BYTES, p, NUM_ENCODED_DATA_BYTES * (1 + 2), 0, rc);

            for(p += 1 + srcSize; srcSize++ != NUM_PLAIN_DATA_BYTES; )
            {
                *p++ = '=';
            }
        }

        return total;
    }
}

/** This function reads in a character string in 4-character chunks, and writes 
 * out the converted form in 3-byte chunks to the destination.
 */
size_t b64_decode_(
    char const*   src
,   size_t              srcLen
,   unsigned char*      dest
,   size_t              destSize
,   unsigned            flags
,   char const**  badChar
,   B64_RC*             rc
)
{
	static const signed char    b64_indexes[] =   
	{
		/* 0 - 31 / 0x00 - 0x1f */
			-1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1
		/* 32 - 63 / 0x20 - 0x3f */
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, 62, -1, -1, -1, 63  /* ... , '+', ... '/' */
		,   52, 53, 54, 55, 56, 57, 58, 59  /* '0' - '7'          */
		,   60, 61, -1, -1, -1, -1, -1, -1  /* '8', '9', ...      */
		/* 64 - 95 / 0x40 - 0x5f */
		,   -1, 0,  1,  2,  3,  4,  5,  6   /* ..., 'A' - 'G'     */
		,   7,  8,  9,  10, 11, 12, 13, 14  /* 'H' - 'O'          */
		,   15, 16, 17, 18, 19, 20, 21, 22  /* 'P' - 'W'          */
		,   23, 24, 25, -1, -1, -1, -1, -1  /* 'X', 'Y', 'Z', ... */
		/* 96 - 127 / 0x60 - 0x7f */
		,   -1, 26, 27, 28, 29, 30, 31, 32  /* ..., 'a' - 'g'     */
		,   33, 34, 35, 36, 37, 38, 39, 40  /* 'h' - 'o'          */
		,   41, 42, 43, 44, 45, 46, 47, 48  /* 'p' - 'w'          */
		,   49, 50, 51, -1, -1, -1, -1, -1  /* 'x', 'y', 'z', ... */

		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  

		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  

		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  

		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
	};

    const size_t    wholeChunks     =   (srcLen / NUM_ENCODED_DATA_BYTES);
    const size_t    remainderBytes  =   (srcLen % NUM_ENCODED_DATA_BYTES);
    size_t          maxTotal        =   (wholeChunks + (0 != remainderBytes)) * NUM_PLAIN_DATA_BYTES;
    unsigned char*  dest_           =   dest;

    ((void)remainderBytes); /* Avoids warning with Borland */

    B64_ENFORCE_PRECONDITION(NULL != badChar, "pointer to bad character may not be NULL");
    B64_ENFORCE_PRECONDITION(NULL != rc, "pointer to return code may not be NULL");

    *badChar    =   NULL;
    *rc         =   B64_RC_OK;

    if(NULL == dest)
    {
        return maxTotal;
    }
    else if(destSize < maxTotal)
    {
        *rc = B64_RC_INSUFFICIENT_BUFFER;

        return 0;
    }
    else
    {
        /* Now we iterate through the src, collecting together four characters
         * at a time from the Base-64 alphabet, until the end-point is reached.
         *
         * 
         */

        char const*       begin       =   src;
        char const* const end         =   begin + srcLen;
        size_t                  currIndex   =   0;
        size_t                  numPads     =   0;
        signed char             indexes[NUM_ENCODED_DATA_BYTES];    /* 4 */

        for(; begin != end; ++begin)
        {
            const char ch = *begin;

            if('=' == ch)
            {
                B64_ENFORCE_ASSUMPTION(currIndex < NUM_ENCODED_DATA_BYTES);

                indexes[currIndex++] = '\0';

                ++numPads;
            }
            else
            {
                /* NOTE: Had to rename 'index' to 'ix', due to name clash with GCC on 64-bit Linux. */
                signed char ix = b64_indexes[(unsigned char)ch];

                if(-1 == ix)
                {
                    switch(ch)
                    {
                        case    ' ':
                        case    '\t':
                        case    '\b':
                        case    '\v':
                            if(B64_F_STOP_ON_UNEXPECTED_WS & flags)
                            {
                                *rc         =   B64_RC_DATA_ERROR;
                                *badChar    =   begin;
                                return 0;
                            }
                            else
                            {
                                /* Fall through */
                            }
                        case    '\r':
                        case    '\n':
                            continue;
                        default:
                            if(B64_F_STOP_ON_UNKNOWN_CHAR & flags)
                            {
                                *rc         =   B64_RC_DATA_ERROR;
                                *badChar    =   begin;
                                return 0;
                            }
                            else
                            {
                                continue;
                            }
                    }
                }
                else
                {
                    numPads = 0;

                    B64_ENFORCE_ASSUMPTION(currIndex < NUM_ENCODED_DATA_BYTES);

                    indexes[currIndex++] = ix;
                }
            }

            if(NUM_ENCODED_DATA_BYTES == currIndex)
            {
                unsigned char bytes[NUM_PLAIN_DATA_BYTES];        /* 3 */

                bytes[0] = (unsigned char)((indexes[0] << 2) + ((indexes[1] & 0x30) >> 4));

                currIndex = 0;

                *dest++ = bytes[0];
                if(2 != numPads)
                {
                    bytes[1] = (unsigned char)(((indexes[1] & 0xf) << 4) + ((indexes[2] & 0x3c) >> 2));

                    *dest++ = bytes[1];

                    if(1 != numPads)
                    {
                        bytes[2] = (unsigned char)(((indexes[2] & 0x3) << 6) + indexes[3]);

                        *dest++ = bytes[2];
                    }
                }
                if(0 != numPads)
                {
                    break;
                }
            }
        }

        return (size_t)(dest - dest_);
    }
}

/* /////////////////////////////////////////////////////////////////////////
 * API functions
 */

size_t b64_encode(
    void const* src
,   size_t      srcSize
,   char* dest
,   size_t      destLen
)
{
    /* Use Null Object (Variable) here for rc, so do not need to check
     * elsewhere.
     */
    B64_RC  rc_;

    return b64_encode_((unsigned char const*)src, srcSize, dest, destLen, 0, &rc_);
}

size_t b64_encode2(
    void const* src
,   size_t      srcSize
,   char* dest
,   size_t      destLen
,   unsigned    flags
,   int         lineLen /* = -1 */
,   B64_RC*     rc     /* = NULL */
)
{
    /* Use Null Object (Variable) here for rc, so do not need to check
     * elsewhere
     */
    B64_RC  rc_;
    if(NULL == rc)
    {
        rc = &rc_;
    }

    switch(B64_F_LINE_LEN_MASK & flags)
    {
        case    B64_F_LINE_LEN_USE_PARAM:
            if(lineLen >= 0)
            {
                break;
            }
            /* Fall through to 64 */
        case    B64_F_LINE_LEN_64:
            lineLen = 64;
            break;
        case    B64_F_LINE_LEN_76:
            lineLen = 76;
            break;
        default:
            B64_ENFORCE_PRECONDITION(0, "Bad line length flag specified to b64_encode2()");
        case    B64_F_LINE_LEN_INFINITE:
            lineLen = 0;
            break;
    }

    B64_ENFORCE_PRECONDITION(0 == (lineLen % 4), "Bad line length flag specified to b64_encode2()");

    return b64_encode_((unsigned char const*)src, srcSize, dest, destLen, (unsigned)lineLen, rc);
}

size_t b64_decode(
    char const*   src
,   size_t              srcLen
,   void*               dest
,   size_t              destSize
)
{
    /* Use Null Object (Variable) here for rc and badChar, so do not need to
     * check elsewhere.
     */
    char const*   badChar_;
    B64_RC              rc_;

    return b64_decode_(src, srcLen, (unsigned char*)dest, destSize, B64_F_STOP_ON_NOTHING, &badChar_, &rc_);
}

size_t b64_decode2(
    char const*   src
,   size_t              srcLen
,   void*               dest
,   size_t              destSize
,   unsigned            flags
,   char const**  badChar /* = NULL */
,   B64_RC*             rc      /* = NULL */
)
{
    char const*   badChar_;
    B64_RC              rc_;

    /* Use Null Object (Variable) here for rc and badChar, so do not need to
     * check elsewhere.
     */
    if(NULL == badChar)
    {
        badChar = &badChar_;
    }
    if(NULL == rc)
    {
        rc = &rc_;
    }

    return b64_decode_(src, srcLen, (unsigned char*)dest, destSize, flags, badChar, rc);
}

/* ////////////////////////////////////////////////////////////////////// */

#ifdef B64_DOCUMENTATION_SKIP_SECTION
struct b64ErrorString_t_
#else /* !B64_DOCUMENTATION_SKIP_SECTION */
typedef struct b64ErrorString_t_    b64ErrorString_t_;
struct b64ErrorString_t_
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */
{
    int                 code;   /*!< The error code.    */
    char const*   str;    /*!< The string.        */
    size_t              len;    /*!< The string length. */
};



#define SEVERITY_STR_DECL(rc, desc)                                                         \
                                                                                            \
    static const char         s_str##rc[] =   desc;                                   \
    static const b64ErrorString_t_  s_rct##rc = { rc, s_str##rc, NUM_ELEMENTS(s_str##rc) - 1 }


#define SEVERITY_STR_ENTRY(rc)                                                          \
                                                                                        \
    &s_rct##rc


char const* b64_LookupCodeA_(int code, b64ErrorString_t_ const** mappings, size_t cMappings, size_t* len)
{
    /* Use Null Object (Variable) here for len, so do not need to check
     * elsewhere.
     */
    size_t  len_;

    if(NULL == len)
    {
        len = &len_;
    }

    /* Checked, indexed search. */
    if( code >= 0 &&
        code < B64_max_RC_value)
    {
        if(code == mappings[code]->code)
        {
            return (*len = mappings[code]->len, mappings[code]->str);
        }
    }

    /* Linear search. Should only be needed if order in
     * b64_LookupErrorStringA_() messed up.
     */
    { size_t i; for(i = 0; i != cMappings; ++i)
    {
        if(code == mappings[i]->code)
        {
            return (*len = mappings[i]->len, mappings[i]->str);
        }
    }}

    return (*len = 0, "");
}

char const* b64_LookupErrorStringA_(int error, size_t* len)
{
    SEVERITY_STR_DECL(B64_RC_OK                     ,   "Operation was successful"                                              );
    SEVERITY_STR_DECL(B64_RC_INSUFFICIENT_BUFFER    ,   "The given translation buffer was not of sufficient size"               );
    SEVERITY_STR_DECL(B64_RC_TRUNCATED_INPUT        ,   "The input did not represent a fully formed stream of octet couplings"  );
    SEVERITY_STR_DECL(B64_RC_DATA_ERROR             ,   "Invalid data"                                                          );

    static const b64ErrorString_t_* s_strings[] = 
    {
        SEVERITY_STR_ENTRY(B64_RC_OK),
        SEVERITY_STR_ENTRY(B64_RC_INSUFFICIENT_BUFFER),
        SEVERITY_STR_ENTRY(B64_RC_TRUNCATED_INPUT),
        SEVERITY_STR_ENTRY(B64_RC_DATA_ERROR),
    };

    return b64_LookupCodeA_(error, s_strings, NUM_ELEMENTS(s_strings), len);
}

char const* b64_getErrorString(B64_RC code)
{
    return b64_LookupErrorStringA_((int)code, NULL);
}

size_t b64_getErrorStringLength(B64_RC code)
{
    size_t  len;

    return (b64_LookupErrorStringA_((int)code, &len), len);
}

/* ///////////////////////////// end of file //////////////////////////// */

#ifndef B64_DOCUMENTATION_SKIP_SECTION
# define B64_VER_C_B64_MAJOR    1
# define B64_VER_C_B64_MINOR    3
# define B64_VER_C_B64_REVISION 3
# define B64_VER_C_B64_EDIT     20
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Includes
 */

#include "../SysBase_Interface.h"

#if defined(B64_VARIANT_TEST) && \
    !defined(B64_NO_USE_XCONTRACT)
# define B64_USING_XCONTRACT_
#endif /* B64_VARIANT_TEST && !B64_NO_USE_XCONTRACT */

#ifdef B64_USING_XCONTRACT_
# include <xcontract/xcontract.h>
#else /* ? B64_USING_XCONTRACT_ */
# include <assert.h>
#endif /* B64_USING_XCONTRACT_ */
#include <string.h>

/* /////////////////////////////////////////////////////////////////////////
 * Contract Enforcements
 */

#ifdef B64_USING_XCONTRACT_
# define B64_ENFORCE_PRECONDITION(expr, msg)        XCONTRACT_ENFORCE_PRECONDITION_PARAMETERS_2((expr), (msg))
# define B64_ENFORCE_ASSUMPTION(expr)               XCONTRACT_ENFORCE_ASSUMPTION_2((expr), "design invariant violated")
#else /* ? B64_USING_XCONTRACT_ */
# define B64_ENFORCE_PRECONDITION(expr, msg)        assert((expr))
# define B64_ENFORCE_ASSUMPTION(expr)               assert((expr))
#endif /* B64_USING_XCONTRACT_ */

/* /////////////////////////////////////////////////////////////////////////
 * Constants and definitions
 */

#ifndef B64_DOCUMENTATION_SKIP_SECTION
# define NUM_PLAIN_DATA_BYTES        (3)
# define NUM_ENCODED_DATA_BYTES      (4)
#endif /* !B64_DOCUMENTATION_SKIP_SECTION */

/* /////////////////////////////////////////////////////////////////////////
 * Macros
 */

#ifndef NUM_ELEMENTS
# define NUM_ELEMENTS(x)        (sizeof(x) / sizeof(x[0]))
#endif /* !NUM_ELEMENTS */

/* /////////////////////////////////////////////////////////////////////////
 * Warnings
 */

#if defined(_MSC_VER) && \
    _MSC_VER < 1000
# pragma warning(disable : 4705)
#endif /* _MSC_VER < 1000 */

/* /////////////////////////////////////////////////////////////////////////
 * Helper functions
 */

#define b64_strchr_     strchr

/** This function reads in 3 bytes at a time, and translates them into 4
 * characters.
 */
size_t urlsafe_b64_encode_(
    unsigned char const*    src
,   size_t                  srcSize
,   char* const       dest
,   size_t                  destLen
,   unsigned                lineLen
,   B64_RC*                 rc
)
{
	static const char b64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";
    size_t total = ((srcSize + (NUM_PLAIN_DATA_BYTES - 1)) / NUM_PLAIN_DATA_BYTES) * NUM_ENCODED_DATA_BYTES;

    B64_ENFORCE_PRECONDITION(NULL != rc, "pointer to return code may not be NULL");
    *rc = B64_RC_OK;

    if(lineLen > 0)
    {
        size_t numLines = (total + (lineLen - 1)) / lineLen;

        total += 2 * (0 == numLines ? 0u : (numLines - 1));
    }

    if(NULL == dest)
    {
        return total;
    }
    else if(destLen < total)
    {
        *rc = B64_RC_INSUFFICIENT_BUFFER;

        return 0;
    }
    else
    {
        char* p   =   dest;
        char* end =   dest + destLen;
        size_t      len =   0;

        for(; NUM_PLAIN_DATA_BYTES <= srcSize; srcSize -= NUM_PLAIN_DATA_BYTES)
        {
            char characters[NUM_ENCODED_DATA_BYTES];

            /* 
             * 
             * |       0       |       1       |       2       |
             *
             * |               |               |               |
             * |       |       |       |       |       |       |
             * |   |   |   |   |   |   |   |   |   |   |   |   |
             * | | | | | | | | | | | | | | | | | | | | | | | | |
             * 
             * |     0     |     1     |     2     |     3     |
             * 
             */

            /* characters[0] is the 6 left-most bits of src[0] */
            characters[0] = (char)((src[0] & 0xfc) >> 2);
            /* characters[0] is the right-most 2 bits of src[0] and the left-most 4 bits of src[1] */
            characters[1] = (char)(((src[0] & 0x03) << 4) + ((src[1] & 0xf0) >> 4));
            /* characters[0] is the right-most 4 bits of src[1] and the 2 left-most bits of src[2] */
            characters[2] = (char)(((src[1] & 0x0f) << 2) + ((src[2] & 0xc0) >> 6));
            /* characters[3] is the right-most 6 bits of src[2] */
            characters[3] = (char)(src[2] & 0x3f);

#ifndef __WATCOMC__
            B64_ENFORCE_ASSUMPTION(characters[0] >= 0 && characters[0] < 64);
            B64_ENFORCE_ASSUMPTION(characters[1] >= 0 && characters[1] < 64);
            B64_ENFORCE_ASSUMPTION(characters[2] >= 0 && characters[2] < 64);
            B64_ENFORCE_ASSUMPTION(characters[3] >= 0 && characters[3] < 64);
#endif /* __WATCOMC__ */

            src += NUM_PLAIN_DATA_BYTES;
            *p++ = b64_chars[(unsigned char)characters[0]];
            B64_ENFORCE_ASSUMPTION(NULL != b64_strchr_(b64_chars, *(p-1)));
            ++len;
            B64_ENFORCE_ASSUMPTION(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[1]];
            B64_ENFORCE_ASSUMPTION(NULL != b64_strchr_(b64_chars, *(p-1)));
            ++len;
            B64_ENFORCE_ASSUMPTION(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[2]];
            B64_ENFORCE_ASSUMPTION(NULL != b64_strchr_(b64_chars, *(p-1)));
            ++len;
            B64_ENFORCE_ASSUMPTION(len != lineLen);

            *p++ = b64_chars[(unsigned char)characters[3]];
            B64_ENFORCE_ASSUMPTION(NULL != b64_strchr_(b64_chars, *(p-1)));

            if( ++len == lineLen &&
                p != end)
            {
                *p++ = '\r';
                *p++ = '\n';
                len = 0;
            }
        }

        if(0 != srcSize)
        {
            /* Deal with the overspill, by boosting it up to three bytes (using 0s)
             * and then appending '=' for any missing characters.
             *
             * This is done into a temporary buffer, so we can call ourselves and
             * have the output continue to be written direct to the destination.
             */

            unsigned char   dummy[NUM_PLAIN_DATA_BYTES];
            size_t          i;

            for(i = 0; i != srcSize; ++i)
            {
                dummy[i] = *src++;
            }

            for(; i != NUM_PLAIN_DATA_BYTES; ++i)
            {
                dummy[i] = '\0';
            }

            urlsafe_b64_encode_(&dummy[0], NUM_PLAIN_DATA_BYTES, p, NUM_ENCODED_DATA_BYTES * (1 + 2), 0, rc);

            for(p += 1 + srcSize; srcSize++ != NUM_PLAIN_DATA_BYTES; )
            {
                *p++ = '=';
            }
        }

        return total;
    }
}

/** This function reads in a character string in 4-character chunks, and writes 
 * out the converted form in 3-byte chunks to the destination.
 */
size_t urlsafe_b64_decode_(
    char const*   src
,   size_t              srcLen
,   unsigned char*      dest
,   size_t              destSize
,   unsigned            flags
,   char const**  badChar
,   B64_RC*             rc
)
{
	static const signed char    b64_indexes[] =   
	{
		/* 0 - 31 / 0x00 - 0x1f */
			-1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1
		/* 32 - 63 / 0x20 - 0x3f */
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, 62, -1, 62, -1, 63  /* ... , '+', ',', '-', '.', '/' */
		,   52, 53, 54, 55, 56, 57, 58, 59  /* '0' - '7'          */
		,   60, 61, -1, -1, -1, -1, -1, -1  /* '8', '9', ...      */
		/* 64 - 95 / 0x40 - 0x5f */
		,   -1, 0,  1,  2,  3,  4,  5,  6   /* ..., 'A' - 'G'     */
		,   7,  8,  9,  10, 11, 12, 13, 14  /* 'H' - 'O'          */
		,   15, 16, 17, 18, 19, 20, 21, 22  /* 'P' - 'W'          */
		,   23, 24, 25, -1, -1, -1, -1, 63  /* 'X', 'Y', 'Z', ... '_' */
		/* 96 - 127 / 0x60 - 0x7f */
		,   -1, 26, 27, 28, 29, 30, 31, 32  /* ..., 'a' - 'g'     */
		,   33, 34, 35, 36, 37, 38, 39, 40  /* 'h' - 'o'          */
		,   41, 42, 43, 44, 45, 46, 47, 48  /* 'p' - 'w'          */
		,   49, 50, 51, -1, -1, -1, -1, -1  /* 'x', 'y', 'z', ... */

		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  

		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  

		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  

		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
		,   -1, -1, -1, -1, -1, -1, -1, -1  
	};

    const size_t    wholeChunks     =   (srcLen / NUM_ENCODED_DATA_BYTES);
    const size_t    remainderBytes  =   (srcLen % NUM_ENCODED_DATA_BYTES);
    size_t          maxTotal        =   (wholeChunks + (0 != remainderBytes)) * NUM_PLAIN_DATA_BYTES;
    unsigned char*  dest_           =   dest;

    ((void)remainderBytes); /* Avoids warning with Borland */

    B64_ENFORCE_PRECONDITION(NULL != badChar, "pointer to bad character may not be NULL");
    B64_ENFORCE_PRECONDITION(NULL != rc, "pointer to return code may not be NULL");

    *badChar    =   NULL;
    *rc         =   B64_RC_OK;

    if(NULL == dest)
    {
        return maxTotal;
    }
    else if(destSize < maxTotal)
    {
        *rc = B64_RC_INSUFFICIENT_BUFFER;

        return 0;
    }
    else
    {
        /* Now we iterate through the src, collecting together four characters
         * at a time from the Base-64 alphabet, until the end-point is reached.
         *
         * 
         */

        char const*       begin       =   src;
        char const* const end         =   begin + srcLen;
        size_t                  currIndex   =   0;
        size_t                  numPads     =   0;
        signed char             indexes[NUM_ENCODED_DATA_BYTES];    /* 4 */

        for(; begin != end; ++begin)
        {
            const char ch = *begin;

            if('=' == ch)
            {
                B64_ENFORCE_ASSUMPTION(currIndex < NUM_ENCODED_DATA_BYTES);

                indexes[currIndex++] = '\0';

                ++numPads;
            }
            else
            {
                /* NOTE: Had to rename 'index' to 'ix', due to name clash with GCC on 64-bit Linux. */
                signed char ix = b64_indexes[(unsigned char)ch];

                if(-1 == ix)
                {
                    switch(ch)
                    {
                        case    ' ':
                        case    '\t':
                        case    '\b':
                        case    '\v':
                            if(B64_F_STOP_ON_UNEXPECTED_WS & flags)
                            {
                                *rc         =   B64_RC_DATA_ERROR;
                                *badChar    =   begin;
                                return 0;
                            }
                            else
                            {
                                /* Fall through */
                            }
                        case    '\r':
                        case    '\n':
                            continue;
                        default:
                            if(B64_F_STOP_ON_UNKNOWN_CHAR & flags)
                            {
                                *rc         =   B64_RC_DATA_ERROR;
                                *badChar    =   begin;
                                return 0;
                            }
                            else
                            {
                                continue;
                            }
                    }
                }
                else
                {
                    numPads = 0;

                    B64_ENFORCE_ASSUMPTION(currIndex < NUM_ENCODED_DATA_BYTES);

                    indexes[currIndex++] = ix;
                }
            }

            if(NUM_ENCODED_DATA_BYTES == currIndex)
            {
                unsigned char bytes[NUM_PLAIN_DATA_BYTES];        /* 3 */

                bytes[0] = (unsigned char)((indexes[0] << 2) + ((indexes[1] & 0x30) >> 4));

                currIndex = 0;

                *dest++ = bytes[0];
                if(2 != numPads)
                {
                    bytes[1] = (unsigned char)(((indexes[1] & 0xf) << 4) + ((indexes[2] & 0x3c) >> 2));

                    *dest++ = bytes[1];

                    if(1 != numPads)
                    {
                        bytes[2] = (unsigned char)(((indexes[2] & 0x3) << 6) + indexes[3]);

                        *dest++ = bytes[2];
                    }
                }
                if(0 != numPads)
                {
                    break;
                }
            }
        }

        return (size_t)(dest - dest_);
    }
}

/* /////////////////////////////////////////////////////////////////////////
 * API functions
 */

size_t urlsafe_b64_encode(
    void const* src
,   size_t      srcSize
,   char* dest
,   size_t      destLen
)
{
    /* Use Null Object (Variable) here for rc, so do not need to check
     * elsewhere.
     */
    B64_RC  rc_;

    return urlsafe_b64_encode_((unsigned char const*)src, srcSize, dest, destLen, 0, &rc_);
}

size_t urlsafe_b64_encode2(
    void const* src
,   size_t      srcSize
,   char* dest
,   size_t      destLen
,   unsigned    flags
,   int         lineLen /* = -1 */
,   B64_RC*     rc     /* = NULL */
)
{
    /* Use Null Object (Variable) here for rc, so do not need to check
     * elsewhere
     */
    B64_RC  rc_;
    if(NULL == rc)
    {
        rc = &rc_;
    }

    switch(B64_F_LINE_LEN_MASK & flags)
    {
        case    B64_F_LINE_LEN_USE_PARAM:
            if(lineLen >= 0)
            {
                break;
            }
            /* Fall through to 64 */
        case    B64_F_LINE_LEN_64:
            lineLen = 64;
            break;
        case    B64_F_LINE_LEN_76:
            lineLen = 76;
            break;
        default:
            B64_ENFORCE_PRECONDITION(0, "Bad line length flag specified to b64_encode2()");
        case    B64_F_LINE_LEN_INFINITE:
            lineLen = 0;
            break;
    }

    B64_ENFORCE_PRECONDITION(0 == (lineLen % 4), "Bad line length flag specified to b64_encode2()");

    return urlsafe_b64_encode_((unsigned char const*)src, srcSize, dest, destLen, (unsigned)lineLen, rc);
}

size_t urlsafe_b64_decode(
    char const*   src
,   size_t              srcLen
,   void*               dest
,   size_t              destSize
)
{
    /* Use Null Object (Variable) here for rc and badChar, so do not need to
     * check elsewhere.
     */
    char const*   badChar_;
    B64_RC              rc_;

    return urlsafe_b64_decode_(src, srcLen, (unsigned char*)dest, destSize, B64_F_STOP_ON_NOTHING, &badChar_, &rc_);
}

size_t urlsafe_b64_decode2(
    char const*   src
,   size_t              srcLen
,   void*               dest
,   size_t              destSize
,   unsigned            flags
,   char const**  badChar /* = NULL */
,   B64_RC*             rc      /* = NULL */
)
{
    char const*   badChar_;
    B64_RC              rc_;

    /* Use Null Object (Variable) here for rc and badChar, so do not need to
     * check elsewhere.
     */
    if(NULL == badChar)
    {
        badChar = &badChar_;
    }
    if(NULL == rc)
    {
        rc = &rc_;
    }

    return urlsafe_b64_decode_(src, srcLen, (unsigned char*)dest, destSize, flags, badChar, rc);
}

/* ///////////////////////////// end of file //////////////////////////// */

namespace SysBase
{
    UINT32 CBase64::Encode(const void* pSource, UINT32 nSourceSize, char* pDest, UINT32 nDestLen)
    {
        return (UINT32)b64_encode(pSource, nSourceSize, pDest, nDestLen);
    }

    UINT32 CBase64::Decode(char const* pSource, UINT32 nSourceLen, void* pDest, UINT32 nDestSize)
    {
        return (UINT32)b64_decode(pSource, nSourceLen, pDest, nDestSize);
    }

    UINT32 CBase64::SafeUrlEncode(const void* pSource, UINT32 nSourceSize, char* pDest, UINT32 nDestLen)
    {
        return (UINT32)urlsafe_b64_encode(pSource, nSourceSize, pDest, nDestLen);
    }

    UINT32 CBase64::SafeUrlDecode(char const* pSource, UINT32 nSourceLen, void* pDest, UINT32 nDestSize)
    {
        return (UINT32)urlsafe_b64_decode(pSource, nSourceLen, pDest, nDestSize);
    }
}