#ifndef PI_BUILTIN_H
#define PI_BUILTIN_H

#include "pi_math.h"   // Math functions
#include "pi_string.h" // String functions
#include "pi_io.h"     // Input/Output functions
#include "pi_sys.h"    // System-related functions
#include "pi_plot.h"   // Graphics functions
#include "pi_time.h"   // Time functions
#include "pi_audio.h"  // Audio functions
#include "pi_col.h"    // Color functions
#include "pi_fun.h"    // Function functions
#include "pi_mat.h"    // Matrix functions
#include "pi_type.h"   // Type functions
#include "pi_obj.h"    // Object functions
#include "pi_render.h" // 3D rendering functions
#include "pi_img.h"    // Image functions

// Builtin functions struct definition
typedef struct
{
    char *name;                          // the name of the function
    Value (*func)(vm_t *, int, Value *); // the body of the function
} BuiltinFunc;

typedef struct
{
    char *name;  // the name of the constant
    Value value; // the value of the constant
} BuiltinConst;

// List of all builtin functions
extern BuiltinFunc builtin_functions[];
// Number of builtin functions
extern int BUILTIN_FUNC_COUNT;
// List of all builtin constants
extern BuiltinConst builtin_constants[];
// Number of builtin constants
extern int BUILTIN_CONST_COUNT;

#endif // PI_BUILTIN_H