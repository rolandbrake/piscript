#include "pi_builtin.h"
#include "../pi_value.h"

BuiltinConst builtin_constants[] = {
    {"PI", {VAL_NUM, {.number = PI}}},
    {"E", {VAL_NUM, {.number = E}}},
    {"WIDTH", {VAL_NUM, {.number = SCREEN_WIDTH}}},
    {"HEIGHT", {VAL_NUM, {.number = SCREEN_HEIGHT}}},
    {"WAVE_SINE", {VAL_NUM, {.number = WAVE_SINE}}},
    {"WAVE_SQUARE", {VAL_NUM, {.number = WAVE_SQUARE}}},
    {"WAVE_TRIANGLE", {VAL_NUM, {.number = WAVE_TRIANGLE}}},
    {"WAVE_NOISE", {VAL_NUM, {.number = WAVE_NOISE}}},
};
int BUILTIN_CONST_COUNT = sizeof(builtin_constants) / sizeof(BuiltinConst);

BuiltinFunc builtin_functions[] = {
    // Math
    {"floor", pi_floor},
    {"ceil", pi_ceil},
    {"round", pi_round},
    {"seed", pi_seed},
    {"rand", pi_rand},
    {"rand_n", pi_rand_n},
    {"sqrt", pi_sqrt},
    {"sin", pi_sin},
    {"cos", pi_cos},
    {"tan", pi_tan},
    {"asin", pi_asin},
    {"acos", pi_acos},
    {"atan", pi_atan},
    {"deg", pi_deg},
    {"rad", pi_rad},
    {"sum", pi_sum},
    {"exp", pi_exp},
    {"log2", pi_log2},
    {"log10", pi_log10},
    {"logE", pi_logE},
    {"pow", pi_pow},
    {"abs", pi_abs},
    {"mean", pi_mean},
    {"avg", pi_avg},
    {"var", pi_var},
    {"dev", pi_dev},
    {"median", pi_median},
    {"mode", pi_mode},
    {"max", pi_max},
    {"min", pi_min},

    // Graphics
    {"pixel", pi_pixel},
    {"line", pi_line},
    {"draw", pi_draw},
    {"clear", pi_clear},
    {"circ", pi_circ},
    {"rect", pi_rect},
    {"poly", pi_poly},
    {"color", pi_color},
    {"sprite", pi_sprite},

    // Image
    {"image", pi_image},
    {"crop", pi_crop},
    {"resize", pi_resize},
    {"flip", pi_flip},
    {"rend2d", pi_rend2d},
    {"scale2d", pi_scale2d},
    {"tran2d", pi_tran2d},
    {"rot2d", pi_rotate2d},
    {"copy2d", pi_copy2d},

    // Time
    {"sleep", pi_sleep},
    {"time", _pi_time},

    // IO    
    {"println", pi_println},
    {"print", pi_print},
    {"printf", pi_printf},
    {"log", pi_log},
    {"key", pi_key},    
    {"input", pi_input},

    // File
    {"open", pi_open},
    {"read", pi_read},
    {"write", pi_write},
    {"seek", pi_seek},
    {"close", pi_close},

    // String
    {"char", pi_char},
    {"ord", pi_ord},
    {"trim", pi_trim},
    {"upper", pi_upper},
    {"lower", pi_lower},
    {"replace", pi_replace},
    {"is_upper", pi_isUpper},
    {"is_lower", pi_isLower},
    {"is_digit", pi_isDigit},
    {"is_numeric", pi_isNumeric},
    {"is_alpha", pi_isAlpha},
    {"is_alnum", pi_isAlnum},
    {"split", pi_split},

    // Audio
    {"sound", pi_sound},
    {"melody", pi_melody},
    {"tone", pi_tone},
    {"play", pi_play},
    {"stop", pi_stop},
    {"pause", pi_pause},
    {"resume", pi_resume},
    {"is_playing", pi_isPlaying},
    {"channel", pi_channel},
    {"set_loop", pi_setLoop},

    // System
    {"fps", pi_fps},
    {"error", pi_error},
    {"zen", pi_zen},
    {"cursor", pi_cursor},
    {"mouse", pi_mouse},

    // Type
    {"type", _pi_type},
    {"is_num", pi_isNum},
    {"is_str", pi_isStr},
    {"is_bool", pi_isBool},
    {"is_list", pi_isList},
    {"is_map", pi_isMap},
    {"as_num", pi_asNum},
    {"as_str", pi_asStr},
    {"as_bool", pi_asBool},

    // Collections
    {"push", pi_push},
    {"pop", pi_pop},
    {"peek", pi_peek},
    {"empty", pi_empty},
    {"sort", pi_sort},
    {"insert", pi_insert},
    {"unshift", pi_unshift},
    {"remove", pi_remove},
    {"append", pi_append},
    {"contains", pi_contains},
    {"index_of", pi_indexOf},
    {"reverse", pi_reverse},
    {"shuffle", pi_shuffle},
    {"copy", pi_copy},
    {"slice", pi_slice},
    {"len", pi_len},
    {"range", pi_range},

    // Functional
    {"map", _pi_map},
    {"filter", pi_filter},
    {"reduce", pi_reduce},
    {"find", pi_find},

    // Matrix
    {"size", pi_size},
    {"mult", pi_mult},
    {"dot", pi_dot},
    {"cross", pi_cross},
    {"eye", pi_eye},
    {"zeros", pi_zeros},
    {"ones", pi_ones},
    {"is_mat", pi_isMat},

    // Object
    {"clone", pi_clone},
    {"values", pi_values},
    {"keys", pi_keys},

    // 3D
    {"load3d", pi_load3d},
    {"rot3d", pi_rotate3d},
    {"tran3d", pi_translate3d},
    {"scale3d", pi_scale3d},
    {"proj3d", pi_project3d},
    {"rend3d", pi_render3d},
};

int BUILTIN_FUNC_COUNT = sizeof(builtin_functions) / sizeof(BuiltinFunc);
