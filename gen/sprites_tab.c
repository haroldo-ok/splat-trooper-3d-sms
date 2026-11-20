#include "../src/defs.h"
#include "spr_bank0.h"
#include "spr_bank1.h"
#include "spr_bank2.h"
#include "spr_bank3.h"
#include "spr_bank4.h"
static const unsigned char pcs_0_0[1] = {
  0,
};
static const unsigned char pcs_0_1[2] = {
  0,1,
};
static const unsigned char pcs_0_2[2] = {
  0,1,
};
static const unsigned char pcs_0_3[3] = {
  16,17,18,
};
static const unsigned char pcs_0_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_0_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_0_6[10] = {
  16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_0_7[16] = {
  1,2,3,4,16,17,18,19,20,21,32,33,34,35,36,37,
};
static const unsigned char pcs_1_0[1] = {
  0,
};
static const unsigned char pcs_1_1[2] = {
  0,1,
};
static const unsigned char pcs_1_2[2] = {
  0,1,
};
static const unsigned char pcs_1_3[3] = {
  16,17,18,
};
static const unsigned char pcs_1_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_1_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_1_6[10] = {
  16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_1_7[14] = {
  2,3,16,17,18,19,20,21,32,33,34,35,36,37,
};
static const unsigned char pcs_2_0[1] = {
  0,
};
static const unsigned char pcs_2_1[2] = {
  0,1,
};
static const unsigned char pcs_2_2[2] = {
  0,1,
};
static const unsigned char pcs_2_3[4] = {
  1,16,17,18,
};
static const unsigned char pcs_2_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_2_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_2_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_2_7[18] = {
  0,1,2,3,4,5,16,17,18,19,20,21,32,33,34,35,36,37,
};
static const unsigned char pcs_3_0[1] = {
  0,
};
static const unsigned char pcs_3_1[2] = {
  0,1,
};
static const unsigned char pcs_3_2[2] = {
  0,1,
};
static const unsigned char pcs_3_3[3] = {
  16,17,18,
};
static const unsigned char pcs_3_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_3_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_3_6[10] = {
  16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_3_7[16] = {
  1,2,3,4,16,17,18,19,20,21,32,33,34,35,36,37,
};
static const unsigned char pcs_4_0[1] = {
  0,
};
static const unsigned char pcs_4_1[2] = {
  0,1,
};
static const unsigned char pcs_4_2[2] = {
  0,1,
};
static const unsigned char pcs_4_3[3] = {
  16,17,18,
};
static const unsigned char pcs_4_4[3] = {
  16,17,18,
};
static const unsigned char pcs_4_5[4] = {
  16,17,18,19,
};
static const unsigned char pcs_4_6[5] = {
  32,33,34,35,36,
};
static const unsigned char pcs_4_7[6] = {
  32,33,34,35,36,37,
};
static const unsigned char pcs_5_0[1] = {
  0,
};
static const unsigned char pcs_5_1[2] = {
  0,1,
};
static const unsigned char pcs_5_2[2] = {
  0,1,
};
static const unsigned char pcs_5_3[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_5_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_5_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_5_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_5_7[16] = {
  1,2,3,4,16,17,18,19,20,21,32,33,34,35,36,37,
};
static const unsigned char pcs_6_0[1] = {
  0,
};
static const unsigned char pcs_6_1[2] = {
  0,1,
};
static const unsigned char pcs_6_2[2] = {
  0,1,
};
static const unsigned char pcs_6_3[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_6_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_6_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_6_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_6_7[15] = {
  1,2,3,4,16,17,18,19,20,21,33,34,35,36,37,
};
static const unsigned char pcs_7_0[1] = {
  0,
};
static const unsigned char pcs_7_1[2] = {
  0,1,
};
static const unsigned char pcs_7_2[2] = {
  0,1,
};
static const unsigned char pcs_7_3[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_7_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_7_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_7_6[13] = {
  0,1,2,3,16,17,18,19,20,33,34,35,36,
};
static const unsigned char pcs_7_7[16] = {
  0,1,2,3,4,16,17,18,19,20,21,33,34,35,36,37,
};
static const unsigned char pcs_8_0[1] = {
  0,
};
static const unsigned char pcs_8_1[2] = {
  0,1,
};
static const unsigned char pcs_8_2[2] = {
  0,1,
};
static const unsigned char pcs_8_3[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_8_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_8_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_8_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_8_7[16] = {
  1,2,3,4,16,17,18,19,20,21,32,33,34,35,36,37,
};
static const unsigned char pcs_9_0[1] = {
  0,
};
static const unsigned char pcs_9_1[2] = {
  0,1,
};
static const unsigned char pcs_9_2[2] = {
  0,1,
};
static const unsigned char pcs_9_3[3] = {
  16,17,18,
};
static const unsigned char pcs_9_4[3] = {
  16,17,18,
};
static const unsigned char pcs_9_5[4] = {
  16,17,18,19,
};
static const unsigned char pcs_9_6[8] = {
  17,18,19,32,33,34,35,36,
};
static const unsigned char pcs_9_7[8] = {
  17,18,19,20,33,34,35,36,
};
static const unsigned char pcs_10_0[1] = {
  0,
};
static const unsigned char pcs_10_1[2] = {
  0,1,
};
static const unsigned char pcs_10_2[2] = {
  0,1,
};
static const unsigned char pcs_10_3[4] = {
  1,16,17,18,
};
static const unsigned char pcs_10_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_10_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_10_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_10_7[15] = {
  1,2,3,4,16,17,18,19,20,21,33,34,35,36,37,
};
static const unsigned char pcs_11_0[1] = {
  0,
};
static const unsigned char pcs_11_1[2] = {
  0,1,
};
static const unsigned char pcs_11_2[2] = {
  0,1,
};
static const unsigned char pcs_11_3[4] = {
  1,16,17,18,
};
static const unsigned char pcs_11_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_11_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_11_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_11_7[15] = {
  1,2,3,4,16,17,18,19,20,21,32,33,34,35,36,
};
static const unsigned char pcs_12_0[1] = {
  0,
};
static const unsigned char pcs_12_1[2] = {
  0,1,
};
static const unsigned char pcs_12_2[2] = {
  0,1,
};
static const unsigned char pcs_12_3[4] = {
  1,16,17,18,
};
static const unsigned char pcs_12_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_12_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_12_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_12_7[14] = {
  1,2,3,4,16,17,18,19,20,21,33,34,35,36,
};
static const unsigned char pcs_13_0[1] = {
  0,
};
static const unsigned char pcs_13_1[2] = {
  0,1,
};
static const unsigned char pcs_13_2[2] = {
  0,1,
};
static const unsigned char pcs_13_3[5] = {
  1,2,16,17,18,
};
static const unsigned char pcs_13_4[5] = {
  1,2,16,17,18,
};
static const unsigned char pcs_13_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_13_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_13_7[14] = {
  2,3,4,16,17,18,19,20,21,33,34,35,36,37,
};
static const unsigned char pcs_14_0[1] = {
  0,
};
static const unsigned char pcs_14_1[2] = {
  0,1,
};
static const unsigned char pcs_14_2[2] = {
  0,1,
};
static const unsigned char pcs_14_3[3] = {
  16,17,18,
};
static const unsigned char pcs_14_4[3] = {
  16,17,18,
};
static const unsigned char pcs_14_5[5] = {
  3,16,17,18,19,
};
static const unsigned char pcs_14_6[7] = {
  17,18,19,20,33,34,35,
};
static const unsigned char pcs_14_7[8] = {
  18,19,20,21,33,34,35,36,
};
static const unsigned char pcs_15_0[1] = {
  0,
};
static const unsigned char pcs_15_1[2] = {
  0,1,
};
static const unsigned char pcs_15_2[2] = {
  0,1,
};
static const unsigned char pcs_15_3[3] = {
  16,17,18,
};
static const unsigned char pcs_15_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_15_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_15_6[10] = {
  16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_15_7[14] = {
  1,2,3,4,16,17,18,19,20,21,33,34,35,36,
};
static const unsigned char pcs_16_0[1] = {
  0,
};
static const unsigned char pcs_16_1[2] = {
  0,1,
};
static const unsigned char pcs_16_2[2] = {
  0,1,
};
static const unsigned char pcs_16_3[3] = {
  16,17,18,
};
static const unsigned char pcs_16_4[5] = {
  0,1,16,17,18,
};
static const unsigned char pcs_16_5[6] = {
  0,1,2,3,17,18,
};
static const unsigned char pcs_16_6[6] = {
  17,18,19,33,34,35,
};
static const unsigned char pcs_16_7[10] = {
  1,2,17,18,19,20,33,34,35,36,
};
static const unsigned char pcs_17_0[1] = {
  0,
};
static const unsigned char pcs_17_1[2] = {
  0,1,
};
static const unsigned char pcs_17_2[2] = {
  0,1,
};
static const unsigned char pcs_17_3[3] = {
  16,17,18,
};
static const unsigned char pcs_17_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_17_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_17_6[8] = {
  16,17,18,19,20,33,34,35,
};
static const unsigned char pcs_17_7[15] = {
  0,1,2,3,4,5,17,18,19,20,21,33,34,35,36,
};
static const unsigned char pcs_18_0[1] = {
  0,
};
static const unsigned char pcs_18_1[2] = {
  0,1,
};
static const unsigned char pcs_18_2[2] = {
  0,1,
};
static const unsigned char pcs_18_3[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_18_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_18_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_18_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_18_7[16] = {
  0,1,2,3,4,5,16,17,18,19,20,21,33,34,35,36,
};
static const unsigned char pcs_19_0[1] = {
  0,
};
static const unsigned char pcs_19_1[2] = {
  0,1,
};
static const unsigned char pcs_19_2[2] = {
  0,1,
};
static const unsigned char pcs_19_3[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_19_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_19_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_19_6[13] = {
  1,2,3,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_19_7[14] = {
  1,2,3,4,16,17,18,19,20,21,33,34,35,36,
};
static const unsigned char pcs_20_0[1] = {
  0,
};
static const unsigned char pcs_20_1[2] = {
  0,1,
};
static const unsigned char pcs_20_2[2] = {
  0,1,
};
static const unsigned char pcs_20_3[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_20_4[6] = {
  0,1,2,16,17,18,
};
static const unsigned char pcs_20_5[8] = {
  0,1,2,3,16,17,18,19,
};
static const unsigned char pcs_20_6[14] = {
  1,2,3,4,16,17,18,19,20,32,33,34,35,36,
};
static const unsigned char pcs_20_7[18] = {
  0,1,2,3,4,5,16,17,18,19,20,21,32,33,34,35,36,37,
};
const SprImg spr_imgs[168] = {
  {7, spr_0_0, 1, 1, 1, pcs_0_0},
  {7, spr_0_1, 2, 2, 1, pcs_0_1},
  {7, spr_0_2, 2, 2, 1, pcs_0_2},
  {7, spr_0_3, 3, 3, 2, pcs_0_3},
  {7, spr_0_4, 6, 3, 2, pcs_0_4},
  {7, spr_0_5, 8, 4, 2, pcs_0_5},
  {7, spr_0_6, 10, 5, 3, pcs_0_6},
  {7, spr_0_7, 16, 6, 3, pcs_0_7},
  {7, spr_1_0, 1, 1, 1, pcs_1_0},
  {7, spr_1_1, 2, 2, 1, pcs_1_1},
  {7, spr_1_2, 2, 2, 1, pcs_1_2},
  {7, spr_1_3, 3, 3, 2, pcs_1_3},
  {7, spr_1_4, 6, 3, 2, pcs_1_4},
  {7, spr_1_5, 8, 4, 2, pcs_1_5},
  {7, spr_1_6, 10, 5, 3, pcs_1_6},
  {7, spr_1_7, 14, 6, 3, pcs_1_7},
  {7, spr_2_0, 1, 1, 1, pcs_2_0},
  {7, spr_2_1, 2, 2, 1, pcs_2_1},
  {7, spr_2_2, 2, 2, 1, pcs_2_2},
  {7, spr_2_3, 4, 3, 2, pcs_2_3},
  {7, spr_2_4, 6, 3, 2, pcs_2_4},
  {7, spr_2_5, 8, 4, 2, pcs_2_5},
  {7, spr_2_6, 13, 5, 3, pcs_2_6},
  {7, spr_2_7, 18, 6, 3, pcs_2_7},
  {7, spr_3_0, 1, 1, 1, pcs_3_0},
  {7, spr_3_1, 2, 2, 1, pcs_3_1},
  {7, spr_3_2, 2, 2, 1, pcs_3_2},
  {7, spr_3_3, 3, 3, 2, pcs_3_3},
  {7, spr_3_4, 6, 3, 2, pcs_3_4},
  {7, spr_3_5, 8, 4, 2, pcs_3_5},
  {7, spr_3_6, 10, 5, 3, pcs_3_6},
  {7, spr_3_7, 16, 6, 3, pcs_3_7},
  {7, spr_4_0, 1, 1, 1, pcs_4_0},
  {7, spr_4_1, 2, 2, 1, pcs_4_1},
  {7, spr_4_2, 2, 2, 1, pcs_4_2},
  {7, spr_4_3, 3, 3, 2, pcs_4_3},
  {7, spr_4_4, 3, 3, 2, pcs_4_4},
  {7, spr_4_5, 4, 4, 2, pcs_4_5},
  {7, spr_4_6, 5, 5, 3, pcs_4_6},
  {7, spr_4_7, 6, 6, 3, pcs_4_7},
  {7, spr_5_0, 1, 1, 1, pcs_5_0},
  {7, spr_5_1, 2, 2, 1, pcs_5_1},
  {7, spr_5_2, 2, 2, 1, pcs_5_2},
  {7, spr_5_3, 6, 3, 2, pcs_5_3},
  {7, spr_5_4, 6, 3, 2, pcs_5_4},
  {7, spr_5_5, 8, 4, 2, pcs_5_5},
  {8, spr_5_6, 13, 5, 3, pcs_5_6},
  {8, spr_5_7, 16, 6, 3, pcs_5_7},
  {8, spr_6_0, 1, 1, 1, pcs_6_0},
  {8, spr_6_1, 2, 2, 1, pcs_6_1},
  {8, spr_6_2, 2, 2, 1, pcs_6_2},
  {8, spr_6_3, 6, 3, 2, pcs_6_3},
  {8, spr_6_4, 6, 3, 2, pcs_6_4},
  {8, spr_6_5, 8, 4, 2, pcs_6_5},
  {8, spr_6_6, 13, 5, 3, pcs_6_6},
  {8, spr_6_7, 15, 6, 3, pcs_6_7},
  {8, spr_7_0, 1, 1, 1, pcs_7_0},
  {8, spr_7_1, 2, 2, 1, pcs_7_1},
  {8, spr_7_2, 2, 2, 1, pcs_7_2},
  {8, spr_7_3, 6, 3, 2, pcs_7_3},
  {8, spr_7_4, 6, 3, 2, pcs_7_4},
  {8, spr_7_5, 8, 4, 2, pcs_7_5},
  {8, spr_7_6, 13, 5, 3, pcs_7_6},
  {8, spr_7_7, 16, 6, 3, pcs_7_7},
  {8, spr_8_0, 1, 1, 1, pcs_8_0},
  {8, spr_8_1, 2, 2, 1, pcs_8_1},
  {8, spr_8_2, 2, 2, 1, pcs_8_2},
  {8, spr_8_3, 6, 3, 2, pcs_8_3},
  {8, spr_8_4, 6, 3, 2, pcs_8_4},
  {8, spr_8_5, 8, 4, 2, pcs_8_5},
  {8, spr_8_6, 13, 5, 3, pcs_8_6},
  {8, spr_8_7, 16, 6, 3, pcs_8_7},
  {8, spr_9_0, 1, 1, 1, pcs_9_0},
  {8, spr_9_1, 2, 2, 1, pcs_9_1},
  {8, spr_9_2, 2, 2, 1, pcs_9_2},
  {8, spr_9_3, 3, 3, 2, pcs_9_3},
  {8, spr_9_4, 3, 3, 2, pcs_9_4},
  {8, spr_9_5, 4, 4, 2, pcs_9_5},
  {8, spr_9_6, 8, 5, 3, pcs_9_6},
  {8, spr_9_7, 8, 6, 3, pcs_9_7},
  {8, spr_10_0, 1, 1, 1, pcs_10_0},
  {8, spr_10_1, 2, 2, 1, pcs_10_1},
  {8, spr_10_2, 2, 2, 1, pcs_10_2},
  {8, spr_10_3, 4, 3, 2, pcs_10_3},
  {8, spr_10_4, 6, 3, 2, pcs_10_4},
  {8, spr_10_5, 8, 4, 2, pcs_10_5},
  {9, spr_10_6, 13, 5, 3, pcs_10_6},
  {9, spr_10_7, 15, 6, 3, pcs_10_7},
  {9, spr_11_0, 1, 1, 1, pcs_11_0},
  {9, spr_11_1, 2, 2, 1, pcs_11_1},
  {9, spr_11_2, 2, 2, 1, pcs_11_2},
  {9, spr_11_3, 4, 3, 2, pcs_11_3},
  {9, spr_11_4, 6, 3, 2, pcs_11_4},
  {9, spr_11_5, 8, 4, 2, pcs_11_5},
  {9, spr_11_6, 13, 5, 3, pcs_11_6},
  {9, spr_11_7, 15, 6, 3, pcs_11_7},
  {9, spr_12_0, 1, 1, 1, pcs_12_0},
  {9, spr_12_1, 2, 2, 1, pcs_12_1},
  {9, spr_12_2, 2, 2, 1, pcs_12_2},
  {9, spr_12_3, 4, 3, 2, pcs_12_3},
  {9, spr_12_4, 6, 3, 2, pcs_12_4},
  {9, spr_12_5, 8, 4, 2, pcs_12_5},
  {9, spr_12_6, 13, 5, 3, pcs_12_6},
  {9, spr_12_7, 14, 6, 3, pcs_12_7},
  {9, spr_13_0, 1, 1, 1, pcs_13_0},
  {9, spr_13_1, 2, 2, 1, pcs_13_1},
  {9, spr_13_2, 2, 2, 1, pcs_13_2},
  {9, spr_13_3, 5, 3, 2, pcs_13_3},
  {9, spr_13_4, 5, 3, 2, pcs_13_4},
  {9, spr_13_5, 8, 4, 2, pcs_13_5},
  {9, spr_13_6, 13, 5, 3, pcs_13_6},
  {9, spr_13_7, 14, 6, 3, pcs_13_7},
  {9, spr_14_0, 1, 1, 1, pcs_14_0},
  {9, spr_14_1, 2, 2, 1, pcs_14_1},
  {9, spr_14_2, 2, 2, 1, pcs_14_2},
  {9, spr_14_3, 3, 3, 2, pcs_14_3},
  {9, spr_14_4, 3, 3, 2, pcs_14_4},
  {9, spr_14_5, 5, 4, 2, pcs_14_5},
  {9, spr_14_6, 7, 5, 3, pcs_14_6},
  {9, spr_14_7, 8, 6, 3, pcs_14_7},
  {9, spr_15_0, 1, 1, 1, pcs_15_0},
  {9, spr_15_1, 2, 2, 1, pcs_15_1},
  {9, spr_15_2, 2, 2, 1, pcs_15_2},
  {9, spr_15_3, 3, 3, 2, pcs_15_3},
  {9, spr_15_4, 6, 3, 2, pcs_15_4},
  {9, spr_15_5, 8, 4, 2, pcs_15_5},
  {9, spr_15_6, 10, 5, 3, pcs_15_6},
  {10, spr_15_7, 14, 6, 3, pcs_15_7},
  {10, spr_16_0, 1, 1, 1, pcs_16_0},
  {10, spr_16_1, 2, 2, 1, pcs_16_1},
  {10, spr_16_2, 2, 2, 1, pcs_16_2},
  {10, spr_16_3, 3, 3, 2, pcs_16_3},
  {10, spr_16_4, 5, 3, 2, pcs_16_4},
  {10, spr_16_5, 6, 4, 2, pcs_16_5},
  {10, spr_16_6, 6, 5, 3, pcs_16_6},
  {10, spr_16_7, 10, 6, 3, pcs_16_7},
  {10, spr_17_0, 1, 1, 1, pcs_17_0},
  {10, spr_17_1, 2, 2, 1, pcs_17_1},
  {10, spr_17_2, 2, 2, 1, pcs_17_2},
  {10, spr_17_3, 3, 3, 2, pcs_17_3},
  {10, spr_17_4, 6, 3, 2, pcs_17_4},
  {10, spr_17_5, 8, 4, 2, pcs_17_5},
  {10, spr_17_6, 8, 5, 3, pcs_17_6},
  {10, spr_17_7, 15, 6, 3, pcs_17_7},
  {10, spr_18_0, 1, 1, 1, pcs_18_0},
  {10, spr_18_1, 2, 2, 1, pcs_18_1},
  {10, spr_18_2, 2, 2, 1, pcs_18_2},
  {10, spr_18_3, 6, 3, 2, pcs_18_3},
  {10, spr_18_4, 6, 3, 2, pcs_18_4},
  {10, spr_18_5, 8, 4, 2, pcs_18_5},
  {10, spr_18_6, 13, 5, 3, pcs_18_6},
  {10, spr_18_7, 16, 6, 3, pcs_18_7},
  {10, spr_19_0, 1, 1, 1, pcs_19_0},
  {10, spr_19_1, 2, 2, 1, pcs_19_1},
  {10, spr_19_2, 2, 2, 1, pcs_19_2},
  {10, spr_19_3, 6, 3, 2, pcs_19_3},
  {10, spr_19_4, 6, 3, 2, pcs_19_4},
  {10, spr_19_5, 8, 4, 2, pcs_19_5},
  {10, spr_19_6, 13, 5, 3, pcs_19_6},
  {10, spr_19_7, 14, 6, 3, pcs_19_7},
  {10, spr_20_0, 1, 1, 1, pcs_20_0},
  {10, spr_20_1, 2, 2, 1, pcs_20_1},
  {10, spr_20_2, 2, 2, 1, pcs_20_2},
  {10, spr_20_3, 6, 3, 2, pcs_20_3},
  {10, spr_20_4, 6, 3, 2, pcs_20_4},
  {10, spr_20_5, 8, 4, 2, pcs_20_5},
  {10, spr_20_6, 14, 5, 3, pcs_20_6},
  {11, spr_20_7, 18, 6, 3, pcs_20_7},
};
