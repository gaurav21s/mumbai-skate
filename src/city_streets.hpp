#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// City builder, part 1: ground, streets, street furniture, trees, buildings.
//
// Layout (metres, north is -Z):
//   main road       z in [-8, 8], runs along X, median railing at z = 0
//   cross road      x in [-7, 7], runs along Z
//   south shops     z > 13, facing the main road
//   NW quadrant     the maidan plaza with kickers, funbox, rails and a skywalk
//   NE quadrant     construction site, station, auto stand
//   railway         elevated viaduct along X at z = -48, deck at y = 7.5
// ============================================================================

static const float SIDEWALK_H = 0.18f;
static const float WORLD_MIN_X = -141.0f, WORLD_MAX_X = 141.0f;

// Painted kerb: alternating black and yellow blocks on a vertical face.
static void kerbStripeZ(float x0, float x1, float z, float h) {
    int i = 0;
    for (float x = x0; x < x1; x += 1.0f, i++) {
        if (i & 1) setc(0.1f, 0.1f, 0.1f); else setc(0.95f, 0.78f, 0.1f);
        rectZ(x, 0.0f, std::min(x + 1.0f, x1), h, z);
    }
}
static void kerbStripeX(float z0, float z1, float x, float h) {
    int i = 0;
    for (float z = z0; z < z1; z += 1.0f, i++) {
        if (i & 1) setc(0.1f, 0.1f, 0.1f); else setc(0.95f, 0.78f, 0.1f);
        rectX(z, 0.0f, std::min(z + 1.0f, z1), h, x);
    }
}

// Sidewalk slab with paver lines. Kerb mask bits: 1 = -z face, 2 = +z face,
// 4 = -x face, 8 = +x face.
static void sidewalk(float x0, float z0, float x1, float z1, int kerbs) {
    setc(0.63f, 0.59f, 0.53f);
    box(x0, 0, z0, x1, SIDEWALK_H, z1);
    solidL(x0, z0, x1, z1, 0, SIDEWALK_H);
    gLighting(false);
    setc(0.52f, 0.49f, 0.44f);
    gBegin(GL_LINES);
    float cx0 = std::max(x0, -220.0f), cx1 = std::min(x1, 220.0f);
    float cz0 = std::max(z0, -220.0f), cz1 = std::min(z1, 220.0f);
    for (float x = cx0; x <= cx1; x += 1.2f) { gVertex(x, SIDEWALK_H + 0.01f, cz0); gVertex(x, SIDEWALK_H + 0.01f, cz1); }
    for (float z = cz0; z <= cz1; z += 1.2f) { gVertex(cx0, SIDEWALK_H + 0.01f, z); gVertex(cx1, SIDEWALK_H + 0.01f, z); }
    gEnd();
    gLighting(true);
    if (kerbs & 1) kerbStripeZ(cx0, cx1, z0 - 0.01f, SIDEWALK_H);
    if (kerbs & 2) kerbStripeZ(cx0, cx1, z1 + 0.01f, SIDEWALK_H);
    if (kerbs & 4) kerbStripeX(cz0, cz1, x0 - 0.01f, SIDEWALK_H);
    if (kerbs & 8) kerbStripeX(cz0, cz1, x1 + 0.01f, SIDEWALK_H);
}

static void buildGround() {
    // base ground
    setc(0.5f, 0.47f, 0.42f);
    rectY(-500, -500, 500, 161, -0.03f);
    // asphalt
    setc(0.24f, 0.24f, 0.26f);
    rectY(-500, -8, 500, 8, 0.0f);
    rectY(-7, -500, 7, -8, 0.0f);
    rectY(-7, 8, 7, 121, 0.0f);
    // repair patches
    for (int i = 0; i < 70; i++) {
        float x = frange(-180, 180), z = frange(-7, 7), w = frange(1, 4), d = frange(0.8f, 2.5f);
        float k = frange(0.8f, 1.15f);
        setc(0.24f * k, 0.24f * k, 0.26f * k);
        rectY(x, z, x + w, std::min(z + d, 7.8f), 0.012f);
    }
    // lane dashes
    setc(0.9f, 0.9f, 0.85f);
    for (float x = -300; x < 300; x += 6.0f) {
        if (x > -16 && x < 16) continue;
        rectY(x, -3.8f, x + 3.0f, -3.6f, 0.02f);
        rectY(x, 3.6f, x + 3.0f, 3.8f, 0.02f);
    }
    for (float z = -300; z < 116; z += 6.0f) {
        if (z > -18 && z < 18) continue;
        rectY(-0.1f, z, 0.1f, z + 3.0f, 0.02f);
    }
    // zebra crossings and stop lines
    for (float z = -7.4f; z < 7.4f; z += 1.0f) {
        if (fabsf(z + 0.25f) < 0.6f) continue;
        rectY(11.8f, z, 14.4f, z + 0.5f, 0.02f);
        rectY(-14.4f, z, -11.8f, z + 0.5f, 0.02f);
    }
    for (float x = -6.6f; x < 6.6f; x += 1.0f) {
        rectY(x, 13.6f, x + 0.5f, 16.2f, 0.02f);
        rectY(x, -16.2f, x + 0.5f, -13.6f, 0.02f);
    }
    rectY(-15.8f, -7.8f, -15.4f, -0.4f, 0.02f);
    rectY(15.4f, 0.4f, 15.8f, 7.8f, 0.02f);
    rectY(0.2f, -17.2f, 6.8f, -16.8f, 0.02f);
    rectY(-6.8f, 16.8f, -0.2f, 17.2f, 0.02f);
    // plaza paving (NW) in two-tone tiles
    for (float x = -141; x < -11; x += 2.0f)
        for (float z = -86; z < -13; z += 2.0f) {
            int k = ((int)((x + 200) / 2) + (int)((z + 200) / 2)) & 1;
            if (k) setc(0.68f, 0.64f, 0.57f); else setc(0.6f, 0.56f, 0.5f);
            rectY(x, z, x + 2.0f, z + 2.0f, 0.012f);
        }
    // NE concrete yard and dirt construction patch
    setc(0.58f, 0.56f, 0.52f);
    rectY(11, -86, 141, -13, 0.012f);
    setc(0.55f, 0.44f, 0.32f);
    rectY(32, -42, 82, -16, 0.02f);
    // puddle-dark oil stains on the yard
    for (int i = 0; i < 25; i++) {
        float x = frange(15, 135), z = frange(-84, -15);
        setc(0.45f, 0.44f, 0.42f);
        rectY(x, z, x + frange(0.5f, 2), z + frange(0.5f, 2), 0.028f);
    }

    // sidewalks
    sidewalk(-400, 8, -7, 13, 1 | 8);
    sidewalk(7, 8, 400, 13, 1 | 4);
    sidewalk(-400, -13, -7, -8, 2 | 8);
    sidewalk(7, -13, 400, -8, 2 | 4);
    sidewalk(-11, 13, -7, 120, 8);
    sidewalk(7, 13, 11, 120, 4);
    sidewalk(-11, -400, -7, -13, 8);
    sidewalk(7, -400, 11, -13, 4);
}

// Road median: a painted concrete divider topped with a grindable railing.
static void medianSection(float x0, float x1) {
    setc(0.7f, 0.7f, 0.68f);
    box(x0, 0, -0.35f, x1, 0.3f, 0.35f);
    solidL(x0, -0.35f, x1, 0.35f, 0, 0.3f);
    kerbStripeZ(x0, x1, 0.354f, 0.3f);
    kerbStripeZ(x0, x1, -0.354f, 0.3f);
    setc(0.2f, 0.45f, 0.3f);
    for (float x = x0 + 0.3f; x < x1; x += 2.0f) box(x - 0.03f, 0.3f, -0.03f, x + 0.03f, 1.0f, 0.03f);
    setc(0.75f, 0.76f, 0.78f);
    cylBetween(V3(x0 + 0.1f, 1.0f, 0), V3(x1 - 0.1f, 1.0f, 0), 0.04f, 6);
    cylBetween(V3(x0 + 0.1f, 0.65f, 0), V3(x1 - 0.1f, 0.65f, 0), 0.025f, 5);
    solidL(x0, -0.06f, x1, 0.06f, 0.3f, 1.0f);
    if (x1 > WORLD_MIN_X && x0 < WORLD_MAX_X)
        railL(std::max(x0, WORLD_MIN_X) + 0.2f, 1.0f, 0, std::min(x1, WORLD_MAX_X) - 0.2f, 1.0f, 0, false);
}

static void buildMedian() {
    float starts[] = {16, 52, 88, 124, 160};
    for (int i = 0; i < 5; i++) {
        medianSection(starts[i], starts[i] + 32.0f);
        medianSection(-starts[i] - 32.0f, -starts[i]);
    }
}

// Street light. `out` points from the pole toward the road (+1 = +z, -1 = -z)
// when alongX, otherwise along x.
struct PolePt { V3 top; };
static std::vector<PolePt> g_poleTops;

static void streetLight(float x, float z, bool alongX, float out) {
    float y0 = SIDEWALK_H;
    setc(0.35f, 0.4f, 0.38f);
    gPush();
    gTranslate(x, y0, z);
    cyl(0.12f, 0.08f, 8.0f, 8, true);
    gPop();
    solidL(x - 0.15f, z - 0.15f, x + 0.15f, z + 0.15f, 0, 8.0f);
    V3 tip = alongX ? V3(x, y0 + 8.3f, z + out * 2.2f) : V3(x + out * 2.2f, y0 + 8.3f, z);
    cylBetween(V3(x, y0 + 7.6f, z), tip, 0.06f, 5);
    setc(0.3f, 0.32f, 0.3f);
    boxc(tip.x, tip.y - 0.2f, tip.z, 0.5f, 0.22f, 0.5f);
    gLighting(false);
    setc(1.0f, 0.95f, 0.75f);
    rectY(tip.x - 0.2f, tip.z - 0.2f, tip.x + 0.2f, tip.z + 0.2f, tip.y - 0.21f);
    gLighting(true);
    PolePt p;
    p.top = V3(x, y0 + 7.2f, z);
    g_poleTops.push_back(p);
    // posters taped around the base
    if (frand() < 0.5f) {
        setc(SHIRTS[irange(0, 9)]);
        gPush();
        gTranslate(x, y0 + 1.4f, z);
        gRotate(frange(0, 360), 0, 1, 0);
        rectZ(-0.13f, 0, 0.13f, 0.4f, 0.125f);
        gPop();
    }
}

// Sagging overhead cable between two points.
static void cable(const V3& a, const V3& b, float sag) {
    gLighting(false);
    setc(0.08f, 0.08f, 0.08f);
    gBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 12; i++) {
        float t = (float)i / 12.0f;
        V3 p = a + (b - a) * t;
        p.y -= sag * 4.0f * t * (1 - t);
        gVertex(p.x, p.y, p.z);
    }
    gEnd();
    gLighting(true);
}

static void palmTree(float x, float y0, float z, float h, bool solid) {
    float leanA = frange(0, TWO_PI), lean = frange(0.4f, 1.4f);
    V3 base(x, y0, z);
    V3 prev = base;
    const int segs = 9;
    for (int i = 1; i <= segs; i++) {
        float t = (float)i / (float)segs;
        V3 p = base + V3(cosf(leanA) * lean * t * t, h * t, sinf(leanA) * lean * t * t);
        float k = (i & 1) ? 1.0f : 0.88f;
        setc(0.5f * k, 0.4f * k, 0.28f * k);
        cylBetween(prev, p, 0.2f - 0.07f * t, 7);
        prev = p;
    }
    V3 top = prev;
    setc(0.4f, 0.26f, 0.12f);
    for (int i = 0; i < 4; i++) {
        gPush();
        gTranslate(top.x + cosf((float)i * 1.6f) * 0.18f, top.y - 0.25f, top.z + sinf((float)i * 1.6f) * 0.18f);
        sphere(0.14f, 7, 5);
        gPop();
    }
    const int fronds = 11;
    for (int f = 0; f < fronds; f++) {
        float a = (float)f * TWO_PI / (float)fronds + frange(-0.2f, 0.2f);
        float L = frange(2.6f, 3.4f);
        float rise = frange(0.2f, 0.9f);
        V3 dir(cosf(a), 0, sinf(a));
        V3 side(-dir.z, 0, dir.x);
        Col g = mixc(C(0.18f, 0.42f, 0.12f), C(0.45f, 0.55f, 0.18f), frand() * 0.5f);
        setc(g);
        V3 p0 = top;
        for (int s = 1; s <= 6; s++) {
            float t = (float)s / 6.0f;
            V3 p1 = top + dir * (L * t) + V3(0, rise * t - 2.2f * t * t, 0);
            float w0 = 0.55f * (1.0f - (float)(s - 1) / 6.0f) + 0.05f, w1 = 0.55f * (1.0f - t) + 0.05f;
            V3 d0 = V3(0, -0.25f, 0);
            quad4(p0, p1, p1 + side * w1 + d0, p0 + side * w0 + d0);
            quad4(p0, p0 - side * w0 + d0, p1 - side * w1 + d0, p1);
            p0 = p1;
        }
    }
    if (solid) solidL(x - 0.3f, z - 0.3f, x + 0.3f, z + 0.3f, y0, y0 + 4.0f);
}

// Gulmohar: a spreading tree with flame-red blossoms.
static void gulmoharTree(float x, float y0, float z, float h) {
    setc(0.36f, 0.26f, 0.18f);
    gPush();
    gTranslate(x, y0, z);
    cyl(0.3f, 0.2f, h * 0.55f, 8, false);
    gPop();
    V3 fork(x, y0 + h * 0.55f, z);
    for (int i = 0; i < 3; i++) {
        float a = (float)i * 2.1f + frand();
        cylBetween(fork, fork + V3(cosf(a) * 1.4f, h * 0.25f, sinf(a) * 1.4f), 0.14f, 6);
    }
    for (int i = 0; i < 6; i++) {
        float a = (float)i * 1.05f + frand() * 0.5f, r = i == 0 ? 0 : frange(1.0f, 1.9f);
        V3 c = fork + V3(cosf(a) * r, h * 0.3f + frange(0, 0.8f), sinf(a) * r);
        float rad = frange(1.3f, 1.9f);
        setc(mixc(C(0.16f, 0.38f, 0.12f), C(0.25f, 0.45f, 0.15f), frand()));
        gPush();
        gTranslate(c.x, c.y, c.z);
        gScale(1.0f, 0.7f, 1.0f);
        sphere(rad, 10, 7);
        gPop();
        for (int k = 0; k < 5; k++) {
            float u = frange(0, TWO_PI), v = frange(0.1f, 1.2f);
            setc(0.92f, frange(0.2f, 0.4f), 0.08f);
            gPush();
            gTranslate(c.x + cosf(u) * cosf(v) * rad * 0.95f, c.y + sinf(v) * rad * 0.66f,
                         c.z + sinf(u) * cosf(v) * rad * 0.95f);
            sphere(frange(0.25f, 0.45f), 6, 4);
            gPop();
        }
    }
    solidL(x - 0.35f, z - 0.35f, x + 0.35f, z + 0.35f, y0, y0 + 3.0f);
}

static void buildStreetFurniture() {
    // street lights on both sides of the main road
    std::vector<V3> north, south;
    for (float x = -196; x <= 196; x += 22.0f) {
        if (fabsf(x) < 14) continue;
        streetLight(x, 8.45f, true, -1);
        south.push_back(g_poleTops.back().top);
        streetLight(x + 11, -8.45f, true, 1);
        north.push_back(g_poleTops.back().top);
    }
    for (size_t i = 1; i < north.size(); i++) {
        if (north[i].x - north[i - 1].x < 30) cable(north[i - 1], north[i], 0.8f);
        if (south[i].x - south[i - 1].x < 30) cable(south[i - 1], south[i], 0.8f);
    }
    // a few cables slung across the road, very much the Mumbai look
    for (size_t i = 0; i + 1 < north.size(); i += 3) cable(north[i], south[i + 1], 1.2f);
    // lights along the cross road
    for (float z = -80; z <= 40; z += 22.0f) {
        if (fabsf(z) < 16) continue;
        streetLight(7.45f, z, false, -1);
        streetLight(-7.45f, z + 11, false, 1);
    }
    // trees: gulmohar on the south kerb, coconut palms on the north kerb
    for (float x = -185; x <= 185; x += 22.0f) {
        if (fabsf(x + 11) < 16) continue;
        gulmoharTree(x + 11, SIDEWALK_H, 8.9f, frange(4.5f, 5.5f));
    }
    for (float x = -174; x <= 185; x += 22.0f) {
        if (fabsf(x) < 16) continue;
        palmTree(x, SIDEWALK_H, -8.9f, frange(6.5f, 8.5f), true);
    }
    // traffic signals at the intersection corners
    struct { float x, z, yaw; int axis; } sig[4] = {
        {-7.6f, -8.6f, -PI * 0.5f, 0},  // faces -X, for +X traffic
        {7.6f, 8.6f, PI * 0.5f, 0},     // faces +X, for -X traffic
        {7.6f, -8.6f, PI, 1},           // faces -Z, for +Z traffic
        {-7.6f, 8.6f, 0.0f, 1},         // faces +Z, for -Z traffic
    };
    for (int i = 0; i < 4; i++) {
        setc(0.15f, 0.15f, 0.15f);
        gPush();
        gTranslate(sig[i].x, SIDEWALK_H, sig[i].z);
        cyl(0.09f, 0.09f, 4.0f, 8, true);
        gRotate(sig[i].yaw * RAD2DEG, 0, 1, 0);
        setc(0.1f, 0.1f, 0.1f);
        box(-0.22f, 3.0f, 0.05f, 0.22f, 4.1f, 0.4f);
        setc(0.95f, 0.8f, 0.1f);
        box(-0.24f, 2.98f, 0.04f, 0.24f, 3.0f, 0.41f);
        gPop();
        solidL(sig[i].x - 0.12f, sig[i].z - 0.12f, sig[i].x + 0.12f, sig[i].z + 0.12f, 0, 4.0f);
        Signal s = {sig[i].x, sig[i].z, sig[i].yaw, sig[i].axis};
        g_signals.push_back(s);
    }
}

// ---------------------------------------------------------------- buildings

static const Col WALLS[9] = {{0.93f, 0.86f, 0.72f}, {0.82f, 0.58f, 0.52f}, {0.56f, 0.73f, 0.7f},
                             {0.94f, 0.8f, 0.48f},  {0.76f, 0.76f, 0.78f}, {0.62f, 0.68f, 0.86f},
                             {0.86f, 0.67f, 0.47f}, {0.9f, 0.9f, 0.86f},   {0.72f, 0.82f, 0.62f}};

static const char* SHOP_NAMES[] = {"SHREE GANESH STORES", "MEDICAL", "IRANI CAFE", "BOMBAY TAILORS",
                                   "MOBILE REPAIR", "SWEET MART", "CYBER CAFE", "HAIR SALOON",
                                   "LAUNDRY", "HARDWARE", "SAREE CENTRE", "PHOTO STUDIO",
                                   "BAKERY", "XEROX", "OPTICIANS", "JEWELLERS", "CHEMIST", "FOOTWEAR"};
static const int SHOP_COUNT = 18;

struct SignStyle { Col board, text; };
static const SignStyle SIGNS[6] = {{{0.8f, 0.1f, 0.1f}, {1.0f, 0.9f, 0.2f}},   {{0.1f, 0.25f, 0.65f}, {1, 1, 1}},
                                   {{0.1f, 0.5f, 0.25f}, {1, 1, 0.9f}},        {{0.98f, 0.82f, 0.1f}, {0.7f, 0.05f, 0.05f}},
                                   {{0.95f, 0.95f, 0.92f}, {0.1f, 0.2f, 0.6f}}, {{0.55f, 0.1f, 0.45f}, {1, 1, 1}}};

static const char* POSTER_WORDS[] = {"FILM", "SALE", "YOGA", "TUITION", "CIRCUS", "ROOM", "JOBS",
                                     "DANCE", "GYM", "MELA", "CRICKET", "CONCERT", "CLASSES", "LOAN"};

// Poster in the XY plane facing +Z, centred at (cx, cy).
static void poster(float cx, float cy, float w, float h, float z) {
    Col bg = SHIRTS[irange(0, 9)];
    setc(bg);
    rectZ(cx - w / 2, cy - h / 2, cx + w / 2, cy + h / 2, z);
    setc(mulc(SAREES[irange(0, 7)], 0.9f));
    if (frand() < 0.5f) discZ(cx, cy + h * 0.1f, std::min(w, h) * 0.28f, z + 0.002f, 12);
    else rectZ(cx - w * 0.35f, cy - h * 0.1f, cx + w * 0.35f, cy + h * 0.35f, z + 0.002f);
    gPush();
    gTranslate(cx, cy - h * 0.38f, z + 0.004f);
    text3DFit(POSTER_WORDS[irange(0, 13)], h * 0.16f, w * 0.85f, bg.r + bg.g + bg.b > 1.8f ? COL_BLACK : COL_WHITE);
    gPop();
}

// Cluster of overlapping posters on a wall region.
static void posterWall(float x0, float x1, float y0, float y1, float z) {
    int n = (int)((x1 - x0) * 0.8f);
    for (int i = 0; i < n; i++) {
        float w = frange(0.5f, 0.9f), h = w * frange(1.2f, 1.5f);
        poster(frange(x0 + w / 2, x1 - w / 2), frange(y0 + h / 2, y1 - h / 2), w, h, z + 0.006f * (float)i);
    }
}

static void signBoard(float x0, float x1, float y0, float y1, float z, const char* name, const SignStyle& st) {
    setc(st.board);
    box(x0, y0, z, x1, y1, z + 0.14f);
    setc(mulc(st.board, 0.7f));
    box(x0 - 0.04f, y0 - 0.04f, z - 0.01f, x1 + 0.04f, y0, z + 0.16f);
    gPush();
    gTranslate((x0 + x1) * 0.5f, y0 + (y1 - y0) * 0.24f, z + 0.15f);
    text3DFit(name, (y1 - y0) * 0.55f, (x1 - x0) * 0.9f, st.text);
    gPop();
}

// Shelves of colourful goods seen through an open shopfront.
static void shopGoods(float a, float b, float z, float gh, bool kirana) {
    setc(0.12f, 0.1f, 0.09f);
    rectZ(a, 0, b, gh - 0.1f, z);
    setc(0.45f, 0.3f, 0.18f);
    for (int s = 0; s < 3; s++) box(a + 0.1f, 0.55f + (float)s * 0.7f, z, b - 0.1f, 0.6f + (float)s * 0.7f, z + 0.35f);
    for (int s = 0; s < 3; s++)
        for (float x = a + 0.2f; x < b - 0.3f; x += frange(0.22f, 0.4f)) {
            setc(mixc(SHIRTS[irange(0, 9)], SAREES[irange(0, 7)], 0.5f));
            float hgt = frange(0.18f, 0.45f);
            box(x, 0.6f + (float)s * 0.7f, z + 0.03f, x + 0.18f, 0.6f + (float)s * 0.7f + hgt, z + 0.3f);
        }
    if (kirana) {
        // sacks of rice and dal on the step, strips of snack packets
        solidL(a + 0.08f, z + 0.78f, b - 0.08f, z + 1.22f, 0, 0.55f);  // the row of sacks
        for (float x = a + 0.3f; x < b - 0.3f; x += 0.55f) {
            setc(0.88f, 0.84f, 0.7f);
            gPush();
            gTranslate(x, 0, z + 1.0f);
            cyl(0.22f, 0.2f, 0.55f, 8, true);
            setc(frand() < 0.5f ? C(0.95f, 0.8f, 0.3f) : C(0.8f, 0.5f, 0.2f));
            gTranslate(0, 0.551f, 0);
            discY(0.19f, 0, 8);
            gPop();
        }
        for (float x = a + 0.2f; x < b - 0.1f; x += 0.3f) {
            setc(SHIRTS[irange(0, 9)]);
            rectZ(x, gh - 1.4f, x + 0.2f, gh - 0.3f, z + 1.45f);
        }
    }
}

static void waterTank(float x, float y, float z) {
    setc(0.08f, 0.08f, 0.09f);
    gPush();
    gTranslate(x, y, z);
    cyl(0.75f, 0.75f, 1.3f, 12, true);
    gTranslate(0, 1.3f, 0);
    cyl(0.75f, 0.2f, 0.25f, 12, true);
    gPop();
}

static void laundry(float x0, float x1, float y, float z) {
    gLighting(false);
    setc(0.2f, 0.2f, 0.2f);
    line3(V3(x0, y, z), V3(x1, y, z));
    gLighting(true);
    for (float x = x0 + 0.1f; x < x1 - 0.3f; x += frange(0.35f, 0.6f)) {
        setc(frand() < 0.5f ? SHIRTS[irange(0, 9)] : SAREES[irange(0, 7)]);
        rectZ(x, y - frange(0.4f, 0.8f), x + frange(0.25f, 0.4f), y, z);
    }
}

static const char* HOARDINGS[][2] = {{"DRINK MORE CHAI", "CUTTING SE CHALEGA"}, {"AAMCHI MUMBAI", "SKATE FEST 2026"},
                                     {"KAMAL SAREES", "WEDDING SALE"},         {"FRESH PAV DAILY", "SINCE 1932"},
                                     {"BOMBAY TALKIES", "NOW SHOWING"},        {"SKATE OR CHAI", "WHY NOT BOTH"}};

// Hoarding on a roof, facing +Z.
static void hoarding(float cx, float y, float z, float w) {
    const char** h = HOARDINGS[irange(0, 5)];
    setc(0.25f, 0.25f, 0.27f);
    box(cx - w * 0.35f - 0.1f, y, z - 0.1f, cx - w * 0.35f + 0.1f, y + 2.0f, z + 0.1f);
    box(cx + w * 0.35f - 0.1f, y, z - 0.1f, cx + w * 0.35f + 0.1f, y + 2.0f, z + 0.1f);
    SignStyle st = SIGNS[irange(0, 5)];
    setc(st.board);
    box(cx - w / 2, y + 2.0f, z, cx + w / 2, y + 5.0f, z + 0.2f);
    gPush();
    gTranslate(cx, y + 3.6f, z + 0.21f);
    text3DFit(h[0], 0.9f, w * 0.9f, st.text);
    gTranslate(0, -1.1f, 0);
    text3DFit(h[1], 0.5f, w * 0.8f, mulc(st.text, 0.9f));
    gPop();
}

// Building in local frame: front at z=0 facing +Z, body back to z=-d, x from 0 to w.
// The ground floor is an arcade of shops set back 1.6 m under the upper floors.
static const char* CHAWL_NAMES[] = {"SHANTI CHAWL", "GANESH NIWAS", "LAXMI CHAWL", "SAI KRUPA", "NAV JEEVAN",
                                    "PARVATI BHAVAN", "OM SHANTI", "JAI HIND CHAWL"};
static int g_chawlCount = 0;

static void building(float w, float d, int floors, const char* forcedShop, int chawl) {
    const float gh = 3.8f, fh = 3.2f;
    if (chawl) {
        char nm[64];
        snprintf(nm, sizeof(nm), "ROOM %d, %s", 4 + (g_chawlCount * 7) % 38, CHAWL_NAMES[g_chawlCount % 8]);
        g_chawlCount++;
        addDestL(nm, w * 0.5f, 1.3f, SIDEWALK_H, 0);
    }
    float H = gh + (float)(floors - 1) * fh;
    Col wall = WALLS[irange(0, 8)];
    Col trim = mulc(wall, 0.78f);
    // masses
    setc(mulc(wall, 0.9f));
    box(0, 0, -d, w, gh, -1.6f);
    setc(wall);
    box(0, gh, -d, w, H, 0);
    solidL(0, -d, w, -1.6f, 0, H);
    // arcade floor, level with the sidewalk
    setc(0.58f, 0.55f, 0.5f);
    box(0, 0, -1.6f, w, SIDEWALK_H, 0);
    solidL(0, -1.6f, w, 0, 0, SIDEWALK_H);
    // shops
    int nShops = std::max(1, (int)(w / 4.6f));
    float sw = w / (float)nShops;
    for (int i = 0; i <= nShops; i++) {
        float x = std::min(std::max((float)i * sw, 0.25f), w - 0.25f);
        setc(trim);
        box(x - 0.25f, 0, -1.6f, x + 0.25f, gh, 0);
        solidL(x - 0.25f, -1.6f, x + 0.25f, 0, 0, gh);
    }
    for (int i = 0; i < nShops; i++) {
        float a = (float)i * sw + 0.25f, b = (float)(i + 1) * sw - 0.25f;
        const char* name = (i == 0 && forcedShop) ? forcedShop : SHOP_NAMES[irange(0, SHOP_COUNT - 1)];
        bool kirana = strcmp(name, "KIRANA STORE") == 0;
        bool open = kirana || frand() < 0.72f;
        if (open) addDestL(name, (a + b) * 0.5f, 1.3f, SIDEWALK_H, 0);
        if (open) {
            shopGoods(a, b, -1.59f, gh, kirana);
            setc(0.5f, 0.52f, 0.55f);
            box(a, gh - 0.75f, -1.62f, b, gh - 0.05f, -1.52f);
            setc(0.42f, 0.3f, 0.2f);
            box(a + 0.3f, 0, -1.3f, b - 0.3f, 0.9f, -1.0f);
            solidL(a + 0.3f, -1.3f, b - 0.3f, -1.0f, 0, 0.9f);
        } else {
            setc(0.55f, 0.56f, 0.58f);
            rectZ(a, 0, b, gh - 0.1f, -1.58f);
            gLighting(false);
            setc(0.42f, 0.43f, 0.45f);
            gBegin(GL_LINES);
            for (float y = 0.2f; y < gh - 0.1f; y += 0.18f) { gVertex(a, y, -1.575f); gVertex(b, y, -1.575f); }
            gEnd();
            gLighting(true);
            if (frand() < 0.6f) posterWall(a + 0.2f, b - 0.2f, 0.8f, 2.4f, -1.57f);
        }
        signBoard(a, b, gh + 0.05f, gh + 0.95f, 0.0f, name, kirana ? SIGNS[3] : SIGNS[irange(0, 5)]);
        // blue tarp awning over some shops
        if (frand() < 0.45f) {
            setc(frand() < 0.7f ? C(0.15f, 0.35f, 0.75f) : C(0.85f, 0.45f, 0.1f));
            quad4(V3(a, gh - 0.1f, 0.15f), V3(b, gh - 0.1f, 0.15f), V3(b, gh - 0.7f, 1.5f), V3(a, gh - 0.7f, 1.5f));
        }
    }
    // upper floors
    for (int f = 1; f < floors; f++) {
        float y0 = gh + (float)(f - 1) * fh;
        setc(trim);
        box(0, y0 - 0.1f, 0, w, y0 + 0.06f, 0.12f);
        if (chawl) {
            // continuous gallery with doors and laundry, the classic chawl front
            setc(mulc(wall, 0.85f));
            box(0, y0 + 0.02f, 0, w, y0 + 0.16f, 1.3f);
            setc(0.3f, 0.45f, 0.4f);
            box(0, y0 + 0.95f, 1.25f, w, y0 + 1.02f, 1.32f);
            for (float x = 0.3f; x < w; x += 0.5f) box(x, y0 + 0.16f, 1.27f, x + 0.04f, y0 + 0.95f, 1.3f);
            for (float x = 0.8f; x < w - 1.0f; x += 2.6f) {
                setc(frand() < 0.5f ? C(0.25f, 0.35f, 0.55f) : C(0.45f, 0.3f, 0.2f));
                rectZ(x, y0 + 0.16f, x + 1.0f, y0 + 2.3f, 0.01f);
                setc(0.15f, 0.18f, 0.22f);
                rectZ(x + 1.3f, y0 + 1.0f, x + 2.1f, y0 + 2.0f, 0.01f);
            }
            laundry(0.4f, w - 0.4f, y0 + 2.5f, 1.1f);
            continue;
        }
        int nW = std::max(1, (int)(w / 2.7f));
        for (int j = 0; j < nW; j++) {
            float cx = ((float)j + 0.5f) * w / (float)nW;
            setc(trim);
            box(cx - 0.72f, y0 + 0.8f, 0, cx + 0.72f, y0 + 2.45f, 0.07f);
            float r = frand();
            if (r < 0.6f) setc(0.16f, 0.2f, 0.26f);
            else if (r < 0.8f) setc(0.25f, 0.45f, 0.45f);
            else setc(0.55f, 0.35f, 0.2f);
            rectZ(cx - 0.6f, y0 + 0.9f, cx + 0.6f, y0 + 2.35f, 0.075f);
            if (frand() < 0.5f) {
                gLighting(false);
                setc(0.2f, 0.2f, 0.2f);
                gBegin(GL_LINES);
                for (float gx = cx - 0.5f; gx <= cx + 0.51f; gx += 0.2f) {
                    gVertex(gx, y0 + 0.9f, 0.09f);
                    gVertex(gx, y0 + 2.35f, 0.09f);
                }
                gEnd();
                gLighting(true);
            }
            float br = frand();
            if (br < 0.3f) {
                setc(mulc(wall, 0.8f));
                box(cx - 1.1f, y0 + 0.05f, 0, cx + 1.1f, y0 + 0.2f, 0.95f);
                setc(0.3f, 0.3f, 0.32f);
                box(cx - 1.1f, y0 + 1.0f, 0.9f, cx + 1.1f, y0 + 1.06f, 0.95f);
                for (float x = cx - 1.05f; x < cx + 1.1f; x += 0.3f) box(x, y0 + 0.2f, 0.9f, x + 0.03f, y0 + 1.0f, 0.93f);
                if (frand() < 0.6f) laundry(cx - 1.0f, cx + 1.0f, y0 + 2.2f, 0.85f);
                if (frand() < 0.4f) {
                    setc(0.2f, 0.5f, 0.2f);
                    for (int k = 0; k < 3; k++) {
                        gPush();
                        gTranslate(cx - 0.8f + (float)k * 0.5f, y0 + 1.15f, 0.8f);
                        sphere(0.18f, 6, 5);
                        gPop();
                    }
                }
            } else if (br < 0.55f) {
                setc(0.85f, 0.86f, 0.84f);
                box(cx + 0.78f, y0 + 1.0f, 0, cx + 1.35f, y0 + 1.45f, 0.45f);
            }
            // monsoon stains under the sill
            if (frand() < 0.5f) {
                setc(mulc(wall, 0.84f));
                rectZ(cx - 0.4f, y0 + 0.1f, cx + 0.3f, y0 + 0.8f, 0.004f);
            }
        }
    }
    // roof
    setc(trim);
    box(0, H, -0.25f, w, H + 0.8f, 0);
    box(0, H, -d, w, H + 0.8f, -d + 0.25f);
    box(0, H, -d, 0.25f, H + 0.8f, 0);
    box(w - 0.25f, H, -d, w, H + 0.8f, 0);
    setc(0.45f, 0.43f, 0.4f);
    rectY(0.25f, -d + 0.25f, w - 0.25f, -0.25f, H + 0.02f);
    int tanks = irange(1, 3);
    for (int i = 0; i < tanks; i++) waterTank(frange(1.2f, w - 1.2f), H, frange(-d + 1.5f, -2.0f));
    if (frand() < 0.4f) {
        setc(mulc(wall, 0.85f));
        box(w * 0.6f, H, -d * 0.6f, w * 0.6f + 2.5f, H + 2.6f, -d * 0.6f + 2.5f);
    }
    if (frand() < 0.3f && w > 10) hoarding(w * 0.5f, H + 0.8f, -d * 0.4f, std::min(w - 1.0f, 12.0f));
    // back wall: plain windows, AC units and a drain pipe, seen from lanes and yards
    for (int f = 1; f < floors; f++) {
        float y0 = gh + (float)(f - 1) * fh;
        for (float x = 1.2f; x < w - 1.4f; x += 3.0f) {
            setc(0.2f, 0.23f, 0.28f);
            rectZ(x, y0 + 0.9f, x + 1.0f, y0 + 2.2f, -d - 0.01f);
            if (frand() < 0.25f) {
                setc(0.85f, 0.86f, 0.84f);
                box(x + 0.1f, y0 + 0.4f, -d - 0.45f, x + 0.8f, y0 + 0.85f, -d);
            }
        }
    }
    setc(0.42f, 0.42f, 0.4f);
    box(w * 0.5f, 0, -d - 0.12f, w * 0.5f + 0.12f, H, -d);
    // posters and stencil on the side walls, low down where people paste them
    for (int side = 0; side < 2; side++) {
        gPush();
        if (side == 0) { gTranslate(0, 0, -d); gRotate(-90, 0, 1, 0); }
        else { gTranslate(w, 0, 0); gRotate(90, 0, 1, 0); }
        if (frand() < 0.7f) posterWall(1.8f, d - 1.0f, 1.0f, 3.0f, 0.01f);
        for (int f = 1; f < floors; f++) {
            float y0 = gh + (float)(f - 1) * fh;
            for (float x = 1.5f; x < d - 1.5f; x += 3.2f) {
                if (frand() < 0.5f) continue;
                setc(0.2f, 0.23f, 0.28f);
                rectZ(x, y0 + 0.9f, x + 0.9f, y0 + 2.1f, 0.01f);
            }
        }
        if (frand() < 0.4f) {
            gPush();
            gTranslate(d * 0.5f, 3.4f, 0.03f);
            text3D("STICK NO BILLS", 0.3f, C(0.9f, 0.9f, 0.9f));
            gPop();
        }
        gPop();
    }
}

// Row of buildings along a street edge. Local frame rotation and origin come
// from the caller through pushXf; this lays buildings from local x=0 to x=len.
static void buildingRow(float length, float depthMin, float depthMax, const char* firstShop) {
    float x = 0;
    bool first = true;
    while (x < length - 5.0f) {
        float w = std::min(frange(11.0f, 18.0f), length - x);
        float d = frange(depthMin, depthMax);
        int floors = irange(2, 5);
        pushXf(x, 0, 0);
        building(w, d, floors, first ? firstShop : nullptr, frand() < 0.3f ? 1 : 0);
        popXf();
        first = false;
        x += w + (frand() < 0.25f ? 1.6f : 0.0f);
    }
}

// ---------------------------------------------------------------- street stalls
// Local frame: centred on x=0, back at z=0, facing +Z toward the road.

static void tarpCanopy(float w, float d, float y0, const Col& c) {
    setc(0.6f, 0.45f, 0.25f);
    cylBetween(V3(-w / 2, y0, 0.05f), V3(-w / 2, y0 + 2.4f, 0.05f), 0.035f, 5);
    cylBetween(V3(w / 2, y0, 0.05f), V3(w / 2, y0 + 2.4f, 0.05f), 0.035f, 5);
    cylBetween(V3(-w / 2, y0, d), V3(-w / 2, y0 + 2.15f, d), 0.035f, 5);
    cylBetween(V3(w / 2, y0, d), V3(w / 2, y0 + 2.15f, d), 0.035f, 5);
    setc(c);
    quad4(V3(-w / 2 - 0.1f, y0 + 2.45f, -0.1f), V3(w / 2 + 0.1f, y0 + 2.45f, -0.1f),
          V3(w / 2 + 0.1f, y0 + 2.15f, d + 0.2f), V3(-w / 2 - 0.1f, y0 + 2.15f, d + 0.2f));
    for (int s = -1; s <= 1; s += 2) {
        solidL((float)s * w / 2 - 0.06f, 0.0f, (float)s * w / 2 + 0.06f, 0.1f, y0, y0 + 2.4f);
        solidL((float)s * w / 2 - 0.06f, d - 0.05f, (float)s * w / 2 + 0.06f, d + 0.05f, y0, y0 + 2.15f);
    }
    solidL(-w / 2 - 0.1f, -0.1f, w / 2 + 0.1f, d + 0.2f, y0 + 2.15f, y0 + 2.45f);  // head room under the tarp
}

static void chaiStall(float y0) {
    setc(0.3f, 0.55f, 0.75f);
    box(-1.0f, y0, 0.35f, 1.0f, y0 + 1.0f, 1.2f);
    setc(COL_STEEL);
    box(-1.05f, y0 + 1.0f, 0.3f, 1.05f, y0 + 1.06f, 1.25f);
    solidL(-1.05f, 0.3f, 1.05f, 1.25f, y0, y0 + 1.06f);
    // stove and kettle
    setc(0.2f, 0.2f, 0.2f);
    box(0.2f, y0 + 1.06f, 0.55f, 0.7f, y0 + 1.2f, 0.95f);
    setc(0.75f, 0.72f, 0.6f);
    gPush();
    gTranslate(0.45f, y0 + 1.2f, 0.75f);
    cyl(0.15f, 0.12f, 0.24f, 10, true);
    gTranslate(0, 0.27f, 0);
    sphere(0.06f, 6, 4);
    gPop();
    cylBetween(V3(0.58f, y0 + 1.3f, 0.75f), V3(0.72f, y0 + 1.42f, 0.75f), 0.02f, 4);
    // little glasses of cutting chai
    for (int i = 0; i < 6; i++) {
        setc(0.7f, 0.45f, 0.25f);
        gPush();
        gTranslate(-0.8f + (float)i * 0.13f, y0 + 1.06f, 1.08f);
        cyl(0.035f, 0.04f, 0.09f, 6, true);
        gPop();
    }
    tarpCanopy(2.6f, 1.6f, y0, C(0.12f, 0.3f, 0.7f));
    signBoard(-0.95f, 0.95f, y0 + 1.72f, y0 + 2.12f, 1.62f, "CHAI", SIGNS[0]);
    gPush();
    gTranslate(0, y0 + 0.45f, 1.21f);
    text3DFit("CUTTING 10/-", 0.18f, 1.7f, COL_WHITE);
    gPop();
    // bench for customers, grindable
    setc(COL_WOOD);
    ledgeBoxL(1.5f, 0.55f, 3.3f, 1.0f, y0, y0 + 0.45f);
    addVendorL(-0.2f, 0.12f, y0);
    addStallL(STALL_CHAI, y0, 0.45f, 0.75f);
}

static void handCart(float y0, const Col& c) {
    setc(c);
    box(-0.9f, y0 + 0.6f, 0.3f, 0.9f, y0 + 0.95f, 1.3f);
    setc(COL_STEEL);
    box(-0.95f, y0 + 0.95f, 0.25f, 0.95f, y0 + 1.0f, 1.35f);
    wheelX(-0.95f, y0 + 0.35f, 0.8f, 0.35f, 0.06f, 0);
    wheelX(0.95f, y0 + 0.35f, 0.8f, 0.35f, 0.06f, 0);
    setc(COL_WOOD);
    cylBetween(V3(-0.9f, y0 + 0.62f, 0.4f), V3(-0.9f, y0, 0.4f), 0.03f, 4);
    cylBetween(V3(0.9f, y0 + 0.62f, 0.4f), V3(0.9f, y0, 0.4f), 0.03f, 4);
    solidL(-1.0f, 0.25f, 1.0f, 1.35f, y0, y0 + 1.0f);
}

static void umbrella(float x, float y0, float z, float r) {
    setc(0.3f, 0.3f, 0.3f);
    cylBetween(V3(x, y0, z), V3(x, y0 + 2.4f, z), 0.03f, 4);
    solidL(x - 0.05f, z - 0.05f, x + 0.05f, z + 0.05f, y0, y0 + 2.4f);
    solidL(x - r * 0.7f, z - r * 0.7f, x + r * 0.7f, z + r * 0.7f, y0 + 2.05f, y0 + 2.5f);
    const int n = 12;
    for (int i = 0; i < n; i++) {
        float a0 = TWO_PI * (float)i / (float)n, a1 = TWO_PI * (float)(i + 1) / (float)n;
        setc((i & 1) ? C(0.95f, 0.85f, 0.2f) : C(0.85f, 0.15f, 0.15f));
        tri3(V3(x, y0 + 2.5f, z), V3(x + cosf(a0) * r, y0 + 2.05f, z + sinf(a0) * r),
             V3(x + cosf(a1) * r, y0 + 2.05f, z + sinf(a1) * r));
    }
}

static void vadaPavStall(float y0) {
    handCart(y0, C(0.85f, 0.45f, 0.1f));
    setc(0.1f, 0.1f, 0.1f);
    gPush();
    gTranslate(0.35f, y0 + 1.0f, 0.8f);
    cyl(0.18f, 0.38f, 0.18f, 12, false);  // kadai
    setc(0.75f, 0.55f, 0.15f);
    gTranslate(0, 0.12f, 0);
    discY(0.32f, 0, 12);
    gPop();
    for (int i = 0; i < 9; i++) {
        setc(0.85f, 0.6f, 0.15f);
        gPush();
        gTranslate(-0.65f + (float)(i % 3) * 0.12f, y0 + 1.05f + (float)(i / 3) * 0.05f, 0.6f + (float)(i % 2) * 0.12f);
        sphere(0.06f, 6, 4);
        gPop();
    }
    setc(0.9f, 0.75f, 0.5f);
    for (int i = 0; i < 4; i++) box(-0.4f + (float)i * 0.13f, y0 + 1.0f, 1.0f, -0.3f + (float)i * 0.13f, y0 + 1.12f, 1.2f);
    umbrella(0, y0, 0.3f, 1.4f);
    signBoard(-0.8f, 0.8f, y0 + 0.62f, y0 + 0.92f, 1.31f, "VADA PAV 15/-", SIGNS[3]);
    addVendorL(0.0f, 0.0f, y0);
    addStallL(STALL_VADAPAV, y0, 0.35f, 0.8f);
}

static void bhelStall(float y0) {
    handCart(y0, C(0.2f, 0.55f, 0.3f));
    for (int i = 0; i < 5; i++) {
        setc(SAREES[i]);
        gPush();
        gTranslate(-0.7f + (float)i * 0.35f, y0 + 1.0f, 0.8f);
        cyl(0.13f, 0.15f, 0.18f, 8, true);
        gPop();
    }
    umbrella(0, y0, 0.3f, 1.3f);
    signBoard(-0.8f, 0.8f, y0 + 0.62f, y0 + 0.92f, 1.31f, "BHEL PURI", SIGNS[1]);
    addVendorL(0.0f, 0.0f, y0);
    addStallL(STALL_BHEL, y0, 0, 0.8f);
}

static void fruitStall(float y0) {
    handCart(y0, C(0.5f, 0.35f, 0.2f));
    for (int layer = 0; layer < 3; layer++)
        for (int i = 0; i < 6 - layer * 2; i++)
            for (int j = 0; j < 3 - layer; j++) {
                setc(mixc(C(0.98f, 0.7f, 0.1f), C(0.9f, 0.45f, 0.1f), frand()));
                gPush();
                gTranslate(-0.6f + (float)i * 0.22f + (float)layer * 0.2f, y0 + 1.08f + (float)layer * 0.14f,
                             0.5f + (float)j * 0.22f + (float)layer * 0.1f);
                gScale(1.0f, 0.85f, 1.0f);
                sphere(0.1f, 6, 5);
                gPop();
            }
    tarpCanopy(2.2f, 1.5f, y0, C(0.9f, 0.5f, 0.1f));
    signBoard(-0.8f, 0.8f, y0 + 0.62f, y0 + 0.92f, 1.31f, "HAPUS AAMBA", SIGNS[2]);
    addVendorL(0.0f, 0.05f, y0);
    addStallL(STALL_FRUIT, y0, 0, 0.8f);
}

// Cart with an umbrella, used for the sea face snacks.
static void snackCart(float y0, const Col& c, const char* sign, int kind) {
    handCart(y0, c);
    if (kind == STALL_BHUTTA) {
        // glowing coals and corn cobs
        gLighting(false);
        setc(1.0f, 0.45f, 0.1f);
        box(-0.5f, y0 + 1.0f, 0.55f, 0.5f, y0 + 1.04f, 1.05f);
        gLighting(true);
        for (int i = 0; i < 5; i++) {
            setc(0.95f, 0.8f, 0.25f);
            cylBetween(V3(-0.4f + (float)i * 0.2f, y0 + 1.08f, 0.6f), V3(-0.4f + (float)i * 0.2f, y0 + 1.08f, 1.0f), 0.04f, 6);
        }
    } else {
        for (int i = 0; i < 6; i++) {
            setc(SAREES[(i * 3) % 8]);
            gPush();
            gTranslate(-0.6f + (float)i * 0.24f, y0 + 1.0f, 0.8f);
            cyl(0.07f, 0.07f, 0.28f, 8, true);
            gPop();
        }
    }
    umbrella(0, y0, 0.3f, 1.35f);
    signBoard(-0.8f, 0.8f, y0 + 0.62f, y0 + 0.92f, 1.31f, sign, SIGNS[kind == STALL_BHUTTA ? 3 : 1]);
    addVendorL(0.0f, 0.0f, y0);
    addStallL(kind, y0, 0, 0.8f);
}

// Paan tapri: a narrow painted kiosk with a paanwala inside.
static void paanShop(float y0) {
    setc(0.15f, 0.5f, 0.3f);
    box(-0.8f, y0, 0, 0.8f, y0 + 2.3f, 0.2f);
    box(-0.8f, y0, 0, -0.65f, y0 + 2.3f, 1.4f);
    box(0.65f, y0, 0, 0.8f, y0 + 2.3f, 1.4f);
    box(-0.8f, y0 + 2.3f, 0, 0.8f, y0 + 2.45f, 1.5f);
    box(-0.8f, y0, 1.2f, 0.8f, y0 + 0.95f, 1.4f);
    solidL(-0.8f, 0, 0.8f, 1.4f, y0, y0 + 2.45f);
    setc(0.2f, 0.12f, 0.08f);
    rectZ(-0.65f, y0 + 0.95f, 0.65f, y0 + 2.3f, 0.21f);
    setc(COL_STEEL);
    box(-0.65f, y0 + 0.95f, 1.0f, 0.65f, y0 + 1.0f, 1.45f);
    for (int i = 0; i < 6; i++) {
        setc(0.2f, 0.6f, 0.15f);
        gPush();
        gTranslate(-0.45f + (float)i * 0.16f, y0 + 1.01f, 1.25f);
        gScale(1, 0.2f, 1);
        sphere(0.07f, 6, 3);
        gPop();
    }
    // strings of mouth freshener sachets hanging at the front
    for (float x = -0.6f; x < 0.6f; x += 0.1f) {
        setc(frand() < 0.5f ? C(0.85f, 0.85f, 0.9f) : SHIRTS[irange(0, 9)]);
        rectZ(x, y0 + 1.5f, x + 0.07f, y0 + 2.25f, 1.41f);
    }
    signBoard(-0.9f, 0.9f, y0 + 2.45f, y0 + 2.95f, 1.2f, "BANARASI PAAN", SIGNS[0]);
    addVendorL(0.0f, 0.55f, y0 + 0.2f);
    addStallL(STALL_PAAN, y0, 0, 1.0f);
}

// BEST bus shelter with a grindable bench.
static void busStop(float y0) {
    setc(0.6f, 0.62f, 0.64f);
    for (int s = -1; s <= 1; s += 2) {
        box((float)s * 2.8f - 0.06f, y0, 1.55f, (float)s * 2.8f + 0.06f, y0 + 2.55f, 1.67f);
        box((float)s * 2.8f - 0.06f, y0, 0.05f, (float)s * 2.8f + 0.06f, y0 + 2.55f, 0.17f);
        solidL((float)s * 2.8f - 0.08f, 1.5f, (float)s * 2.8f + 0.08f, 1.7f, y0, y0 + 2.55f);
    }
    setc(0.75f, 0.12f, 0.1f);
    box(-3.1f, y0 + 2.55f, -0.05f, 3.1f, y0 + 2.7f, 1.95f);
    setc(0.85f, 0.85f, 0.82f);
    box(-2.8f, y0 + 0.4f, 0.05f, 2.8f, y0 + 2.4f, 0.12f);
    solidL(-2.8f, 0.0f, 2.8f, 0.15f, y0, y0 + 2.4f);
    posterWall(-2.4f, -0.4f, y0 + 0.9f, y0 + 2.2f, 0.125f);
    setc(0.1f, 0.2f, 0.5f);
    rectZ(0.2f, y0 + 0.9f, 2.5f, y0 + 2.2f, 0.125f);
    gPush();
    gTranslate(1.35f, y0 + 1.7f, 0.13f);
    text3DFit("ROUTES 83 84 86 124", 0.14f, 2.1f, COL_WHITE);
    gTranslate(0, -0.35f, 0);
    text3DFit("BEST UNDERTAKING", 0.16f, 2.1f, COL_YELLOW);
    gPop();
    setc(COL_STEEL);
    ledgeBoxL(-2.3f, 0.3f, 2.3f, 0.75f, y0, y0 + 0.45f);
    // round BEST sign on a pole
    setc(0.3f, 0.3f, 0.3f);
    cylBetween(V3(3.4f, y0, 1.6f), V3(3.4f, y0 + 3.0f, 1.6f), 0.05f, 6);
    solidL(3.3f, 1.5f, 3.5f, 1.7f, y0, y0 + 3.0f);
    setc(0.8f, 0.1f, 0.08f);
    discZ(3.4f, y0 + 3.1f, 0.45f, 1.62f, 16);
    gPush();
    gTranslate(3.4f, y0 + 2.98f, 1.64f);
    text3D("BEST", 0.22f, COL_WHITE);
    gPop();
}

