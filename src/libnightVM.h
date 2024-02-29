#ifndef NVM_LIBNIGHTVM_H
#define NVM_LIBNIGHTVM_H

#include "implementation.h"
#include "opcodes.h"

#define READBUF_SIZE 1048576

extern enum error read_esff23(char *file, void **code, ysm_ui *code_alignment, ysm_ui *heap_alignment, ysm_l *reg);

extern enum error read_esff23x(char *file, void **code, ysm_ui *code_alignment, ysm_ui *heap_alignment, ysm_l *reg);

extern void close_opened_libs(char **opened_libs);

extern enum error eval(int argc, char **argv, ysm_l *stack, void **code, ysm_ui code_align, void **heap, ysm_ui heap_align, ysm_l *reg, ysm_l *call_stack, unsigned int *exit_status, enum load_type ltype, char **opened_libs, size_t *lib_pt, size_t *sym_pt, char **lib_names, char **sym_names);

extern void free_lib_names(char **lib_names, size_t lib_size);

extern void free_sym_names(char **sym_names, size_t sym_size);

#endif
