#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Career: chapters, tasks, gaps, unlocks, rupees, gear and the save file
//
// The story is a list of chapters. Each chapter is a list of tasks (goals).
// Gameplay reports events (a trick landed, a gap cleared, a combo banked) and
// every open task that matches moves forward. Side missions reuse the same
// goal type, so one event can advance both the chapter and a mission.
// ============================================================================

enum GoalKind {
    GK_TRICK,        // land a trick whose name contains key (alternatives split by '|')
    GK_SPIN,         // land a spin of at least param degrees
    GK_GAP,          // clear the gap named key
    GK_GRIND,        // grind a rail tagged tag for param seconds
    GK_REACH,        // roll onto the area box
    GK_COMBO,        // bank a combo worth param points
    GK_COMBO_MANUAL, // bank a combo of param tricks that includes a manual
    GK_COMBO_LEN,    // bank a combo of param tricks
    GK_GRAB,         // land a grab held for param seconds
    GK_LETTERS,      // collect the C-H-A-I letters
    GK_CHAI,         // stop at a chai stall
    GK_SCORE,        // earn param points since the chapter started
    GK_MISSION,      // finish the side mission named key
    GK_DELIVERIES,   // finish need deliveries
    GK_DELIVER,      // mission only: reach the drop point `at`
    GK_CHECKPOINT,   // mission only: pass checkpoint number tag of a race
    GK_COLLECT,      // mission only: pick up the loose kites
};

struct Goal {
    int kind;
    std::string text, key;
    std::string baseText;   // template: {n} count, {pl} plural s, {p} points, {c} tricks, {s} seconds, {d} degrees
    int need, have, baseNeed;
    float param, baseParam;
    int tag;
    float bx0, bz0, bx1, bz1, bymin;
    V3 at;
    bool hasAt;
    std::string how, hint;          // short free tip, and the full walkthrough bought with G
    std::string baseHow, baseHint;
    bool hintPaid;
    Goal& where(float x, float y, float z) { at = V3(x, y, z); hasAt = true; return *this; }
    Goal& tip(const std::string& h, const std::string& full) {
        how = baseHow = h;
        hint = baseHint = full;
        return *this;
    }
    Goal& rail(int t) { tag = t; return *this; }
    Goal& area(float x0, float z0, float x1, float z1, float ymin) {
        bx0 = x0; bz0 = z0; bx1 = x1; bz1 = z1; bymin = ymin;
        return *this;
    }
};

static Goal mkGoal(int kind, const std::string& text, const std::string& key, int need, float param) {
    Goal g;
    g.kind = kind;
    g.text = text;
    g.key = key;
    g.baseText = text;
    g.need = g.baseNeed = need;
    g.have = 0;
    g.param = g.baseParam = param;
    g.tag = 0;
    g.bx0 = g.bz0 = g.bx1 = g.bz1 = g.bymin = 0;
    g.hasAt = false;
    g.hintPaid = false;
    return g;
}
static bool goalDone(const Goal& g) { return g.have >= g.need; }

static std::string fmtSecs(float s) {
    char buf[16];
    if (fabsf(s - roundf(s)) < 0.05f) snprintf(buf, sizeof(buf), "%d", (int)roundf(s));
    else snprintf(buf, sizeof(buf), "%.1f", s);
    return buf;
}
static void replaceAll(std::string& s, const std::string& a, const std::string& b) {
    for (size_t k = s.find(a); k != std::string::npos; k = s.find(a, k + b.size())) s.replace(k, a.size(), b);
}

// Applies the difficulty to one task: counts, targets and durations, then rebuilds its text.
static void scaleGoal(Goal& g) {
    const DiffTune& t = D();
    g.need = g.baseNeed;
    g.param = g.baseParam;
    switch (g.kind) {
        case GK_TRICK:
        case GK_DELIVERIES: g.need = std::max(1, (int)lroundf((float)g.baseNeed * t.countMul)); break;
        case GK_COMBO:
        case GK_SCORE: g.param = roundf(g.baseParam * t.scoreMul / 50.0f) * 50.0f; break;
        case GK_COMBO_LEN:
        case GK_COMBO_MANUAL: g.param = std::max(2.0f, roundf(g.baseParam * (0.5f + 0.5f * t.countMul))); break;
        case GK_GRIND:
        case GK_GRAB: g.param = g.baseParam * t.holdMul; break;
        case GK_SPIN: g.param = t.allTricks ? 180.0f : g.baseParam; break;
        default: break;
    }
    g.have = std::min(g.have, g.need);
    std::string* outs[3] = {&g.text, &g.how, &g.hint};
    const std::string* ins[3] = {&g.baseText, &g.baseHow, &g.baseHint};
    for (int k = 0; k < 3; k++) {
        std::string s = *ins[k];
        replaceAll(s, "{n}", std::to_string(g.need));
        replaceAll(s, "{pl}", g.need > 1 ? "s" : "");
        replaceAll(s, "{p}", withCommas((long)g.param));
        replaceAll(s, "{c}", std::to_string((int)g.param));
        replaceAll(s, "{s}", fmtSecs(g.param));
        replaceAll(s, "{d}", std::to_string((int)g.param));
        *outs[k] = s;
    }
}

struct Chapter {
    const char* name;
    const char* english;
    const char* brief;
    const char* reward1;
    const char* reward2;
    float timeLimit;  // 0 for none
    float baseTime;
    V3 spawn;
    float spawnYaw;
    int rupees;       // paid on completion
    std::vector<Goal> goals;
};
static std::vector<Chapter> g_chapters;

// Side mission in progress. Content and rules live in missions.hpp.
struct ActiveMission {
    bool on;
    int id;
    std::string title;
    std::vector<Goal> goals;
    float timeLeft, timeLimit;
    float freshness;  // deliveries: bails spoil the order
    long score0;      // score when the mission started, for score attacks
    int job;          // job board job type, for MIS_JOB
};
static ActiveMission g_mis;

// ---------------------------------------------------------------- unlocks

enum UnlockLevel { UL_HEEL = 1, UL_GRAB = 2, UL_MANUAL = 3, UL_CHAI = 4, UL_RAIN = 5, UL_LEGEND = 6 };
static bool g_careerOn = true;  // off in the self test, so tasks never teleport the skater
static bool g_noSave = false;
static bool has(int level) { return g_unlockLevel >= level; }
static bool g_tutorial = false;  // tutorial running: every trick can be tried
static bool g_tutorialDone = false;
// Tricks follow the story on medium and hard; easy and the tutorial open them all.
static bool trickOpen(int level) { return D().allTricks || g_tutorial || has(level); }
static float g_lockMsgT = 0;

static void lockedMsg(const char* what, int level) {
    if (g_lockMsgT > 0) return;
    g_lockMsgT = 1.5f;
    char buf[128];
    snprintf(buf, sizeof(buf), "%s IS LOCKED. FINISH CHAPTER %d", what, level);
    popup(buf, C(0.75f, 0.75f, 0.8f), 22, 1.6f);
}

// ---------------------------------------------------------------- rupees and gear

enum GearId {
    GEAR_BEARINGS, GEAR_WHEELS, GEAR_POPDECK, GEAR_WAX,
    DECK_KAALIPEELI, DECK_BEST, DECK_WARLI, DECK_GOLD,
    FIT_JERSEY, FIT_KURTA, FIT_RAINCOAT,
    HAT_SNAPBACK, HAT_BUCKET, HAT_TOPI, HAT_BEANIE,
    GLASSES_AVIATOR, GLASSES_ROUND,
    NECK_CHAIN, NECK_GAMCHA,
    BAG_BACKPACK,
    GEAR_COUNT
};
enum GearKind { GEAR_UPGRADE, GEAR_DECK, GEAR_OUTFIT, GEAR_HAT, GEAR_GLASSES, GEAR_NECK, GEAR_BAG };
struct GearDef {
    const char* name;
    const char* desc;
    int price;
    int kind;
    int needLevel;
};
static const GearDef GEAR[GEAR_COUNT] = {
    {"ABEC-7 BEARINGS", "Faster top speed and pushes", 300, GEAR_UPGRADE, 0},
    {"SOFT 54MM WHEELS", "Grip on wet roads, softer sketchy landings", 250, GEAR_UPGRADE, 0},
    {"POPSICLE PRO DECK", "Pops 10 percent higher", 450, GEAR_UPGRADE, 1},
    {"LEDGE WAX", "Grinds and manuals wobble less", 350, GEAR_UPGRADE, 2},
    {"KAALI-PEELI DECK", "Black and yellow, like the taxis", 200, GEAR_DECK, 0},
    {"BEST RED DECK", "Bus red with a cream stripe", 200, GEAR_DECK, 1},
    {"WARLI ART DECK", "Terracotta with white Warli figures", 300, GEAR_DECK, 3},
    {"GOLD DECK", "For legends only", 0, GEAR_DECK, 6},
    {"BLUE JERSEY", "Wankhede stand ready", 150, GEAR_OUTFIT, 0},
    {"KURTA AND GAMCHA", "Festival fit", 150, GEAR_OUTFIT, 2},
    {"YELLOW RAINCOAT", "Monsoon essential", 100, GEAR_OUTFIT, 4},
    {"RED SNAPBACK", "Worn forward, like you mean it", 120, GEAR_HAT, 0},
    {"BUCKET HAT", "Sun's out on the Sea Face", 150, GEAR_HAT, 0},
    {"GANDHI TOPI", "Crisp white, like the dabbawalas", 80, GEAR_HAT, 0},
    {"WOOL BEANIE", "For the one cold day a year", 100, GEAR_HAT, 1},
    {"AVIATOR SHADES", "Gold frames, dark lenses", 180, GEAR_GLASSES, 0},
    {"ROUND SPECS", "Irani cafe philosopher", 120, GEAR_GLASSES, 0},
    {"GOLD CHAIN", "Heavy. Probably not real", 250, GEAR_NECK, 1},
    {"CHECKED GAMCHA", "Red and white, round the neck", 90, GEAR_NECK, 0},
    {"SKATE BACKPACK", "Carries tiffins and spare bearings", 200, GEAR_BAG, 0},
};
static int g_rupees = 100;  // a little pocket money to start
static bool g_owned[GEAR_COUNT];
static int g_deck = -1;    // equipped deck, -1 for the starter deck
static int g_outfit = -1;  // equipped outfit, -1 for the starter tee
static int g_hat = -1, g_glasses = -1, g_neck = -1, g_bag = -1;  // equipped accessories, -1 for none
// The equipped slot for a kind of gear, or null for upgrades.
static int* gearSlot(int kind) {
    switch (kind) {
        case GEAR_DECK: return &g_deck;
        case GEAR_OUTFIT: return &g_outfit;
        case GEAR_HAT: return &g_hat;
        case GEAR_GLASSES: return &g_glasses;
        case GEAR_NECK: return &g_neck;
        case GEAR_BAG: return &g_bag;
        default: return nullptr;
    }
}
static float g_chai = 0;   // chai power meter, 0 to 1
static float g_energy = 100;  // stamina, 0 to 100. Boosting spends it, food restores it
static float g_boostT = 0;    // seconds of boost left
static const float ENERGY_REST_CAP = 55.0f;  // resting only refills this far; food goes beyond

struct Food {
    const char* name;
    int price;
    float energy;
};
// One menu item per stall kind (see StallKind)
static const Food FOODS[7] = {
    {"CUTTING CHAI", 10, 30}, {"VADA PAV", 15, 50}, {"MEETHA PAAN", 10, 25}, {"BHEL PURI", 20, 45},
    {"MANGO SLICES", 25, 60}, {"BHUTTA", 20, 45},   {"BARAF GOLA", 15, 35},
};
static float boostCost() { return g_diff == DIFF_EASY ? 12.0f : 20.0f; }

// ---------------------------------------------------------------- skill level
// XP comes from banked combos, deliveries, missions and jobs. Every level makes
// the skater pop a little higher, push a little faster and wobble a little less.
static long g_xp = 0;
static const int MAX_SKILL = 10;
static long xpForLevel(int lvl) { return 400L * (long)(lvl - 1) * (long)lvl / 2; }  // 0, 400, 1200, 2400...
static int skillLevel() {
    int l = 1;
    while (l < MAX_SKILL && g_xp >= xpForLevel(l + 1)) l++;
    return l;
}
static float skillBonus() { return (float)(skillLevel() - 1); }
static void addXP(long n, const char* why) {
    if (n <= 0) return;
    int before = skillLevel();
    g_xp += n;
    char buf[80];
    snprintf(buf, sizeof(buf), "+%ld XP  %s", n, why);
    popup(buf, C(0.6f, 0.8f, 1.0f), 18, 1.6f);
    int after = skillLevel();
    if (after > before) {
        popup("SKILL LEVEL " + std::to_string(after) + "!  HIGHER POP, FASTER PUSH, STEADIER BALANCE",
              C(0.5f, 0.85f, 1.0f), 28, 3.0f);
        confetti(P.pos);
    }
}
static bool tired() { return g_energy < 15.0f; }
static int g_deliveries = 0;
static bool g_friend[5];  // the four crew members, then Chintu (not crew, just a kid with kites)
static const char* FRIEND_NAMES[5] = {"RAJU", "PRIYA", "SAM", "TUKARAM", "CHINTU"};

static int friendCount() {
    int n = 0;
    for (int i = 0; i < 4; i++) n += g_friend[i] ? 1 : 0;
    return n;
}

// Physics tuning that gear and chai power change.
static bool chaiFull() { return trickOpen(UL_CHAI) && g_chai >= 1.0f; }
static float gearPushMax() {
    float m = 10.5f + (g_owned[GEAR_BEARINGS] ? 1.6f : 0.0f) + (chaiFull() ? 1.0f : 0.0f) + 0.12f * skillBonus();
    if (g_boostT > 0) m += 7.0f;
    if (tired()) m *= 0.85f;
    return m;
}
static float gearPushAcc() { return 5.5f * (g_owned[GEAR_BEARINGS] ? 1.15f : 1.0f) * (g_boostT > 0 ? 2.6f : 1.0f); }
static float gearOllie() {
    return (g_owned[GEAR_POPDECK] ? 1.1f : 1.0f) * (g_deck == DECK_GOLD ? 1.05f : 1.0f) * (1.0f + 0.015f * skillBonus());
}
static float gearWobble() {
    return (g_owned[GEAR_WAX] ? 0.6f : 1.0f) * (chaiFull() ? 0.65f : 1.0f) * D().wobble * (1.0f - 0.04f * skillBonus());
}
// widest landing angle, off the direction of travel, that still rolls away
static float gearSketchy() { return (g_owned[GEAR_WHEELS] ? 1.42f : 1.25f) + D().sketchyBonus; }
static float gearGrip() { return (g_rain > 0.5f && !g_owned[GEAR_WHEELS]) ? 3.5f : 10.0f; }

// ---------------------------------------------------------------- gaps

struct GapDef {
    const char* name;
    float ax0, az0, ax1, az1, aymin;  // takeoff box and minimum takeoff height
    float bx0, bz0, bx1, bz1, bymax;  // landing box and maximum landing height
    float pts;
};
static const GapDef GAPS[] = {
    {"Kicker Gap", -45.8f, -30.5f, -40.5f, -25.5f, 0.5f, -62.0f, -33.0f, -50.3f, -23.0f, 5.0f, 400},
    {"Funbox Launch", -82.5f, -33.2f, -73.5f, -24.8f, 0.9f, -100.0f, -40.0f, -86.3f, -18.0f, 0.3f, 250},
    {"Funbox Launch", -82.5f, -33.2f, -73.5f, -24.8f, 0.9f, -69.7f, -40.0f, -55.0f, -18.0f, 0.3f, 250},
    {"Garden Stair Gap", -112.5f, -34.2f, -108.5f, -25.8f, 1.3f, -103.3f, -38.0f, -88.0f, -22.0f, 0.3f, 600},
    {"Main Road Gap", -1.8f, -11.2f, 1.8f, -8.5f, 0.8f, -12.0f, -1.0f, 12.0f, 14.0f, 5.0f, 800},
    {"Skywalk Drop", -111.0f, -72.5f, -49.0f, -67.5f, 3.0f, -141.0f, -86.0f, -11.0f, -13.0f, 0.5f, 700},
    {"Slab Drop", 53.5f, -38.5f, 78.5f, -23.5f, 2.9f, 30.0f, -44.0f, 85.0f, -12.0f, 0.3f, 600},
    {"Sea Face Steps", 40.0f, 144.5f, 70.0f, 146.3f, 0.9f, 38.0f, 125.0f, 72.0f, 141.8f, 0.5f, 500},
    {"Median Hop", 16.0f, -8.0f, 141.0f, -0.45f, -1.0f, 16.0f, 0.45f, 141.0f, 8.0f, 5.0f, 300},
    {"Median Hop", 16.0f, 0.45f, 141.0f, 8.0f, -1.0f, 16.0f, -8.0f, 141.0f, -0.45f, 5.0f, 300},
    {"Median Hop", -141.0f, -8.0f, -16.0f, -0.45f, -1.0f, -141.0f, 0.45f, -16.0f, 8.0f, 5.0f, 300},
    {"Median Hop", -141.0f, 0.45f, -16.0f, 8.0f, -1.0f, -141.0f, -8.0f, -16.0f, -0.45f, 5.0f, 300},
};
static const int GAP_COUNT = (int)(sizeof(GAPS) / sizeof(GAPS[0]));

static bool inBox(const V3& p, float x0, float z0, float x1, float z1) {
    return p.x >= x0 && p.x <= x1 && p.z >= z0 && p.z <= z1;
}

// ---------------------------------------------------------------- task progress

static bool g_checkChapter = false, g_checkMission = false;

// Defined in tutorial.hpp
static void tutOnTrick(const std::string& name);
static void tutOnGrab(float secs);
static void tutOnCombo(int mult);

static void bump(Goal& g, int amount, bool mission) {
    if (goalDone(g) || amount <= 0) return;
    g.have = std::min(g.need, g.have + amount);
    if (goalDone(g)) {
        popup("TASK DONE: " + g.text, C(0.45f, 1.0f, 0.55f), 26, 2.6f);
        for (int i = 0; i < 40; i++) sparkle(P.pos, C(0.5f, 1.0f, 0.6f));
    } else if (g.need > 1 && g.kind != GK_CHECKPOINT) {
        popup(g.text + "  " + std::to_string(g.have) + "/" + std::to_string(g.need), C(0.8f, 0.95f, 0.8f), 20, 1.6f);
    }
    if (mission) g_checkMission = true;
    else g_checkChapter = true;
}

static Chapter* curChapter() {
    if (!g_careerOn || g_tutorial || g_unlockLevel >= (int)g_chapters.size()) return nullptr;
    return &g_chapters[g_unlockLevel];
}

// Runs fn(goal, isMission) over every open task.
template <typename F> static void forOpenGoals(F fn) {
    Chapter* c = curChapter();
    if (c)
        for (size_t i = 0; i < c->goals.size(); i++)
            if (!goalDone(c->goals[i])) fn(c->goals[i], false);
    if (g_careerOn && g_mis.on)
        for (size_t i = 0; i < g_mis.goals.size(); i++)
            if (!goalDone(g_mis.goals[i])) fn(g_mis.goals[i], true);
}

static std::string lower(std::string s) {
    for (size_t i = 0; i < s.size(); i++) s[i] = (char)tolower((unsigned char)s[i]);
    return s;
}

// True when name contains any of the '|' separated keys, ignoring case.
static bool nameMatches(const std::string& rawName, const std::string& rawKeys) {
    std::string name = lower(rawName), keys = lower(rawKeys);
    size_t start = 0;
    while (start <= keys.size()) {
        size_t bar = keys.find('|', start);
        std::string k = keys.substr(start, bar == std::string::npos ? std::string::npos : bar - start);
        if (!k.empty() && name.find(k) != std::string::npos) return true;
        if (bar == std::string::npos) break;
        start = bar + 1;
    }
    return false;
}

static void evTrick(const std::string& name) {
    forOpenGoals([&](Goal& g, bool m) {
        if (g.kind == GK_TRICK && nameMatches(name, g.key)) bump(g, 1, m);
        if (g.kind == GK_SPIN && (name.rfind("FS ", 0) == 0 || name.rfind("BS ", 0) == 0) &&
            (float)atoi(name.c_str() + 3) >= g.param)
            bump(g, 1, m);
    });
}
static void evGap(const std::string& name) {
    forOpenGoals([&](Goal& g, bool m) { if (g.kind == GK_GAP && g.key == name) bump(g, 1, m); });
}
static void evGrind(int tag, float secs) {
    forOpenGoals([&](Goal& g, bool m) {
        if (g.kind == GK_GRIND && (g.tag == RT_NONE || g.tag == tag) && secs >= g.param) bump(g, 1, m);
    });
}
static void evGrab(float secs) {
    tutOnGrab(secs);
    forOpenGoals([&](Goal& g, bool m) { if (g.kind == GK_GRAB && secs >= g.param) bump(g, 1, m); });
}
static void evCombo(long total, int mult, bool manual) {
    forOpenGoals([&](Goal& g, bool m) {
        if (g.kind == GK_COMBO && (float)total >= g.param) bump(g, 1, m);
        if (g.kind == GK_COMBO_LEN && (float)mult >= g.param) bump(g, 1, m);
        if (g.kind == GK_COMBO_MANUAL && manual && (float)mult >= g.param) bump(g, 1, m);
    });
}
static void evLetters(int got) {
    forOpenGoals([&](Goal& g, bool m) { if (g.kind == GK_LETTERS) bump(g, got - g.have, m); });
}
static void evChai() {
    forOpenGoals([&](Goal& g, bool m) { if (g.kind == GK_CHAI) bump(g, 1, m); });
}
static void evMissionDone(const std::string& key) {
    forOpenGoals([&](Goal& g, bool m) { if (g.kind == GK_MISSION && g.key == key) bump(g, 1, m); });
}
static void evDelivery() {
    forOpenGoals([&](Goal& g, bool m) { if (g.kind == GK_DELIVERIES) bump(g, 1, m); });
}

// Adds any gap the last jump cleared and reports it.
static void checkGaps(const V3& from, const V3& to) {
    std::vector<std::string> done;
    for (int i = 0; i < GAP_COUNT; i++) {
        const GapDef& d = GAPS[i];
        if (from.y < d.aymin || to.y > d.bymax) continue;
        if (!inBox(from, d.ax0, d.az0, d.ax1, d.az1) || !inBox(to, d.bx0, d.bz0, d.bx1, d.bz1)) continue;
        if (std::find(done.begin(), done.end(), std::string(d.name)) != done.end()) continue;
        done.push_back(d.name);
        addTrick(d.name, d.pts);
        evGap(d.name);
    }
}

// Reports every trick from index `from` in the combo as landed.
static void landedTricks(int from) {
    for (size_t i = (size_t)std::max(from, 0); i < g_combo.list.size(); i++) {
        evTrick(g_combo.list[i].name);
        tutOnTrick(g_combo.list[i].name);
    }
}

static const char* HYPE[] = {"EKDUM JHAKAAS!", "KYA BAAT HAI!", "BINDAAS!", "ZABARDAST!", "APUN KA STYLE!",
                             "FULL PAISA VASOOL!", "BHAARI!", "AAG LAGA DI!"};

// Called by bankCombo once a combo is safely landed.
static void careerCombo(long total, const std::vector<TrickEntry>& list) {
    bool manual = false;
    for (size_t i = 0; i < list.size(); i++)
        if (list[i].name.find("Manual") != std::string::npos) manual = true;
    evCombo(total, (int)list.size(), manual);
    if (total >= 400) addXP(total / 20, "combo");
    tutOnCombo((int)list.size());
    if (total >= 2500) popup(HYPE[(int)(fxrand() * 7.99f)], C(1.0f, 0.55f, 0.15f), 44, 1.8f);
    if (trickOpen(UL_CHAI) && g_chai < 1.0f) {
        g_chai = std::min(1.0f, g_chai + (float)total / 5000.0f);
        if (g_chai >= 1.0f) popup("CHAI POWER FULL!  B IN THE AIR: BOMBAY BACKFLIP", C(1.0f, 0.8f, 0.3f), 26, 2.6f);
    }
}

// ---------------------------------------------------------------- chapters

static const V3 SPAWN_GARDEN(-96.0f, 0.0f, -30.0f);
static const V3 SPAWN_SITE(46.0f, SIDEWALK_H, -11.0f);
static const V3 SPAWN_SKYWALK(-26.0f, 0.0f, -70.0f);
static const V3 SPAWN_ROAD(0.0f, 0.0f, -60.0f);

// Re-applies the difficulty to every chapter; safe to call mid-game.
static void applyDifficulty() {
    for (size_t i = 0; i < g_chapters.size(); i++) {
        Chapter& c = g_chapters[i];
        c.timeLimit = c.baseTime * D().timeMul;
        for (size_t k = 0; k < c.goals.size(); k++) scaleGoal(c.goals[k]);
    }
}

static void initChapters() {
    g_chapters.clear();
    Chapter c;

    c = Chapter();
    c.name = "PEHLA DIN";
    c.english = "First day on the block";
    c.brief = "New kid in the plaza. Show the crew you can actually skate.";
    c.reward1 = "UNLOCKED: Heelflip (K), Pop Shove-it (L), grind switches";
    c.reward2 = "The garden steps are open";
    c.spawn = SPAWN_POS; c.spawnYaw = SPAWN_YAW; c.rupees = 200;
    c.goals.push_back(mkGoal(GK_TRICK, "Land {n} kickflip{pl} (J)", "Kickflip", 3, 0).tip("Tap SPACE to jump, then press J while you are in the air.",
                          "Roll at any speed, tap SPACE and press J straight away. The board spins once under you and you land on it by yourself. Bailing? Press J earlier, right after the ollie. Holding SPACE a moment before letting go gives a higher pop and more time."));
    c.goals.push_back(mkGoal(GK_GAP, "Jump the kicker gap", "Kicker Gap", 1, 0).where(-48.5f, 1.2f, -28.0f).tip("Push hard at the wooden kicker and press SPACE at its top edge.",
                          "The kicker is the small wooden ramp in the middle of the plaza, under the green beam. Line up straight, hold W to full speed, and ollie right at the steel lip. You must land on the far ramp, past the gap. Hold SPACE while riding up and let go at the lip for the biggest jump."));
    c.goals.push_back(mkGoal(GK_GRIND, "Grind the yellow plaza bar", "", 1, 0.3f).rail(RT_PLAZA_BAR)
                          .where(-58.0f, 0.5f, -19.0f).tip("Ride alongside the low yellow rail, ollie, and land on top of it.",
                          "The yellow rail is on the north side of the plaza, under the green beam. Ride parallel to it, about an arm's length away, at medium speed. Tap SPACE and steer a little toward it so you come down on top. While grinding, keep the balance needle in the green with A and D."));
    c.goals.push_back(mkGoal(GK_COMBO, "Bank a {p} point combo", "", 1, 1000).tip("Link tricks: land, then jump into the next trick before the blue bar empties.",
                          "A combo scores its points times the number of tricks. Land a trick, then ollie into another one before the thin blue bar under the combo runs out. Grinds, spins, gaps and powerslides (S at speed) all count. Three or four linked tricks usually clear small targets. Bigger ones need grinds and gaps in the chain."));
    c.goals.push_back(mkGoal(GK_MISSION, "Beat Raju in a race to the station", "raju", 1, 0).tip("Talk to Raju (E twice), then follow the orange rings.",
                          "Raju waits in the plaza with a yellow diamond over his head. Stop right next to him and press E, then E again to start. Hold W and follow the arrow to each orange ring, all the way to Dadar station. Stay on the sidewalk to dodge traffic. Press V for a burst of speed on the straights."));
    g_chapters.push_back(c);

    c = Chapter();
    c.name = "SEEDHI PE CHADH";
    c.english = "Take the stairs";
    c.brief = "The mali opened the garden. Heelflips, handrails and your first delivery job.";
    c.reward1 = "UNLOCKED: Indy (I) and Melon (U) grabs";
    c.reward2 = "The construction site is open";
    c.spawn = SPAWN_GARDEN; c.spawnYaw = -PI * 0.5f; c.rupees = 300;
    c.goals.push_back(mkGoal(GK_TRICK, "Land {n} heelflip{pl} (K)", "Heelflip", 5, 0).tip("Tap SPACE to jump, then press K in the air.",
                          "Same as a kickflip but with K. Every heelflip you land counts, even inside a combo or after a jump off a kicker, so a quick way is ollie + K over and over while rolling."));
    c.goals.push_back(mkGoal(GK_GAP, "Ollie the garden stair gap", "Garden Stair Gap", 1, 0)
                          .where(-106.0f, 1.0f, -30.0f).tip("From the top of the garden steps, ollie off and land past the bottom step.",
                          "Ride up the garden steps at the west end of the plaza onto the raised platform. Turn around, push back toward the steps and press SPACE at the very top edge. You have to fly over all eight steps. Hold SPACE on the approach and release at the edge for more height."));
    c.goals.push_back(mkGoal(GK_GRIND, "Grind a garden handrail", "", 1, 0.3f).rail(RT_STAIR)
                          .where(-106.5f, 1.7f, -34.3f).tip("Ollie onto one of the steel handrails running down the garden steps.",
                          "There is a sloped steel handrail on each side of the garden steps. Start on the platform at the top, ride parallel to a rail and tap SPACE to hop onto it. You slide down to the bottom. Keep the balance needle in the green with A and D."));
    c.goals.push_back(mkGoal(GK_SPIN, "Land a {d} spin (hold A or D in the air)", "", 1, 360).tip("Get big air, then hold A or D the whole time you're in the air.",
                          "You spin about 430 degrees a second while holding A or D in the air, so a {d} needs a big jump. Charge the ollie (hold SPACE, let go) or launch off the plaza kicker, and keep holding A until you land. You can land facing forward or backward."));
    c.goals.push_back(mkGoal(GK_DELIVERIES, "Deliver {n} order{pl} from the food stalls (E)", "", 2, 0).tip("Stop at a stall with RS over it, press E twice, then ride to the blue beam.",
                          "Any food cart or stall with a green RS above it has delivery jobs. Stop beside the vendor, press E to hear the order and E again to accept. A blue beam and the arrow show the address. Get there before the timer ends. Bails damage the order, and tricks on the way earn a tip."));
    c.goals.push_back(mkGoal(GK_REACH, "Roll down the cross road to the Sea Face", "", 1, 0)
                          .area(-141.0f, 121.0f, 141.0f, 160.0f, 0.1f).where(0.0f, 0.18f, 124.0f).tip("Take the cross road south, straight down the lane of shops, to the sea.",
                          "From the main road crossing, head south down the cross road. Watch for autos turning around just past the crossing. Keep going down the lane of shops until the paving opens onto the sea. The arrow at the top points the way."));
    g_chapters.push_back(c);

    c = Chapter();
    c.name = "SITE PE SESSION";
    c.english = "Construction site session";
    c.brief = "Shree Sai Developers are on lunch break. Sneak in and session the slabs.";
    c.reward1 = "UNLOCKED: Manual (N) and Nose Manual (M)";
    c.reward2 = "The skywalk is open and C-H-A-I letters appear";
    c.spawn = SPAWN_SITE; c.spawnYaw = PI; c.rupees = 400;
    c.goals.push_back(mkGoal(GK_REACH, "Ride the plank up to the first slab", "", 1, 0)
                          .area(53.75f, -38.25f, 78.25f, -23.75f, 3.0f).where(41.0f, 0.5f, -34.5f).tip("Inside the construction site, push up the wooden plank ramp.",
                          "Enter the site through the gap in the blue fence on the main road side. The long plank ramp leads up to the first concrete floor. Push at it with plenty of speed and you roll up onto the slab."));
    c.goals.push_back(mkGoal(GK_GRIND, "Grind the whole concrete pipe", "", 1, 1.0f).rail(RT_PIPE)
                          .where(59.0f, 1.3f, -19.3f).tip("Ollie onto the big grey pipe near the site gate and ride it to the end.",
                          "The large concrete pipe lies just inside the site gate. Ride alongside it, tap SPACE and steer onto the top. Hold the balance with A and D until you come off the far end."));
    c.goals.push_back(mkGoal(GK_GRAB, "Hold a grab for {s} sec and land it", "", 1, 1.0f).tip("Jump big, hold I (or U), and let go just before you land.",
                          "Grabs need air time. Launch off a kicker or use a charged ollie, press and hold I in the air for {s} seconds, then let go before you touch the ground. Holding the grab while landing is a bail."));
    c.goals.push_back(mkGoal(GK_GAP, "Drop off the slab edge", "Slab Drop", 1, 0).where(66.0f, 3.2f, -24.5f).tip("Ride off any edge of the first concrete slab and land below.",
                          "Get up onto the first slab by the plank ramp, then roll off any open edge. You fall to the ground floor; land facing the way you're rolling. A kickflip on the way down scores extra."));
    c.goals.push_back(mkGoal(GK_MISSION, "Pass Priya's trick challenge", "priya", 1, 0).tip("Find Priya by the garden steps and press E twice.",
                          "Priya waits at the foot of the garden steps. Her challenge wants a varial flip (L then J in one jump), a grind and a combo, all before the clock runs out. Warm up the flips first, then start it."));
    g_chapters.push_back(c);

    c = Chapter();
    c.name = "SKYWALK PE CHAI";
    c.english = "Tea on the skywalk";
    c.brief = "The skywalk is open. Collect C-H-A-I, film a line with Sam, then grab a cutting chai.";
    c.reward1 = "UNLOCKED: CHAI POWER. Big combos fill the meter";
    c.reward2 = "Meter full? Press B in the air for a BOMBAY BACKFLIP";
    c.spawn = SPAWN_SKYWALK; c.spawnYaw = -PI * 0.5f; c.rupees = 500;
    c.goals.push_back(mkGoal(GK_LETTERS, "Collect the letters C-H-A-I", "", 4, 0).tip("Follow the arrow to each floating letter; some need a jump.",
                          "H floats above the kicker gap in the plaza, I over the garden platform, C on the skywalk deck and A above the first slab of the construction site. Ride through each one. The arrow always points at the nearest letter left."));
    c.goals.push_back(mkGoal(GK_GRIND, "Grind the skywalk railing for {s} sec", "", 1, 2.0f).rail(RT_SKYWALK)
                          .where(-80.0f, 4.4f, -68.0f).tip("Ride up onto the skywalk and ollie onto its blue railing.",
                          "Push up either skywalk ramp. Up on the deck, ride parallel to the blue railing and tap SPACE to hop onto it. It is long, so just keep your balance with A and D for {s} seconds."));
    c.goals.push_back(mkGoal(GK_GAP, "Ollie off the skywalk to the ground", "Skywalk Drop", 1, 0)
                          .where(-80.0f, 3.4f, -70.0f).tip("On the skywalk, hold SPACE, then let go to pop over the railing.",
                          "The railing is high, so you need a charged ollie. Ride along the deck angled toward the railing, hold SPACE for half a second, and release. You clear the rail and drop to the plaza. Or grind the railing and ollie off its outside."));
    c.goals.push_back(mkGoal(GK_COMBO_MANUAL, "Bank a {c} trick combo with a manual", "", 1, 4).tip("Land a trick, press N as you land to manual, then ollie into another trick.",
                          "Do a flip, and press N just before you land: you roll straight into a manual. Keep the balance with W and S, then press SPACE to ollie into the next trick. Every trick, the manual included, adds to the count."));
    c.goals.push_back(mkGoal(GK_MISSION, "Film a line with Sam at the site", "sam", 1, 0).tip("Talk to Sam inside the construction site (E twice).",
                          "Sam stands inside the site with a camera. His line needs a drop off the slab, a grind on the concrete pipe and a combo. Plan the route before you start the clock."));
    c.goals.push_back(mkGoal(GK_CHAI, "Buy a cutting chai at a chai stall (F)", "", 1, 0).tip("Stop at a chai stall and press F to buy a cutting chai.",
                          "Chai stalls are blue carts with a CHAI sign, one next to the plaza on the main road. Stop beside the vendor and press F. It costs a few rupees, restores energy and fills chai power."));
    g_chapters.push_back(c);

    c = Chapter();
    c.name = "TRAFFIC KA RAJA";
    c.english = "King of traffic";
    c.brief = "The crew built a kicker on the cross road. Dadar traffic is the final boss.";
    c.reward1 = "Free KAALI-PEELI DECK";
    c.reward2 = "Dark clouds over Dadar. The monsoon is coming";
    c.spawn = SPAWN_ROAD; c.spawnYaw = 0; c.rupees = 700;
    c.goals.push_back(mkGoal(GK_TRICK, "Jump over a BEST bus (road kicker, green light)", "Bus Jump", 1, 0)
                          .where(0.0f, 1.6f, -11.0f).tip("Wait for a red bus, then launch off the road kicker across the main road.",
                          "The kicker is in the middle of the cross road, just north of the crossing (green beam). Start about 30 m north of it. When the main road light turns green and a red bus is coming along the near lane, push hard, press V for boost and ollie at the lip. You fly over the near lanes."));
    c.goals.push_back(mkGoal(GK_GAP, "Clear the main road gap", "Main Road Gap", 1, 0).where(0.0f, 1.6f, -11.0f).tip("Launch off the road kicker and land beyond the middle of the main road.",
                          "Same kicker as the bus jump. Hit it at full speed (boost with V) and ollie at the lip. You need to land past the centre of the main road. Watch for cars where you land."));
    c.goals.push_back(mkGoal(GK_TRICK, "Hop an auto, taxi or car", "Auto Hop|Taxi Hop|Car Hop", 1, 0).tip("Ollie right over a moving auto, taxi or car.",
                          "Autos are 1.8 m tall, so you need a charged ollie (hold SPACE, release) or the Pro Deck from the shop. Ride next to the traffic on the cross road and pop over one as it passes, or use the road kicker."));
    c.goals.push_back(mkGoal(GK_TRICK, "Land a Bombay Backflip (B)", "Bombay Backflip", 1, 0).tip("Fill chai power, then ollie and press B straight away.",
                          "Chai power fills when you bank combos, or at once when you buy a cutting chai (F at a chai stall). When the meter glows, ollie and press B immediately. The flip takes half a second. Launching off a kicker makes it safer."));
    c.goals.push_back(mkGoal(GK_MISSION, "Help Tukaram with the dabba run", "tukaram", 1, 0).tip("Talk to Tukaram in front of Dadar station (E twice).",
                          "Tukaram the dabbawala stands outside the station. You carry three tiffins to three addresses in order, following the blue beam. A bail knocks a tiffin off, so ride carefully and skip risky tricks."));
    c.goals.push_back(mkGoal(GK_COMBO, "Bank a {p} point combo", "", 1, 10000).tip("Link tricks: land, then jump into the next trick before the blue bar empties.",
                          "A combo scores its points times the number of tricks. Land a trick, then ollie into another one before the thin blue bar under the combo runs out. Grinds, spins, gaps and powerslides (S at speed) all count. Three or four linked tricks usually clear small targets. Bigger ones need grinds and gaps in the chain."));
    g_chapters.push_back(c);

    c = Chapter();
    c.name = "BAARISH SESSION";
    c.english = "Monsoon session";
    c.brief = "It is pouring. Slippery roads, the whole crew out. Beat the score before the rain stops.";
    c.reward1 = "GOLD DECK unlocked";
    c.reward2 = "You are a Mumbai Skate legend";
    c.baseTime = 150;
    c.spawn = SPAWN_POS; c.spawnYaw = SPAWN_YAW; c.rupees = 1000;
    c.goals.push_back(mkGoal(GK_SCORE, "Score {p} points before time runs out", "", 1, 20000).tip("Keep chaining combos anywhere before the clock hits zero.",
                          "Only banked points count. Long combos score far more than single tricks, so link kickflips, grinds, manuals and powerslides. The plaza has everything close together. The roads are slippery in the rain unless you bought soft wheels."));
    c.goals.push_back(mkGoal(GK_COMBO_LEN, "Bank a {c} trick combo in the rain", "", 1, 6).tip("Link {c} tricks with manuals and powerslides between jumps.",
                          "Manuals (N) and powerslides (S at speed) keep a combo alive between jumps. A good pattern is ollie + flip, land into a manual, ollie + flip, repeat."));
    g_chapters.push_back(c);
    applyDifficulty();
}

// ---------------------------------------------------------------- chapter flow

enum CardType { CARD_NONE, CARD_INTRO, CARD_COMPLETE, CARD_LEGEND };
static int g_card = CARD_NONE;
static float g_cardT = 0;
static int g_cardChapter = 0;
static std::string g_cardBonus;
static long g_chapterScore0 = 0;
static float g_chTimer = 0;
static int g_lettersGot = 0;
static bool g_teleport = false;  // ask the driver to snap the camera after a respawn

static void saveGame();

static void startChapterClock() {
    g_chapterScore0 = g_score;
    Chapter* c = curChapter();
    g_chTimer = c ? c->timeLimit : 0;
}

static void spawnAtChapter() {
    Chapter* c = curChapter();
    V3 p = c ? c->spawn : SPAWN_POS;
    float yaw = c ? c->spawnYaw : SPAWN_YAW;
    resetPlayer(p, yaw);
    g_combo.list.clear();
    g_combo.grace = 0;
    g_teleport = true;
}

static void completeChapter() {
    Chapter& c = g_chapters[g_unlockLevel];
    g_cardChapter = g_unlockLevel;
    g_unlockLevel++;
    g_card = CARD_COMPLETE;
    g_cardT = 0;
    long bonus = 2500L * g_unlockLevel;
    g_score += bonus;
    g_rupees += c.rupees;
    g_cardBonus = "+" + withCommas(bonus) + " POINTS    +RS " + std::to_string(c.rupees);
    if (g_unlockLevel == UL_RAIN) g_owned[DECK_KAALIPEELI] = true;
    if (g_unlockLevel == UL_LEGEND) {
        g_owned[DECK_GOLD] = true;
        g_deck = DECK_GOLD;
    }
    confetti(P.pos);
    g_shake = 0.3f;
    addXP(500, "chapter");
    saveGame();
}

static void checkChapterDone() {
    Chapter* c = curChapter();
    if (!c) return;
    for (size_t i = 0; i < c->goals.size(); i++)
        if (!goalDone(c->goals[i])) return;
    completeChapter();
}


static const char* FOOD_LINES[] = {"Ek cutting, bhaiya!", "Garam garam!", "Khao, piyo, skate karo!",
                                   "Taaza hai, ekdum!"};

// Buys the food sold at a stall. Returns false if the player can't pay.
static bool buyFood(int stallIdx) {
    const Stall& st = g_stalls[(size_t)stallIdx];
    const Food& f = FOODS[st.kind];
    if (g_rupees < f.price) {
        popup("NOT ENOUGH RUPEES. DELIVERIES PAY!", C(1.0f, 0.5f, 0.4f), 22, 1.8f);
        return false;
    }
    if (g_energy >= 99.5f && st.kind != STALL_CHAI) {
        popup("YOU'RE FULL. COME BACK LATER", C(0.85f, 0.85f, 0.9f), 22, 1.6f);
        return false;
    }
    g_rupees -= f.price;
    g_energy = std::min(100.0f, g_energy + f.energy);
    char buf[96];
    snprintf(buf, sizeof(buf), "%s  -RS %d  +%d ENERGY", f.name, f.price, (int)f.energy);
    popup(buf, C(1.0f, 0.8f, 0.45f), 24, 2.0f);
    popup(FOOD_LINES[(int)(fxrand() * 3.99f)], C(1.0f, 0.95f, 0.7f), 20, 1.6f);
    if (st.kind == STALL_CHAI) {
        if (trickOpen(UL_CHAI)) {
            g_chai = 1.0f;
            popup("CHAI POWER FULL!", C(1.0f, 0.8f, 0.3f), 24, 2.0f);
        }
        evChai();
    }
    return true;
}

// Per-step career bookkeeping: cards, timers, area tasks, chai stops.
static void careerUpdate(float dt) {
    if (g_lockMsgT > 0) g_lockMsgT -= dt;
    // energy: boosting burns it, resting slowly brings it back up to about half
    if (g_boostT > 0) g_boostT = std::max(0.0f, g_boostT - dt);
    else if (g_energy < ENERGY_REST_CAP) g_energy = std::min(ENERGY_REST_CAP, g_energy + dt * (hspeed() < 0.5f ? 6.0f : 1.2f));
    if (!g_careerOn) return;
    if (g_card != CARD_NONE) {
        g_cardT += dt;
        if (g_card == CARD_COMPLETE && g_cardT > 5.0f) {
            if (g_unlockLevel < (int)g_chapters.size()) {
                g_card = CARD_INTRO;
                g_cardT = 0;
                spawnAtChapter();
                startChapterClock();
            } else {
                g_card = CARD_LEGEND;
                g_cardT = 0;
            }
        } else if (g_card == CARD_INTRO && g_cardT > 6.0f) {
            g_card = CARD_NONE;
        } else if (g_card == CARD_LEGEND && g_cardT > 9.0f) {
            g_card = CARD_NONE;
        }
    }
    Chapter* c = curChapter();
    if (c) {
        for (size_t i = 0; i < c->goals.size(); i++) {
            Goal& g = c->goals[i];
            if (goalDone(g)) continue;
            if (g.kind == GK_REACH && P.state == P_GROUND && P.pos.y >= g.bymin &&
                inBox(P.pos, g.bx0, g.bz0, g.bx1, g.bz1))
                bump(g, 1, false);
            if (g.kind == GK_SCORE && (float)(g_score - g_chapterScore0) >= g.param) bump(g, 1, false);
        }
        if (c->timeLimit > 0 && g_card == CARD_NONE) {
            g_chTimer -= dt;
            if (g_chTimer <= 0) {
                popup("TIME UP! THE RAIN WAITS FOR NO ONE. AGAIN!", C(1.0f, 0.5f, 0.4f), 30, 3.0f);
                for (size_t i = 0; i < c->goals.size(); i++) c->goals[i].have = 0;
                startChapterClock();
            }
        }
    }
    if (g_checkChapter) {
        g_checkChapter = false;
        checkChapterDone();
        saveGame();
    }
    // the sky follows the story: rain only during the monsoon chapter
    float wantRain = (g_unlockLevel == UL_RAIN && c) ? 1.0f : 0.0f;
    g_rain = approach(g_rain, wantRain, dt * 0.25f);
}

// ---------------------------------------------------------------- save file

static std::string savePath() {
    const char* h = getenv("HOME");
    return std::string(h ? h : ".") + "/.mumbai_skate_save";
}

static void saveGame() {
    if (g_noSave || !g_careerOn) return;
    FILE* f = fopen(savePath().c_str(), "w");
    if (!f) return;
    int mask = 0;
    for (size_t i = 0; i < g_letters.size(); i++)
        if (g_letters[i].got) mask |= 1 << i;
    fprintf(f, "mumbai-skate-save 2\n");
    fprintf(f, "level %d\nscore %ld\nbest %ld\nrupees %d\nletters %d\ndeliveries %d\nchai %.3f\n", g_unlockLevel,
            g_score, g_bestCombo, g_rupees, mask, g_deliveries, g_chai);
    fprintf(f, "friends %d %d %d %d %d\n", g_friend[0], g_friend[1], g_friend[2], g_friend[3], g_friend[4]);
    fprintf(f, "owned");
    for (int i = 0; i < GEAR_COUNT; i++) fprintf(f, " %d", g_owned[i] ? 1 : 0);
    fprintf(f, "\ndeck %d\noutfit %d\ndifficulty %d\ntutorial %d\n", g_deck, g_outfit, g_diff, g_tutorialDone ? 1 : 0);
    fprintf(f, "wear %d %d %d %d\nenergy %.1f\nxp %ld\ngraphics %d\n", g_hat, g_glasses, g_neck, g_bag, g_energy, g_xp, g_gfx);
    Chapter* c = curChapter();
    if (c && c->timeLimit <= 0) {
        fprintf(f, "goals");
        for (size_t i = 0; i < c->goals.size(); i++) fprintf(f, " %d", c->goals[i].have);
        fprintf(f, "\n");
    }
    fclose(f);
}

static bool loadGame() {
    FILE* f = fopen(savePath().c_str(), "r");
    if (!f) return false;
    char line[512];
    if (!fgets(line, sizeof(line), f) || strncmp(line, "mumbai-skate-save 2", 19) != 0) {
        fclose(f);
        return false;
    }
    std::vector<int> goals;
    int mask = 0;
    while (fgets(line, sizeof(line), f)) {
        char key[32];
        int off = 0;
        if (sscanf(line, "%31s %n", key, &off) != 1) continue;
        const char* rest = line + off;
        std::string k = key;
        if (k == "level") g_unlockLevel = clampf((float)atoi(rest), 0, (float)g_chapters.size());
        else if (k == "score") g_score = atol(rest);
        else if (k == "best") g_bestCombo = atol(rest);
        else if (k == "rupees") g_rupees = atoi(rest);
        else if (k == "letters") mask = atoi(rest);
        else if (k == "deliveries") g_deliveries = atoi(rest);
        else if (k == "chai") g_chai = clampf((float)atof(rest), 0, 1);
        else if (k == "deck") g_deck = atoi(rest);
        else if (k == "outfit") g_outfit = atoi(rest);
        else if (k == "difficulty") g_diff = (int)clampf((float)atoi(rest), 0, 2);
        else if (k == "tutorial") g_tutorialDone = atoi(rest) != 0;
        else if (k == "energy") g_energy = clampf((float)atof(rest), 0, 100);
        else if (k == "xp") g_xp = std::max(0L, atol(rest));
        else if (k == "graphics") g_gfx = (int)clampf((float)atoi(rest), 0, 2);
        else if (k == "wear") sscanf(rest, "%d %d %d %d", &g_hat, &g_glasses, &g_neck, &g_bag);
        else if (k == "friends" || k == "owned" || k == "goals") {
            std::vector<int> v;
            const char* p = rest;
            char* end = nullptr;
            for (;;) {
                long x = strtol(p, &end, 10);
                if (end == p) break;
                v.push_back((int)x);
                p = end;
            }
            if (k == "friends")
                for (int i = 0; i < 5 && i < (int)v.size(); i++) g_friend[i] = v[i] != 0;
            if (k == "owned")
                for (int i = 0; i < GEAR_COUNT && i < (int)v.size(); i++) g_owned[i] = v[i] != 0;
            if (k == "goals") goals = v;
        }
    }
    fclose(f);
    g_lettersGot = 0;
    for (size_t i = 0; i < g_letters.size(); i++) {
        g_letters[i].got = (mask >> i) & 1;
        g_lettersGot += g_letters[i].got ? 1 : 0;
    }
    applyDifficulty();
    Chapter* c = curChapter();
    if (c && goals.size() == c->goals.size())
        for (size_t i = 0; i < goals.size(); i++) c->goals[i].have = std::min(goals[i], c->goals[i].need);
    if (g_deck >= GEAR_COUNT || (g_deck >= 0 && !g_owned[g_deck])) g_deck = -1;
    if (g_outfit >= GEAR_COUNT || (g_outfit >= 0 && !g_owned[g_outfit])) g_outfit = -1;
    int* slots[4] = {&g_hat, &g_glasses, &g_neck, &g_bag};
    for (int i = 0; i < 4; i++)
        if (*slots[i] >= GEAR_COUNT || (*slots[i] >= 0 && !g_owned[*slots[i]])) *slots[i] = -1;
    return true;
}
