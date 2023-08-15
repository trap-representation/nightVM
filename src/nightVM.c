#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <search.h>
#include <stdbool.h>

#include "implementation.h"
#include "alloc.h"

#include "libnightVM.h"

static int throw_err(nightVM_l reg_pc_val, nightVM_l reg_sp_val, unsigned char code){
  char *err_arr[]={"error: panic", "error: failed to open library", "error: failed to invoke symbol", "error: illegal instruction", "error: access outside of code", "implementation error: internal allocation failed", "implementation error: failed to open input", "implementation error: failed to read from input", "implementation error: input is not a valid ESXF", "implementation error: invalid ESXF magic number", "implementation error: unsupported ESFF used", "implementation error: failed to create hash table", "implementation error: failed to put entry into hash table"};
  fprintf(stderr,"%s (at %" PRINVML "; stack pointer at %" PRINVML ")\n",err_arr[code],reg_pc_val,reg_sp_val);
  return code;
}

int main(int argc, char *argv[]){
  nightVM_l *stack;
  void *code=NULL;
  void *heap;
  uint32_t code_alignment, heap_alignment;
  nightVM_l reg[1+reg_gpr0+7];
  char *in_file_name="./a.esxf";
  bool randomize_code_base=false;
  bool randomize_stack_base=false;
  int args_start=0;
  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"-i")==0 || strcmp(argv[i],"--input")==0){
      i++;
      if(i==argc){
        fprintf(stderr,"expected a file-path after %s\n",argv[i-1]);
        return 1;
      }
      else{
        in_file_name=argv[i];
        i++;
        args_start=i;
        for(;i<argc;i++){
          if(strcmp(argv[i],"@-")==0){
            argv[i]=NULL;
            break;
          }
        }
      }
    }
    else if(strcmp(argv[i],"-rc")==0 || strcmp(argv[i],"--randomize-code-base")==0){
      randomize_code_base=true;
    }
    else if(strcmp(argv[i],"-rs")==0 || strcmp(argv[i],"--randomize-stack-base")==0){
      randomize_stack_base=true;
    }
    else if(strcmp(argv[i],"-c")==0 || strcmp(argv[i],"--credits")==0){
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
    else if(strcmp(argv[i],"-a")==0 || strcmp(argv[i],"--about")==0){
      printf("\
    NightVM is an implementation of YSM.\n\
    This program is licensed under GPLv3 Copyright (C) 2023 Somdipto Chakraborty.\n");
      return 0;
    }
    else if(strcmp(argv[i],"-h")==0 || strcmp(argv[i],"--help")==0){
      fprintf(stderr,"    --help (-h): display this message and quit\n");
      fprintf(stderr,"*   --input (-i) file [args...]: specify the input file (default: %s); optional arguments following the file are supplied to the loaded program\n",in_file_name);
      fprintf(stderr,"    --randomize-code-base (-rc): randomize the base address of the code\n");
      fprintf(stderr,"    --randomize-stack-base (-rs): randomize the base address of the stack\n");
      fprintf(stderr,"    --credits (-c): display license information and quit\n");
      fprintf(stderr,"    --about (-a): display information about this implementation and quit\n\n");

      fprintf(stderr,"    in the aforementioned list, certain arguments that can be followed by a sequence of arguments are marked with an asterisk (*); those can end with @- to denote the end of the sequence\n");
      fprintf(stderr,"    for a normative version of this list, read the nightVM manual\n");
      return 0;
    }
    else{
      fprintf(stderr,"%s is an invalid argument; use the --help (-h) argument for a list of valid arguments\n",argv[i]);
      return 1;
    }
  }
  unsigned char ret;
  if((ret=read_file(in_file_name,&code,&code_alignment,&heap_alignment,reg))){
    return throw_err(0,0,ret-1);
  }
  srand(time(NULL));
  int_fast64_t code_base=0;
  if(randomize_code_base){
    void *nonrandomized_code=code;
    code_base=(rand()%MAX_CODE_BASE)+1;
    if(code_base%_Alignof(nightVM_l)!=0){
      code_base+=(code_base+_Alignof(nightVM_l)-1)-((code_base+_Alignof(nightVM_l)-1)%_Alignof(nightVM_l))-code_base;
    }
    if((code=aligned_malloc(code_alignment*sizeof(nightVM_uc),(reg[reg_cs]+code_base)*sizeof(nightVM_uc)))==NULL){
      free(nonrandomized_code);
      return throw_err(0,0,err_failed_allocation-1);
    }
    memcpy(&((unsigned char *)code)[code_base],nonrandomized_code,reg[reg_cs]);
    reg[reg_cs]+=code_base;
    reg[reg_pc]+=code_base;
    free(nonrandomized_code);
  }
  reg[reg_cb]=code_base;
  int_fast64_t stack_base=0;
  if(randomize_stack_base){
    stack_base=(rand()%MAX_STACK_BASE)+1;
    if((stack=aligned_malloc(_Alignof(nightVM_l),reg[reg_ssz]+stack_base*sizeof(nightVM_l)))==NULL){
      free(code);
      return throw_err(0,0,err_failed_allocation-1);
    }
  }
  else{
    if((stack=aligned_malloc(_Alignof(nightVM_l),reg[reg_ssz]*sizeof(nightVM_l)))==NULL){
      free(code);
      return throw_err(0,0,err_failed_allocation-1);
    }
  }
  if((heap=aligned_malloc(heap_alignment*sizeof(nightVM_uc),reg[reg_hsz]*sizeof(nightVM_uc)))==NULL){
    free(code);
    free(stack);
    return throw_err(0,0,err_failed_allocation-1);
  }
  reg[reg_sp]=0;
  char **opened_libs;
  if((opened_libs=malloc(65536*sizeof(char *)))==NULL){
    return throw_err(0,0,err_failed_allocation-1);
  }
  opened_libs[0]=NULL;
  size_t lib_pt=0;
  size_t sym_pt=0;
  char **lib_names;
  if((lib_names=malloc(65536*sizeof(char *)))==NULL){
    return throw_err(0,0,err_failed_allocation-1);
  }
  char **sym_names;
  if((sym_names=malloc(65536*sizeof(char *)))==NULL){
    free(lib_names);
    return throw_err(0,0,err_failed_allocation-1);
  }
  if(hcreate(65536)==0){
    free(sym_names);
    free(lib_names);
    close_opened_libs(opened_libs);
    free(opened_libs);
    return throw_err(0,0,err_failed_to_create_hash_table-1);
  }
  unsigned int exit_status;
  int exit_code=eval(argc-args_start,&argv[args_start],&stack[stack_base],&code,code_alignment,&heap,heap_alignment,reg,&exit_status,load_type_nembd,opened_libs,&lib_pt,&sym_pt,lib_names,sym_names);
  if(exit_status){
    throw_err(reg[reg_pc],reg[reg_sp],exit_code-1);
  }
  hdestroy();
  free_sym_names(sym_names,sym_pt);
  free_lib_names(lib_names,lib_pt);
  close_opened_libs(opened_libs);
  free(opened_libs);
  free(code);
  free(stack);
  free(heap);
  return exit_code;
}
