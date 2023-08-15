#ifndef NVM_OPCODES_H
#define NVM_OPCODES_H

/* base instruction set */

#define op_halt 0
#define op_pushuc 1
#define op_pushus 2
#define op_pushui 3
#define op_pushc 4
#define op_pushs 5
#define op_pushi 6
#define op_pushl 7
#define op_adduc 8
#define op_addus 9
#define op_addui 10
#define op_addc 11
#define op_adds 12
#define op_addi 13
#define op_addl 14
#define op_subuc 15
#define op_subus 16
#define op_subui 17
#define op_subc 18
#define op_subs 19
#define op_subi 20
#define op_subl 21
#define op_muluc 22
#define op_mulus 23
#define op_mului 24
#define op_mulc 25
#define op_muls 26
#define op_muli 27
#define op_mull 28
#define op_divuc 29
#define op_divus 30
#define op_divui 31
#define op_divc 32
#define op_divs 33
#define op_divi 34
#define op_divl 35
#define op_ldcuc 36
#define op_ldcus 37
#define op_ldcui 38
#define op_ldcc 39
#define op_ldcs 40
#define op_ldci 41
#define op_ldcl 42
#define op_jgt 43
#define op_jls 44
#define op_jeq 45
#define op_jne 46
#define op_jle 47
#define op_jge 48
#define op_jz 49
#define op_jnz 50
#define op_jmp 51
#define op_nop 52
#define op_lshuc 53
#define op_lshus 54
#define op_lshui 55
#define op_lshc 56
#define op_lshs 57
#define op_lshi 58
#define op_lshl 59
#define op_rshuc 60
#define op_rshus 61
#define op_rshui 62
#define op_rshc 63
#define op_rshs 64
#define op_rshi 65
#define op_rshl 66
#define op_anduc 67
#define op_andus 68
#define op_andui 69
#define op_andc 70
#define op_ands 71
#define op_andi 72
#define op_andl 73
#define op_oruc 74
#define op_orus 75
#define op_orui 76
#define op_orc 77
#define op_ors 78
#define op_ori 79
#define op_orl 80
#define op_notuc 81
#define op_notus 82
#define op_notui 83
#define op_notc 84
#define op_nots 85
#define op_noti 86
#define op_notl 87
#define op_xoruc 88
#define op_xorus 89
#define op_xorui 90
#define op_xorc 91
#define op_xors 92
#define op_xori 93
#define op_xorl 94
#define op_remuc 95
#define op_remus 96
#define op_remui 97
#define op_remc 98
#define op_rems 99
#define op_remi 100
#define op_reml 101
#define op_swap 102
#define op_pop 103
#define op_sthuc 104
#define op_sthus 105
#define op_sthui 106
#define op_sthc 107
#define op_sths 108
#define op_sthi 109
#define op_sthl 110
#define op_ldhuc 111
#define op_ldhus 112
#define op_ldhui 113
#define op_ldhc 114
#define op_ldhs 115
#define op_ldhi 116
#define op_ldhl 117
#define op_dup 118
#define op_panic 119
#define op_over 120
#define op_str0 121
#define op_str1 122
#define op_str2 123
#define op_str3 124
#define op_str4 125
#define op_str5 126
#define op_str6 127
#define op_str7 128
#define op_ldr0 129
#define op_ldr1 130
#define op_ldr2 131
#define op_ldr3 132
#define op_ldr4 133
#define op_ldr5 134
#define op_ldr6 135
#define op_ldr7 136
#define op_copy 137
#define op_pcopy 138
#define op_popa 139
#define op_sts 140
#define op_rbs 141
#define op_rbe 142
#define op_haltr 143
#define op_pushsp 144
#define op_aldhuc 145
#define op_aldhus 146
#define op_aldhui 147
#define op_aldhc 148
#define op_aldhs 149
#define op_aldhi 150
#define op_aldhl 151
#define op_asthuc 152
#define op_asthus 153
#define op_asthui 154
#define op_asthc 155
#define op_asths 156
#define op_asthi 157
#define op_asthl 158
#define op_aldcuc 159
#define op_aldcus 160
#define op_aldcui 161
#define op_aldcc 162
#define op_aldcs 163
#define op_aldci 164
#define op_aldcl 165
#define op_incsp 166
#define op_decsp 167
#define op_rjgt 168
#define op_rjls 169
#define op_rjeq 170
#define op_rjne 171
#define op_rjle 172
#define op_rjge 173
#define op_rjz 174
#define op_rjnz 175
#define op_rjmp 176
#define op_pushpc 177
#define op_pushcs 178
#define op_pushlt 179
#define op_force_panic 180

/* extended instruction set */

#define op_open 181
#define op_invoke 182
#define op_trap 183
#define op_pushn1 184
#define op_push0 185
#define op_push1 186
#define op_push2 187
#define op_push3 188
#define op_push4 189
#define op_push5 190
#define op_push7 191

#endif
