#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Player
// ============================================================================

enum PState { P_GROUND, P_AIR, P_GRIND, P_BAIL };
enum GrindType { G_5050, G_BOARDSLIDE, G_NOSEGRIND, G_50, G_CROOKED };
static const char* GRIND_NAMES[5] = {"50-50 Grind", "Boardslide", "Nosegrind", "5-0 Grind", "Crooked Grind"};
static const float GRIND_BASE[5] = {100, 150, 150, 150, 200};
static const float GRIND_RATE[5] = {250, 300, 300, 300, 350};

enum FlipCode { F_KICK, F_DKICK, F_TKICK, F_HEEL, F_DHEEL, F_THEEL, F_SHOVE, F_360SHOVE, F_VARKICK, F_VARHEEL,
                F_TREFLIP, F_LASER, F_COUNT };
static const char* FLIP_NAMES[F_COUNT] = {"Kickflip", "Double Kickflip", "Triple Kickflip", "Heelflip",
                                          "Double Heelflip", "Triple Heelflip", "Pop Shove-it", "360 Shove-it",
                                          "Varial Kickflip", "Varial Heelflip", "360 Flip", "Laser Flip"};
static const float FLIP_PTS[F_COUNT] = {100, 300, 700, 100, 300, 700, 80, 250, 250, 250, 600, 600};

// key: 0 = kickflip key, 1 = heelflip key, 2 = shove-it key. Returns -1 if the
// press starts a new trick instead of upgrading the one in progress.
static int flipUpgrade(int code, int key) {
    switch (code) {
        case F_KICK: return key == 0 ? F_DKICK : (key == 2 ? F_VARKICK : -1);
        case F_DKICK: return key == 0 ? F_TKICK : -1;
        case F_HEEL: return key == 1 ? F_DHEEL : (key == 2 ? F_VARHEEL : -1);
        case F_DHEEL: return key == 1 ? F_THEEL : -1;
        case F_SHOVE: return key == 2 ? F_360SHOVE : (key == 0 ? F_VARKICK : (key == 1 ? F_VARHEEL : -1));
        case F_360SHOVE: return key == 0 ? F_TREFLIP : (key == 1 ? F_LASER : -1);
        case F_VARKICK: return key == 2 ? F_TREFLIP : -1;
        case F_VARHEEL: return key == 2 ? F_LASER : -1;
        default: return -1;
    }
}

static const float GRAVITY = 20.0f;
static const float SLOPE_G = 14.0f;
static const float OLLIE_V = 6.8f;
static const float BRAKE = 8.0f;
static const float MAX_SPEED = 22.0f;
static const float SPIN_RATE = 7.5f;    // rad/s
static const float FLIP_RATE = 1100.0f; // deg/s
static const float SHOVE_RATE = 800.0f; // deg/s
static const float STEP_UP = 0.32f;
// ---------------------------------------------------------------- difficulty
// One table holds every number the three modes change. Easy opens all tricks
// from the start; medium and hard unlock them chapter by chapter.
enum Difficulty { DIFF_EASY, DIFF_MEDIUM, DIFF_HARD };
struct DiffTune {
    const char* name;
    const char* blurb;
    float wobble;        // grind and manual wobble
    float sketchyBonus;  // extra landing angle (radians) that still rolls away
    float catchDeg;      // unfinished flip rotation that still lands
    float slam;          // wall impact speed that knocks you down
    float catchR;        // how far off a rail you can be and still lock on
    float grace;         // seconds to link the next trick after landing
    float timeMul;       // mission and chapter timers
    float rajuSpeed;
    float countMul;      // "land 5 heelflips" style counts
    float scoreMul;      // combo and score targets
    float holdMul;       // grind and grab durations
    float spoil;         // delivery damage per bail
    float chaiSpoil;     // cutting chai damage per bail
    float dropBail;      // landing speed that is too hard
    bool allTricks;
    bool pedBail;
};
static const DiffTune DIFFS[3] = {
    {"EASY", "Every trick open from the start. Soft landings, long timers, smaller targets.",
     0.45f, 0.35f, 80.0f, 99.0f, 0.75f, 1.4f, 1.7f, 6.4f, 0.6f, 0.5f, 0.5f, 0.2f, 0.35f, -30.0f, true, false},
    {"MEDIUM", "Tricks unlock with the story. Fair timers and forgiving balance.",
     0.72f, 0.15f, 55.0f, 12.0f, 0.62f, 1.0f, 1.3f, 7.3f, 0.8f, 0.75f, 0.75f, 0.3f, 0.5f, -24.0f, false, true},
    {"HARD", "Tight timers, strict landings, full targets. The original rules.",
     1.0f, 0.0f, 45.0f, 9.0f, 0.5f, 0.75f, 1.0f, 8.2f, 1.0f, 1.0f, 1.0f, 0.34f, 1.0f, -21.0f, false, true},
};
static int g_diff = DIFF_MEDIUM;

// ---------------------------------------------------------------- graphics
// Three presets trade looks for heat and battery. The frame cap matters most:
// an uncapped game keeps the graphics chip at full load the whole time.
enum GfxLevel { GFX_LOW, GFX_MEDIUM, GFX_HIGH };
struct GfxTune {
    const char* name;
    const char* blurb;
    int fps;            // frame cap
    float drawDist;     // metres; fog hides the edge
    bool msaa;
    bool worldShadows;  // baked sun shadows of the city
    int dynShadows;     // 0 skater only, 1 plus traffic and friends, 2 plus pedestrians
    bool clouds;
    float rainMul;
    float pedDist;      // pedestrians further than this are not drawn
};
static const GfxTune GFXS[3] = {
    {"LOW", "30 fps, short view, simple shadows. Coolest and longest battery.", 30, 130.0f, false, false, 0, false, 0.35f, 60.0f},
    {"MEDIUM", "60 fps, medium view, city and traffic shadows.", 60, 200.0f, true, true, 1, true, 0.7f, 90.0f},
    {"HIGH", "60 fps, full view and every shadow. Runs warmest.", 60, 290.0f, true, true, 2, true, 1.0f, 140.0f},
};
static int g_gfx = GFX_MEDIUM;
static const GfxTune& G() { return GFXS[g_gfx]; }
static const DiffTune& D() { return DIFFS[g_diff]; }
static float comboGrace() { return D().grace; }
static const V3 SPAWN_POS(-24.0f, 0.0f, -28.0f);  // lined up with the first kicker
static const float SPAWN_YAW = -PI * 0.5f;

struct Player {
    V3 pos, vel;
    float yaw;
    int state;
    // air
    float airTime, spinAccum, spinVel, maxY;
    float roll, rollTarget, shove, shoveTarget;  // board flip animation in degrees
    int flipCode, flipIdx;
    int grab, grabIdx;
    float grabTime;
    int airStartCount;
    std::vector<int> hopped;
    float manualBuffer;
    // grind
    int rail, grindDir, grindType, grindIdx;
    float railT, grindSpeed, grindTime;
    float balance, balVel;
    // manual
    bool manual, noseManual;
    int manualIdx;
    float manualTime;
    // ground
    float slopeVy;
    float pushT;
    float pushPhase;     // 0..1 through one kick-push stroke
    float pushDir;       // +1 pushing toward the nose, -1 rolling fakie
    bool pushing;
    float crouch;
    float landT;
    V3 lastSafe;
    // bail
    float bailT;
    V3 boardPos, boardVel;
    float boardSpin;
    // misc
    float railCooldown;
    int lastRail;
    float ollieCharge, ollieHoldT;
    // career and feel
    V3 takeoff;          // where the current jump left the ground, for gaps
    float airGrabT;      // longest grab held during this jump
    bool backflipOn;
    float backflip;      // Bombay Backflip rotation in degrees
    float railTime;      // seconds on the current rail
    bool sliding;        // powerslide
    float slideAng, slideDir;
    float lean;          // carve lean in degrees
};
static Player P;

static void resetPlayer(const V3& pos, float yaw) {
    P = Player();
    P.pos = pos;
    P.vel = V3(0, 0, 0);
    P.yaw = yaw;
    P.state = P_GROUND;
    P.airTime = P.spinAccum = P.spinVel = 0;
    P.maxY = pos.y;
    P.roll = P.rollTarget = P.shove = P.shoveTarget = 0;
    P.flipCode = -1;
    P.flipIdx = -1;
    P.grab = 0;
    P.grabIdx = -1;
    P.grabTime = 0;
    P.airStartCount = 0;
    P.manualBuffer = 0;
    P.rail = -1;
    P.grindDir = 1;
    P.grindType = 0;
    P.grindIdx = -1;
    P.railT = P.grindSpeed = P.grindTime = 0;
    P.balance = P.balVel = 0;
    P.manual = P.noseManual = false;
    P.manualIdx = -1;
    P.manualTime = 0;
    P.slopeVy = 0;
    P.pushT = 0;
    P.pushing = false;
    P.crouch = 0;
    P.landT = 1;
    P.lastSafe = pos;
    P.bailT = 0;
    P.boardSpin = 0;
    P.railCooldown = 0;
    P.lastRail = -1;
    P.ollieCharge = 0;
}

static float hspeed() { return sqrtf(P.vel.x * P.vel.x + P.vel.z * P.vel.z); }

static const char* vehName(int type) {
    switch (type) {
        case VEH_BUS: case VEH_DOUBLE: return "BEST bus";
        case VEH_AUTO: return "auto rickshaw";
        case VEH_TAXI: return "kaali-peeli taxi";
        default: return "car";
    }
}


