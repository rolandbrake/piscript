#include <math.h>
#include "pi_render.h"

#include "../pi_value.h"

/**
 * Computes the normalized normal vector of a triangle.
 *
 * @param t The triangle to compute the normal from.
 * @return The normalized normal vector.
 */
vec3d norm(triangle t)
{
    vec3d n = {0, 0, 0};

    // Compute vectors
    float ax = t.v[1].x - t.v[0].x;
    float ay = t.v[1].y - t.v[0].y;
    float az = t.v[1].z - t.v[0].z;

    float bx = t.v[2].x - t.v[0].x;
    float by = t.v[2].y - t.v[0].y;
    float bz = t.v[2].z - t.v[0].z;

    // Cross product (line1 x line2) to get normal
    n.x = ay * bz - az * by;
    n.y = az * bx - ax * bz;
    n.z = ax * by - ay * bx;

    // Normalize normal (optional but helps)
    float length = sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
    if (length != 0.0f)
    {
        n.x /= length;
        n.y /= length;
        n.z /= length;
    }

    return n;
}

float dot(vec3d a, vec3d b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

/**
 * Loads a 3D model from a file in Wavefront OBJ format.
 *
 * @param filename The path to the file to load.
 * @return A new 3D model object with the loaded vertices and triangles.
 */
Value pi_load3d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_STRING(argv[0]))
        vm_error(vm,"[load3d] expects at least 1 argument: file path");

    const char *filename = AS_CSTRING(argv[0]);
    FILE *file = fopen(filename, "r");
    if (!file)
        vm_errorf(vm,"[load3d] can't open file: %s", filename);

    ObjImage *texture = NULL;
    if (argc >= 2)
    {
        if (!IS_OBJ(argv[1]) || ((Object *)AS_OBJ(argv[1]))->type != OBJ_IMAGE)
            vm_error(vm,"[load3d] second argument must be an image object");
        texture = (ObjImage *)AS_OBJ(argv[1]);
    }

    list_t *vertices = list_create(sizeof(vec3d));
    list_t *uvs = list_create(sizeof(vec2d));
    list_t *tris = list_create(sizeof(triangle));

    bool has_texture = false;

    char line[256];
    short color = -1;
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#' || line[0] == 's')
        {
            // Handle custom color metadata: # color r g b
            if (strncmp(line, "# color", 7) == 0)
            {
                int r, g, b;
                if (sscanf(line, "# color %d %d %d", &r, &g, &b) == 3)
                    color = (short)find_paletteColor(r, g, b);
            }
            continue;
        }

        if (strncmp(line, "v ", 2) == 0)
        {
            vec3d v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            list_add(vertices, &v);
        }
        else if (strncmp(line, "vt ", 3) == 0)
        {
            vec2d uv;
            sscanf(line, "vt %f %f", &uv.u, &uv.v);
            list_add(uvs, &uv);
            has_texture = true;
        }
        else if (strncmp(line, "f ", 2) == 0)
        {
            triangle t = {0};
            if (has_texture)
            {
                int vi[3], ti[3];
                int matches = sscanf(line, "f %d/%d %d/%d %d/%d",
                                     &vi[0], &ti[0], &vi[1], &ti[1], &vi[2], &ti[2]);
                if (matches < 6)
                    vm_error(vm,"[load3d] face line must be in format: f v1/vt1 v2/vt2 v3/vt3");

                for (int i = 0; i < 3; i++)
                {
                    t.v[i] = *(vec3d *)list_getAt(vertices, vi[i] - 1);
                    t.t[i] = *(vec2d *)list_getAt(uvs, ti[i] - 1);
                }
            }
            else
            {
                int vi[3];
                int matches = sscanf(line, "f %d %d %d", &vi[0], &vi[1], &vi[2]);
                if (matches < 3)
                    vm_error(vm,"[load3d] face line must be in format: f v1 v2 v3");

                for (int i = 0; i < 3; i++)
                {
                    t.v[i] = *(vec3d *)list_getAt(vertices, vi[i] - 1);
                    t.t[i] = (vec2d){0, 0};
                }
            }

            t.color = color;
            t.brightness = 1.0f;
            list_add(tris, &t);
        }
    }

    fclose(file);

    // Copy triangles into contiguous array for model
    triangle *data = malloc(sizeof(triangle) * tris->size);
    memcpy(data, tris->data, sizeof(triangle) * tris->size);

    ObjModel3d *model = new_model3d(data, tris->size, texture);

    list_free(vertices);
    list_free(uvs);
    list_free(tris);

    return NEW_OBJ(model);
}

/**
 * Rotate a 3D model by a given angle around all three axes.
 *
 * @param model: The 3D model to rotate.
 * @param rx: The rotation angle in radians around the x-axis.
 * @param ry: The rotation angle in radians around the y-axis.
 * @param rz: The rotation angle in radians around the z-axis.
 * @return A new 3D model object with the rotated vertices and same color as the input model.
 */
Value pi_rotate3d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 4 || !IS_OBJ(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]) || !IS_NUM(argv[3]))
        vm_error(vm,"[rotate] expects model, rx, ry, rz");

    if (!IS_MODEL(argv[0]))
        vm_error(vm,"[rotate] First argument must be a 3D model");

    ObjModel3d *model = (ObjModel3d *)AS_OBJ(argv[0]);

    // Convert degrees to radians
    float rx = AS_NUM(argv[1]) * DEG_TO_RAD;
    float ry = AS_NUM(argv[2]) * DEG_TO_RAD;
    float rz = AS_NUM(argv[3]) * DEG_TO_RAD;

    // Precompute sine and cosine for each axis
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float sz = sinf(rz), cz = cosf(rz);

    // Allocate new triangle array
    int count = model->count;
    triangle *rotated_tris = malloc(sizeof(triangle) * count);

    for (int i = 0; i < count; i++)
    {
        triangle t = model->triangles[i];
        triangle r;

        for (int j = 0; j < 3; j++)
        {
            vec3d p = t.v[j];

            // Rotate around X
            float y1 = p.y * cx - p.z * sx;
            float z1 = p.y * sx + p.z * cx;
            float x1 = p.x;

            // Rotate around Y
            float x2 = x1 * cy + z1 * sy;
            float z2 = -x1 * sy + z1 * cy;
            float y2 = y1;

            // Rotate around Z
            float x3 = x2 * cz - y2 * sz;
            float y3 = x2 * sz + y2 * cz;
            float z3 = z2;

            r.v[j] = (vec3d){x3, y3, z3};
            r.t[j] = t.t[j]; // Preserve UV coordinates
        }

        r.color = t.color;
        r.brightness = t.brightness;
        rotated_tris[i] = r;
    }

    // Create new model with rotated triangles and original texture
    ObjModel3d *rotated = new_model3d(rotated_tris, count, model->texture);

    return NEW_OBJ(rotated);
}

/**
 * Translates a 3D model by a given offset in all three dimensions.
 *
 * @param model: The 3D model to translate.
 * @param tx: The translation offset in the x-axis.
 * @param ty: The translation offset in the y-axis.
 * @param tz: The translation offset in the z-axis.
 * @return A new 3D model object with the translated vertices and same color as the input model.
 */
Value pi_translate3d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 4 || !IS_OBJ(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]) || !IS_NUM(argv[3]))
        vm_error(vm,"[translate] expects model, tx, ty, tz");

    if (!IS_MODEL(argv[0]))
        vm_error(vm,"[translate] First argument must be a 3D model");

    ObjModel3d *in = (ObjModel3d *)AS_OBJ(argv[0]);
    float tx = AS_NUM(argv[1]);
    float ty = AS_NUM(argv[2]);
    float tz = AS_NUM(argv[3]);

    int count = in->count;
    triangle *translated = malloc(sizeof(triangle) * count);
    if (!translated)
        vm_error(vm,"[translate] out of memory");

    for (int i = 0; i < count; i++)
    {
        triangle tri = in->triangles[i];
        triangle t;

        for (int j = 0; j < 3; j++)
        {
            vec3d p = tri.v[j];
            t.v[j] = (vec3d){p.x + tx, p.y + ty, p.z + tz};
            t.t[j] = tri.t[j]; // Copy UV coords
        }

        t.color = tri.color;
        t.brightness = tri.brightness;

        translated[i] = t;
    }

    ObjModel3d *out = new_model3d(translated, count, in->texture);
    return NEW_OBJ(out);
}

/**
 * Scales a 3D model by a factor in all three dimensions.
 *
 * @param model: The 3D model to scale.
 * @param sx: The scale factor in the x-axis.
 * @param sy: The scale factor in the y-axis.
 * @param sz: The scale factor in the z-axis.
 * @return A new 3D model object with the scaled vertices and same color as the input model.
 */
Value pi_scale3d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 4 || !IS_OBJ(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]) || !IS_NUM(argv[3]))
        vm_error(vm,"[scale] expects model, sx, sy, sz");

    if (!IS_MODEL(argv[0]))
        vm_error(vm,"[scale] First argument must be a 3D model");

    ObjModel3d *in = (ObjModel3d *)AS_OBJ(argv[0]);

    float sx = AS_NUM(argv[1]);
    float sy = AS_NUM(argv[2]);
    float sz = AS_NUM(argv[3]);

    int count = in->count;
    triangle *scaled = malloc(sizeof(triangle) * count);
    if (!scaled)
        vm_error(vm,"[scale] out of memory");

    for (int i = 0; i < count; i++)
    {
        triangle tri = in->triangles[i];
        triangle t;

        for (int j = 0; j < 3; j++)
        {
            vec3d p = tri.v[j];
            t.v[j] = (vec3d){p.x * sx, p.y * sy, p.z * sz};
            t.t[j] = tri.t[j]; // Preserve UV coordinates
        }

        t.color = tri.color;
        t.brightness = tri.brightness;
        scaled[i] = t;
    }

    ObjModel3d *out = new_model3d(scaled, count, in->texture);
    return NEW_OBJ(out);
}

/**
 * Projects a 3D model to a 2D plane.
 *
 * @param fov: The field of view in degrees (angle in the vertical direction)
 * @param cameraZ: The Z position of the camera (positive for eye level)
 * @return A new 3D model object with the projected vertices and same color as the input model.
 */
Value pi_project3d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3 || !IS_MODEL(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]))
        vm_error(vm,"[project3d] expects a 3D model object and two numbers (fov, cameraZ)");

    ObjModel3d *model = (ObjModel3d *)AS_OBJ(argv[0]);
    float fov = AS_NUM(argv[1]);
    float cameraZ = AS_NUM(argv[2]);

    float fNear = 0.1f;
    float fFar = 1000.0f;
    float fAspectRatio = 1.0f; // square screen
    float fFovRad = 1.0f / tanf(fov * 0.5f / 180.0f * 3.14159f);

    // Build projection matrix
    float m[4][4] = {0};
    m[0][0] = fAspectRatio * fFovRad;
    m[1][1] = fFovRad;
    m[2][2] = fFar / (fFar - fNear);
    m[3][2] = (-fFar * fNear) / (fFar - fNear);
    m[2][3] = 1.0f;
    m[3][3] = 0.0f;

    vec3d lightDir = {0.0f, 0.0f, -1.0f};

    int count = model->count;
    triangle *projected_tris = malloc(sizeof(triangle) * count);
    if (!projected_tris)
        vm_error(vm,"[project3d] out of memory");

    for (int i = 0; i < count; i++)
    {
        triangle t = model->triangles[i];
        triangle projected;

        // Compute surface normal and brightness
        vec3d normal = norm(t);
        float intensity = dot(normal, lightDir);
        float brightness = intensity * 0.5f + 0.6f;

        if (brightness < 0.0f)
            brightness = 0.0f;
        if (brightness > 1.0f)
            brightness = 1.0f;

        projected.brightness = brightness;

        for (int j = 0; j < 3; j++)
        {
            vec3d in = t.v[j];
            float px = in.x;
            float py = in.y;
            float pz = in.z - cameraZ;
            float pw = 1.0f;

            float tx = px * m[0][0] + py * m[1][0] + pz * m[2][0] + pw * m[3][0];
            float ty = px * m[0][1] + py * m[1][1] + pz * m[2][1] + pw * m[3][1];
            float tz = px * m[0][2] + py * m[1][2] + pz * m[2][2] + pw * m[3][2];
            float tw = px * m[0][3] + py * m[1][3] + pz * m[2][3] + pw * m[3][3];

            if (tw != 0.0f)
            {
                tx /= tw;
                ty /= tw;
                tz /= tw;
            }

            // Convert to screen space (128×128)
            projected.v[j].x = (tx + 1.0f) * 64.0f;
            projected.v[j].y = (ty + 1.0f) * 64.0f;
            projected.v[j].z = tz;

            // Preserve UV coordinates
            projected.t[j] = t.t[j];
        }

        projected.color = t.color;
        projected_tris[i] = projected;
    }

    ObjModel3d *out = new_model3d(projected_tris, count, model->texture);
    return NEW_OBJ(out);
}

/**
 * Checks if a triangle is visible to the camera (i.e. if it faces the camera).
 *
 * @param t The triangle to check.
 * @return True if the triangle is visible, false if it is not.
 */
bool is_triangleVisible(triangle t)
{

    // Compute vectors
    float ax = t.v[1].x - t.v[0].x;
    float ay = t.v[1].y - t.v[0].y;
    float az = t.v[1].z - t.v[0].z;

    float bx = t.v[2].x - t.v[0].x;
    float by = t.v[2].y - t.v[0].y;
    float bz = t.v[2].z - t.v[0].z;

    // Cross product (line1 x line2) to get normal
    float nx = ay * bz - az * by;
    float ny = az * bx - ax * bz;
    float nz = ax * by - ay * bx;

    // Normalize normal (optional but helps)
    float length = sqrtf(nx * nx + ny * ny + nz * nz);
    if (length != 0.0f)
    {
        nx /= length;
        ny /= length;
        nz /= length;
    }

    // Camera ray from origin to triangle point 0
    float rx = t.v[0].x;
    float ry = t.v[0].y;
    float rz = t.v[0].z;

    // Dot product of normal and camera ray
    float dp = nx * rx + ny * ry + nz * rz;

    // If the dot product is less than 0, the triangle faces the camera
    return dp < 0.0f;
}

// --- Triangle Drawing with Lighting ---

/**
 * Draws the outline of a triangle with the specified vertices (x0, y0), (x1, y1), (x2, y2) on the screen.
 *
 * @param screen The screen to draw on.
 * @param x0 First x-coordinate of the triangle.
 * @param y0 First y-coordinate of the triangle.
 * @param x1 Second x-coordinate of the triangle.
 * @param y1 Second y-coordinate of the triangle.
 * @param x2 Third x-coordinate of the triangle.
 * @param y2 Third y-coordinate of the triangle.
 * @param color The color of the triangle outline.
 */
void draw_triangle(Screen *screen, float x0, float y0, float x1, float y1, float x2, float y2, int color)
{
    // Draw three lines to form the triangle outline
    draw_line(screen, (int)x0, (int)y0, (int)x1, (int)y1, color);
    draw_line(screen, (int)x1, (int)y1, (int)x2, (int)y2, color);
    draw_line(screen, (int)x2, (int)y2, (int)x0, (int)y0, color);
}

/**
 * Draws a filled triangle with the specified vertices (x0, y0), (x1, y1), (x2, y2) on the screen.
 *
 * @param screen The screen to draw on.
 * @param x0 First x-coordinate of the triangle.
 * @param y0 First y-coordinate of the triangle.
 * @param x1 Second x-coordinate of the triangle.
 * @param y1 Second y-coordinate of the triangle.
 * @param x2 Third x-coordinate of the triangle.
 * @param y2 Third y-coordinate of the triangle.
 * @param color The color of the triangle to draw.
 * @param brightness Adjusts the brightness of the triangle by scaling the color's RGB values.
 */
void draw_fillTriangle(Screen *screen, float x0, float y0, float x1, float y1, float x2, float y2, int color, float brightness)
{
    // Approximate shading by darkening base color
    int _color = color;

    int minY = (int)fminf(y0, fminf(y1, y2));
    int maxY = (int)fmaxf(y0, fmaxf(y1, y2));

    for (int y = minY; y <= maxY; y++)
    {
        float xs[3], ys[3];
        xs[0] = x0;
        ys[0] = y0;
        xs[1] = x1;
        ys[1] = y1;
        xs[2] = x2;
        ys[2] = y2;

        float inters[3];
        int count = 0;

        for (int i = 0; i < 3; i++)
        {
            int j = (i + 1) % 3;
            float y1 = ys[i], y2 = ys[j];
            float x1 = xs[i], x2 = xs[j];

            if ((y1 < y && y2 >= y) || (y2 < y && y1 >= y))
            {
                float t = (y - y1) / (y2 - y1);
                inters[count++] = x1 + t * (x2 - x1);
            }
        }

        if (count == 2)
        {
            int x_start = (int)fminf(inters[0], inters[1]);
            int x_end = (int)fmaxf(inters[0], inters[1]);

            // Draw the triangle line
            for (int x = x_start; x <= x_end; x++)
                set_pixel_shaded(screen, x, y, _color, brightness);
        }
    }
}

/**
 * Linear interpolation between two values `a` and `b` using the parameter `t` to
 * compute the weighted average.
 *
 * @param a The first value.
 * @param b The second value.
 * @param t The interpolation parameter. Must be in the range [0, 1].
 * @return The interpolated value.
 */
float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

/**
 * Clamps the given value to the specified range [min, max].
 *
 * @param val The value to clamp.
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @return The clamped value.
 */
int clamp(int val, int min, int max)
{
    return val < min ? min : (val > max ? max : val);
}

/**
 * Draws a textured triangle on the screen.
 *
 * @param screen The screen to draw on.
 * @param p0 The first vertex of the triangle.
 * @param t0 The texture coordinates of the first vertex.
 * @param p1 The second vertex of the triangle.
 * @param t1 The texture coordinates of the second vertex.
 * @param p2 The third vertex of the triangle.
 * @param t2 The texture coordinates of the third vertex.
 * @param texture The texture to use.
 * @param brightness The brightness of the triangle (0.0 to 1.0).
 */
void draw_texturedTriangle(
    Screen *screen,
    vec3d p0, vec2d t0,
    vec3d p1, vec2d t1,
    vec3d p2, vec2d t2,
    ObjImage *texture,
    float brightness)
{
    // Compute bounding box of the triangle
    float minX = fminf(fminf(p0.x, p1.x), p2.x);
    float maxX = fmaxf(fmaxf(p0.x, p1.x), p2.x);
    float minY = fminf(fminf(p0.y, p1.y), p2.y);
    float maxY = fmaxf(fmaxf(p0.y, p1.y), p2.y);

    int x0 = clamp((int)floorf(minX), 0, SCREEN_WIDTH - 1);
    int x1 = clamp((int)ceilf(maxX), 0, SCREEN_WIDTH - 1);
    int y0 = clamp((int)floorf(minY), 0, SCREEN_HEIGHT - 1);
    int y1 = clamp((int)ceilf(maxY), 0, SCREEN_HEIGHT - 1);

    // Precompute edge functions
    float denom = (p1.y - p2.y) * (p0.x - p2.x) + (p2.x - p1.x) * (p0.y - p2.y);
    if (fabsf(denom) < 1e-6f)
        return; // Degenerate triangle

    for (int y = y0; y <= y1; y++)
    {
        for (int x = x0; x <= x1; x++)
        {
            float px = x + 0.5f;
            float py = y + 0.5f;

            // Barycentric weights
            float w0 = ((p1.y - p2.y) * (px - p2.x) + (p2.x - p1.x) * (py - p2.y)) / denom;
            float w1 = ((p2.y - p0.y) * (px - p2.x) + (p0.x - p2.x) * (py - p2.y)) / denom;
            float w2 = 1.0f - w0 - w1;

            // Inside triangle check
            if (w0 < 0.0f || w1 < 0.0f || w2 < 0.0f)
                continue;

            // Interpolate texture coordinates
            float u = w0 * t0.u + w1 * t1.u + w2 * t2.u;
            float v = w0 * t0.v + w1 * t1.v + w2 * t2.v;

            // Clamp UV to valid range
            int tx = clamp((int)(u * texture->width), 0, texture->width - 1);
            int ty = clamp((int)(v * texture->height), 0, texture->height - 1);

            uint8_t color_index = texture->pixels[ty * texture->width + tx];

            set_pixel_shaded(screen, x, y, color_index, brightness);
        }
    }
}

/**
 * Calculates the average Z-coordinate of the vertices of a triangle.
 *
 * @param t The triangle to calculate the average Z-coordinate for.
 * @return The average Z-coordinate of the triangle's vertices.
 */
float average(const triangle *t)
{
    return (t->v[0].z + t->v[1].z + t->v[2].z) / 3.0f;
}

/**
 * Compares two triangles by their average Z-coordinate.
 *
 * @param a The first triangle to compare.
 * @param b The second triangle to compare.
 * @return A negative value if the first triangle is behind the second,
 *         zero if they are at the same depth, and a positive value if the
 *         first triangle is in front of the second.
 */
int compare_triangles(const void *a, const void *b)
{
    const triangle *t1 = (const triangle *)a;
    const triangle *t2 = (const triangle *)b;

    float z1 = (t1->v[0].z + t1->v[1].z + t1->v[2].z) / 3.0f;
    float z2 = (t2->v[0].z + t2->v[1].z + t2->v[2].z) / 3.0f;

    return (z1 < z2) ? 1 : (z1 > z2) ? -1
                                     : 0; // Descending order
}

// --- Lighting-enhanced pi_render() ---

/**
 * Renders a 3D model on the screen using the painter's algorithm.
 *
 * This function expects between 1 and 3 arguments: the 3D model to render,
 * the color index to use for rendering, and optionally a boolean indicating
 * whether the model should be rendered as filled or outlined.
 *
 * @param vm The virtual machine instance.
 * @param argc The number of arguments (1 to 3).
 * @param argv The arguments: model (3D model), color (int, optional), filled (bool, optional).
 * @return A nil value indicating completion.
 */
Value pi_render3d(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_MODEL(argv[0]))
        vm_error(vm,"[render] expects a 3D model");

    ObjModel3d *model = (ObjModel3d *)AS_OBJ(argv[0]);

    int color = 6; // Default color
    if (argc > 1 && IS_NUM(argv[1]))
        color = ((int)round(AS_NUM(argv[1]))) % 32;

    bool filled = false;
    if (argc > 2)
        filled = as_bool(argv[2]);

    // Sort triangles by depth (back-to-front)
    qsort(model->triangles, model->count, sizeof(triangle), compare_triangles);

    for (int i = 0; i < model->count; i++)
    {
        triangle t = model->triangles[i];

        // Skip invisible triangles
        if (!is_triangleVisible(t))
            continue;

        // Apply solid color if no texture
        if (!model->texture && t.color == -1)
            t.color = color;

        if (filled)
        {

            if (model->texture)
            {
                // Textured triangle fill (with UVs and texture reference)
                draw_texturedTriangle(
                    vm->screen,
                    t.v[0], t.t[0],
                    t.v[1], t.t[1],
                    t.v[2], t.t[2],
                    model->texture,
                    t.brightness);
            }
            else
            {
                // Solid fill triangle
                draw_fillTriangle(
                    vm->screen,
                    t.v[0].x, t.v[0].y,
                    t.v[1].x, t.v[1].y,
                    t.v[2].x, t.v[2].y,
                    t.color,
                    t.brightness);
            }
        }
        else
        {
            // Wireframe render
            draw_triangle(
                vm->screen,
                t.v[0].x, t.v[0].y,
                t.v[1].x, t.v[1].y,
                t.v[2].x, t.v[2].y,
                t.color);
        }
    }

    return NEW_NIL();
}
