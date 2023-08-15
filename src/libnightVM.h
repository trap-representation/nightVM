#ifndef NVM_LIBNIGHTVM_H
#define NVM_LIBNIGHTVM_H

#include "alloc.h"
#include "implementation.h"
#include "opcodes.h"

extern unsigned int read_file(char *file, void **code, nightVM_ui *code_alignment, nightVM_ui *heap_alignment, nightVM_l *reg);

extern void close_opened_libs(char **opened_libs);

extern unsigned int eval(int argc, char **argv, nightVM_l *stack, void **code, nightVM_ui code_align, void **heap, nightVM_ui heap_align, nightVM_l *reg, unsigned int *exit_status, unsigned int load_type, char **opened_libs, size_t *lib_pt, size_t *sym_pt, char **lib_names, char **sym_names);

extern void free_lib_names(char **lib_names, size_t lib_size);

extern void free_sym_names(char **sym_names, size_t sym_size);

#endif
