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

typedef unsigned long nightvm_uns;
typedef long ysm_l;
typedef unsigned int ysm_ui;
typedef int ysm_i;
typedef unsigned short ysm_us;
typedef short ysm_s;
typedef unsigned char ysm_uc;
typedef char ysm_c;

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
#define PRIYSML "ld"
#define PRIYSMUI "u"
#define PRIYSMxUI "x"
#define PRIYSMoUI "o"
#define PRIYSMI "d"
#define PRIYSMUS "hu"
#define PRIYSMxUS "hx"
#define PRIYSMoUS "ho"
#define PRIYSMS "hd"
#define PRIYSMUC "hhu"
#define PRIYSMxUC "hhx"
#define PRIYSMoUC "hho"
#define PRIYSMC "hhd"

#define SCNNVMUNS "lu"
#define SCNNVMxUNS "lx"
#define SCNNVMoUNS "lo"
#define SCNYSML "ld"
#define SCNYSMUI "u"
#define SCNYSMxUI "x"
#define SCNYSMoUI "o"
#define SCNYSMI "d"
#define SCNYSMUS "hu"
#define SCNYSMxUS "hx"
#define SCNYSMoUS "ho"
#define SCNYSMS "hd"
#define SCNYSMUC "hhu"
#define SCNYSMxUC "hhx"
#define SCNYSMoUC "hho"
#define SCNYSMC "hhd"

#define SIZEOF_UNS sizeof(nightvm_uns)/sizeof(ysm_uc)
#define SIZEOF_UC 1
#define SIZEOF_US sizeof(ysm_us)/sizeof(ysm_uc)
#define SIZEOF_UI sizeof(ysm_ui)/sizeof(ysm_uc)
#define SIZEOF_C 1
#define SIZEOF_S sizeof(ysm_s)/sizeof(ysm_uc)
#define SIZEOF_I sizeof(ysm_i)/sizeof(ysm_uc)
#define SIZEOF_L sizeof(ysm_l)/sizeof(ysm_uc)

#define ALIGNOF_UNS _Alignof(nightvm_uns)/sizeof(ysm_uc)
#define ALIGNOF_UC 1
#define ALIGNOF_US _Alignof(ysm_us)/sizeof(ysm_uc)
#define ALIGNOF_UI _Alignof(ysm_ui)/sizeof(ysm_uc)
#define ALIGNOF_C 1
#define ALIGNOF_S _Alignof(ysm_s)/sizeof(ysm_uc)
#define ALIGNOF_I _Alignof(ysm_i)/sizeof(ysm_uc)
#define ALIGNOF_L _Alignof(ysm_l)/sizeof(ysm_uc)

static size_t cseqlen(ysm_c *cseq){
  size_t len = 0;

  for(; cseq[len] != 0; len++);

  return len;
}

static char *cseq2str(ysm_c *cseq){
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

static ysm_c *str2cseq(char *str){
  size_t len = strlen(str);

  ysm_c *cseq;
  if((cseq = malloc(sizeof(ysm_uc) * (len + 1))) == NULL){
    return NULL;
  }

  for(size_t i = 0; i < len; i++){
    cseq[i] = str[i];
  }

  cseq[len]=0;

  return cseq;
}

static ysm_c *cseqcpystr(ysm_c *cseq, char *str, size_t size){
  for(size_t i = 0;i < size && str[i] != '\0'; i++){
    cseq[i] = str[i];
    cseq[i + 1] = 0;
  }

  return cseq;
}

static char *strcpycseq(char *str, ysm_c *cseq, size_t size){
  for(size_t i = 0;i < size && cseq[i] != '\0'; i++){
    str[i] = cseq[i];
    str[i + 1] = 0;
  }

  return str;
}

static ysm_c *cseqcpycseq(ysm_c *cseq1, ysm_c *cseq2, size_t size){
  for(size_t i = 0;i < size && cseq2[i] != 0; i++){
    cseq1[i] = cseq2[i];
    cseq1[i + 1] = 0;
  }

  return cseq1;
}

static int cseqcmpstr(ysm_c *cseq, char *str){
  size_t lenc = cseqlen(cseq);

  for(size_t i = 0; i <= lenc; i++){
    if(cseq[i] != str[i]){
      return cseq[i] - str[i];
    }
  }

  return 0;
}

static int cseqcmpcseq(ysm_c *cseq1, ysm_c *cseq2){
  size_t lenc1 = cseqlen(cseq1);

  for(size_t i = 0; i <= lenc1; i++){
    if(cseq1[i] != cseq2[i]){
      return cseq1[i] - cseq2[i];
    }
  }

  return 0;
}

#endif
