/**
 * GS1 Barcode Syntax Dictionary
 *
 * @author Copyright (c) 2022-2025 GS1 AISBL.
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "gs1syntaxdictionary.h"

#ifndef LINTER
#error
#endif

#define RUN_LINTER(lint,s,l,ep,el)	lint(s,l,ep,el)

#define MAX_DATA 4096


int LLVMFuzzerTestOneInput(const uint8_t* const buf, size_t len) {

	size_t err_pos, err_len;
	gs1_lint_err_t err;

	err = RUN_LINTER(LINTER, (char*)buf, len, &err_pos, &err_len);

	if (err != GS1_LINTER_OK) {
		assert(err_pos < len || err_pos == 0);
		assert(err_pos + err_len <= len);
	}

	return 0;

}
