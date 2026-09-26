#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Game state: popups, scoring, combos
// ============================================================================

struct Popup {
    std::string text;
    float t, life;
    Col col;
    float size;
    float y;  // fraction of screen height
};
static std::vector<Popup> g_popups;

static void popup(const std::string& s, const Col& c, float size = 34.0f, float life = 1.6f) {
    for (size_t i = 0; i < g_popups.size(); i++) g_popups[i].y += (size * 1.25f) / 720.0f;
    Popup p;
    p.text = s;
    p.t = 0;
    p.life = life;
    p.col = c;
    p.size = size;
    p.y = 0.6f;
    g_popups.push_back(p);
    if (g_popups.size() > 6) g_popups.erase(g_popups.begin());
}

struct TrickEntry {
    std::string name;
    float pts;
};

// Defined in career.hpp and missions.hpp.
static void careerCombo(long total, const std::vector<TrickEntry>& list);
static void friendsCheer(long total);

struct Combo {
    std::vector<TrickEntry> list;
    float grace;  // seconds left before a landed combo is banked
};
static Combo g_combo;
static long g_score = 0, g_bestCombo = 0;
static float g_toastCooldown = 0;

// Points for a trick, halved for every earlier use of the same trick in this combo.
static float trickValue(const std::string& name, float pts, int exclude) {
    int n = 0;
    for (size_t i = 0; i < g_combo.list.size(); i++)
        if ((int)i != exclude && g_combo.list[i].name == name) n++;
    return pts * powf(0.5f, (float)n);
}
static int addTrick(const std::string& name, float pts) {
    TrickEntry e;
    e.name = name;
    e.pts = trickValue(name, pts, -1);
    g_combo.list.push_back(e);
    return (int)g_combo.list.size() - 1;
}
static void setTrick(int idx, const std::string& name, float pts) {
    if (idx < 0 || idx >= (int)g_combo.list.size()) return;
    g_combo.list[idx].name = name;
    g_combo.list[idx].pts = trickValue(name, pts, idx);
}
static float comboBase() {
    float s = 0;
    for (size_t i = 0; i < g_combo.list.size(); i++) s += g_combo.list[i].pts;
    return s;
}
static int comboMult() { return (int)g_combo.list.size(); }

static void bankCombo() {
    if (g_combo.list.empty()) return;
    long total = (long)(comboBase() * (float)comboMult());
    g_score += total;
    if (total > g_bestCombo) g_bestCombo = total;
    if (comboMult() > 1) {
        popup("+" + withCommas(total) + "  x" + std::to_string(comboMult()) + " COMBO", C(1.0f, 0.85f, 0.2f), 40, 2.2f);
    } else {
        popup("+" + withCommas(total), C(1.0f, 0.85f, 0.2f), 34, 1.6f);
    }
    careerCombo(total, g_combo.list);
    friendsCheer(total);
    g_combo.list.clear();
}

static void loseCombo() {
    if (!g_combo.list.empty()) popup("COMBO LOST", C(0.9f, 0.5f, 0.5f), 24, 1.6f);
    g_combo.list.clear();
}

// ============================================================================
// Input
// ============================================================================

static bool g_key[256];
static bool g_keyHit[256];
static bool g_keyRel[256];  // released since the last simulation step
static bool g_special[4];  // up, down, left, right arrows

struct Input {
    bool up, down, left, right;
    bool ollie, kick, heel, shove, indy, melon, manual, noseManual;  // presses this step
    bool indyHeld, melonHeld;
    bool ollieHeld;  // Space is down: crouching for a bigger pop
    bool backflip;   // B pressed
    bool boost;      // V pressed
};

