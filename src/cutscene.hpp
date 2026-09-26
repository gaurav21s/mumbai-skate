#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Story cutscenes
//
// Two short films made from camera moves over the live city: the opening,
// which plays when a new game starts, and the ending, after the finale. The
// traffic, trains and fireworks keep running underneath; only the skater and
// the story wait. Space, Enter or Esc skips.
// ============================================================================

enum CutKind { CUT_NONE, CUT_INTRO, CUT_ENDING };
enum ShotMove { MOVE_WORLD, MOVE_ORBIT, MOVE_CRANE };

struct Shot {
    float dur, hour;
    int move;
    V3 eye0, eye1, look0, look1;  // MOVE_WORLD: world points. MOVE_ORBIT: x,y = start and end angle
    const char* line1;
    const char* line2;
    bool title;                   // big MUMBAI SKATE title over the shot
};

static const Shot INTRO_SHOTS[] = {
    {6.5f, 6.2f, MOVE_WORLD, V3(-70, 34, -120), V3(10, 30, -112), V3(-40, 8, -50), V3(20, 8, -50),
     "MUMBAI, 6:15 AM", "The first local of the day rolls into Dadar.", false},
    {6.0f, 7.3f, MOVE_WORLD, V3(60, 3.5f, 10), V3(22, 3.5f, 10), V3(0, 2, 4), V3(-40, 2, 4),
     "The chai is already boiling.", "The BEST buses are already full.", false},
    {6.5f, 8.0f, MOVE_WORLD, V3(-18, 9, -8), V3(-44, 7, -12), V3(-45, 1, -28), V3(-62, 1, -28),
     "In the plaza under the railway bridge,", "the kids of the block skate before school.", false},
    {6.5f, 8.4f, MOVE_ORBIT, V3(0.6f, 2.6f, 0), V3(), V3(), V3(),
     "You just moved here.", "One board. A hundred rupees. Nobody knows your name yet.", false},
    {6.0f, 8.5f, MOVE_CRANE, V3(), V3(), V3(), V3(),
     "", "Every galli has a legend. Time to become this one's.", true},
};
static const Shot ENDING_SHOTS[] = {
    {7.0f, 22.0f, MOVE_WORLD, V3(115, 20, 176), V3(0, 16, 174), V3(55, 4, 150), V3(55, 7, 150),
     "The rain has stopped.", "The whole block is out on the Sea Face.", false},
    {7.5f, 22.0f, MOVE_WORLD, V3(26, 2.8f, 131), V3(84, 2.8f, 131), V3(40, 2.0f, 147), V3(70, 2.0f, 147),
     "Raju. Priya. Sam. Tukaram. Chintu.", "The crew that took in the new kid.", false},
    {6.5f, 22.0f, MOVE_ORBIT, V3(3.6f, 5.4f, 0), V3(), V3(), V3(),
     "From a hundred rupees and a borrowed board", "to the legend of the galli.", false},
    {7.5f, 22.0f, MOVE_WORLD, V3(55, 3.5f, 142), V3(55, 16, 118), V3(55, 3, 160), V3(55, 18, 210),
     "Aamchi galli, aamcha raja.", "Thanks for playing Mumbai Skate.", true},
};

static float g_cutT = 0;       // time into the current shot
static int g_cutShot = 0;
static float g_cutCheer = 0;

static const Shot* cutShots(int& n) {
    if (g_cut == CUT_INTRO) { n = (int)(sizeof(INTRO_SHOTS) / sizeof(INTRO_SHOTS[0])); return INTRO_SHOTS; }
    n = (int)(sizeof(ENDING_SHOTS) / sizeof(ENDING_SHOTS[0]));
    return ENDING_SHOTS;
}

static void endCutscene() {
    int was = g_cut;
    g_cut = CUT_NONE;
    g_skyHourOverride = -1.0f;
    g_popups.clear();
    g_camInit = false;
    updateCamera(0, true);
    if (was == CUT_ENDING) {
        // the credits card follows, over the live show
        g_card = CARD_LEGEND;
        g_cardT = 0;
    }
}

static void startCutscene(int kind) {
    g_cut = kind;
    g_cutT = 0;
    g_cutShot = 0;
    g_popups.clear();
    if (kind == CUT_INTRO && !g_trains.empty()) g_trains[0].x = -45.0f;  // the first local, just coming into view
    if (kind == CUT_ENDING) {
        // everyone on the viewing deck for the last shots
        resetPlayer(V3(55.0f, 1.0f, 151.0f), PI);
        const V3 spots[3] = {V3(51.5f, 1.0f, 150.6f), V3(58.5f, 1.0f, 150.6f), V3(55.0f, 1.0f, 153.2f)};
        int k = 0;
        for (size_t i = 0; i < g_friends.size(); i++) {
            Friend& f = g_friends[i];
            if (!f.skater || k >= 3) continue;
            f.pos = spots[k++];
            f.yaw = PI;
            f.speed = 0;
            f.air = false;
            f.vy = 0;
        }
        finaleUpdate(0);  // the crowd and Tukaram take their places
    }
}

static void skipCutscene() {
    if (g_cut != CUT_NONE) endCutscene();
}

// Runs the world, not the story, and moves on through the shots.
static void cutsceneStep(float dt) {
    int n = 0;
    const Shot* shots = cutShots(n);
    g_cutT += dt;
    if (g_cutT >= shots[g_cutShot].dur) {
        g_cutT = 0;
        if (++g_cutShot >= n) {
            endCutscene();
            return;
        }
    }
    g_skyHourOverride = shots[g_cutShot].hour;
    updateTraffic(dt);
    updateTrains(dt);
    updatePeds(dt);
    updateParticles(dt);
    if (g_cut == CUT_ENDING) {
        fireworksUpdate(dt, true, 0.3f);
        g_cutCheer -= dt;
        if (g_cutCheer <= 0) {
            g_cutCheer = 1.6f;
            for (size_t k = 0; k < g_crowd.size(); k++)
                if (fxrand() < 0.5f) g_peds[(size_t)g_crowd[k]].bumpT = 0.9f;
        }
    }
}

static void cutsceneCamera() {
    int n = 0;
    const Shot& s = cutShots(n)[g_cutShot];
    float t = clampf(g_cutT / s.dur, 0, 1);
    t = t * t * (3.0f - 2.0f * t);
    V3 focus = P.pos + V3(0, 1.1f, 0);
    if (s.move == MOVE_WORLD) {
        g_camPos = s.eye0 + (s.eye1 - s.eye0) * t;
        g_camLook = s.look0 + (s.look1 - s.look0) * t;
    } else if (s.move == MOVE_ORBIT) {
        float a = P.yaw + lerpf(s.eye0.x, s.eye0.y, t);
        g_camPos = focus + V3(sinf(a) * 4.2f, 0.7f, cosf(a) * 4.2f);
        g_camLook = focus;
    } else {
        V3 back = yawDir(P.yaw);
        g_camPos = focus - back * lerpf(2.5f, 9.0f, t) + V3(0, lerpf(0.4f, 6.0f, t), 0);
        g_camLook = focus + back * lerpf(1.0f, 8.0f, t) + V3(0, lerpf(0.0f, 2.0f, t), 0);
    }
    g_camYaw = dirYaw(g_camLook.x - g_camPos.x, g_camLook.z - g_camPos.z);
}

// Letterbox bars, captions and fades. Called with the HUD's 2D setup.
static void drawCutsceneOverlay(float W, float H) {
    int n = 0;
    const Shot* shots = cutShots(n);
    const Shot& s = shots[g_cutShot];
    float bar = H * 0.12f;
    rect2D(0, 0, W, bar, 0, 0, 0, 1);
    rect2D(0, H - bar, W, H, 0, 0, 0, 1);
    // fade in at the start, out at the end, a quick dip between shots
    float fade = 0;
    if (g_cutShot == 0) fade = std::max(fade, 1.0f - g_cutT / 1.2f);
    if (g_cutShot == n - 1) fade = std::max(fade, 1.0f - (s.dur - g_cutT) / 1.2f);
    fade = std::max(fade, std::max(0.0f, 1.0f - g_cutT / 0.35f) * 0.8f * (g_cutShot > 0 ? 1.0f : 0.0f));
    if (fade > 0.01f) rect2D(0, 0, W, H, 0, 0, 0, clampf(fade, 0, 1));
    float a = clampf(std::min((g_cutT - 0.5f) * 1.5f, (s.dur - g_cutT - 0.3f) * 1.5f), 0, 1);
    if (s.title) {
        float big = std::min(W / 12.0f, 96.0f);
        strokeText2D(g_cut == CUT_INTRO ? "MUMBAI SKATE" : "THE END", W / 2, H * 0.52f, big, 0.5f, C(1.0f, 0.72f, 0.2f), a);
    }
    if (s.line1[0]) strokeText2D(s.line1, W / 2, bar * 0.55f, 20, 0.5f, COL_WHITE, a);
    if (s.line2[0]) strokeText2D(s.line2, W / 2, bar * 0.2f, 16, 0.5f, C(0.85f, 0.9f, 1.0f), a);
    bmpTextShadow(W - 150, H - bar * 0.6f, "SPACE  skip", C(0.6f, 0.6f, 0.65f), GLUT_BITMAP_HELVETICA_12);
}
