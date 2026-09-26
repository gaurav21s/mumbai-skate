#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Sun shadows
//
// The city's shadows are baked once: every lit triangle that faces the sun is
// flattened along the sun direction onto the ground. Moving things (the
// skater, traffic, people, trains) are flattened each frame with a projection
// matrix. Both only mark the stencil buffer; one dark full-screen quad then
// shades the marked pixels, so overlapping shadows never double up.
//
// The same pass darkens the bottom of walls a little, a cheap stand-in for
// light that the ground and nearby walls block.
// ============================================================================

static const V3 SUN_DIR(-0.45f, 0.62f, 0.64f);  // toward the sun
static GLuint g_worldShadowList = 0;
static size_t g_shadowVerts = 0;

// Ground height on a 1 m grid, sampled from the collision world.
static const float SG_X0 = -150.0f, SG_Z0 = -100.0f;
static const int SG_W = 300, SG_H = 290;
static std::vector<float> g_groundGrid;

static void buildGroundGrid() {
    g_groundGrid.assign((size_t)SG_W * SG_H, 0.0f);
    for (int j = 0; j < SG_H; j++)
        for (int i = 0; i < SG_W; i++) {
            float x = SG_X0 + (float)i + 0.5f, z = SG_Z0 + (float)j + 0.5f;
            g_groundGrid[(size_t)j * SG_W + i] = groundAt(x, z, 0.3f).h;
        }
}
static float gridGround(float x, float z) {
    int i = (int)floorf(x - SG_X0), j = (int)floorf(z - SG_Z0);
    if (i < 0 || j < 0 || i >= SG_W || j >= SG_H) return 0.0f;
    return g_groundGrid[(size_t)j * SG_W + i];
}

// Point p flattened along the sun onto the ground under where its shadow lands.
static V3 shadowPoint(const V3& p) {
    const V3& L = SUN_DIR;
    V3 q = p - L * (p.y / L.y);
    float g = gridGround(q.x, q.z);
    q = p - L * ((p.y - g) / L.y);
    q.y = g + 0.03f;
    return q;
}

// Runs on the recorded world mesh before it is compiled: darkens wall bases
// and collects the shadow triangles.
static void bakeWorldShadows(Mesh& m) {
    buildGroundGrid();
    std::vector<float> sv;
    size_t n = m.tv.size() / 3;
    for (size_t v = 0; v < n; v++) {
        float y = m.tv[v * 3 + 1], ny = m.tn[v * 3 + 1];
        if (fabsf(ny) > 0.5f) continue;
        float above = y - gridGround(m.tv[v * 3], m.tv[v * 3 + 2]);
        float k = 0.7f + 0.3f * clampf(above / 1.6f, 0, 1);
        for (int c = 0; c < 3; c++) m.tc[v * 4 + c] = (unsigned char)((float)m.tc[v * 4 + c] * k);
    }
    for (size_t t = 0; t + 2 < n; t += 3) {
        V3 a(m.tv[t * 3], m.tv[t * 3 + 1], m.tv[t * 3 + 2]);
        V3 b(m.tv[t * 3 + 3], m.tv[t * 3 + 4], m.tv[t * 3 + 5]);
        V3 c(m.tv[t * 3 + 6], m.tv[t * 3 + 7], m.tv[t * 3 + 8]);
        V3 nrm = cross(b - a, c - a);
        float area2 = len(nrm);
        if (area2 < 0.006f) continue;
        if (dot(nrm, SUN_DIR) <= 0.0f) continue;  // back faces add nothing to the outline
        float top = std::max(a.y, std::max(b.y, c.y));
        if (top - gridGround(a.x, a.z) < 0.35f) continue;
        if (fabsf(a.x) > 320.0f || fabsf(a.z) > 320.0f) continue;
        V3 pa = shadowPoint(a), pb = shadowPoint(b), pc = shadowPoint(c);
        const V3* ps[3] = {&pa, &pb, &pc};
        for (int k = 0; k < 3; k++) {
            sv.push_back(ps[k]->x);
            sv.push_back(ps[k]->y);
            sv.push_back(ps[k]->z);
        }
    }
    g_shadowVerts = sv.size() / 3;
    g_worldShadowList = glGenLists(1);
    glEnableClientState(GL_VERTEX_ARRAY);
    glNewList(g_worldShadowList, GL_COMPILE);
    if (!sv.empty()) {
        glVertexPointer(3, GL_FLOAT, 0, sv.data());
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)g_shadowVerts);
    }
    glEndList();
    glDisableClientState(GL_VERTEX_ARRAY);
}

// Pushes a matrix that squashes whatever is drawn next onto the plane y = h.
static void pushShadowMatrix(float h) {
    const V3& L = SUN_DIR;
    float kx = L.x / L.y, kz = L.z / L.y;
    GLfloat m[16] = {1, 0, 0, 0, -kx, 0, -kz, 0, 0, 0, 1, 0, kx * h, h + 0.03f, kz * h, 1};
    glPushMatrix();
    glMultMatrixf(m);
}
