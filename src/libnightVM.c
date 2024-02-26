#define _XOPEN_SOURCE 700

#include <stdio.h> 
#include <math.h> 
#include <stdlib.h> 
#include <dlfcn.h> 
#include <string.h> 
#include <limits.h> 
#include <search.h> 

#include "libnightVM.h"

_Static_assert(_Alignof(nightVM_l) % _Alignof(nightVM_us) == 0 && _Alignof(nightVM_l) % _Alignof(nightVM_ui) == 0 && _Alignof(nightVM_l) % _Alignof(nightVM_s) == 0 && _Alignof(nightVM_l) % _Alignof(nightVM_i) == 0, "static assert failure in " __FILE__);

static void *aligned_malloc(size_t alignment, size_t size) {
  if (size % alignment != 0) {
    size += (size + alignment - 1) - ((size + alignment - 1) % alignment) - size;
  }

  void *return_mem = aligned_alloc(alignment, size);

  return return_mem;
}

static void *aligned_realloc(void *ptr, size_t alignment, size_t size, size_t prev_size) {
  void *new_mem = aligned_malloc(alignment, size);

  if (new_mem != NULL && ptr != NULL) {
    size_t size_to_copy = prev_size > size ? size : prev_size;
    memcpy(new_mem, ptr, size_to_copy); free(ptr);
  }

  return new_mem;
}

enum error read_esff23(char *file, void **code, nightVM_ui *code_alignment, nightVM_ui *heap_alignment, nightVM_l *reg) {
  FILE *fp;
  if ((fp = fopen(file, "rb")) == NULL) {
    return ERR_FAILED_TO_OPEN_FILE_FOR_READING;
  }

  nightVM_ui magic;
  if (fread(&magic, sizeof(nightVM_ui), 1, fp) < 1) {
    fclose(fp);
    return ERR_INVALID_FILE_FORMAT;
  }

  if (magic == 0xE3EFF) {
    nightVM_ui specification_version;
    if (fread(&specification_version, sizeof(nightVM_ui), 1, fp) < 1) {
      fclose(fp);
      return ERR_INVALID_FILE_FORMAT;
    }

    if (specification_version == 0x00023) {
      nightVM_uns uns;
      if (fread(code_alignment, sizeof(nightVM_ui), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }

      if (fread(heap_alignment, sizeof(nightVM_ui), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }

      if (fread(&uns, sizeof(nightVM_uns), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }
      reg[REG_HSZ] = uns;

      if (fread(&uns, sizeof(nightVM_uns), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }
      reg[REG_SSZ] = uns;

      if (fread(&uns, sizeof(nightVM_uns), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }
      reg[REG_PC] = uns;

      void *read_buf;
      if ((read_buf = malloc(READBUF_SIZE * sizeof(nightVM_uc))) == NULL) {
	fclose(fp);
	return ERR_FAILED_ALLOCATION;
      }

      reg[REG_CS] = 0;
      *code = NULL;

      size_t read;
      while ((read = fread(read_buf, sizeof(nightVM_uc), READBUF_SIZE, fp))) {
	if (ferror(fp)) {
	  free(read_buf);
	  free(*code);
	  fclose(fp);
	  return ERR_FAILED_TO_READ_FROM_FILE;
	}

	if ((*code = aligned_realloc(*code, *code_alignment, (reg[REG_CS] + read) * sizeof(nightVM_uc), reg[REG_CS])) == NULL) {
	  return ERR_FAILED_ALLOCATION;
	}

	memcpy(&((nightVM_c *) *code)[reg[REG_CS]], read_buf, read * sizeof(nightVM_uc));
	reg[REG_CS] += read;
      }

      if (ferror(fp)) {
	free(read_buf);
	free(*code);
	fclose(fp);
	return ERR_FAILED_TO_READ_FROM_FILE;
      }

      free(read_buf);
      fclose(fp);
    }
    else{
      fclose(fp);
      return ERR_INVALID_SPECIFICATION_VERSION;
    }
  }
  else{
    fclose(fp);
    return ERR_INVALID_MAGIC;
  }

  return ERR_SUCCESS;
}

enum error read_esff23x(char *file, void **code, nightVM_ui *code_alignment, nightVM_ui *heap_alignment, nightVM_l *reg) {
  FILE *fp;
  if ((fp = fopen(file, "rb")) == NULL) {
    return ERR_FAILED_TO_OPEN_FILE_FOR_READING;
  }

  nightVM_c bread;
  while (fread(&bread, sizeof(nightVM_uc), 1, fp)) {
    if (bread == 0) {
      break;
    }
  }

  if (ferror(fp)) {
    fclose(fp);
    return ERR_FAILED_TO_READ_FROM_FILE;
  }

  nightVM_ui magic;
  if (fread(&magic, sizeof(nightVM_ui), 1, fp) < 1) {
    fclose(fp);
    return ERR_INVALID_FILE_FORMAT;
  }

  if (magic == 0xE3EF6) {
    nightVM_ui specification_version;
    if (fread(&specification_version, sizeof(nightVM_ui), 1, fp) < 1) {
      fclose(fp);
      return ERR_INVALID_FILE_FORMAT;
    }

    if (specification_version == 0x00023) {
      nightVM_uns uns;
      if (fread(code_alignment, sizeof(nightVM_ui), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }

      if (fread(heap_alignment, sizeof(nightVM_ui), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }

      if (fread(&uns, sizeof(nightVM_uns), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }
      reg[REG_HSZ] = uns;

      if (fread(&uns, sizeof(nightVM_uns), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }
      reg[REG_SSZ] = uns;

      if (fread(&uns, sizeof(nightVM_uns), 1, fp) < 1) {
	fclose(fp);
	return ERR_INVALID_FILE_FORMAT;
      }
      reg[REG_PC] = uns;

      void *read_buf;
      if ((read_buf = malloc(READBUF_SIZE*sizeof(nightVM_uc))) == NULL) {
	fclose(fp);
	return ERR_FAILED_ALLOCATION;
      }

      reg[REG_CS] = 0;
      *code = NULL;

      size_t read;
      while ((read = fread(read_buf, sizeof(nightVM_uc), READBUF_SIZE, fp))) {
	if (ferror(fp)) {
	  free(read_buf);
	  free(*code);
	  fclose(fp);
	  return ERR_FAILED_TO_READ_FROM_FILE;
	}

	if ((*code = aligned_realloc(*code, *code_alignment, (reg[REG_CS] + read) * sizeof(nightVM_uc), reg[REG_CS])) == NULL) {
	  return ERR_FAILED_ALLOCATION;
	}

	memcpy(&((nightVM_c *) *code)[reg[REG_CS]], read_buf, read * sizeof(nightVM_uc));
	reg[REG_CS] += read;
      }

      if (ferror(fp)) {
	free(read_buf);
	free(*code);
	fclose(fp);
	return ERR_FAILED_TO_READ_FROM_FILE;
      }

      free(read_buf);
      fclose(fp);
    }
    else{
      fclose(fp);
      return ERR_INVALID_SPECIFICATION_VERSION;
    }
  }
  else{
    fclose(fp);
    return ERR_INVALID_MAGIC;
  }

  return ERR_SUCCESS;
}

void close_opened_libs(char **opened_libs) {
  for (size_t i = 0; opened_libs[i] != NULL; i++) {
    dlclose(opened_libs[i]);
  }
}

void free_lib_names(char **lib_names, size_t lib_size) {
  if (lib_names != NULL) {
    for (size_t i = 0; i < lib_size; i++) {
      free(lib_names[i]);
    }

    free(lib_names);
  }
}

void free_sym_names(char **sym_names, size_t sym_size) {
  if (sym_names != NULL) {
    for (size_t i = 0;i < sym_size; i++) {
      free(sym_names[i]);
    }

    free(sym_names);
  }
}

enum error eval(int argc, char **argv, nightVM_l *stack, void **code, nightVM_ui code_align, void **heap, nightVM_ui heap_align, nightVM_l *reg, nightVM_l *call_stack, unsigned int *exit_status, enum load_type ltype, char **opened_libs, size_t *lib_pt, size_t *sym_pt, char **lib_names, char **sym_names) {
  *exit_status = 0;
  void (*symbol)(int argc, char **argv, nightVM_l *stack, void **code, nightVM_ui code_align, void **heap, nightVM_ui heap_align, nightVM_l ssz, nightVM_l *hsz, nightVM_l *sp, nightVM_l *cs, nightVM_l pc, nightVM_l lop, nightVM_l *call_stack, nightVM_l clp, nightVM_l *gpr);

  /* initialization */

  nightVM_uc *code_uc = *code;
  nightVM_us *code_us = *code;
  nightVM_ui *code_ui = *code;
  nightVM_c *code_c = *code;
  nightVM_s *code_s = *code;
  nightVM_i *code_i = *code;
  nightVM_l *code_l = *code;
  nightVM_uc *heap_uc = *heap;
  nightVM_us *heap_us = *heap;
  nightVM_ui *heap_ui = *heap;
  nightVM_c *heap_c = *heap;
  nightVM_s *heap_s = *heap;
  nightVM_i *heap_i = *heap;
  nightVM_l *heap_l = *heap;

  /* --- */

  /* non-necessary for temporary storage */

  nightVM_uc uc, uc_1;
  nightVM_us us, us_1;
  nightVM_ui ui, ui_1;
  nightVM_uns uns, uns_1;
  nightVM_c c, c_1;
  nightVM_s s, s_1;
  nightVM_i i, i_1;
  nightVM_l l, l_1;
  nightVM_uc opcode;
  char resolved_name[PATH_MAX];
  char *str;
  ENTRY hash_table_entry;
  ENTRY *hash_table_entry_ret;

  /* --- */

  while (1) {
    if (reg[REG_PC] >= reg[REG_CS] || reg[REG_PC] < reg[REG_CB]) {
      *exit_status = 1;
      reg[REG_IA] = reg[REG_PC];
      return ERR_TRAP;
    }

    opcode = code_uc[reg[REG_PC]];

    if (reg[REG_LOP] == OP_RBS && opcode != OP_RBE) {
      continue;
    }

    reg[REG_LOP] = opcode;

    switch(opcode) {
    case OP_TRAP:
      *exit_status = 1;
      reg[REG_IA] = reg[REG_PC];
      return ERR_TRAP;

    case OP_HALT:
      return ERR_SUCCESS;

    case OP_HALTR:
      return stack[reg[REG_SP] - 1];

    case OP_RBS:
      reg[REG_PC]++;
      break;

    case OP_RBE:
      reg[REG_PC]++;
      break;

    case OP_STS:
      stack[stack[reg[REG_SP] - 1]] = stack[reg[REG_SP] - 2];
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_PUSHUC:
      reg[REG_PC]++;

      stack[reg[REG_SP]] = code_uc[reg[REG_PC]];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSHUS:
      reg[REG_PC]++;

      memcpy(&us, &code_uc[reg[REG_PC]], sizeof(nightVM_us));
      stack[reg[REG_SP]] = us;
      reg[REG_SP]++;

      reg[REG_PC] += SIZEOF_US;
      break;

    case OP_PUSHUI:
      reg[REG_PC]++;

      memcpy(&ui, &code_uc[reg[REG_PC]], sizeof(nightVM_ui));
      stack[reg[REG_SP]] = ui;
      reg[REG_SP]++;

      reg[REG_PC] += SIZEOF_UI;
      break;

    case OP_PUSHC:
      reg[REG_PC]++;

      stack[reg[REG_SP]] = ((nightVM_c *) *code)[reg[REG_PC]];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSHS:
      reg[REG_PC]++;

      memcpy(&s, &code_uc[reg[REG_PC]], sizeof(nightVM_s));
      stack[reg[REG_SP]] = s;
      reg[REG_SP]++;

      reg[REG_PC] += SIZEOF_S;
      break;

    case OP_PUSHI:
      reg[REG_PC]++;

      memcpy(&i, &code_uc[reg[REG_PC]], sizeof(nightVM_i));
      stack[reg[REG_SP]] = i;
      reg[REG_SP]++;

      reg[REG_PC] += SIZEOF_I;
      break;

    case OP_PUSHL:
      reg[REG_PC]++;

      memcpy(&stack[reg[REG_SP]], &code_uc[reg[REG_PC]], sizeof(nightVM_l));
      reg[REG_SP]++;

      reg[REG_PC] += SIZEOF_L;
      break;

    case OP_PUSHPC:
      stack[reg[REG_SP]] = reg[REG_PC];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSHCS:
      stack[reg[REG_SP]] = reg[REG_CS];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSHSP:
      stack[reg[REG_SP]] = reg[REG_SP];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSHLT:
      stack[reg[REG_SP]] = ltype;
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSHN1:
      stack[reg[REG_SP]] =  - 1;
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSH0:
      stack[reg[REG_SP]] = 0;
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSH1:
      stack[reg[REG_SP]] = 1;
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSH2:
      stack[reg[REG_SP]] = 2;
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSH3:
      stack[reg[REG_SP]] = 3;
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSH4:
      stack[reg[REG_SP]] = 4;
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSH5:
      stack[reg[REG_SP]] = 5;
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_PUSH7:
      stack[reg[REG_SP]] = 7;
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_ADDUC:
      stack[reg[REG_SP] - 2] = (nightVM_uc) (((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) + ((nightVM_uc) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ADDUS:
      stack[reg[REG_SP] - 2] = (nightVM_us) (((nightVM_us) stack[reg[REG_SP] - 1] + 0u) + ((nightVM_us) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ADDUI:
      stack[reg[REG_SP] - 2] = (nightVM_ui) (((nightVM_ui) stack[reg[REG_SP] - 1] + 0u) + ((nightVM_ui) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ADDC:
      stack[reg[REG_SP] - 2] = (nightVM_c) ((nightVM_c) stack[reg[REG_SP] - 1] + (nightVM_c) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ADDS:
      stack[reg[REG_SP] - 2] = (nightVM_s) ((nightVM_s) stack[reg[REG_SP] - 1] + (nightVM_s) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ADDI:
      stack[reg[REG_SP] - 2] = (nightVM_i) ((nightVM_i) stack[reg[REG_SP] - 1] + (nightVM_i) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ADDL:
      stack[reg[REG_SP] - 2] = stack[reg[REG_SP] - 1] + stack[reg[REG_SP] - 2];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_SUBUC:
      stack[reg[REG_SP] - 2] = (nightVM_uc) (((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) - ((nightVM_uc) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_SUBUS:
      stack[reg[REG_SP] - 2] = (nightVM_us) (((nightVM_us) stack[reg[REG_SP] - 1] + 0u) - ((nightVM_us) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_SUBUI:
      stack[reg[REG_SP] - 2] = (nightVM_ui) (((nightVM_ui) stack[reg[REG_SP] - 1] + 0u) - ((nightVM_ui) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_SUBC:
      stack[reg[REG_SP] - 2] = (nightVM_c) ((nightVM_c) stack[reg[REG_SP] - 1] - (nightVM_c) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_SUBS:
      stack[reg[REG_SP] - 2] = (nightVM_s) ((nightVM_s) stack[reg[REG_SP] - 1] - (nightVM_s) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_SUBI:
      stack[reg[REG_SP] - 2] = (nightVM_i) ((nightVM_i) stack[reg[REG_SP] - 1] - (nightVM_i) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_SUBL:
      stack[reg[REG_SP] - 2] = stack[reg[REG_SP] - 1] - stack[reg[REG_SP] - 2];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_MULUC:
      stack[reg[REG_SP] - 2] = (nightVM_uc) (((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) * ((nightVM_uc) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_MULUS:
      stack[reg[REG_SP] - 2] = (nightVM_us) (((nightVM_us) stack[reg[REG_SP] - 1] + 0u) * ((nightVM_us) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_MULUI:
      stack[reg[REG_SP] - 2] = (nightVM_ui) (((nightVM_ui) stack[reg[REG_SP] - 1] + 0u) * ((nightVM_ui) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_MULC:
      stack[reg[REG_SP] - 2] = (nightVM_c) ((nightVM_c) stack[reg[REG_SP] - 1] * (nightVM_c) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_MULS:
      stack[reg[REG_SP] - 2] = (nightVM_s) ((nightVM_s) stack[reg[REG_SP] - 1] * (nightVM_s) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_MULI:
      stack[reg[REG_SP] - 2] = (nightVM_i) ((nightVM_i) stack[reg[REG_SP] - 1] * (nightVM_i) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_MULL:
      stack[reg[REG_SP] - 2] = stack[reg[REG_SP] - 1] * stack[reg[REG_SP] - 2];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_REMUC:
      stack[reg[REG_SP] - 2] = (nightVM_uc) (((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) % ((nightVM_uc) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_REMUS:
      stack[reg[REG_SP] - 2] = (nightVM_us) (((nightVM_us) stack[reg[REG_SP] - 1] + 0u) % ((nightVM_us) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_REMUI:
      stack[reg[REG_SP] - 2] = (nightVM_ui) (((nightVM_ui) stack[reg[REG_SP] - 1] + 0u) % ((nightVM_ui) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_REMC:
      stack[reg[REG_SP] - 2] = (nightVM_c) ((nightVM_c) stack[reg[REG_SP] - 1] % (nightVM_c) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_REMS:
      stack[reg[REG_SP] - 2] = (nightVM_s) ((nightVM_s) stack[reg[REG_SP] - 1] % (nightVM_s) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_REMI:
      stack[reg[REG_SP] - 2] = (nightVM_i) ((nightVM_i) stack[reg[REG_SP] - 1] % (nightVM_i) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_REML:
      stack[reg[REG_SP] - 2] = stack[reg[REG_SP] - 1] % stack[reg[REG_SP] - 2];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_DIVUC:
      stack[reg[REG_SP] - 2] = (nightVM_uc) (((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) / ((nightVM_uc) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_DIVUS:
      stack[reg[REG_SP] - 2] = (nightVM_us) (((nightVM_us) stack[reg[REG_SP] - 1] + 0u) / ((nightVM_us) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_DIVUI:
      stack[reg[REG_SP] - 2] = (nightVM_ui) (((nightVM_ui) stack[reg[REG_SP] - 1] + 0u) / ((nightVM_ui) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_DIVC:
      stack[reg[REG_SP] - 2] = (nightVM_c) ((nightVM_c) stack[reg[REG_SP] - 1] / (nightVM_c) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_DIVS:
      stack[reg[REG_SP] - 2] = (nightVM_s) ((nightVM_s) stack[reg[REG_SP] - 1] / (nightVM_s) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_DIVI:
      stack[reg[REG_SP] - 2] = (nightVM_i) ((nightVM_i) stack[reg[REG_SP] - 1] / (nightVM_i) stack[reg[REG_SP] - 2]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_DIVL:
      stack[reg[REG_SP] - 2] = stack[reg[REG_SP] - 1] / stack[reg[REG_SP] - 2];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_LSHUC:
      stack[reg[REG_SP] - 2] = (((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) << ((nightVM_uc) stack[reg[REG_SP] - 2] + 0u)) & 0xFF;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_LSHUS:
      stack[reg[REG_SP] - 2] = (((nightVM_us) stack[reg[REG_SP] - 1] + 0u) << ((nightVM_us) stack[reg[REG_SP] - 2] + 0u)) & 0xFFFF;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_LSHUI:
      stack[reg[REG_SP] - 2] = (((nightVM_ui) stack[reg[REG_SP] - 1] + 0u) << ((nightVM_ui)stack[reg[REG_SP] - 2] + 0u)) & 0xFFFFFF;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_LSHC:
      c = stack[reg[REG_SP] - 1];
      c_1 = stack[reg[REG_SP] - 2];

      memcpy(&uc, &c, sizeof(nightVM_uc));
      memcpy(&uc_1, &c_1, sizeof(nightVM_uc));

      uc = ((uc + 0u) << (uc_1 + 0u)) & 0xFF;
      memcpy(&c, &uc, sizeof(nightVM_c));
      stack[reg[REG_SP] - 2] = c;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_LSHS:
      s = stack[reg[REG_SP] - 1];
      s_1 = stack[reg[REG_SP] - 2];

      memcpy(&us, &s, sizeof(nightVM_us));
      memcpy(&us_1, &s_1, sizeof(nightVM_us));

      us = ((us + 0u) << (us_1 + 0u)) & 0xFFFF;
      memcpy(&s, &us, sizeof(nightVM_s));
      stack[reg[REG_SP] - 2] = s;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_LSHI:
      i = stack[reg[REG_SP] - 1];
      i_1 = stack[reg[REG_SP] - 2];

      memcpy(&ui, &i, sizeof(nightVM_ui));
      memcpy(&ui_1, &i_1, sizeof(nightVM_ui));

      ui = ((ui + 0u) << (ui_1 + 0u)) & 0xFFFFFF;
      memcpy(&i, &ui, sizeof(nightVM_i));
      stack[reg[REG_SP] - 2] = i;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_LSHL:
      l = stack[reg[REG_SP] - 1];
      l_1 = stack[reg[REG_SP] - 2];

      memcpy(&uns, &l, sizeof(nightVM_uns));
      memcpy(&uns_1, &l_1, sizeof(nightVM_uns));

      uns = ((uns + 0u) << (uns_1 + 0u)) & 0xFFFFFFFF;
      memcpy(&l, &uns, sizeof(nightVM_l));
      stack[reg[REG_SP] - 2] = l;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_RSHUC:
      stack[reg[REG_SP] - 2] = ((nightVM_uc)stack[reg[REG_SP] - 1] + 0u) >> ((nightVM_uc)stack[reg[REG_SP] - 2] + 0u);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_RSHUS:
      stack[reg[REG_SP] - 2] = ((nightVM_us)stack[reg[REG_SP] - 1] + 0u) >> ((nightVM_us)stack[reg[REG_SP] - 2] + 0u);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_RSHUI:
      stack[reg[REG_SP] - 2] = (((nightVM_ui)stack[reg[REG_SP] - 1] + 0u) >> ((nightVM_ui)stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_RSHC:
      c = stack[reg[REG_SP] - 1];
      c_1 = stack[reg[REG_SP] - 2];

      memcpy(&uc, &c, sizeof(nightVM_uc));
      memcpy(&uc_1, &c_1, sizeof(nightVM_uc));

      uc = (uc + 0u) >> (uc_1 + 0u);
      memcpy(&c, &uc, sizeof(nightVM_c));
      stack[reg[REG_SP] - 2] = c;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_RSHS:
      s = stack[reg[REG_SP] - 1];
      s_1 = stack[reg[REG_SP] - 2];

      memcpy(&us, &s, sizeof(nightVM_us));
      memcpy(&us_1, &s_1, sizeof(nightVM_us));

      us = (us + 0u) >> (us_1 + 0u);
      memcpy(&s, &us, sizeof(nightVM_s));
      stack[reg[REG_SP] - 2] = s;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_RSHI:
      i = stack[reg[REG_SP] - 1];
      i_1 = stack[reg[REG_SP] - 2];

      memcpy(&ui, &i, sizeof(nightVM_ui));
      memcpy(&ui_1, &i_1, sizeof(nightVM_ui));

      ui = (ui + 0u) >> (ui_1 + 0u);
      memcpy(&i, &ui, sizeof(nightVM_i));
      stack[reg[REG_SP] - 2] = i;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_RSHL:
      l = stack[reg[REG_SP] - 1];
      l_1 = stack[reg[REG_SP] - 2];

      memcpy(&uns, &l, sizeof(nightVM_uns));
      memcpy(&uns_1, &l_1, sizeof(nightVM_uns));

      uns = (uns + 0u) >> (uns_1 + 0u);
      memcpy(&l, &uns, sizeof(nightVM_l));
      stack[reg[REG_SP] - 2] = l;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ANDUC:
      stack[reg[REG_SP] - 2] = (((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) & ((nightVM_uc) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ANDUS:
      stack[reg[REG_SP] - 2] = (((nightVM_us) stack[reg[REG_SP] - 1] + 0u) & ((nightVM_us) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ANDUI:
      stack[reg[REG_SP] - 2] = (((nightVM_ui) stack[reg[REG_SP] - 1] + 0u) & ((nightVM_ui) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ANDC:
      c = stack[reg[REG_SP] - 1];
      c_1 = stack[reg[REG_SP] - 2];

      memcpy(&uc, &c, sizeof(nightVM_uc));
      memcpy(&uc_1, &c_1, sizeof(nightVM_uc));

      uc = (uc + 0u) & (uc_1 + 0u);
      memcpy(&c, &uc, sizeof(nightVM_c));
      stack[reg[REG_SP] - 2] = c;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ANDS:
      s = stack[reg[REG_SP] - 1];
      s_1 = stack[reg[REG_SP] - 2];

      memcpy(&us, &s, sizeof(nightVM_us));
      memcpy(&us_1, &s_1, sizeof(nightVM_us));

      us = (us + 0u) & (us_1 + 0u);
      memcpy(&s, &us, sizeof(nightVM_s));
      stack[reg[REG_SP] - 2] = s;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ANDI:
      i = stack[reg[REG_SP] - 1];
      i_1 = stack[reg[REG_SP] - 2];

      memcpy(&ui, &i, sizeof(nightVM_ui));
      memcpy(&ui_1, &i_1, sizeof(nightVM_ui));

      ui = (ui + 0u) & (ui_1 + 0u);
      memcpy(&i, &ui, sizeof(nightVM_i));
      stack[reg[REG_SP] - 2] = i;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ANDL:
      l = stack[reg[REG_SP] - 1];
      l_1 = stack[reg[REG_SP] - 2];

      memcpy(&uns, &l, sizeof(nightVM_uns));
      memcpy(&uns_1, &l_1, sizeof(nightVM_uns));

      uns = (uns + 0u) & (uns_1 + 0u);
      memcpy(&l, &uns, sizeof(nightVM_l));
      stack[reg[REG_SP] - 2] = l;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ORUC:
      stack[reg[REG_SP] - 2] = (((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) | ((nightVM_uc) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ORUS:
      stack[reg[REG_SP] - 2] = (((nightVM_us) stack[reg[REG_SP] - 1] + 0u) | ((nightVM_us) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;
    case OP_ORUI:
      stack[reg[REG_SP] - 2] = (((nightVM_ui) stack[reg[REG_SP] - 1] + 0u) | ((nightVM_ui) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ORC:
      c = stack[reg[REG_SP] - 1];
      c_1 = stack[reg[REG_SP] - 2];

      memcpy(&uc, &c, sizeof(nightVM_uc));
      memcpy(&uc_1, &c_1, sizeof(nightVM_uc));

      uc = (uc + 0u) | (uc_1 + 0u);
      memcpy(&c, &uc, sizeof(nightVM_c));
      stack[reg[REG_SP] - 2] = c;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ORS:
      s = stack[reg[REG_SP] - 1];
      s_1 = stack[reg[REG_SP] - 2];

      memcpy(&us, &s, sizeof(nightVM_us));
      memcpy(&us_1, &s_1, sizeof(nightVM_us));

      us = (us + 0u) | (us_1 + 0u);
      memcpy(&s, &us, sizeof(nightVM_s));
      stack[reg[REG_SP] - 2] = s;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ORI:
      i = stack[reg[REG_SP] - 1];
      i_1 = stack[reg[REG_SP] - 2];

      memcpy(&ui, &i, sizeof(nightVM_ui));
      memcpy(&ui_1, &i_1, sizeof(nightVM_ui));

      ui = (ui + 0u) | (ui_1 + 0u);
      memcpy(&i, &ui, sizeof(nightVM_i));
      stack[reg[REG_SP] - 2] = i;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_ORL:
      l = stack[reg[REG_SP] - 1];
      l_1 = stack[reg[REG_SP] - 2];

      memcpy(&uns, &l, sizeof(nightVM_uns));
      memcpy(&uns_1, &l_1, sizeof(nightVM_uns));

      uns = (uns + 0u) | (uns_1 + 0u);
      memcpy(&l, &uns, sizeof(nightVM_l));
      stack[reg[REG_SP] - 2] = l;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_XORUC:
      stack[reg[REG_SP] - 2] = ((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) ^ ((nightVM_uc) stack[reg[REG_SP] - 2] + 0u);

      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_XORUS:
      stack[reg[REG_SP] - 2] = (((nightVM_us) stack[reg[REG_SP] - 1] + 0u) ^ ((nightVM_us) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_XORUI:
      stack[reg[REG_SP] - 2] = (((nightVM_ui) stack[reg[REG_SP] - 1] + 0u)^((nightVM_ui) stack[reg[REG_SP] - 2] + 0u));
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_XORC:
      c = stack[reg[REG_SP] - 1];
      c_1 = stack[reg[REG_SP] - 2];

      memcpy(&uc, &c, sizeof(nightVM_uc));
      memcpy(&uc_1, &c_1, sizeof(nightVM_uc));

      uc = (uc + 0u) ^ (uc_1 + 0u);
      memcpy(&c, &uc, sizeof(nightVM_c));
      stack[reg[REG_SP] - 2] = c;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_XORS:
      s = stack[reg[REG_SP] - 1];
      s_1 = stack[reg[REG_SP] - 2];

      memcpy(&us, &s, sizeof(nightVM_us));
      memcpy(&us_1, &s_1, sizeof(nightVM_us));

      us = (us + 0u) ^ (us_1 + 0u);
      memcpy(&s, &us, sizeof(nightVM_s));
      stack[reg[REG_SP] - 2] = s;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_XORI:
      i = stack[reg[REG_SP] - 1];
      i_1 = stack[reg[REG_SP] - 2];

      memcpy(&ui, &i, sizeof(nightVM_ui));
      memcpy(&ui_1, &i_1, sizeof(nightVM_ui));

      ui = (ui + 0u) ^ (ui_1 + 0u);
      memcpy(&i, &ui, sizeof(nightVM_i));
      stack[reg[REG_SP] - 2] = i;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_XORL:
      l = stack[reg[REG_SP] - 1];
      l_1 = stack[reg[REG_SP] - 2];

      memcpy(&uns, &l, sizeof(nightVM_uns));
      memcpy(&uns_1, &l_1, sizeof(nightVM_uns));

      uns = (uns + 0u) ^ (uns_1 + 0u);
      memcpy(&l, &uns, sizeof(nightVM_l));
      stack[reg[REG_SP] - 2] = l;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_NOTUC:
      stack[reg[REG_SP] - 1] = ~((nightVM_uc) stack[reg[REG_SP] - 1] + 0u) & 0xFF;

      reg[REG_PC]++;
      break;

    case OP_NOTUS:
      stack[reg[REG_SP] - 1] = ~((nightVM_us) stack[reg[REG_SP] - 1] + 0u) & 0xFFFF;

      reg[REG_PC]++;
      break;

    case OP_NOTUI:
      stack[reg[REG_SP] - 1] = ~((nightVM_ui) stack[reg[REG_SP] - 1] + 0u) & 0xFFFFFF;

      reg[REG_PC]++;
      break;

    case OP_NOTC:
      c = stack[reg[REG_SP] - 1];

      memcpy(&uc, &c, sizeof(nightVM_uc));
      uc = ~(uc + 0u) & 0xFF;

      memcpy(&c, &uc, sizeof(nightVM_c));
      stack[reg[REG_SP] - 1] = c;

      reg[REG_PC]++;
      break;

    case OP_NOTS:
      s = stack[reg[REG_SP] - 1];

      memcpy(&us, &s, sizeof(nightVM_us));
      us = ~(us + 0u) & 0xFFFF;

      memcpy(&s, &us, sizeof(nightVM_s));
      stack[reg[REG_SP] - 1] = s;

      reg[REG_PC]++;
      break;

    case OP_NOTI:
      i = stack[reg[REG_SP] - 1];

      memcpy(&ui, &i, sizeof(nightVM_ui));
      ui = ~(ui + 0u) & 0xFFFFFF;

      memcpy(&i, &ui, sizeof(nightVM_i));
      stack[reg[REG_SP] - 1] = i;

      reg[REG_PC]++;
      break;

    case OP_NOTL:
      l = stack[reg[REG_SP] - 1];

      memcpy(&uns, &l, sizeof(nightVM_uns));
      uns = ~(uns + 0u) & 0xFFFFFFFF;

      memcpy(&l, &uns, sizeof(nightVM_l));
      stack[reg[REG_SP] - 1] = l;

      reg[REG_PC]++;
      break;

    case OP_LDCUC:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      memcpy(&uc, &code_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_uc));
      stack[reg[REG_SP] - 1] = uc;

      reg[REG_PC]++;
      break;

    case OP_ALDCUC:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      stack[reg[REG_SP] - 1] = code_uc[stack[reg[REG_SP]]];

      reg[REG_PC]++;
      break;

    case OP_LDCUS:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      memcpy(&us, &code_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_us));
      stack[reg[REG_SP] - 1] = us;

      reg[REG_PC]++;
      break;

    case OP_ALDCUS:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      stack[reg[REG_SP] - 1] = code_us[stack[reg[REG_SP]] / SIZEOF_US];

      reg[REG_PC]++;
      break;

    case OP_LDCUI:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      memcpy(&ui, &code_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_ui));
      stack[reg[REG_SP] - 1] = ui;

      reg[REG_PC]++;
      break;

    case OP_ALDCUI:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      stack[reg[REG_SP] - 1] = code_ui[stack[reg[REG_SP]] / SIZEOF_UI];

      reg[REG_PC]++;
      break;

    case OP_LDCC:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      memcpy(&c, &code_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_c));
      stack[reg[REG_SP] - 1] = c;

      reg[REG_PC]++;
      break;

    case OP_ALDCC:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      stack[reg[REG_SP] - 1] = code_c[stack[reg[REG_SP]]];

      reg[REG_PC]++;
      break;

    case OP_LDCS:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      memcpy(&s, &code_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_s));
      stack[reg[REG_SP] - 1] = s;

      reg[REG_PC]++;
      break;

    case OP_ALDCS:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      stack[reg[REG_SP] - 1] = code_s[stack[reg[REG_SP]] / SIZEOF_S];

      reg[REG_PC]++;
      break;

    case OP_LDCI:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      memcpy(&i, &code_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_i));
      stack[reg[REG_SP] - 1] = i;

      reg[REG_PC]++;
      break;

    case OP_ALDCI:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      stack[reg[REG_SP] - 1] = code_i[stack[reg[REG_SP]] / SIZEOF_I];

      reg[REG_PC]++;
      break;

    case OP_LDCL:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      memcpy(&stack[reg[REG_SP] - 1], &code_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_l));

      reg[REG_PC]++;
      break;

    case OP_ALDCL:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      stack[reg[REG_SP] - 1] = code_l[stack[reg[REG_SP]] / SIZEOF_L];

      reg[REG_PC]++;
      break;

    case OP_LDHUC:
      memcpy(&uc, &heap_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_uc));
      stack[reg[REG_SP] - 1] = uc;

      reg[REG_PC]++;
      break;

    case OP_ALDHUC:
      stack[reg[REG_SP] - 1] = heap_uc[stack[reg[REG_SP]]];

      reg[REG_PC]++;
      break;

    case OP_LDHUS:
      memcpy(&us, &heap_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_us));
      stack[reg[REG_SP] - 1] = us;

      reg[REG_PC]++;
      break;

    case OP_ALDHUS:
      stack[reg[REG_SP] - 1] = heap_uc[stack[reg[REG_SP]] / SIZEOF_US];

      reg[REG_PC]++;
      break;

    case OP_LDHUI:
      memcpy(&ui, &heap_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_ui));
      stack[reg[REG_SP] - 1] = ui;

      reg[REG_PC]++;
      break;

    case OP_ALDHUI:
      stack[reg[REG_SP] - 1] = heap_ui[stack[reg[REG_SP]] / SIZEOF_UI];

      reg[REG_PC]++;
      break;

    case OP_LDHC:
      memcpy(&c, &heap_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_c));
      stack[reg[REG_SP] - 1] = c;

      reg[REG_PC]++;
      break;

    case OP_ALDHC:
      stack[reg[REG_SP] - 1] = heap_c[stack[reg[REG_SP]]];

      reg[REG_PC]++;
      break;

    case OP_LDHS:
      memcpy(&s, &heap_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_s));
      stack[reg[REG_SP] - 1] = s;

      reg[REG_PC]++;
      break;

    case OP_ALDHS:
      stack[reg[REG_SP] - 1] = heap_s[stack[reg[REG_SP]] / SIZEOF_S];

      reg[REG_PC]++;
      break;

    case OP_LDHI:
      memcpy(&i, &heap_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_i));
      stack[reg[REG_SP] - 1] = i;

      reg[REG_PC]++;
      break;

    case OP_ALDHI:
      stack[reg[REG_SP] - 1] = heap_i[stack[reg[REG_SP]]/SIZEOF_I];

      reg[REG_PC]++;
      break;

    case OP_LDHL:
      memcpy(&stack[reg[REG_SP] - 1], &heap_uc[stack[reg[REG_SP] - 1]], sizeof(nightVM_l));

      reg[REG_PC]++;
      break;

    case OP_ALDHL:
      stack[reg[REG_SP] - 1] = heap_l[stack[reg[REG_SP]]/SIZEOF_L];

      reg[REG_PC]++;
      break;

    case OP_STHUC:
      uc = stack[reg[REG_SP] - 2];
      memcpy(&heap_uc[stack[reg[REG_SP] - 1]], &uc, sizeof(nightVM_uc));
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_ASTHUC:
      heap_uc[stack[reg[REG_SP] - 1]] = stack[reg[REG_SP] - 2];;
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_STHUS:
      us = stack[reg[REG_SP] - 2];
      memcpy(&heap_uc[stack[reg[REG_SP] - 1]], &us, sizeof(nightVM_us));
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_ASTHUS:
      heap_us[stack[reg[REG_SP] - 1]/SIZEOF_US] = stack[reg[REG_SP] - 2];;
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_STHUI:
      ui = stack[reg[REG_SP] - 2];
      memcpy(&heap_uc[stack[reg[REG_SP] - 1]], &ui, sizeof(nightVM_ui));
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_ASTHUI:
      heap_ui[stack[reg[REG_SP] - 1]/SIZEOF_UI] = stack[reg[REG_SP] - 2];;
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_STHC:
      c = stack[reg[REG_SP] - 2];
      memcpy(&heap_uc[stack[reg[REG_SP] - 1]], &c, sizeof(nightVM_c));
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_ASTHC:
      heap_c[stack[reg[REG_SP] - 1]] = stack[reg[REG_SP] - 2];;
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_STHS:
      s = stack[reg[REG_SP] - 2];
      memcpy(&heap_uc[stack[reg[REG_SP] - 1]], &s, sizeof(nightVM_s));
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_ASTHS:
      heap_s[stack[reg[REG_SP] - 1]/SIZEOF_S] = stack[reg[REG_SP] - 2];;
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_STHI:
      i = stack[reg[REG_SP] - 2];
      memcpy(&heap_uc[stack[reg[REG_SP] - 1]], &i, sizeof(nightVM_i));
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_ASTHI:
      heap_i[stack[reg[REG_SP] - 1]/SIZEOF_I] = stack[reg[REG_SP] - 2];;
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_STHL:
      l = stack[reg[REG_SP] - 2];
      memcpy(&heap_uc[stack[reg[REG_SP] - 1]], &l, sizeof(nightVM_l));
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_ASTHL:
      heap_us[stack[reg[REG_SP] - 1]/SIZEOF_L] = stack[reg[REG_SP] - 2];;
      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_LDR0:
      stack[reg[REG_SP]] = reg[REG_GPR0];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_LDR1:
      stack[reg[REG_SP]] = reg[REG_GPR1];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_LDR2:
      stack[reg[REG_SP]] = reg[REG_GPR2];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_LDR3:
      stack[reg[REG_SP]] = reg[REG_GPR3];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_LDR4:
      stack[reg[REG_SP]] = reg[REG_GPR4];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_LDR5:
      stack[reg[REG_SP]] = reg[REG_GPR5];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_LDR6:
      stack[reg[REG_SP]] = reg[REG_GPR6];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_LDR7:
      stack[reg[REG_SP]] = reg[REG_GPR7];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_STR0:
      reg[REG_GPR0] = stack[reg[REG_SP] - 1];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_STR1:
      reg[REG_GPR1] = stack[reg[REG_SP] - 1];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_STR2:
      reg[REG_GPR2] = stack[reg[REG_SP] - 1];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_STR3:
      reg[REG_GPR3] = stack[reg[REG_SP] - 1];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_STR4:
      reg[REG_GPR4] = stack[reg[REG_SP] - 1];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_STR5:
      reg[REG_GPR5] = stack[reg[REG_SP] - 1];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_STR6:
      reg[REG_GPR6] = stack[reg[REG_SP] - 1];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_STR7:
      reg[REG_GPR7] = stack[reg[REG_SP] - 1];
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_PANIC:
      if (stack[reg[REG_SP] - 1]) {
	*exit_status = 1;
	return ERR_PANIC;
      }

      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_FORCE_PANIC:
      *exit_status = 1;
      return ERR_PANIC;

    case OP_INCSP:
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_DECSP:
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_SWAP:
      l = stack[reg[REG_SP] - 1];

      stack[reg[REG_SP] - 1] = stack[reg[REG_SP] - 2];

      stack[reg[REG_SP] - 2] = l;

      reg[REG_PC]++;
      break;

    case OP_POP:
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_COPY:
      stack[reg[REG_SP] - 1] = stack[stack[reg[REG_SP] - 1]];

      reg[REG_PC]++;
      break;

    case OP_PCOPY:
      l = stack[stack[reg[REG_SP] - 1]];

      memmove(&stack[stack[reg[REG_SP] - 1]], &stack[stack[reg[REG_SP] - 1] + 1], (reg[REG_SP] - (stack[reg[REG_SP] - 1] + 1)) * sizeof(nightVM_l));

      stack[reg[REG_SP] - 2] = l;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_PUT:
      l = stack[reg[REG_SP] - 2];

      memmove(&stack[stack[reg[REG_SP] - 1] + 1], &stack[stack[reg[REG_SP] - 1]], (reg[REG_SP] - stack[reg[REG_SP] - 1]) * sizeof(nightVM_l));

      stack[stack[reg[REG_SP] - 1]] = l;
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_POPA:
      memmove(&stack[stack[reg[REG_SP] - 1]], &stack[stack[reg[REG_SP] - 1] + 1], (reg[REG_SP] - (stack[reg[REG_SP] - 1] + 1)) * sizeof(nightVM_l));

      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    case OP_DUP:
      stack[reg[REG_SP]] = stack[reg[REG_SP] - 1];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_OVER:
      stack[reg[REG_SP]] = stack[reg[REG_SP] - 2];
      reg[REG_SP]++;

      reg[REG_PC]++;
      break;

    case OP_RJGT:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] > stack[reg[REG_SP]]) {
	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RJLS:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] < stack[reg[REG_SP]]) {
	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RJEQ:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] == stack[reg[REG_SP]]) {
	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RJLE:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] <= stack[reg[REG_SP]]) {
	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RJGE:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] >= stack[reg[REG_SP]]) {
	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RJNE:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 2] != stack[reg[REG_SP]]) {
	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RJZ:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 2;

      if (stack[reg[REG_SP]] == 0) {
	reg[REG_PC] += stack[reg[REG_SP] + 1];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RJNZ:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 2;

      if (stack[reg[REG_SP]] != 0) {
	reg[REG_PC] += stack[reg[REG_SP] + 1];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RJMP:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP]--;

      reg[REG_PC] += stack[reg[REG_SP]];
      break;

    case OP_JGT:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] > stack[reg[REG_SP]]) {
	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_JLS:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] < stack[reg[REG_SP]]) {
	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_JEQ:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] == stack[reg[REG_SP]]) {
	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_JLE:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] <= stack[reg[REG_SP]]) {
	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_JGE:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] >= stack[reg[REG_SP]]) {
	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_JNE:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] != stack[reg[REG_SP]]) {
	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_JZ:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 2;

      if (stack[reg[REG_SP]] == 0) {
	reg[REG_PC] = stack[reg[REG_SP] + 1];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_JNZ:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 2;

      if (stack[reg[REG_SP]] != 0) {
	reg[REG_PC] = stack[reg[REG_SP] + 1];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_JMP:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP]--;

      reg[REG_PC] = stack[reg[REG_SP]];
      break;

    case OP_RET:
      reg[REG_CLP]--;

      reg[REG_PC] = call_stack[reg[REG_CLP]];
      break;

    case OP_CLGT:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] > stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_CLLS:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] < stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_CLEQ:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] == stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_CLLE:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] <= stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_CLGE:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] >= stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_CLNE:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] != stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] = stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_CLZ:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 2;

      if (stack[reg[REG_SP]] == 0) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] = stack[reg[REG_SP] + 1];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_CLNZ:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 2;

      if (stack[reg[REG_SP]] != 0) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] = stack[reg[REG_SP] + 1];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_CALL:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
      reg[REG_CLP]++;
      reg[REG_SP]--;

      reg[REG_PC] = stack[reg[REG_SP]];
      break;

    case OP_RCLGT:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] > stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RCLLS:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] < stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RCLEQ:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] == stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RCLLE:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] <= stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RCLGE:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] >= stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RCLNE:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 3;

      if (stack[reg[REG_SP] + 1] != stack[reg[REG_SP]]) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] += stack[reg[REG_SP] + 2];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RCLZ:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 2;

      if (stack[reg[REG_SP]] == 0) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] += stack[reg[REG_SP] + 1];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RCLNZ:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      reg[REG_SP] -= 2;

      if (stack[reg[REG_SP]] != 0) {
	call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
	reg[REG_CLP]++;

	reg[REG_PC] += stack[reg[REG_SP] + 1];
      }
      else{
	reg[REG_PC]++;
      }

      break;

    case OP_RCALL:
      if (reg[REG_PC] + stack[reg[REG_SP] - 1] >= reg[REG_CS] || reg[REG_PC] + stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = reg[REG_PC] + stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      call_stack[reg[REG_CLP]] = reg[REG_PC] + 1;
      reg[REG_CLP]++;
      reg[REG_SP]--;

      reg[REG_PC] += stack[reg[REG_SP]];
      break;

    case OP_CGT:
      if (stack[reg[REG_SP] - 1] > stack[reg[REG_SP] - 2]) {
	stack[reg[REG_SP] - 2] = 1;
      }
      else{
	stack[reg[REG_SP] - 2] = 0;
      }

      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_CLS:
      if (stack[reg[REG_SP] - 1] < stack[reg[REG_SP] - 2]) {
	stack[reg[REG_SP] - 2] = 1;
      }
      else{
	stack[reg[REG_SP] - 2] = 0;
      }

      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_CEQ:
      if (stack[reg[REG_SP] - 1] == stack[reg[REG_SP] - 2]) {
	stack[reg[REG_SP] - 2] = 1;
      }
      else{
	stack[reg[REG_SP] - 2] = 0;
      }

      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_CLE:
      if (stack[reg[REG_SP] - 1] <= stack[reg[REG_SP] - 2]) {
	stack[reg[REG_SP] - 2] = 1;
      }
      else{
	stack[reg[REG_SP] - 2] = 0;
      }

      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_CGE:
      if (stack[reg[REG_SP] - 1] >= stack[reg[REG_SP] - 2]) {
	stack[reg[REG_SP] - 2] = 1;
      }
      else{
	stack[reg[REG_SP] - 2] = 0;
      }

      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_CNE:
      if (stack[reg[REG_SP] - 1] != stack[reg[REG_SP] - 2]) {
	stack[reg[REG_SP] - 2] = 1;
      }
      else{
	stack[reg[REG_SP] - 2] = 0;
      }

      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_CZ:
      if (stack[reg[REG_SP] - 1] == 0) {
	stack[reg[REG_SP] - 1] = 1;
      }
      else{
	stack[reg[REG_SP] - 1] = 0;
      }

      reg[REG_PC]++;
      break;

    case OP_CNZ:
      if (stack[reg[REG_SP] - 1] != 0) {
	stack[reg[REG_SP] - 1] = 1;
      }
      else{
	stack[reg[REG_SP] - 1] = 0;
      }

      reg[REG_PC]++;
      break;

    case OP_DBPRINT:
      fprintf(stderr, "%" PRINVML "\n", stack[reg[REG_SP] - 1]);
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_NOP:
      reg[REG_PC]++;
      break;

    case OP_OPEN:
      if (stack[reg[REG_SP] - 1] >= reg[REG_CS] || stack[reg[REG_SP] - 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] - 1];
	return ERR_TRAP;
      }

      if ((str = cseq2str(&((nightVM_c *) *code)[stack[reg[REG_SP] - 1]])) == NULL) {
	*exit_status = 1;
	return ERR_FAILED_ALLOCATION;
      }

      if (realpath(str, resolved_name) == NULL) {
	free(str);
	*exit_status = 1;
	return ERR_OPEN;
      }

      free(str);

      hash_table_entry.key = resolved_name;

      if (hsearch(hash_table_entry, FIND) == NULL) {
	hash_table_entry.data = dlopen(hash_table_entry.key, RTLD_LAZY|RTLD_LOCAL);
	if (hash_table_entry.data == NULL) {
	  *exit_status = 1;
	  return ERR_OPEN;
	}

	opened_libs[*lib_pt] = hash_table_entry.data;
	if ((lib_names[*lib_pt] = malloc(strlen(hash_table_entry.key) + 1)) == NULL) {
	  *exit_status = 1;
	  return ERR_FAILED_ALLOCATION;
	}

	strcpy(lib_names[*lib_pt], resolved_name);
	hash_table_entry.key = lib_names[*lib_pt];
	(*lib_pt)++;
	opened_libs[*lib_pt] = NULL;

	if (hsearch(hash_table_entry, ENTER) == NULL) {
	  *exit_status = 1;
	  return ERR_HASH_TABLE_ENTER;
	}
      }
      reg[REG_SP]--;

      reg[REG_PC]++;
      break;

    case OP_INVOKE:
      reg[REG_SP] -= 2;

      if (stack[reg[REG_SP]] >= reg[REG_CS] || stack[reg[REG_SP]] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP]];
	return ERR_TRAP;
      }

      if (stack[reg[REG_SP] + 1] >= reg[REG_CS] || stack[reg[REG_SP] + 1] < reg[REG_CB]) {
	*exit_status = 1;
	reg[REG_IA] = stack[reg[REG_SP] + 1];
	return ERR_TRAP;
      }

      if ((str = cseq2str(&((nightVM_c *) *code)[stack[reg[REG_SP]]])) == NULL) {
	*exit_status = 1;
	return ERR_FAILED_ALLOCATION;
      }

      if (realpath(str, resolved_name) == NULL) {
	free(str);
	*exit_status = 1;
	return ERR_INVOKE;
      }

      free(str);

      hash_table_entry.key = resolved_name;

      if ((hash_table_entry_ret = hsearch(hash_table_entry, FIND)) != NULL) {
	void *object = hash_table_entry_ret->data;
	if ((str = cseq2str(&((nightVM_c *) *code)[stack[reg[REG_SP] + 1]])) == NULL) {
	  *exit_status = 1;
	  return ERR_FAILED_ALLOCATION;
	}

	hash_table_entry.key = str;

	if ((hash_table_entry_ret = hsearch(hash_table_entry, FIND)) == NULL) {
	  hash_table_entry.data = dlsym(object, hash_table_entry.key);
	  if (hash_table_entry.data == NULL) {
	    free(str);
	    *exit_status = 1;
	    return ERR_INVOKE;
	  }

	  if ((sym_names[*sym_pt] = malloc(strlen(hash_table_entry.key) + 1)) == NULL) {
	    free(str);
	    *exit_status = 1;
	    return ERR_FAILED_ALLOCATION;
	  }

	  strcpy(sym_names[*sym_pt], hash_table_entry.key);
	  hash_table_entry.key = sym_names[*sym_pt];
	  free(str);
	  (*sym_pt)++;

	  if (hsearch(hash_table_entry, ENTER) == NULL) {
	    *exit_status = 1;
	    return ERR_HASH_TABLE_ENTER;
	  }

	  hash_table_entry_ret = &hash_table_entry;
	}
	else{
	  free(str);
	}

	symbol = (void (*)(int argc, char **argv, nightVM_l *stack, void **code, nightVM_ui code_align, void **heap, nightVM_ui heap_align, nightVM_l ssz, nightVM_l *hsz, nightVM_l *sp, nightVM_l *cs, nightVM_l pc, nightVM_l lop, nightVM_l *call_stack, nightVM_l clp, nightVM_l *gpr)) hash_table_entry_ret->data;

	symbol(argc, argv, stack, code, code_align, heap, heap_align, reg[REG_SSZ], &reg[REG_HSZ], &reg[REG_SP], &reg[REG_CS], reg[REG_PC], reg[REG_LOP], call_stack, reg[REG_CLP], &reg[REG_GPR0]);

	reg[REG_PC]++;
	break;
      }
      else{
	*exit_status = 1;
	return ERR_INVOKE;
      }

      reg[REG_SP] -= 2;

      reg[REG_PC]++;
      break;

    default:
      *exit_status = 1;
      return ERR_ILLEGAL_INSTRUCTION;
    }
  }

  return ERR_SUCCESS;
}
