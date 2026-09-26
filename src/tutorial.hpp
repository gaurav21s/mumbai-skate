#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Tutorial and menus
//
// The tutorial runs before chapter 1 on a new game. Each step names a move,
// says what it is, shows the keys, and waits until the player lands it.
// Every trick can be tried here, even ones the story unlocks later.
//
// The title screen picks continue or new game, difficulty and the tutorial;
// P opens the same kind of menu mid-game.
// ============================================================================

enum TutKind { TS_SPEED, TS_TURN, TS_STOP, TS_AIR, TS_POP, TS_TRICK, TS_SPIN, TS_GRAB, TS_GRIND, TS_MANUAL,
               TS_COMBO, TS_SLIDE, TS_BOOST, TS_TALK };

struct TutStep {
    const char* title;
    const char* line1;
    const char* line2;
    const char* keys;   // key caps to draw, separated by spaces
    int kind;
    const char* key;    // trick name to match for TS_TRICK
    float param;
    int unlockLevel;    // chapter the story opens this move at (0 = always open)
};

static const TutStep TUT[] = {
    {"PUSH", "Hold W to kick-push. Get up to 15 km/h.", "Let go and you keep rolling.", "W", TS_SPEED, "", 4.2f, 0},
    {"CARVE", "Lean into turns with A and D.", "Turning is slower at speed, like a real board.", "A D", TS_TURN, "", 3.0f, 0},
    {"BRAKE", "Tap S to drag your foot and slow down. Come to a full stop.", "", "S", TS_STOP, "", 0, 0},
    {"OLLIE", "Tap SPACE to ollie: you snap the tail and the board jumps with you.",
     "Every trick in the air starts with an ollie.", "SPACE", TS_AIR, "", 0, 0},
    {"BIG POP", "Hold SPACE to crouch, then let go. The longer the crouch, the higher the pop.",
     "Watch the POP bar under the skater. Clear 1.4 metres.", "SPACE", TS_POP, "", 1.4f, 0},
    {"KICKFLIP", "Ollie, then press J in the air.",
     "Your front foot flicks off the toe edge and the board flips a full turn along its length.", "SPACE J",
     TS_TRICK, "Kickflip", 0, 0},
    {"HEELFLIP", "Ollie, then press K in the air.",
     "The same flip the other way: kicked off your heel, so the board spins toward your toes.", "SPACE K",
     TS_TRICK, "Heelflip", 0, 1},
    {"POP SHOVE-IT", "Ollie, then press L in the air.",
     "The board spins flat under you, half a turn, while you keep facing forward.", "SPACE L", TS_TRICK,
     "Shove-it", 0, 1},
    {"MIXING FLIPS", "Press two flip keys in one jump.",
     "L then J = Varial Kickflip.  J then J = Double Kickflip.  L L then J = 360 Flip.", "L J", TS_TRICK,
     "Varial|Double|360 Flip|Laser Flip|Triple", 0, 1},
    {"SPIN", "Hold A or D in the air to turn your whole body. Land a 180.",
     "After a 180 you roll backwards. That is called fakie, and it is fine.", "A D", TS_SPIN, "", 180, 0},
    {"GRAB", "Ollie, then hold I (indy) or U (melon) to grab the board.",
     "Holding longer scores more. Let go before you land or you'll fall.", "I U", TS_GRAB, "", 0, 2},
    {"GRIND", "Ollie onto the yellow rail and land on it. Hold the grind for 1 second.",
     "The balance bar drifts. Keep it in the green with A and D.", "SPACE A D", TS_GRIND, "", 1.0f, 0},
    {"MANUAL", "While rolling, press N to lift the front wheels. Hold it 1.5 seconds.",
     "W and S keep the balance bar centred. Press N again to put the nose down.", "N W S", TS_MANUAL, "", 1.5f, 3},
    {"COMBO", "Link tricks: after you land, ollie or manual again before the blue bar runs out.",
     "Each linked trick adds to the multiplier. Bank a x3 combo.", "SPACE N", TS_COMBO, "", 3, 0},
    {"POWERSLIDE", "At speed, hold S. The board kicks sideways and you slide to a stop.",
     "A powerslide right after landing also keeps a combo going.", "W S", TS_SLIDE, "", 0, 0},
    {"BOOST", "Press V for a burst of speed. It uses energy, the bar under your score.",
     "Energy refills slowly by itself. Food from a stall (F) fills it all the way.", "V", TS_BOOST, "", 0, 0},
    {"TALK", "People with a yellow diamond have work for you. Ride up to Raju, stop, press E.",
     "E accepts an offer, Q says no. That is it. The block is yours.", "E", TS_TALK, "", 0, 0},
};
static const int TUT_COUNT = (int)(sizeof(TUT) / sizeof(TUT[0]));

static int g_tutStep = 0;
static float g_tutDoneT = -1;   // counting down to the next step after a success
static float g_tutTurn = 0, g_tutLastYaw = 0, g_tutPeak = 0;
static bool g_tutMoving = false;
static int g_tutPrevState = P_GROUND;
static const V3 TUT_SPAWN(-20.0f, 0.0f, -26.0f);

static void tutReset() {
    g_tutDoneT = -1;
    g_tutTurn = 0;
    g_tutLastYaw = P.yaw;
    g_tutPeak = 0;
    g_tutMoving = false;
}

static void startTutorial() {
    g_tutorial = true;
    g_tutStep = 0;
    endMission();
    resetPlayer(TUT_SPAWN, SPAWN_YAW);
    g_combo.list.clear();
    g_teleport = true;
    g_card = CARD_NONE;
    tutReset();
}

static void endTutorial() {
    g_tutorial = false;
    g_tutorialDone = true;
    g_card = curChapter() ? CARD_INTRO : CARD_NONE;
    g_cardT = 0;
    startChapterClock();
    popup("TUTORIAL COMPLETE!", C(0.4f, 1.0f, 0.6f), 36, 2.5f);
    addXP(100, "tutorial");
    saveGame();
}

static void tutPass() {
    if (!g_tutorial || g_tutDoneT >= 0) return;
    g_tutDoneT = 1.1f;
    static const char* nice[] = {"NICE!", "SHABAASH!", "PERFECT!", "BINDAAS!", "GOT IT!"};
    popup(nice[g_tutStep % 5], C(0.4f, 1.0f, 0.6f), 34, 1.2f);
    for (int k = 0; k < 30; k++) sparkle(P.pos, C(0.4f, 1.0f, 0.8f));
}

static void tutSkip() {
    if (!g_tutorial) return;
    g_tutDoneT = 0.01f;
}

// Hooks called from career.hpp
static void tutOnTrick(const std::string& name) {
    if (!g_tutorial || g_tutStep >= TUT_COUNT) return;
    const TutStep& s = TUT[g_tutStep];
    if (s.kind == TS_TRICK && nameMatches(name, s.key)) tutPass();
    if (s.kind == TS_SPIN && (name.rfind("FS ", 0) == 0 || name.rfind("BS ", 0) == 0) &&
        (float)atoi(name.c_str() + 3) >= s.param)
        tutPass();
}
static void tutOnGrab(float) {
    if (g_tutorial && g_tutStep < TUT_COUNT && TUT[g_tutStep].kind == TS_GRAB) tutPass();
}
static void tutOnCombo(int mult) {
    if (g_tutorial && g_tutStep < TUT_COUNT && TUT[g_tutStep].kind == TS_COMBO && (float)mult >= TUT[g_tutStep].param)
        tutPass();
}

static void tutorialUpdate(float dt) {
    if (!g_tutorial) return;
    if (g_tutDoneT >= 0) {
        g_tutDoneT -= dt;
        if (g_tutDoneT < 0) {
            g_tutStep++;
            tutReset();
            if (g_tutStep >= TUT_COUNT) endTutorial();
        }
        g_tutPrevState = P.state;
        return;
    }
    const TutStep& s = TUT[g_tutStep];
    bool landed = g_tutPrevState == P_AIR && P.state == P_GROUND;
    switch (s.kind) {
        case TS_SPEED:
            if (hspeed() >= s.param) tutPass();
            break;
        case TS_TURN:
            if (P.state == P_GROUND && hspeed() > 1.0f) g_tutTurn += fabsf(wrapAngle(P.yaw - g_tutLastYaw));
            g_tutLastYaw = P.yaw;
            if (g_tutTurn >= s.param) tutPass();
            break;
        case TS_STOP:
            if (hspeed() > 3.0f) g_tutMoving = true;
            if (g_tutMoving && hspeed() < 0.3f && P.state == P_GROUND) tutPass();
            break;
        case TS_AIR:
            if (landed) tutPass();
            break;
        case TS_POP:
            if (P.state == P_AIR) g_tutPeak = std::max(g_tutPeak, P.pos.y - P.takeoff.y);
            if (landed) {
                if (g_tutPeak >= s.param) tutPass();
                else popup("HOLD SPACE LONGER BEFORE LETTING GO", C(1.0f, 0.8f, 0.5f), 22, 1.4f);
                g_tutPeak = 0;
            }
            break;
        case TS_GRIND:
            if (P.state == P_GRIND && P.railTime >= s.param) tutPass();
            break;
        case TS_MANUAL:
            if (P.state == P_GROUND && P.manual && P.manualTime >= s.param) tutPass();
            break;
        case TS_SLIDE:
            if (P.sliding && hspeed() > 3.0f) tutPass();
            break;
        case TS_BOOST:
            if (g_boostT > 0) tutPass();
            break;
        case TS_TALK:
            if (g_talk.open && g_talk.kind == TALK_FRIEND) tutPass();
            break;
        default: break;
    }
    g_tutPrevState = P.state;
}

// Where the tutorial wants the player to go, if anywhere.
static bool tutTarget(V3& out, std::string& label) {
    if (!g_tutorial || g_tutStep >= TUT_COUNT) return false;
    if (TUT[g_tutStep].kind == TS_GRIND) {
        out = V3(-58.0f, 0.5f, -19.0f);
        label = "YELLOW RAIL";
        return true;
    }
    if (TUT[g_tutStep].kind == TS_TALK) {
        out = g_friends[0].pos;
        label = "TALK TO RAJU";
        return true;
    }
    return false;
}

// ---------------------------------------------------------------- menus

enum MenuKind { MENU_NONE, MENU_TITLE, MENU_PAUSE };
static int g_menu = MENU_NONE;
static int g_menuSel = 0;
static bool g_hasProgress = false;   // a save with progress exists
static bool g_tutorialChoice = true;
static bool g_newGameArmed = false;  // NEW GAME asks twice
static bool g_quitRequested = false;

static void resetProgress() {
    g_unlockLevel = 0;
    g_score = g_bestCombo = 0;
    g_rupees = 100;
    g_energy = 100;
    g_hat = g_glasses = g_neck = g_bag = -1;
    g_deliveries = 0;
    g_chai = 0;
    g_deck = g_outfit = -1;
    g_tutorialDone = false;
    for (int i = 0; i < GEAR_COUNT; i++) g_owned[i] = false;
    for (int i = 0; i < 5; i++) g_friend[i] = false;
    for (size_t i = 0; i < g_letters.size(); i++) g_letters[i].got = false;
    g_lettersGot = 0;
    for (size_t c = 0; c < g_chapters.size(); c++)
        for (size_t k = 0; k < g_chapters[c].goals.size(); k++) g_chapters[c].goals[k].have = 0;
    endMission();
    for (size_t i = 0; i < g_friends.size(); i++) {
        g_friends[i].pos = g_friends[i].home;
        g_friends[i].yaw = g_friends[i].homeYaw;
    }
    g_hasProgress = false;
}

static int titleItems() { return g_hasProgress ? 6 : 5; }
// title rows: play, difficulty, graphics, tutorial, [new game], quit
static int titleRow(int sel) {
    if (!g_hasProgress && sel >= 4) return sel + 1;
    return sel;
}

static void changeGraphics(int d) {
    g_gfx = (g_gfx + d + 3) % 3;
    saveGame();
}

static void startPlaying() {
    g_menu = MENU_NONE;
    g_newGameArmed = false;
    if (g_tutorialChoice && (!g_hasProgress || !g_tutorialDone)) {
        startTutorial();
    } else {
        g_card = curChapter() ? CARD_INTRO : CARD_NONE;
        g_cardT = 0;
        startChapterClock();
        spawnAtChapter();
    }
}

static void changeDifficulty(int d) {
    g_diff = (g_diff + d + 3) % 3;
    applyDifficulty();
    saveGame();
}

static void menuKey(int key) {
    // key: 0 up, 1 down, 2 left, 3 right, 4 select, 5 back
    if (g_menu == MENU_TITLE) {
        int n = titleItems();
        if (key == 0) g_menuSel = (g_menuSel + n - 1) % n;
        if (key == 1) g_menuSel = (g_menuSel + 1) % n;
        if (key != 4) g_newGameArmed = g_newGameArmed && key != 0 && key != 1;
        int row = titleRow(g_menuSel);
        if (row == 1 && (key == 2 || key == 3 || key == 4)) changeDifficulty(key == 2 ? -1 : 1);
        if (row == 2 && (key == 2 || key == 3 || key == 4)) changeGraphics(key == 2 ? -1 : 1);
        if (row == 3 && (key == 2 || key == 3 || key == 4)) g_tutorialChoice = !g_tutorialChoice;
        if (key == 4) {
            if (row == 0) startPlaying();
            if (row == 4) {
                if (!g_newGameArmed) {
                    g_newGameArmed = true;
                } else {
                    resetProgress();
                    g_tutorialChoice = true;
                    saveGame();
                    startPlaying();
                }
            }
            if (row == 5) g_quitRequested = true;
        }
        if (key == 5) g_quitRequested = true;
        return;
    }
    if (g_menu == MENU_PAUSE) {
        const int n = 5;
        if (key == 0) g_menuSel = (g_menuSel + n - 1) % n;
        if (key == 1) g_menuSel = (g_menuSel + 1) % n;
        if (g_menuSel == 1 && (key == 2 || key == 3 || key == 4)) changeDifficulty(key == 2 ? -1 : 1);
        if (g_menuSel == 2 && (key == 2 || key == 3 || key == 4)) changeGraphics(key == 2 ? -1 : 1);
        if (key == 4) {
            if (g_menuSel == 0) g_menu = MENU_NONE;
            if (g_menuSel == 3) {
                g_menu = MENU_NONE;
                if (g_tutorial) {
                    g_tutorial = false;
                    endTutorial();
                } else {
                    startTutorial();
                }
            }
            if (g_menuSel == 4) g_quitRequested = true;
        }
        if (key == 5) g_menu = MENU_NONE;
    }
}
