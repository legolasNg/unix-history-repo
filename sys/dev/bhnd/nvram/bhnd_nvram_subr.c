/*-
 * Copyright (c) 2015-2016 Landon Fuller <landonf@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer,
 *    without modification.
 * 2. Redistributions in binary form must reproduce at minimum a disclaimer
 *    similar to the "NO WARRANTY" disclaimer below ("Disclaimer") and any
 *    redistribution must be conditioned upon including a substantially
 *    similar Disclaimer requirement for further binary redistribution.
 *
 * NO WARRANTY
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF NONINFRINGEMENT, MERCHANTIBILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGES.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>

#ifdef _KERNEL

#include <sys/ctype.h>
#include <sys/kernel.h>
#include <sys/limits.h>
#include <sys/malloc.h>
#include <sys/systm.h>

#include <machine/_inttypes.h>

#else /* !_KERNEL */

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#endif /* _KERNEL */

#include "bhnd_nvram_io.h"
#include "bhnd_nvram_private.h"
#include "bhnd_nvram_value.h"

#include "bhnd_nvram_map_data.h"

/*
 * Common NVRAM/SPROM support, including NVRAM variable map
 * lookup.
 */

#ifdef _KERNEL
MALLOC_DEFINE(M_BHND_NVRAM, "bhnd_nvram", "bhnd nvram data");
#endif

/** signed/unsigned 32-bit integer value storage */
union bhnd_nvram_int_storage {
	uint32_t	u32;
	int32_t		s32;
};

/*
 * CRC-8 lookup table used to checksum SPROM and NVRAM data via
 * bhnd_nvram_crc8().
 * 
 * Generated with following parameters:
 * 	polynomial:	CRC-8 (x^8 + x^7 + x^6 + x^4 + x^2 + 1)
 * 	reflected bits:	false
 * 	reversed:	true
 */
const uint8_t bhnd_nvram_crc8_tab[] = {
	0x00, 0xf7, 0xb9, 0x4e, 0x25, 0xd2, 0x9c, 0x6b, 0x4a, 0xbd, 0xf3,
	0x04, 0x6f, 0x98, 0xd6, 0x21, 0x94, 0x63, 0x2d, 0xda, 0xb1, 0x46,
	0x08, 0xff, 0xde, 0x29, 0x67, 0x90, 0xfb, 0x0c, 0x42, 0xb5, 0x7f,
	0x88, 0xc6, 0x31, 0x5a, 0xad, 0xe3, 0x14, 0x35, 0xc2, 0x8c, 0x7b,
	0x10, 0xe7, 0xa9, 0x5e, 0xeb, 0x1c, 0x52, 0xa5, 0xce, 0x39, 0x77,
	0x80, 0xa1, 0x56, 0x18, 0xef, 0x84, 0x73, 0x3d, 0xca, 0xfe, 0x09,
	0x47, 0xb0, 0xdb, 0x2c, 0x62, 0x95, 0xb4, 0x43, 0x0d, 0xfa, 0x91,
	0x66, 0x28, 0xdf, 0x6a, 0x9d, 0xd3, 0x24, 0x4f, 0xb8, 0xf6, 0x01,
	0x20, 0xd7, 0x99, 0x6e, 0x05, 0xf2, 0xbc, 0x4b, 0x81, 0x76, 0x38,
	0xcf, 0xa4, 0x53, 0x1d, 0xea, 0xcb, 0x3c, 0x72, 0x85, 0xee, 0x19,
	0x57, 0xa0, 0x15, 0xe2, 0xac, 0x5b, 0x30, 0xc7, 0x89, 0x7e, 0x5f,
	0xa8, 0xe6, 0x11, 0x7a, 0x8d, 0xc3, 0x34, 0xab, 0x5c, 0x12, 0xe5,
	0x8e, 0x79, 0x37, 0xc0, 0xe1, 0x16, 0x58, 0xaf, 0xc4, 0x33, 0x7d,
	0x8a, 0x3f, 0xc8, 0x86, 0x71, 0x1a, 0xed, 0xa3, 0x54, 0x75, 0x82,
	0xcc, 0x3b, 0x50, 0xa7, 0xe9, 0x1e, 0xd4, 0x23, 0x6d, 0x9a, 0xf1,
	0x06, 0x48, 0xbf, 0x9e, 0x69, 0x27, 0xd0, 0xbb, 0x4c, 0x02, 0xf5,
	0x40, 0xb7, 0xf9, 0x0e, 0x65, 0x92, 0xdc, 0x2b, 0x0a, 0xfd, 0xb3,
	0x44, 0x2f, 0xd8, 0x96, 0x61, 0x55, 0xa2, 0xec, 0x1b, 0x70, 0x87,
	0xc9, 0x3e, 0x1f, 0xe8, 0xa6, 0x51, 0x3a, 0xcd, 0x83, 0x74, 0xc1,
	0x36, 0x78, 0x8f, 0xe4, 0x13, 0x5d, 0xaa, 0x8b, 0x7c, 0x32, 0xc5,
	0xae, 0x59, 0x17, 0xe0, 0x2a, 0xdd, 0x93, 0x64, 0x0f, 0xf8, 0xb6,
	0x41, 0x60, 0x97, 0xd9, 0x2e, 0x45, 0xb2, 0xfc, 0x0b, 0xbe, 0x49,
	0x07, 0xf0, 0x9b, 0x6c, 0x22, 0xd5, 0xf4, 0x03, 0x4d, 0xba, 0xd1,
	0x26, 0x68, 0x9f
};

/**
 * Return a human readable name for @p type.
 * 
 * @param type The type to query.
 */
const char *
bhnd_nvram_type_name(bhnd_nvram_type type)
{
	switch (type) {
	case BHND_NVRAM_TYPE_UINT8:
		return ("uint8");
	case BHND_NVRAM_TYPE_UINT16:
		return ("uint16");
	case BHND_NVRAM_TYPE_UINT32:
		return ("uint32");
	case BHND_NVRAM_TYPE_UINT64:
		return ("uint64");
	case BHND_NVRAM_TYPE_CHAR:
		return ("char");
	case BHND_NVRAM_TYPE_INT8:
		return ("int8");
	case BHND_NVRAM_TYPE_INT16:
		return ("int16");
	case BHND_NVRAM_TYPE_INT32:
		return ("int32");
	case BHND_NVRAM_TYPE_INT64:
		return ("int64");
	case BHND_NVRAM_TYPE_STRING:
		return ("string");
	case BHND_NVRAM_TYPE_UINT8_ARRAY:
		return ("uint8[]");
	case BHND_NVRAM_TYPE_UINT16_ARRAY:
		return ("uint16[]");
	case BHND_NVRAM_TYPE_UINT32_ARRAY:
		return ("uint32[]");
	case BHND_NVRAM_TYPE_UINT64_ARRAY:
		return ("uint64[]");
	case BHND_NVRAM_TYPE_INT8_ARRAY:
		return ("int8[]");
	case BHND_NVRAM_TYPE_INT16_ARRAY:
		return ("int16[]");
	case BHND_NVRAM_TYPE_INT32_ARRAY:
		return ("int32[]");
	case BHND_NVRAM_TYPE_INT64_ARRAY:
		return ("int64[]");
	case BHND_NVRAM_TYPE_CHAR_ARRAY:
		return ("char[]");
	case BHND_NVRAM_TYPE_STRING_ARRAY:
		return ("string[]");
	}

	/* Quiesce gcc4.2 */
	BHND_NV_PANIC("bhnd nvram type %u unknown", type);
}

/**
 * Return true if @p type is a signed integer type, false otherwise.
 * 
 * Will return false for all array types.
 * 
 * @param type The type to query.
 */
bool
bhnd_nvram_is_signed_type(bhnd_nvram_type type)
{
	switch (type) {
	case BHND_NVRAM_TYPE_INT8:
	case BHND_NVRAM_TYPE_INT16:
	case BHND_NVRAM_TYPE_INT32:
	case BHND_NVRAM_TYPE_INT64:
		BHND_NV_ASSERT(bhnd_nvram_is_int_type(type), ("non-int type?"));
		return (true);

	case BHND_NVRAM_TYPE_CHAR:
	case BHND_NVRAM_TYPE_UINT8:
	case BHND_NVRAM_TYPE_UINT16:
	case BHND_NVRAM_TYPE_UINT32:
	case BHND_NVRAM_TYPE_UINT64:
	case BHND_NVRAM_TYPE_STRING:
	case BHND_NVRAM_TYPE_UINT8_ARRAY:
	case BHND_NVRAM_TYPE_UINT16_ARRAY:
	case BHND_NVRAM_TYPE_UINT32_ARRAY:
	case BHND_NVRAM_TYPE_UINT64_ARRAY:
	case BHND_NVRAM_TYPE_INT8_ARRAY:
	case BHND_NVRAM_TYPE_INT16_ARRAY:
	case BHND_NVRAM_TYPE_INT32_ARRAY:
	case BHND_NVRAM_TYPE_INT64_ARRAY:
	case BHND_NVRAM_TYPE_CHAR_ARRAY:
	case BHND_NVRAM_TYPE_STRING_ARRAY:
		return (false);
	}

	/* Quiesce gcc4.2 */
	BHND_NV_PANIC("bhnd nvram type %u unknown", type);
}

/**
 * Return true if @p type is an unsigned integer type, false otherwise.
 * 
 * @param type The type to query.
 *
 * @return Will return false for all array types.
 * @return Will return true for BHND_NVRAM_TYPE_CHAR.
 */
bool
bhnd_nvram_is_unsigned_type(bhnd_nvram_type type)
{
	/* If an integer type, must be either signed or unsigned */
	if (!bhnd_nvram_is_int_type(type))
		return (false);

	return (!bhnd_nvram_is_signed_type(type));
}

/**
 * Return true if bhnd_nvram_is_signed_type() or bhnd_nvram_is_unsigned_type()
 * returns true for @p type.
 * 
 * @param type The type to query.
 */
bool
bhnd_nvram_is_int_type(bhnd_nvram_type type)
{
	switch (type) {
	case BHND_NVRAM_TYPE_UINT8:
	case BHND_NVRAM_TYPE_UINT16:
	case BHND_NVRAM_TYPE_UINT32:
	case BHND_NVRAM_TYPE_UINT64:
	case BHND_NVRAM_TYPE_INT8:
	case BHND_NVRAM_TYPE_INT16:
	case BHND_NVRAM_TYPE_INT32:
	case BHND_NVRAM_TYPE_INT64:
		return (true);

	case BHND_NVRAM_TYPE_CHAR:
	case BHND_NVRAM_TYPE_STRING:
	case BHND_NVRAM_TYPE_UINT8_ARRAY:
	case BHND_NVRAM_TYPE_UINT16_ARRAY:
	case BHND_NVRAM_TYPE_UINT32_ARRAY:
	case BHND_NVRAM_TYPE_UINT64_ARRAY:
	case BHND_NVRAM_TYPE_INT8_ARRAY:
	case BHND_NVRAM_TYPE_INT16_ARRAY:
	case BHND_NVRAM_TYPE_INT32_ARRAY:
	case BHND_NVRAM_TYPE_INT64_ARRAY:
	case BHND_NVRAM_TYPE_CHAR_ARRAY:
	case BHND_NVRAM_TYPE_STRING_ARRAY:
		return (false);
	}

	/* Quiesce gcc4.2 */
	BHND_NV_PANIC("bhnd nvram type %u unknown", type);
}

/**
 * Return true if @p type is an array type, false otherwise.
 * 
 * @param type The type to query.
 */
bool
bhnd_nvram_is_array_type(bhnd_nvram_type type)
{
	switch (type) {
	case BHND_NVRAM_TYPE_UINT8:
	case BHND_NVRAM_TYPE_UINT16:
	case BHND_NVRAM_TYPE_UINT32:
	case BHND_NVRAM_TYPE_UINT64:
	case BHND_NVRAM_TYPE_INT8:
	case BHND_NVRAM_TYPE_INT16:
	case BHND_NVRAM_TYPE_INT32:
	case BHND_NVRAM_TYPE_INT64:
	case BHND_NVRAM_TYPE_CHAR:
	case BHND_NVRAM_TYPE_STRING:
		return (false);

	case BHND_NVRAM_TYPE_UINT8_ARRAY:
	case BHND_NVRAM_TYPE_UINT16_ARRAY:
	case BHND_NVRAM_TYPE_UINT32_ARRAY:
	case BHND_NVRAM_TYPE_UINT64_ARRAY:
	case BHND_NVRAM_TYPE_INT8_ARRAY:
	case BHND_NVRAM_TYPE_INT16_ARRAY:
	case BHND_NVRAM_TYPE_INT32_ARRAY:
	case BHND_NVRAM_TYPE_INT64_ARRAY:
	case BHND_NVRAM_TYPE_CHAR_ARRAY:
	case BHND_NVRAM_TYPE_STRING_ARRAY:
		return (true);
	}

	/* Quiesce gcc4.2 */
	BHND_NV_PANIC("bhnd nvram type %u unknown", type);
}

/**
 * If @p type is an array type, return the base element type. Otherwise,
 * returns @p type.
 * 
 * @param type The type to query.
 */
bhnd_nvram_type
bhnd_nvram_base_type(bhnd_nvram_type type)
{
	switch (type) {
	case BHND_NVRAM_TYPE_UINT8:
	case BHND_NVRAM_TYPE_UINT16:
	case BHND_NVRAM_TYPE_UINT32:
	case BHND_NVRAM_TYPE_UINT64:
	case BHND_NVRAM_TYPE_INT8:
	case BHND_NVRAM_TYPE_INT16:
	case BHND_NVRAM_TYPE_INT32:
	case BHND_NVRAM_TYPE_INT64:
	case BHND_NVRAM_TYPE_CHAR:
	case BHND_NVRAM_TYPE_STRING:
		return (type);

	case BHND_NVRAM_TYPE_UINT8_ARRAY:	return (BHND_NVRAM_TYPE_UINT8);
	case BHND_NVRAM_TYPE_UINT16_ARRAY:	return (BHND_NVRAM_TYPE_UINT16);
	case BHND_NVRAM_TYPE_UINT32_ARRAY:	return (BHND_NVRAM_TYPE_UINT32);
	case BHND_NVRAM_TYPE_UINT64_ARRAY:	return (BHND_NVRAM_TYPE_UINT64);
	case BHND_NVRAM_TYPE_INT8_ARRAY:	return (BHND_NVRAM_TYPE_INT8);
	case BHND_NVRAM_TYPE_INT16_ARRAY:	return (BHND_NVRAM_TYPE_INT16);
	case BHND_NVRAM_TYPE_INT32_ARRAY:	return (BHND_NVRAM_TYPE_INT32);
	case BHND_NVRAM_TYPE_INT64_ARRAY:	return (BHND_NVRAM_TYPE_INT64);
	case BHND_NVRAM_TYPE_CHAR_ARRAY:	return (BHND_NVRAM_TYPE_CHAR);
	case BHND_NVRAM_TYPE_STRING_ARRAY:	return (BHND_NVRAM_TYPE_STRING);
	}

	/* Quiesce gcc4.2 */
	BHND_NV_PANIC("bhnd nvram type %u unknown", type);
}

/**
 * Calculate the number of elements represented by a value of @p len bytes
 * with @p type.
 *
 * @param	type	The value type.
 * @param	data	The actual data to be queried, or NULL if unknown.
 * @param	len	The length in bytes of @p data, or if @p data is NULL,
 *			the expected length in bytes.
 * @param[out]	nelem	On success, the number of elements. If @p type is not
 *			a fixed width type (e.g. BHND_NVRAM_TYPE_STRING_ARRAY),
 *			and @p data is NULL, an @p nelem value of 0 will be
 *			returned.
 *
 * @retval 0		success
 * @retval EFTYPE	if @p type is not an array type, and @p len is not
 *			equal to the size of a single element of @p type.
 * @retval EFAULT	if @p len is not correctly aligned for elements of
 *			@p type.
 */
int
bhnd_nvram_value_nelem(bhnd_nvram_type type, const void *data, size_t len,
    size_t *nelem)
{
	bhnd_nvram_type	base_type;
	size_t		base_size;

	/* Length must be aligned to the element size */
	base_type = bhnd_nvram_base_type(type);
	base_size = bhnd_nvram_value_size(base_type, NULL, 0, 1);
	if (base_size != 0 && len % base_size != 0)
		return (EFAULT);

	switch (type) {
	case BHND_NVRAM_TYPE_STRING:
	case BHND_NVRAM_TYPE_STRING_ARRAY: {
		const char	*p;
		size_t		 nleft;

		/* Cannot determine the element count without parsing
		 * the actual data */
		if (data == NULL) {
			*nelem = 0;
			return (0);
		}

		/* Iterate over the NUL-terminated strings to calculate
		 * total element count */
		p = data;
		nleft = len;
		*nelem = 0;
		while (nleft > 0) {
			size_t slen;

			/* Increment element count */
			(*nelem)++;

			/* If not a string array, data must not contain more
			 * than one entry. */
			if (!bhnd_nvram_is_array_type(type) && *nelem > 1)
				return (EFTYPE);

			/* Determine string length */
			slen = strnlen(p, nleft);
			nleft -= slen;
	
			/* Advance input */
			p += slen;

			/* Account for trailing NUL, if we haven't hit the end
			 * of the input */
			if (nleft > 0) {
				nleft--;
				p++;
			}
		}

		return (0);
	}
	case BHND_NVRAM_TYPE_INT8:
	case BHND_NVRAM_TYPE_UINT8:
	case BHND_NVRAM_TYPE_CHAR:
	case BHND_NVRAM_TYPE_INT16:
	case BHND_NVRAM_TYPE_UINT16:
	case BHND_NVRAM_TYPE_INT32:
	case BHND_NVRAM_TYPE_UINT32:
	case BHND_NVRAM_TYPE_INT64:
	case BHND_NVRAM_TYPE_UINT64:
		/* Length must be equal to the size of exactly one
		 * element (arrays can represent zero elements -- non-array
		 * types cannot) */
		if (len != base_size)
			return (EFTYPE);
		*nelem = 1;
		return (0);

	case BHND_NVRAM_TYPE_UINT8_ARRAY:
	case BHND_NVRAM_TYPE_UINT16_ARRAY:
	case BHND_NVRAM_TYPE_UINT32_ARRAY:
	case BHND_NVRAM_TYPE_UINT64_ARRAY:
	case BHND_NVRAM_TYPE_INT8_ARRAY:
	case BHND_NVRAM_TYPE_INT16_ARRAY:
	case BHND_NVRAM_TYPE_INT32_ARRAY:
	case BHND_NVRAM_TYPE_INT64_ARRAY:
	case BHND_NVRAM_TYPE_CHAR_ARRAY:
		BHND_NV_ASSERT(base_size != 0, ("invalid base size"));
		*nelem = len / base_size;
		return (0);
	}

	/* Quiesce gcc4.2 */
	BHND_NV_PANIC("bhnd nvram type %u unknown", type);
}

/**
 * Return the size, in bytes, of a value of @p type with @p nelem elements.
 * 
 * @param	type	The value type.
 * @param	data	The actual data to be queried, or NULL if unknown. If
 *			NULL and the base type is not a fixed width type
 *			(e.g. BHND_NVRAM_TYPE_STRING), 0 will be returned.
 * @param	nbytes	The size of @p data, in bytes, or 0 if @p data is NULL.
 * @param	nelem	The number of elements. If @p type is not an array type,
 *			this value must be 1.
 * 
 * @retval 0		If @p type has a variable width, and @p data is NULL.
 * @retval 0		If a @p nelem value greater than 1 is provided for a
 *			non-array @p type.
 * @retval 0		If a @p nelem value of 0 is provided.
 * @retval 0		If the result would exceed the maximum value
 *			representable by size_t.
 * @retval non-zero	The size, in bytes, of @p type with @p nelem elements.
 */
size_t
bhnd_nvram_value_size(bhnd_nvram_type type, const void *data, size_t nbytes,
    size_t nelem)
{
	/* If nelem 0, nothing to do */
	if (nelem == 0)
		return (0);

	/* Non-array types must have an nelem value of 1 */
	if (!bhnd_nvram_is_array_type(type) && nelem != 1)
		return (0);

	switch (type) {
	case BHND_NVRAM_TYPE_UINT8_ARRAY:
	case BHND_NVRAM_TYPE_UINT16_ARRAY:
	case BHND_NVRAM_TYPE_UINT32_ARRAY:
	case BHND_NVRAM_TYPE_UINT64_ARRAY:
	case BHND_NVRAM_TYPE_INT8_ARRAY:
	case BHND_NVRAM_TYPE_INT16_ARRAY:
	case BHND_NVRAM_TYPE_INT32_ARRAY:
	case BHND_NVRAM_TYPE_INT64_ARRAY:
	case BHND_NVRAM_TYPE_CHAR_ARRAY: {
		bhnd_nvram_type	base_type;
		size_t		base_size;

		base_type = bhnd_nvram_base_type(type);
		base_size = bhnd_nvram_value_size(base_type, NULL, 0, 1);

		/* Would nelem * base_size overflow? */
		if (SIZE_MAX / nelem < base_size) {
			BHND_NV_LOG("cannot represent size %s * %zu\n",
			    bhnd_nvram_type_name(base_type), nelem);
			return (0);
		}

		return (nelem * base_size);
	}

	case BHND_NVRAM_TYPE_STRING_ARRAY: {
		const char	*p;
		size_t		 total_size;

		if (data == NULL)
			return (0);

		/* Iterate over the NUL-terminated strings to calculate
		 * total byte length */
		p = data;
		total_size = 0;
		for (size_t i = 0; i < nelem; i++) {
			size_t	elem_size;

			elem_size = strnlen(p, nbytes - total_size);
			p += elem_size;

			/* Check for (and skip) terminating NUL */
			if (total_size < nbytes && *p == '\0') {
				elem_size++;
				p++;
			}

			/* Would total_size + elem_size overflow?
			 * 
			 * A memory range larger than SIZE_MAX shouldn't be,
			 * possible, but include the check for completeness */
			if (SIZE_MAX - total_size < elem_size)
				return (0);

			total_size += elem_size;
		}

		return (total_size);
	}

	case BHND_NVRAM_TYPE_STRING: {
		size_t size;

		if (data == NULL)
			return (0);

		/* Find length */
		size = strnlen(data, nbytes);

		/* Is there a terminating NUL, or did we just hit the
		 * end of the string input */
		if (size < nbytes)
			size++;

		return (size);
	}
	case BHND_NVRAM_TYPE_INT8:
	case BHND_NVRAM_TYPE_UINT8:
	case BHND_NVRAM_TYPE_CHAR:
		return (sizeof(uint8_t));

	case BHND_NVRAM_TYPE_INT16:
	case BHND_NVRAM_TYPE_UINT16:
		return (sizeof(uint16_t));

	case BHND_NVRAM_TYPE_INT32:
	case BHND_NVRAM_TYPE_UINT32:
		return (sizeof(uint32_t));

	case BHND_NVRAM_TYPE_UINT64:
	case BHND_NVRAM_TYPE_INT64:
		return (sizeof(uint64_t));
	}

	/* Quiesce gcc4.2 */
	BHND_NV_PANIC("bhnd nvram type %u unknown", type);
}

/**
 * Iterate over all strings in the @p inp string array.
 *
 * @param	inp	The string array to be iterated. This must be a buffer
 *			of one or more NUL-terminated strings --
 *			@see BHND_NVRAM_TYPE_STRING_ARRAY.
 * @param	ilen	The size, in bytes, of @p inp, including any
 *			terminating NUL character(s).
 * @param	prev	The value previously returned by
 *			bhnd_nvram_string_array_next(), or NULL to begin
 *			iteration.
 *
 * @retval non-NULL	A reference to the next NUL-terminated string
 * @retval NULL		If the end of the string array is reached.
 */
const char *
bhnd_nvram_string_array_next(const char *inp, size_t ilen, const char *prev)
{
	size_t nremain, plen;

	if (ilen == 0)
		return (NULL);

	if (prev == NULL)
		return (inp);

	/* Advance to next value */
	BHND_NV_ASSERT(prev >= inp, ("invalid prev pointer"));
	BHND_NV_ASSERT(prev < (inp+ilen), ("invalid prev pointer"));

	nremain = ilen - (size_t)(prev - inp);
	plen = strnlen(prev, nremain);
	nremain -= plen;

	/* Only a trailing NUL remains? */
	if (nremain <= 1)
		return (NULL);

	return (prev + plen + 1);
}

/**
 * Format a string representation of @p inp using @p fmt, with, writing the
 * result to @p outp.
 *
 * Refer to bhnd_nvram_val_vprintf() for full format string documentation.
 *
 * @param		fmt	The format string.
 * @param		inp	The value to be formatted.
 * @param		ilen	The size of @p inp, in bytes.
 * @param		itype	The type of @p inp.
 * @param[out]		outp	On success, the string value will be written to
 *				this buffer. This argment may be NULL if the
 *				value is not desired.
 * @param[in,out]	olen	The capacity of @p outp. On success, will be set
 *				to the actual size of the formatted string.
 *
 * @retval 0		success
 * @retval EINVAL	If @p fmt contains unrecognized format string
 *			specifiers.
 * @retval ENOMEM	If the @p outp is non-NULL, and the provided @p olen
 *			is too small to hold the encoded value.
 * @retval EFTYPE	If value coercion from @p inp to a string value via
 *			@p fmt is unsupported.
 * @retval ERANGE	If value coercion of @p value would overflow (or
 *			underflow) the representation defined by @p fmt.
 */
int
bhnd_nvram_value_printf(const char *fmt, const void *inp, size_t ilen,
    bhnd_nvram_type itype, char *outp, size_t *olen, ...)
{
	va_list	ap;
	int	error;

	va_start(ap, olen);
	error = bhnd_nvram_value_vprintf(fmt, inp, ilen, itype, outp, olen, ap);
	va_end(ap);

	return (error);
}

/**
 * Format a string representation of @p inp using @p fmt, with, writing the
 * result to @p outp.
 *
 * Refer to bhnd_nvram_val_vprintf() for full format string documentation.
 *
 * @param		fmt	The format string.
 * @param		inp	The value to be formatted.
 * @param		ilen	The size of @p inp, in bytes.
 * @param		itype	The type of @p inp.
 * @param[out]		outp	On success, the string value will be written to
 *				this buffer. This argment may be NULL if the
 *				value is not desired.
 * @param[in,out]	olen	The capacity of @p outp. On success, will be set
 *				to the actual size of the formatted string.
 * @param		ap	Argument list.
 *
 * @retval 0		success
 * @retval EINVAL	If @p fmt contains unrecognized format string
 *			specifiers.
 * @retval ENOMEM	If the @p outp is non-NULL, and the provided @p olen
 *			is too small to hold the encoded value.
 * @retval EFTYPE	If value coercion from @p inp to a string value via
 *			@p fmt is unsupported.
 * @retval ERANGE	If value coercion of @p value would overflow (or
 *			underflow) the representation defined by @p fmt.
 */
int
bhnd_nvram_value_vprintf(const char *fmt, const void *inp, size_t ilen,
    bhnd_nvram_type itype, char *outp, size_t *olen, va_list ap)
{
	bhnd_nvram_val	val;
	int		error;

	/* Map input buffer as a value instance */
	error = bhnd_nvram_val_init(&val, NULL, inp, ilen, itype,
	    BHND_NVRAM_VAL_BORROW_DATA);
	if (error)
		return (error);

	/* Attempt to format the value */
	error = bhnd_nvram_val_vprintf(&val, fmt, outp, olen, ap);

	/* Clean up */
	bhnd_nvram_val_release(&val);
	return (error);
}

/* used by bhnd_nvram_find_vardefn() */
static int
bhnd_nvram_find_vardefn_compare(const void *key, const void *rhs)
{
	const struct bhnd_nvram_vardefn *r = rhs;

	return (strcmp((const char *)key, r->name));
}

/**
 * Find and return the variable definition for @p varname, if any.
 * 
 * @param varname variable name
 * 
 * @retval bhnd_nvram_vardefn If a valid definition for @p varname is found.
 * @retval NULL If no definition for @p varname is found. 
 */
const struct bhnd_nvram_vardefn *
bhnd_nvram_find_vardefn(const char *varname)
{
	return (bsearch(varname, bhnd_nvram_vardefns, bhnd_nvram_num_vardefns,
	    sizeof(bhnd_nvram_vardefns[0]), bhnd_nvram_find_vardefn_compare));
}

/**
 * Return the variable ID for a variable definition.
 * 
 * @param defn Variable definition previously returned by
 * bhnd_nvram_find_vardefn() or bhnd_nvram_get_vardefn().
 */
size_t
bhnd_nvram_get_vardefn_id(const struct bhnd_nvram_vardefn *defn)
{
	BHND_NV_ASSERT(
	    defn >= bhnd_nvram_vardefns &&
	    defn <= &bhnd_nvram_vardefns[bhnd_nvram_num_vardefns-1],
	    ("invalid variable definition pointer %p", defn));

	return (defn - bhnd_nvram_vardefns);
}

/**
 * Return the variable definition with the given @p id, or NULL
 * if no such variable ID is defined.
 * 
 * @param id variable ID.
 *
 * @retval bhnd_nvram_vardefn If a valid definition for @p id is found.
 * @retval NULL If no definition for @p id is found. 
 */
const struct bhnd_nvram_vardefn *
bhnd_nvram_get_vardefn(size_t id)
{
	if (id >= bhnd_nvram_num_vardefns)
		return (NULL);

	return (&bhnd_nvram_vardefns[id]);
}

/**
 * Validate an NVRAM variable name.
 * 
 * Scans for special characters (path delimiters, value delimiters, path
 * alias prefixes), returning false if the given name cannot be used
 * as a relative NVRAM key.
 * 
 * @param name A relative NVRAM variable name to validate.
 * @param name_len The length of @p name, in bytes.
 * 
 * @retval true If @p name is a valid relative NVRAM key.
 * @retval false If @p name should not be used as a relative NVRAM key.
 */
bool
bhnd_nvram_validate_name(const char *name, size_t name_len)
{
	size_t limit;

	limit = strnlen(name, name_len);
	if (limit == 0)
		return (false);

	/* Disallow path alias prefixes ([0-9]+:.*) */
	if (limit >= 2 && bhnd_nv_isdigit(*name)) {
		for (const char *p = name; (size_t)(p - name) < limit; p++) {
			if (bhnd_nv_isdigit(*p))
				continue;
			else if (*p == ':')
				return (false);
			else
				break;
		}
	}

	/* Scan for special characters */
	for (const char *p = name; (size_t)(p - name) < limit; p++) {
		switch (*p) {
		case '/':	/* path delimiter */
		case '=':	/* key=value delimiter */
			return (false);

		default:
			if (!isascii(*p) || bhnd_nv_isspace(*p))
				return (false);
		}
	}

	return (true);
}

/**
 * Coerce value @p inp of type @p itype to @p otype, writing the
 * result to @p outp.
 *
 * @param		inp	The value to be coerced.
 * @param		ilen	The size of @p inp, in bytes.
 * @param		itype	The base data type of @p inp.
 * @param[out]		outp	On success, the value will be written to this 
 *				buffer. This argment may be NULL if the value
 *				is not desired.
 * @param[in,out]	olen	The capacity of @p outp. On success, will be set
 *				to the actual size of the requested value.
 * @param		otype	The data type to be written to @p outp.
 *
 * @retval 0		success
 * @retval ENOMEM	If @p outp is non-NULL and a buffer of @p olen is too
 *			small to hold the requested value.
 * @retval EFTYPE	If the variable data cannot be coerced to @p otype.
 * @retval ERANGE	If value coercion would overflow @p otype.
 */
int
bhnd_nvram_value_coerce(const void *inp, size_t ilen, bhnd_nvram_type itype,
    void *outp, size_t *olen, bhnd_nvram_type otype)
{
	bhnd_nvram_val	val;
	int		error;

	/* Wrap input buffer in a value instance */
	error = bhnd_nvram_val_init(&val, NULL, inp, ilen,
	    itype, BHND_NVRAM_VAL_BORROW_DATA|BHND_NVRAM_VAL_FIXED);
	if (error)
		return (error);

	/* Try to encode as requested type */
	error = bhnd_nvram_val_encode(&val, outp, olen, otype);

	/* Clean up and return error */
	bhnd_nvram_val_release(&val);
	return (error);
}

/**
 * Parses the string in the optionally NUL-terminated @p str to as an integer
 * value of @p otype, accepting any integer format supported by the standard
 * strtoul().
 * 
 * - Any leading whitespace in @p str -- as defined by the equivalent of
 *   calling isspace_l() with an ASCII locale -- will be ignored.
 * - A @p str may be prefixed with a single optional '+' or '-' sign denoting
 *   signedness.
 * - A hexadecimal @p str may include an '0x' or '0X' prefix, denoting that a
 *   base 16 integer follows.
 * - An octal @p str may include a '0' prefix, denoting that an octal integer
 *   follows.
 * 
 * If a @p base of 0 is specified, the base will be determined according
 * to the string's initial prefix, as per strtoul()'s documented behavior.
 *
 * When parsing a base 16 integer to a signed representation, if no explicit
 * sign prefix is given, the string will be parsed as the raw two's complement
 * representation of the signed integer value.
 *
 * @param		str	The string to be parsed.
 * @param		maxlen	The maximum number of bytes to be read in
 *				@p str.
 * @param		base	The input string's base (2-36), or 0.
 * @param[out]		nbytes	On success or failure, will be set to the total
 *				number of parsed bytes. If the total number of
 *				bytes is not desired, a NULL pointer may be
 *				provided.
 * @param[out]		outp	On success, the parsed integer value will be
 *				written to @p outp. This argment may be NULL if
 *				the value is not desired.
 * @param[in,out]	olen	The capacity of @p outp. On success, will be set
 *				to the actual size of the requested value.
 * @param		otype	The integer type to be parsed.
 *
 * @retval 0		success
 * @retval EINVAL	if an invalid @p base is specified.
 * @retval EINVAL	if an unsupported (or non-integer) @p otype is
 *			specified.
 * @retval ENOMEM	If @p outp is non-NULL and a buffer of @p olen is too
 *			small to hold the requested value.
 * @retval EFTYPE	if @p str cannot be parsed as an integer of @p base.
 * @retval ERANGE	If the integer parsed from @p str is too large to be
 *			represented as a value of @p otype.
 */
int
bhnd_nvram_parse_int(const char *str, size_t maxlen,  u_int base,
    size_t *nbytes, void *outp, size_t *olen, bhnd_nvram_type otype)
{
	uint64_t	value;
	uint64_t	carry_max, value_max;
	uint64_t	type_max;
	size_t		limit, local_nbytes;
	size_t		ndigits;
	bool		negative, sign, twos_compl;

	/* Must be an integer type */
	if (!bhnd_nvram_is_int_type(otype))
		return (EINVAL);

	/* Determine output byte limit */
	if (outp != NULL)
		limit = *olen;
	else
		limit = 0;

	/* We always need a byte count. If the caller provides a NULL nbytes,
	 * track our position in a stack variable */
	if (nbytes == NULL)
		nbytes = &local_nbytes;

	value = 0;
	ndigits = 0;
	*nbytes = 0;
	negative = false;
	sign = false;

	/* Validate the specified base */
	if (base != 0 && !(base >= 2 && base <= 36))
		return (EINVAL);

	/* Skip any leading whitespace */
	for (; *nbytes < maxlen; (*nbytes)++) {
		if (!bhnd_nv_isspace(str[*nbytes]))
			break;
	}

	/* Empty string? */
	if (*nbytes == maxlen)
		return (EFTYPE);

	/* Parse and skip sign */
	if (str[*nbytes] == '-') {
		negative = true;
		sign = true;
		(*nbytes)++;
	} else if (str[*nbytes] == '+') {
		sign = true;
		(*nbytes)++;
	}

	/* Truncated after sign character? */
	if (*nbytes == maxlen)
		return (EFTYPE);

	/* Identify (or validate) hex base, skipping 0x/0X prefix */
	if (base == 16 || base == 0) {
		/* Check for (and skip) 0x/0X prefix */
		if (maxlen - *nbytes >= 2 && str[*nbytes] == '0' &&
		    (str[*nbytes+1] == 'x' || str[*nbytes+1] == 'X'))
		{
			base = 16;
			(*nbytes) += 2;
		}
	}

	/* Truncated after hex prefix? */
	if (*nbytes == maxlen)
		return (EFTYPE);

	/* Differentiate decimal/octal by looking for a leading 0 */
	if (base == 0) {
		if (str[*nbytes] == '0') {
			base = 8;
		} else {
			base = 10;
		}
	}

	/* Only enable twos-compliment signed integer parsing enabled if the
	 * input is base 16, and no explicit sign prefix was provided */
	if (!sign && base == 16)
		twos_compl = true;
	else
		twos_compl = false;

	/* Determine the maximum value representable by the requested type */
	switch (otype) {
	case BHND_NVRAM_TYPE_CHAR:
	case BHND_NVRAM_TYPE_UINT8:
		type_max = (uint64_t)UINT8_MAX;
		break;
	case BHND_NVRAM_TYPE_UINT16:
		type_max = (uint64_t)UINT16_MAX;
		break;
	case BHND_NVRAM_TYPE_UINT32:
		type_max = (uint64_t)UINT32_MAX;
		break;
	case BHND_NVRAM_TYPE_UINT64:
		type_max = (uint64_t)UINT64_MAX;
		break;

	case BHND_NVRAM_TYPE_INT8:
		if (twos_compl)
			type_max = (uint64_t)UINT8_MAX;
		else if (negative)
			type_max = -(uint64_t)INT8_MIN;
		else
			type_max = (uint64_t)INT8_MAX;
		break;

	case BHND_NVRAM_TYPE_INT16:
		if (twos_compl)
			type_max = (uint64_t)UINT16_MAX;
		else if (negative)
			type_max = -(uint64_t)INT16_MIN;
		else
			type_max = (uint64_t)INT16_MAX;
		break;

	case BHND_NVRAM_TYPE_INT32:
		if (twos_compl)
			type_max = (uint64_t)UINT32_MAX;
		else if (negative)
			type_max = -(uint64_t)INT32_MIN;
		else
			type_max = (uint64_t)INT32_MAX;
		break;

	case BHND_NVRAM_TYPE_INT64:
		if (twos_compl)
			type_max = (uint64_t)UINT64_MAX;
		else if (negative)
			type_max = -(uint64_t)INT64_MIN;
		else
			type_max = (uint64_t)INT64_MAX;
		break;

	default:
		BHND_NV_LOG("unsupported integer type: %d\n", otype);
		return (EINVAL);
	}

	/* The maximum value after which an additional carry would overflow */
	value_max = type_max / (uint64_t)base;

	/* The maximum carry value given a value equal to value_max */
	carry_max = type_max % (uint64_t)base;

	/* Consume input until we hit maxlen or a non-digit character */
	for (; *nbytes < maxlen; (*nbytes)++) {
		u_long	carry;
		char	c;

		/* Parse carry value */
		c = str[*nbytes];
		if (bhnd_nv_isdigit(c)) {
			carry = c - '0';
		} else if (bhnd_nv_isxdigit(c)) {
			if (bhnd_nv_isupper(c))
				carry = (c - 'A') + 10;
			else
				carry = (c - 'a') + 10;
		} else {
			/* Hit first non-digit character */
			break;
		}

		/* If carry is outside the base, it's not a valid digit
		 * in the current parse context; consider it a non-digit
		 * character */
		if (carry >= (uint64_t)base)
			break;

		/* Increment count of parsed digits */
		ndigits++;

		if (value > value_max) {
			/* -Any- carry value would overflow */
			return (ERANGE);
		} else if (value == value_max && carry > carry_max) {
			/* -This- carry value would overflow */
			return (ERANGE);
		}

		value *= (uint64_t)base;
		value += carry;
	}

	/* If we hit a non-digit character before parsing the first digit,
	 * we hit an empty integer string. */
	if (ndigits == 0)
		return (EFTYPE);

	if (negative)
		value = -value;

	/* Provide (and verify) required length */
	*olen = bhnd_nvram_value_size(otype, NULL, 0, 1);
	if (outp == NULL)
		return (0);
	else if (limit < *olen)
		return (ENOMEM);

	/* Provide result */
	switch (otype) {
	case BHND_NVRAM_TYPE_CHAR:
	case BHND_NVRAM_TYPE_UINT8:
		*(uint8_t *)outp = (uint8_t)value;
		break;
	case BHND_NVRAM_TYPE_UINT16:
		*(uint16_t *)outp = (uint16_t)value;
		break;
	case BHND_NVRAM_TYPE_UINT32:
		*(uint32_t *)outp = (uint32_t)value;
		break;
	case BHND_NVRAM_TYPE_UINT64:
		*(uint64_t *)outp = (uint64_t)value;
		break;

	case BHND_NVRAM_TYPE_INT8:
		*(int8_t *)outp = (int8_t)(int64_t)value;
		break;
	case BHND_NVRAM_TYPE_INT16:
		*(int16_t *)outp = (int16_t)(int64_t)value;
		break;
	case BHND_NVRAM_TYPE_INT32:
		*(int32_t *)outp = (int32_t)(int64_t)value;
		break;
	case BHND_NVRAM_TYPE_INT64:
		*(int64_t *)outp = (int64_t)value;
		break;
	default:
		/* unreachable */
		BHND_NV_PANIC("unhandled type %d\n", otype);
	}

	return (0);
}

/**
 * Parse a 'name=value' string.
 * 
 * @param env The string to be parsed.
 * @param env_len The length of @p envp.
 * @param delim The delimiter used in @p envp. This will generally be '='.
 * @param[out] name If not NULL, a pointer to the name string. This argument
 * may be NULL.
 * @param[out] name_len On success, the length of the name substring. This
 * argument may be NULL.
 * @param[out] value On success, a pointer to the value substring. This argument
 * may be NULL.
 * @param[out] value_len On success, the length of the value substring. This
 * argument may be NULL.
 * 
 * @retval 0 success
 * @retval EINVAL if parsing @p envp fails.
 */
int
bhnd_nvram_parse_env(const char *env, size_t env_len, char delim,
    const char **name, size_t *name_len, const char **value, size_t *value_len)
{
	const char *p;

	/* Name */
	if ((p = memchr(env, delim, env_len)) == NULL) {
		BHND_NV_LOG("delimiter '%c' not found in '%.*s'\n", delim,
		    BHND_NV_PRINT_WIDTH(env_len), env);
		return (EINVAL);
	}

	/* Name */
	if (name != NULL)
		*name = env;
	if (name_len != NULL)
		*name_len = p - env;

	/* Skip delim */
	p++;

	/* Value */
	if (value != NULL)
		*value = p;
	if (value_len != NULL)
		*value_len = env_len - (p - env);

	return (0);
}


/**
 * Parse a field value, returning the actual pointer to the first
 * non-whitespace character and the total size of the field.
 *
 * @param[in,out] inp The field string to parse. Will be updated to point
 * at the first non-whitespace character found.
 * @param ilen The length of @p inp, in bytes.
 * @param delim The field delimiter to search for.
 *
 * @return Returns the actual size of the field data.
 */
size_t
bhnd_nvram_parse_field(const char **inp, size_t ilen, char delim)
{
	const char	*p, *sp;
	
	/* Skip any leading whitespace */
	for (sp = *inp; (size_t)(sp-*inp) < ilen && bhnd_nv_isspace(*sp); sp++)
		continue;
	
	*inp = sp;
	
	/* Find the last field character */
	for (p = *inp; (size_t)(p - *inp) < ilen; p++) {
		if (*p == delim || *p == '\0')
			break;
	}
	
	return (p - *inp);
}

/**
 * Parse a field value, returning the actual pointer to the first
 * non-whitespace character and the total size of the field, minus
 * any trailing whitespace.
 *
 * @param[in,out] inp The field string to parse. Will be updated to point
 * at the first non-whitespace character found.
 * @param ilen The length of the parsed field, in bytes, excluding the
 * field elimiter and any trailing whitespace.
 * @param delim The field delimiter to search for.
 *
 * @return Returns the actual size of the field data.
 */
size_t
bhnd_nvram_trim_field(const char **inp, size_t ilen, char delim)
{
	const char	*sp;
	size_t		 plen;
	
	plen = bhnd_nvram_parse_field(inp, ilen, delim);
	
	/* Trim trailing whitespace */
	sp = *inp;
	while (plen > 0) {
		if (!bhnd_nv_isspace(*(sp + plen - 1)))
			break;
		
		plen--;
	}
	
	return (plen);
}
