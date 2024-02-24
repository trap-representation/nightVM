#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <search.h>
#include <stdbool.h>
#include <math.h>

#include "nightVM.h"
#include "implementation.h"
#include "libnightVM.h"
#include "opcodes.h"

enum file_format {
	FFORM_ESFF23,
	FFORM_ESFF23X
};

static int throw_err(nightVM_l pc, nightVM_l sp, nightVM_l ia, nightVM_l lop, unsigned char code) {
	char *err_arr[]={"error: panic", "error: failed to open library", "error: failed to invoke symbol", "error: illegal instruction", "error: access outside of code", "implementation error: internal allocation failed", "implementation error: failed to open input", "implementation error: failed to read from input", "implementation error: input is not a valid ESXF", "implementation error: invalid ESXF magic number", "implementation error: unsupported ESFF used", "implementation error: failed to create hash table", "implementation error: failed to put entry into hash table"};

	fprintf(stderr,"%s (at %" PRINVML "; sp set to %" PRINVML "; ia set to %" PRINVML "; lop set to %" PRINVML ")\n", err_arr[code], pc, sp, ia, lop);

	return code;
}

static void *aligned_malloc(size_t alignment, size_t size) {
	if (size % alignment!=0) {
		size += (size + alignment - 1) - ((size + alignment - 1) % alignment) - size;
	}

	void *return_mem = aligned_alloc(alignment, size);

	return return_mem;
}

int main(int argc, char *argv[]) {
	nightVM_l *stack;
	void *code;
	void *heap;
	nightVM_l *call_stack;
	nightVM_ui code_alignment, heap_alignment;
	nightVM_l reg[REG_GPR7 + 1];

	char *in_file_name = "./a.esxf";
	bool randomize_code_base = false;

	int args_start = 0;

	enum file_format fformat = FFORM_ESFF23X;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
			i++;

			if (i == argc) {
				fprintf(stderr, "error: expected file [args...] after %s\n", argv[i - 1]);

				return 1;
			}
			else {
				in_file_name = argv[i];

				i++;
				args_start = i;

				for (; i < argc; i++) {
					if (strcmp(argv[i], "@-") == 0) {
						argv[i] = NULL;
						break;
					}
				}
			}
		}
		else if (strcmp(argv[i], "-rc") == 0 || strcmp(argv[i], "--randomize-code-base") == 0) {
			randomize_code_base = true;
		}
		else if (strcmp(argv[i], "-fform") == 0 || strcmp(argv[i], "--fileformat") == 0) {
			i++;

			if (i==argc) {
				fprintf(stderr,"implementation error: expected a fileformat after %s\n", argv[i - 1]);

				return 1;
			}
			else{
				if (strcmp(argv[i], "ESFF23") == 0) {
					fformat = FFORM_ESFF23;
				}
				else if (strcmp(argv[i],"ESFF23x") == 0) {
					fformat = FFORM_ESFF23X;
				}
				else{
					fprintf(stderr,"implementation error: not a valid fileformat %s\n", argv[i]);

					return 1;
				}
			}
		}
		else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--credits") == 0) {
			printf("\
					Copyright (C) 2023  Somdipto Chakraborty\n\n\
					\
					This program is free software: you can redistribute it and/or modify\n\
					it under the terms of the GNU General Public License as published by\n\
					the Free Software Foundation, either version 3 of the License, or\n\
					(at your option) any later version.\n\n\
					\
					This program is distributed in the hope that it will be useful,\n\
					but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
					MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n\
					GNU General Public License for more details.\n\n\
					\
					You should have received a copy of the GNU General Public License\n\
					along with this program.  If not, see <https://www.gnu.org/licenses/>.\n");

			return 0;
		}
		else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--about") == 0) {
			printf("\
					NightVM is an implementation of YSM.\n\
					This program is licensed under GPLv3 Copyright (C) 2023 Somdipto Chakraborty.\n");

			return 0;
		}
		else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			fprintf(stderr, "    --help (-h): display this message and quit\n");
			fprintf(stderr, "*   --input (-i) file [args...]: specify the input file (default: %s); optional arguments following the file are supplied to the loaded program\n", in_file_name);
			fprintf(stderr, "    --fileformat (-fform) fileformat: specify the input file format (default: ESFF23x)\n");
			fprintf(stderr, "    --randomize-code-base (-rc): randomize the base address of the code\n");
			fprintf(stderr, "    --credits (-c): display license information and quit\n");
			fprintf(stderr, "    --about (-a): display information about this implementation and quit\n\n");

			fprintf(stderr, "    in the aforementioned list, certain arguments that can be followed by a sequence of arguments are marked with an asterisk (*); those can end with @- to denote the end of the sequence\n");
			fprintf(stderr, "    for a normative version of this list, read the nightVM manual\n");
			return 0;
		}
		else{
			fprintf(stderr, "implementation error: %s is an invalid argument; use the --help (-h) argument for a list of valid arguments\n", argv[i]);

			return 1;
		}
	}

	enum error ret;
	if (fformat == FFORM_ESFF23) {
		if ((ret = read_esff23(in_file_name, &code, &code_alignment, &heap_alignment, reg)) != ERR_SUCCESS) {
			return throw_err(0, 0, 0, OP_NOP, ret - 1);
		}
	}
	else if (fformat == FFORM_ESFF23X) {
		if ((ret = read_esff23x(in_file_name, &code, &code_alignment, &heap_alignment, reg)) != ERR_SUCCESS) {
			return throw_err(0, 0, 0, OP_NOP, ret - 1);
		}
	}

	srand(time(NULL));

	nightVM_l code_base = 0;

	if (randomize_code_base) {
		void *nonrandomized_code = code;

		code_base = (rand() % MAX_CODE_BASE) + 1;

		if (code_base % _Alignof(nightVM_l) != 0) {
			code_base += (code_base + _Alignof(nightVM_l) - 1) - ((code_base + _Alignof(nightVM_l) - 1) % _Alignof(nightVM_l)) - code_base;
		}

		if ((code = aligned_malloc(code_alignment * sizeof(nightVM_uc), (reg[REG_CS] + code_base) * sizeof(nightVM_uc))) == NULL) {
			free(nonrandomized_code);

			return throw_err(0, 0, 0, OP_NOP, ERR_FAILED_ALLOCATION - 1);
		}

		memcpy(&((unsigned char *) code)[code_base], nonrandomized_code, reg[REG_CS]);

		reg[REG_CS] += code_base;
		reg[REG_PC] += code_base;

		free(nonrandomized_code);
	}

	reg[REG_CB] = code_base;

	if ((stack = aligned_malloc(_Alignof(nightVM_l), reg[REG_SSZ] * sizeof(nightVM_l))) == NULL) {
		free(code);

		return throw_err(0, 0, 0, OP_NOP, ERR_FAILED_ALLOCATION - 1);
	}

	if ((heap = aligned_malloc(heap_alignment * sizeof(nightVM_uc), reg[REG_HSZ] * sizeof(nightVM_uc))) == NULL) {
		free(code);
		free(stack);

		return throw_err(0, 0, 0, OP_NOP, ERR_FAILED_ALLOCATION - 1);
	}

	if ((call_stack = aligned_malloc(_Alignof(nightVM_l), CALLSTACK_SIZE * sizeof(nightVM_l))) == NULL) {
		free(code);
		free(stack);
		free(call_stack);

		return throw_err(0,0,0,OP_NOP,ERR_FAILED_ALLOCATION - 1);
	}

	char **opened_libs;
	if ((opened_libs = malloc(LIBS_N * sizeof(char *))) == NULL) {
		free(code);
		free(stack);
		free(call_stack);

		return throw_err(0, 0, 0, OP_NOP, ERR_FAILED_ALLOCATION - 1);
	}

	opened_libs[0]=NULL;

	size_t lib_pt=0;
	size_t sym_pt=0;

	char **lib_names;
	if ((lib_names=malloc(LIBS_N*sizeof(char *)))==NULL) {
		free(code);
		free(stack);
		free(call_stack);

		return throw_err(0, 0, 0, OP_NOP, ERR_FAILED_ALLOCATION - 1);
	}

	char **sym_names;
	if ((sym_names = malloc(SYMS_N * sizeof(char *))) == NULL) {
		free(code);
		free(stack);
		free(call_stack);
		free(lib_names);

		return throw_err(0, 0, 0, OP_NOP, ERR_FAILED_ALLOCATION - 1);
	}

	if (hcreate(LIBS_N)==0) {
		free(code);
		free(stack);
		free(call_stack);
		free(sym_names);
		free(lib_names);
		close_opened_libs(opened_libs);
		free(opened_libs);

		return throw_err(0, 0, 0, OP_NOP, ERR_FAILED_TO_CREATE_HASH_TABLE - 1);
	}

	reg[REG_SP]=0;
	reg[REG_IA]=0;
	reg[REG_CLP]=0;
	reg[REG_LOP]=OP_NOP;

	unsigned int exit_status;

	enum error exit_code = eval(argc-args_start, &argv[args_start], stack, &code, code_alignment, &heap, heap_alignment, reg, call_stack, &exit_status, LT_NEMBED, opened_libs, &lib_pt, &sym_pt, lib_names, sym_names);

	if (exit_status) {
		throw_err(reg[REG_PC], reg[REG_SP], reg[REG_IA], reg[REG_LOP], exit_code - 1);
	}

	hdestroy();
	free_sym_names(sym_names, sym_pt);
	free_lib_names(lib_names, lib_pt);
	close_opened_libs(opened_libs);
	free(opened_libs);
	free(code);
	free(call_stack);
	free(stack);
	free(heap);

	return exit_code;
}
