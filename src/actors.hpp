#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Moving things: vehicles, trains and pedestrians. Their state lives here so the
// world builder can register vendors and parked autos while it builds.
// ============================================================================

enum VehType { VEH_BUS, VEH_DOUBLE, VEH_AUTO, VEH_TAXI, VEH_CAR };

struct Vehicle {
    int type;
    int lane;
    float pos;        // front bumper coordinate along the lane axis
    float speed;
    float cruise;
    float len, wid, hgt;
    Col col;
    float honkCooldown;
    int route;
    float wheelSpin;
    GLuint list;
    bool turning;     // cross-road U-turn in progress
    float turnA;      // 0..PI around the U-turn
};

struct Lane {
    int axis;       // 0: runs along X, 1: runs along Z
    float fixed;    // the other coordinate
    int dir;        // +1 or -1
    float minP, maxP;
    float stopAt;   // front bumper stops here on red
};

static const float UTURN_Z = 27.0f;  // where cross-road traffic turns around

// Indian traffic keeps left. For +X travel, left is -Z.
static Lane g_lanes[6] = {
    {0, -5.4f, +1, -190.0f, 190.0f, -15.6f},
    {0, -2.0f, +1, -190.0f, 190.0f, -15.6f},
    {0, 2.0f, -1, -190.0f, 190.0f, 15.6f},
    {0, 5.4f, -1, -190.0f, 190.0f, 15.6f},
    // the cross road: southbound traffic U-turns just past the crossing and heads back north
    {1, 3.5f, +1, -150.0f, UTURN_Z, -17.0f},
    {1, -3.5f, -1, -150.0f, UTURN_Z, 17.0f},
};

static std::vector<Vehicle> g_vehicles;

struct Train {
    float x;      // front of the lead car
    int dir;
    float speed;
    float z;
    int cars;
    int livery;
    std::vector<GLuint> carLists;
};
static std::vector<Train> g_trains;
static const float TRAIN_CAR_L = 19.5f;
static const float TRAIN_GAP = 0.6f;
static const float DECK_TOP = 7.5f;

enum PedStyle { PED_MAN, PED_SAREE, PED_SALWAR, PED_DABBAWALA, PED_KID, PED_VENDOR };

struct Ped {
    V3 pos;
    V3 a, b;          // patrol endpoints
    int target;       // 0 walking toward a, 1 toward b
    float speed;
    float phase;
    int style;
    Col top, bottom;
    int skin;
    float yaw;
    bool moving;
    float wait;
    float bumpT;
    float scale;
    bool umbrella;
    GLuint body, leg, arm;
};
static std::vector<Ped> g_peds;

struct Signal { float x, z, yaw; int axis; };
static std::vector<Signal> g_signals;

// Places the missions use, recorded while the city is built.
enum StallKind { STALL_CHAI, STALL_VADAPAV, STALL_PAAN, STALL_BHEL, STALL_FRUIT, STALL_BHUTTA, STALL_GOLA };
struct Stall {
    int kind;
    V3 stand;   // where a customer stands to order
    V3 kettle;  // chai stalls only: steam comes from here
};
static std::vector<Stall> g_stalls;

// A door or counter an order can be delivered to.
struct Dest {
    std::string name;
    V3 pos;
    int needLevel;
};
static std::vector<Dest> g_dests;
static V3 g_shopPos;   // counter of the skate shop under the railway bridge
static V3 g_boardPos;  // the job board beside it

static void addStallL(int kind, float y, float kx, float kz) {
    Stall s;
    s.kind = kind;
    float wx, wz;
    xfPt(0.0f, 2.3f, wx, wz);
    s.stand = V3(wx, y, wz);
    xfPt(kx, kz, wx, wz);
    s.kettle = V3(wx, y + 1.5f, wz);
    g_stalls.push_back(s);
}
static void addDestL(const std::string& name, float lx, float lz, float y, int need) {
    Dest d;
    d.name = name;
    float wx, wz;
    xfPt(lx, lz, wx, wz);
    d.pos = V3(wx, y, wz);
    d.needLevel = need;
    g_dests.push_back(d);
}

// Collectible letters that spell CHAI.
struct Letter { V3 pos; char ch; bool got; };
static std::vector<Letter> g_letters;

// Clothing palettes
static const Col SHIRTS[10] = {{0.85f, 0.2f, 0.2f}, {0.2f, 0.45f, 0.8f}, {0.95f, 0.95f, 0.9f}, {0.9f, 0.6f, 0.1f},
                               {0.3f, 0.65f, 0.35f}, {0.55f, 0.25f, 0.6f}, {0.95f, 0.8f, 0.6f}, {0.2f, 0.2f, 0.25f},
                               {0.9f, 0.4f, 0.6f}, {0.4f, 0.75f, 0.8f}};
static const Col SAREES[8] = {{0.85f, 0.1f, 0.35f}, {0.95f, 0.55f, 0.05f}, {0.1f, 0.55f, 0.45f}, {0.55f, 0.1f, 0.55f},
                              {0.9f, 0.8f, 0.1f}, {0.15f, 0.3f, 0.75f}, {0.75f, 0.15f, 0.1f}, {0.95f, 0.45f, 0.55f}};
static const Col PANTS[5] = {{0.2f, 0.22f, 0.3f}, {0.35f, 0.3f, 0.25f}, {0.12f, 0.12f, 0.14f}, {0.55f, 0.5f, 0.4f},
                             {0.25f, 0.3f, 0.45f}};

static void addPed(const V3& a, const V3& b, int style, bool moving) {
    Ped p;
    p.a = a;
    p.b = b;
    p.pos = a + (b - a) * frand();
    p.target = frand() < 0.5f ? 0 : 1;
    p.speed = frange(0.9f, 1.5f);
    p.phase = frange(0, TWO_PI);
    p.style = style;
    p.skin = irange(0, 3);
    p.top = SHIRTS[irange(0, 9)];
    p.bottom = PANTS[irange(0, 4)];
    if (style == PED_SAREE) { p.bottom = SAREES[irange(0, 7)]; p.top = mulc(p.bottom, 0.8f); }
    if (style == PED_SALWAR) { p.top = SAREES[irange(0, 7)]; p.bottom = C(0.92f, 0.9f, 0.85f); }
    if (style == PED_DABBAWALA) { p.top = C(0.96f, 0.96f, 0.94f); p.bottom = C(0.94f, 0.94f, 0.92f); p.speed = 1.7f; }
    if (style == PED_KID) { p.top = C(0.96f, 0.96f, 0.96f); p.bottom = C(0.12f, 0.16f, 0.35f); }
    if (style == PED_VENDOR) { p.top = frand() < 0.5f ? C(0.95f, 0.95f, 0.9f) : C(0.75f, 0.6f, 0.4f); }
    p.yaw = 0;
    p.moving = moving;
    p.wait = 0;
    p.bumpT = 0;
    p.scale = style == PED_KID ? 0.72f : frange(0.95f, 1.07f);
    p.umbrella = moving && style != PED_DABBAWALA && frand() < 0.12f;
    g_peds.push_back(p);
}

// Stationary vendor placed in the current local frame, facing local +Z.
static void addVendorL(float lx, float lz, float y) {
    float wx, wz;
    xfPt(lx, lz, wx, wz);
    addPed(V3(wx, y, wz), V3(wx, y, wz), PED_VENDOR, false);
    g_peds.back().pos = V3(wx, y, wz);
    g_peds.back().yaw = xfYaw(0);
}

// ---------------------------------------------------------------- vehicle meshes
// Local frame: centre of the vehicle on the ground, nose toward +Z.

static void wheelX(float x, float y, float z, float r, float w, float spin) {
    gPush();
    gTranslate(x, y, z);
    gRotate(90, 0, 0, 1);
    gRotate(spin * RAD2DEG, 0, 1, 0);
    gTranslate(0, -w * 0.5f, 0);
    setc(0.06f, 0.06f, 0.06f);
    cyl(r, r, w, 10, true);
    setc(0.55f, 0.55f, 0.55f);
    gTranslate(0, -0.005f, 0);
    cyl(r * 0.45f, r * 0.45f, w + 0.01f, 6, true);
    gPop();
}

// Text on a vehicle side. side = +1 for the +X side, -1 for the -X side.
static void sideText(const char* s, float side, float x, float y, float z, float h, const Col& c) {
    gPush();
    gTranslate(x * side, y, z);
    gRotate(side * 90.0f, 0, 1, 0);
    text3D(s, h, c);
    gPop();
}

static void drawBus(bool dbl, int route, float spin) {
    const float L = 11.0f, W = 2.5f, H = dbl ? 4.4f : 3.15f;
    Col red = C(0.78f, 0.1f, 0.08f), cream = C(0.95f, 0.88f, 0.7f);
    setc(red);
    box(-W / 2, 0.45f, -L / 2, W / 2, H - 0.15f, L / 2);
    setc(0.85f, 0.85f, 0.82f);
    box(-W / 2 + 0.05f, H - 0.15f, -L / 2 + 0.05f, W / 2 - 0.05f, H, L / 2 - 0.05f);
    setc(cream);
    box(-W / 2 - 0.01f, 1.2f, -L / 2 - 0.01f, W / 2 + 0.01f, 1.32f, L / 2 + 0.01f);
    if (dbl) box(-W / 2 - 0.01f, 2.55f, -L / 2 - 0.01f, W / 2 + 0.01f, 2.67f, L / 2 + 0.01f);
    // windows
    setc(0.12f, 0.16f, 0.2f);
    for (int deck = 0; deck < (dbl ? 2 : 1); deck++) {
        float y0 = deck == 0 ? 1.45f : 2.8f, y1 = y0 + 0.9f;
        for (float z = -L / 2 + 0.6f; z < L / 2 - 1.4f; z += 1.25f) {
            rectX(z, y0, z + 1.0f, y1, W / 2 + 0.012f);
            rectX(z, y0, z + 1.0f, y1, -W / 2 - 0.012f);
        }
    }
    // doors on the kerb side (left, which is +X for a vehicle facing +Z)
    setc(0.08f, 0.08f, 0.09f);
    rectX(L / 2 - 1.9f, 0.5f, L / 2 - 0.9f, 2.4f, W / 2 + 0.015f);
    rectX(-L / 2 + 1.2f, 0.5f, -L / 2 + 2.2f, 2.4f, W / 2 + 0.015f);
    // windscreen, destination board, lights
    setc(0.1f, 0.13f, 0.16f);
    rectZ(-W / 2 + 0.1f, 1.35f, W / 2 - 0.1f, 2.45f, L / 2 + 0.012f);
    rectZ(-W / 2 + 0.2f, 1.45f, W / 2 - 0.2f, 2.35f, -L / 2 - 0.012f);
    setc(0.02f, 0.02f, 0.02f);
    rectZ(-W / 2 + 0.15f, 2.55f, W / 2 - 0.15f, 2.95f, L / 2 + 0.014f);
    char buf[32];
    snprintf(buf, sizeof(buf), "%d DADAR", route);
    gPush();
    gTranslate(0, 2.64f, L / 2 + 0.02f);
    text3DFit(buf, 0.24f, W - 0.5f, C(1.0f, 0.55f, 0.1f));
    gPop();
    gLighting(false);
    setc(1, 1, 0.85f);
    rectZ(-W / 2 + 0.15f, 0.65f, -W / 2 + 0.45f, 0.85f, L / 2 + 0.014f);
    rectZ(W / 2 - 0.45f, 0.65f, W / 2 - 0.15f, 0.85f, L / 2 + 0.014f);
    setc(0.8f, 0.05f, 0.05f);
    rectZ(-W / 2 + 0.15f, 0.65f, -W / 2 + 0.45f, 0.85f, -L / 2 - 0.014f);
    rectZ(W / 2 - 0.45f, 0.65f, W / 2 - 0.15f, 0.85f, -L / 2 - 0.014f);
    gLighting(true);
    sideText("BEST", 1, W / 2 + 0.02f, 0.72f, -1.0f, 0.38f, cream);
    sideText("BEST", -1, W / 2 + 0.02f, 0.72f, 0.5f, 0.38f, cream);
    setc(0.15f, 0.15f, 0.15f);
    box(-W / 2 + 0.05f, 0.25f, -L / 2 + 0.3f, W / 2 - 0.05f, 0.45f, L / 2 - 0.3f);
    for (int s = -1; s <= 1; s += 2) {
        wheelX(s * (W / 2 - 0.2f), 0.5f, L / 2 - 2.0f, 0.5f, 0.32f, spin);
        wheelX(s * (W / 2 - 0.2f), 0.5f, -L / 2 + 2.4f, 0.5f, 0.32f, spin);
    }
}

static void drawAuto(float spin) {
    Col black = C(0.07f, 0.07f, 0.07f), yellow = C(0.98f, 0.78f, 0.05f), green = C(0.1f, 0.45f, 0.2f);
    setc(black);
    box(-0.66f, 0.28f, -1.25f, 0.66f, 0.95f, 0.55f);        // passenger tub
    box(-0.36f, 0.28f, 0.55f, 0.36f, 1.05f, 1.3f);          // nose
    setc(green);
    box(-0.67f, 0.6f, -1.26f, 0.67f, 0.68f, 0.56f);         // trim stripe
    setc(yellow);
    box(-0.7f, 1.62f, -1.35f, 0.7f, 1.78f, 0.95f);          // canopy roof
    box(-0.7f, 0.95f, -1.4f, 0.7f, 1.72f, -1.22f);          // canopy back
    setc(0.2f, 0.2f, 0.2f);
    box(-0.66f, 0.95f, 0.85f, -0.6f, 1.62f, 0.93f);
    box(0.6f, 0.95f, 0.85f, 0.66f, 1.62f, 0.93f);
    // windscreen
    setc(0.15f, 0.2f, 0.24f);
    quad4(V3(-0.5f, 1.05f, 1.28f), V3(0.5f, 1.05f, 1.28f), V3(0.55f, 1.58f, 0.95f), V3(-0.55f, 1.58f, 0.95f));
    // seats
    setc(0.35f, 0.18f, 0.12f);
    box(-0.6f, 0.75f, -1.2f, 0.6f, 0.95f, -0.6f);
    box(-0.6f, 0.95f, -1.22f, 0.6f, 1.45f, -1.1f);
    box(-0.25f, 0.85f, 0.1f, 0.25f, 1.0f, 0.45f);
    // driver in khaki
    setc(0.62f, 0.54f, 0.36f);
    box(-0.18f, 1.0f, 0.18f, 0.18f, 1.45f, 0.42f);
    setc(COL_SKIN[1]);
    gPush();
    gTranslate(0, 1.55f, 0.33f);
    sphere(0.11f, 8, 6);
    gPop();
    setc(0.3f, 0.3f, 0.3f);
    cylBetween(V3(-0.3f, 1.1f, 0.75f), V3(0.3f, 1.1f, 0.75f), 0.02f, 5);
    gLighting(false);
    setc(1, 1, 0.8f);
    rectZ(-0.1f, 0.8f, 0.1f, 0.95f, 1.31f);
    gLighting(true);
    wheelX(0, 0.24f, 1.05f, 0.24f, 0.14f, spin);
    wheelX(-0.6f, 0.24f, -0.8f, 0.24f, 0.14f, spin);
    wheelX(0.6f, 0.24f, -0.8f, 0.24f, 0.14f, spin);
}

static void drawCarBody(const Col& body, const Col& roof, float spin, float L, float W) {
    setc(body);
    box(-W / 2, 0.3f, -L / 2, W / 2, 0.92f, L / 2);
    setc(roof);
    box(-W / 2 + 0.08f, 1.4f, -L * 0.22f, W / 2 - 0.08f, 1.48f, L * 0.18f);
    setc(body);
    box(-W / 2 + 0.06f, 0.92f, -L * 0.24f, -W / 2 + 0.14f, 1.42f, L * 0.2f);
    box(W / 2 - 0.14f, 0.92f, -L * 0.24f, W / 2 - 0.06f, 1.42f, L * 0.2f);
    setc(0.12f, 0.15f, 0.19f);
    box(-W / 2 + 0.12f, 0.92f, -L * 0.23f, W / 2 - 0.12f, 1.4f, L * 0.19f);
    quad4(V3(-W / 2 + 0.1f, 0.92f, L * 0.19f), V3(W / 2 - 0.1f, 0.92f, L * 0.19f),
          V3(W / 2 - 0.1f, 1.4f, L * 0.13f), V3(-W / 2 + 0.1f, 1.4f, L * 0.13f));
    gLighting(false);
    setc(1, 1, 0.85f);
    rectZ(-W / 2 + 0.1f, 0.6f, -W / 2 + 0.35f, 0.78f, L / 2 + 0.01f);
    rectZ(W / 2 - 0.35f, 0.6f, W / 2 - 0.1f, 0.78f, L / 2 + 0.01f);
    setc(0.75f, 0.05f, 0.05f);
    rectZ(-W / 2 + 0.1f, 0.6f, -W / 2 + 0.35f, 0.78f, -L / 2 - 0.01f);
    rectZ(W / 2 - 0.35f, 0.6f, W / 2 - 0.1f, 0.78f, -L / 2 - 0.01f);
    gLighting(true);
    for (int s = -1; s <= 1; s += 2) {
        wheelX(s * (W / 2 - 0.1f), 0.3f, L * 0.32f, 0.3f, 0.2f, spin);
        wheelX(s * (W / 2 - 0.1f), 0.3f, -L * 0.32f, 0.3f, 0.2f, spin);
    }
}

// Kaali-peeli: the black and yellow Premier Padmini taxi.
static void drawTaxi(float spin) {
    drawCarBody(C(0.06f, 0.06f, 0.06f), C(0.98f, 0.8f, 0.05f), spin, 3.9f, 1.55f);
    setc(0.98f, 0.8f, 0.05f);
    box(-0.7f, 1.3f, -0.85f, 0.7f, 1.41f, 0.72f);
    setc(0.3f, 0.3f, 0.3f);
    for (int i = -1; i <= 1; i++) box(-0.6f, 1.48f, (float)i * 0.3f - 0.02f, 0.6f, 1.53f, (float)i * 0.3f + 0.02f);
}

static void drawVehicleMesh(const Vehicle& v) {
    switch (v.type) {
        case VEH_BUS: drawBus(false, v.route, v.wheelSpin); break;
        case VEH_DOUBLE: drawBus(true, v.route, v.wheelSpin); break;
        case VEH_AUTO: drawAuto(v.wheelSpin); break;
        case VEH_TAXI: drawTaxi(v.wheelSpin); break;
        default: drawCarBody(v.col, v.col, v.wheelSpin, 4.2f, 1.75f); break;
    }
}

// ---------------------------------------------------------------- local train

static void trainPerson(float x, float y, float z, float side, int seed) {
    setc(SHIRTS[seed % 10]);
    box(x - 0.18f, y, z - 0.12f, x + 0.18f, y + 0.6f, z + 0.12f);
    setc(COL_SKIN[seed % 4]);
    gPush();
    gTranslate(x, y + 0.72f, z + side * 0.05f);
    sphere(0.11f, 7, 5);
    gPop();
    // one arm reaching for the door pole
    setc(SHIRTS[seed % 10]);
    cylBetween(V3(x + 0.15f, y + 0.5f, z), V3(x + 0.35f, y + 1.0f, z - side * 0.15f), 0.045f, 4);
}

static void drawTrainCar(float xa, float xb, float zc, int livery, int cab, int seed) {
    const float W = 3.5f, yb = DECK_TOP + 0.25f;
    Col upper = livery == 0 ? C(0.92f, 0.86f, 0.68f) : C(0.88f, 0.88f, 0.9f);
    Col lower = livery == 0 ? C(0.55f, 0.13f, 0.15f) : C(0.45f, 0.24f, 0.55f);
    Col stripe = livery == 0 ? C(0.95f, 0.75f, 0.2f) : C(0.95f, 0.8f, 0.1f);
    float z0 = zc - W / 2, z1 = zc + W / 2;
    setc(0.15f, 0.15f, 0.16f);
    box(xa + 1.5f, DECK_TOP + 0.1f, z0 + 0.4f, xa + 4.0f, yb + 0.35f, z1 - 0.4f);
    box(xb - 4.0f, DECK_TOP + 0.1f, z0 + 0.4f, xb - 1.5f, yb + 0.35f, z1 - 0.4f);
    setc(0.25f, 0.25f, 0.27f);
    box(xa, yb + 0.3f, z0 + 0.1f, xb, yb + 0.55f, z1 - 0.1f);
    setc(lower);
    box(xa, yb + 0.55f, z0, xb, yb + 1.5f, z1);
    setc(upper);
    box(xa, yb + 1.5f, z0, xb, yb + 3.4f, z1);
    setc(stripe);
    box(xa - 0.01f, yb + 1.45f, z0 - 0.01f, xb + 0.01f, yb + 1.6f, z1 + 0.01f);
    setc(0.62f, 0.63f, 0.66f);
    box(xa + 0.1f, yb + 3.4f, z0 + 0.25f, xb - 0.1f, yb + 3.72f, z1 - 0.25f);
    float L = xb - xa;
    float doors[3] = {xa + L * 0.16f, xa + L * 0.5f, xa + L * 0.84f};
    for (int s = -1; s <= 1; s += 2) {
        float zf = s > 0 ? z1 + 0.012f : z0 - 0.012f;
        gPush();
        gTranslate(0, 0, zf);
        // windows between doors
        setc(0.14f, 0.16f, 0.2f);
        for (float x = xa + 0.8f; x < xb - 1.0f; x += 1.55f) {
            bool nearDoor = false;
            for (int d = 0; d < 3; d++) if (fabsf(x + 0.5f - doors[d]) < 1.3f) nearDoor = true;
            if (!nearDoor) rectZ(x, yb + 2.0f, x + 1.0f, yb + 2.95f, 0);
        }
        setc(0.05f, 0.05f, 0.06f);
        for (int d = 0; d < 3; d++) rectZ(doors[d] - 0.65f, yb + 0.65f, doors[d] + 0.65f, yb + 3.2f, 0.001f * s);
        gPop();
        // commuters hanging out of the doors
        for (int d = 0; d < 3; d++) {
            int k = seed * 7 + d * 3 + (s > 0 ? 1 : 0);
            if (k % 3 != 0) continue;
            trainPerson(doors[d] - 0.2f, yb + 0.7f, zc + s * (W / 2 + 0.05f), (float)s, k);
            if (k % 2 == 0) trainPerson(doors[d] + 0.35f, yb + 0.7f, zc + s * (W / 2 - 0.1f), (float)s, k + 5);
        }
    }
    if (cab != 0) {
        float xf = cab > 0 ? xb + 0.012f : xa - 0.012f;
        gPush();
        gTranslate(xf, 0, 0);
        setc(0.1f, 0.12f, 0.15f);
        rectX(z0 + 0.3f, yb + 2.0f, z1 - 0.3f, yb + 3.0f, 0);
        gLighting(false);
        setc(1, 1, 0.85f);
        rectX(z0 + 0.3f, yb + 0.9f, z0 + 0.7f, yb + 1.2f, 0.001f * cab);
        rectX(z1 - 0.7f, yb + 0.9f, z1 - 0.3f, yb + 1.2f, 0.001f * cab);
        gLighting(true);
        gPop();
    }
    if (seed % 3 == 1) {
        setc(0.2f, 0.2f, 0.2f);
        float xm = (xa + xb) * 0.5f, yt = yb + 3.72f;
        cylBetween(V3(xm - 0.8f, yt, zc), V3(xm, yt + 0.9f, zc), 0.03f, 4);
        cylBetween(V3(xm + 0.8f, yt, zc), V3(xm, yt + 0.9f, zc), 0.03f, 4);
        box(xm - 0.4f, yt + 0.88f, zc - 0.6f, xm + 0.4f, yt + 0.95f, zc + 0.6f);
    }
}


static void bakeTrain(Train& t) {
    t.carLists.clear();
    for (int i = 0; i < t.cars; i++) {
        int cab = 0;
        if (i == 0) cab = t.dir;
        if (i == t.cars - 1) cab = -t.dir;
        recBegin();
        drawTrainCar(0, TRAIN_CAR_L, 0, t.livery, cab, i + t.livery * 11);
        t.carLists.push_back(recEnd());
    }
}

// ---------------------------------------------------------------- people

// Tapered cylinder from a to b, radius r0 at a and r1 at b.
static void coneBetween(const V3& a, const V3& b, float r0, float r1, int n = 8) {
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
    cyl(r0, r1, L, n, false);
    gPop();
}
static void ball(const V3& p, float r, const Col& c) {
    setc(c);
    gPush();
    gTranslate(p.x, p.y, p.z);
    sphere(r, 8, 6);
    gPop();
}

// A sneaker, toe toward +Z, sole on y.
static void sneaker(const V3& p, const Col& upper) {
    setc(0.95f, 0.95f, 0.93f);
    box(p.x - 0.062f, p.y - 0.045f, p.z - 0.11f, p.x + 0.062f, p.y - 0.015f, p.z + 0.17f);
    setc(upper);
    box(p.x - 0.058f, p.y - 0.015f, p.z - 0.1f, p.x + 0.058f, p.y + 0.055f, p.z + 0.08f);
    gPush();
    gTranslate(p.x, p.y - 0.005f, p.z + 0.1f);
    gScale(1.0f, 0.75f, 1.2f);
    sphere(0.058f, 8, 5);
    gPop();
    setc(0.97f, 0.97f, 0.97f);
    box(p.x - 0.03f, p.y + 0.052f, p.z - 0.02f, p.x + 0.03f, p.y + 0.058f, p.z + 0.07f);  // laces
}

// Face and ears in the head's frame (face toward +Z, head centre at the origin).
static void drawFace(const Col& skin) {
    Col dark = C(0.06f, 0.05f, 0.05f);
    ball(V3(-0.042f, 0.018f, 0.104f), 0.017f, C(0.96f, 0.96f, 0.94f));
    ball(V3(0.042f, 0.018f, 0.104f), 0.017f, C(0.96f, 0.96f, 0.94f));
    ball(V3(-0.042f, 0.018f, 0.117f), 0.009f, dark);
    ball(V3(0.042f, 0.018f, 0.117f), 0.009f, dark);
    setc(dark);
    box(-0.065f, 0.048f, 0.1f, -0.02f, 0.058f, 0.118f);
    box(0.02f, 0.048f, 0.1f, 0.065f, 0.058f, 0.118f);
    ball(V3(0, -0.012f, 0.118f), 0.019f, mulc(skin, 0.9f));
    setc(0.35f, 0.12f, 0.1f);
    box(-0.025f, -0.056f, 0.103f, 0.025f, -0.046f, 0.112f);
    ball(V3(-0.118f, 0.0f, 0.0f), 0.028f, skin);
    ball(V3(0.118f, 0.0f, 0.0f), 0.028f, skin);
}

// Everything on a person except the swinging legs and arms, in their local frame
// (facing +Z, feet at y = 0).
static void drawPersonBody(const Ped& p) {
    const Col& skin = COL_SKIN[p.skin];
    Col hair = C(0.06f, 0.05f, 0.05f);
    bool woman = p.style == PED_SAREE || p.style == PED_SALWAR;
    if (p.style == PED_SAREE) {
        // pleated skirt, a strip of midriff, then the blouse
        setc(p.bottom);
        cyl(0.27f, 0.16f, 0.96f, 14, true);
        setc(mulc(p.bottom, 0.82f));
        for (int k = -2; k <= 2; k++) box((float)k * 0.045f - 0.008f, 0.02f, 0.2f - fabsf((float)k) * 0.02f, (float)k * 0.045f + 0.008f, 0.9f, 0.24f - fabsf((float)k) * 0.02f);
        setc(skin);
        gPush();
        gTranslate(0, 0.94f, 0);
        gScale(1.0f, 1.0f, 0.65f);
        cyl(0.145f, 0.145f, 0.1f, 12, false);
        gPop();
        setc(p.top);
        gPush();
        gTranslate(0, 1.03f, 0);
        gScale(1.0f, 1.0f, 0.62f);
        cyl(0.15f, 0.17f, 0.32f, 12, false);
        gTranslate(0, 0.32f, 0);
        cyl(0.17f, 0.06f, 0.05f, 12, true);
        gPop();
        // pallu over the left shoulder, hanging down the back
        setc(p.bottom);
        quad4(V3(0.19f, 1.38f, 0.1f), V3(-0.13f, 0.95f, 0.13f), V3(0.02f, 0.95f, 0.14f), V3(0.21f, 1.25f, 0.11f));
        quad4(V3(0.19f, 1.38f, -0.1f), V3(0.21f, 1.25f, -0.11f), V3(0.12f, 0.62f, -0.14f), V3(-0.04f, 0.62f, -0.13f));
        setc(C(0.95f, 0.8f, 0.2f));
        quad4(V3(0.12f, 0.62f, -0.141f), V3(-0.04f, 0.62f, -0.131f), V3(-0.03f, 0.68f, -0.132f), V3(0.12f, 0.68f, -0.142f));
    } else {
        // hips, then a chest that widens toward the shoulders
        setc(p.bottom);
        gPush();
        gTranslate(0, 0.86f, 0);
        gScale(1.0f, 0.55f, 0.68f);
        sphere(0.17f, 10, 6);
        gPop();
        float hem = (p.style == PED_SALWAR || p.style == PED_DABBAWALA) ? 0.5f : 0.82f;
        setc(p.top);
        gPush();
        gTranslate(0, hem, 0);
        gScale(1.0f, 1.0f, 0.62f);
        cyl(hem < 0.7f ? 0.21f : 0.16f, 0.18f, 1.36f - hem, 12, false);
        gTranslate(0, 1.36f - hem, 0);
        cyl(0.18f, 0.06f, 0.05f, 12, true);
        gPop();
        setc(mulc(p.top, 0.78f));
        gPush();
        gTranslate(0, 1.37f, 0);
        cyl(0.065f, 0.058f, 0.03f, 10, false);  // collar
        gPop();
        if (p.style == PED_SALWAR) {
            // dupatta across the chest
            setc(mulc(p.top, 0.7f));
            quad4(V3(-0.2f, 1.38f, 0.12f), V3(0.2f, 1.38f, 0.12f), V3(0.2f, 1.2f, -0.12f), V3(-0.2f, 1.2f, -0.12f));
        }
        if (p.style == PED_MAN && p.skin % 2 == 0) {
            setc(mulc(p.top, 0.85f));  // shirt pocket and button line
            box(0.05f, 1.15f, 0.108f, 0.12f, 1.24f, 0.113f);
            box(-0.006f, 0.85f, 0.11f, 0.006f, 1.34f, 0.114f);
        }
    }
    // shoulders
    Col sleeve = p.top;
    ball(V3(-0.2f, 1.33f, 0), 0.068f, sleeve);
    ball(V3(0.2f, 1.33f, 0), 0.068f, sleeve);
    // neck, head, face and hair
    setc(skin);
    coneBetween(V3(0, 1.36f, 0), V3(0, 1.47f, 0.01f), 0.05f, 0.046f);
    gPush();
    gTranslate(0, 1.56f, 0);
    setc(skin);
    gPush();
    gScale(0.93f, 1.08f, 1.0f);
    sphere(0.115f, 12, 9);
    gPop();
    drawFace(skin);
    setc(hair);
    gPush();
    gTranslate(0, 0.035f, -0.022f);
    gScale(0.95f, 0.9f, 1.0f);
    sphere(0.117f, 12, 8);
    gPop();
    if (woman) {
        box(-0.09f, 0.06f, 0.06f, 0.09f, 0.1f, 0.1f);
        gPush();
        gTranslate(0, -0.02f, -0.12f);
        sphere(0.07f, 7, 5);  // bun
        gPop();
        if (p.style == PED_SALWAR) coneBetween(V3(0, -0.02f, -0.13f), V3(0, -0.34f, -0.15f), 0.045f, 0.02f);  // braid
    } else {
        box(-0.08f, 0.06f, 0.07f, 0.08f, 0.095f, 0.1f);
        if (p.style == PED_MAN && p.skin != 2) box(-0.045f, -0.036f, 0.108f, 0.045f, -0.026f, 0.118f);  // moustache
    }
    gPop();
    if (p.style == PED_DABBAWALA) {
        setc(0.98f, 0.98f, 0.98f);
        box(-0.1f, 1.63f, -0.12f, 0.1f, 1.73f, 0.12f);  // Gandhi cap
        setc(COL_WOOD);
        box(-0.35f, 1.75f, -0.3f, 0.35f, 1.85f, 0.3f);  // crate of tiffins
        setc(COL_STEEL);
        for (int i = -1; i <= 1; i++)
            for (int j = -1; j <= 1; j += 2) {
                gPush();
                gTranslate((float)i * 0.22f, 1.85f, (float)j * 0.14f);
                cyl(0.09f, 0.09f, 0.22f, 7, true);
                gPop();
            }
    }
    if (p.umbrella) {
        setc(0.08f, 0.08f, 0.1f);
        cylBetween(V3(0.23f, 1.05f, 0.35f), V3(0.23f, 2.1f, 0.35f), 0.015f, 4);
        gPush();
        gTranslate(0.23f, 1.85f, 0.35f);
        cyl(0.55f, 0.0f, 0.3f, 10, false);
        gPop();
    }
}

// Bakes a person's body, one leg and one arm into display lists. Limbs hang
// from their pivot at the origin.
static void buildPedLists(Ped& p) {
    recBegin();
    drawPersonBody(p);
    p.body = recEnd();
    p.leg = 0;
    if (p.style != PED_SAREE) {
        recBegin();
        setc(p.bottom);
        coneBetween(V3(0, 0, 0), V3(0, -0.42f, 0.02f), 0.078f, 0.064f);
        ball(V3(0, -0.42f, 0.02f), 0.064f, p.bottom);
        setc(p.bottom);
        coneBetween(V3(0, -0.42f, 0.02f), V3(0, -0.8f, 0), 0.062f, 0.055f);
        bool sandal = p.style == PED_DABBAWALA || p.style == PED_VENDOR;
        setc(sandal ? C(0.35f, 0.22f, 0.12f) : C(0.12f, 0.1f, 0.09f));
        box(-0.055f, -0.87f, -0.08f, 0.055f, -0.845f, 0.17f);
        if (sandal) {
            setc(COL_SKIN[p.skin]);
            box(-0.045f, -0.845f, -0.06f, 0.045f, -0.8f, 0.15f);
        } else {
            box(-0.055f, -0.845f, -0.07f, 0.055f, -0.79f, 0.12f);
            ball(V3(0, -0.83f, 0.12f), 0.05f, C(0.12f, 0.1f, 0.09f));
        }
        p.leg = recEnd();
    }
    recBegin();
    bool longSleeve = p.style == PED_DABBAWALA || p.style == PED_SALWAR;
    Col skin = COL_SKIN[p.skin];
    setc(p.top);
    coneBetween(V3(0, 0, 0), V3(0, longSleeve ? -0.52f : -0.25f, 0), 0.058f, 0.048f);
    if (!longSleeve) {
        setc(skin);
        coneBetween(V3(0, -0.25f, 0), V3(0, -0.53f, 0.02f), 0.042f, 0.035f);
    }
    ball(V3(0, -0.58f, 0.02f), 0.047f, skin);
    p.arm = recEnd();
}

static void drawPerson(const Ped& p) {
    float sw = p.moving ? sinf(p.phase) * 0.55f : 0.0f;
    gPush();
    gTranslate(p.pos.x, p.pos.y, p.pos.z);
    gRotate(p.yaw * RAD2DEG, 0, 1, 0);
    if (p.bumpT > 0) gRotate(sinf(p.bumpT * 20.0f) * 12.0f, 0, 0, 1);
    gScale(p.scale, p.scale, p.scale);
    if (p.moving) gTranslate(0, fabsf(cosf(p.phase)) * 0.03f, 0);
    drawMesh(p.body);
    if (p.leg) {
        for (int s = -1; s <= 1; s += 2) {
            gPush();
            gTranslate((float)s * 0.09f, 0.86f, 0);
            gRotate(-(float)s * sw * RAD2DEG, 1, 0, 0);
            drawMesh(p.leg);
            gPop();
        }
    }
    float armL = p.umbrella ? 0.0f : -sw * 0.8f, armR = p.umbrella ? -1.2f : sw * 0.8f;
    gPush();
    gTranslate(-0.23f, 1.37f, 0);
    gRotate(armL * RAD2DEG, 1, 0, 0);
    drawMesh(p.arm);
    gPop();
    gPush();
    gTranslate(0.23f, 1.37f, 0);
    gRotate(armR * RAD2DEG, 1, 0, 0);
    drawMesh(p.arm);
    gPop();
    gPop();
}

