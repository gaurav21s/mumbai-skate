#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Collision world
//
// The level is made of axis-aligned solids. A solid is either a flat box or a
// ramp (a wedge whose top rises along one axis). Rails are line segments the
// player can grind; ledge tops register rails along their edges.
// ============================================================================

struct Solid {
    float x0, z0, x1, z1;  // footprint
    float y0, y1;          // bottom and (highest) top
    float yLow;            // ramp only: top height at the low end
    int axis;              // 0 flat; ramps: 1 rises toward +x, -1 toward -x, 2 toward +z, -2 toward -z

    float topAt(float x, float z) const {
        if (axis == 0) return y1;
        float t;
        switch (axis) {
            case 1: t = (x - x0) / (x1 - x0); break;
            case -1: t = (x1 - x) / (x1 - x0); break;
            case 2: t = (z - z0) / (z1 - z0); break;
            default: t = (z1 - z) / (z1 - z0); break;
        }
        return yLow + (y1 - yLow) * clampf(t, 0, 1);
    }
    void grad(float& gx, float& gz) const {
        gx = gz = 0;
        float dh = y1 - yLow;
        switch (axis) {
            case 1: gx = dh / (x1 - x0); break;
            case -1: gx = -dh / (x1 - x0); break;
            case 2: gz = dh / (z1 - z0); break;
            case -2: gz = -dh / (z1 - z0); break;
            default: break;
        }
    }
    bool inside(float x, float z, float m) const {
        return x > x0 - m && x < x1 + m && z > z0 - m && z < z1 + m;
    }
};

// Named rails that chapter tasks refer to.
enum RailTag { RT_NONE, RT_PLAZA_BAR, RT_FUNBOX, RT_STAIR, RT_PIPE, RT_SKYWALK, RT_MEDIAN, RT_SEAWALL, RT_SEAFACE };

struct Rail {
    V3 a, b;
    bool ledge;     // ledge edge rather than a round rail
    int tag;        // RailTag
    int needLevel;  // chapters that must be finished before it can be grinded
};

// Set while building to stamp every rail created in that stretch.
static int g_railTag = RT_NONE;
static int g_railNeed = 0;

// Barricades that close off an area until enough chapters are finished.
struct Gate {
    float x0, z0, x1, z1, h;
    int needLevel;
    bool sign;  // front barricade with a sign, rather than a plain side fence
    GLuint list;
};
static std::vector<Gate> g_gates;
static int g_unlockLevel = 0;  // number of chapters finished
static bool gateOpen(const Gate& g) { return g_unlockLevel >= g.needLevel; }
static void addGate(float x0, float z0, float x1, float z1, float h, int need, bool sign) {
    Gate g = {x0, z0, x1, z1, h, need, sign, 0};
    g_gates.push_back(g);
}

static std::vector<Solid> g_solids;
static std::vector<Rail> g_rails;

// Local placement frames. World pieces are authored in a local frame (front
// facing +Z) and dropped into the city with a translation and a quarter-turn
// rotation. The same frame feeds both the GL matrix stack and collision.
struct Xf { float ox, oz; int rot; };
static Xf g_xf = {0, 0, 0};
static std::vector<Xf> g_xfStack;

static void xfVec(float lx, float lz, float& wx, float& wz) {
    switch (g_xf.rot & 3) {
        case 0: wx = lx; wz = lz; break;
        case 1: wx = lz; wz = -lx; break;
        case 2: wx = -lx; wz = -lz; break;
        default: wx = -lz; wz = lx; break;
    }
}
static void xfPt(float lx, float lz, float& wx, float& wz) {
    xfVec(lx, lz, wx, wz);
    wx += g_xf.ox;
    wz += g_xf.oz;
}
static void pushXf(float ox, float oz, int rot) {
    g_xfStack.push_back(g_xf);
    float wx, wz;
    xfPt(ox, oz, wx, wz);
    g_xf.ox = wx;
    g_xf.oz = wz;
    g_xf.rot = (g_xf.rot + rot) & 3;
    gPush();
    gTranslate(ox, 0, oz);
    gRotate((float)rot * 90.0f, 0, 1, 0);
}
static void popXf() {
    gPop();
    g_xf = g_xfStack.back();
    g_xfStack.pop_back();
}
static float xfYaw(float localYaw) { return localYaw + (float)g_xf.rot * PI * 0.5f; }

static void solidL(float lx0, float lz0, float lx1, float lz1, float y0, float y1) {
    float ax, az, bx, bz;
    xfPt(lx0, lz0, ax, az);
    xfPt(lx1, lz1, bx, bz);
    Solid s;
    s.x0 = std::min(ax, bx); s.x1 = std::max(ax, bx);
    s.z0 = std::min(az, bz); s.z1 = std::max(az, bz);
    s.y0 = y0; s.y1 = y1; s.yLow = y1; s.axis = 0;
    g_solids.push_back(s);
}

static void rampL(float lx0, float lz0, float lx1, float lz1, float y0, float yLow, float yHigh, int axis) {
    float ax, az, bx, bz;
    xfPt(lx0, lz0, ax, az);
    xfPt(lx1, lz1, bx, bz);
    float dx = 0, dz = 0;
    if (axis == 1) dx = 1;
    if (axis == -1) dx = -1;
    if (axis == 2) dz = 1;
    if (axis == -2) dz = -1;
    float wx, wz;
    xfVec(dx, dz, wx, wz);
    Solid s;
    s.x0 = std::min(ax, bx); s.x1 = std::max(ax, bx);
    s.z0 = std::min(az, bz); s.z1 = std::max(az, bz);
    s.y0 = y0; s.y1 = yHigh; s.yLow = yLow;
    s.axis = wx > 0.5f ? 1 : (wx < -0.5f ? -1 : (wz > 0.5f ? 2 : -2));
    g_solids.push_back(s);
}

static void railL(float ax, float ay, float az, float bx, float by, float bz, bool ledge) {
    Rail r;
    float wx, wz;
    xfPt(ax, az, wx, wz);
    r.a = V3(wx, ay, wz);
    xfPt(bx, bz, wx, wz);
    r.b = V3(wx, by, wz);
    r.ledge = ledge;
    r.tag = g_railTag;
    r.needLevel = g_railNeed;
    g_rails.push_back(r);
}

// Box that is drawn, collides, and has grindable edges along its long sides.
static void ledgeBoxL(float x0, float z0, float x1, float z1, float y0, float y1) {
    box(x0, y0, z0, x1, y1, z1);
    solidL(x0, z0, x1, z1, y0, y1);
    if (x1 - x0 >= z1 - z0) {
        railL(x0 + 0.05f, y1, z0 + 0.04f, x1 - 0.05f, y1, z0 + 0.04f, true);
        railL(x0 + 0.05f, y1, z1 - 0.04f, x1 - 0.05f, y1, z1 - 0.04f, true);
    } else {
        railL(x0 + 0.04f, y1, z0 + 0.05f, x0 + 0.04f, y1, z1 - 0.05f, true);
        railL(x1 - 0.04f, y1, z0 + 0.05f, x1 - 0.04f, y1, z1 - 0.05f, true);
    }
}

struct GroundInfo { float h, gx, gz; };

// Highest walkable surface under (x, z) whose top is not above yRef.
static GroundInfo groundAt(float x, float z, float yRef) {
    GroundInfo g = {0, 0, 0};
    for (size_t i = 0; i < g_solids.size(); i++) {
        const Solid& s = g_solids[i];
        if (!s.inside(x, z, 0)) continue;
        float t = s.topAt(x, z);
        if (t <= yRef && t > g.h) {
            g.h = t;
            s.grad(g.gx, g.gz);
        }
    }
    return g;
}

static const float PLAYER_R = 0.3f;
static const float PROM_Z0 = 120.0f, PROM_Z1 = 160.0f;  // sea face promenade paving
static const float SEAWALL_Z = 160.0f;
static const float PLAYER_H = 1.6f;

// True when a body of radius PLAYER_R standing at height y cannot occupy (x, z)
// because a surface rises more than `step` above its feet.
static bool blockedAt(float x, float z, float y, float step) {
    for (size_t i = 0; i < g_solids.size(); i++) {
        const Solid& s = g_solids[i];
        if (!s.inside(x, z, PLAYER_R)) continue;
        if (s.y0 > y + PLAYER_H) continue;
        float t = s.topAt(clampf(x, s.x0, s.x1), clampf(z, s.z0, s.z1));
        if (t > y + step) return true;
    }
    for (size_t i = 0; i < g_gates.size(); i++) {
        const Gate& g = g_gates[i];
        if (gateOpen(g) || y >= g.h) continue;
        if (x > g.x0 - PLAYER_R && x < g.x1 + PLAYER_R && z > g.z0 - PLAYER_R && z < g.z1 + PLAYER_R) return true;
    }
    return false;
}

// Fraction along segment a->b where it enters the box, or `best` if it misses first.
static float rayBox(const V3& a, const V3& b, const float lo[3], const float hi[3], float best) {
    V3 d = b - a;
    float o[3] = {a.x, a.y, a.z}, dd[3] = {d.x, d.y, d.z};
    float tmin = 0.0f, tmax = best;
    for (int k = 0; k < 3; k++) {
        if (fabsf(dd[k]) < 1e-6f) {
            if (o[k] < lo[k] || o[k] > hi[k]) return best;
        } else {
            float t1 = (lo[k] - o[k]) / dd[k], t2 = (hi[k] - o[k]) / dd[k];
            if (t1 > t2) std::swap(t1, t2);
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);
            if (tmin > tmax) return best;
        }
    }
    return tmin > 0.0f ? tmin : best;
}

// Fraction along segment a->b where it first enters a large solid. Used to keep
// the camera out of buildings.
static float rayBlock(const V3& a, const V3& b) {
    float best = 1.0f;
    V3 d = b - a;
    for (size_t i = 0; i < g_solids.size(); i++) {
        const Solid& s = g_solids[i];
        if (s.y1 < 1.2f) continue;
        if (s.x1 - s.x0 < 0.5f && s.z1 - s.z0 < 0.5f) continue;
        float tmin = 0.0f, tmax = best;
        float lo[3] = {s.x0, s.y0, s.z0}, hi[3] = {s.x1, s.y1, s.z1};
        float o[3] = {a.x, a.y, a.z}, dd[3] = {d.x, d.y, d.z};
        bool hit = true;
        for (int k = 0; k < 3 && hit; k++) {
            if (fabsf(dd[k]) < 1e-6f) {
                if (o[k] < lo[k] || o[k] > hi[k]) hit = false;
            } else {
                float t1 = (lo[k] - o[k]) / dd[k], t2 = (hi[k] - o[k]) / dd[k];
                if (t1 > t2) std::swap(t1, t2);
                tmin = std::max(tmin, t1);
                tmax = std::min(tmax, t2);
                if (tmin > tmax) hit = false;
            }
        }
        if (hit && tmin > 0.0f && tmin < best) best = tmin;
    }
    return best;
}

