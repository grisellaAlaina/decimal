#include "s21_decimal.h"

/// |----------------------------------------------------|
/// | ~ ~ ~ ~ ~ ~ ~ ~ functions helpers ~ ~ ~ ~ ~ ~ ~ ~ ~|
/// |----------------------------------------------------|

int s21_get_bit(s21_decimal value, int bit) {
  int res = 0;
  if (bit / 32 < 4) {
    unsigned int mask = 1u << (bit % 32);
    res = value.bits[bit / 32] & mask;
  }
  return !!res;
}

void s21_set_bit(s21_decimal *value, int bit, int num) {
  unsigned int mask = 1u << (bit % 32);
  if (num) {
    value->bits[bit / 32] |= mask;
  } else if (!num) {
    value->bits[bit / 32] &= ~mask;
  }
}

void s21_set_sign(s21_decimal *value, int sign) {
  unsigned int mask = 1u << 31;
  if (sign != 0) {
    value->bits[3] |= mask;
  } else {
    value->bits[3] &= ~mask;
  }
}

int s21_get_sign(s21_decimal *value) {
  unsigned int mask = 1u << 31;
  return !!(value->bits[3] & mask);
}

int s21_get_scale(s21_decimal *value) { return (char)(value->bits[3] >> 16); }

int s21_get_float_exp(float *src) {
  return ((*(int *)src & ~MAX_BIT) >> 23) - 127;
}

int s21_get_float_sign(float *src) {
  unsigned int mask = 1u << 31;
  return !!!!((*(int *)src) & mask);
}

void s21_set_scale(s21_decimal *value, int scale) {
  int clearMask = ~(0xFF << 16);
  value->bits[3] &= clearMask;
  int mask = scale << 16;
  value->bits[3] |= mask;
}

int s21_last_bit(s21_decimal value) {
  int last_bit = 95;
  for (; last_bit >= 0 && s21_get_bit(value, last_bit) == 0; last_bit--) {
  }
  return last_bit;
}

void s21_init_decimal(s21_decimal *dec) {
  dec->bits[0] = dec->bits[1] = dec->bits[2] = dec->bits[3] = 0;
}

int s21_shift_left(s21_decimal *value, int num) {
  int res = OK;
  int lastbit = s21_last_bit(*value);
  if (lastbit + num > 95) {
    res = TOO_MUCH_OR_INF;
  }
  if (res == OK) {
    for (int i = 0; i < num; i++) {
      int value_31bit = s21_get_bit(*value, 31);
      int value_63bit = s21_get_bit(*value, 63);
      value->bits[0] <<= 1;
      value->bits[1] <<= 1;
      value->bits[2] <<= 1;
      if (value_31bit) s21_set_bit(value, 32, 1);
      if (value_63bit) s21_set_bit(value, 64, 1);
    }
  }
  return res;
}

void s21_bits_copy(s21_decimal value, s21_decimal *dest) {
  dest->bits[0] = value.bits[0];
  dest->bits[1] = value.bits[1];
  dest->bits[2] = value.bits[2];
}

int s21_zero_check(s21_decimal value1, s21_decimal value2) {
  int zero = 1;
  s21_decimal *tmp1 = &value1;
  s21_decimal *tmp2 = &value2;
  if (tmp1 && tmp2) {
    if (!value1.bits[0] && !value2.bits[0] && !value1.bits[1] &&
        !value2.bits[1] && !value1.bits[2] && !value2.bits[2])
      zero = 0;
  }
  return zero;
}

void s21_check_scale(s21_decimal *value1, s21_decimal *value2) {
  int scale_value1 = s21_get_scale(value1);
  int scale_value2 = s21_get_scale(value2);
  if (scale_value1 != scale_value2) {
    s21_scale_equalization(value1, value2, 0);
  }
}
/// |----------------------------------------------------|
/// |\ \ \ \ \ \ \ less \\ greater // equal / / / / / / /|
/// |----------------------------------------------------|

int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
  int res = 0;
  s21_check_scale(&value_1, &value_2);
  int num1 = 0, num2 = 0;
  int sign1 = s21_get_sign(&value_1), sign2 = s21_get_sign(&value_2);
  for (int i = 95; i >= 0; i--) {
    num1 = s21_get_bit(value_1, i);
    num2 = s21_get_bit(value_2, i);
    if (num1 > num2) {
      res = 1;
      break;
    }
    if (num1 < num2) {
      res = 0;
      break;
    }
  }
  if (sign1 && !sign2)
    res = 0;
  else if (!sign1 && sign2)
    res = 1;
  else if (sign1 && sign2)
    res = !res;
  return s21_is_equal(value_1, value_2) ? 0 : res;
}

int s21_is_less(s21_decimal value1, s21_decimal value2) {
  return s21_is_greater(value2, value1);
}

int s21_is_equal(s21_decimal value1, s21_decimal value2) {
  s21_check_scale(&value1, &value2);
  int res = 1;
  int sign1 = s21_get_sign(&value1), sign2 = s21_get_sign(&value2);
  for (int i = 95; i >= 0; i--) {
    if (s21_get_bit(value1, i) != s21_get_bit(value2, i)) {
      res = 0;
      break;
    }
  }
  if (sign1 != sign2) res = 0;
  return res;
}

int s21_is_greater_or_equal(s21_decimal value1, s21_decimal value2) {
  return (s21_is_greater(value1, value2) || s21_is_equal(value1, value2));
}

int s21_is_less_or_equal(s21_decimal value1, s21_decimal value2) {
  return (s21_is_less(value1, value2) || s21_is_equal(value1, value2));
}

int s21_is_not_equal(s21_decimal value1, s21_decimal value2) {
  return (!s21_is_equal(value1, value2));
}

/// |----------------------------------------------------|
/// |- - - - - - - - - from ... to ... - - - - - - - - - |
/// |----------------------------------------------------|

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  char result = OK;
  if (dst) {
    s21_init_decimal(dst);
    if (src < 0) {
      s21_set_sign(dst, 1);
      src = abs(src);
    }
    dst->bits[0] = src;
  } else {
    result = CONVERTING_ERROR;
  }
  return result;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  int res = OK;
  s21_truncate(src, &src);
  if (src.bits[1] != 0 || src.bits[2] != 0) {
    res = CONVERTING_ERROR;
    *dst = 0;
  }
  if (src.bits[0] != abs(src.bits[0])) {
    res = CONVERTING_ERROR;
    *dst = 0;
  } else {
    *dst = src.bits[0];
    *dst *= s21_get_sign(&src) ? -1 : 1;
  }
  return res;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  s21_init_decimal(dst);
  int result = CONVERTING_ERROR, sign = s21_get_float_sign(&src),
      exp = s21_get_float_exp(&src);
  if (dst && src != 0) {
    double temp = (double)fabs(src);
    int off = 0;
    for (; off < 28 && (int)temp / (int)pow(2, 21) == 0; temp *= 10, off++) {
    }
    temp = round(temp);
    if (off <= 28 && (exp > -94 && exp < 96)) {
      floatbits mant;
      temp = (float)temp;
      for (; fmod(temp, 10) == 0 && off > 0; off--, temp /= 10) {
      }
      mant.fl = temp;
      exp = s21_get_float_exp(&mant.fl);
      dst->bits[exp / 32] |= 1 << exp % 32;
      for (int i = exp - 1, j = 22; j >= 0; i--, j--) {
        unsigned long int bit = 1 << (i % 32);
        if ((mant.ui & (1 << j)) != 0) dst->bits[i / 32] |= bit;
        dst->bits[3] = (sign << 31) | (off << 16);
        result = OK;
      }
    }
  }
  return result;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  int res = 1;
  if (dst != NULL) {
    double temp = 0;
    for (int i = 0; i < 96; i++)
      if (s21_get_bit(src, i)) temp += pow(2, i);
    int scale = s21_get_scale(&src);
    for (int i = scale; i > 0; i--) temp /= 10.0;
    if (s21_get_sign(&src)) temp *= -1;
    *dst = (float)temp;
    res = 0;
  }
  return res;
}

/// |----------------------------------------------------|
/// | - - - - - - - - - other functions - - - - - - - - -|
/// |----------------------------------------------------|

int s21_floor(s21_decimal value, s21_decimal *result) {
  int res = 1;
  if (result != NULL) {
    s21_init_decimal(result);
    s21_decimal one = {{1, 0, 0, 0}};
    s21_truncate(value, &value);
    if (s21_get_sign(&value)) s21_sub(value, one, &value);
    *result = value;
    res = 0;
  }
  return res;
}

int s21_round(s21_decimal value, s21_decimal *result) {
  int res = OK;
  if (result != NULL) {
    s21_decimal one = {{1, 0, 0, 0}};
    s21_decimal five = {{5, 0, 0, 0}};
    int sign = s21_get_sign(&value);
    s21_set_sign(&value, 0);
    s21_decimal trunc = {{0, 0, 0, 0}};
    s21_truncate(value, &trunc);
    s21_decimal buf = {{0, 0, 0, 0}};
    s21_sub(value, trunc, &buf);
    s21_set_scale(&five, 1);
    *result = trunc;
    if (s21_is_greater_or_equal(buf, five) == 1) {
      s21_add(*result, one, result);
    }
    s21_set_sign(result, sign);
  } else {
    res = CONVERTING_ERROR;
  }
  return res;
}

int s21_truncate(s21_decimal value, s21_decimal *result) {
  s21_decimal ten = {{10, 0, 0, 0}};
  s21_decimal tmp = {{0, 0, 0, 0}};
  int res = OK;
  if (result != NULL) {
    int sign = s21_get_sign(&value);
    int scale = s21_get_scale(&value);
    if (!scale) {
      *result = value;
    } else {
      for (int i = scale; i > 0; i--) {
        *result = s21_div_only_bits(value, ten, &tmp);
        s21_div(value, ten, &tmp);
        value = *result;
      }
    }
    if (sign) s21_set_sign(result, 1);
  } else {
    res = CONVERTING_ERROR;
  }
  return res;
}

/// |----------------------------------------------------|
/// | - - - - - - - - - new function - - - - - - - - - - |
/// |----------------------------------------------------|

int s21_shift_right(s21_decimal *first, int shift) {
  int res_val = 1;
  for (int i = 0; i < shift; i++) {
    int value_32bit = s21_get_bit(*first, 32);
    int value_64bit = s21_get_bit(*first, 64);
    first->bits[0] >>= 1;
    first->bits[1] >>= 1;
    first->bits[2] >>= 1;
    value_32bit ? s21_set_bit(first, 31, 1) : s21_set_bit(first, 31, 0);
    value_64bit ? s21_set_bit(first, 63, 1) : s21_set_bit(first, 63, 0);
    s21_set_bit(first, 95, 0);
    res_val = 0;
  }
  return res_val;
}

void first_prepare(s21_decimal tmp_div, s21_decimal *tmp_mod,
                   s21_decimal *tmp_del, s21_decimal value_2, int *discharge) {
  *discharge = 0;
  int shift = s21_last_bit(tmp_div) - s21_last_bit(value_2);
  int n = 0;
  while (1) {
    s21_bits_copy(tmp_div, tmp_del);
    s21_shift_right(tmp_del, shift - n);
    if (s21_is_greater_or_equal(*tmp_del, value_2) == 1) {
      break;
    } else {
      n++;
    }
  }
  s21_bits_copy(tmp_div, tmp_mod);
  *discharge = s21_last_bit(tmp_div) - s21_last_bit(*tmp_del);
  for (int i = 95; i > s21_last_bit(tmp_div) - s21_last_bit(*tmp_del) - 1; i--)
    s21_set_bit(tmp_mod, i, 0);
}

void blue_electrical_tape(s21_decimal tmp_buf, s21_decimal *tmp_div,
                          s21_decimal *tmp_mod, int *discharge) {
  s21_bits_copy(tmp_buf, tmp_div);
  s21_shift_left(tmp_div, 1);
  s21_get_bit(*tmp_mod, *discharge - 1) == 1 ? s21_set_bit(tmp_div, 0, 1)
                                             : s21_set_bit(tmp_div, 0, 0);
  s21_set_bit(tmp_mod, *discharge - 1, 0);
  (*discharge)--;
}

void s21_reset(s21_decimal *value) {
  value->bits[0] = value->bits[1] = value->bits[2] = value->bits[3] = 0;
}

void first_step(s21_decimal *tmp_div, s21_decimal value_2, int *scale_c,
                s21_decimal *tmp_res, int *index_res) {
  s21_decimal ten = {{10, 0, 0, 0}};
  int x = s21_is_greater_or_equal(*tmp_div, value_2);
  while (x != 1) {
    s21_set_bit(tmp_res, *index_res, 0);
    (*index_res)--;
    s21_mul(*tmp_div, ten, tmp_div);
    (*scale_c)++;
    x = s21_is_greater_or_equal(*tmp_div, value_2);
  }
}

int s21_negate(s21_decimal value, s21_decimal *result) {
  int res = OK;
  if (result != NULL) {
    int sign = s21_get_sign(&value);
    if (sign) s21_set_sign(&value, 0);
    if (!sign) s21_set_sign(&value, 1);
    *result = value;
  } else {
    res = CONVERTING_ERROR;
  }
  return res;
}

/// |----------------------------------------------------|
/// | - - - - - - - - - ariphmetic - - - - - - - - - - - |
/// |----------------------------------------------------|

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int res = 0;
  int sign_result = 0;
  s21_init_decimal(result);
  if (s21_get_sign(&value_1) != s21_get_sign(&value_2)) {
    sign_result = 1;
  } else {
    sign_result = 0;
  }
  int last_bit_1 = s21_last_bit(value_1);
  s21_decimal tmp_res = {{0, 0, 0, 0}};
  for (int i = 0; i <= last_bit_1; i++) {
    s21_init_decimal(&tmp_res);
    int value_bit_1 = s21_get_bit(value_1, i);
    if (value_bit_1) {
      tmp_res = value_2;
      if ((95 - last_bit_1 - 1) == i && sign_result == 0) {
        res = TOO_MUCH_OR_INF;
        break;
      } else if ((95 - last_bit_1 - 1) == i && sign_result == 1) {
        res = TOO_FEW_OR_NEG_INF;
        break;
      }
      s21_shift_left(&tmp_res, i);
      res = s21_bit_addition(result, &tmp_res, result);
    }
  }
  while (res != OK &&
         (s21_get_scale(&value_1) > 0 || s21_get_scale(&value_2) > 0)) {
    s21_decimal *chosen_num = NULL, *other_num = NULL;
    if (s21_last_bit(value_1) > s21_last_bit(value_2) &&
        s21_get_scale(&value_1) > 0) {
      chosen_num = &value_1;
      other_num = &value_2;
    } else if (s21_last_bit(value_2) > s21_last_bit(value_1) &&
               s21_get_scale(&value_2) > 0) {
      chosen_num = &value_2;
      other_num = &value_1;
    } else {
      break;
    }
    int chosen_num_scale = s21_get_scale(chosen_num);
    s21_decimal ten = {{10, 0, 0, 0}};
    s21_decimal remainder = {{0, 0, 0, 0}};
    s21_decimal tmp_div = s21_div_only_bits(*chosen_num, ten, &remainder);
    s21_decimal zero = {{0, 0, 0, 0}};
    if (s21_zero_check(tmp_div, zero) == 1) {
      s21_bits_copy(tmp_div, chosen_num);
    } else {
      s21_bits_copy(remainder, chosen_num);
    }
    s21_set_scale(chosen_num, --chosen_num_scale);
    res = s21_mul(*chosen_num, *other_num, result);
  }
  int scale = s21_get_scale(&value_1) + s21_get_scale(&value_2);
  s21_set_scale(result, scale);
  s21_set_sign(result, sign_result);
  if ((res != OK) || scale > 28) {
    s21_init_decimal(result);
  }
  return res;
}

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int res = 0, flag = 0;
  s21_init_decimal(result);
  s21_scale_equalization(&value_1, &value_2, 0);
  int sign1 = s21_get_sign(&value_1);
  int sign2 = s21_get_sign(&value_2);
  if (sign1 != sign2) {
    if (sign1 == 1) flag = 1;
    if (sign2 == 1) flag = 2;
  }
  if (flag == 0) {
    int buf = 0;
    for (int i = 0; i < 96; i++) {
      int tmp1 = s21_get_bit(value_1, i);
      int tmp2 = s21_get_bit(value_2, i);
      int tmp_res = tmp1 + tmp2 + buf;
      if (tmp_res == 0) {
        s21_set_bit(result, i, 0);
        buf = 0;
      } else if (tmp_res == 1) {
        s21_set_bit(result, i, 1);
        buf = 0;
      } else if (tmp_res == 2) {
        if (i == 95) {
          res = 1;
          break;
        }
        s21_set_bit(result, i, 0);
        buf = 1;
      } else if (tmp_res == 3) {
        if (i == 95) {
          res = 1;
          break;
        }
        buf = 1;
        s21_set_bit(result, i, 1);
      }
    }
    s21_set_scale(result, s21_get_scale(&value_1));
    if (sign1 == 1) {
      s21_set_sign(result, 1);
    }
  } else if (flag == 1) {
    s21_set_sign(&value_1, 0);
    res = s21_sub(value_2, value_1, result);
  } else if (flag == 2) {
    s21_set_sign(&value_2, 0);
    res = s21_sub(value_1, value_2, result);
  }
  if (s21_get_scale(result) > 28 ||
      (result->bits[0] == 0 && result->bits[1] == 0 && result->bits[2] == 0))
    res = 2;
  if (res != 0) s21_init_decimal(result);
  return res;
}

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  int res = 0, flag = 0;
  s21_init_decimal(result);
  s21_scale_equalization(&value_1, &value_2, 0);
  int sign1 = s21_get_bit(value_1, 127);
  int sign2 = s21_get_sign(&value_2);
  if (sign1 != sign2) {
    if (sign1 == 0) flag = 1;
    if (sign2 == 0) flag = 2;
  }
  if (flag == 0) {
    if (sign1 == 0) {
      if (s21_is_less(value_1, value_2)) {
        s21_set_sign(result, 1);
        s21_decimal hold = value_1;
        value_1 = value_2;
        value_2 = hold;
      }
      for (int i = 0; i < 96; i++) {
        int tmp1 = s21_get_bit(value_1, i);
        int tmp2 = s21_get_bit(value_2, i);
        int tmp_res = tmp1 - tmp2;
        if (tmp_res == 0) {
          s21_set_bit(result, i, 0);
          continue;
        } else if (tmp_res == 1) {
          s21_set_bit(result, i, 1);
        } else if (tmp_res == -1) {
          int n = i + 1;
          while (s21_get_bit(value_1, n) != 1) {
            s21_set_bit(&value_1, n, 1);
            n++;
          }
          s21_set_bit(&value_1, n, 0);
          s21_set_bit(result, i, 1);
        }
      }
      s21_set_scale(result, s21_get_scale(&value_1));
    } else if (sign1 == 1) {
      s21_set_bit(&value_1, 127, 0);
      s21_set_bit(&value_2, 127, 0);
      res = s21_sub(value_2, value_1, result);
    }
  } else if (flag == 1) {
    s21_set_bit(&value_2, 127, 0);
    res = s21_add(value_1, value_2, result);
  } else if (flag == 2) {
    s21_set_bit(&value_2, 127, 1);
    res = s21_add(value_1, value_2, result);
  }
  if (s21_get_scale(result) > 28 ||
      (s21_get_scale(result) == 28 && result->bits[0] == 0 &&
       result->bits[1] == 0 && result->bits[2] == 0))
    res = 2;
  if (res != 0) s21_init_decimal(result);
  return res;
}

// int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
//   s21_decimal remainder = {{1, 0, 0, 0}};
//   s21_decimal tmp_res = {{0, 0, 0, 0}};
//   s21_decimal zero = {{0, 0, 0, 0}};
//   int res = 0, count = 0;
//   int scale1 = s21_get_scale(&value_1), scale2 = s21_get_scale(&value_2),
//       scale = 0;
//   int sign1 = s21_get_sign(&value_1), sign2 = s21_get_sign(&value_2);
//   s21_set_sign(&value_1, 0);
//   s21_set_sign(&value_2, 0);
//   (scale1 >= scale2) ? scale = (scale1 - scale2)
//                      : s21_scale_equalization(&value_1, &value_2, 0);
//   if (s21_is_equal(value_2, zero)) res = s21_NAN;
//   if (s21_is_greater(value_2, value_1) && scale == 0) {
//     s21_decimal ten = {{10, 0, 0, 0}};
//     while (s21_is_greater(value_2, value_1)) {
//       s21_mul(value_1, ten, &value_1);
//       scale++;
//       if (scale == 28) break;
//     }
//   }
//   s21_set_scale(&value_1, scale);
//   while (count < 10) {
//     *result = tmp_res;
//     res = s21_integer_division(value_1, value_2, &tmp_res, &remainder, 0);
//     if (s21_get_scale(&tmp_res) == 0 && s21_last_bit(tmp_res) > 93) {
//       res = TOO_MUCH_OR_INF;
//       break;
//     }
//     s21_add(tmp_res, *result, &tmp_res);
//     if (remainder.bits[0] == 0 && remainder.bits[1] == 0 &&
//         remainder.bits[2] == 0)
//       break;
//     value_1 = remainder;
//     s21_set_scale(&value_1, scale + count);
//     count++;
//   }
//   *result = tmp_res;
//   (sign1 != sign2) ? s21_set_sign(result, 1) : NULL;
//   if (res != 0) s21_init_decimal(result);
//   return res;
// }

int s21_integer_division(s21_decimal value_1, s21_decimal value_2,
                         s21_decimal *result, s21_decimal *remainder,
                         int error_code) {
  s21_decimal tmp_div = {{0, 0, 0, 0}};
  s21_decimal tmp_del = {{0, 0, 0, 0}};
  s21_decimal tmp_mod = {{0, 0, 0, 0}};
  s21_decimal tmp_res = {{0, 0, 0, 0}};
  s21_decimal tmp_buf = {{0, 0, 0, 0}};
  int scale_value1 = s21_get_scale(&value_1);
  int index_res = 95, res = 0, discharge = 1;
  int sign1 = s21_get_sign(&value_1), sign2 = s21_get_sign(&value_2);
  int scale1 = s21_get_scale(&value_1), scale2 = s21_get_scale(&value_2);
  s21_bits_copy(value_1, &tmp_div);
  s21_set_scale(&tmp_div, 0);
  s21_set_scale(&value_2, 0);
  s21_set_sign(&tmp_div, 0);
  s21_set_sign(&value_2, 0);
  if (value_2.bits[0] == 0 && value_2.bits[1] == 0 && value_2.bits[2] == 0) {
    res = s21_NAN;
  } else if ((value_1.bits[0] == 0 && value_1.bits[1] == 0 &&
              value_1.bits[2] == 0) ||
             scale1 > 28 || scale2 > 28) {
    res = TOO_FEW_OR_NEG_INF;
  } else {
    if (s21_is_greater_or_equal(tmp_div, value_2) && error_code) {
      s21_first_prepare(tmp_div, &tmp_mod, &tmp_del, value_2, &discharge);
    } else if (!error_code) {
      s21_first_step(&tmp_div, value_2, &scale_value1, &tmp_res, &index_res);
      s21_first_prepare(tmp_div, &tmp_mod, &tmp_del, value_2, &discharge);
    }
    while (discharge != -1 && index_res >= 0 && scale_value1 <= 28) {
      if (s21_is_greater_or_equal(tmp_del, value_2)) {
        res = s21_sub(tmp_del, value_2, &tmp_buf);
        s21_set_bit(&tmp_res, index_res, 1);
        index_res--;
        s21_setting(tmp_buf, &tmp_del, &tmp_mod, &discharge);
      } else {
        s21_setting(tmp_del, &tmp_del, &tmp_mod, &discharge);
        s21_set_bit(&tmp_res, index_res, 0);
        index_res--;
      }
    }
    s21_init_decimal(result);
    for (int i = 95; i > index_res; i--) {
      s21_get_bit(tmp_res, i) == 1 ? s21_set_bit(result, i - index_res - 1, 1)
                                   : s21_set_bit(result, i - index_res - 1, 0);
    }
    (sign1 != sign2) ? s21_set_sign(result, 1) : NULL;
    s21_set_scale(result, scale_value1);
    s21_shift_right(&tmp_del, 1);
    *remainder = tmp_del;
  }
  if (res != 0) s21_init_decimal(result);
  return res;
}

void s21_setting(s21_decimal tmp_buf, s21_decimal *tmp_del,
                 s21_decimal *tmp_mod, int *discharge) {
  s21_bits_copy(tmp_buf, tmp_del);
  s21_shift_left(tmp_del, 1);
  s21_get_bit(*tmp_mod, *discharge - 1) == 1 ? s21_set_bit(tmp_del, 0, 1)
                                             : s21_set_bit(tmp_del, 0, 0);
  s21_set_bit(tmp_mod, *discharge - 1, 0);
  (*discharge)--;
}

void s21_first_step(s21_decimal *tmp_div, s21_decimal value_2,
                    int *scale_value1, s21_decimal *tmp_res, int *index_res) {
  s21_decimal ten = {{10, 0, 0, 0}};
  int x = s21_is_greater_or_equal(*tmp_div, value_2);
  while (x != 1) {
    s21_set_bit(tmp_res, *index_res, 0);
    (*index_res)--;
    s21_mul(*tmp_div, ten, tmp_div);
    (*scale_value1)++;
    x = s21_is_greater_or_equal(*tmp_div, value_2);
  }
}

void s21_first_prepare(s21_decimal tmp_div, s21_decimal *tmp_mod,
                       s21_decimal *tmp_del, s21_decimal value_2,
                       int *discharge) {
  *discharge = 0;
  int shift = 0;
  shift = s21_last_bit(tmp_div) - s21_last_bit(value_2);
  int n = 0;
  while (1) {
    s21_bits_copy(tmp_div, tmp_del);
    s21_shift_right(tmp_del, shift - n);
    if (s21_is_greater_or_equal(*tmp_del, value_2) == 1) {
      break;
    } else {
      n++;
    }
  }
  s21_bits_copy(tmp_div, tmp_mod);
  *discharge = s21_last_bit(tmp_div) - s21_last_bit(*tmp_del);
  for (int i = 95; i > *discharge - 1; i--) {
    s21_set_bit(tmp_mod, i, 0);
  }
}

int s21_div(s21_decimal value1, s21_decimal value2, s21_decimal *result) {
  int res = OK;
  s21_init_decimal(result);
  s21_decimal zero = {{0, 0, 0, 0}};
  s21_decimal min_zero = {{0, 0, 0, MAX_BIT}};
  if (s21_is_equal(value2, zero) || s21_is_equal(value2, min_zero))
    res = s21_NAN;
  if (res == s21_NAN) s21_init_decimal(result);
  if (res == OK) {
    int begin_scale = s21_get_scale(&value1) - s21_get_scale(&value2);
    int sign = s21_get_sign(&value1) - s21_get_sign(&value2);
    s21_decimal remainder = {{0, 0, 0, 0}};
    s21_decimal tmp = {{0, 0, 0, 0}};
    s21_set_scale(&value2, 0);
    s21_set_scale(&value1, 0);
    s21_set_sign(&value1, 0);
    s21_set_sign(&value2, 0);
    tmp = s21_div_only_bits(value1, value2, &remainder);
    s21_bits_copy(tmp, result);
    s21_decimal border_value = {{-1, -1, -1, 0}};
    s21_decimal ten = {{10, 0, 0, 0}};
    s21_set_scale(&border_value, 1);
    int inside_scale = 0;
    for (; inside_scale <= 27 && s21_is_equal(remainder, zero) == 0;) {
      if (s21_is_less(*result, border_value) == 0) {
        break;
      }
      res = s21_mul(remainder, ten, &remainder);
      tmp = s21_div_only_bits(remainder, value2, &remainder);
      if (res == OK) res = s21_mul(*result, ten, result);
      if (res == OK) res = s21_add(*result, tmp, result);
      inside_scale++;
    }
    s21_decimal garbage = {{0, 0, 0, 0}};
    int end_scale = begin_scale + inside_scale;
    for (; end_scale > 28;) {
      s21_div_only_bits(*result, ten, &garbage);
      end_scale--;
    }
    for (; end_scale < 0;) {
      if (res == OK) res = s21_mul(*result, ten, result);
      end_scale++;
    }
    s21_set_scale(result, end_scale);
    s21_set_sign(result, sign);
  }
  return res;
}

int s21_mod(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  s21_decimal tmp = {{0, 0, 0, 0}};
  int res = OK;
  res = s21_div(value_1, value_2, &tmp);
  if (res == OK) res = s21_truncate(tmp, &tmp);
  if (res == OK) res = s21_mul(tmp, value_2, &tmp);
  if (res == OK) res = s21_sub(value_1, tmp, result);
  return res;
}

/// |----------------------------------------------------|
/// | - - - - - - - additional functions - - - - - - - - |
/// |----------------------------------------------------|

void clear_bits(s21_decimal *varPtr) {
  memset(varPtr->bits, 0, sizeof(varPtr->bits));
}

s21_decimal s21_div_only_bits(s21_decimal value1, s21_decimal value2,
                              s21_decimal *buf) {
  s21_init_decimal(buf);
  s21_decimal result = {{0, 0, 0, 0}};
  for (int i = s21_last_bit(value1); i >= 0; i--) {
    if (s21_get_bit(value1, i)) s21_set_bit(buf, 0, 1);
    if (s21_is_greater_or_equal(*buf, value2) == 1) {
      s21_sub(*buf, value2, buf);
      if (i != 0) s21_shift_left(buf, 1);
      if (s21_get_bit(value1, i - 1)) s21_set_bit(buf, 0, 1);
      s21_shift_left(&result, 1);
      s21_set_bit(&result, 0, 1);
    } else {
      s21_shift_left(&result, 1);
      if (i != 0) s21_shift_left(buf, 1);
      if ((i - 1) >= 0 && s21_get_bit(value1, i - 1)) s21_set_bit(buf, 0, 1);
    }
  }
  return result;
}

int s21_scale_equalization(s21_decimal *value_1, s21_decimal *value_2,
                           int err_num) {
  int res = err_num;
  s21_decimal *bigger = NULL;
  s21_decimal *smaller = NULL;
  s21_decimal remainder = {{0, 0, 0, 0}};
  if (s21_get_scale(value_1) > s21_get_scale(value_2)) {
    bigger = value_1;
    smaller = value_2;
  } else {
    bigger = value_2;
    smaller = value_1;
  }
  if (s21_get_scale(value_1) != s21_get_scale(value_2)) {
    int sign1 = s21_get_sign(bigger), sign2 = s21_get_sign(smaller);
    s21_decimal ten = {{10, 0, 0, 0}};
    s21_set_sign(bigger, 0);
    s21_set_sign(smaller, 0);
    while (s21_get_scale(smaller) != s21_get_scale(bigger) &&
           s21_last_bit(*smaller) < 93 && s21_get_scale(smaller) <= 28) {
      int res = 0;
      int scale_small = s21_get_scale(smaller);
      s21_set_scale(smaller, 0);
      res = s21_mul(ten, *smaller, smaller);
      if (res != 0) break;
      s21_set_scale(smaller, scale_small + 1);
    }
    while (s21_get_scale(smaller) != s21_get_scale(bigger)) {
      int res = 0;
      int scale_big = s21_get_scale(bigger);
      if (s21_get_scale(bigger) - s21_get_scale(smaller) == 1) {
        if (bigger->bits[0] >= 5 && bigger->bits[0] < 10) {
          bigger->bits[0] = 1;
          s21_set_scale(bigger, scale_big - 1);
          break;
        }
      }
      res = s21_integer_division(*bigger, ten, bigger, &remainder, 1);
      if (res != 0) break;
      s21_set_scale(bigger, scale_big - 1);
    }
    s21_set_sign(bigger, sign1);
    s21_set_sign(smaller, sign2);
  }
  return res;
}

int s21_bit_addition(s21_decimal *value1, s21_decimal *value2,
                     s21_decimal *result) {
  int buf = 0, res = OK;
  for (int i = 0; i < 96; i++) {
    int bit1 = s21_get_bit(*value1, i);
    int bit2 = s21_get_bit(*value2, i);
    if (bit1 && bit2) {
      if (buf) {
        s21_set_bit(result, i, 1);
      } else {
        s21_set_bit(result, i, 0);
        buf = 1;
      }
    } else if (!bit1 && !bit2) {
      if (buf) {
        s21_set_bit(result, i, 1);
        buf = 0;
      } else {
        s21_set_bit(result, i, 0);
      }
    } else if (bit1 != bit2) {
      if (buf) {
        s21_set_bit(result, i, 0);
      } else {
        s21_set_bit(result, i, 1);
      }
    }
    if (i == 95 && buf == 1) res = TOO_MUCH_OR_INF;
  }
  return res;
}
