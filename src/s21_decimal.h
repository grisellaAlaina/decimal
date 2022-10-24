#ifndef SRC_S21_DECIMAL_H_
#define SRC_S21_DECIMAL_H_

/// ???? ///
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// ULONG_MAX
// LONG_MAX
// LONG_MIN
/// ???? ///

#define TRUE 1
#define FALSE 0
#define SUCCESS 0
#define CONVERTING_ERROR 1
#define MAX_BIT 2147483648u

typedef enum {
  OK = 0,
  TOO_MUCH_OR_INF = 1,
  TOO_FEW_OR_NEG_INF = 2,
  s21_NAN = 3,
} error_message;

typedef struct {
  int bits[4];
  // error_message error_num;
} s21_decimal;

typedef union {
  int ui;
  float fl;
} floatbits;

enum { LOW, MID, HIGH, SCALE };

int s21_add(s21_decimal value_1, s21_decimal value_2,
            s21_decimal *result);  // +

int s21_sub(s21_decimal value_1, s21_decimal value_2,
            s21_decimal *result);  // -

int s21_mul(s21_decimal value_1, s21_decimal value_2,
            s21_decimal *result);  // *

int s21_div(s21_decimal value_1, s21_decimal value_2,
            s21_decimal *result);  // /

int s21_mod(s21_decimal value_1, s21_decimal value_2,
            s21_decimal *result);  // Mod остаток от деления

int s21_is_less(s21_decimal, s21_decimal);  // <

int s21_is_less_or_equal(s21_decimal, s21_decimal);  // <=

int s21_is_greater(s21_decimal, s21_decimal);  // >

int s21_is_greater_or_equal(s21_decimal, s21_decimal);  // >=

int s21_is_equal(s21_decimal, s21_decimal);  // ==

int s21_is_not_equal(s21_decimal, s21_decimal);  // !=

int s21_from_int_to_decimal(int src, s21_decimal *dst);

int s21_from_float_to_decimal(float src, s21_decimal *dst);

int s21_from_decimal_to_int(s21_decimal src, int *dst);

int s21_from_decimal_to_float(s21_decimal src, float *dst);

int s21_floor(s21_decimal value, s21_decimal *result);

int s21_round(s21_decimal value, s21_decimal *result);

int s21_truncate(s21_decimal value, s21_decimal *result);

int s21_negate(s21_decimal value, s21_decimal *result);

void s21_init_decimal(s21_decimal *dec);
s21_decimal adding(s21_decimal *value_1, s21_decimal *value_2);
void set_bit(s21_decimal *res, int i, int value);
int get_bit(s21_decimal value, int i);
void s21_find_bigger(s21_decimal value_1, s21_decimal value_2,
                     int *who_greater);
int s21_check_sign(s21_decimal *value);
int s21_scale_equalization(s21_decimal *value_1, s21_decimal *value_2,
                           int error_scale);
void s21_set_bit(s21_decimal *res, int i, int value);
void s21_set_scale(s21_decimal *value, int scale);
int s21_last_bit(s21_decimal value);
int s21_shift_left(s21_decimal *value, int num);
int s21_zero_check(s21_decimal value1, s21_decimal value2);
void s21_convert_to_addcode(s21_decimal *value);
int s21_bit_addition(s21_decimal *value1, s21_decimal *value2,
                     s21_decimal *result);
s21_decimal s21_check_for_add(s21_decimal value1, s21_decimal value2);
s21_decimal s21_div_only_bits(s21_decimal value1, s21_decimal value2,
                              s21_decimal *buf);
int s21_get_bit(s21_decimal value, int bit);
int s21_get_scale(s21_decimal *value);
void s21_set_bit(s21_decimal *value, int bit, int num);
void s21_set_sign(s21_decimal *value, int sign);
void s21_check_scale(s21_decimal *value1, s21_decimal *value2);
int s21_get_sign(s21_decimal *value);
int s21_check_for_mul(s21_decimal value1, s21_decimal value2);
void s21_bits_copy(s21_decimal value, s21_decimal *dest);
int s21_shift_right(s21_decimal *first, int shift);

int s21_integer_division(s21_decimal value_1, s21_decimal value_2,
                         s21_decimal *result, s21_decimal *remainder,
                         int error_code);
void s21_first_prepare(s21_decimal tmp_div, s21_decimal *tmp_mod,
                       s21_decimal *tmp_del, s21_decimal value_2,
                       int *discharge);
void s21_first_step(s21_decimal *tmp_div, s21_decimal value_2, int *scale_c,
                    s21_decimal *tmp_res, int *index_res);
void s21_blue_electrical_tape(s21_decimal tmp_buf, s21_decimal *tmp_div,
                              s21_decimal *tmp_mod, int *discharge);
void s21_setting(s21_decimal tmp_buf, s21_decimal *tmp_del,
                 s21_decimal *tmp_mod, int *discharge);

#endif  // SRC_S21_DECIMAL_H_
