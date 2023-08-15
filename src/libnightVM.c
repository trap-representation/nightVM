#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <limits.h>
#include <search.h>

#include "libnightVM.h"

static inline void *aligned_malloc(size_t alignment, size_t size){
  void *return_mem;
  if(size==0 || alignment%sizeof(void *)!=0 || ceil(log2l(alignment))!=floor(log2l(alignment))){
    return NULL;
  }
  else if(posix_memalign(&return_mem,alignment,size)==0){
    return return_mem;
  }
  else{
    return NULL;
  }
}

static inline void *aligned_realloc(void *ptr, size_t alignment, size_t size, size_t prev_size){
  void *new_mem=aligned_malloc(alignment,size);
  if(new_mem!=NULL && ptr!=NULL){
    size_t size_to_copy=prev_size>size?size:prev_size;
    memcpy(new_mem,ptr,size_to_copy);
    free(ptr);
  }
  return new_mem;
}

#define READBUF_SIZE 1048576

unsigned int read_file(char *file, void **code, nightVM_ui *code_alignment, nightVM_ui *heap_alignment, nightVM_l *reg){
  FILE *fp;
  if((fp=fopen(file,"rb"))==NULL){
    return err_failed_to_open_file_for_reading;
  }
  nightVM_ui magic;
  if(fread(&magic,sizeof(nightVM_ui),1,fp)<1){
    fclose(fp);
    return err_invalid_file_format;
  }
  if(magic==0X000E3EFF){
    nightVM_ui specification_version;
    if(fread(&specification_version,sizeof(nightVM_ui),1,fp)<1){
      fclose(fp);
      return err_invalid_file_format;
    }
    if(specification_version==0x00000023){
      nightVM_uns uns;
      if(fread(code_alignment,sizeof(nightVM_ui),1,fp)<1){
        fclose(fp);
        return err_invalid_file_format;
      }
      if(fread(heap_alignment,sizeof(nightVM_ui),1,fp)<1){
        fclose(fp);
        return err_invalid_file_format;
      }
      if(fread(&uns,sizeof(nightVM_uns),1,fp)<1){
        fclose(fp);
        return err_invalid_file_format;
      }
      reg[reg_hsz]=uns;
      if(fread(&uns,sizeof(nightVM_uns),1,fp)<1){
        fclose(fp);
        return err_invalid_file_format;
      }
      reg[reg_ssz]=uns;
      if(fread(&uns,sizeof(nightVM_uns),1,fp)<1){
        fclose(fp);
        return err_invalid_file_format;
      }
      reg[reg_pc]=uns;
      void *read_buf;
      if((read_buf=malloc(READBUF_SIZE*sizeof(nightVM_uc)))==NULL){
        fclose(fp);
        return err_failed_allocation;
      }
      size_t read;
      reg[reg_cs]=0;
      while((read=fread(read_buf,sizeof(nightVM_uc),READBUF_SIZE,fp))){
        if(ferror(fp)){
          free(read_buf);
          free(*code);
          fclose(fp);
          return err_failed_to_read_from_file;
        }
        if((*code=aligned_realloc(*code,*code_alignment,(reg[reg_cs]+read)*sizeof(nightVM_uc),reg[reg_cs]))==NULL){
          return err_failed_allocation;
        }
        memcpy(&((nightVM_c *)*code)[reg[reg_cs]],read_buf,read*sizeof(nightVM_uc));
        reg[reg_cs]+=read;
      }
      if(ferror(fp)){
        free(read_buf);
        free(*code);
        fclose(fp);
        return err_failed_to_read_from_file;
      }
      free(read_buf);
      fclose(fp);
    }
    else{
      fclose(fp);
      return err_invalid_specification_version;
    }
  }
  else{
    fclose(fp);
    return err_invalid_magic;
  }
  return 0;
}

void close_opened_libs(char **opened_libs){
  for(size_t i=0;opened_libs[i]!=NULL;i++){
    dlclose(opened_libs[i]);
  }
}

void free_lib_names(char **lib_names, size_t lib_size){
  if(lib_names!=NULL){
    for(size_t i=0;i<lib_size;i++){
      free(lib_names[i]);
    }
    free(lib_names);
  }
}

void free_sym_names(char **sym_names, size_t sym_size){
  if(sym_names!=NULL){
    for(size_t i=0;i<sym_size;i++){
      free(sym_names[i]);
    }
    free(sym_names);
  }
}

unsigned int eval(int argc, char **argv, nightVM_l *stack, void **code, nightVM_ui code_align, void **heap, nightVM_ui heap_align, nightVM_l *reg, unsigned int *exit_status, unsigned int load_type, char **opened_libs, size_t *lib_pt, size_t *sym_pt, char **lib_names, char **sym_names){
  *exit_status=0;
  char resolved_name[PATH_MAX];
  char *str;
  ENTRY hash_table_entry;
  ENTRY *hash_table_entry_ret;
  void (*symbol)(int argc, char **argv, nightVM_l *stack, void **code, nightVM_ui code_align, void **heap, nightVM_ui heap_align, nightVM_l reg_ssz_val, nightVM_l *reg_hsz_val, nightVM_l *reg_sp_val, nightVM_l *reg_cs_val, nightVM_l reg_pc_val, nightVM_l *gpr);
  nightVM_uc *code_uc=*code;
  nightVM_uc *heap_uc=*heap;
  nightVM_uc uc, uc_1;
  nightVM_us us, us_1;
  nightVM_ui ui, ui_1;
  nightVM_uns uns, uns_1;
  nightVM_c c, c_1;
  nightVM_s s, s_1;
  nightVM_i i, i_1;
  nightVM_l l, l_1;
  nightVM_uc opcode;
  while(1){
    if(reg[reg_pc]>=reg[reg_cs] || reg[reg_pc]<reg[reg_cb]){
      *exit_status=1;
      reg[reg_ia]=reg[reg_pc];
      return err_trap;
    }
    opcode=code_uc[reg[reg_pc]];
    switch(opcode){
    case op_trap:
      *exit_status=1;
      reg[reg_ia]=reg[reg_pc];
      return err_trap;
    case op_halt:
      return 0;
    case op_haltr:
      return stack[reg[reg_sp]-1];
    case op_rbs:
      reg[reg_pc]++;
      break;
    case op_rbe:
      reg[reg_pc]++;
      break;
    case op_sts:
      stack[stack[reg[reg_sp]-1]]=stack[reg[reg_sp]-2];
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    case op_pushuc:
      reg[reg_pc]++;
      stack[reg[reg_sp]]=code_uc[reg[reg_pc]];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_pushus:
      reg[reg_pc]++;
      memcpy(&us,&code_uc[reg[reg_pc]],sizeof(nightVM_us));
      stack[reg[reg_sp]]=us;
      reg[reg_sp]++;
      reg[reg_pc]+=SIZEOF_US;
      break;
    case op_pushui:
      reg[reg_pc]++;
      memcpy(&ui,&code_uc[reg[reg_pc]],sizeof(nightVM_ui));
      stack[reg[reg_sp]]=ui;
      reg[reg_sp]++;
      reg[reg_pc]+=SIZEOF_UI;
      break;
    case op_pushc:
      reg[reg_pc]++;
      stack[reg[reg_sp]]=((nightVM_c *)*code)[reg[reg_pc]];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_pushs:
      reg[reg_pc]++;
      memcpy(&s,&code_uc[reg[reg_pc]],sizeof(nightVM_s));
      stack[reg[reg_sp]]=s;
      reg[reg_sp]++;
      reg[reg_pc]+=SIZEOF_S;
      break;
    case op_pushi:
      reg[reg_pc]++;
      memcpy(&i,&code_uc[reg[reg_pc]],sizeof(nightVM_i));
      stack[reg[reg_sp]]=i;
      reg[reg_sp]++;
      reg[reg_pc]+=SIZEOF_I;
      break;
    case op_pushl:
      reg[reg_pc]++;
      memcpy(&stack[reg[reg_sp]],&code_uc[reg[reg_pc]],sizeof(nightVM_l));
      reg[reg_sp]++;
      reg[reg_pc]+=SIZEOF_L;
      break;
    case op_pushpc:
      stack[reg[reg_sp]]=reg[reg_pc];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_pushcs:
      stack[reg[reg_sp]]=reg[reg_cs];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_pushsp:
      stack[reg[reg_sp]]=reg[reg_sp];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_pushlt:
      stack[reg[reg_sp]]=load_type;
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_pushn1:
      stack[reg[reg_sp]]=-1;
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_push0:
      stack[reg[reg_sp]]=0;
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_push1:
      stack[reg[reg_sp]]=1;
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_push2:
      stack[reg[reg_sp]]=2;
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_push3:
      stack[reg[reg_sp]]=3;
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_push4:
      stack[reg[reg_sp]]=4;
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_push5:
      stack[reg[reg_sp]]=5;
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_push7:
      stack[reg[reg_sp]]=7;
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_adduc:
      stack[reg[reg_sp]-2]=(nightVM_uc)(((nightVM_uc)stack[reg[reg_sp]-1]+0u)+((nightVM_uc)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_addus:
      stack[reg[reg_sp]-2]=(nightVM_us)(((nightVM_us)stack[reg[reg_sp]-1]+0u)+((nightVM_us)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_addui:
      stack[reg[reg_sp]-2]=(nightVM_ui)(((nightVM_ui)stack[reg[reg_sp]-1]+0u)+((nightVM_ui)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_addc:
      stack[reg[reg_sp]-2]=(nightVM_c)((nightVM_c)stack[reg[reg_sp]-1]+(nightVM_c)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_adds:
      stack[reg[reg_sp]-2]=(nightVM_s)((nightVM_s)stack[reg[reg_sp]-1]+(nightVM_s)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_addi:
      stack[reg[reg_sp]-2]=(nightVM_i)((nightVM_i)stack[reg[reg_sp]-1]+(nightVM_i)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_addl:
      stack[reg[reg_sp]-2]=stack[reg[reg_sp]-1]+stack[reg[reg_sp]-2];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_subuc:
      stack[reg[reg_sp]-2]=(nightVM_uc)(((nightVM_uc)stack[reg[reg_sp]-1]+0u)-((nightVM_uc)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_subus:
      stack[reg[reg_sp]-2]=(nightVM_us)(((nightVM_us)stack[reg[reg_sp]-1]+0u)-((nightVM_us)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_subui:
      stack[reg[reg_sp]-2]=(nightVM_ui)(((nightVM_ui)stack[reg[reg_sp]-1]+0u)-((nightVM_ui)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_subc:
      stack[reg[reg_sp]-2]=(nightVM_c)((nightVM_c)stack[reg[reg_sp]-1]-(nightVM_c)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_subs:
      stack[reg[reg_sp]-2]=(nightVM_s)((nightVM_s)stack[reg[reg_sp]-1]-(nightVM_s)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_subi:
      stack[reg[reg_sp]-2]=(nightVM_i)((nightVM_i)stack[reg[reg_sp]-1]-(nightVM_i)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_subl:
      stack[reg[reg_sp]-2]=stack[reg[reg_sp]-1]-stack[reg[reg_sp]-2];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_muluc:
      stack[reg[reg_sp]-2]=(nightVM_uc)(((nightVM_uc)stack[reg[reg_sp]-1]+0u)*((nightVM_uc)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_mulus:
      stack[reg[reg_sp]-2]=(nightVM_us)(((nightVM_us)stack[reg[reg_sp]-1]+0u)*((nightVM_us)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_mului:
      stack[reg[reg_sp]-2]=(nightVM_ui)(((nightVM_ui)stack[reg[reg_sp]-1]+0u)*((nightVM_ui)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_mulc:
      stack[reg[reg_sp]-2]=(nightVM_c)((nightVM_c)stack[reg[reg_sp]-1]*(nightVM_c)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_muls:
      stack[reg[reg_sp]-2]=(nightVM_s)((nightVM_s)stack[reg[reg_sp]-1]*(nightVM_s)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_muli:
      stack[reg[reg_sp]-2]=(nightVM_i)((nightVM_i)stack[reg[reg_sp]-1]*(nightVM_i)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_mull:
      stack[reg[reg_sp]-2]=stack[reg[reg_sp]-1]*stack[reg[reg_sp]-2];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_remuc:
      stack[reg[reg_sp]-2]=(nightVM_uc)(((nightVM_uc)stack[reg[reg_sp]-1]+0u)%((nightVM_uc)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_remus:
      stack[reg[reg_sp]-2]=(nightVM_us)(((nightVM_us)stack[reg[reg_sp]-1]+0u)%((nightVM_us)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_remui:
      stack[reg[reg_sp]-2]=(nightVM_ui)(((nightVM_ui)stack[reg[reg_sp]-1]+0u)%((nightVM_ui)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_remc:
      stack[reg[reg_sp]-2]=(nightVM_c)((nightVM_c)stack[reg[reg_sp]-1]%(nightVM_c)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_rems:
      stack[reg[reg_sp]-2]=(nightVM_s)((nightVM_s)stack[reg[reg_sp]-1]%(nightVM_s)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_remi:
      stack[reg[reg_sp]-2]=(nightVM_i)((nightVM_i)stack[reg[reg_sp]-1]%(nightVM_i)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_reml:
      stack[reg[reg_sp]-2]=stack[reg[reg_sp]-1]%stack[reg[reg_sp]-2];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_divuc:
      stack[reg[reg_sp]-2]=(nightVM_uc)(((nightVM_uc)stack[reg[reg_sp]-1]+0u)/((nightVM_uc)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_divus:
      stack[reg[reg_sp]-2]=(nightVM_us)(((nightVM_us)stack[reg[reg_sp]-1]+0u)/((nightVM_us)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_divui:
      stack[reg[reg_sp]-2]=(nightVM_ui)(((nightVM_ui)stack[reg[reg_sp]-1]+0u)/((nightVM_ui)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_divc:
      stack[reg[reg_sp]-2]=(nightVM_c)((nightVM_c)stack[reg[reg_sp]-1]/(nightVM_c)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_divs:
      stack[reg[reg_sp]-2]=(nightVM_s)((nightVM_s)stack[reg[reg_sp]-1]/(nightVM_s)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_divi:
      stack[reg[reg_sp]-2]=(nightVM_i)((nightVM_i)stack[reg[reg_sp]-1]/(nightVM_i)stack[reg[reg_sp]-2]);
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_divl:
      stack[reg[reg_sp]-2]=stack[reg[reg_sp]-1]/stack[reg[reg_sp]-2];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_lshuc:
      stack[reg[reg_sp]-2]=(((nightVM_uc)stack[reg[reg_sp]-1]+0u)<<((nightVM_uc)stack[reg[reg_sp]-2]+0u))&0xff;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_lshus:
      stack[reg[reg_sp]-2]=(((nightVM_us)stack[reg[reg_sp]-1]+0u)<<((nightVM_us)stack[reg[reg_sp]-2]+0u))&0xffff;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_lshui:
      stack[reg[reg_sp]-2]=(((nightVM_ui)stack[reg[reg_sp]-1]+0u)<<((nightVM_ui)stack[reg[reg_sp]-2]+0u))&0xffffff;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_lshc:
      c=stack[reg[reg_sp]-1];
      c_1=stack[reg[reg_sp]-2];
      memcpy(&uc,&c,sizeof(nightVM_uc));
      memcpy(&uc_1,&c_1,sizeof(nightVM_uc));
      uc=((uc+0u)<<(uc_1+0u))&0xff;
      memcpy(&c,&uc,sizeof(nightVM_c));
      stack[reg[reg_sp]-2]=c;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_lshs:
      s=stack[reg[reg_sp]-1];
      s_1=stack[reg[reg_sp]-2];
      memcpy(&us,&s,sizeof(nightVM_us));
      memcpy(&us_1,&s_1,sizeof(nightVM_us));
      us=((us+0u)<<(us_1+0u))&0xffff;
      memcpy(&s,&us,sizeof(nightVM_s));
      stack[reg[reg_sp]-2]=s;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_lshi:
      i=stack[reg[reg_sp]-1];
      i_1=stack[reg[reg_sp]-2];
      memcpy(&ui,&i,sizeof(nightVM_ui));
      memcpy(&ui_1,&i_1,sizeof(nightVM_ui));
      ui=((ui+0u)<<(ui_1+0u))&0xffffff;
      memcpy(&i,&ui,sizeof(nightVM_i));
      stack[reg[reg_sp]-2]=i;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_lshl:
      l=stack[reg[reg_sp]-1];
      l_1=stack[reg[reg_sp]-2];
      memcpy(&uns,&l,sizeof(nightVM_uns));
      memcpy(&uns_1,&l_1,sizeof(nightVM_uns));
      uns=((uns+0u)<<(uns_1+0u))&0xffffffff;
      memcpy(&l,&uns,sizeof(nightVM_l));
      stack[reg[reg_sp]-2]=l;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_rshuc:
      stack[reg[reg_sp]-2]=(((nightVM_uc)stack[reg[reg_sp]-1]+0u)>>((nightVM_uc)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_rshus:
      stack[reg[reg_sp]-2]=(((nightVM_us)stack[reg[reg_sp]-1]+0u)>>((nightVM_us)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_rshui:
      stack[reg[reg_sp]-2]=(((nightVM_ui)stack[reg[reg_sp]-1]+0u)>>((nightVM_ui)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_rshc:
      c=stack[reg[reg_sp]-1];
      c_1=stack[reg[reg_sp]-2];
      memcpy(&uc,&c,sizeof(nightVM_uc));
      memcpy(&uc_1,&c_1,sizeof(nightVM_uc));
      uc=((uc+0u)>>(uc_1+0u));
      memcpy(&c,&uc,sizeof(nightVM_c));
      stack[reg[reg_sp]-2]=c;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_rshs:
      s=stack[reg[reg_sp]-1];
      s_1=stack[reg[reg_sp]-2];
      memcpy(&us,&s,sizeof(nightVM_us));
      memcpy(&us_1,&s_1,sizeof(nightVM_us));
      us=((us+0u)>>(us_1+0u));
      memcpy(&s,&us,sizeof(nightVM_s));
      stack[reg[reg_sp]-2]=s;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_rshi:
      i=stack[reg[reg_sp]-1];
      i_1=stack[reg[reg_sp]-2];
      memcpy(&ui,&i,sizeof(nightVM_ui));
      memcpy(&ui_1,&i_1,sizeof(nightVM_ui));
      ui=((ui+0u)>>(ui_1+0u));
      memcpy(&i,&ui,sizeof(nightVM_i));
      stack[reg[reg_sp]-2]=i;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_rshl:
      l=stack[reg[reg_sp]-1];
      l_1=stack[reg[reg_sp]-2];
      memcpy(&uns,&l,sizeof(nightVM_uns));
      memcpy(&uns_1,&l_1,sizeof(nightVM_uns));
      uns=((uns+0u)>>(uns_1+0u));
      memcpy(&l,&uns,sizeof(nightVM_l));
      stack[reg[reg_sp]-2]=l;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_anduc:
      stack[reg[reg_sp]-2]=(((nightVM_uc)stack[reg[reg_sp]-1]+0u)&((nightVM_uc)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_andus:
      stack[reg[reg_sp]-2]=(((nightVM_us)stack[reg[reg_sp]-1]+0u)&((nightVM_us)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_andui:
      stack[reg[reg_sp]-2]=(((nightVM_ui)stack[reg[reg_sp]-1]+0u)&((nightVM_ui)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_andc:
      c=stack[reg[reg_sp]-1];
      c_1=stack[reg[reg_sp]-2];
      memcpy(&uc,&c,sizeof(nightVM_uc));
      memcpy(&uc_1,&c_1,sizeof(nightVM_uc));
      uc=((uc+0u)&(uc_1+0u));
      memcpy(&c,&uc,sizeof(nightVM_c));
      stack[reg[reg_sp]-2]=c;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_ands:
      s=stack[reg[reg_sp]-1];
      s_1=stack[reg[reg_sp]-2];
      memcpy(&us,&s,sizeof(nightVM_us));
      memcpy(&us_1,&s_1,sizeof(nightVM_us));
      us=((us+0u)&(us_1+0u));
      memcpy(&s,&us,sizeof(nightVM_s));
      stack[reg[reg_sp]-2]=s;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_andi:
      i=stack[reg[reg_sp]-1];
      i_1=stack[reg[reg_sp]-2];
      memcpy(&ui,&i,sizeof(nightVM_ui));
      memcpy(&ui_1,&i_1,sizeof(nightVM_ui));
      ui=((ui+0u)&(ui_1+0u));
      memcpy(&i,&ui,sizeof(nightVM_i));
      stack[reg[reg_sp]-2]=i;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_andl:
      l=stack[reg[reg_sp]-1];
      l_1=stack[reg[reg_sp]-2];
      memcpy(&uns,&l,sizeof(nightVM_uns));
      memcpy(&uns_1,&l_1,sizeof(nightVM_uns));
      uns=((uns+0u)&(uns_1+0u));
      memcpy(&l,&uns,sizeof(nightVM_l));
      stack[reg[reg_sp]-2]=l;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_oruc:
      stack[reg[reg_sp]-2]=(((nightVM_uc)stack[reg[reg_sp]-1]+0u)|((nightVM_uc)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_orus:
      stack[reg[reg_sp]-2]=(((nightVM_us)stack[reg[reg_sp]-1]+0u)|((nightVM_us)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_orui:
      stack[reg[reg_sp]-2]=(((nightVM_ui)stack[reg[reg_sp]-1]+0u)|((nightVM_ui)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_orc:
      c=stack[reg[reg_sp]-1];
      c_1=stack[reg[reg_sp]-2];
      memcpy(&uc,&c,sizeof(nightVM_uc));
      memcpy(&uc_1,&c_1,sizeof(nightVM_uc));
      uc=((uc+0u)|(uc_1+0u));
      memcpy(&c,&uc,sizeof(nightVM_c));
      stack[reg[reg_sp]-2]=c;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_ors:
      s=stack[reg[reg_sp]-1];
      s_1=stack[reg[reg_sp]-2];
      memcpy(&us,&s,sizeof(nightVM_us));
      memcpy(&us_1,&s_1,sizeof(nightVM_us));
      us=((us+0u)|(us_1+0u));
      memcpy(&s,&us,sizeof(nightVM_s));
      stack[reg[reg_sp]-2]=s;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_ori:
      i=stack[reg[reg_sp]-1];
      i_1=stack[reg[reg_sp]-2];
      memcpy(&ui,&i,sizeof(nightVM_ui));
      memcpy(&ui_1,&i_1,sizeof(nightVM_ui));
      ui=((ui+0u)|(ui_1+0u));
      memcpy(&i,&ui,sizeof(nightVM_i));
      stack[reg[reg_sp]-2]=i;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_orl:
      l=stack[reg[reg_sp]-1];
      l_1=stack[reg[reg_sp]-2];
      memcpy(&uns,&l,sizeof(nightVM_uns));
      memcpy(&uns_1,&l_1,sizeof(nightVM_uns));
      uns=((uns+0u)|(uns_1+0u));
      memcpy(&l,&uns,sizeof(nightVM_l));
      stack[reg[reg_sp]-2]=l;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_xoruc:
      stack[reg[reg_sp]-2]=(((nightVM_uc)stack[reg[reg_sp]-1]+0u)^((nightVM_uc)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_xorus:
      stack[reg[reg_sp]-2]=(((nightVM_us)stack[reg[reg_sp]-1]+0u)^((nightVM_us)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_xorui:
      stack[reg[reg_sp]-2]=(((nightVM_ui)stack[reg[reg_sp]-1]+0u)^((nightVM_ui)stack[reg[reg_sp]-2]+0u));
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_xorc:
      c=stack[reg[reg_sp]-1];
      c_1=stack[reg[reg_sp]-2];
      memcpy(&uc,&c,sizeof(nightVM_uc));
      memcpy(&uc_1,&c_1,sizeof(nightVM_uc));
      uc=((uc+0u)^(uc_1+0u));
      memcpy(&c,&uc,sizeof(nightVM_c));
      stack[reg[reg_sp]-2]=c;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_xors:
      s=stack[reg[reg_sp]-1];
      s_1=stack[reg[reg_sp]-2];
      memcpy(&us,&s,sizeof(nightVM_us));
      memcpy(&us_1,&s_1,sizeof(nightVM_us));
      us=((us+0u)^(us_1+0u));
      memcpy(&s,&us,sizeof(nightVM_s));
      stack[reg[reg_sp]-2]=s;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_xori:
      i=stack[reg[reg_sp]-1];
      i_1=stack[reg[reg_sp]-2];
      memcpy(&ui,&i,sizeof(nightVM_ui));
      memcpy(&ui_1,&i_1,sizeof(nightVM_ui));
      ui=((ui+0u)^(ui_1+0u));
      memcpy(&i,&ui,sizeof(nightVM_i));
      stack[reg[reg_sp]-2]=i;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_xorl:
      l=stack[reg[reg_sp]-1];
      l_1=stack[reg[reg_sp]-2];
      memcpy(&uns,&l,sizeof(nightVM_uns));
      memcpy(&uns_1,&l_1,sizeof(nightVM_uns));
      uns=((uns+0u)^(uns_1+0u));
      memcpy(&l,&uns,sizeof(nightVM_l));
      stack[reg[reg_sp]-2]=l;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_notuc:
      stack[reg[reg_sp]-1]=~((nightVM_uc)stack[reg[reg_sp]-1]+0u)&0xff;
      reg[reg_pc]++;
      break;
    case op_notus:
      stack[reg[reg_sp]-1]=~((nightVM_us)stack[reg[reg_sp]-1]+0u)&0xffff;
      reg[reg_pc]++;
      break;
    case op_notui:
      stack[reg[reg_sp]-1]=~((nightVM_ui)stack[reg[reg_sp]-1]+0u)&0xffffff;
      reg[reg_pc]++;
      break;
    case op_notc:
      c=stack[reg[reg_sp]-1];
      memcpy(&uc,&c,sizeof(nightVM_uc));
      uc=~(uc+0u)&0xff;
      memcpy(&c,&uc,sizeof(nightVM_c));
      stack[reg[reg_sp]-1]=c;
      reg[reg_pc]++;
      break;
    case op_nots:
      s=stack[reg[reg_sp]-1];
      memcpy(&us,&s,sizeof(nightVM_us));
      us=~(us+0u)&0xffff;
      memcpy(&s,&us,sizeof(nightVM_s));
      stack[reg[reg_sp]-1]=s;
      reg[reg_pc]++;
      break;
    case op_noti:
      i=stack[reg[reg_sp]-1];
      memcpy(&ui,&i,sizeof(nightVM_ui));
      ui=~(ui+0u)&0xffffff;
      memcpy(&i,&ui,sizeof(nightVM_i));
      stack[reg[reg_sp]-1]=i;
      reg[reg_pc]++;
      break;
    case op_notl:
      l=stack[reg[reg_sp]-1];
      memcpy(&uns,&l,sizeof(nightVM_uns));
      uns=~(uns+0u)&0xffffffff;
      memcpy(&l,&uns,sizeof(nightVM_l));
      stack[reg[reg_sp]-1]=l;
      reg[reg_pc]++;
      break;
    case op_ldcuc: case op_aldcuc:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      memcpy(&uc,&code_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_uc));
      stack[reg[reg_sp]-1]=uc;
      reg[reg_pc]++;
      break;
    case op_ldcus: case op_aldcus:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      memcpy(&us,&code_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_us));
      stack[reg[reg_sp]-1]=us;
      reg[reg_pc]++;
      break;
    case op_ldcui: case op_aldcui:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      memcpy(&ui,&code_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_ui));
      stack[reg[reg_sp]-1]=ui;
      reg[reg_pc]++;
      break;
    case op_ldcc: case op_aldcc:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      memcpy(&c,&code_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_c));
      stack[reg[reg_sp]-1]=c;
      reg[reg_pc]++;
      break;
    case op_ldcs: case op_aldcs:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      memcpy(&s,&code_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_s));
      stack[reg[reg_sp]-1]=s;
      reg[reg_pc]++;
      break;
    case op_ldci: case op_aldci:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      memcpy(&i,&code_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_i));
      stack[reg[reg_sp]-1]=i;
      reg[reg_pc]++;
      break;
    case op_ldcl: case op_aldcl:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      memcpy(&stack[reg[reg_sp]-1],&code_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_l));
      reg[reg_pc]++;
      break;
    case op_ldhuc: case op_aldhuc:
      memcpy(&uc,&heap_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_uc));
      stack[reg[reg_sp]-1]=uc;
      reg[reg_pc]++;
      break;
    case op_ldhus: case op_aldhus:
      memcpy(&us,&heap_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_us));
      stack[reg[reg_sp]-1]=us;
      reg[reg_pc]++;
      break;
    case op_ldhui: case op_aldhui:
      memcpy(&ui,&heap_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_ui));
      stack[reg[reg_sp]-1]=ui;
      reg[reg_pc]++;
      break;
    case op_ldhc: case op_aldhc:
      memcpy(&c,&heap_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_c));
      stack[reg[reg_sp]-1]=c;
      reg[reg_pc]++;
      break;
    case op_ldhs: case op_aldhs:
      memcpy(&s,&heap_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_s));
      stack[reg[reg_sp]-1]=s;
      reg[reg_pc]++;
      break;
    case op_ldhi: case op_aldhi:
      memcpy(&i,&heap_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_i));
      stack[reg[reg_sp]-1]=i;
      reg[reg_pc]++;
      break;
    case op_ldhl: case op_aldhl:
      memcpy(&stack[reg[reg_sp]-1],&heap_uc[stack[reg[reg_sp]-1]],sizeof(nightVM_l));
      reg[reg_pc]++;
      break;
    case op_sthuc: case op_asthuc:
      uc=stack[reg[reg_sp]-2];
      memcpy(&heap_uc[stack[reg[reg_sp]-1]],&uc,sizeof(nightVM_uc));
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    case op_sthus: case op_asthus:
      us=stack[reg[reg_sp]-2];
      memcpy(&heap_uc[stack[reg[reg_sp]-1]],&us,sizeof(nightVM_us));
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    case op_sthui: case op_asthui:
      ui=stack[reg[reg_sp]-2];
      memcpy(&heap_uc[stack[reg[reg_sp]-1]],&ui,sizeof(nightVM_ui));
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    case op_sthc: case op_asthc:
      c=stack[reg[reg_sp]-2];
      memcpy(&heap_uc[stack[reg[reg_sp]-1]],&c,sizeof(nightVM_c));
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    case op_sths: case op_asths:
      s=stack[reg[reg_sp]-2];
      memcpy(&heap_uc[stack[reg[reg_sp]-1]],&s,sizeof(nightVM_s));
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    case op_sthi: case op_asthi:
      i=stack[reg[reg_sp]-2];
      memcpy(&heap_uc[stack[reg[reg_sp]-1]],&i,sizeof(nightVM_i));
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    case op_sthl: case op_asthl:
      l=stack[reg[reg_sp]-2];
      memcpy(&heap_uc[stack[reg[reg_sp]-1]],&l,sizeof(nightVM_l));
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    case op_ldr0:
      stack[reg[reg_sp]]=reg[reg_gpr0];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_ldr1:
      stack[reg[reg_sp]]=reg[reg_gpr0+1];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_ldr2:
      stack[reg[reg_sp]]=reg[reg_gpr0+2];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_ldr3:
      stack[reg[reg_sp]]=reg[reg_gpr0+3];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_ldr4:
      stack[reg[reg_sp]]=reg[reg_gpr0+4];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_ldr5:
      stack[reg[reg_sp]]=reg[reg_gpr0+5];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_ldr6:
      stack[reg[reg_sp]]=reg[reg_gpr0+6];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_ldr7:
      stack[reg[reg_sp]]=reg[reg_gpr0+7];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_str0:
      reg[reg_gpr0]=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_str1:
      reg[reg_gpr0+1]=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_str2:
      reg[reg_gpr0+2]=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_str3:
      reg[reg_gpr0+3]=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_str4:
      reg[reg_gpr0+4]=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_str5:
      reg[reg_gpr0+5]=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_str6:
      reg[reg_gpr0+6]=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_str7:
      reg[reg_gpr0+7]=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_panic:
      if(stack[reg[reg_sp]-1]){
        *exit_status=1;
        return err_panic;
      }
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_force_panic:
      *exit_status=1;
      return err_panic;
    case op_incsp:
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_decsp:
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_swap:
      l=stack[reg[reg_sp]-1];
      stack[reg[reg_sp]-1]=stack[reg[reg_sp]-2];
      stack[reg[reg_sp]-2]=l;
      reg[reg_pc]++;
      break;
    case op_pop:
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_copy:
      stack[reg[reg_sp]-1]=stack[stack[reg[reg_sp]-1]];
      reg[reg_pc]++;
      break;
    case op_pcopy:
      l=stack[reg[reg_sp]-1];
      memmove(&stack[stack[reg[reg_sp]-1]],&stack[stack[reg[reg_sp]-1]+1],reg[reg_sp]*sizeof(nightVM_l));
      stack[reg[reg_sp]-2]=l;
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_popa:
      l=stack[reg[reg_sp]-1];
      memmove(&stack[stack[reg[reg_sp]-1]],&stack[stack[reg[reg_sp]-1]+1],reg[reg_sp]*sizeof(nightVM_l));
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    case op_dup:
      stack[reg[reg_sp]]=stack[reg[reg_sp]-1];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_over:
      stack[reg[reg_sp]]=stack[reg[reg_sp]-2];
      reg[reg_sp]++;
      reg[reg_pc]++;
      break;
    case op_rjgt:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]>stack[reg[reg_sp]-3]){
        reg[reg_pc]+=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_rjls:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]<stack[reg[reg_sp]-3]){
        reg[reg_pc]+=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_rjeq:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]==stack[reg[reg_sp]-3]){
        reg[reg_pc]+=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_rjle:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]<=stack[reg[reg_sp]-3]){
        reg[reg_pc]+=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_rjge:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]>=stack[reg[reg_sp]-3]){
        reg[reg_pc]+=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_rjne:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]!=stack[reg[reg_sp]-3]){
        reg[reg_pc]+=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_rjz:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]==0){
        reg[reg_pc]+=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=2;
      break;
    case op_rjnz:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]!=0){
        reg[reg_pc]+=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=2;
      break;
    case op_rjmp:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      reg[reg_pc]+=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      break;
    case op_jgt:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]>stack[reg[reg_sp]-3]){
        reg[reg_pc]=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_jls:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]<stack[reg[reg_sp]-3]){
        reg[reg_pc]=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_jeq:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]==stack[reg[reg_sp]-3]){
        reg[reg_pc]=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_jle:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]<=stack[reg[reg_sp]-3]){
        reg[reg_pc]=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_jge:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]>=stack[reg[reg_sp]-3]){
        reg[reg_pc]=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_jne:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]!=stack[reg[reg_sp]-3]){
        reg[reg_pc]=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=3;
      break;
    case op_jz:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]==0){
        reg[reg_pc]=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=2;
      break;
    case op_jnz:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if(stack[reg[reg_sp]-2]!=0){
        reg[reg_pc]=stack[reg[reg_sp]-1];
      }
      else{
        reg[reg_pc]++;
      }
      reg[reg_sp]-=2;
      break;
    case op_jmp:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      reg[reg_pc]=stack[reg[reg_sp]-1];
      reg[reg_sp]--;
      break;
    case op_nop:
      reg[reg_pc]++;
      break;
    case op_open:
      if(stack[reg[reg_sp]-1]>=reg[reg_cs] || stack[reg[reg_sp]-1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]-1];
        return err_trap;
      }
      if((str=cseq2str(&((nightVM_c *)*code)[stack[reg[reg_sp]-1]]))==NULL){
        *exit_status=1;
        return err_failed_allocation;
      }
      if(realpath(str,resolved_name)==NULL){
        free(str);
        *exit_status=1;
        return err_open;
      }
      free(str);
      hash_table_entry.key=resolved_name;
      if(hsearch(hash_table_entry,FIND)==NULL){
        hash_table_entry.data=dlopen(hash_table_entry.key,RTLD_LAZY|RTLD_LOCAL);
        if(hash_table_entry.data==NULL){
          *exit_status=1;
          return err_open;
        }
        opened_libs[*lib_pt]=hash_table_entry.data;
        if((lib_names[*lib_pt]=malloc(strlen(hash_table_entry.key)+1))==NULL){
          *exit_status=1;
          return err_failed_allocation;
        }
        strcpy(lib_names[*lib_pt],resolved_name);
        hash_table_entry.key=lib_names[*lib_pt];
        (*lib_pt)++;
        opened_libs[*lib_pt]=NULL;
        if(hsearch(hash_table_entry,ENTER)==NULL){
          *exit_status=1;
          return err_hash_table_enter;
        }
      }
      reg[reg_sp]--;
      reg[reg_pc]++;
      break;
    case op_invoke:
      reg[reg_sp]-=2;
      if(stack[reg[reg_sp]]>=reg[reg_cs] || stack[reg[reg_sp]]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]];
        return err_trap;
      }
      if(stack[reg[reg_sp]+1]>=reg[reg_cs] || stack[reg[reg_sp]+1]<reg[reg_cb]){
        *exit_status=1;
        reg[reg_ia]=stack[reg[reg_sp]+1];
        return err_trap;
      }
      if((str=cseq2str(&((nightVM_c *)*code)[stack[reg[reg_sp]]]))==NULL){
        *exit_status=1;
        return err_failed_allocation;
      }
      if(realpath(str,resolved_name)==NULL){
        free(str);
        *exit_status=1;
        return err_invoke;
      }
      free(str);
      hash_table_entry.key=resolved_name;
      if((hash_table_entry_ret=hsearch(hash_table_entry,FIND))!=NULL){
        void *object=hash_table_entry_ret->data;
        if((str=cseq2str(&((nightVM_c *)*code)[stack[reg[reg_sp]+1]]))==NULL){
          *exit_status=1;
          return err_failed_allocation;
        }
        hash_table_entry.key=str;
        if((hash_table_entry_ret=hsearch(hash_table_entry,FIND))==NULL){
          hash_table_entry.data=dlsym(object,hash_table_entry.key);
          if(hash_table_entry.data==NULL){
            free(str);
            *exit_status=1;
            return err_invoke;
          }
          if((sym_names[*sym_pt]=malloc(strlen(hash_table_entry.key)+1))==NULL){
            free(str);
            *exit_status=1;
            return err_failed_allocation;
          }
          strcpy(sym_names[*sym_pt],hash_table_entry.key);
          hash_table_entry.key=sym_names[*sym_pt];
          free(str);
          (*sym_pt)++;
          if(hsearch(hash_table_entry,ENTER)==NULL){
            *exit_status=1;
            return err_hash_table_enter;
          }
          hash_table_entry_ret=&hash_table_entry;
        }
        else{
          free(str);
        }
        symbol=(void (*)(int argc, char **argv, nightVM_l *stack, void **code, nightVM_ui code_align, void **heap, nightVM_ui heap_align, nightVM_l reg_ssz_val, nightVM_l *reg_hsz_val, nightVM_l *reg_sp_val, nightVM_l *reg_cs_val, nightVM_l reg_pc_val, nightVM_l *gpr))hash_table_entry_ret->data;
        symbol(argc,argv,stack,code,code_align,heap,heap_align,reg[reg_ssz],&reg[reg_hsz],&reg[reg_sp],&reg[reg_cs],reg[reg_pc],&reg[reg_gpr0]);
        reg[reg_pc]++;
        break;
      }
      else{
        *exit_status=1;
        return err_invoke;
      }
      reg[reg_sp]-=2;
      reg[reg_pc]++;
      break;
    default:
      *exit_status=1;
      return err_illegal_instruction;
    }
  }
  return 0;
}
