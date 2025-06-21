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
        error("[load3d] expects a file path string.");

    const char *filename = AS_CSTRING(argv[0]);
    FILE *file = fopen(filename, "r");
    if (!file)
        error("[load3d] can't open file: %s", filename);

    // Temporary list to store raw vertex data.
    list_t *vertices = list_create(sizeof(vec3d));

    // Final list to store triangles with color.
    list_t *triangles = list_create(sizeof(triangle));

    // Reads the file line by line and parses the vertices and triangles.
    char line[256];
    while (fgets(line, sizeof(line), file))
    {
        if (line[0] == '#' || line[0] == 's')
            continue;

        if (line[0] == 'v')
        {
            vec3d v;
            sscanf(line, "v %f %f %f", &v.x, &v.y, &v.z);
            list_add(vertices, &v);
        }
        else if (line[0] == 'f')
        {
            int a, b, c;
            sscanf(line, "f %d %d %d", &a, &b, &c);

            triangle t;
            t.v[0] = ((vec3d *)vertices->data)[a - 1];
            t.v[1] = ((vec3d *)vertices->data)[b - 1];
            t.v[2] = ((vec3d *)vertices->data)[c - 1];
            t.color = 0; // default color, can be set later

            list_add(triangles, &t);
        }
    }

    fclose(file);
    free(vertices); // no longer needed after triangulation

    return NEW_OBJ(new_model3d(triangles));
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
Value pi_rotate(vm_t *vm, int argc, Value *argv)
{
    if (argc < 4 || !IS_OBJ(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]) || !IS_NUM(argv[3]))
        error("[rotate] expects model, rx, ry, rz");

    if (!IS_MODEL(argv[0]))
        error("[rotate] First argument must be a 3D model");

    ObjModel3d *model = (ObjModel3d *)AS_OBJ(argv[0]);

    // Convert degrees to radians
    float rx = AS_NUM(argv[1]) * DEG_TO_RAD;
    float ry = AS_NUM(argv[2]) * DEG_TO_RAD;
    float rz = AS_NUM(argv[3]) * DEG_TO_RAD;

    // Precompute sine and cosine for each axis
    float sx = sinf(rx), cx = cosf(rx);
    float sy = sinf(ry), cy = cosf(ry);
    float sz = sinf(rz), cz = cosf(rz);

    // Create new model object
    ObjModel3d *rotated = new_model3d(list_create(sizeof(triangle)));

    // Iterate over each triangle in the model
    for (int i = 0; i < model->triangles->size; i++)
    {
        triangle t = *(triangle *)list_getAt(model->triangles, i);
        triangle r;

        // Iterate over each vertex in the triangle
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
        }

        r.color = t.color; // keep original color

        list_add(rotated->triangles, &r);
    }

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
Value pi_translate(vm_t *vm, int argc, Value *argv)
{
    if (argc < 4 || !IS_OBJ(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]) || !IS_NUM(argv[3]))
        error("[translate] expects model, tx, ty, tz");

    if (!IS_MODEL(argv[0]))
        error("[translate] First argument must be a 3D model");

    ObjModel3d *in = (ObjModel3d *)AS_OBJ(argv[0]);
    float tx = AS_NUM(argv[1]);
    float ty = AS_NUM(argv[2]);
    float tz = AS_NUM(argv[3]);

    ObjModel3d *out = new_model3d(list_create(sizeof(triangle)));
    out->triangles = list_create(sizeof(triangle));

    for (int i = 0; i < in->triangles->size; i++)
    {
        triangle tri = *(triangle *)list_getAt(in->triangles, i);
        for (int j = 0; j < 3; j++)
        {
            // Translate each vertex of the triangle
            tri.v[j].x += tx;
            tri.v[j].y += ty;
            tri.v[j].z += tz;
        }
        list_add(out->triangles, &tri);
    }

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
Value pi_scale(vm_t *vm, int argc, Value *argv)
{
    if (argc < 4 || !IS_OBJ(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]) || !IS_NUM(argv[3]))
        error("[scale] expects model, sx, sy, sz");

    if (!IS_MODEL(argv[0]))
        error("[scale] First argument must be a 3D model");

    ObjModel3d *in = (ObjModel3d *)AS_OBJ(argv[0]);

    float sx = AS_NUM(argv[1]);
    float sy = AS_NUM(argv[2]);
    float sz = AS_NUM(argv[3]);

    // Create new model object with scaled vertices
    ObjModel3d *out = new_model3d(list_create(sizeof(triangle)));
    out->triangles = list_create(sizeof(triangle));

    for (int i = 0; i < in->triangles->size; i++)
    {
        triangle tri = *(triangle *)list_getAt(in->triangles, i);
        for (int j = 0; j < 3; j++)
        {
            tri.v[j].x *= sx;
            tri.v[j].y *= sy;
            tri.v[j].z *= sz;
        }
        list_add(out->triangles, &tri);
    }

    return NEW_OBJ(out);
}

/**
 * Projects a 3D model to a 2D plane.
 *
 * @param fov: The field of view in degrees (angle in the vertical direction)
 * @param cameraZ: The Z position of the camera (positive for eye level)
 * @return A new 3D model object with the projected vertices and same color as the input model.
 */
Value pi_project(vm_t *vm, int argc, Value *argv)
{
    if (argc < 3 || !IS_MODEL(argv[0]) || !IS_NUM(argv[1]) || !IS_NUM(argv[2]))
        error("[project3d] expects a 3D model object and two numbers (fov, cameraZ)");

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

    vec3d lightDir = {0.0f, 0.0f, -1.0f}; // From camera forward

    list_t *_tris = list_create(sizeof(triangle));

    for (int i = 0; i < model->triangles->size; i++)
    {
        triangle t = *(triangle *)list_getAt(model->triangles, i);
        triangle projected;

        // Calculate brightness BEFORE projection using normal
        vec3d normal = norm(t);

        // Compute light intensity
        float intensity = dot(normal, lightDir);
        float brightness = intensity * 0.5f + 0.6f;

        if (brightness < 0.0f)
            brightness = 0.0f;
        if (brightness > 1.0f)
            brightness = 1.0f;

        projected.brightness = brightness;

        // Project each vertex
        for (int j = 0; j < 3; j++)
        {
            vec3d in = t.v[j];
            float px = in.x;
            float py = in.y;
            float pz = in.z - cameraZ; // camera Z offset
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

            // Map to screen (128×128)
            projected.v[j].x = (tx + 1.0f) * 64.0f;
            projected.v[j].y = (ty + 1.0f) * 64.0f;
            projected.v[j].z = tz;
        }

        projected.color = t.color; // preserve color if needed
        list_add(_tris, &projected);
    }

    ObjModel3d *_model = new_model3d(_tris);
    return NEW_OBJ(_model);
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
    float z1 = average(t1);
    float z2 = average(t2);
    // Sort in reverse: far (larger Z) first
    return (z1 < z2) ? 1 : (z1 > z2 ? -1 : 0);
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
Value pi_render(vm_t *vm, int argc, Value *argv)
{
    if (argc < 1 || !IS_MODEL(argv[0]))
        error("[render] expects a 3D model");

    ObjModel3d *model = (ObjModel3d *)AS_OBJ(argv[0]);

    int color = 6; // default to white
    if (argc > 1 && IS_NUM(argv[1]))
        color = ((int)round(AS_NUM(argv[1])) % 32);

    bool filled = false;
    if (argc > 2)
        filled = as_bool(argv[2]);

    // Sort triangles back-to-front (painter's algorithm)
    qsort(model->triangles->data, model->triangles->size, sizeof(triangle), compare_triangles);

    for (int i = 0; i < model->triangles->size; i++)
    {
        triangle t = *(triangle *)list_getAt(model->triangles, i);

        t.color = color;

        // Skip rendering if the triangle is not visible
        if (!is_triangleVisible(t))
            continue;

        if (filled)
            draw_fillTriangle(vm->screen,
                              t.v[0].x, t.v[0].y,
                              t.v[1].x, t.v[1].y,
                              t.v[2].x, t.v[2].y,
                              t.color, t.brightness);
        else
            draw_triangle(vm->screen,
                          t.v[0].x, t.v[0].y,
                          t.v[1].x, t.v[1].y,
                          t.v[2].x, t.v[2].y,
                          t.color);
    }

    return NEW_NIL();
}
