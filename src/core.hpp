#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

#ifdef __APPLE__
#  ifndef GL_SILENCE_DEPRECATION
#    define GL_SILENCE_DEPRECATION
#  endif
#  include <OpenGL/OpenGL.h>
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#else
#  ifndef GL_GLEXT_PROTOTYPES
#    define GL_GLEXT_PROTOTYPES  // glGenBuffers and friends
#  endif
#  ifdef _WIN32
#    include <windows.h>
#  endif
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glut.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
#include <map>

// ============================================================================
// Math helpers
// ============================================================================

static const float PI = 3.14159265f;
static const float TWO_PI = 6.2831853f;
static const float RAD2DEG = 57.2957795f;

struct V3 {
    float x, y, z;
    V3() : x(0), y(0), z(0) {}
    V3(float a, float b, float c) : x(a), y(b), z(c) {}
    V3 operator+(const V3& o) const { return V3(x + o.x, y + o.y, z + o.z); }
    V3 operator-(const V3& o) const { return V3(x - o.x, y - o.y, z - o.z); }
    V3 operator*(float s) const { return V3(x * s, y * s, z * s); }
    V3& operator+=(const V3& o) { x += o.x; y += o.y; z += o.z; return *this; }
    V3& operator-=(const V3& o) { x -= o.x; y -= o.y; z -= o.z; return *this; }
    V3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
};

static float dot(const V3& a, const V3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static V3 cross(const V3& a, const V3& b) {
    return V3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
static float len(const V3& a) { return sqrtf(dot(a, a)); }
static V3 norm(const V3& a) {
    float l = len(a);
    return l > 1e-6f ? a * (1.0f / l) : V3(0, 0, 0);
}
static float clampf(float v, float a, float b) { return v < a ? a : (v > b ? b : v); }
static float lerpf(float a, float b, float t) { return a + (b - a) * t; }
static float approach(float v, float target, float step) {
    if (v < target) return std::min(v + step, target);
    return std::max(v - step, target);
}
static float wrapAngle(float a) {
    while (a > PI) a -= TWO_PI;
    while (a < -PI) a += TWO_PI;
    return a;
}
static float signf(float v) { return v < 0 ? -1.0f : 1.0f; }
// Forward vector for a yaw angle. Yaw 0 faces +Z, positive yaw turns toward +X,
// which matches gRotate(yaw, 0, 1, 0).
static V3 yawDir(float yaw) { return V3(sinf(yaw), 0, cosf(yaw)); }
static float dirYaw(float x, float z) { return atan2f(x, z); }

// World generation RNG. Seeded once so the city is the same every run.
static unsigned int g_seed = 1234567u;
static float frand() {
    g_seed = g_seed * 1664525u + 1013904223u;
    return (float)((g_seed >> 8) & 0xFFFFFF) / 16777216.0f;
}
static float frange(float a, float b) { return a + (b - a) * frand(); }
static int irange(int a, int b) { return std::min(b, a + (int)(frand() * (float)(b - a + 1))); }

// Gameplay RNG, kept separate so gameplay noise never changes the city.
static unsigned int g_playSeed = 99991u;
static float prand() {
    g_playSeed = g_playSeed * 1103515245u + 12345u;
    return (float)((g_playSeed >> 9) & 0x7FFFFF) / 8388608.0f;
}

struct Col { float r, g, b; };
static Col C(float r, float g, float b) { Col c = {r, g, b}; return c; }
static void gColor(float r, float g, float b);
static void setc(const Col& c) { gColor(c.r, c.g, c.b); }
static void setc(float r, float g, float b) { gColor(r, g, b); }
static Col mulc(const Col& c, float k) {
    return C(std::min(1.0f, c.r * k), std::min(1.0f, c.g * k), std::min(1.0f, c.b * k));
}
static Col mixc(const Col& a, const Col& b, float t) {
    return C(lerpf(a.r, b.r, t), lerpf(a.g, b.g, t), lerpf(a.b, b.b, t));
}

// Common colours
static const Col COL_WOOD = {0.52f, 0.33f, 0.18f};
static const Col COL_STEEL = {0.70f, 0.72f, 0.75f};
static const Col COL_WHITE = {0.95f, 0.95f, 0.92f};
static const Col COL_BLACK = {0.05f, 0.05f, 0.05f};
static const Col COL_YELLOW = {0.98f, 0.80f, 0.10f};
static const Col COL_SKIN[4] = {{0.55f, 0.36f, 0.24f}, {0.45f, 0.29f, 0.19f}, {0.66f, 0.46f, 0.32f}, {0.38f, 0.25f, 0.17f}};

// ============================================================================
// Geometry recorder
//
// Drawing code calls the g* functions below instead of raw OpenGL. Normally
// they pass straight through. While recording, they transform vertices on the
// CPU and collect them into flat triangle and line lists, which are compiled
// into a display list with one glDrawArrays each. The city has tens of
// thousands of small shapes; drawn one glBegin at a time it ran at 3 fps.
// ============================================================================

struct Mesh {
    std::vector<float> tv, tn;      // lit triangles: positions, normals
    std::vector<unsigned char> tc;  // and colours
    std::vector<float> uv;          // unlit triangles (signs, lamps)
    std::vector<unsigned char> uc;
    std::vector<float> lv;          // lines
    std::vector<unsigned char> lc;
    void clear() {
        tv.clear(); tn.clear(); tc.clear();
        uv.clear(); uc.clear();
        lv.clear(); lc.clear();
    }
};

struct Chunk {
    GLuint list;
    float lo[3], hi[3];
    size_t verts;
};

struct Mat4 { float m[16]; };  // column-major, like OpenGL
static Mat4 matIdentity() {
    Mat4 r;
    for (int i = 0; i < 16; i++) r.m[i] = (i % 5 == 0) ? 1.0f : 0.0f;
    return r;
}
static Mat4 matMul(const Mat4& a, const Mat4& b) {
    Mat4 r;
    for (int c = 0; c < 4; c++)
        for (int row = 0; row < 4; row++) {
            float s = 0;
            for (int k = 0; k < 4; k++) s += a.m[k * 4 + row] * b.m[c * 4 + k];
            r.m[c * 4 + row] = s;
        }
    return r;
}

static bool g_rec = false;
static Mesh g_recMesh;
static Mat4 g_M = matIdentity();
static std::vector<Mat4> g_mStack;
static bool g_lit = true;
static GLenum g_mode = GL_TRIANGLES;
static float g_cur[3] = {1, 1, 1};
static V3 g_curN(0, 1, 0);
struct RecV { V3 p, n; unsigned char c[4]; };
static std::vector<RecV> g_prim;

static void gColor(float r, float g, float b) {
    if (!g_rec) { glColor3f(r, g, b); return; }
    g_cur[0] = r; g_cur[1] = g; g_cur[2] = b;
}
static void gNormal(float x, float y, float z) {
    if (!g_rec) { glNormal3f(x, y, z); return; }
    g_curN = V3(x, y, z);
}
static void gVertex(float x, float y, float z) {
    if (!g_rec) { glVertex3f(x, y, z); return; }
    const float* m = g_M.m;
    RecV v;
    v.p = V3(m[0] * x + m[4] * y + m[8] * z + m[12], m[1] * x + m[5] * y + m[9] * z + m[13],
             m[2] * x + m[6] * y + m[10] * z + m[14]);
    const V3& n = g_curN;
    v.n = norm(V3(m[0] * n.x + m[4] * n.y + m[8] * n.z, m[1] * n.x + m[5] * n.y + m[9] * n.z,
                  m[2] * n.x + m[6] * n.y + m[10] * n.z));
    for (int i = 0; i < 3; i++) v.c[i] = (unsigned char)(clampf(g_cur[i], 0, 1) * 255.0f + 0.5f);
    v.c[3] = 255;
    g_prim.push_back(v);
}
static void gBegin(GLenum mode) {
    if (!g_rec) { glBegin(mode); return; }
    g_mode = mode;
    g_prim.clear();
}
static void pushVert(std::vector<float>& pos, std::vector<unsigned char>& col, const RecV& v) {
    pos.push_back(v.p.x); pos.push_back(v.p.y); pos.push_back(v.p.z);
    for (int i = 0; i < 4; i++) col.push_back(v.c[i]);
}
// Night lights: while g_recGlow is set, triangles go to g_glowMesh instead,
// which is drawn additively after dark (see nightlights.hpp). Glow code picks
// its random numbers from glowRand so the city itself never changes.
static bool g_recGlow = false;
static Mesh g_glowMesh;
static unsigned int g_glowSeed = 4242u;
static float glowRand() {
    g_glowSeed = g_glowSeed * 1664525u + 1013904223u;
    return (float)((g_glowSeed >> 8) & 0xFFFFFF) / 16777216.0f;
}
static void emitTri(const RecV& a, const RecV& b, const RecV& c) {
    const RecV* vs[3] = {&a, &b, &c};
    for (int i = 0; i < 3; i++) {
        if (g_recGlow) {
            pushVert(g_glowMesh.uv, g_glowMesh.uc, *vs[i]);
        } else if (g_lit) {
            pushVert(g_recMesh.tv, g_recMesh.tc, *vs[i]);
            g_recMesh.tn.push_back(vs[i]->n.x); g_recMesh.tn.push_back(vs[i]->n.y); g_recMesh.tn.push_back(vs[i]->n.z);
        } else {
            pushVert(g_recMesh.uv, g_recMesh.uc, *vs[i]);
        }
    }
}
static void emitLine(const RecV& a, const RecV& b) {
    if (g_recGlow) return;
    pushVert(g_recMesh.lv, g_recMesh.lc, a);
    pushVert(g_recMesh.lv, g_recMesh.lc, b);
}
static void gEnd() {
    if (!g_rec) { glEnd(); return; }
    const std::vector<RecV>& p = g_prim;
    size_t n = p.size();
    switch (g_mode) {
        case GL_TRIANGLES:
            for (size_t i = 0; i + 2 < n; i += 3) emitTri(p[i], p[i + 1], p[i + 2]);
            break;
        case GL_QUADS:
            for (size_t i = 0; i + 3 < n; i += 4) { emitTri(p[i], p[i + 1], p[i + 2]); emitTri(p[i], p[i + 2], p[i + 3]); }
            break;
        case GL_QUAD_STRIP:
            for (size_t i = 0; i + 3 < n; i += 2) { emitTri(p[i], p[i + 1], p[i + 3]); emitTri(p[i], p[i + 3], p[i + 2]); }
            break;
        case GL_TRIANGLE_STRIP:
            for (size_t i = 0; i + 2 < n; i++) {
                if (i & 1) emitTri(p[i + 1], p[i], p[i + 2]);
                else emitTri(p[i], p[i + 1], p[i + 2]);
            }
            break;
        case GL_TRIANGLE_FAN:
            for (size_t i = 1; i + 1 < n; i++) emitTri(p[0], p[i], p[i + 1]);
            break;
        case GL_LINES:
            for (size_t i = 0; i + 1 < n; i += 2) emitLine(p[i], p[i + 1]);
            break;
        case GL_LINE_STRIP:
            for (size_t i = 0; i + 1 < n; i++) emitLine(p[i], p[i + 1]);
            break;
        default: break;
    }
    g_prim.clear();
}
static void gPush() {
    if (!g_rec) { glPushMatrix(); return; }
    g_mStack.push_back(g_M);
}
static void gPop() {
    if (!g_rec) { glPopMatrix(); return; }
    g_M = g_mStack.back();
    g_mStack.pop_back();
}
static void gTranslate(float x, float y, float z) {
    if (!g_rec) { glTranslatef(x, y, z); return; }
    Mat4 t = matIdentity();
    t.m[12] = x; t.m[13] = y; t.m[14] = z;
    g_M = matMul(g_M, t);
}
static void gScale(float x, float y, float z) {
    if (!g_rec) { glScalef(x, y, z); return; }
    Mat4 t = matIdentity();
    t.m[0] = x; t.m[5] = y; t.m[10] = z;
    g_M = matMul(g_M, t);
}
static void gRotate(float deg, float x, float y, float z) {
    if (!g_rec) { glRotatef(deg, x, y, z); return; }
    float l = sqrtf(x * x + y * y + z * z);
    if (l < 1e-8f) return;
    x /= l; y /= l; z /= l;
    float a = deg / RAD2DEG, c = cosf(a), s = sinf(a), ic = 1.0f - c;
    Mat4 r = matIdentity();
    r.m[0] = x * x * ic + c;     r.m[4] = x * y * ic - z * s; r.m[8] = x * z * ic + y * s;
    r.m[1] = y * x * ic + z * s; r.m[5] = y * y * ic + c;     r.m[9] = y * z * ic - x * s;
    r.m[2] = x * z * ic - y * s; r.m[6] = y * z * ic + x * s; r.m[10] = z * z * ic + c;
    g_M = matMul(g_M, r);
}
static void gLighting(bool on) {
    if (!g_rec) {
        if (on) glEnable(GL_LIGHTING);
        else glDisable(GL_LIGHTING);
        return;
    }
    g_lit = on;
}

static void recBegin() {
    g_recMesh.clear();
    g_glowMesh.clear();
    g_recGlow = false;
    g_rec = true;
    g_M = matIdentity();
    g_mStack.clear();
    g_lit = true;
}

// Records a night light: an unlit shape that only shows after dark.
static void glowBegin() { g_recGlow = true; }
static void glowEnd() { g_recGlow = false; }

static size_t g_worldVerts = 0;
static std::vector<Chunk> g_worldChunks;
static void buildChunks(const Mesh& src, std::vector<Chunk>& out, float cell);

// Stops recording and bakes what was captured into a display list.
// ---------------------------------------------------------------- meshes on the GPU
// Recorded meshes are uploaded once into a vertex buffer. Drawing them then
// costs the CPU almost nothing, unlike display lists, which Apple's OpenGL
// re-copies every frame.

// Set for the 3D pass in the evening and at night: unlit shapes are drawn
// lit from straight above, so they darken with the rest of the city.
static bool g_unlitShade = false;

struct GpuMesh {
    GLuint vbo;
    GLsizei litN, unlitN, lineN;  // vertex counts per section
    size_t unlitOff, lineOff;     // byte offsets of the later sections
};
static std::vector<GpuMesh> g_gpu;

struct LitVert { float p[3], n[3]; unsigned char c[4]; };
struct FlatVert { float p[3]; unsigned char c[4]; };

// Uploads a mesh and returns a handle for drawMesh (0 means empty).
static GLuint uploadMesh(const Mesh& m) {
    GpuMesh g;
    g.litN = (GLsizei)(m.tv.size() / 3);
    g.unlitN = (GLsizei)(m.uv.size() / 3);
    g.lineN = (GLsizei)(m.lv.size() / 3);
    if (g.litN + g.unlitN + g.lineN == 0) return 0;
    std::vector<unsigned char> buf;
    buf.resize((size_t)g.litN * sizeof(LitVert) + (size_t)(g.unlitN + g.lineN) * sizeof(FlatVert));
    unsigned char* w = buf.data();
    for (GLsizei i = 0; i < g.litN; i++, w += sizeof(LitVert)) {
        LitVert v;
        memcpy(v.p, &m.tv[(size_t)i * 3], 12);
        memcpy(v.n, &m.tn[(size_t)i * 3], 12);
        memcpy(v.c, &m.tc[(size_t)i * 4], 4);
        memcpy(w, &v, sizeof(v));
    }
    g.unlitOff = (size_t)(w - buf.data());
    for (GLsizei i = 0; i < g.unlitN; i++, w += sizeof(FlatVert)) {
        FlatVert v;
        memcpy(v.p, &m.uv[(size_t)i * 3], 12);
        memcpy(v.c, &m.uc[(size_t)i * 4], 4);
        memcpy(w, &v, sizeof(v));
    }
    g.lineOff = (size_t)(w - buf.data());
    for (GLsizei i = 0; i < g.lineN; i++, w += sizeof(FlatVert)) {
        FlatVert v;
        memcpy(v.p, &m.lv[(size_t)i * 3], 12);
        memcpy(v.c, &m.lc[(size_t)i * 4], 4);
        memcpy(w, &v, sizeof(v));
    }
    glGenBuffers(1, &g.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)buf.size(), buf.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    g_gpu.push_back(g);
    return (GLuint)g_gpu.size();
}

static void drawMesh(GLuint handle) {
    if (handle == 0 || handle > g_gpu.size()) return;
    const GpuMesh& g = g_gpu[handle - 1];
    glBindBuffer(GL_ARRAY_BUFFER, g.vbo);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    if (g.litN) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, sizeof(LitVert), (const void*)0);
        glNormalPointer(GL_FLOAT, sizeof(LitVert), (const void*)12);
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(LitVert), (const void*)24);
        glDrawArrays(GL_TRIANGLES, 0, g.litN);
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    if (g.unlitN || g.lineN) {
        GLboolean lit = glIsEnabled(GL_LIGHTING);
        if (g_unlitShade && lit) glNormal3f(0, 1, 0);  // after dark, painted lines and signs dim too
        else glDisable(GL_LIGHTING);
        if (g.unlitN) {
            glVertexPointer(3, GL_FLOAT, sizeof(FlatVert), (const void*)g.unlitOff);
            glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(FlatVert), (const void*)(g.unlitOff + 12));
            glDrawArrays(GL_TRIANGLES, 0, g.unlitN);
        }
        if (g.lineN) {
            glVertexPointer(3, GL_FLOAT, sizeof(FlatVert), (const void*)g.lineOff);
            glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(FlatVert), (const void*)(g.lineOff + 12));
            glDrawArrays(GL_LINES, 0, g.lineN);
        }
        if (lit) glEnable(GL_LIGHTING);
    }
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static GLuint recEnd() {
    g_rec = false;
    const Mesh& m = g_recMesh;
    g_worldVerts = (m.tv.size() + m.uv.size() + m.lv.size()) / 3;
    return uploadMesh(m);
}

// Uploads the night lights recorded since recBegin (0 if there were none).
static GLuint recEndGlow() {
    GLuint h = uploadMesh(g_glowMesh);
    g_glowMesh.clear();
    return h;
}

// Stops recording and compiles what was captured as tiles (see buildChunks).
static void recEndChunks(std::vector<Chunk>& out, float cell) {
    g_rec = false;
    g_worldVerts = (g_recMesh.tv.size() + g_recMesh.uv.size() + g_recMesh.lv.size()) / 3;
    buildChunks(g_recMesh, out, cell);
}

// ---------------------------------------------------------------- chunked meshes
// The city is split into square tiles, each compiled into its own display
// list with a bounding box, so a frame only draws the tiles the camera can see.


struct ChunkBucket {
    Mesh m;
    float lo[3] = {1e9f, 1e9f, 1e9f}, hi[3] = {-1e9f, -1e9f, -1e9f};
    void grow(const float* p) {
        for (int k = 0; k < 3; k++) {
            lo[k] = std::min(lo[k], p[k]);
            hi[k] = std::max(hi[k], p[k]);
        }
    }
};

// Tile key for a primitive: its centre's tile, or the shared "huge" bucket when
// it spans more than a tile (the ground plane, the sea).
static long chunkKey(const float* v, int n, float cell) {
    float x0 = 1e9f, x1 = -1e9f, z0 = 1e9f, z1 = -1e9f;
    for (int i = 0; i < n; i++) {
        x0 = std::min(x0, v[i * 3]);
        x1 = std::max(x1, v[i * 3]);
        z0 = std::min(z0, v[i * 3 + 2]);
        z1 = std::max(z1, v[i * 3 + 2]);
    }
    if (x1 - x0 > cell * 1.5f || z1 - z0 > cell * 1.5f) return -1;
    long ix = (long)floorf((x0 + x1) * 0.5f / cell) + 1000, iz = (long)floorf((z0 + z1) * 0.5f / cell) + 1000;
    return ix * 100000L + iz;
}

static void compileMesh(const Mesh& m) {
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    if (!m.tv.empty()) {
        glEnableClientState(GL_NORMAL_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, m.tv.data());
        glNormalPointer(GL_FLOAT, 0, m.tn.data());
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, m.tc.data());
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(m.tv.size() / 3));
        glDisableClientState(GL_NORMAL_ARRAY);
    }
    if (!m.uv.empty() || !m.lv.empty()) {
        GLboolean lit = glIsEnabled(GL_LIGHTING);
        if (g_unlitShade && lit) glNormal3f(0, 1, 0);
        else glDisable(GL_LIGHTING);
        if (!m.uv.empty()) {
            glVertexPointer(3, GL_FLOAT, 0, m.uv.data());
            glColorPointer(4, GL_UNSIGNED_BYTE, 0, m.uc.data());
            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)(m.uv.size() / 3));
        }
        if (!m.lv.empty()) {
            glVertexPointer(3, GL_FLOAT, 0, m.lv.data());
            glColorPointer(4, GL_UNSIGNED_BYTE, 0, m.lc.data());
            glDrawArrays(GL_LINES, 0, (GLsizei)(m.lv.size() / 3));
        }
        if (lit) glEnable(GL_LIGHTING);
    }
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

// Splits a recorded mesh into tiles of size `cell` and compiles each one.
static void buildChunks(const Mesh& src, std::vector<Chunk>& out, float cell) {
    std::map<long, ChunkBucket> buckets;
    size_t nt = src.tv.size() / 9;
    for (size_t t = 0; t < nt; t++) {
        ChunkBucket& b = buckets[chunkKey(&src.tv[t * 9], 3, cell)];
        for (int v = 0; v < 3; v++) {
            const float* p = &src.tv[t * 9 + v * 3];
            b.grow(p);
            b.m.tv.insert(b.m.tv.end(), p, p + 3);
            b.m.tn.insert(b.m.tn.end(), &src.tn[t * 9 + v * 3], &src.tn[t * 9 + v * 3] + 3);
            b.m.tc.insert(b.m.tc.end(), &src.tc[(t * 3 + v) * 4], &src.tc[(t * 3 + v) * 4] + 4);
        }
    }
    size_t nu = src.uv.size() / 9;
    for (size_t t = 0; t < nu; t++) {
        ChunkBucket& b = buckets[chunkKey(&src.uv[t * 9], 3, cell)];
        for (int v = 0; v < 3; v++) {
            const float* p = &src.uv[t * 9 + v * 3];
            b.grow(p);
            b.m.uv.insert(b.m.uv.end(), p, p + 3);
            b.m.uc.insert(b.m.uc.end(), &src.uc[(t * 3 + v) * 4], &src.uc[(t * 3 + v) * 4] + 4);
        }
    }
    size_t nl = src.lv.size() / 6;
    for (size_t l = 0; l < nl; l++) {
        ChunkBucket& b = buckets[chunkKey(&src.lv[l * 6], 2, cell)];
        for (int v = 0; v < 2; v++) {
            const float* p = &src.lv[l * 6 + v * 3];
            b.grow(p);
            b.m.lv.insert(b.m.lv.end(), p, p + 3);
            b.m.lc.insert(b.m.lc.end(), &src.lc[(l * 2 + v) * 4], &src.lc[(l * 2 + v) * 4] + 4);
        }
    }
    for (auto& kv : buckets) {
        const ChunkBucket& b = kv.second;
        Chunk c;
        for (int k = 0; k < 3; k++) {
            c.lo[k] = b.lo[k];
            c.hi[k] = b.hi[k];
        }
        c.verts = (b.m.tv.size() + b.m.uv.size() + b.m.lv.size()) / 3;
        c.list = uploadMesh(b.m);
        out.push_back(c);
    }
}

// The six clip planes of the current projection and modelview, for culling.
struct Frustum {
    float p[6][4];
};
static Frustum currentFrustum() {
    GLfloat pm[16], mv[16], m[16];
    glGetFloatv(GL_PROJECTION_MATRIX, pm);
    glGetFloatv(GL_MODELVIEW_MATRIX, mv);
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++) {
            float s = 0;
            for (int k = 0; k < 4; k++) s += pm[k * 4 + r] * mv[c * 4 + k];
            m[c * 4 + r] = s;
        }
    Frustum f;
    for (int i = 0; i < 3; i++)
        for (int sgn = 0; sgn < 2; sgn++) {
            float* pl = f.p[i * 2 + sgn];
            float s = sgn ? -1.0f : 1.0f;
            for (int c = 0; c < 4; c++) pl[c] = m[c * 4 + 3] + s * m[c * 4 + i];
        }
    return f;
}
static bool boxVisible(const Frustum& f, const float lo[3], const float hi[3]) {
    for (int i = 0; i < 6; i++) {
        const float* pl = f.p[i];
        float x = pl[0] >= 0 ? hi[0] : lo[0], y = pl[1] >= 0 ? hi[1] : lo[1], z = pl[2] >= 0 ? hi[2] : lo[2];
        if (pl[0] * x + pl[1] * y + pl[2] * z + pl[3] < 0) return false;
    }
    return true;
}

static size_t g_drawnVerts = 0;
// Draws the tiles inside the view and nearer than maxDist to the eye.
static void drawChunks(const std::vector<Chunk>& chunks, const Frustum& f, const V3& eye, float maxDist) {
    for (size_t i = 0; i < chunks.size(); i++) {
        const Chunk& c = chunks[i];
        float dx = std::max(std::max(c.lo[0] - eye.x, 0.0f), eye.x - c.hi[0]);
        float dz = std::max(std::max(c.lo[2] - eye.z, 0.0f), eye.z - c.hi[2]);
        if (dx * dx + dz * dz > maxDist * maxDist) continue;
        if (!boxVisible(f, c.lo, c.hi)) continue;
        drawMesh(c.list);
        g_drawnVerts += c.verts;
    }
}

// ============================================================================
// Immediate-mode primitives. All of them take the current glColor.
// ============================================================================

static void box(float x0, float y0, float z0, float x1, float y1, float z1) {
    gBegin(GL_QUADS);
    gNormal(0, 0, 1);
    gVertex(x0, y0, z1); gVertex(x1, y0, z1); gVertex(x1, y1, z1); gVertex(x0, y1, z1);
    gNormal(0, 0, -1);
    gVertex(x1, y0, z0); gVertex(x0, y0, z0); gVertex(x0, y1, z0); gVertex(x1, y1, z0);
    gNormal(1, 0, 0);
    gVertex(x1, y0, z1); gVertex(x1, y0, z0); gVertex(x1, y1, z0); gVertex(x1, y1, z1);
    gNormal(-1, 0, 0);
    gVertex(x0, y0, z0); gVertex(x0, y0, z1); gVertex(x0, y1, z1); gVertex(x0, y1, z0);
    gNormal(0, 1, 0);
    gVertex(x0, y1, z1); gVertex(x1, y1, z1); gVertex(x1, y1, z0); gVertex(x0, y1, z0);
    gNormal(0, -1, 0);
    gVertex(x0, y0, z0); gVertex(x1, y0, z0); gVertex(x1, y0, z1); gVertex(x0, y0, z1);
    gEnd();
}

// Box centred on (cx, cz) with its bottom at y0.
static void boxc(float cx, float y0, float cz, float sx, float sy, float sz) {
    box(cx - sx * 0.5f, y0, cz - sz * 0.5f, cx + sx * 0.5f, y0 + sy, cz + sz * 0.5f);
}

// Flat rectangles: rectZ lies in a plane of constant z, rectX constant x, rectY constant y.
static void rectZ(float x0, float y0, float x1, float y1, float z) {
    gBegin(GL_QUADS);
    gNormal(0, 0, 1);
    gVertex(x0, y0, z); gVertex(x1, y0, z); gVertex(x1, y1, z); gVertex(x0, y1, z);
    gEnd();
}
static void rectX(float z0, float y0, float z1, float y1, float x) {
    gBegin(GL_QUADS);
    gNormal(1, 0, 0);
    gVertex(x, y0, z0); gVertex(x, y0, z1); gVertex(x, y1, z1); gVertex(x, y1, z0);
    gEnd();
}
static void rectY(float x0, float z0, float x1, float z1, float y) {
    gBegin(GL_QUADS);
    gNormal(0, 1, 0);
    gVertex(x0, y, z0); gVertex(x0, y, z1); gVertex(x1, y, z1); gVertex(x1, y, z0);
    gEnd();
}
static void quad4(const V3& a, const V3& b, const V3& c, const V3& d) {
    V3 n = norm(cross(b - a, c - a));
    gBegin(GL_QUADS);
    gNormal(n.x, n.y, n.z);
    gVertex(a.x, a.y, a.z); gVertex(b.x, b.y, b.z); gVertex(c.x, c.y, c.z); gVertex(d.x, d.y, d.z);
    gEnd();
}
static void tri3(const V3& a, const V3& b, const V3& c) {
    V3 n = norm(cross(b - a, c - a));
    gBegin(GL_TRIANGLES);
    gNormal(n.x, n.y, n.z);
    gVertex(a.x, a.y, a.z); gVertex(b.x, b.y, b.z); gVertex(c.x, c.y, c.z);
    gEnd();
}

// Cylinder or cone along +Y from y=0 to y=h.
static void cyl(float r0, float r1, float h, int n, bool caps = true) {
    float ny = (r0 - r1) / std::max(h, 0.001f);
    gBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= n; i++) {
        float a = TWO_PI * (float)i / (float)n, c = cosf(a), s = sinf(a);
        gNormal(c, ny, s);
        gVertex(c * r0, 0, s * r0);
        gVertex(c * r1, h, s * r1);
    }
    gEnd();
    if (!caps) return;
    if (r1 > 0.0001f) {
        gBegin(GL_TRIANGLE_FAN);
        gNormal(0, 1, 0);
        gVertex(0, h, 0);
        for (int i = n; i >= 0; i--) {
            float a = TWO_PI * (float)i / (float)n;
            gVertex(cosf(a) * r1, h, sinf(a) * r1);
        }
        gEnd();
    }
    if (r0 > 0.0001f) {
        gBegin(GL_TRIANGLE_FAN);
        gNormal(0, -1, 0);
        gVertex(0, 0, 0);
        for (int i = 0; i <= n; i++) {
            float a = TWO_PI * (float)i / (float)n;
            gVertex(cosf(a) * r0, 0, sinf(a) * r0);
        }
        gEnd();
    }
}

static void sphere(float r, int sl, int st) {
    for (int i = 0; i < st; i++) {
        float a0 = PI * (-0.5f + (float)i / (float)st);
        float a1 = PI * (-0.5f + (float)(i + 1) / (float)st);
        gBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= sl; j++) {
            float b = TWO_PI * (float)j / (float)sl;
            float x1 = cosf(a1) * cosf(b), y1 = sinf(a1), z1 = cosf(a1) * sinf(b);
            float x0 = cosf(a0) * cosf(b), y0 = sinf(a0), z0 = cosf(a0) * sinf(b);
            gNormal(x1, y1, z1); gVertex(x1 * r, y1 * r, z1 * r);
            gNormal(x0, y0, z0); gVertex(x0 * r, y0 * r, z0 * r);
        }
        gEnd();
    }
}

// Cylinder of radius r from point a to point b.
static void cylBetween(const V3& a, const V3& b, float r, int n = 6) {
    V3 d = b - a;
    float L = len(d);
    if (L < 1e-5f) return;
    V3 dn = d * (1.0f / L);
    V3 ax = cross(V3(0, 1, 0), dn);
    float s = len(ax);
    gPush();
    gTranslate(a.x, a.y, a.z);
    if (s > 1e-6f) gRotate(atan2f(s, dn.y) * RAD2DEG, ax.x / s, ax.y / s, ax.z / s);
    else if (dn.y < 0) gRotate(180, 1, 0, 0);
    cyl(r, r, L, n, false);
    gPop();
}

static void discY(float r, float y, int n) {
    gBegin(GL_TRIANGLE_FAN);
    gNormal(0, 1, 0);
    gVertex(0, y, 0);
    for (int i = 0; i <= n; i++) {
        float a = TWO_PI * (float)i / (float)n;
        gVertex(cosf(a) * r, y, sinf(a) * r);
    }
    gEnd();
}

// Disc in the XY plane (facing +Z), used for signs, clocks and murals.
static void discZ(float cx, float cy, float r, float z, int n) {
    gBegin(GL_TRIANGLE_FAN);
    gNormal(0, 0, 1);
    gVertex(cx, cy, z);
    for (int i = 0; i <= n; i++) {
        float a = TWO_PI * (float)i / (float)n;
        gVertex(cx + cosf(a) * r, cy + sinf(a) * r, z);
    }
    gEnd();
}

static void line3(const V3& a, const V3& b) {
    gBegin(GL_LINES);
    gVertex(a.x, a.y, a.z);
    gVertex(b.x, b.y, b.z);
    gEnd();
}

// Keeps the current colour and puts it back at the end of the scope.
struct SavedColor {
    float c[3];
    SavedColor() { c[0] = g_cur[0]; c[1] = g_cur[1]; c[2] = g_cur[2]; }
    ~SavedColor() { gColor(c[0], c[1], c[2]); }
};

// Quad in a plane of constant z, colour a at the bottom and b at the top.
static void glowQuadZ(float x0, float y0, float x1, float y1, float z, const Col& a, const Col& b) {
    gBegin(GL_QUADS);
    setc(a); gVertex(x0, y0, z); gVertex(x1, y0, z);
    setc(b); gVertex(x1, y1, z); gVertex(x0, y1, z);
    gEnd();
}

// Light spilling across the ground ahead (+z) of a lamp, fading to nothing.
static void glowSpill(float hw0, float hw1, float z0, float z1, float y, const Col& c) {
    gBegin(GL_QUADS);
    setc(c); gVertex(-hw0, y, z0); gVertex(hw0, y, z0);
    setc(0, 0, 0); gVertex(hw1, y, z1); gVertex(-hw1, y, z1);
    gEnd();
}

// ============================================================================
// Text. A small built-in vector font: capitals, digits and some punctuation.
// Glyphs sit on a 4 x 6 grid and every group of four digits is one stroke
// (x0 y0 x1 y1). Strokes are drawn as quads, so they have real thickness and
// batch with the rest of the city.
// ============================================================================

static const char* glyphFor(char ch) {
    switch (toupper((unsigned char)ch)) {
        case 'A': return "0004 0426 2644 4440 0343";
        case 'B': return "0006 0636 3645 4544 4433 0333 3342 4241 4130 3000";
        case 'C': return "4536 3616 1605 0501 0110 1030 3041";
        case 'D': return "0006 0636 3645 4541 4130 3000";
        case 'E': return "4606 0600 0040 0333";
        case 'F': return "4606 0600 0333";
        case 'G': return "4536 3616 1605 0501 0110 1030 3041 4143 4323";
        case 'H': return "0006 4046 0343";
        case 'I': return "1636 2620 1030";
        case 'J': return "4641 4130 3010 1001";
        case 'K': return "0006 4602 1340";
        case 'L': return "0600 0040";
        case 'M': return "0006 0623 2346 4640";
        case 'N': return "0006 0640 4046";
        case 'O': return "1030 3041 4145 4536 3616 1605 0501 0110";
        case 'P': return "0006 0636 3645 4544 4433 3303";
        case 'Q': return "1030 3041 4145 4536 3616 1605 0501 0110 2240";
        case 'R': return "0006 0636 3645 4544 4433 3303 2340";
        case 'S': return "4536 3616 1605 0504 0413 1333 3342 4241 4130 3010 1001";
        case 'T': return "0646 2620";
        case 'U': return "0601 0110 1030 3041 4146";
        case 'V': return "0620 2046";
        case 'W': return "0610 1023 2330 3046";
        case 'X': return "0640 0046";
        case 'Y': return "0623 4623 2320";
        case 'Z': return "0646 4600 0040";
        case '0': return "1030 3041 4145 4536 3616 1605 0501 0110 3511";
        case '1': return "1526 2620 1030";
        case '2': return "0516 1636 3645 4544 4400 0040";
        case '3': return "0516 1636 3645 4544 4433 3313 3342 4241 4130 3010 1001";
        case '4': return "3036 3602 0242";
        case '5': return "4606 0603 0333 3342 4241 4130 3000";
        case '6': return "4536 3616 1605 0501 0110 1030 3041 4142 4233 3303";
        case '7': return "0646 4610";
        case '8': return "1333 1304 0405 0516 1636 3645 4544 4433 3342 4241 4130 3010 1001 0102 0213";
        case '9': return "4313 1304 0405 0516 1636 3645 4541 4130 3010 1001";
        case '-': return "1333";
        case '/': return "0046";
        case '.': return "2021";
        case ',': return "2110";
        case '!': return "2622 2120";
        case '\'': return "2625";
        case ':': return "2425 2122";
        case '+': return "2125 0343";
        case '(': return "3625 2521 2130";
        case ')': return "1625 2521 2110";
        case '?': return "0516 1636 3645 4544 4423 2322 2120";
        case '=': return "0242 0444";
        default: return "";
    }
}
static const float GLYPH_ADV = 5.5f;

static float textWidth(const char* s, float h) {
    int n = (int)strlen(s);
    return n == 0 ? 0.0f : ((float)n * GLYPH_ADV - 1.5f) * h / 6.0f;
}

// Emits quads for every stroke of s inside an open GL_QUADS batch.
static void emitText(const char* s, float x, float y, float z, float h, float w) {
    float sc = h / 6.0f, hw = w * 0.5f;
    for (; *s; s++, x += GLYPH_ADV * sc) {
        const char* g = glyphFor(*s);
        while (g[0] && g[1] && g[2] && g[3]) {
            float x0 = x + (float)(g[0] - '0') * sc, y0 = y + (float)(g[1] - '0') * sc;
            float x1 = x + (float)(g[2] - '0') * sc, y1 = y + (float)(g[3] - '0') * sc;
            float dx = x1 - x0, dy = y1 - y0, l = sqrtf(dx * dx + dy * dy);
            float ux = l > 1e-6f ? dx / l : 0.0f, uy = l > 1e-6f ? dy / l : 1.0f;
            x0 -= ux * hw; y0 -= uy * hw; x1 += ux * hw; y1 += uy * hw;
            float nx = -uy * hw, ny = ux * hw;
            gVertex(x0 + nx, y0 + ny, z);
            gVertex(x0 - nx, y0 - ny, z);
            gVertex(x1 - nx, y1 - ny, z);
            gVertex(x1 + nx, y1 + ny, z);
            g += 4;
            while (*g == ' ') g++;
        }
    }
}

// Text in the XY plane facing +Z, centred on x=0, baseline at y=0, cap height h.
// bold scales the stroke thickness.
static void text3D(const char* s, float h, const Col& c, float bold = 1.0f) {
    gLighting(false);
    gColor(c.r, c.g, c.b);
    gBegin(GL_QUADS);
    gNormal(0, 0, 1);
    emitText(s, -textWidth(s, h) * 0.5f, 0, 0, h, h * 0.14f * bold);
    gEnd();
    gLighting(true);
}

// Like text3D but shrinks the text so it never exceeds maxW.
static void text3DFit(const char* s, float h, float maxW, const Col& c) {
    float w = textWidth(s, h);
    if (w > maxW) h *= maxW / w;
    text3D(s, h, c);
}

// Screen-space helpers, used while an orthographic projection in pixels is active.
static void rect2D(float x0, float y0, float x1, float y1, float r, float g, float b, float a) {
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x0, y0); glVertex2f(x1, y0); glVertex2f(x1, y1); glVertex2f(x0, y1);
    glEnd();
}

static void bmpText(float x, float y, const char* s, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (; *s; s++) glutBitmapCharacter(font, *s);
}
static int bmpWidth(const char* s, void* font = GLUT_BITMAP_HELVETICA_18) {
    int w = 0;
    for (; *s; s++) w += glutBitmapWidth(font, *s);
    return w;
}
// Bitmap text. The large size gets a drop shadow; small text always sits on a
// dark panel, and skipping its shadow halves the number of bitmap draws.
static void bmpTextShadow(float x, float y, const char* s, const Col& c, void* font = GLUT_BITMAP_HELVETICA_18) {
    if (font == GLUT_BITMAP_HELVETICA_18) {
        glColor4f(0, 0, 0, 0.8f);
        bmpText(x + 1, y - 1, s, font);
    }
    glColor4f(c.r, c.g, c.b, 1);
    bmpText(x, y, s, font);
}

// Large outlined HUD text. align: 0 = left, 0.5 = centred, 1 = right.
static void strokeText2D(const char* s, float x, float y, float h, float align, const Col& c, float alpha,
                         bool outline = true) {
    float x0 = x - textWidth(s, h) * align;
    float t = std::max(1.6f, h * 0.13f);
    if (outline) {
        glColor4f(0, 0, 0, alpha * 0.8f);
        glBegin(GL_QUADS);
        emitText(s, x0 + 1.5f, y - 1.5f, 0, h, t * 2.3f);
        glEnd();
    }
    glColor4f(c.r, c.g, c.b, alpha);
    glBegin(GL_QUADS);
    emitText(s, x0, y, 0, h, t);
    glEnd();
}

// strokeText2D, shrunk so it never runs wider than maxW pixels.
static void strokeFit2D(const char* s, float x, float y, float h, float maxW, float align, const Col& c, float alpha,
                        bool outline = true) {
    float w = textWidth(s, h);
    if (w > maxW) h *= maxW / w;
    strokeText2D(s, x, y, h, align, c, alpha, outline);
}

static std::string withCommas(long v) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%ld", v < 0 ? -v : v);
    std::string s = buf, out;
    int n = (int)s.size();
    for (int i = 0; i < n; i++) {
        out += s[i];
        int left = n - 1 - i;
        if (left > 0 && left % 3 == 0) out += ',';
    }
    return v < 0 ? "-" + out : out;
}

