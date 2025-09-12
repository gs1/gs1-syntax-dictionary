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
 * @file lint_pcenc.c
 *
 * @brief The `pcenc` linter ensures that the data has correct
 * percent-encoding.
 *
 * @remark Percent-encoding is defined by [RFC 3986: Uniform Resource Identifier (URI): Generic Syntax](https://www.rfc-editor.org/rfc/rfc3986.html) section "2.1. Percent-Encoding".
 *
 */


#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "gs1syntaxdictionary.h"
#include "gs1syntaxdictionary-utils.h"


/**
 * Used to ensure that an AI component conforms with correct percent encoding.
 *
 * @param [in] data Pointer to the data to be linted. Must not be `NULL`.
 * @param [in] data_len Length of the data to be linted.
 * @param [out] err_pos To facilitate error highlighting, the start position of
 *                      the bad data is written to this pointer, if not `NULL`.
 * @param [out] err_len The length of the bad data is written to this pointer, if
 *                      not `NULL`.
 *
 * @return #GS1_LINTER_OK if okay.
 * @return #GS1_LINTER_INVALID_PERCENT_SEQUENCE if the data contains an
 *         invalid percent sequence.
 *
 */
GS1_SYNTAX_DICTIONARY_API gs1_lint_err_t gs1_lint_pcenc(const char* const data, size_t data_len, size_t* const err_pos, size_t* const err_len)
{

	size_t pos;

	assert(data);

	/*
	 * Find each instance of "%" in the data and ensure that there are at
	 * least two following characters and that these two characters
	 * represent a hex value.
	 *
	 */
	for (pos = 0; pos < data_len; pos++) {
		if (data[pos] == '%') {
			if (GS1_LINTER_UNLIKELY(pos + 2 >= data_len)) {
				GS1_LINTER_RETURN_ERROR(
					GS1_LINTER_INVALID_PERCENT_SEQUENCE,
					pos,
					data_len - pos
				);
			}

			if (GS1_LINTER_UNLIKELY(!isxdigit((int)data[pos + 1]) || !isxdigit((int)data[pos + 2])))
				GS1_LINTER_RETURN_ERROR(
					GS1_LINTER_INVALID_PERCENT_SEQUENCE,
					pos,
					3
				);

			pos += 2;  /* Skip the two hex digits, loop will increment past '%' */
		}
	}

	GS1_LINTER_RETURN_OK;

}


#ifdef UNIT_TESTS

#include "unittest.h"

void test_lint_pcenc(void)
{

	UNIT_TEST_PASS(gs1_lint_pcenc, "");
	UNIT_TEST_PASS(gs1_lint_pcenc, "A");
	UNIT_TEST_PASS(gs1_lint_pcenc, "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHI");

	UNIT_TEST_PASS(gs1_lint_pcenc, "%00");
	UNIT_TEST_PASS(gs1_lint_pcenc, "%FF");
	UNIT_TEST_PASS(gs1_lint_pcenc, "%Ff");
	UNIT_TEST_PASS(gs1_lint_pcenc, "%fF");
	UNIT_TEST_PASS(gs1_lint_pcenc, "%ff");

	UNIT_TEST_FAIL(gs1_lint_pcenc, "%fg", GS1_LINTER_INVALID_PERCENT_SEQUENCE, "*%fg*");
	UNIT_TEST_FAIL(gs1_lint_pcenc, "%gf", GS1_LINTER_INVALID_PERCENT_SEQUENCE, "*%gf*");
	UNIT_TEST_FAIL(gs1_lint_pcenc, "%g",  GS1_LINTER_INVALID_PERCENT_SEQUENCE, "*%g*");

	UNIT_TEST_PASS(gs1_lint_pcenc, "%20");
	UNIT_TEST_PASS(gs1_lint_pcenc, "ABC%20");
	UNIT_TEST_PASS(gs1_lint_pcenc, "ABC%00");
	UNIT_TEST_PASS(gs1_lint_pcenc, "ABC%FF");
	UNIT_TEST_PASS(gs1_lint_pcenc, "ABC%ff");
	UNIT_TEST_FAIL(gs1_lint_pcenc, "ABC%fg", GS1_LINTER_INVALID_PERCENT_SEQUENCE, "ABC*%fg*");
	UNIT_TEST_FAIL(gs1_lint_pcenc, "ABC%gf", GS1_LINTER_INVALID_PERCENT_SEQUENCE, "ABC*%gf*");
	UNIT_TEST_FAIL(gs1_lint_pcenc, "ABC%g",  GS1_LINTER_INVALID_PERCENT_SEQUENCE, "ABC*%g*");

	UNIT_TEST_PASS(gs1_lint_pcenc, "ABC%20XYZ");

	UNIT_TEST_PASS(gs1_lint_pcenc, "ABC%00XYZ");
	UNIT_TEST_PASS(gs1_lint_pcenc, "ABC%FFXYZ");
	UNIT_TEST_PASS(gs1_lint_pcenc, "ABC%ffXYZ");

	UNIT_TEST_FAIL(gs1_lint_pcenc, "ABC%fgXYZ", GS1_LINTER_INVALID_PERCENT_SEQUENCE, "ABC*%fg*XYZ");
	UNIT_TEST_FAIL(gs1_lint_pcenc, "ABC%gfXYZ", GS1_LINTER_INVALID_PERCENT_SEQUENCE, "ABC*%gf*XYZ");
	UNIT_TEST_FAIL(gs1_lint_pcenc, "ABC%gXYZ",  GS1_LINTER_INVALID_PERCENT_SEQUENCE, "ABC*%gX*YZ");

	UNIT_TEST_PASS(gs1_lint_pcenc, "ABCDEFGHIJKLMNOPQRSTUVWXYZ%00+12345");

}

#endif  /* UNIT_TESTS */
