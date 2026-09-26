#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// City builder, part 2: skate spots, railway, station, construction site,
// mural wall and the far skyline.
// ============================================================================

// Wedge ramp. axis says which way it rises (see Solid::axis).
static void wedge(float x0, float z0, float x1, float z1, float yLow, float yHigh, int axis, const Col& top,
                  const Col& side, bool planks) {
    auto h = [&](float x, float z) {
        float t;
        switch (axis) {
            case 1: t = (x - x0) / (x1 - x0); break;
            case -1: t = (x1 - x) / (x1 - x0); break;
            case 2: t = (z - z0) / (z1 - z0); break;
            default: t = (z1 - z) / (z1 - z0); break;
        }
        return yLow + (yHigh - yLow) * t;
    };
    V3 c[4] = {V3(x0, 0, z0), V3(x1, 0, z0), V3(x1, 0, z1), V3(x0, 0, z1)};
    V3 t[4];
    for (int i = 0; i < 4; i++) t[i] = V3(c[i].x, h(c[i].x, c[i].z), c[i].z);
    setc(top);
    quad4(t[0], t[3], t[2], t[1]);
    setc(side);
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) & 3;
        if (t[i].y < 0.01f && t[j].y < 0.01f) continue;
        quad4(c[i], c[j], t[j], t[i]);
    }
    if (planks) {
        gLighting(false);
        setc(mulc(top, 0.7f));
        gBegin(GL_LINES);
        bool alongX = axis == 1 || axis == -1;
        int n = 10;
        for (int i = 1; i < n; i++) {
            float f = (float)i / (float)n;
            if (alongX) {
                float x = lerpf(x0, x1, f);
                gVertex(x, h(x, z0) + 0.01f, z0);
                gVertex(x, h(x, z1) + 0.01f, z1);
            } else {
                float z = lerpf(z0, z1, f);
                gVertex(x0, h(x0, z) + 0.01f, z);
                gVertex(x1, h(x1, z) + 0.01f, z);
            }
        }
        gEnd();
        gLighting(true);
    }
    rampL(x0, z0, x1, z1, 0, yLow, yHigh, axis);
}

// Kicker with a steel lip at the high end.
static void kicker(float x0, float z0, float x1, float z1, float hgt, int axis) {
    wedge(x0, z0, x1, z1, 0, hgt, axis, C(0.72f, 0.52f, 0.3f), C(0.5f, 0.35f, 0.2f), true);
    setc(COL_STEEL);
    if (axis == 1) cylBetween(V3(x1, hgt, z0), V3(x1, hgt, z1), 0.04f, 5);
    if (axis == -1) cylBetween(V3(x0, hgt, z0), V3(x0, hgt, z1), 0.04f, 5);
    if (axis == 2) cylBetween(V3(x0, hgt, z1), V3(x1, hgt, z1), 0.04f, 5);
    if (axis == -2) cylBetween(V3(x0, hgt, z0), V3(x1, hgt, z0), 0.04f, 5);
}

// Round rail on posts, grindable.
static void roundRail(const V3& a, const V3& b, const Col& c) {
    setc(c);
    cylBetween(a, b, 0.045f, 8);
    setc(mulc(c, 0.7f));
    V3 d = b - a;
    int posts = std::max(2, (int)(len(d) / 3.0f) + 1);
    for (int i = 0; i < posts; i++) {
        V3 p = a + d * ((float)i / (float)(posts - 1));
        float gy = groundAt(p.x, p.z, p.y - 0.1f).h;
        cylBetween(V3(p.x, gy, p.z), p, 0.035f, 6);
    }
    railL(a.x, a.y, a.z, b.x, b.y, b.z, false);
}

static void planterWithPalm(float cx, float cz) {
    setc(0.72f, 0.7f, 0.66f);
    ledgeBoxL(cx - 1.2f, cz - 1.2f, cx + 1.2f, cz + 1.2f, 0, 0.55f);
    // ledgeBoxL only adds the long sides, so add the other two edges by hand
    railL(cx - 1.16f, 0.55f, cz - 1.15f, cx - 1.16f, 0.55f, cz + 1.15f, true);
    railL(cx + 1.16f, 0.55f, cz - 1.15f, cx + 1.16f, 0.55f, cz + 1.15f, true);
    setc(0.3f, 0.22f, 0.15f);
    rectY(cx - 1.1f, cz - 1.1f, cx + 1.1f, cz + 1.1f, 0.56f);
    palmTree(cx, 0.55f, cz, frange(6.0f, 8.0f), false);
}

static void letterAt(float x, float y, float z, char ch) {
    Letter l;
    l.pos = V3(x, y, z);
    l.ch = ch;
    l.got = false;
    g_letters.push_back(l);
}

static void buildPlaza() {
    // kicker to landing ramp across a gap
    kicker(-45.0f, -30.0f, -41.5f, -26.0f, 1.1f, -1);
    wedge(-54.0f, -30.0f, -50.5f, -26.0f, 0, 1.1f, 1, C(0.72f, 0.52f, 0.3f), C(0.5f, 0.35f, 0.2f), true);
    letterAt(-47.8f, 3.0f, -28.0f, 'H');

    // funbox: up ramp, flat top with ledges and a flat bar, down ramp
    Col conc = C(0.7f, 0.68f, 0.64f), concSide = C(0.58f, 0.56f, 0.53f);
    wedge(-86.0f, -33.0f, -82.0f, -25.0f, 0, 1.0f, 1, conc, concSide, false);
    setc(conc);
    box(-82.0f, 0, -33.0f, -74.0f, 1.0f, -25.0f);
    solidL(-82.0f, -33.0f, -74.0f, -25.0f, 0, 1.0f);
    railL(-81.9f, 1.0f, -32.96f, -74.1f, 1.0f, -32.96f, true);
    railL(-81.9f, 1.0f, -25.04f, -74.1f, 1.0f, -25.04f, true);
    wedge(-74.0f, -33.0f, -70.0f, -25.0f, 0, 1.0f, -1, conc, concSide, false);
    setc(COL_STEEL);
    cylBetween(V3(-82.0f, 1.0f, -33.0f), V3(-74.0f, 1.0f, -33.0f), 0.035f, 5);
    cylBetween(V3(-82.0f, 1.0f, -25.0f), V3(-74.0f, 1.0f, -25.0f), 0.035f, 5);
    g_railTag = RT_FUNBOX;
    roundRail(V3(-81.5f, 1.45f, -27.0f), V3(-74.5f, 1.45f, -27.0f), C(0.85f, 0.2f, 0.15f));
    g_railTag = RT_NONE;

    // ground-level flat bar
    g_railTag = RT_PLAZA_BAR;
    roundRail(V3(-66.0f, 0.5f, -19.0f), V3(-50.0f, 0.5f, -19.0f), C(0.95f, 0.75f, 0.1f));
    g_railTag = RT_NONE;
    // long concrete ledge near the start
    setc(0.75f, 0.73f, 0.7f);
    ledgeBoxL(-36.0f, -41.0f, -20.0f, -40.0f, 0, 0.45f);
    // benches
    setc(0.6f, 0.58f, 0.55f);
    ledgeBoxL(-100.0f, -39.0f, -92.0f, -38.2f, 0, 0.5f);
    ledgeBoxL(-100.0f, -21.8f, -92.0f, -21.0f, 0, 0.5f);
    ledgeBoxL(-64.0f, -40.5f, -57.0f, -39.7f, 0, 0.5f);

    planterWithPalm(-30.0f, -19.5f);
    planterWithPalm(-95.0f, -17.0f);
    planterWithPalm(-70.0f, -40.0f);
    planterWithPalm(-22.0f, -60.0f);
    planterWithPalm(-135.0f, -60.0f);

    // raised garden platform with steps and sloped handrails
    Col plat = C(0.66f, 0.6f, 0.52f);
    setc(plat);
    box(-132.0f, 0, -42.0f, -110.0f, 1.6f, -18.0f);
    solidL(-132.0f, -42.0f, -110.0f, -18.0f, 0, 1.6f);
    setc(0.35f, 0.5f, 0.25f);
    rectY(-130.0f, -40.0f, -112.0f, -20.0f, 1.61f);
    railL(-110.04f, 1.6f, -41.9f, -110.04f, 1.6f, -34.1f, true);
    railL(-110.04f, 1.6f, -25.9f, -110.04f, 1.6f, -18.1f, true);
    railL(-131.9f, 1.6f, -18.04f, -110.1f, 1.6f, -18.04f, true);
    railL(-131.9f, 1.6f, -41.96f, -110.1f, 1.6f, -41.96f, true);
    setc(COL_STEEL);
    cylBetween(V3(-110.0f, 1.6f, -42.0f), V3(-110.0f, 1.6f, -34.0f), 0.035f, 5);
    cylBetween(V3(-110.0f, 1.6f, -26.0f), V3(-110.0f, 1.6f, -18.0f), 0.035f, 5);
    cylBetween(V3(-132.0f, 1.6f, -18.0f), V3(-110.0f, 1.6f, -18.0f), 0.035f, 5);
    const int steps = 8;
    for (int k = 0; k < steps; k++) {
        float xa = -110.0f + (float)k * 7.0f / (float)steps, xb = xa + 7.0f / (float)steps;
        float top = 1.6f * (float)(steps - k) / (float)steps;
        setc(k & 1 ? C(0.7f, 0.66f, 0.6f) : C(0.64f, 0.6f, 0.55f));
        box(xa, 0, -34.0f, xb, top, -26.0f);
    }
    solidL(-110.0f, -34.0f, -109.1f, -26.0f, 0, 1.6f);
    rampL(-109.1f, -34.0f, -103.0f, -26.0f, 0, 0.2f, 1.6f, -1);
    g_railTag = RT_STAIR;
    g_railNeed = 1;
    roundRail(V3(-110.2f, 2.5f, -34.3f), V3(-103.0f, 0.9f, -34.3f), COL_STEEL);
    roundRail(V3(-110.2f, 2.5f, -25.7f), V3(-103.0f, 0.9f, -25.7f), COL_STEEL);
    g_railTag = RT_NONE;
    g_railNeed = 0;
    gulmoharTree(-124.0f, 1.6f, -24.0f, 5.0f);
    setc(0.55f, 0.4f, 0.25f);
    ledgeBoxL(-128.0f, -38.0f, -118.0f, -37.2f, 1.6f, 2.1f);
    letterAt(-121.0f, 2.9f, -30.0f, 'I');

    // skywalk: an elevated walkway with ramps at both ends and railings
    const float SW_Y = 3.4f, SZ0 = -72.0f, SZ1 = -68.0f;
    Col swc = C(0.68f, 0.68f, 0.66f), rail = C(0.2f, 0.45f, 0.65f);
    setc(swc);
    box(-110.0f, SW_Y - 0.3f, SZ0, -50.0f, SW_Y, SZ1);
    solidL(-110.0f, SZ0, -50.0f, SZ1, SW_Y - 0.3f, SW_Y);
    wedge(-128.0f, SZ0, -110.0f, SZ1, 0, SW_Y, 1, swc, mulc(swc, 0.85f), false);
    wedge(-50.0f, SZ0, -32.0f, SZ1, 0, SW_Y, -1, swc, mulc(swc, 0.85f), false);
    for (float x = -105.0f; x <= -55.0f; x += 10.0f) {
        setc(0.6f, 0.6f, 0.58f);
        box(x - 0.25f, 0, SZ0 + 0.2f, x + 0.25f, SW_Y - 0.3f, SZ0 + 0.7f);
        box(x - 0.25f, 0, SZ1 - 0.7f, x + 0.25f, SW_Y - 0.3f, SZ1 - 0.2f);
        solidL(x - 0.25f, SZ0 + 0.2f, x + 0.25f, SZ0 + 0.7f, 0, SW_Y - 0.3f);
        solidL(x - 0.25f, SZ1 - 0.7f, x + 0.25f, SZ1 - 0.2f, 0, SW_Y - 0.3f);
    }
    for (int s = 0; s < 2; s++) {
        float z = s == 0 ? SZ0 + 0.05f : SZ1 - 0.05f;
        // deck railing
        setc(rail);
        cylBetween(V3(-110.0f, SW_Y + 1.0f, z), V3(-50.0f, SW_Y + 1.0f, z), 0.045f, 6);
        for (float x = -110.0f; x <= -50.0f; x += 2.0f)
            cylBetween(V3(x, SW_Y, z), V3(x, SW_Y + 1.0f, z), 0.03f, 4);
        solidL(-110.0f, z - 0.05f, -50.0f, z + 0.05f, SW_Y, SW_Y + 1.0f);
        g_railTag = RT_SKYWALK;
        g_railNeed = 3;
        railL(-109.9f, SW_Y + 1.0f, z, -50.1f, SW_Y + 1.0f, z, false);
        // ramp railings
        cylBetween(V3(-128.0f, 1.0f, z), V3(-110.0f, SW_Y + 1.0f, z), 0.045f, 6);
        cylBetween(V3(-50.0f, SW_Y + 1.0f, z), V3(-32.0f, 1.0f, z), 0.045f, 6);
        for (float x = -128.0f; x < -110.0f; x += 2.0f) {
            float y = SW_Y * (x + 128.0f) / 18.0f;
            cylBetween(V3(x, y, z), V3(x, y + 1.0f, z), 0.03f, 4);
            float x2 = -32.0f - (x + 128.0f);
            cylBetween(V3(x2, y, z), V3(x2, y + 1.0f, z), 0.03f, 4);
        }
        rampL(-128.0f, z - 0.05f, -110.0f, z + 0.05f, 0, 1.0f, SW_Y + 1.0f, 1);
        rampL(-50.0f, z - 0.05f, -32.0f, z + 0.05f, 0, 1.0f, SW_Y + 1.0f, -1);
        railL(-127.9f, 1.02f, z, -110.1f, SW_Y + 0.98f, z, false);
        railL(-49.9f, SW_Y + 0.98f, z, -32.1f, 1.02f, z, false);
        g_railTag = RT_NONE;
        g_railNeed = 0;
    }
    // skywalk signboard
    setc(0.1f, 0.3f, 0.6f);
    box(-86.0f, SW_Y + 1.2f, SZ1 - 0.08f, -74.0f, SW_Y + 2.2f, SZ1);
    setc(0.3f, 0.3f, 0.3f);
    cylBetween(V3(-85.5f, SW_Y + 1.0f, SZ1 - 0.05f), V3(-85.5f, SW_Y + 1.2f, SZ1 - 0.05f), 0.04f, 4);
    cylBetween(V3(-74.5f, SW_Y + 1.0f, SZ1 - 0.05f), V3(-74.5f, SW_Y + 1.2f, SZ1 - 0.05f), 0.04f, 4);
    gPush();
    gTranslate(-80.0f, SW_Y + 1.5f, SZ1 + 0.01f);
    text3D("SKYWALK TO STATION", 0.45f, COL_WHITE);
    gPop();
    letterAt(-80.0f, SW_Y + 1.3f, -70.0f, 'C');

    // a gulmohar and some benches north of the tracks
    gulmoharTree(-60.0f, 0, -60.0f, 5.0f);
    gulmoharTree(-100.0f, 0, -58.0f, 5.5f);
    setc(0.6f, 0.58f, 0.55f);
    ledgeBoxL(-90.0f, -61.0f, -82.0f, -60.2f, 0, 0.5f);
}

// ---------------------------------------------------------------- mural wall

// Thick line segment in the XY plane, used for Warli stick figures.
static void seg2(float x0, float y0, float x1, float y1, float w, float z) {
    float dx = x1 - x0, dy = y1 - y0, l = sqrtf(dx * dx + dy * dy);
    if (l < 1e-5f) return;
    float nx = -dy / l * w * 0.5f, ny = dx / l * w * 0.5f;
    gBegin(GL_QUADS);
    gNormal(0, 0, 1);
    gVertex(x0 + nx, y0 + ny, z); gVertex(x0 - nx, y0 - ny, z);
    gVertex(x1 - nx, y1 - ny, z); gVertex(x1 + nx, y1 + ny, z);
    gEnd();
}

static void warliFigure(float x, float y, float s, float z, float pose) {
    // two triangles meeting at the waist, a round head, stick limbs
    gBegin(GL_TRIANGLES);
    gNormal(0, 0, 1);
    gVertex(x - 0.18f * s, y + 0.75f * s, z); gVertex(x + 0.18f * s, y + 0.75f * s, z); gVertex(x, y + 0.45f * s, z);
    gVertex(x, y + 0.45f * s, z); gVertex(x - 0.16f * s, y + 0.2f * s, z); gVertex(x + 0.16f * s, y + 0.2f * s, z);
    gEnd();
    discZ(x, y + 0.88f * s, 0.1f * s, z, 10);
    float w = 0.035f * s;
    seg2(x - 0.1f * s, y + 0.2f * s, x - 0.18f * s - pose * 0.05f * s, y, w, z);
    seg2(x + 0.1f * s, y + 0.2f * s, x + 0.18f * s + pose * 0.05f * s, y, w, z);
    seg2(x - 0.17f * s, y + 0.74f * s, x - 0.35f * s, y + (0.55f + pose * 0.2f) * s, w, z);
    seg2(x + 0.17f * s, y + 0.74f * s, x + 0.35f * s, y + (0.55f - pose * 0.2f) * s, w, z);
}

static void buildMuralWall() {
    const float z = -86.0f, H = 5.5f;
    setc(0.8f, 0.78f, 0.74f);
    box(-141.0f, 0, z - 1.0f, -11.0f, H, z);
    solidL(-141.0f, z - 1.0f, -11.0f, z, 0, H);
    setc(0.6f, 0.58f, 0.55f);
    box(-141.0f, H, z - 1.05f, -11.0f, H + 0.2f, z + 0.05f);
    float fz = z + 0.01f;
    // Warli panel: white figures on terracotta
    setc(0.62f, 0.3f, 0.18f);
    rectZ(-140.0f, 0.6f, -101.0f, H - 0.3f, fz);
    setc(0.97f, 0.95f, 0.9f);
    for (int i = 0; i < 12; i++) {
        float a = (float)i * TWO_PI / 12.0f;
        warliFigure(-125.0f + cosf(a) * 3.0f, 2.2f + sinf(a) * 1.1f, 0.9f, fz + 0.01f, (i & 1) ? 1.0f : -1.0f);
    }
    for (int i = 0; i < 7; i++) warliFigure(-111.0f + (float)i * 1.3f, 1.0f, 1.1f, fz + 0.01f, (float)((i % 3) - 1));
    for (int i = 0; i < 3; i++) {
        float hx = -137.0f + (float)i * 3.0f;
        gBegin(GL_TRIANGLES);
        gNormal(0, 0, 1);
        gVertex(hx - 1.1f, 3.6f, fz + 0.01f); gVertex(hx + 1.1f, 3.6f, fz + 0.01f); gVertex(hx, 4.6f, fz + 0.01f);
        gEnd();
        seg2(hx - 0.8f, 3.6f, hx - 0.8f, 2.6f, 0.06f, fz + 0.01f);
        seg2(hx + 0.8f, 3.6f, hx + 0.8f, 2.6f, 0.06f, fz + 0.01f);
    }
    discZ(-104.5f, 4.2f, 0.5f, fz + 0.01f, 16);
    // sea panel with sun, waves and the city's nickname
    setc(0.1f, 0.25f, 0.55f);
    rectZ(-101.0f, 0.6f, -60.0f, H - 0.3f, fz);
    setc(0.98f, 0.55f, 0.1f);
    discZ(-68.0f, 3.9f, 0.9f, fz + 0.01f, 24);
    for (int i = 0; i < 12; i++) {
        float a = (float)i * TWO_PI / 12.0f;
        seg2(-68.0f + cosf(a) * 1.1f, 3.9f + sinf(a) * 1.1f, -68.0f + cosf(a) * 1.5f, 3.9f + sinf(a) * 1.5f, 0.12f, fz + 0.01f);
    }
    for (int row = 0; row < 3; row++) {
        setc(row == 1 ? C(0.4f, 0.7f, 0.95f) : C(0.9f, 0.95f, 1.0f));
        float yb = 0.9f + (float)row * 0.45f;
        for (float x = -100.5f; x < -60.5f; x += 0.4f)
            seg2(x, yb + sinf(x * 1.3f) * 0.12f, x + 0.4f, yb + sinf((x + 0.4f) * 1.3f) * 0.12f, 0.1f, fz + 0.01f);
    }
    gPush();
    gTranslate(-84.0f, 3.2f, fz + 0.02f);
    text3D("AAMCHI MUMBAI", 1.0f, C(0.98f, 0.9f, 0.3f), 1.0f);
    gPop();
    // rangoli panel
    setc(0.95f, 0.8f, 0.3f);
    rectZ(-60.0f, 0.6f, -36.0f, H - 0.3f, fz);
    float rx = -48.0f, ry = 2.9f;
    Col rings[5] = {{0.8f, 0.1f, 0.2f}, {0.1f, 0.5f, 0.3f}, {0.95f, 0.45f, 0.05f}, {0.4f, 0.1f, 0.5f}, {1, 1, 1}};
    for (int ring = 0; ring < 5; ring++) {
        float r = 2.2f - (float)ring * 0.42f;
        setc(rings[ring]);
        discZ(rx, ry, r, fz + 0.01f + (float)ring * 0.004f, 32);
        setc(rings[(ring + 2) % 5]);
        int petals = 8 + ring * 2;
        for (int p = 0; p < petals; p++) {
            float a = (float)p * TWO_PI / (float)petals;
            gBegin(GL_TRIANGLES);
            gNormal(0, 0, 1);
            float zz = fz + 0.012f + (float)ring * 0.004f;
            gVertex(rx + cosf(a - 0.15f) * r * 0.8f, ry + sinf(a - 0.15f) * r * 0.8f, zz);
            gVertex(rx + cosf(a + 0.15f) * r * 0.8f, ry + sinf(a + 0.15f) * r * 0.8f, zz);
            gVertex(rx + cosf(a) * r * 1.02f, ry + sinf(a) * r * 1.02f, zz);
            gEnd();
        }
    }
    // graffiti panel
    setc(0.85f, 0.25f, 0.55f);
    rectZ(-36.0f, 0.6f, -12.0f, H - 0.3f, fz);
    gPush();
    gTranslate(-24.0f, 2.6f, fz + 0.02f);
    text3D("SKATE", 1.6f, COL_BLACK, 2.0f);
    gTranslate(0.05f, 0.05f, 0.01f);
    text3D("SKATE", 1.6f, C(0.3f, 0.95f, 0.85f), 1.0f);
    gPop();
    setc(COL_WHITE);
    gPush();
    gTranslate(-17.0f, 4.3f, fz + 0.01f);
    gScale(1.0f, 0.55f, 1.0f);
    discZ(0, 0, 0.9f, 0, 20);
    setc(0.2f, 0.6f, 0.3f);
    discZ(0, 0, 0.45f, 0.01f, 16);
    setc(COL_BLACK);
    discZ(0, 0, 0.2f, 0.02f, 12);
    gPop();
    // sitting ledge (katta) along the whole wall
    setc(0.72f, 0.7f, 0.66f);
    ledgeBoxL(-138.0f, z, -14.0f, z + 0.8f, 0, 0.5f);
}

// ---------------------------------------------------------------- railway

static void buildViaduct() {
    const float Z0 = -52.0f, Z1 = -44.0f;
    Col conc = C(0.66f, 0.64f, 0.6f);
    for (float x = -420.0f; x < 420.0f; x += 40.0f) {
        setc(conc);
        box(x, DECK_TOP - 1.2f, Z0, x + 40.0f, DECK_TOP, Z1);
        setc(mulc(conc, 0.9f));
        box(x, DECK_TOP, Z0, x + 40.0f, DECK_TOP + 0.9f, Z0 + 0.3f);
        box(x, DECK_TOP, Z1 - 0.3f, x + 40.0f, DECK_TOP + 0.9f, Z1);
        setc(0.45f, 0.42f, 0.38f);
        box(x, DECK_TOP, -51.4f, x + 40.0f, DECK_TOP + 0.12f, -48.6f);
        box(x, DECK_TOP, -47.4f, x + 40.0f, DECK_TOP + 0.12f, -44.6f);
        setc(0.55f, 0.55f, 0.58f);
        for (int t = 0; t < 2; t++) {
            float zc = t == 0 ? -50.0f : -46.0f;
            box(x, DECK_TOP + 0.12f, zc - 0.78f, x + 40.0f, DECK_TOP + 0.25f, zc - 0.7f);
            box(x, DECK_TOP + 0.12f, zc + 0.7f, x + 40.0f, DECK_TOP + 0.25f, zc + 0.78f);
        }
    }
    // sleepers where the camera can see them
    setc(0.35f, 0.32f, 0.3f);
    for (float x = -220.0f; x < 220.0f; x += 1.2f) {
        box(x, DECK_TOP + 0.1f, -51.1f, x + 0.25f, DECK_TOP + 0.16f, -48.9f);
        box(x, DECK_TOP + 0.1f, -47.1f, x + 0.25f, DECK_TOP + 0.16f, -44.9f);
    }
    // piers, with posters and a steel girder span over the cross road
    for (int k = 0; k < 12; k++) {
        for (int s = -1; s <= 1; s += 2) {
            float x = (float)s * (20.0f + 18.0f * (float)k);
            setc(mulc(conc, 0.95f));
            box(x - 1.0f, 0, -49.0f, x + 1.0f, DECK_TOP - 1.9f, -47.0f);
            box(x - 1.3f, DECK_TOP - 1.9f, Z0 - 0.5f, x + 1.3f, DECK_TOP - 1.2f, Z1 + 0.5f);
            if (fabsf(x) < WORLD_MAX_X) solidL(x - 1.0f, -49.0f, x + 1.0f, -47.0f, 0, DECK_TOP - 1.9f);
            if (fabsf(x) < 150.0f) {
                gPush();
                gTranslate(x, 0, -47.0f);
                posterWall(-0.9f, 0.9f, 0.9f, 2.6f, 0.01f);
                gPop();
                gPush();
                gTranslate(x, 0, -49.0f);
                gRotate(180, 0, 1, 0);
                posterWall(-0.9f, 0.9f, 0.9f, 2.6f, 0.01f);
                gPop();
            }
        }
    }
    setc(0.25f, 0.32f, 0.4f);
    for (int s = 0; s < 2; s++) {
        float z = s == 0 ? Z0 - 0.1f : Z1 + 0.1f;
        cylBetween(V3(-20, DECK_TOP + 2.8f, z), V3(20, DECK_TOP + 2.8f, z), 0.12f, 6);
        for (float x = -20.0f; x < 20.0f; x += 4.0f) {
            cylBetween(V3(x, DECK_TOP, z), V3(x, DECK_TOP + 2.8f, z), 0.08f, 5);
            cylBetween(V3(x, DECK_TOP, z), V3(x + 4.0f, DECK_TOP + 2.8f, z), 0.06f, 5);
        }
    }
    setc(0.95f, 0.8f, 0.1f);
    box(-9.0f, DECK_TOP - 1.1f, Z1 + 0.01f, 9.0f, DECK_TOP - 0.2f, Z1 + 0.05f);
    gPush();
    gTranslate(0, DECK_TOP - 0.85f, Z1 + 0.06f);
    text3D("CLEARANCE 5.4 M", 0.45f, COL_BLACK);
    gPop();
    // overhead line masts and wires
    for (float x = -216.0f; x <= 216.0f; x += 36.0f) {
        setc(0.4f, 0.42f, 0.45f);
        box(x - 0.15f, DECK_TOP, Z0 + 0.3f, x + 0.15f, DECK_TOP + 6.0f, Z0 + 0.6f);
        box(x - 0.1f, DECK_TOP + 5.6f, Z0 + 0.3f, x + 0.1f, DECK_TOP + 5.8f, Z1 - 0.8f);
    }
    gLighting(false);
    setc(0.1f, 0.1f, 0.1f);
    gBegin(GL_LINES);
    for (int t = 0; t < 2; t++) {
        float zc = t == 0 ? -50.0f : -46.0f;
        gVertex(-420, DECK_TOP + 5.3f, zc); gVertex(420, DECK_TOP + 5.3f, zc);
        gVertex(-420, DECK_TOP + 5.55f, zc); gVertex(420, DECK_TOP + 5.55f, zc);
    }
    gEnd();
    gLighting(true);
}

static void buildStation() {
    // station hall in local frame, front at z=0 facing +Z (the main road)
    pushXf(92.0f, -28.0f, 0);
    const float W = 42.0f, D = 15.0f, H = DECK_TOP - 0.3f;
    setc(0.93f, 0.88f, 0.75f);
    box(0, 0, -D, W, H, 0);
    solidL(0, -D, W, 0, 0, H);
    setc(0.55f, 0.15f, 0.12f);
    box(-0.02f, 0, -D, W + 0.02f, 0.9f, 0.02f);
    for (int i = 0; i < 6; i++) {
        float cx = 4.0f + (float)i * 6.8f;
        setc(0.18f, 0.14f, 0.12f);
        rectZ(cx - 1.4f, 0.9f, cx + 1.4f, 3.6f, 0.03f);
        discZ(cx, 3.6f, 1.4f, 0.03f, 16);
        setc(0.55f, 0.15f, 0.12f);
        box(cx - 1.6f, 0.9f, 0, cx - 1.4f, 3.6f, 0.1f);
        box(cx + 1.4f, 0.9f, 0, cx + 1.6f, 3.6f, 0.1f);
    }
    // the yellow station board with black lettering
    setc(COL_YELLOW);
    box(8.0f, 5.0f, 0, 34.0f, 6.8f, 0.25f);
    setc(COL_BLACK);
    box(7.8f, 4.9f, -0.01f, 34.2f, 5.0f, 0.27f);
    gPush();
    gTranslate(21.0f, 5.35f, 0.27f);
    text3D("DADAR", 1.2f, COL_BLACK, 1.3f);
    gPop();
    setc(0.93f, 0.93f, 0.9f);
    discZ(38.0f, 5.6f, 0.9f, 0.04f, 24);
    setc(COL_BLACK);
    seg2(38.0f, 5.6f, 38.0f, 6.25f, 0.08f, 0.05f);
    seg2(38.0f, 5.6f, 38.45f, 5.6f, 0.08f, 0.05f);
    signBoard(1.0f, 7.0f, 4.1f, 4.7f, 0.1f, "TICKETS", SIGNS[1]);
    signBoard(35.0f, 41.0f, 4.1f, 4.7f, 0.1f, "PLATFORM 1", SIGNS[4]);
    // canopy over the entrance
    setc(0.45f, 0.45f, 0.48f);
    box(0, 4.0f, 0, W, 4.15f, 2.5f);
    popXf();
    // platform roof up on the viaduct
    setc(0.5f, 0.5f, 0.52f);
    box(92.0f, 12.0f, -54.0f, 134.0f, 12.25f, -42.0f);
    setc(0.35f, 0.35f, 0.38f);
    for (float x = 94.0f; x <= 132.0f; x += 6.0f) {
        box(x - 0.12f, DECK_TOP - 1.2f, -53.8f, x + 0.12f, 12.0f, -53.5f);
        box(x - 0.12f, DECK_TOP - 1.2f, -42.5f, x + 0.12f, 12.0f, -42.2f);
    }
    // auto stand in front of the station
    for (int i = 0; i < 5; i++) {
        float x = 98.0f + (float)i * 4.5f;
        gPush();
        gTranslate(x, 0, -20.0f);
        gRotate(8.0f * (float)(i % 2), 0, 1, 0);
        drawAuto(0);
        gPop();
        solidL(x - 0.72f, -21.4f, x + 0.72f, -18.6f, 0, 1.8f);
    }
    for (int i = 0; i < 2; i++) {
        float x = 124.0f + (float)i * 5.5f;
        gPush();
        gTranslate(x, 0, -20.0f);
        drawTaxi(0);
        gPop();
        solidL(x - 0.8f, -22.0f, x + 0.8f, -18.0f, 0, 1.5f);
    }
    setc(0.3f, 0.3f, 0.3f);
    cylBetween(V3(95.0f, 0, -17.0f), V3(95.0f, 2.8f, -17.0f), 0.05f, 6);
    solidL(94.9f, -17.1f, 95.1f, -16.9f, 0, 2.8f);
    signBoard(94.0f, 96.8f, 2.8f, 3.4f, -16.95f, "AUTO STAND", SIGNS[3]);
    palmTree(88.0f, 0, -16.0f, 7.5f, true);
    palmTree(138.0f, 0, -16.0f, 7.0f, true);
}

// ---------------------------------------------------------------- construction site

static void tinFence(float x0, float z0, float x1, float z1) {
    Col blue = C(0.15f, 0.35f, 0.7f);
    setc(blue);
    box(x0, 0, z0, x1, 2.2f, z1);
    solidL(x0, z0, x1, z1, 0, 2.2f);
    gLighting(false);
    setc(mulc(blue, 0.75f));
    gBegin(GL_LINES);
    if (x1 - x0 > z1 - z0) {
        for (float x = x0; x < x1; x += 0.25f) {
            gVertex(x, 0, z0 - 0.01f); gVertex(x, 2.2f, z0 - 0.01f);
            gVertex(x, 0, z1 + 0.01f); gVertex(x, 2.2f, z1 + 0.01f);
        }
    } else {
        for (float z = z0; z < z1; z += 0.25f) {
            gVertex(x0 - 0.01f, 0, z); gVertex(x0 - 0.01f, 2.2f, z);
            gVertex(x1 + 0.01f, 0, z); gVertex(x1 + 0.01f, 2.2f, z);
        }
    }
    gEnd();
    gLighting(true);
}

static void buildConstruction() {
    tinFence(32.0f, -16.1f, 42.0f, -15.9f);
    tinFence(50.0f, -16.1f, 82.0f, -15.9f);
    tinFence(32.0f, -42.1f, 82.0f, -41.9f);
    tinFence(31.9f, -42.0f, 32.1f, -16.0f);
    tinFence(81.9f, -42.0f, 82.1f, -16.0f);
    gPush();
    gTranslate(66.0f, 1.3f, -15.88f);
    text3D("SHREE SAI DEVELOPERS", 0.55f, COL_WHITE);
    gTranslate(0, -0.8f, 0);
    text3D("LUXURY 2 BHK  COMING SOON", 0.35f, COL_YELLOW);
    gPop();
    // concrete frame going up
    Col conc = C(0.62f, 0.62f, 0.6f);
    float cxs[4] = {54, 62, 70, 78}, czs[3] = {-38, -31, -24};
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 3; j++) {
            setc(conc);
            boxc(cxs[i], 0, czs[j], 0.5f, 9.6f, 0.5f);
            solidL(cxs[i] - 0.25f, czs[j] - 0.25f, cxs[i] + 0.25f, czs[j] + 0.25f, 0, 9.6f);
            gLighting(false);
            setc(0.35f, 0.2f, 0.12f);
            for (int r = 0; r < 4; r++) {
                float ox = (r & 1) ? 0.15f : -0.15f, oz = (r & 2) ? 0.15f : -0.15f;
                line3(V3(cxs[i] + ox, 9.6f, czs[j] + oz), V3(cxs[i] + ox * 1.3f, 10.8f, czs[j] + oz * 1.3f));
            }
            gLighting(true);
        }
    for (int f = 1; f <= 3; f++) {
        float y = 3.2f * (float)f;
        setc(f == 3 ? mulc(conc, 0.9f) : conc);
        box(53.75f, y - 0.25f, -38.25f, 78.25f, y, -23.75f);
        solidL(53.75f, -38.25f, 78.25f, -23.75f, y - 0.25f, y);
    }
    letterAt(66.0f, 4.4f, -34.5f, 'A');
    // plank ramp up to the first slab
    wedge(40.0f, -36.5f, 53.75f, -32.5f, 0, 3.2f, 1, C(0.62f, 0.46f, 0.28f), C(0.45f, 0.32f, 0.2f), true);
    // bamboo scaffolding on the south face
    Col bamboo = C(0.78f, 0.66f, 0.38f);
    setc(bamboo);
    for (float x = 53.0f; x <= 79.0f; x += 2.0f) {
        cylBetween(V3(x, 0, -23.0f), V3(x + 0.1f, 11.0f, -23.0f), 0.05f, 5);
        solidL(x - 0.08f, -23.08f, x + 0.08f, -22.92f, 0, 11.0f);
    }
    for (float y = 1.6f; y < 11.0f; y += 1.6f) cylBetween(V3(52.5f, y, -22.95f), V3(79.5f, y, -22.95f), 0.045f, 5);
    for (float x = 53.0f; x < 79.0f; x += 4.0f) cylBetween(V3(x, 0.2f, -22.9f), V3(x + 4.0f, 6.6f, -22.9f), 0.04f, 5);
    gLighting(false);
    setc(0.1f, 0.35f, 0.15f);
    gBegin(GL_LINES);
    for (float x = 60.0f; x < 72.0f; x += 0.3f) { gVertex(x, 6.4f, -22.8f); gVertex(x, 11.0f, -22.8f); }
    for (float y = 6.4f; y < 11.0f; y += 0.3f) { gVertex(60.0f, y, -22.8f); gVertex(72.0f, y, -22.8f); }
    gEnd();
    gLighting(true);
    // kicker made from scrap planks
    kicker(36.0f, -24.0f, 39.0f, -20.0f, 1.0f, 1);
    // big concrete pipe lying on its side: grind the top
    setc(0.6f, 0.6f, 0.58f);
    gPush();
    gTranslate(56.0f, 0.65f, -19.3f);
    gRotate(-90, 0, 0, 1);
    cyl(0.65f, 0.65f, 6.0f, 14, true);
    gPop();
    solidL(56.0f, -19.95f, 62.0f, -18.65f, 0, 1.3f);
    g_railTag = RT_PIPE;
    railL(56.1f, 1.3f, -19.3f, 61.9f, 1.3f, -19.3f, true);
    g_railTag = RT_NONE;
    // sand pile, bricks, cement bags, mixer
    setc(0.8f, 0.68f, 0.45f);
    gPush();
    gTranslate(76.0f, 0, -19.5f);
    gScale(1.0f, 1.0f, 0.8f);
    cyl(2.4f, 0.2f, 1.2f, 16, true);
    gPop();
    solidL(74.8f, -20.5f, 77.2f, -18.5f, 0, 0.7f);
    setc(0.65f, 0.25f, 0.15f);
    for (int k = 0; k < 3; k++) box(44.0f + (float)k * 1.3f, 0, -40.5f, 45.1f + (float)k * 1.3f, 1.0f, -38.8f);
    solidL(44.0f, -40.5f, 48.0f, -38.8f, 0, 1.0f);
    setc(0.85f, 0.85f, 0.82f);
    for (int k = 0; k < 6; k++) box(48.5f + (float)(k % 3) * 0.7f, (float)(k / 3) * 0.2f, -21.0f, 49.1f + (float)(k % 3) * 0.7f, 0.2f + (float)(k / 3) * 0.2f, -20.1f);
    solidL(48.5f, -21.0f, 50.6f, -20.1f, 0, 0.4f);
    setc(0.85f, 0.45f, 0.1f);
    gPush();
    gTranslate(36.5f, 0.8f, -38.0f);
    gRotate(60, 0, 0, 1);
    cyl(0.5f, 0.8f, 1.4f, 12, true);
    gPop();
    setc(0.3f, 0.3f, 0.3f);
    box(35.5f, 0, -38.8f, 37.8f, 0.6f, -37.2f);
    solidL(35.2f, -39.0f, 38.2f, -37.0f, 0, 1.8f);
}

// ---------------------------------------------------------------- skyline

static void buildSkyline() {
    for (int i = 0; i < 70; i++) {
        float a = frange(0, TWO_PI), r = frange(215.0f, 270.0f);
        float x = cosf(a) * r, z = sinf(a) * r;
        float w = frange(14, 30), d = frange(14, 30), h = frange(22, 60);
        if (fabsf(z) < 25.0f || z > 150.0f) continue;  // keep the road ends and the sea open
        Col c = mixc(C(0.8f, 0.77f, 0.72f), C(0.66f, 0.7f, 0.76f), frand());
        setc(c);
        box(x - w / 2, 0, z - d / 2, x + w / 2, h, z + d / 2);
        setc(mulc(c, 0.75f));
        for (float y = 4.0f; y < h - 2.0f; y += 3.2f) {
            rectZ(x - w / 2 + 1, y, x + w / 2 - 1, y + 1.2f, z + d / 2 + 0.05f);
            rectZ(x - w / 2 + 1, y, x + w / 2 - 1, y + 1.2f, z - d / 2 - 0.05f);
        }
    }
}

// ---------------------------------------------------------------- whole city

static void buildPeds() {
    for (int i = 0; i < 16; i++) {
        float side = i < 8 ? -1.0f : 1.0f;
        float z = frange(8.9f, 10.4f);
        float x0 = side * frange(14.0f, 120.0f), x1 = x0 + side * frange(15.0f, 40.0f);
        int st = irange(0, 3);
        if (i % 7 == 6) st = PED_KID;
        addPed(V3(x0, SIDEWALK_H, z), V3(clampf(x1, -138, 138), SIDEWALK_H, z), st, true);
    }
    for (int i = 0; i < 14; i++) {
        float side = i < 7 ? -1.0f : 1.0f;
        float z = frange(-10.8f, -9.2f);
        float x0 = side * frange(14.0f, 120.0f), x1 = x0 + side * frange(15.0f, 40.0f);
        addPed(V3(x0, SIDEWALK_H, z), V3(clampf(x1, -138, 138), SIDEWALK_H, z), irange(0, 3), true);
    }
    for (int i = 0; i < 6; i++) {
        float x = (i & 1) ? frange(8.3f, 10.7f) : -frange(8.3f, 10.7f);
        bool north = i < 4;
        float z0 = north ? frange(-80.0f, -40.0f) : frange(15.0f, 25.0f);
        float z1 = north ? z0 + frange(15.0f, 25.0f) : frange(28.0f, 38.0f);
        addPed(V3(x, SIDEWALK_H, z0), V3(x, SIDEWALK_H, std::min(z1, -15.0f + (north ? 0.0f : 60.0f))), irange(0, 3), true);
    }
    addPed(V3(-135.0f, 0, -43.0f), V3(-60.0f, 0, -43.0f), PED_MAN, true);
    addPed(V3(-100.0f, 0, -43.5f), V3(-15.0f, 0, -43.5f), PED_SAREE, true);
    addPed(V3(-130.0f, 0, -57.0f), V3(-20.0f, 0, -57.0f), PED_DABBAWALA, true);
    addPed(V3(-125.0f, 0, -56.0f), V3(-70.0f, 0, -56.0f), PED_SALWAR, true);
    addPed(V3(-108.0f, 3.4f, -70.8f), V3(-52.0f, 3.4f, -70.8f), PED_MAN, true);
    addPed(V3(15.0f, 0, -56.0f), V3(135.0f, 0, -56.0f), PED_DABBAWALA, true);
    addPed(V3(90.0f, 0, -26.5f), V3(136.0f, 0, -26.5f), PED_SAREE, true);
    addPed(V3(92.0f, 0, -25.5f), V3(130.0f, 0, -25.5f), PED_MAN, true);
}

// Spots added for the career: the road kicker, the skate shop and the barricades.
static void buildMissionSpots() {
    // kicker on the centre line of the cross road, pointed across the main road
    kicker(-1.5f, -13.0f, 1.5f, -9.5f, 1.6f, 2);
    for (int s = -1; s <= 1; s += 2)
        for (int k = 0; k < 3; k++) {
            float cz = -16.0f + (float)k * 2.5f;
            solidL((float)s * 1.9f - 0.15f, cz - 0.15f, (float)s * 1.9f + 0.15f, cz + 0.15f, 0, 0.6f);
            setc(0.95f, 0.45f, 0.1f);
            gPush();
            gTranslate((float)s * 1.9f, 0, cz);
            cyl(0.18f, 0.03f, 0.6f, 8, false);
            setc(COL_WHITE);
            gTranslate(0, 0.28f, 0);
            cyl(0.11f, 0.08f, 0.1f, 8, false);
            gPop();
        }
    pushXf(9.3f, -18.0f, 3);
    setc(0.3f, 0.3f, 0.3f);
    cylBetween(V3(-1.0f, SIDEWALK_H, 0.05f), V3(-1.0f, 3.0f, 0.05f), 0.05f, 6);
    cylBetween(V3(1.0f, SIDEWALK_H, 0.05f), V3(1.0f, 3.0f, 0.05f), 0.05f, 6);
    solidL(-1.08f, -0.03f, -0.92f, 0.13f, 0, 3.0f);
    solidL(0.92f, -0.03f, 1.08f, 0.13f, 0, 3.0f);
    signBoard(-1.3f, 1.3f, 2.2f, 3.0f, 0.1f, "JUMP AT OWN RISK", SIGNS[3]);
    popXf();

    // Skate Crew Adda: a plywood skate shop tucked under the railway bridge
    pushXf(-15.0f, -49.6f, 0);
    setc(0.55f, 0.4f, 0.25f);
    box(-2.6f, 0, 0, 2.6f, 3.0f, 0.2f);
    box(-2.6f, 0, 0, -2.4f, 3.0f, 2.2f);
    box(2.4f, 0, 0, 2.6f, 3.0f, 2.2f);
    solidL(-2.6f, 0, 2.6f, 0.2f, 0, 3.0f);
    solidL(-2.6f, 0, -2.4f, 2.2f, 0, 3.0f);
    solidL(2.4f, 0, 2.6f, 2.2f, 0, 3.0f);
    setc(0.2f, 0.2f, 0.22f);
    box(-2.7f, 3.0f, -0.1f, 2.7f, 3.15f, 2.6f);
    setc(0.3f, 0.22f, 0.15f);
    box(-2.4f, 0, 1.8f, 2.4f, 1.0f, 2.2f);
    solidL(-2.4f, 1.8f, 2.4f, 2.2f, 0, 1.0f);
    // decks on the back wall
    for (int i = 0; i < 7; i++) {
        float x = -2.0f + (float)i * 0.66f;
        setc(SAREES[i % 8]);
        box(x - 0.1f, 0.9f, 0.21f, x + 0.1f, 2.6f, 0.24f);
        setc(COL_BLACK);
        box(x - 0.1f, 0.9f, 0.245f, x + 0.1f, 1.0f, 0.25f);
    }
    // wheels in a tray on the counter
    for (int i = 0; i < 8; i++) {
        setc(i & 1 ? C(0.95f, 0.9f, 0.3f) : COL_WHITE);
        gPush();
        gTranslate(-1.5f + (float)i * 0.25f, 1.0f, 2.0f);
        cyl(0.07f, 0.07f, 0.06f, 8, true);
        gPop();
    }
    signBoard(-2.5f, 2.5f, 3.15f, 3.95f, 2.3f, "SKATE CREW ADDA", SIGNS[5]);
    gPush();
    gTranslate(0, 0.35f, 2.21f);
    text3DFit("DECKS WHEELS BEARINGS", 0.2f, 4.2f, COL_YELLOW);
    gPop();
    posterWall(-2.4f, 2.4f, 1.1f, 2.0f, 2.23f);
    popXf();
    g_shopPos = V3(-15.0f, 0.0f, -46.0f);
    // job board: a notice board papered with odd jobs
    pushXf(-23.0f, -46.2f, 0);
    setc(0.4f, 0.28f, 0.18f);
    box(-1.3f, 0, -0.08f, -1.18f, 2.3f, 0.08f);
    box(1.18f, 0, -0.08f, 1.3f, 2.3f, 0.08f);
    solidL(-1.3f, -0.1f, 1.3f, 0.1f, 0, 2.3f);
    setc(0.62f, 0.45f, 0.28f);
    box(-1.25f, 0.9f, -0.05f, 1.25f, 2.05f, 0.05f);
    for (int k = 0; k < 7; k++) {
        float px = -1.05f + (float)(k % 4) * 0.55f + (k >= 4 ? 0.25f : 0.0f), py = k >= 4 ? 1.05f : 1.5f;
        setc(k % 3 == 0 ? C(0.96f, 0.95f, 0.88f) : (k % 3 == 1 ? C(1.0f, 0.9f, 0.55f) : C(0.8f, 0.9f, 1.0f)));
        rectZ(px, py, px + 0.4f, py + 0.42f, 0.06f);
        setc(0.8f, 0.1f, 0.1f);
        discZ(px + 0.2f, py + 0.38f, 0.025f, 0.065f, 6);
    }
    signBoard(-1.3f, 1.3f, 2.1f, 2.6f, 0.02f, "KAAM CHAHIYE? JOBS", SIGNS[3]);
    popXf();
    g_boardPos = V3(-23.0f, 0.0f, -44.4f);
    gPush();
    gTranslate(-19.0f, 0, -47.0f);
    setc(0.1f, 0.1f, 0.1f);
    gTranslate(0, 2.8f, 0.01f);
    text3D("SKATE", 0.6f, C(0.3f, 0.95f, 0.85f), 1.5f);
    gTranslate(0, -0.8f, 0);
    text3D("OR DIE", 0.5f, C(1.0f, 0.4f, 0.6f), 1.5f);
    gPop();

    // named delivery drops that are not shopfronts
    g_xf.ox = 0; g_xf.oz = 0; g_xf.rot = 0;
    addDestL("DADAR STATION TICKET WINDOW", 96.0f, -26.0f, 0.0f, 0);
    addDestL("BEST BUS STOP", -66.0f, 11.5f, SIDEWALK_H, 0);
    addDestL("AUTO STAND UNION OFFICE", 100.0f, -17.0f, 0.0f, 0);
    addDestL("GARDEN MALI (GARDENER)", -122.0f, -34.0f, 1.6f, 1);
    addDestL("SITE OFFICE, SHREE SAI", 40.0f, -28.0f, 0.0f, 2);
    addDestL("SKYWALK SHOESHINE BOY", -80.0f, -70.0f, 3.4f, 3);
    addDestL("SKATE CREW ADDA", -15.0f, -45.5f, 0.0f, 0);

    // barricades, opened as chapters are finished
    addGate(-102.8f, -35.4f, -102.3f, -24.6f, 2.6f, 1, true);  // garden steps
    addGate(-110.0f, -35.4f, -102.3f, -34.9f, 2.6f, 1, false);
    addGate(-110.0f, -25.1f, -102.3f, -24.6f, 2.6f, 1, false);
    addGate(42.0f, -16.4f, 50.0f, -15.6f, 2.6f, 2, true);        // construction site
    addGate(-128.9f, -72.3f, -128.3f, -67.7f, 2.6f, 3, true);    // skywalk west ramp
    addGate(-31.7f, -72.3f, -31.1f, -67.7f, 2.6f, 3, true);      // skywalk east ramp
}

// Drops delivery points that fall outside the playable block or inside something solid.
static void pruneDests() {
    std::vector<Dest> keep;
    for (size_t i = 0; i < g_dests.size(); i++) {
        const Dest& d = g_dests[i];
        if (d.pos.x < WORLD_MIN_X + 3 || d.pos.x > WORLD_MAX_X - 3 || d.pos.z < -84.0f || d.pos.z > 158.0f) continue;
        if (d.pos.z > 42.0f && d.pos.z < 120.0f && fabsf(d.pos.x) > 12.0f) continue;  // behind the shop lane
        if (blockedAt(d.pos.x, d.pos.z, d.pos.y, 0.32f)) continue;
        keep.push_back(d);
    }
    g_dests = keep;
}

static void buildSeaFace();  // city_seaface.hpp

static void buildWorld() {
    g_seed = 1234567u;
    g_solids.clear();
    g_rails.clear();
    g_peds.clear();
    g_signals.clear();
    g_letters.clear();
    g_poleTops.clear();
    g_stalls.clear();
    g_dests.clear();
    g_gates.clear();
    g_chawlCount = 0;

    buildGround();
    g_railTag = RT_MEDIAN;
    buildMedian();
    g_railTag = RT_NONE;
    buildStreetFurniture();

    // south row facing the main road, and the rows that close off the edges
    pushXf(-11.0f, 13.0f, 2); buildingRow(250.0f, 12.0f, 16.0f, "KIRANA STORE"); popXf();
    pushXf(261.0f, 13.0f, 2); buildingRow(250.0f, 12.0f, 16.0f, nullptr); popXf();
    pushXf(11.0f, -62.0f, 0); buildingRow(250.0f, 12.0f, 16.0f, "IRANI CAFE"); popXf();
    pushXf(11.0f, -44.0f, 3); buildingRow(30.0f, 12.0f, 14.0f, "CHEMIST"); popXf();
    pushXf(141.0f, -13.0f, 0); buildingRow(120.0f, 40.0f, 49.0f, nullptr); popXf();
    pushXf(-261.0f, -13.0f, 0); buildingRow(120.0f, 40.0f, 60.0f, nullptr); popXf();
    pushXf(-261.0f, -88.0f, 0); buildingRow(250.0f, 15.0f, 20.0f, nullptr); popXf();
    pushXf(11.0f, -88.0f, 0); buildingRow(250.0f, 15.0f, 20.0f, nullptr); popXf();
    pushXf(-11.0f, 120.0f, 1); buildingRow(90.0f, 12.0f, 16.0f, nullptr); popXf();
    pushXf(11.0f, 30.0f, 3); buildingRow(90.0f, 12.0f, 16.0f, nullptr); popXf();
    pushXf(-11.0f, -108.0f, 1); buildingRow(100.0f, 12.0f, 16.0f, nullptr); popXf();
    pushXf(11.0f, -208.0f, 3); buildingRow(100.0f, 12.0f, 16.0f, nullptr); popXf();

    // stalls on the north sidewalk, right by the plaza
    pushXf(-20.0f, -12.9f, 0); chaiStall(SIDEWALK_H); popXf();
    pushXf(-36.0f, -12.9f, 0); vadaPavStall(SIDEWALK_H); popXf();
    pushXf(-47.0f, -12.9f, 0); paanShop(SIDEWALK_H); popXf();
    pushXf(-62.0f, -12.9f, 0); bhelStall(SIDEWALK_H); popXf();
    pushXf(30.0f, -12.9f, 0); fruitStall(SIDEWALK_H); popXf();
    // and on the south sidewalk, facing the road
    pushXf(-66.0f, 12.9f, 2); busStop(SIDEWALK_H); popXf();
    pushXf(40.0f, 12.9f, 2); busStop(SIDEWALK_H); popXf();
    pushXf(-95.0f, 12.9f, 2); fruitStall(SIDEWALK_H); popXf();
    pushXf(28.0f, 12.9f, 2); paanShop(SIDEWALK_H); popXf();
    pushXf(56.0f, 12.9f, 2); chaiStall(SIDEWALK_H); popXf();
    pushXf(80.0f, 12.9f, 2); bhelStall(SIDEWALK_H); popXf();
    pushXf(-120.0f, 12.9f, 2); vadaPavStall(SIDEWALK_H); popXf();
    pushXf(-30.0f, 12.9f, 2); chaiStall(SIDEWALK_H); popXf();

    buildPlaza();
    buildMuralWall();
    buildViaduct();
    buildStation();
    buildConstruction();
    buildSkyline();
    buildPeds();

    // invisible walls around the playable block
    g_xf.ox = 0; g_xf.oz = 0; g_xf.rot = 0;
    solidL(-600.0f, -600.0f, WORLD_MIN_X, 600.0f, 0, 80.0f);
    solidL(WORLD_MAX_X, -600.0f, 600.0f, 600.0f, 0, 80.0f);
    solidL(WORLD_MIN_X, -600.0f, WORLD_MAX_X, -86.0f, 0, 80.0f);
    solidL(WORLD_MIN_X, 30.0f, -11.0f, 104.0f, 0, 80.0f);
    solidL(11.0f, 30.0f, WORLD_MAX_X, 104.0f, 0, 80.0f);
    solidL(WORLD_MIN_X, SEAWALL_Z + 0.85f, WORLD_MAX_X, 600.0f, 0, 80.0f);  // nobody swims today
    // the buildings that face the promenade
    pushXf(-141.0f, PROM_Z0, 0); buildingRow(113.0f, 12.0f, 16.0f, "IRANI CAFE"); popXf();
    pushXf(28.0f, PROM_Z0, 0); buildingRow(113.0f, 12.0f, 16.0f, "KIRANA STORE"); popXf();
    buildSeaFace();
    buildMissionSpots();
    pruneDests();
}

