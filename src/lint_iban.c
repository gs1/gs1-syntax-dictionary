/*
 * GS1 Barcode Syntax Dictionary. Copyright (c) 2022-2024 GS1 AISBL.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/**
 * @file lint_iban.c
 *
 * @brief The `iban` linter ensures that the data conforms to the format
 * required for an International Bank Account Number (IBAN) number.
 *
 * @remark The format for an IBAN is specified by [ISO 13616-1: Financial services - International bank account number (IBAN) - Part 1: Structure of the IBAN](https://www.iso.org/standard/81090.html).
 *
 */


#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "gs1syntaxdictionary.h"
#include "gs1syntaxdictionary-utils.h"


#ifndef IBAN_MIN_LENGTH
#define IBAN_MIN_LENGTH 10  ///< No clear minimum length; sufficient for check characters.
#endif


/**
 * Used to validate that an AI component conforms to the format required for an
 * IBAN.
 *
 * @param [in] data Pointer to the null-terminated data to be linted. Must not
 *                  be `NULL`.
 * @param [out] err_pos To facilitate error highlighting, the start position of
 *                      the bad data is written to this pointer, if not `NULL`.
 * @param [out] err_len The length of the bad data is written to this pointer, if
 *                      not `NULL`.
 *
 * @return #GS1_LINTER_OK if okay.
 * @return #GS1_LINTER_INCORRECT_IBAN_CHECKSUM if the IBAN checksum is
 *         incorrect for the data.
 * @return #GS1_LINTER_IBAN_TOO_SHORT if the data is too short to be an IBAN.
 * @return #GS1_LINTER_INVALID_IBAN_CHARACTER if the data contains a character
 *         that isn't permissible within an IBAN.
 * @return #GS1_LINTER_ILLEGAL_IBAN_COUNTRY_CODE if the leading two characters
 *         are not a valid ISO 3166 alpha-2 country code.
 *
 */
GS1_SYNTAX_DICTIONARY_API gs1_lint_err_t gs1_lint_iban(const char* const data, size_t* const err_pos, size_t* const err_len)
{

	char cc[3] = {0};
	gs1_lint_err_t ret;
	size_t len, pos;
	const char *p;
	unsigned int csum;

	/*
	 * IBAN character checksum weights (0 = invalid; weight off by 1)
	 *
	 */
	static const unsigned char iban_weights[256] = {
		['0'] = 1, ['1'] = 2, ['2'] = 3, ['3'] = 4, ['4'] = 5,
		['5'] = 6, ['6'] = 7, ['7'] = 8, ['8'] = 9, ['9'] = 10,
		['A'] = 11, ['B'] = 12, ['C'] = 13, ['D'] = 14, ['E'] = 15,
		['F'] = 16, ['G'] = 17, ['H'] = 18, ['I'] = 19, ['J'] = 20,
		['K'] = 21, ['L'] = 22, ['M'] = 23, ['N'] = 24, ['O'] = 25,
		['P'] = 26, ['Q'] = 27, ['R'] = 28, ['S'] = 29, ['T'] = 30,
		['U'] = 31, ['V'] = 32, ['W'] = 33, ['X'] = 34, ['Y'] = 35, ['Z'] = 36
	};

	assert(data);

	len = strlen(data);

	if (GS1_LINTER_UNLIKELY(len <= IBAN_MIN_LENGTH))
		GS1_LINTER_RETURN_ERROR(
			GS1_LINTER_IBAN_TOO_SHORT,
			0,
			len
		);

	/*
	 *  The first two characters must be an ISO 3166 alpha-2 country code.
	 *
	 */
	strncpy(cc, data, 2);
	ret = gs1_lint_iso3166alpha2(cc, err_pos, err_len);
	assert(ret == GS1_LINTER_OK || ret == GS1_LINTER_NOT_ISO3166_ALPHA2);
	if (GS1_LINTER_UNLIKELY(ret == GS1_LINTER_NOT_ISO3166_ALPHA2))
		GS1_LINTER_RETURN_ERROR(
			GS1_LINTER_ILLEGAL_IBAN_COUNTRY_CODE,
			0,
			2
		);

	/*
	 * Next two character much be within set of valid IBAN characters.
	 *
	 */
	for (pos = 2; pos < 4; pos++) {
		if (GS1_LINTER_UNLIKELY(iban_weights[(unsigned char)data[pos]] == 0))
			GS1_LINTER_RETURN_ERROR(
				GS1_LINTER_INVALID_IBAN_CHARACTER,
				pos,
				1
			);
	}

	/*
	 * Compute the IBAN checksum as the sum of digits, with characters
	 * converted to digits (A => 10; B => 11), starting with the forth
	 * character and wrapping at the end.
	 */
	csum = 0;
	p = data + 4;
	do {
		unsigned char weight = iban_weights[(unsigned char)*p];

		if (GS1_LINTER_UNLIKELY(weight == 0))
			GS1_LINTER_RETURN_ERROR(
				GS1_LINTER_INVALID_IBAN_CHARACTER,
				(size_t)(p - data),
				1
			);

		csum *= weight <= 10 ? 10 : 100;
		csum += weight - 1;		/* Compensate off by 1 */
		csum %= 97;

		/*
		 * Next character, wrapping at the end.
		 */
		if (++p == data + len)
			p = data;

	} while (p != data + 4);

	/*
	 * Sum (mod 97) is 1 for correctly formatted IBANs.
	 *
	 */
	if (GS1_LINTER_UNLIKELY(csum != 1))
		GS1_LINTER_RETURN_ERROR(
			GS1_LINTER_INCORRECT_IBAN_CHECKSUM,
			2,
			2
		);

	GS1_LINTER_RETURN_OK;

}


#ifdef UNIT_TESTS

#include "unittest.h"

void test_lint_iban(void)
{

	UNIT_TEST_PASS(gs1_lint_iban, "FR7630006000011234567890189");
	UNIT_TEST_PASS(gs1_lint_iban, "DE91100000000123456789");
	UNIT_TEST_PASS(gs1_lint_iban, "GR9608100010000001234567890");
	UNIT_TEST_PASS(gs1_lint_iban, "MU43BOMM0101123456789101000MUR");
	UNIT_TEST_PASS(gs1_lint_iban, "PK70BANK0000123456789000");
	UNIT_TEST_PASS(gs1_lint_iban, "PL10105000997603123456789123");
	UNIT_TEST_PASS(gs1_lint_iban, "RO09BCYP0000001234567890");
	UNIT_TEST_PASS(gs1_lint_iban, "SA4420000001234567891234");
	UNIT_TEST_PASS(gs1_lint_iban, "ES7921000813610123456789");
	UNIT_TEST_PASS(gs1_lint_iban, "CH5604835012345678009");
	UNIT_TEST_PASS(gs1_lint_iban, "GB98MIDL07009312345678");

	UNIT_TEST_PASS(gs1_lint_iban, "BE71096123456769");
	UNIT_TEST_FAIL(gs1_lint_iban, "BE71096123456760", GS1_LINTER_INCORRECT_IBAN_CHECKSUM, "BE*71*096123456760");

	UNIT_TEST_PASS(gs1_lint_iban, "LC14BOSL123456789012345678901234");
	UNIT_TEST_FAIL(gs1_lint_iban, "LC14BOSL123456789012345678901230", GS1_LINTER_INCORRECT_IBAN_CHECKSUM, "LC*14*BOSL123456789012345678901230");

	UNIT_TEST_FAIL(gs1_lint_iban, "", GS1_LINTER_IBAN_TOO_SHORT, "**");
	UNIT_TEST_FAIL(gs1_lint_iban, "B", GS1_LINTER_IBAN_TOO_SHORT, "*B*");
	UNIT_TEST_FAIL(gs1_lint_iban, "BE", GS1_LINTER_IBAN_TOO_SHORT, "*BE*");
	UNIT_TEST_FAIL(gs1_lint_iban, "BE7", GS1_LINTER_IBAN_TOO_SHORT, "*BE7*");
	UNIT_TEST_FAIL(gs1_lint_iban, "BE71", GS1_LINTER_IBAN_TOO_SHORT, "*BE71*");

	UNIT_TEST_FAIL(gs1_lint_iban, "LC14BOSLxx3456789012345678901230", GS1_LINTER_INVALID_IBAN_CHARACTER, "LC14BOSL*x*x3456789012345678901230");

	UNIT_TEST_FAIL(gs1_lint_iban, "XX361234567890", GS1_LINTER_ILLEGAL_IBAN_COUNTRY_CODE, "*XX*361234567890");

}

#endif  /* UNIT_TESTS */
