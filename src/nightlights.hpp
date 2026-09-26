#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Night lights and fireworks
//
// Lit windows, shopfronts, signs, lamps and vehicle lights are recorded into
// separate glow meshes while the city is built. After dark they are added on
// top of the dimmed city, scaled by how dark it is, so they fade in at dusk.
// The finale adds strings of festival bulbs, lanterns and fireworks over the
// Sea Face.
// ============================================================================

static std::vector<Chunk> g_glowChunks;   // windows, shops, signs, lamp pools
static std::vector<Chunk> g_festChunks;   // festival lights for the Sea Face show

// A soft pool of lamp light on the ground, following its bumps.
static void groundPool(float cx, float cz, float r, const Col& col) {
    const int SEG = 18;
    const float ring[3] = {0.0f, 0.45f, 1.0f}, k[3] = {1.0f, 0.45f, 0.0f};
    gBegin(GL_QUADS);
    for (int j = 0; j < 2; j++)
        for (int i = 0; i < SEG; i++) {
            float a0 = TWO_PI * (float)i / (float)SEG, a1 = TWO_PI * (float)(i + 1) / (float)SEG;
            float rr[4] = {ring[j], ring[j], ring[j + 1], ring[j + 1]};
            float aa[4] = {a0, a1, a1, a0};
            float kk[4] = {k[j], k[j], k[j + 1], k[j + 1]};
            for (int v = 0; v < 4; v++) {
                float x = cx + cosf(aa[v]) * rr[v] * r, z = cz + sinf(aa[v]) * rr[v] * r;
                setc(mulc(col, kk[v]));
                gVertex(x, gridGround(x, z) + 0.035f, z);
            }
        }
    gEnd();
}

// Glowing disc facing the camera from one side (plane of constant z or x).
static void glowDisc(const V3& c, float r, const Col& col, bool alongX) {
    gBegin(GL_TRIANGLE_FAN);
    setc(col);
    gVertex(c.x, c.y, c.z);
    setc(mulc(col, 0.25f));
    for (int i = 0; i <= 10; i++) {
        float a = TWO_PI * (float)i / 10.0f;
        float u = cosf(a) * r, v = sinf(a) * r;
        if (alongX) gVertex(c.x + u, c.y + v, c.z);
        else gVertex(c.x, c.y + v, c.z + u);
    }
    gEnd();
}

static const Col BULBS[6] = {{1.0f, 0.25f, 0.2f}, {1.0f, 0.85f, 0.2f}, {0.3f, 1.0f, 0.4f},
                             {0.35f, 0.55f, 1.0f}, {1.0f, 0.35f, 0.85f}, {1.0f, 0.55f, 0.15f}};

// A sagging string of coloured bulbs from a to b.
static void bulbString(const V3& a, const V3& b, float sag, float spacing) {
    V3 d = b - a;
    float L = len(V3(d.x, 0, d.z));
    int n = std::max(2, (int)(L / spacing));
    bool alongX = fabsf(d.x) > fabsf(d.z);
    for (int i = 0; i <= n; i++) {
        float t = (float)i / (float)n;
        V3 p = a + d * t;
        p.y -= sag * 4.0f * t * (1.0f - t);
        const Col& c = BULBS[i % 6];
        gBegin(GL_QUADS);
        setc(c);
        float s = 0.09f;
        if (alongX) { gVertex(p.x - s, p.y - s, p.z); gVertex(p.x + s, p.y - s, p.z); gVertex(p.x + s, p.y + s, p.z); gVertex(p.x - s, p.y + s, p.z); }
        else { gVertex(p.x, p.y - s, p.z - s); gVertex(p.x, p.y - s, p.z + s); gVertex(p.x, p.y + s, p.z + s); gVertex(p.x, p.y + s, p.z - s); }
        gEnd();
        glowDisc(p, 0.28f, mulc(c, 0.35f), alongX);
    }
}

// Akash kandil: a paper star lantern, seen from any side.
static void lantern(const V3& p, const Col& c) {
    for (int side = 0; side < 2; side++) {
        gBegin(GL_TRIANGLE_FAN);
        setc(c);
        gVertex(p.x, p.y, p.z);
        setc(mulc(c, 0.55f));
        for (int i = 0; i <= 10; i++) {
            float a = TWO_PI * (float)i / 10.0f;
            float r = (i & 1) ? 0.22f : 0.48f;
            float u = cosf(a) * r, v = sinf(a) * r;
            if (side == 0) gVertex(p.x + u, p.y + v, p.z);
            else gVertex(p.x, p.y + v, p.z + u);
        }
        gEnd();
    }
    // paper tails
    setc(mulc(c, 0.6f));
    gBegin(GL_QUADS);
    gVertex(p.x - 0.08f, p.y - 0.45f, p.z); gVertex(p.x + 0.08f, p.y - 0.45f, p.z);
    gVertex(p.x + 0.05f, p.y - 1.2f, p.z); gVertex(p.x - 0.05f, p.y - 1.2f, p.z);
    gEnd();
}

// Records every baked night light. Runs once, after the world and its ground
// grid are built, while recording is still on.
static void buildNightGlow() {
    glowBegin();
    // pools of sodium light under every street lamp, and their streaks on the sea
    for (size_t i = 0; i < g_poleTops.size(); i++) {
        const V3& t = g_poleTops[i].tip;
        groundPool(t.x, t.z, 7.5f, C(0.45f, 0.32f, 0.15f));
        if (t.z > 150.0f) {
            gBegin(GL_QUADS);
            setc(0.32f, 0.22f, 0.1f);
            gVertex(t.x - 0.4f, -1.17f, SEAWALL_Z + 1.4f); gVertex(t.x + 0.4f, -1.17f, SEAWALL_Z + 1.4f);
            setc(0, 0, 0);
            gVertex(t.x + 1.6f, -1.17f, SEAWALL_Z + 22.0f); gVertex(t.x - 1.6f, -1.17f, SEAWALL_Z + 22.0f);
            gEnd();
        }
    }
    // the bulb over each food stall
    for (size_t i = 0; i < g_stalls.size(); i++) groundPool(g_stalls[i].stand.x, g_stalls[i].stand.z, 4.0f, C(0.42f, 0.3f, 0.15f));
    // the Sea Link: deck lights and lit cables
    const float lz = 255.8f, deckY = 18.0f;
    for (float x = -400.0f; x <= 400.0f; x += 9.0f) {
        setc(1.0f, 0.78f, 0.45f);
        rectZ(x - 0.5f, deckY - 0.9f, x + 0.5f, deckY - 0.4f, lz);
    }
    for (int t = -1; t <= 1; t += 2) {
        float tx = (float)t * 70.0f;
        glowDisc(V3(tx, 126.0f, lz + 4.0f), 1.6f, C(1.0f, 0.15f, 0.1f), true);
        for (int k = 1; k <= 12; k++) {
            float y = 100.0f + (float)k * 1.8f;
            for (int s = -1; s <= 1; s += 2) {
                float ex = tx + (float)s * (8.0f + (float)k * 6.5f);
                gBegin(GL_QUADS);
                setc(0.13f, 0.14f, 0.18f);
                gVertex(tx, y - 0.2f, 261.5f); gVertex(tx, y + 0.2f, 261.5f);
                setc(0.06f, 0.07f, 0.09f);
                gVertex(ex, deckY + 0.2f, 261.5f); gVertex(ex, deckY - 0.2f, 261.5f);
                gEnd();
            }
        }
    }
    glowEnd();
    buildChunks(g_glowMesh, g_glowChunks, 32.0f);
    g_glowMesh.clear();

    // festival lights, only switched on for the show
    glowBegin();
    for (float x = -134.0f; x < 134.0f; x += 14.0f)
        bulbString(V3(x, SIDEWALK_H + 7.7f, 158.8f), V3(x + 14.0f, SIDEWALK_H + 7.7f, 158.8f), 1.3f, 0.7f);
    for (float z = 64.0f; z <= 112.0f; z += 16.0f) {
        bulbString(V3(7.45f, SIDEWALK_H + 7.6f, z), V3(-7.45f, SIDEWALK_H + 7.6f, z + 8.0f), 1.2f, 0.6f);
        if (z + 16.0f <= 112.0f) bulbString(V3(-7.45f, SIDEWALK_H + 7.6f, z + 8.0f), V3(7.45f, SIDEWALK_H + 7.6f, z + 16.0f), 1.2f, 0.6f);
    }
    int li = 0;
    for (size_t i = 0; i < g_poleTops.size(); i++) {
        const PolePt& p = g_poleTops[i];
        if (p.top.z < 60.0f) continue;
        V3 at = (p.top + p.tip) * 0.5f;
        lantern(V3(at.x, p.top.y - 0.4f, at.z), BULBS[(li++ * 5) % 6]);
    }
    // a big lantern arch over the viewing deck
    for (int k = 0; k <= 8; k++) {
        float t = (float)k / 8.0f;
        lantern(V3(42.0f + 26.0f * t, 6.5f + 2.2f * sinf(t * PI), 156.2f), BULBS[k % 6]);
    }
    glowEnd();
    buildChunks(g_glowMesh, g_festChunks, 32.0f);
    g_glowMesh.clear();
}

// Camera-facing halo, drawn in world space.
static void halo(const V3& p, float r, const Col& c, const V3& right, const V3& up) {
    glBegin(GL_TRIANGLE_FAN);
    glColor3f(c.r, c.g, c.b);
    glVertex3f(p.x, p.y, p.z);
    glColor3f(0, 0, 0);
    for (int i = 0; i <= 12; i++) {
        float a = TWO_PI * (float)i / 12.0f;
        V3 q = p + right * (cosf(a) * r) + up * (sinf(a) * r);
        glVertex3f(q.x, q.y, q.z);
    }
    glEnd();
}

static void drawNightLights() {
    float k = g_sky.lights;
    if (k < 0.02f) return;
    gLighting(false);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendColor(0, 0, 0, k);
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-0.3f, -2.0f);  // small slope term, or glows show through signs seen edge on
    GLfloat black[4] = {0, 0, 0, 1};
    glFogfv(GL_FOG_COLOR, black);  // lights fade into the dark with distance
    float dist = G().drawDist;
    drawChunks(g_glowChunks, g_frustum, g_camPos, dist);
    if (seaFaceShow()) drawChunks(g_festChunks, g_frustum, g_camPos, dist);
    for (size_t i = 0; i < g_vehicles.size(); i++) {
        const Vehicle& v = g_vehicles[i];
        if (!v.glow || !vehicleSeen(v)) continue;
        float cx, cz, yaw;
        vehCenter(v, cx, cz, yaw);
        glPushMatrix();
        glTranslatef(cx, 0, cz);
        glRotatef(yaw * RAD2DEG, 0, 1, 0);
        drawMesh(v.glow);
        glPopMatrix();
    }
    for (size_t i = 0; i < g_trains.size(); i++) {
        const Train& t = g_trains[i];
        for (int c = 0; c < t.cars && c < (int)t.carGlow.size(); c++) {
            float front = t.x - (float)t.dir * (float)c * (TRAIN_CAR_L + TRAIN_GAP);
            float xa = t.dir > 0 ? front - TRAIN_CAR_L : front;
            if (!seen(xa, DECK_TOP, t.z - 2.0f, xa + TRAIN_CAR_L, DECK_TOP + 4.5f, t.z + 2.0f)) continue;
            glPushMatrix();
            glTranslatef(xa, 0, t.z);
            drawMesh(t.carGlow[(size_t)c]);
            glPopMatrix();
        }
    }
    // halos round the lamp heads
    glDisable(GL_POLYGON_OFFSET_FILL);
    V3 fwd = norm(g_camLook - g_camPos);
    V3 right = norm(cross(fwd, V3(0, 1, 0)));
    V3 up = cross(right, fwd);
    for (size_t i = 0; i < g_poleTops.size(); i++) {
        const V3& t = g_poleTops[i].tip;
        V3 d = t - g_camPos;
        if (dot(d, d) > 140.0f * 140.0f || dot(d, fwd) < 0) continue;
        halo(t, 1.5f, C(0.55f, 0.42f, 0.25f), right, up);
    }
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    GLfloat fogc[4] = {g_sky.fog.r, g_sky.fog.g, g_sky.fog.b, 1};
    glFogfv(GL_FOG_COLOR, fogc);
    gLighting(true);
}

// ---------------------------------------------------------------- fireworks

struct Rocket {
    V3 p, v;
    float fuse;
    int col;
};
static std::vector<Rocket> g_rockets;
static float g_fwTimer = 1.0f;

static void burst(const V3& p, int col) {
    const Col& ca = BULBS[col % 6];
    const Col& cb = BULBS[(col + 2) % 6];
    int n = 110;
    for (int i = 0; i < n; i++) {
        // even spread over a sphere
        float y = 1.0f - 2.0f * ((float)i + 0.5f) / (float)n, r = sqrtf(std::max(0.0f, 1.0f - y * y));
        float a = (float)i * 2.39996f;
        V3 d(cosf(a) * r, y, sinf(a) * r);
        emit(p, d * fxrange(8.0f, 10.5f), (i & 1) ? ca : mixc(cb, C(1, 1, 1), 0.3f), fxrange(1.5f, 2.3f), 0.3f, 2.2f, true, 1.1f);
    }
    for (int i = 0; i < 6; i++) emit(p, V3(0, 0, 0), C(1.0f, 0.95f, 0.85f), 0.25f, 2.2f, 0.0f, true, 0.0f);
}

// Launches rockets over the sea every `every` seconds while `on`.
static void fireworksUpdate(float dt, bool on, float every) {
    if (on) {
        g_fwTimer -= dt;
        if (g_fwTimer <= 0) {
            g_fwTimer = every * fxrange(0.6f, 1.4f);
            Rocket r;
            r.p = V3(fxrange(0.0f, 115.0f), -1.0f, fxrange(185.0f, 225.0f));
            r.v = V3(fxrange(-2.0f, 2.0f), fxrange(19.0f, 23.0f), fxrange(-2.0f, 0.5f));
            r.fuse = fxrange(1.1f, 1.4f);
            r.col = (int)(fxrand() * 5.99f);
            g_rockets.push_back(r);
        }
    }
    for (size_t i = 0; i < g_rockets.size();) {
        Rocket& r = g_rockets[i];
        r.v.y -= 6.0f * dt;
        r.p += r.v * dt;
        r.fuse -= dt;
        if (fxrand() < 0.6f) emit(r.p, V3(fxrange(-0.3f, 0.3f), -1.0f, fxrange(-0.3f, 0.3f)), C(1.0f, 0.7f, 0.35f), 0.5f, 0.16f, 1.0f, true, 1.5f);
        if (r.fuse <= 0) {
            burst(r.p, r.col);
            g_rockets[i] = g_rockets.back();
            g_rockets.pop_back();
            continue;
        }
        i++;
    }
}
