#ifndef NVM_IMPLEMENTATION_H
#define NVM_IMPLEMENTATION_H

enum regs {
  REG_PC,
  REG_SP,
  REG_CS,
  REG_HSZ,
  REG_SSZ,
  REG_CB,
  REG_IA,
  REG_CLP,
  REG_LOP,
  REG_GPR0,
  REG_GPR1,
  REG_GPR2,
  REG_GPR3,
  REG_GPR4,
  REG_GPR5,
  REG_GPR6,
  REG_GPR7
};

enum load_type {
  LT_NEMBED,
  LT_EMBED
};

#define MAX_CODE_BASE 0x400000
#define MAX_STACK_BASE 0x400000

enum error {
  ERR_SUCCESS,
  ERR_PANIC,
  ERR_OPEN,
  ERR_INVOKE,
  ERR_ILLEGAL_INSTRUCTION,
  ERR_TRAP,
  ERR_FAILED_ALLOCATION,
  ERR_FAILED_TO_OPEN_FILE_FOR_READING,
  ERR_FAILED_TO_READ_FROM_FILE,
  ERR_INVALID_FILE_FORMAT,
  ERR_INVALID_MAGIC,
  ERR_INVALID_SPECIFICATION_VERSION,
  ERR_FAILED_TO_CREATE_HASH_TABLE,
  ERR_HASH_TABLE_ENTER,
  ERR_OTHER
};

typedef unsigned long nightVM_uns;
typedef long nightVM_l;
typedef unsigned int nightVM_ui;
typedef int nightVM_i;
typedef unsigned short nightVM_us;
typedef short nightVM_s;
typedef unsigned char nightVM_uc;
typedef char nightVM_c;

#define NVMUNS_MAX ULONG_MAX
#define NVML_MAX LONG_MAX
#define NVML_MIN LONG_MIN
#define NVMUI_MAX UINT_MAX
#define NVMI_MAX INT_MAX
#define NVMI_MIN INT_MIN
#define NVMUS_MAX USHORT_MAX
#define NVMS_MAX SHORT_MAX
#define NVMS_MIN SHORT_MIN
#define NVMUC_MAX UCHAR_MAX
#define NVMC_MAX CHAR_MAX
#define NVMC_MIN CHAR_MIN

#define PRINVMUNS "lu"
#define PRINVMxUNS "lx"
#define PRINVMoUNS "lo"
#define PRINVML "ld"
#define PRINVMUI "u"
#define PRINVMxUI "x"
#define PRINVMoUI "o"
#define PRINVMI "d"
#define PRINVMUS "hu"
#define PRINVMxUS "hx"
#define PRINVMoUS "ho"
#define PRINVMS "hd"
#define PRINVMUC "hhu"
#define PRINVMxUC "hhx"
#define PRINVMoUC "hho"
#define PRINVMC "hhd"

#define SCNNVMUNS "lu"
#define SCNNVMxUNS "lx"
#define SCNNVMoUNS "lo"
#define SCNNVML "ld"
#define SCNNVMUI "u"
#define SCNNVMxUI "x"
#define SCNNVMoUI "o"
#define SCNNVMI "d"
#define SCNNVMUS "hu"
#define SCNNVMxUS "hx"
#define SCNNVMoUS "ho"
#define SCNNVMS "hd"
#define SCNNVMUC "hhu"
#define SCNNVMxUC "hhx"
#define SCNNVMoUC "hho"
#define SCNNVMC "hhd"

#define SIZEOF_UNS sizeof(nightVM_uns)/sizeof(nightVM_uc)
#define SIZEOF_UC 1
#define SIZEOF_US sizeof(nightVM_us)/sizeof(nightVM_uc)
#define SIZEOF_UI sizeof(nightVM_ui)/sizeof(nightVM_uc)
#define SIZEOF_C 1
#define SIZEOF_S sizeof(nightVM_s)/sizeof(nightVM_uc)
#define SIZEOF_I sizeof(nightVM_i)/sizeof(nightVM_uc)
#define SIZEOF_L sizeof(nightVM_l)/sizeof(nightVM_uc)

#define ALIGNOF_UNS _Alignof(nightVM_uns)/sizeof(nightVM_uc)
#define ALIGNOF_UC 1
#define ALIGNOF_US _Alignof(nightVM_us)/sizeof(nightVM_uc)
#define ALIGNOF_UI _Alignof(nightVM_ui)/sizeof(nightVM_uc)
#define ALIGNOF_C 1
#define ALIGNOF_S _Alignof(nightVM_s)/sizeof(nightVM_uc)
#define ALIGNOF_I _Alignof(nightVM_i)/sizeof(nightVM_uc)
#define ALIGNOF_L _Alignof(nightVM_l)/sizeof(nightVM_uc)

static size_t cseqlen(nightVM_c *cseq){
  size_t len = 0;

  for(; cseq[len] != 0; len++);

  return len;
}

static char *cseq2str(nightVM_c *cseq){
  size_t len = cseqlen(cseq);

  char *str;
  if((str = malloc(len + 1)) == NULL){
    return NULL;
  }

  for(size_t i = 0; i < len; i++){
    str[i] = cseq[i];
  }

  str[len] = '\0';

  return str;
}

static nightVM_c *str2cseq(char *str){
  size_t len = strlen(str);

  nightVM_c *cseq;
  if((cseq = malloc(sizeof(nightVM_uc) * (len + 1))) == NULL){
    return NULL;
  }

  for(size_t i = 0; i < len; i++){
    cseq[i] = str[i];
  }

  cseq[len]=0;

  return cseq;
}

static nightVM_c *cseqcpystr(nightVM_c *cseq, char *str, size_t size){
  for(size_t i = 0;i < size && str[i] != '\0'; i++){
    cseq[i] = str[i];
    cseq[i + 1] = 0;
  }

  return cseq;
}

static char *strcpycseq(char *str, nightVM_c *cseq, size_t size){
  for(size_t i = 0;i < size && cseq[i] != '\0'; i++){
    str[i] = cseq[i];
    str[i + 1] = 0;
  }

  return str;
}

static nightVM_c *cseqcpycseq(nightVM_c *cseq1, nightVM_c *cseq2, size_t size){
  for(size_t i = 0;i < size && cseq2[i] != 0; i++){
    cseq1[i] = cseq2[i];
    cseq1[i + 1] = 0;
  }

  return cseq1;
}

static int cseqcmpstr(nightVM_c *cseq, char *str){
  size_t lenc = cseqlen(cseq);

  for(size_t i = 0; i <= lenc; i++){
    if(cseq[i] != str[i]){
      return cseq[i] - str[i];
    }
  }

  return 0;
}

static int cseqcmpcseq(nightVM_c *cseq1, nightVM_c *cseq2){
  size_t lenc1 = cseqlen(cseq1);

  for(size_t i = 0; i <= lenc1; i++){
    if(cseq1[i] != cseq2[i]){
      return cseq1[i] - cseq2[i];
    }
  }

  return 0;
}

#endif
