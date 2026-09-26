#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Effects: particles, camera shake and the monsoon
//
// Particles use their own random stream so that spawning sparks never changes
// traffic or anything else the self test depends on.
// ============================================================================

struct Particle {
    V3 p, v;
    float life, maxLife, size, grav, drag;
    Col c;
    bool additive;
};
static std::vector<Particle> g_parts;
static float g_shake = 0;  // camera shake strength, decays over time
static float g_rain = 0;   // 0 dry, 1 full monsoon

static unsigned int g_fxSeed = 777u;
static float fxrand() {
    g_fxSeed = g_fxSeed * 1664525u + 1013904223u;
    return (float)((g_fxSeed >> 8) & 0xFFFFFF) / 16777216.0f;
}
static float fxrange(float a, float b) { return a + (b - a) * fxrand(); }

static void emit(const V3& p, const V3& v, const Col& c, float life, float size, float grav, bool additive = false,
                 float drag = 0.8f) {
    if (g_parts.size() > 3000) return;
    Particle q;
    q.p = p;
    q.v = v;
    q.c = c;
    q.life = q.maxLife = life;
    q.size = size;
    q.grav = grav;
    q.drag = drag;
    q.additive = additive;
    g_parts.push_back(q);
}

static void dustBurst(const V3& p, int n, float spd) {
    for (int i = 0; i < n; i++) {
        float a = fxrange(0, TWO_PI), s = spd * fxrange(0.4f, 1.0f);
        Col c = g_rain > 0.5f ? C(0.7f, 0.78f, 0.9f) : C(0.78f, 0.7f, 0.58f);
        emit(p + V3(0, 0.05f, 0), V3(cosf(a) * s, fxrange(0.3f, 1.2f), sinf(a) * s), c, fxrange(0.35f, 0.7f),
             fxrange(0.08f, 0.16f), 3.0f, false, 2.5f);
    }
}

static void sparks(const V3& p, const V3& vel) {
    for (int i = 0; i < 2; i++) {
        V3 v = vel * -0.25f + V3(fxrange(-1.5f, 1.5f), fxrange(0.5f, 2.5f), fxrange(-1.5f, 1.5f));
        emit(p, v, mixc(C(1.0f, 0.85f, 0.3f), C(1.0f, 0.45f, 0.1f), fxrand()), fxrange(0.2f, 0.45f), 0.05f, 9.0f,
             true, 0.5f);
    }
}

static void confetti(const V3& p) {
    for (int i = 0; i < 320; i++) {
        V3 at = p + V3(fxrange(-5, 5), fxrange(2.5f, 6), fxrange(-5, 5));
        Col c = (i & 1) ? SAREES[i % 8] : SHIRTS[i % 10];
        emit(at, V3(fxrange(-1, 1), fxrange(-0.5f, 1.5f), fxrange(-1, 1)), c, fxrange(2.5f, 4.0f), 0.06f, 1.6f, false,
             1.2f);
    }
}

static void sparkle(const V3& p, const Col& c) {
    emit(p + V3(fxrange(-0.4f, 0.4f), fxrange(0.0f, 1.6f), fxrange(-0.4f, 0.4f)),
         V3(0, fxrange(0.3f, 0.9f), 0), c, fxrange(0.4f, 0.8f), 0.06f, -0.5f, true, 1.0f);
}

static void updateParticles(float dt) {
    for (size_t i = 0; i < g_parts.size();) {
        Particle& q = g_parts[i];
        q.life -= dt;
        if (q.life <= 0) {
            q = g_parts.back();
            g_parts.pop_back();
            continue;
        }
        q.v.y -= q.grav * dt;
        q.v *= expf(-q.drag * dt);
        q.p += q.v * dt;
        i++;
    }
    g_shake = std::max(0.0f, g_shake - dt * 1.6f);
}

// Particles are camera-facing squares; camera axes come from the caller.
static void drawParticles(const V3& camRight, const V3& camUp, const V3& camPos) {
    if (g_parts.empty()) return;
    gLighting(false);
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    for (int pass = 0; pass < 2; pass++) {
        glBlendFunc(GL_SRC_ALPHA, pass == 0 ? GL_ONE_MINUS_SRC_ALPHA : GL_ONE);
        gBegin(GL_QUADS);
        for (size_t i = 0; i < g_parts.size(); i++) {
            const Particle& q = g_parts[i];
            if (q.additive != (pass == 1)) continue;
            V3 dc = q.p - camPos;
            if (dot(dc, dc) < 2.5f * 2.5f) continue;  // too close to the lens
            float a = clampf(q.life / q.maxLife * 1.5f, 0, 1);
            glColor4f(q.c.r, q.c.g, q.c.b, a);
            V3 r = camRight * q.size, u = camUp * q.size;
            V3 v0 = q.p - r - u, v1 = q.p + r - u, v2 = q.p + r + u, v3 = q.p - r + u;
            gVertex(v0.x, v0.y, v0.z);
            gVertex(v1.x, v1.y, v1.z);
            gVertex(v2.x, v2.y, v2.z);
            gVertex(v3.x, v3.y, v3.z);
        }
        gEnd();
    }
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    gLighting(true);
}

// Rain streaks in a box around the camera. Positions come from a hash of the
// streak index and time, so nothing needs storing.
static void drawRain(const V3& cam, float t) {
    if (g_rain < 0.02f) return;
    gLighting(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    int n = (int)(1400.0f * g_rain);
    gBegin(GL_LINES);
    for (int i = 0; i < n; i++) {
        unsigned int h = (unsigned int)i * 2654435761u;
        float fx = (float)(h & 1023) / 1023.0f, fz = (float)((h >> 10) & 1023) / 1023.0f;
        float fy = (float)((h >> 20) & 1023) / 1023.0f;
        float x = cam.x - 25.0f + fx * 50.0f, z = cam.z - 25.0f + fz * 50.0f;
        float y = cam.y + 14.0f - fmodf(fy * 28.0f + t * 22.0f, 28.0f);
        glColor4f(0.75f, 0.8f, 0.9f, 0.0f);
        gVertex(x + 0.08f, y + 0.9f, z);
        glColor4f(0.75f, 0.8f, 0.9f, 0.45f * g_rain);
        gVertex(x, y, z);
    }
    gEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    gLighting(true);
}
