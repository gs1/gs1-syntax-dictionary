/*
 * GS1 Syntax Dictionary. Copyright (c) 2022-2024 GS1 AISBL.
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
 * @file gs1syntaxdictionary-utils.h
 *
 * @brief TODO This header contains the declarations for the reference linter
 * functions of type ::gs1_linter_t referred to in the [GS1 Syntax
 * Dictionary](https://ref.gs1.org/tools/gs1-barcode-syntax-resource/syntax-dictionary/),
 * as well as the list of all possible return codes with a description of the
 * indicated error conditions.
 *
 */

#ifndef GS1_SYNTAXDICTIONARY_UTILS_H
#define GS1_SYNTAXDICTIONARY_UTILS_H

#include <limits.h>


#define GS1_LINTER_BITFIELD_LOOKUP(needle, haystack) do {			\
	int w = CHAR_BIT * sizeof(haystack[0]);					\
	assert((size_t)(needle/w) < sizeof(haystack) / sizeof(haystack[0]));	\
	if (haystack[needle/w] & (1ull << (w-1) >> (needle%w)))			\
		valid = 1;							\
} while (0)


#define GS1_LINTER_BINARY_SEARCH(needle, haystack) do {		\
	size_t s = 0;						\
	size_t e = sizeof(haystack) / sizeof(haystack[0]);	\
	while (s < e) {						\
		const size_t m = s + (e - s) / 2;		\
		const int cmp = strcmp(haystack[m], needle);	\
		if (cmp < 0)					\
			s = m + 1;				\
		else if (cmp > 0)				\
			e = m;					\
		else {						\
			valid = 1;				\
			break;					\
		}						\
	}							\
} while (0)

#endif  /* GS1_SYNTAXDICTIONARY_UTILS_H */
