#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// City builder, part 3: the road to the sea
//
// South of the main road the cross road runs through a lane of shops (cross
// traffic U-turns just past the crossing) and opens onto the
// Sea Face promenade: a long seawall ledge, a stepped viewing deck with
// handrails, a manual pad, a kicker, snack carts and kids flying kites, with
// the Sea Link standing in the haze.
// ============================================================================


struct Kite {
    V3 anchor;  // the kid's hand
    V3 base;    // where the kite flies
    Col col;
};
static std::vector<Kite> g_kites;
static std::vector<V3> g_kitePickups;  // loose kites for Chintu's mission


static void seaLink() {
    // cable-stayed bridge far out in the bay
    const float z = 262.0f, deckY = 18.0f;
    Col grey = C(0.72f, 0.74f, 0.78f);
    setc(grey);
    box(-420.0f, deckY - 1.2f, z - 6.0f, 420.0f, deckY, z + 6.0f);
    for (float x = -400.0f; x <= 400.0f; x += 40.0f) box(x - 1.5f, -2.0f, z - 3.0f, x + 1.5f, deckY - 1.2f, z + 3.0f);
    for (int t = -1; t <= 1; t += 2) {
        float tx = (float)t * 70.0f;
        setc(grey);
        // inverted-Y pylon
        quad4(V3(tx - 6, deckY, z - 5), V3(tx - 1.5f, 110, z - 1), V3(tx - 1.5f, 110, z + 1), V3(tx - 6, deckY, z + 5));
        quad4(V3(tx + 6, deckY, z + 5), V3(tx + 1.5f, 110, z + 1), V3(tx + 1.5f, 110, z - 1), V3(tx + 6, deckY, z - 5));
        box(tx - 1.5f, 95.0f, z - 1.5f, tx + 1.5f, 125.0f, z + 1.5f);
        gLighting(false);
        setc(0.85f, 0.87f, 0.9f);
        gBegin(GL_LINES);
        for (int k = 1; k <= 12; k++) {
            float y = 100.0f + (float)k * 1.8f;
            for (int s = -1; s <= 1; s += 2) {
                gVertex(tx, y, z);
                gVertex(tx + (float)s * (8.0f + (float)k * 6.5f), deckY, z);
            }
        }
        gEnd();
        gLighting(true);
    }
}

static void tetrapod(float x, float y, float z, float rot) {
    setc(0.55f, 0.55f, 0.53f);
    gPush();
    gTranslate(x, y, z);
    gRotate(rot, 0, 1, 0);
    gRotate(rot * 0.7f, 1, 0, 0);
    V3 legs[4] = {V3(0, 1, 0), V3(0.94f, -0.33f, 0), V3(-0.47f, -0.33f, 0.82f), V3(-0.47f, -0.33f, -0.82f)};
    for (int i = 0; i < 4; i++) {
        V3 e = legs[i] * 0.9f;
        cylBetween(V3(0, 0, 0), e, 0.28f, 7);
    }
    gPop();
}

static void addKite(const V3& hand, const V3& fly, const Col& c) {
    Kite k = {hand, fly, c};
    g_kites.push_back(k);
}

static void buildSeaFace() {
    g_kites.clear();
    g_kitePickups.clear();

    // the lane of shops between the crossing and the sea
    for (float z = 64.0f; z <= 112.0f; z += 16.0f) {
        streetLight(7.45f, z, false, -1);
        streetLight(-7.45f, z + 8.0f, false, 1);
        palmTree(9.2f, SIDEWALK_H, z + 8.0f, frange(6.5f, 8.0f), true);
        palmTree(-9.2f, SIDEWALK_H, z, frange(6.5f, 8.0f), true);
    }
    pushXf(9.3f, 40.0f, 3);
    setc(0.3f, 0.3f, 0.3f);
    cylBetween(V3(0, SIDEWALK_H, 0.05f), V3(0, 3.2f, 0.05f), 0.05f, 6);
    solidL(-0.08f, -0.03f, 0.08f, 0.13f, 0, 3.2f);
    signBoard(-1.4f, 1.4f, 2.4f, 3.2f, 0.1f, "SEA FACE  >>", SIGNS[1]);
    popXf();

    // promenade paving, raised to sidewalk height, in two tones
    sidewalk(-141.0f, PROM_Z0, 141.0f, PROM_Z1, 1);
    for (float x = -140.0f; x < 140.0f; x += 4.0f) {
        setc((((int)((x + 200) / 4)) & 1) ? C(0.72f, 0.62f, 0.5f) : C(0.64f, 0.56f, 0.46f));
        rectY(x, 152.0f, x + 4.0f, PROM_Z1, SIDEWALK_H + 0.012f);
    }
    // seawall ledge: the long grind
    setc(0.66f, 0.64f, 0.6f);
    g_railTag = RT_SEAWALL;
    ledgeBoxL(-141.0f, SEAWALL_Z, 141.0f, SEAWALL_Z + 0.8f, 0, 0.75f);
    g_railTag = RT_NONE;
    setc(0.55f, 0.54f, 0.5f);
    box(-141.0f, -1.6f, SEAWALL_Z + 0.8f, 141.0f, 0.75f, SEAWALL_Z + 1.2f);
    // rocks, tetrapods and the sea
    for (int i = 0; i < 90; i++) {
        float x = frange(-160, 160), z = frange(SEAWALL_Z + 1.5f, SEAWALL_Z + 7.0f);
        tetrapod(x, frange(-1.3f, -0.6f), z, frange(0, 360));
    }
    setc(0.12f, 0.36f, 0.5f);
    rectY(-600.0f, SEAWALL_Z + 1.0f, 600.0f, 600.0f, -1.2f);
    gLighting(false);
    for (int i = 0; i < 260; i++) {
        float x = frange(-400, 400), z = frange(SEAWALL_Z + 8, 300);
        float w = frange(2, 9);
        setc(mixc(C(0.3f, 0.55f, 0.68f), C(0.85f, 0.9f, 0.92f), frand() * 0.6f));
        rectY(x, z, x + w, z + 0.25f, -1.18f);
    }
    gLighting(true);
    seaLink();

    // lamp posts along the wall, palms and benches along the paving
    for (float x = -134.0f; x <= 134.0f; x += 14.0f) {
        streetLight(x, 158.8f, true, -1);
        if (fabsf(x - 55.0f) > 20.0f) palmTree(x + 7.0f, SIDEWALK_H, 150.0f, frange(6.5f, 8.5f), true);
    }
    for (float x = -120.0f; x <= 120.0f; x += 30.0f) {
        if (fabsf(x - 55.0f) < 22.0f) continue;
        setc(0.5f, 0.36f, 0.22f);
        ledgeBoxL(x - 2.0f, 155.6f, x + 2.0f, 156.3f, 0, SIDEWALK_H + 0.45f);
    }

    // stepped viewing deck with handrails, like the steps at Carter Road
    const float dx0 = 40.0f, dx1 = 70.0f, dz0 = 146.0f, dz1 = 155.0f, top = 1.0f;
    setc(0.7f, 0.66f, 0.6f);
    box(dx0, 0, dz0, dx1, top, dz1);
    solidL(dx0, dz0, dx1, dz1, 0, top);
    for (int k = 0; k < 4; k++) {
        float z0 = 142.0f + (float)k, h = SIDEWALK_H + 0.205f * (float)(k + 1);
        setc(k & 1 ? C(0.74f, 0.7f, 0.63f) : C(0.68f, 0.64f, 0.58f));
        box(dx0, 0, z0, dx1, h, z0 + 1.0f);
    }
    rampL(dx0, 142.0f, dx1, 145.0f, 0, SIDEWALK_H + 0.205f, top, 2);
    solidL(dx0, 145.0f, dx1, 146.0f, 0, top);
    g_railTag = RT_SEAFACE;
    railL(dx0 + 0.1f, top, dz1 - 0.04f, dx1 - 0.1f, top, dz1 - 0.04f, true);
    railL(dx0 + 0.04f, top, 146.1f, dx0 + 0.04f, top, dz1 - 0.1f, true);
    railL(dx1 - 0.04f, top, 146.1f, dx1 - 0.04f, top, dz1 - 0.1f, true);
    setc(COL_STEEL);
    cylBetween(V3(dx0, top, dz1), V3(dx1, top, dz1), 0.035f, 5);
    for (int s = 0; s < 3; s++) {
        float x = s == 0 ? dx0 + 2.0f : (s == 1 ? 55.0f : dx1 - 2.0f);
        roundRail(V3(x, top + 0.9f, 145.8f), V3(x, SIDEWALK_H + 0.75f, 141.8f), COL_STEEL);
    }
    g_railTag = RT_NONE;
    pushXf(55.0f, dz1 - 0.2f, 2);  // faces the promenade and the steps
    signBoard(-9.0f, 9.0f, top + 1.6f, top + 2.6f, 0.0f, "SEA FACE PROMENADE", SIGNS[4]);
    popXf();
    setc(0.3f, 0.3f, 0.3f);
    cylBetween(V3(47.0f, top, dz1 - 0.1f), V3(47.0f, top + 1.6f, dz1 - 0.1f), 0.05f, 5);
    cylBetween(V3(63.0f, top, dz1 - 0.1f), V3(63.0f, top + 1.6f, dz1 - 0.1f), 0.05f, 5);
    solidL(46.9f, dz1 - 0.2f, 47.1f, dz1, top, top + 1.6f);
    solidL(62.9f, dz1 - 0.2f, 63.1f, dz1, top, top + 1.6f);

    // manual pad and a kicker on the paving
    setc(0.75f, 0.73f, 0.7f);
    ledgeBoxL(-30.0f, 136.0f, -20.0f, 138.0f, 0, SIDEWALK_H + 0.28f);
    railL(-29.96f, SIDEWALK_H + 0.28f, 136.1f, -29.96f, SIDEWALK_H + 0.28f, 137.9f, true);
    railL(-20.04f, SIDEWALK_H + 0.28f, 136.1f, -20.04f, SIDEWALK_H + 0.28f, 137.9f, true);
    wedge(-78.0f, 138.0f, -74.5f, 142.0f, SIDEWALK_H, SIDEWALK_H + 1.0f, -1, C(0.72f, 0.52f, 0.3f), C(0.5f, 0.35f, 0.2f), true);
    setc(COL_STEEL);
    cylBetween(V3(-78.0f, SIDEWALK_H + 1.0f, 138.0f), V3(-78.0f, SIDEWALK_H + 1.0f, 142.0f), 0.04f, 5);

    // snack carts facing the sea
    pushXf(-50.0f, 144.0f, 0); snackCart(SIDEWALK_H, C(0.2f, 0.2f, 0.2f), "BHUTTA 30/-", STALL_BHUTTA); popXf();
    pushXf(-5.0f, 144.0f, 0); snackCart(SIDEWALK_H, C(0.2f, 0.6f, 0.85f), "BARAF GOLA", STALL_GOLA); popXf();
    pushXf(90.0f, 144.0f, 0); chaiStall(SIDEWALK_H); popXf();
    pushXf(-100.0f, 144.0f, 0); bhelStall(SIDEWALK_H); popXf();

    // kids flying kites
    float kidX[4] = {-118.0f, -64.0f, 18.0f, 108.0f};
    for (int i = 0; i < 4; i++) {
        V3 at(kidX[i], SIDEWALK_H, 153.0f);
        addPed(at, at, PED_KID, false);
        g_peds.back().pos = at;
        g_peds.back().yaw = 0;
        addKite(at + V3(0.2f, 0.95f, 0.3f), at + V3(frange(-8, 8), frange(16, 24), frange(18, 30)), SAREES[i * 2 % 8]);
    }
    // strollers on the promenade
    addPed(V3(-130.0f, SIDEWALK_H, 157.5f), V3(-20.0f, SIDEWALK_H, 157.5f), PED_SAREE, true);
    addPed(V3(0.0f, SIDEWALK_H, 157.2f), V3(130.0f, SIDEWALK_H, 157.2f), PED_MAN, true);
    addPed(V3(-60.0f, SIDEWALK_H, 131.0f), V3(60.0f, SIDEWALK_H, 131.0f), PED_SALWAR, true);
    addPed(V3(-3.0f, SIDEWALK_H, 60.0f), V3(-3.0f, SIDEWALK_H, 118.0f), PED_MAN, true);

    // loose kites for Chintu, caught on things around the sea face
    g_kitePickups = {V3(55.0f, 2.4f, 150.0f), V3(-76.0f, 2.8f, 140.0f), V3(-25.0f, 1.4f, 137.0f),
                     V3(120.0f, 1.9f, 160.4f), V3(-9.0f, 1.6f, 90.0f)};

    // named drop points out here
    g_xf.ox = 0; g_xf.oz = 0; g_xf.rot = 0;
    addDestL("SEA FACE VIEWING DECK", 55.0f, 150.0f, 1.0f, 0);
    addDestL("KITE KID CHINTU", 30.0f, 140.0f, SIDEWALK_H, 0);
}
