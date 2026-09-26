#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Rendering: skater, dynamic objects, sky, camera, HUD
// ============================================================================

static int g_winW = 1280, g_winH = 720;
static Frustum g_frustum;  // of the current frame, for culling
static Mesh g_playerMesh, g_friendMesh[5];  // riders posed this frame, in world space
static float g_time = 0;
static int g_cut = 0;  // cutscene playing (CutKind in cutscene.hpp), 0 for none
static bool g_showHelp = true;
static int g_camMode = 0;
static V3 g_camPos, g_camLook;
static float g_camYaw = 0;
static bool g_camInit = false;

// Clothes and colours for a rider. The player's outfit comes from the shop;
// friends each have their own.
struct Outfit {
    Col shirt, pants, shoes, cap, skin, stripe;
    bool capOn, ponytail, longTop;
    int hat, glasses, neck, bag;  // accessory gear ids, -1 for none
};
static Outfit g_fit;
static float g_drawPushPhase = 0, g_drawPushDir = 1, g_drawBalance = 0;
static Col g_deckA = {0.95f, 0.35f, 0.15f}, g_deckB = {0.95f, 0.35f, 0.15f};

static Outfit makeOutfit(const Col& shirt, const Col& pants, const Col& cap, const Col& skin, const Col& stripe) {
    Outfit o;
    o.shirt = shirt;
    o.pants = pants;
    o.shoes = C(0.92f, 0.92f, 0.9f);
    o.cap = cap;
    o.skin = skin;
    o.stripe = stripe;
    o.capOn = true;
    o.ponytail = false;
    o.longTop = false;
    o.hat = o.glasses = o.neck = o.bag = -1;
    return o;
}

// Fills in the accessories the player is wearing. Called after the outfit is chosen.
static void wearAccessories(Outfit& o) {
    o.hat = g_hat;
    o.glasses = g_glasses;
    o.neck = g_neck;
    o.bag = g_bag;
    if (o.hat >= 0) o.capOn = false;
}

static Outfit playerOutfit() {
    Outfit o = makeOutfit(C(0.96f, 0.5f, 0.1f), C(0.18f, 0.27f, 0.5f), C(0.1f, 0.55f, 0.45f), COL_SKIN[0], COL_WHITE);
    if (g_outfit == FIT_JERSEY) {
        o.shirt = C(0.1f, 0.3f, 0.75f);
        o.stripe = C(0.95f, 0.75f, 0.2f);
        o.cap = C(0.1f, 0.2f, 0.5f);
    } else if (g_outfit == FIT_KURTA) {
        o.shirt = C(0.95f, 0.92f, 0.82f);
        o.stripe = C(0.9f, 0.3f, 0.1f);
        o.pants = C(0.9f, 0.88f, 0.8f);
        o.longTop = true;
        o.capOn = false;
    } else if (g_outfit == FIT_RAINCOAT) {
        o.shirt = C(0.98f, 0.85f, 0.1f);
        o.stripe = C(0.2f, 0.2f, 0.2f);
        o.cap = C(0.98f, 0.85f, 0.1f);
        o.longTop = true;
    }
    wearAccessories(o);
    return o;
}

static void setDeck(int deck) {
    switch (deck) {
        case DECK_KAALIPEELI: g_deckA = C(0.08f, 0.08f, 0.08f); g_deckB = C(0.98f, 0.8f, 0.05f); break;
        case DECK_BEST: g_deckA = C(0.78f, 0.1f, 0.08f); g_deckB = C(0.95f, 0.88f, 0.7f); break;
        case DECK_WARLI: g_deckA = C(0.62f, 0.3f, 0.18f); g_deckB = C(0.97f, 0.95f, 0.9f); break;
        case DECK_GOLD: g_deckA = C(0.95f, 0.75f, 0.2f); g_deckB = C(1.0f, 0.95f, 0.6f); break;
        default: g_deckA = C(0.95f, 0.35f, 0.15f); g_deckB = C(0.95f, 0.35f, 0.15f); break;
    }
}

// Board in its own frame: nose toward +Z, deck top at y = 0.1.
static void drawBoard(float wheelSpin) {
    setc(0.08f, 0.08f, 0.08f);
    box(-0.105f, 0.08f, -0.33f, 0.105f, 0.1f, 0.33f);
    setc(g_deckA);
    box(-0.105f, 0.066f, -0.33f, 0.105f, 0.08f, 0.33f);
    setc(g_deckB);
    box(-0.03f, 0.064f, -0.3f, 0.03f, 0.066f, 0.3f);
    for (int s = -1; s <= 1; s += 2) {
        gPush();
        gTranslate(0, 0.083f, (float)s * 0.33f);
        gRotate((float)s * -20.0f, 1, 0, 0);
        setc(0.08f, 0.08f, 0.08f);
        if (s > 0) box(-0.1f, 0.0f, 0, 0.1f, 0.017f, 0.11f);
        else box(-0.1f, 0.0f, -0.11f, 0.1f, 0.017f, 0);
        setc(g_deckA);
        if (s > 0) box(-0.1f, -0.012f, 0, 0.1f, 0.0f, 0.11f);
        else box(-0.1f, -0.012f, -0.11f, 0.1f, 0.0f, 0);
        gPop();
        setc(COL_STEEL);
        box(-0.075f, 0.04f, (float)s * 0.21f - 0.025f, 0.075f, 0.066f, (float)s * 0.21f + 0.025f);
        box(-0.12f, 0.028f, (float)s * 0.21f - 0.008f, 0.12f, 0.04f, (float)s * 0.21f + 0.008f);
        for (int w = -1; w <= 1; w += 2) {
            gPush();
            gTranslate((float)w * 0.115f, 0.032f, (float)s * 0.21f);
            gRotate(90, 0, 0, 1);
            gRotate(wheelSpin * RAD2DEG, 0, 1, 0);
            gTranslate(0, -0.02f, 0);
            setc(0.95f, 0.92f, 0.8f);
            cyl(0.032f, 0.032f, 0.04f, 8, true);
            gPop();
        }
    }
}


// Rider in the body frame: facing +Z, the board nose toward +X, feet on the deck at footY.
// Pose: 0 riding, 1 pushing, 2 air, 3 balancing, 4 indy grab, 5 melon grab.
static void drawRider(float footY, float crouch, int pose) {
    const Col& skin = g_fit.skin;
    float c = clampf(crouch, 0, 1);
    V3 footL(0.2f, footY + 0.05f, 0.0f), footR(-0.2f, footY + 0.05f, 0.0f);
    float armSwing = 0;
    if (pose == 1) {
        // one kick-push stroke: step down in front, drag back along the ground, lift and return
        float p = g_drawPushPhase, dir = g_drawPushDir;
        V3 deck(-0.2f * dir, footY + 0.05f, 0.0f);
        V3 front(0.12f * dir, 0.05f, 0.24f), back(-0.52f * dir, 0.05f, 0.24f);
        V3 f;
        if (p < 0.15f) {
            float t = p / 0.15f;
            f = deck + (front - deck) * t;
            f.y += sinf(t * PI) * 0.1f;
        } else if (p < 0.6f) {
            float t = (p - 0.15f) / 0.45f;
            f = front + (back - front) * t;
            c = std::max(c, 0.25f * sinf(t * PI));  // the standing leg bends as you push
        } else {
            float t = (p - 0.6f) / 0.4f;
            f = back + (deck - back) * t;
            f.y += sinf(t * PI) * 0.16f;
        }
        if (dir > 0) {
            footR = f;
            footL.x = 0.14f;  // weight over the front truck
        } else {
            footL = f;
            footR.x = -0.14f;
        }
        armSwing = sinf(p * TWO_PI) * 0.18f;
    }
    float hipY = footY + 0.9f - 0.38f * c;
    float kz = 0.1f + 0.25f * c;
    V3 hipL(0.1f, hipY, 0), hipR(-0.1f, hipY, 0);
    V3 kneeL = (footL + hipL) * 0.5f + V3(0.03f, 0, kz), kneeR = (footR + hipR) * 0.5f + V3(-0.03f, 0, kz);
    // legs: tapered, slightly baggy pants with knees and cuffs
    Col pants = g_fit.pants;
    setc(pants);
    coneBetween(hipL, kneeL, 0.085f, 0.07f);
    coneBetween(kneeL, footL + V3(0, 0.07f, 0), 0.068f, 0.062f);
    coneBetween(hipR, kneeR, 0.085f, 0.07f);
    coneBetween(kneeR, footR + V3(0, 0.07f, 0), 0.068f, 0.062f);
    ball(kneeL, 0.071f, pants);
    ball(kneeR, 0.071f, pants);
    sneaker(footL, mulc(g_fit.shoes, 0.85f));
    sneaker(footR, mulc(g_fit.shoes, 0.85f));
    // torso leans forward as the rider crouches
    float lean = 0.15f + 0.45f * c;
    V3 top(0, hipY + cosf(lean) * 0.56f, sinf(lean) * 0.56f);
    gPush();
    gTranslate(0, hipY, 0);
    gRotate(lean * RAD2DEG, 1, 0, 0);
    // hips
    setc(pants);
    gPush();
    gTranslate(0, 0.02f, 0);
    gScale(1.0f, 0.55f, 0.7f);
    sphere(0.17f, 10, 6);
    gPop();
    // chest: an oval that widens toward the shoulders
    setc(g_fit.shirt);
    gPush();
    gScale(1.0f, 1.0f, 0.62f);
    float hem = g_fit.longTop ? -0.3f : 0.02f;
    gTranslate(0, hem, 0);
    cyl(g_fit.longTop ? 0.19f : 0.155f, 0.19f, 0.54f - hem, 12, false);
    gTranslate(0, 0.54f - hem, 0);
    cyl(0.19f, 0.07f, 0.06f, 12, true);  // shoulder line into the neck
    gPop();
    setc(g_fit.stripe);
    gPush();
    gTranslate(0, 0.3f, 0);
    gScale(1.0f, 1.0f, 0.62f);
    cyl(0.183f, 0.185f, 0.06f, 12, false);
    gPop();
    setc(mulc(g_fit.shirt, 0.78f));
    gPush();
    gTranslate(0, 0.57f, 0);
    cyl(0.07f, 0.06f, 0.03f, 10, false);  // collar
    gPop();
    // things worn on the body: chest faces +Z in this frame
    if (g_fit.neck == NECK_CHAIN) {
        setc(1.0f, 0.8f, 0.25f);
        for (int k = 0; k < 12; k++) {
            float a0 = TWO_PI * (float)k / 12.0f, a1 = TWO_PI * (float)(k + 1) / 12.0f;
            float d0 = 0.03f * std::max(0.0f, sinf(a0)), d1 = 0.03f * std::max(0.0f, sinf(a1));  // sags at the front
            cylBetween(V3(cosf(a0) * 0.11f, 0.575f - d0 * 3.0f, sinf(a0) * 0.1f),
                       V3(cosf(a1) * 0.11f, 0.575f - d1 * 3.0f, sinf(a1) * 0.1f), 0.012f, 4);
        }
        box(-0.025f, 0.44f, 0.105f, 0.025f, 0.49f, 0.12f);
    } else if (g_fit.neck == NECK_GAMCHA) {
        setc(0.85f, 0.15f, 0.12f);
        gPush();
        gTranslate(0, 0.52f, 0);
        gScale(1.0f, 1.0f, 0.8f);
        cyl(0.13f, 0.1f, 0.08f, 10, true);
        gPop();
        box(0.05f, 0.28f, 0.1f, 0.13f, 0.56f, 0.125f);
        setc(COL_WHITE);
        for (int k = 0; k < 4; k++) box(0.05f, 0.3f + (float)k * 0.065f, 0.126f, 0.13f, 0.32f + (float)k * 0.065f, 0.129f);
    }
    if (g_fit.bag == BAG_BACKPACK) {
        setc(0.1f, 0.45f, 0.45f);
        box(-0.15f, 0.1f, -0.27f, 0.15f, 0.52f, -0.1f);
        setc(0.95f, 0.75f, 0.2f);
        box(-0.12f, 0.16f, -0.28f, 0.12f, 0.2f, -0.27f);
        setc(0.08f, 0.3f, 0.3f);
        box(-0.12f, 0.1f, 0.105f, -0.08f, 0.56f, 0.115f);
        box(0.08f, 0.1f, 0.105f, 0.12f, 0.56f, 0.115f);
    }
    gPop();
    // neck
    setc(skin);
    coneBetween(top + V3(0, -0.03f, 0), top + V3(0, 0.1f, 0.01f), 0.052f, 0.048f);
    // head, turned to look where the board is going
    gPush();
    gTranslate(top.x, top.y + 0.17f, top.z + 0.02f);
    gRotate(70, 0, 1, 0);
    setc(skin);
    gPush();
    gScale(0.93f, 1.08f, 1.0f);
    sphere(0.12f, 12, 9);
    gPop();
    drawFace(skin);
    // hair: a cap of hair over the top and back
    setc(0.06f, 0.05f, 0.05f);
    gPush();
    gTranslate(0, 0.035f, -0.022f);
    gScale(0.95f, 0.9f, 1.0f);
    sphere(0.121f, 12, 8);
    gPop();
    box(-0.08f, 0.06f, 0.07f, 0.08f, 0.1f, 0.105f);  // fringe
    if (g_fit.ponytail) {
        gPush();
        gTranslate(0, -0.04f, -0.14f);
        sphere(0.07f, 7, 5);
        gTranslate(0, -0.1f, -0.03f);
        sphere(0.05f, 6, 4);
        gPop();
    }
    if (g_fit.capOn) {
        setc(g_fit.cap);
        gTranslate(0, 0.045f, -0.01f);
        gScale(1, 0.62f, 1);
        sphere(0.128f, 10, 6);
    }
    gPop();
    if (g_fit.capOn) {
        gPush();
        gTranslate(top.x, top.y + 0.23f, top.z);
        gRotate(70, 0, 1, 0);
        setc(g_fit.cap);
        box(-0.08f, 0, -0.26f, 0.08f, 0.02f, -0.1f);  // cap worn backwards
        gPop();
    }
    // hats and glasses, in the head's frame (face toward +Z)
    if (g_fit.hat >= 0 || g_fit.glasses >= 0) {
        gPush();
        gTranslate(top.x, top.y + 0.17f, top.z + 0.02f);
        gRotate(70, 0, 1, 0);
        switch (g_fit.hat) {
            case HAT_SNAPBACK:
                setc(0.85f, 0.12f, 0.12f);
                gPush();
                gTranslate(0, 0.055f, -0.02f);
                gScale(1, 0.62f, 1);
                sphere(0.13f, 10, 6);
                gPop();
                box(-0.085f, 0.06f, 0.08f, 0.085f, 0.08f, 0.25f);
                break;
            case HAT_BUCKET:
                setc(0.62f, 0.58f, 0.42f);
                gPush();
                gTranslate(0, 0.065f, -0.01f);
                cyl(0.2f, 0.19f, 0.025f, 14, true);
                cyl(0.132f, 0.11f, 0.12f, 12, true);
                gPop();
                break;
            case HAT_TOPI:
                setc(0.97f, 0.97f, 0.95f);
                box(-0.06f, 0.08f, -0.13f, 0.06f, 0.16f, 0.12f);
                box(-0.02f, 0.16f, -0.12f, 0.02f, 0.18f, 0.11f);
                break;
            case HAT_BEANIE:
                setc(0.55f, 0.1f, 0.15f);
                gPush();
                gTranslate(0, 0.05f, -0.015f);
                gScale(1, 0.8f, 1);
                sphere(0.132f, 10, 6);
                gPop();
                setc(0.45f, 0.08f, 0.12f);
                gPush();
                gTranslate(0, -0.005f, -0.015f);
                cyl(0.134f, 0.134f, 0.05f, 12, false);
                gPop();
                break;
            default: break;
        }
        if (g_fit.glasses == GLASSES_AVIATOR) {
            setc(0.05f, 0.05f, 0.06f);
            discZ(-0.045f, 0.018f, 0.034f, 0.125f, 10);
            discZ(0.045f, 0.018f, 0.034f, 0.125f, 10);
            setc(1.0f, 0.8f, 0.25f);
            box(-0.015f, 0.032f, 0.122f, 0.015f, 0.038f, 0.127f);
            box(-0.12f, 0.03f, -0.02f, -0.113f, 0.037f, 0.12f);
            box(0.113f, 0.03f, -0.02f, 0.12f, 0.037f, 0.12f);
        } else if (g_fit.glasses == GLASSES_ROUND) {
            setc(0.08f, 0.06f, 0.05f);
            discZ(-0.043f, 0.02f, 0.031f, 0.124f, 12);
            discZ(0.043f, 0.02f, 0.031f, 0.124f, 12);
            setc(0.72f, 0.82f, 0.88f);
            discZ(-0.043f, 0.02f, 0.024f, 0.1255f, 12);
            discZ(0.043f, 0.02f, 0.024f, 0.1255f, 12);
            setc(0.08f, 0.06f, 0.05f);
            box(-0.015f, 0.028f, 0.122f, 0.015f, 0.033f, 0.126f);
        }
        gPop();
    }
    // arms: short sleeves (full ones on long tops), bare forearms and hands
    V3 shL = top + V3(0.2f, -0.07f, -0.02f), shR = top + V3(-0.2f, -0.07f, -0.02f);
    V3 handL, handR;
    float b = g_drawBalance;
    switch (pose) {
        case 2:  // air
            handL = shL + V3(0.3f, -0.15f, 0.1f);
            handR = shR + V3(-0.3f, -0.15f, 0.1f);
            break;
        case 3:  // balancing on a rail or in a manual
            handL = shL + V3(0.45f, -0.05f - b * 0.3f, 0.05f);
            handR = shR + V3(-0.45f, -0.05f + b * 0.3f, 0.05f);
            break;
        case 4:  // indy grab: back hand to the toe edge
            handL = shL + V3(0.3f, 0.25f, 0.1f);
            handR = V3(-0.02f, footY + 0.06f, 0.14f);
            break;
        case 5:  // melon grab: front hand to the heel edge
            handL = V3(0.05f, footY + 0.06f, -0.13f);
            handR = shR + V3(-0.3f, 0.25f, 0.1f);
            break;
        default:
            handL = shL + V3(0.1f, -0.5f, 0.05f + armSwing);
            handR = shR + V3(-0.1f, -0.5f, 0.05f - armSwing);
    }
    V3 elL = (shL + handL) * 0.5f + V3(0.05f, -0.05f, -0.05f);
    V3 elR = (shR + handR) * 0.5f + V3(-0.05f, -0.05f, -0.05f);
    const V3* sh[2] = {&shL, &shR};
    const V3* el[2] = {&elL, &elR};
    const V3* hd[2] = {&handL, &handR};
    for (int k = 0; k < 2; k++) {
        ball(*sh[k], 0.07f, g_fit.shirt);
        V3 sleeveEnd = g_fit.longTop ? *el[k] : *sh[k] + (*el[k] - *sh[k]) * 0.6f;
        setc(g_fit.shirt);
        coneBetween(*sh[k], sleeveEnd, 0.066f, 0.06f);
        setc(skin);
        coneBetween(sleeveEnd, *el[k], 0.047f, 0.044f);
        ball(*el[k], 0.045f, g_fit.longTop ? g_fit.shirt : skin);
        setc(g_fit.longTop ? g_fit.shirt : skin);
        coneBetween(*el[k], *hd[k], 0.043f, 0.036f);
        ball(*hd[k], 0.046f, skin);
    }
}

static void drawCarried();
static void drawClouds();
static void drawStars();
static float g_tiltP = 0, g_tiltR = 0;  // board pitch and roll following the surface, degrees

// How steep the surface under the skater is, along and across the board.
static void surfaceTilt(float& pitch, float& roll) {
    pitch = roll = 0;
    V3 f = yawDir(P.yaw), r(cosf(P.yaw), 0, -sinf(P.yaw));
    if (P.state == P_GROUND) {
        GroundInfo gi = groundAt(P.pos.x, P.pos.z, P.pos.y + 0.05f);
        pitch = atanf(gi.gx * f.x + gi.gz * f.z) * RAD2DEG;
        roll = atanf(gi.gx * r.x + gi.gz * r.z) * RAD2DEG;
    } else if (P.state == P_GRIND && P.rail >= 0) {
        V3 d = g_rails[(size_t)P.rail].b - g_rails[(size_t)P.rail].a;
        float h = sqrtf(d.x * d.x + d.z * d.z);
        if (h < 1e-3f) return;
        float slope = d.y / h;
        V3 dh(d.x / h, 0, d.z / h);
        pitch = atanf(slope * dot(f, dh)) * RAD2DEG;
        roll = atanf(slope * dot(r, dh)) * RAD2DEG;
    }
}

static void drawSkater() {
    float wheelSpin = g_time * hspeed() * 3.0f;
    g_fit = playerOutfit();
    setDeck(g_deck);
    g_drawPushPhase = P.pushPhase;
    g_drawPushDir = P.pushDir == 0 ? 1.0f : P.pushDir;
    g_drawBalance = P.balance;
    if (P.state == P_BAIL) {
        gPush();
        gTranslate(P.boardPos.x, P.boardPos.y, P.boardPos.z);
        gRotate(P.yaw * RAD2DEG + P.boardSpin * 40.0f, 0, 1, 0);
        gRotate(P.boardSpin * 70.0f, 0, 0, 1);
        drawBoard(wheelSpin);
        gPop();
        gPush();
        gTranslate(P.pos.x, P.pos.y, P.pos.z);
        gRotate(P.yaw * RAD2DEG - 90.0f, 0, 1, 0);
        float fall = std::min(1.0f, P.bailT * 2.5f);
        gTranslate(0, 0.15f * fall, -0.3f * fall);
        gRotate(-82.0f * fall, 1, 0, 0);
        gTranslate(0, -0.1f, 0);
        drawRider(0.0f, 0.3f, 2);
        gPop();
        return;
    }
    gPush();
    gTranslate(P.pos.x, P.pos.y, P.pos.z);
    gRotate(P.yaw * RAD2DEG, 0, 1, 0);
    float wantP, wantR;
    surfaceTilt(wantP, wantR);
    g_tiltP = lerpf(g_tiltP, wantP, 0.35f);
    g_tiltR = lerpf(g_tiltR, wantR, 0.35f);
    gRotate(-g_tiltP, 1, 0, 0);
    gRotate(g_tiltR, 0, 0, 1);
    if (P.backflipOn) {
        gTranslate(0, 0.9f, 0);
        gRotate(-P.backflip, 0, 0, 1);
        gTranslate(0, -0.9f, 0);
    }
    gRotate(P.slideAng, 0, 1, 0);
    gRotate(-P.lean, 0, 0, 1);
    float lift = 0, pitch = 0, pivot = 0;
    int pose = 0;
    float crouch = P.crouch;
    if (P.state == P_GROUND) {
        if (P.pushing) pose = 1;
        if (P.manual) {
            pose = 3;
            pitch = P.noseManual ? 11.0f : -11.0f;
            pivot = P.noseManual ? 0.3f : -0.3f;
            crouch = 0.35f;
        }
        if (P.landT < 0.25f) crouch = std::max(crouch, 1.0f - P.landT * 4.0f);
    } else if (P.state == P_AIR) {
        pose = 2;
        crouch = std::max(crouch, 0.55f);
        if (P.grab) {
            pose = P.grab == 1 ? 4 : 5;
            crouch = 1.0f;
            lift = 0.22f * std::min(1.0f, P.grabTime * 8.0f);
            pitch = P.grab == 1 ? -8.0f : 8.0f;
        }
    } else if (P.state == P_GRIND) {
        pose = 3;
        crouch = 0.45f;
        if (P.grindType == G_NOSEGRIND || P.grindType == G_CROOKED) { pitch = 12.0f; pivot = 0.3f; }
        if (P.grindType == G_50) { pitch = -12.0f; pivot = -0.3f; }
    }
    // board
    gPush();
    gTranslate(0, lift, pivot);
    gRotate(pitch, 1, 0, 0);
    gTranslate(0, 0, -pivot);
    gRotate(P.shove, 0, 1, 0);
    gTranslate(0, 0.07f, 0);
    gRotate(P.roll, 0, 0, 1);
    gTranslate(0, -0.07f, 0);
    drawBoard(wheelSpin);
    gPop();
    // rider: body turned 90 degrees so the chest faces the toe edge
    gPush();
    gRotate(-90.0f, 0, 1, 0);
    float flipLift = (fabsf(P.roll - P.rollTarget) > 1.0f || fabsf(P.shove - P.shoveTarget) > 1.0f) ? 0.12f : 0.0f;
    drawRider(0.1f + lift + flipLift, crouch, pose);
    gPop();
    drawCarried();
    gPop();
}

static void blobShadow(float x, float z, float y, float r, float alpha) {
    glColor4f(0, 0, 0, alpha);
    gBegin(GL_TRIANGLE_FAN);
    gVertex(x, y, z);
    for (int i = 0; i <= 16; i++) {
        float a = TWO_PI * (float)i / 16.0f;
        gVertex(x + cosf(a) * r, y, z + sinf(a) * r);
    }
    gEnd();
}

// A faint contact blob under the skater, so the board still looks grounded on
// ramps where the flat sun shadow is hidden.
static void drawShadows() {
    gLighting(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-2.0f, -4.0f);
    float gy = groundAt(P.pos.x, P.pos.z, P.pos.y + 0.1f).h;
    float h = P.pos.y - gy;
    float base = G().worldShadows ? 0.22f : 0.4f;  // on low this blob is the only shadow
    blobShadow(P.pos.x, P.pos.z, gy + 0.02f, 0.4f + h * 0.05f, std::max(0.05f, base - h * 0.05f));
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    gLighting(true);
}

static void drawSignals() {
    gLighting(false);
    for (size_t i = 0; i < g_signals.size(); i++) {
        const Signal& s = g_signals[i];
        int st = lightState(s.axis);
        gPush();
        gTranslate(s.x, SIDEWALK_H, s.z);
        gRotate(s.yaw * RAD2DEG, 0, 1, 0);
        float ys[3] = {3.85f, 3.55f, 3.25f};
        Col on[3] = {{1.0f, 0.15f, 0.1f}, {1.0f, 0.7f, 0.1f}, {0.2f, 1.0f, 0.4f}};
        int lit = st == 0 ? 2 : (st == 1 ? 1 : 0);
        for (int k = 0; k < 3; k++) {
            if (k == lit) setc(on[k]);
            else setc(mulc(on[k], 0.22f));
            discZ(0, ys[k], 0.11f, 0.41f, 12);
        }
        gPop();
    }
    gLighting(true);
}

static void drawLetters() {
    for (size_t i = 0; i < g_letters.size(); i++) {
        const Letter& l = g_letters[i];
        if (l.got) continue;
        float bob = sinf(g_time * 2.0f + (float)i) * 0.15f;
        // light beam so letters can be spotted from across the block
        gLighting(false);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);
        glColor4f(1.0f, 0.85f, 0.3f, 0.18f);
        gPush();
        gTranslate(l.pos.x, l.pos.y - 1.0f, l.pos.z);
        cyl(0.25f, 0.25f, 25.0f, 10, false);
        gPop();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        gLighting(true);
        gPush();
        gTranslate(l.pos.x, l.pos.y + bob, l.pos.z);
        gRotate(g_time * 120.0f + (float)i * 40.0f, 0, 1, 0);
        setc(0.1f, 0.6f, 0.55f);
        discZ(0, 0, 0.55f, -0.02f, 20);
        setc(1.0f, 0.8f, 0.2f);
        gBegin(GL_QUAD_STRIP);
        for (int k = 0; k <= 24; k++) {
            float a = TWO_PI * (float)k / 24.0f;
            gNormal(0, 0, 1);
            gVertex(cosf(a) * 0.55f, sinf(a) * 0.55f, 0);
            gVertex(cosf(a) * 0.68f, sinf(a) * 0.68f, 0);
        }
        gEnd();
        char s[2] = {l.ch, 0};
        gTranslate(0, -0.3f, 0.01f);
        text3D(s, 0.6f, COL_WHITE, 1.3f);
        gTranslate(0, 0, -0.04f);
        gRotate(180, 0, 1, 0);
        text3D(s, 0.6f, COL_WHITE, 1.3f);
        gPop();
    }
}

// ---------------------------------------------------------------- friends, markers and barricades

static Outfit friendOutfit(int id) {
    switch (id) {
        case MIS_RAJU: return makeOutfit(C(0.85f, 0.15f, 0.15f), C(0.2f, 0.2f, 0.22f), C(0.98f, 0.8f, 0.1f), COL_SKIN[1], COL_BLACK);
        case MIS_PRIYA: {
            Outfit o = makeOutfit(C(0.55f, 0.2f, 0.6f), C(0.25f, 0.35f, 0.6f), C(0, 0, 0), COL_SKIN[2], C(1.0f, 0.5f, 0.7f));
            o.capOn = false;
            o.ponytail = true;
            o.glasses = GLASSES_ROUND;
            return o;
        }
        default: {
            Outfit o = makeOutfit(C(0.25f, 0.5f, 0.3f), C(0.6f, 0.55f, 0.4f), COL_BLACK, COL_SKIN[3], C(0.9f, 0.9f, 0.9f));
            o.longTop = true;
            o.glasses = GLASSES_AVIATOR;
            o.bag = BAG_BACKPACK;
            return o;
        }
    }
}
static const int FRIEND_DECKS[5] = {DECK_BEST, DECK_WARLI, DECK_KAALIPEELI, -1, -1};

// Text on a plane that turns to face the camera.
static void billboardText(const char* s, const V3& at, float h, const Col& c) {
    gPush();
    gTranslate(at.x, at.y, at.z);
    gRotate((g_camYaw + PI) * RAD2DEG, 0, 1, 0);
    text3D(s, h, c, 1.4f);
    gPop();
}

static void drawFriend(const Friend& f) {
    if (!f.skater) return;
    g_fit = friendOutfit(f.id);
    setDeck(FRIEND_DECKS[f.id]);
    g_drawPushPhase = fmodf(f.pushT / 0.7f, 1.0f);
    g_drawPushDir = 1;
    g_drawBalance = 0;
    gPush();
    gTranslate(f.pos.x, f.pos.y, f.pos.z);
    gRotate(f.yaw * RAD2DEG, 0, 1, 0);
    if (!f.air) {
        GroundInfo gi = groundAt(f.pos.x, f.pos.z, f.pos.y + 0.05f);
        V3 fw = yawDir(f.yaw), rt(cosf(f.yaw), 0, -sinf(f.yaw));
        gRotate(-atanf(gi.gx * fw.x + gi.gz * fw.z) * RAD2DEG, 1, 0, 0);
        gRotate(atanf(gi.gx * rt.x + gi.gz * rt.z) * RAD2DEG, 0, 0, 1);
    }
    gPush();
    gTranslate(0, 0.07f, 0);
    gRotate(f.roll, 0, 0, 1);
    gTranslate(0, -0.07f, 0);
    drawBoard(g_time * f.speed * 3.0f);
    gPop();
    gRotate(-90.0f, 0, 1, 0);
    int pose = f.air ? 2 : (f.speed > 0.5f && f.speed < 6.0f ? 1 : 0);
    drawRider(0.1f + (f.air && f.roll > 1.0f ? 0.12f : 0.0f), f.air ? 0.6f : 0.15f, pose);
    if (f.id == MIS_SAM) {
        // Sam always has the camera out
        setc(0.1f, 0.1f, 0.1f);
        box(0.25f, 1.05f, 0.25f, 0.4f, 1.17f, 0.45f);
        setc(0.3f, 0.5f, 0.9f);
        box(0.28f, 1.08f, 0.45f, 0.37f, 1.14f, 0.47f);
    }
    gPop();
}

// Floating markers for people you can talk to, the shop and the stalls.
static void drawTalkMarkers() {
    float bob = sinf(g_time * 3.0f) * 0.12f;
    for (size_t i = 0; i < g_friends.size(); i++) {
        const Friend& f = g_friends[i];
        if (f.racing || f.follow || g_cut) continue;  // no labels on the crew riding with you, or in the films
        V3 top = f.pos + V3(0, 2.25f + bob, 0);
        bool open = g_unlockLevel >= f.needLevel;
        if (g_friend[f.id]) {
            billboardText(f.name, top - V3(0, 0.1f, 0), 0.22f, C(0.7f, 0.95f, 1.0f));
        } else if (open && !g_mis.on) {
            // spinning yellow diamond: this friend has a mission for you
            gPush();
            gTranslate(top.x, top.y + 0.55f, top.z);
            gRotate(g_time * 140.0f, 0, 1, 0);
            setc(1.0f, 0.8f, 0.1f);
            V3 up(0, 0.32f, 0), dn(0, -0.32f, 0);
            V3 ring[4] = {V3(0.2f, 0, 0), V3(0, 0, 0.2f), V3(-0.2f, 0, 0), V3(0, 0, -0.2f)};
            for (int k = 0; k < 4; k++) {
                tri3(up, ring[k], ring[(k + 1) & 3]);
                tri3(dn, ring[(k + 1) & 3], ring[k]);
            }
            gPop();
            billboardText(f.name, top - V3(0, 0.1f, 0), 0.2f, COL_WHITE);
        } else {
            billboardText(f.name, top - V3(0, 0.1f, 0), 0.2f, C(0.6f, 0.6f, 0.62f));
        }
    }
    billboardText("SHOP", g_shopPos + V3(0, 3.1f + bob, 0), 0.4f, C(1.0f, 0.5f, 0.8f));
    if (!g_mis.on) billboardText("JOBS", g_boardPos + V3(0, 3.2f + bob, 0), 0.4f, C(1.0f, 0.85f, 0.3f));
    if (!g_mis.on)
        for (size_t i = 0; i < g_stalls.size(); i++) {
            const Stall& s = g_stalls[i];
            float d = len(s.stand - P.pos);
            if (d > 45.0f || d < 5.0f) continue;  // up close the on-screen prompt takes over
            billboardText("RS", s.stand + V3(0, 2.9f + bob, 0), 0.3f, C(0.5f, 1.0f, 0.6f));
        }
}

// Vertical light beam, drawn additive.
static void beam(const V3& base, const Col& c, float r, float h, float a) {
    gLighting(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    glColor4f(c.r, c.g, c.b, a);
    gPush();
    gTranslate(base.x, base.y, base.z);
    cyl(r, r, h, 12, false);
    gPop();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    gLighting(true);
}

static void groundRing(const V3& at, float r, const Col& c) {
    gLighting(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glDepthMask(GL_FALSE);
    float pulse = 0.5f + 0.5f * sinf(g_time * 4.0f);
    glColor4f(c.r, c.g, c.b, 0.35f + 0.3f * pulse);
    gBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 32; i++) {
        float a = TWO_PI * (float)i / 32.0f;
        gVertex(at.x + cosf(a) * r, at.y + 0.05f, at.z + sinf(a) * r);
        gVertex(at.x + cosf(a) * (r + 0.25f), at.y + 0.05f, at.z + sinf(a) * (r + 0.25f));
    }
    gEnd();
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    gLighting(true);
}

// A standing hoop the racer rides through, facing along `dir`.
static void raceRing(const V3& at, float yaw, bool next) {
    Col c = next ? C(1.0f, 0.55f, 0.1f) : C(0.6f, 0.45f, 0.35f);
    gPush();
    gTranslate(at.x, at.y, at.z);
    gRotate(yaw * RAD2DEG, 0, 1, 0);
    gLighting(false);
    setc(c);
    const float R = 2.4f, w = 0.32f;
    gBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= 28; i++) {
        float a = PI * (float)i / 28.0f;
        gVertex(cosf(a) * R, sinf(a) * R + 0.2f, 0);
        gVertex(cosf(a) * (R - w), sinf(a) * (R - w) + 0.2f, 0);
    }
    gEnd();
    gLighting(true);
    gPop();
}

static void drawObjectives() {
    V3 tt;
    std::string tl;
    if (tutTarget(tt, tl)) beam(V3(tt.x, groundAt(tt.x, tt.z, tt.y + 0.5f).h, tt.z), C(0.4f, 0.9f, 1.0f), 0.35f, 30.0f, 0.2f);
    // chapter tasks with a place get a green beacon
    Chapter* c = curChapter();
    if (c && !g_mis.on)
        for (size_t i = 0; i < c->goals.size(); i++) {
            const Goal& g = c->goals[i];
            if (!goalOpen(g) || !g.hasAt) continue;
            float gy = groundAt(g.at.x, g.at.z, g.at.y + 0.5f).h;
            beam(V3(g.at.x, gy, g.at.z), C(0.3f, 1.0f, 0.5f), 0.35f, 30.0f, 0.16f);
        }
    if (!g_mis.on) return;
    for (size_t i = 0; i < g_mis.goals.size(); i++) {
        const Goal& g = g_mis.goals[i];
        if (goalDone(g)) continue;
        if (g.kind == GK_DELIVER) {
            beam(g.at, C(0.3f, 0.7f, 1.0f), 0.5f, 40.0f, 0.22f);
            groundRing(g.at, 2.6f, C(0.3f, 0.7f, 1.0f));
            billboardText("DROP HERE", g.at + V3(0, 2.6f + sinf(g_time * 3.0f) * 0.1f, 0), 0.35f, C(0.6f, 0.85f, 1.0f));
            break;
        }
        if (g.kind == GK_COLLECT) {
            for (size_t k = 0; k < g_kitePickups.size(); k++) {
                if (g_kiteGot[k]) continue;
                V3 p = g_kitePickups[k] + V3(0, sinf(g_time * 2.0f + (float)k) * 0.15f, 0);
                beam(V3(p.x, groundAt(p.x, p.z, p.y).h, p.z), C(1.0f, 0.5f, 0.8f), 0.3f, 25.0f, 0.18f);
                gPush();
                gTranslate(p.x, p.y, p.z);
                gRotate(g_time * 90.0f + (float)k * 40.0f, 0, 1, 0);
                gLighting(false);
                setc(SAREES[k % 8]);
                gBegin(GL_QUADS);
                gVertex(0, 0.55f, 0); gVertex(0.4f, 0, 0); gVertex(0, -0.55f, 0); gVertex(-0.4f, 0, 0);
                gEnd();
                gLighting(true);
                gPop();
            }
            break;
        }
        if (g.kind == GK_CHECKPOINT) {
            for (size_t k = (size_t)g.have; k < g_raceCps.size(); k++) {
                V3 a = g_raceCps[k];
                V3 b = k + 1 < g_raceCps.size() ? g_raceCps[k + 1] : a + (a - g_raceCps[k - 1]);
                float gy = groundAt(a.x, a.z, 1.0f).h;
                float yaw = dirYaw(b.x - a.x, b.z - a.z) + PI * 0.5f;
                raceRing(V3(a.x, gy, a.z), yaw, (int)k == g.have);
                if ((int)k == g.have) beam(V3(a.x, gy, a.z), C(1.0f, 0.6f, 0.2f), 0.3f, 25.0f, 0.18f);
            }
            break;
        }
        if (g.hasAt) {
            float gy = groundAt(g.at.x, g.at.z, g.at.y + 0.5f).h;
            beam(V3(g.at.x, gy, g.at.z), C(1.0f, 0.8f, 0.3f), 0.35f, 30.0f, 0.18f);
        }
    }
}

// Barricade geometry: posts, striped boards and, for front gates, a sign.
static void drawGateGeom(const Gate& g) {
    bool alongX = (g.x1 - g.x0) >= (g.z1 - g.z0);
    float L = alongX ? g.x1 - g.x0 : g.z1 - g.z0;
    float cx = (g.x0 + g.x1) * 0.5f, cz = (g.z0 + g.z1) * 0.5f;
    gPush();
    gTranslate(cx, 0, cz);
    if (!alongX) gRotate(90, 0, 1, 0);
    float x0 = -L * 0.5f;
    setc(0.3f, 0.3f, 0.32f);
    for (float x = x0; x <= -x0 + 0.01f; x += std::max(1.0f, L / 4.0f)) box(x - 0.06f, 0, -0.06f, x + 0.06f, 2.0f, 0.06f);
    for (int row = 0; row < 3; row++) {
        float y0 = 0.5f + (float)row * 0.55f;
        int k = row;
        for (float x = x0; x < -x0 - 0.01f; x += 0.5f, k++) {
            if (k & 1) setc(0.08f, 0.08f, 0.08f); else setc(0.98f, 0.78f, 0.1f);
            box(x, y0, -0.05f, std::min(x + 0.5f, -x0), y0 + 0.28f, 0.05f);
        }
    }
    if (g.sign) {
        char buf[48];
        snprintf(buf, sizeof(buf), "FINISH CHAPTER %d", g.needLevel);
        for (int side = 0; side < 2; side++) {
            gPush();
            if (side) gRotate(180, 0, 1, 0);
            setc(0.75f, 0.1f, 0.1f);
            box(-1.1f, 2.05f, 0.06f, 1.1f, 2.6f, 0.1f);
            gTranslate(0, 2.36f, 0.11f);
            text3DFit("BAND HAI", 0.2f, 2.0f, COL_WHITE);
            gTranslate(0, -0.22f, 0);
            text3DFit(buf, 0.1f, 2.0f, C(1.0f, 0.9f, 0.5f));
            gPop();
        }
    }
    gPop();
}

static void bakeGates() {
    for (size_t i = 0; i < g_gates.size(); i++) {
        recBegin();
        drawGateGeom(g_gates[i]);
        g_gates[i].list = recEnd();
    }
}

// What the skater is carrying: a food parcel, a chai kettle or a stack of dabbas.
static void drawCarried() {
    if (!g_mis.on || g_carryItem < 0) return;
    gPush();
    gTranslate(0, 0.95f, -0.32f);
    if (g_mis.id == MIS_TUKARAM) {
        int left = 0;
        for (size_t i = 0; i < g_mis.goals.size(); i++) left += goalDone(g_mis.goals[i]) ? 0 : 1;
        setc(COL_STEEL);
        for (int k = 0; k < left; k++) {
            gPush();
            gTranslate(0, -0.1f + (float)k * 0.12f, 0);
            cyl(0.08f, 0.08f, 0.11f, 8, true);
            gPop();
        }
    } else {
        switch (g_carryItem) {
            case STALL_CHAI:
                setc(0.8f, 0.78f, 0.7f);
                cyl(0.07f, 0.06f, 0.16f, 8, true);
                break;
            case STALL_PAAN:
                setc(0.2f, 0.6f, 0.15f);
                cyl(0.06f, 0.0f, 0.15f, 6, true);
                break;
            case 7:  // a bundle of newspapers under the arm
                setc(0.93f, 0.92f, 0.88f);
                box(-0.14f, -0.1f, -0.1f, 0.14f, 0.06f, 0.1f);
                setc(0.3f, 0.3f, 0.32f);
                box(-0.145f, -0.02f, -0.101f, 0.145f, 0.0f, 0.101f);
                break;
            case STALL_FRUIT:
                setc(0.95f, 0.75f, 0.2f);
                box(-0.12f, -0.1f, -0.1f, 0.12f, 0.08f, 0.1f);
                break;
            default:
                setc(0.85f, 0.72f, 0.5f);
                box(-0.1f, -0.12f, -0.07f, 0.1f, 0.06f, 0.07f);
                break;
        }
    }
    gPop();
}

// Kites on the sea face, bobbing on their strings.
static void drawKites() {
    for (size_t i = 0; i < g_kites.size(); i++) {
        const Kite& k = g_kites[i];
        V3 p = k.base + V3(sinf(g_time * 0.7f + (float)i) * 1.5f, sinf(g_time * 1.1f + (float)i * 2.0f) * 0.8f, 0);
        gLighting(false);
        glColor4f(0.95f, 0.95f, 0.95f, 1);
        gBegin(GL_LINE_STRIP);
        for (int s = 0; s <= 10; s++) {
            float t = (float)s / 10.0f;
            V3 q = k.anchor + (p - k.anchor) * t;
            q.y -= sinf(t * PI) * 2.5f;
            gVertex(q.x, q.y, q.z);
        }
        gEnd();
        gPush();
        gTranslate(p.x, p.y, p.z);
        gRotate(sinf(g_time * 1.3f + (float)i) * 20.0f, 0, 0, 1);
        setc(k.col);
        gBegin(GL_QUADS);
        gVertex(0, 0.8f, 0); gVertex(0.6f, 0, 0); gVertex(0, -0.8f, 0); gVertex(-0.6f, 0, 0);
        gEnd();
        gPop();
        gLighting(true);
    }
}

// Cheap visibility tests for moving things, against this frame's view.
static bool seen(float x0, float y0, float z0, float x1, float y1, float z1) {
    float lo[3] = {x0, y0, z0}, hi[3] = {x1, y1, z1};
    return boxVisible(g_frustum, lo, hi);
}
static bool vehicleSeen(const Vehicle& v) {
    float x0, z0, x1, z1;
    vehBounds(v, x0, z0, x1, z1);
    float cx = (x0 + x1) * 0.5f - g_camPos.x, cz = (z0 + z1) * 0.5f - g_camPos.z;
    if (cx * cx + cz * cz > G().drawDist * G().drawDist) return false;
    return seen(x0, 0, z0, x1, v.hgt, z1);
}
static bool pedSeen(const Ped& p) {
    float dx = p.pos.x - g_camPos.x, dz = p.pos.z - g_camPos.z;
    if (dx * dx + dz * dz > G().pedDist * G().pedDist) return false;
    return seen(p.pos.x - 0.6f, p.pos.y, p.pos.z - 0.6f, p.pos.x + 0.6f, p.pos.y + 2.2f, p.pos.z + 0.6f);
}
static void drawTrainCulled(const Train& t) {
    for (int i = 0; i < t.cars; i++) {
        float front = t.x - (float)t.dir * (float)i * (TRAIN_CAR_L + TRAIN_GAP);
        float xa = t.dir > 0 ? front - TRAIN_CAR_L : front;
        if (fabsf(xa + TRAIN_CAR_L * 0.5f - g_camPos.x) > G().drawDist + 20.0f) continue;
        if (!seen(xa, DECK_TOP, t.z - 2.0f, xa + TRAIN_CAR_L, DECK_TOP + 5.0f, t.z + 2.0f)) continue;
        gPush();
        gTranslate(xa, 0, t.z);
        drawMesh(t.carLists[i]);
        gPop();
    }
}

// Everything solid that moves or changes: drawn before the shadow pass.
static void drawDynamicOpaque() {
    drawSignals();
    for (size_t i = 0; i < g_vehicles.size(); i++) {
        const Vehicle& v = g_vehicles[i];
        float cx, cz, yaw;
        vehCenter(v, cx, cz, yaw);
        if (!vehicleSeen(v)) continue;
        gPush();
        gTranslate(cx, 0, cz);
        gRotate(yaw * RAD2DEG, 0, 1, 0);
        drawMesh(v.list);
        gPop();
    }
    for (size_t i = 0; i < g_trains.size(); i++) drawTrainCulled(g_trains[i]);
    for (size_t i = 0; i < g_peds.size(); i++) {
        const Ped& p = g_peds[i];
        if (!pedSeen(p)) continue;
        drawPerson(p);
    }
    for (size_t i = 0; i < g_gates.size(); i++)
        if (!gateOpen(g_gates[i])) drawMesh(g_gates[i].list);
    // the riders are posed once per frame into batches, drawn here and again for shadows
    for (size_t i = 0; i < g_friends.size() && i < 5; i++) {
        g_friendMesh[i].clear();
        if (!g_friends[i].skater) continue;
        if (len(g_friends[i].pos - g_camPos) > G().drawDist) continue;
        recBegin();
        drawFriend(g_friends[i]);
        g_rec = false;
        std::swap(g_friendMesh[i], g_recMesh);
        compileMesh(g_friendMesh[i]);
    }
    recBegin();
    drawSkater();
    g_rec = false;
    std::swap(g_playerMesh, g_recMesh);
    compileMesh(g_playerMesh);
    drawKites();
}

// Marks the stencil wherever a shadow lands, then darkens those pixels once.
static void drawShadowPass() {
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    gLighting(false);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -4.0f);
    if (G().worldShadows) drawChunks(g_shadowChunks, g_frustum, g_camPos, G().drawDist);
    pushShadowMatrix(0.0f);
    for (size_t i = 0; i < g_vehicles.size() && G().dynShadows >= 1; i++) {
        const Vehicle& v = g_vehicles[i];
        float cx, cz, yaw;
        vehCenter(v, cx, cz, yaw);
        if (fabsf(cx - g_camPos.x) > 60.0f || fabsf(cz - g_camPos.z) > 60.0f) continue;
        glPushMatrix();
        glTranslatef(cx, 0, cz);
        glRotatef(yaw * RAD2DEG, 0, 1, 0);
        drawMesh(v.list);
        glPopMatrix();
    }
    for (size_t i = 0; i < g_trains.size() && G().dynShadows >= 1; i++) {
        const Train& t = g_trains[i];
        for (int c = 0; c < t.cars; c++) {
            float front = t.x - (float)t.dir * (float)c * (TRAIN_CAR_L + TRAIN_GAP);
            float xa = t.dir > 0 ? front - TRAIN_CAR_L : front;
            if (fabsf(xa + TRAIN_CAR_L * 0.5f - g_camPos.x) > 80.0f || fabsf(t.z - g_camPos.z) > 80.0f) continue;
            glPushMatrix();
            glTranslatef(xa, 0, t.z);
            drawMesh(t.carLists[c]);
            glPopMatrix();
        }
    }
    glPopMatrix();
    for (size_t i = 0; i < g_peds.size() && G().dynShadows >= 2; i++) {
        const Ped& p = g_peds[i];
        if (fabsf(p.pos.x - g_camPos.x) > 30.0f || fabsf(p.pos.z - g_camPos.z) > 30.0f) continue;
        pushShadowMatrix(p.pos.y);
        drawPerson(p);
        glPopMatrix();
    }
    for (size_t i = 0; i < g_friends.size() && G().dynShadows >= 1; i++) {
        const Friend& f = g_friends[i];
        if (!f.skater) continue;
        pushShadowMatrix(groundAt(f.pos.x, f.pos.z, f.pos.y + 0.1f).h);
        compileMesh(g_friendMesh[i]);
        glPopMatrix();
    }
    if (P.state != P_BAIL) {
        pushShadowMatrix(groundAt(P.pos.x, P.pos.z, P.pos.y + 0.1f).h);
        compileMesh(g_playerMesh);
        glPopMatrix();
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    // shade the marked pixels
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.05f, 0.06f, 0.14f, 0.34f * g_sky.shadow);
    glBegin(GL_QUADS);
    glVertex2f(0, 0); glVertex2f(1, 0); glVertex2f(1, 1); glVertex2f(0, 1);
    glEnd();
    glDisable(GL_BLEND);
    glEnable(GL_FOG);
    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    gLighting(true);
}

// Glows, markers and weather: drawn last, over the shadows.
static void drawDynamicTransparent() {
    drawShadows();
    if (has(UL_MANUAL)) drawLetters();
    drawObjectives();
    drawTalkMarkers();
    V3 fwd = norm(g_camLook - g_camPos);
    V3 right = norm(cross(fwd, V3(0, 1, 0)));
    V3 up = cross(right, fwd);
    drawParticles(right, up, g_camPos);
    drawRain(g_camPos, g_time);
}

static void drawSky() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    gLighting(false);
    glDisable(GL_FOG);
    gBegin(GL_QUADS);
    const Col& a = g_sky.low;
    const Col& b = g_sky.mid;
    const Col& c = g_sky.top;
    gColor(a.r, a.g, a.b); glVertex2f(0, 0); glVertex2f(1, 0);
    gColor(b.r, b.g, b.b); glVertex2f(1, 0.45f); glVertex2f(0, 0.45f);
    gColor(b.r, b.g, b.b); glVertex2f(0, 0.45f); glVertex2f(1, 0.45f);
    gColor(c.r, c.g, c.b); glVertex2f(1, 1); glVertex2f(0, 1);
    gEnd();
    drawStars();
    drawClouds();
    glEnable(GL_DEPTH_TEST);
}

// Stars, placed by compass direction like the clouds, twinkling a little.
static void drawStars() {
    if (g_sky.stars < 0.02f) return;
    float W = (float)g_winW, H = (float)g_winH;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, W, 0, H, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    float aspect = W / std::max(1.0f, H);
    float fovX = 2.0f * atanf(tanf(64.0f * 0.5f / RAD2DEG) * aspect);
    for (int big = 0; big < 2; big++) {
        glPointSize(big ? 2.5f : 1.5f);
        glBegin(GL_POINTS);
        for (int i = 0; i < 260; i++) {
            unsigned int h = (unsigned int)(i + 7) * 2246822519u;
            h ^= h >> 13;
            h *= 3266489917u;
            if (((h >> 3) % 9 == 0) != (big == 1)) continue;
            float az = (float)(h & 4095) / 4095.0f * TWO_PI;
            float el = 0.5f + (float)((h >> 12) & 1023) / 1023.0f * 0.5f;
            float rel = wrapAngle(az - g_camYaw);
            if (fabsf(rel) > fovX * 0.5f + 0.1f) continue;
            float tw = 0.65f + 0.35f * sinf(g_time * (1.5f + (float)(h >> 24) * 0.02f) + (float)i);
            float warm = (float)((h >> 22) & 3) * 0.05f;
            glColor4f(0.9f + warm, 0.9f, 1.0f - warm, g_sky.stars * tw * (0.4f + 0.6f * el));
            glVertex2f(W * (0.5f - rel / fovX), H * el);
        }
        glEnd();
    }
    glPointSize(1.0f);
    glDisable(GL_BLEND);
}

// Soft cumulus puffs painted into the sky, placed by compass direction so they
// swing past as the camera turns.
static void drawClouds() {
    if (!G().clouds) return;
    float W = (float)g_winW, H = (float)g_winH;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, W, 0, H, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    float aspect = W / std::max(1.0f, H);
    float fovX = 2.0f * atanf(tanf(64.0f * 0.5f / RAD2DEG) * aspect);
    for (int i = 0; i < 14; i++) {
        unsigned int h = (unsigned int)(i + 1) * 2654435761u;
        float az = (float)(h & 1023) / 1023.0f * TWO_PI + g_time * 0.004f;
        float el = 0.62f + (float)((h >> 10) & 255) / 255.0f * 0.3f;
        float size = 40.0f + (float)((h >> 18) & 63);
        float rel = wrapAngle(az - g_camYaw);
        if (fabsf(rel) > fovX * 0.5f + 0.4f) continue;
        float cx = W * (0.5f - rel / fovX), cy = H * el;
        // lit from below at sunset, dark shapes at night
        Col c = mixc(C(1.0f, 0.98f, 0.95f), mulc(g_sky.low, 1.1f), clampf(g_sky.night * 1.6f, 0, 0.85f));
        c = mixc(c, mulc(C(0.62f, 0.65f, 0.7f), 0.2f + 0.8f * (1.0f - g_sky.night)), g_rain);
        for (int k = 0; k < 6; k++) {
            float ox = ((float)k - 2.5f) * size * 0.45f;
            float oy = sinf((float)k * 1.7f + (float)i) * size * 0.12f;
            float r = size * (0.45f + 0.25f * sinf((float)k * 2.3f + (float)i * 0.7f));
            glBegin(GL_TRIANGLE_FAN);
            glColor4f(c.r, c.g, c.b, 0.55f * (1.0f - 0.45f * g_sky.night));
            glVertex2f(cx + ox, cy + oy);
            glColor4f(c.r, c.g, c.b, 0.0f);
            for (int a = 0; a <= 20; a++) {
                float t = TWO_PI * (float)a / 20.0f;
                glVertex2f(cx + ox + cosf(t) * r * 1.4f, cy + oy + sinf(t) * r * 0.7f);
            }
            glEnd();
        }
    }
    glDisable(GL_BLEND);
}

static const V3 MOON_DIR(0.5f, 0.42f, 0.76f);

// Sun by day, moon by night. The sun keeps its direction (shadows are baked
// for it) but grows and turns orange toward sunset.
static void drawSun() {
    gLighting(false);
    glDisable(GL_FOG);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    for (int body = 0; body < 2; body++) {
        float vis = body == 0 ? g_sky.sunA : g_sky.moon;
        if (vis < 0.01f) continue;
        V3 s = g_camPos + (body == 0 ? SUN_DIR : norm(MOON_DIR)) * 400.0f;
        V3 toCam = norm(g_camPos - s);
        V3 r = norm(cross(V3(0, 1, 0), toCam));
        V3 u = cross(toCam, r);
        Col core = body == 0 ? g_sky.sunDisc : C(0.95f, 0.95f, 0.88f);
        float grow = body == 0 ? 1.0f + 0.5f * clampf(g_sky.night * 2.5f, 0, 1) : 0.7f;
        for (int layer = 0; layer < 3; layer++) {
            float rad = (layer == 0 ? 40.0f : (layer == 1 ? 22.0f : 12.0f)) * grow;
            float a = (layer == 0 ? 0.12f : (layer == 1 ? 0.25f : 1.0f)) * vis;
            if (body == 1 && layer < 2) a *= 0.5f;
            gBegin(GL_TRIANGLE_FAN);
            glColor4f(core.r, core.g, core.b, a);
            gVertex(s.x, s.y, s.z);
            glColor4f(core.r, core.g * 0.9f, core.b * 0.75f, layer == 2 ? a : 0.0f);
            for (int i = 0; i <= 24; i++) {
                float t = TWO_PI * (float)i / 24.0f;
                V3 p = s + r * (cosf(t) * rad) + u * (sinf(t) * rad);
                gVertex(p.x, p.y, p.z);
            }
            gEnd();
        }
    }
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_FOG);
    gLighting(true);
}

static void updateCamera(float dt, bool snap) {
    float hs = hspeed();
    float target = g_camYaw;
    if (P.state != P_BAIL) {
        if (hs > 1.2f) target = dirYaw(P.vel.x, P.vel.z);
        else target = fabsf(wrapAngle(P.yaw - g_camYaw)) < 1.75f ? P.yaw : wrapAngle(P.yaw + PI);
    }
    float rate = P.state == P_AIR ? 1.5f : 4.0f;
    if (snap) g_camYaw = target;
    else g_camYaw = wrapAngle(g_camYaw + wrapAngle(target - g_camYaw) * std::min(1.0f, dt * rate));
    float dist = 5.5f, height = 2.3f;
    if (g_camMode == 1) { dist = 9.0f; height = 3.8f; }
    if (g_camMode == 2) { dist = 3.2f; height = 1.6f; }
    V3 back = yawDir(g_camYaw);
    V3 focus = P.pos + V3(0, 1.2f, 0);
    V3 want = focus - back * dist + V3(0, height - 1.2f + 0.5f, 0);
    float t = rayBlock(focus, want);
    // keep the camera out of buses and autos too
    for (size_t i = 0; i < g_vehicles.size(); i++) {
        float x0, z0, x1, z1;
        vehBounds(g_vehicles[i], x0, z0, x1, z1);
        if (x1 < std::min(focus.x, want.x) - 1 || x0 > std::max(focus.x, want.x) + 1 ||
            z1 < std::min(focus.z, want.z) - 1 || z0 > std::max(focus.z, want.z) + 1)
            continue;
        float lo[3] = {x0 - 0.3f, 0, z0 - 0.3f}, hi[3] = {x1 + 0.3f, g_vehicles[i].hgt + 0.3f, z1 + 0.3f};
        t = rayBox(focus, want, lo, hi, t);
    }
    if (t < 1.0f) want = focus + (want - focus) * std::max(0.08f, t - 0.05f);
    if (snap || !g_camInit) {
        g_camPos = want;
        g_camInit = true;
    } else {
        float kx = std::min(1.0f, dt * 10.0f), ky = std::min(1.0f, dt * 5.0f);
        g_camPos.x = lerpf(g_camPos.x, want.x, kx);
        g_camPos.z = lerpf(g_camPos.z, want.z, kx);
        g_camPos.y = lerpf(g_camPos.y, want.y, ky);
    }
    float floorY = groundAt(g_camPos.x, g_camPos.z, g_camPos.y).h + 0.4f;
    if (g_camPos.y < floorY) g_camPos.y = floorY;
    g_camLook = focus + back * 2.0f;
}

