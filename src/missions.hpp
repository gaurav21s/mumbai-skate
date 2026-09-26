#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Side missions, delivery jobs, friends and the skate shop
//
// Friends stand at their spot with a marker over their head. Talk to one (E)
// to hear their offer, E again to accept. Finishing their mission the first
// time adds them to your crew; after that they skate around the block and
// cheer your combos. Food stalls hand out delivery jobs that pay rupees, and
// rupees buy gear at the Skate Crew Adda under the railway bridge.
// ============================================================================

static bool tutTarget(V3& out, std::string& label);  // tutorial.hpp

enum MissionId { MIS_RAJU = 0, MIS_PRIYA, MIS_SAM, MIS_TUKARAM, MIS_CHINTU, MIS_DELIVERY = 10, MIS_JOB = 11 };
static const char* MISSION_KEYS[5] = {"raju", "priya", "sam", "tukaram", "chintu"};
static std::vector<bool> g_kiteGot;

// Job board jobs (the rest of the job code is further down).
enum JobType { JOB_PAPER, JOB_SCORE, JOB_PHOTO, JOB_TRICKS, JOB_COMBO, JOB_GRIND, JOB_TYPES };
static const char* JOB_NAMES[JOB_TYPES] = {"PAPER ROUND", "SCORE ATTACK", "SPONSOR PHOTO SHOOT", "TRICK LIST",
                                           "COMBO KING", "GRIND SESSION"};
static const char* JOB_BLURBS[JOB_TYPES] = {"drop the evening papers at 5 doors", "score big against the clock",
                                            "hit 3 spots for the Bombay Boards camera", "land 3 named tricks",
                                            "bank one long combo", "hold one long grind"};
static const int JOB_PAY[JOB_TYPES] = {450, 350, 550, 450, 350, 350};
static const int JOB_XP[JOB_TYPES] = {180, 200, 250, 250, 200, 180};
struct Job {
    int type, pay, xp;
};
static Job g_jobs[3] = {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}};
static int g_jobSlot = -1;

static void rollJob(int slot) {
    int t;
    for (int tries = 0;; tries++) {
        t = (int)(fxrand() * (float)JOB_TYPES) % JOB_TYPES;
        bool dup = false;
        for (int k = 0; k < 3; k++)
            if (k != slot && g_jobs[k].type == t) dup = true;
        if (!dup || tries > 20) break;
    }
    float k = 1.0f + 0.08f * skillBonus();
    g_jobs[slot].type = t;
    g_jobs[slot].pay = (int)((float)JOB_PAY[t] * k / 10.0f) * 10;
    g_jobs[slot].xp = JOB_XP[t];
}
static void rollAllJobs() {
    for (int i = 0; i < 3; i++) g_jobs[i].type = -1;
    for (int i = 0; i < 3; i++) rollJob(i);
}


struct Friend {
    int id;
    const char* name;
    V3 home;
    float homeYaw;
    int needLevel;              // chapters finished before their mission opens
    const char* offer[3];       // what they say when you talk to them
    const char* locked;         // what they say before their mission opens
    const char* cheers[3];
    std::vector<V3> loop;       // where they skate once they are in the crew
    bool skater;                // false: stands, drawn as a pedestrian
    int pedStyle;               // look when not a skater
    // runtime
    V3 pos;
    float yaw, speed, vy;
    bool air;
    int wp;
    float hopT, roll, pushT, cheerT, raceDelay;
    bool racing;
    int raceIdx;
};
static std::vector<Friend> g_friends;

// Race route for Raju: plaza to Dadar station.
static std::vector<V3> g_raceCps;

static void initFriends() {
    g_friends.clear();
    Friend f;

    f = Friend();
    f.id = MIS_RAJU; f.name = "RAJU"; f.home = V3(-27.0f, 0, -23.0f); f.homeYaw = -PI * 0.5f; f.needLevel = 0;
    f.offer[0] = "Oye naya launda! Race me to Dadar station.";
    f.offer[1] = "Loser buys the cutting chai. Follow the rings.";
    f.offer[2] = "E: CHAL, RACE!    Q: BAAD MEIN";
    f.locked = "";
    f.cheers[0] = "RAJU: Kya line hai, bhai!"; f.cheers[1] = "RAJU: Plaza ka raja!"; f.cheers[2] = "RAJU: Ek aur, ek aur!";
    f.loop = {V3(-15, 0, -30), V3(-40, 0, -28), V3(-62, 0, -28), V3(-66, 0, -35), V3(-40, 0, -35), V3(-15, 0, -35)};
    f.skater = true;
    g_friends.push_back(f);

    f = Friend();
    f.id = MIS_PRIYA; f.name = "PRIYA"; f.home = V3(-98.5f, 0, -29.0f); f.homeYaw = PI * 0.5f; f.needLevel = 1;
    f.offer[0] = "Heard you're the new kid. Let's see real tricks.";
    f.offer[1] = "Varial flip, a boardslide, and a 2,000 combo. 90 seconds.";
    f.offer[2] = "E: CHALLENGE ACCEPTED    Q: NOT NOW";
    f.locked = "The mali has the garden keys. Finish chapter 1 and come find me.";
    f.cheers[0] = "PRIYA: Clean! Ekdum clean!"; f.cheers[1] = "PRIYA: Okay okay, you're good."; f.cheers[2] = "PRIYA: Style hai boss!";
    f.loop = {V3(-103, 0, -50), V3(-132, 0, -50), V3(-132, 0, -80), V3(-103, 0, -80)};
    f.skater = true;
    g_friends.push_back(f);

    f = Friend();
    f.id = MIS_SAM; f.name = "SAM"; f.home = V3(46.0f, 0, -21.0f); f.homeYaw = 0; f.needLevel = 2;
    f.offer[0] = "Bro I'm filming a part. Give me one clean line.";
    f.offer[1] = "Slab drop, pipe grind, and a 2,500 combo. 75 seconds.";
    f.offer[2] = "E: ROLL CAMERA    Q: LATER";
    f.locked = "Site's locked, bro. Finish chapter 2 and the gate opens.";
    f.cheers[0] = "SAM: Got it on camera!"; f.cheers[1] = "SAM: Clip of the year!"; f.cheers[2] = "SAM: Slow-mo that one!";
    f.loop = {V3(30, 0, -45), V3(85, 0, -45), V3(85, 0, -58), V3(30, 0, -58)};
    f.skater = true;
    g_friends.push_back(f);

    f = Friend();
    f.id = MIS_TUKARAM; f.name = "TUKARAM"; f.home = V3(104.0f, 0, -25.5f); f.homeYaw = 0; f.needLevel = 3;
    f.offer[0] = "Arre, my cycle is punctured and three dabbas are late!";
    f.offer[1] = "Take them on your board. Don't drop them. 3 minutes.";
    f.offer[2] = "E: DABBA DO    Q: SORRY KAKA";
    f.locked = "Busy day, beta. Come back after chapter 3.";
    f.cheers[0] = "TUKARAM: Shabaash, beta!"; f.cheers[1] = "TUKARAM: Right on time!"; f.cheers[2] = "TUKARAM: Ekdum first class!";
    f.skater = false;
    f.pedStyle = PED_DABBAWALA;
    g_friends.push_back(f);

    f = Friend();
    f.id = MIS_CHINTU; f.name = "CHINTU"; f.home = V3(30.0f, SIDEWALK_H, 140.0f); f.homeYaw = PI; f.needLevel = 0;
    f.offer[0] = "Bhaiya! The big kids cut my kites and they flew everywhere!";
    f.offer[1] = "Bring back the loose kites before the wind takes them. Please?";
    f.offer[2] = "E: HAAN, CHAL    Q: BAAD MEIN";
    f.locked = "";
    f.cheers[0] = "CHINTU: Wah bhaiya!"; f.cheers[1] = "CHINTU: Mujhe bhi sikhao!"; f.cheers[2] = "CHINTU: Superhit!";
    f.skater = false;
    f.pedStyle = PED_KID;
    g_friends.push_back(f);

    for (size_t i = 0; i < g_friends.size(); i++) {
        Friend& q = g_friends[i];
        q.pos = q.home;
        q.yaw = q.homeYaw;
        q.hopT = 2.0f + (float)i;
        if (!q.skater) {
            // drawn and bumped like any pedestrian
            addPed(q.home, q.home, q.pedStyle, false);
            g_peds.back().pos = q.home;
            g_peds.back().yaw = q.homeYaw;
        }
    }
    g_raceCps = {V3(-14, 0, -26), V3(0, 0, -22), V3(20, 0, -14.8f), V3(45, 0, -14.6f), V3(70, 0, -14.6f),
                 V3(90, 0, -23), V3(100, 0, -25.5f)};
}

// ---------------------------------------------------------------- mission banner and dialog

struct Banner {
    std::string title, sub;
    Col col;
    float t, life;
};
static Banner g_banner = {"", "", {1, 1, 1}, 0, 0};
static void banner(const std::string& title, const std::string& sub, const Col& c, float life = 3.2f) {
    g_banner.title = title;
    g_banner.sub = sub;
    g_banner.col = c;
    g_banner.t = 0;
    g_banner.life = life;
}

// What the E key would do right now, and the dialog it opened.
enum TalkKind { TALK_NONE, TALK_FRIEND, TALK_STALL, TALK_SHOP, TALK_BOARD };
struct Talk {
    int kind;
    int who;         // friend index or stall index
    bool open;       // dialog showing
    std::string speaker;
    std::string lines[3];
    int dest;        // delivery job on offer: destination index
    V3 at;           // where the conversation happens; walking away closes it
};
static Talk g_talk = {TALK_NONE, -1, false, "", {"", "", ""}, -1, V3()};
static bool g_shopOpen = false;
static int g_shopSel = 0;

// Delivery job state (only meaningful while g_mis.id == MIS_DELIVERY)
static const char* ITEM_NAMES[7] = {"CUTTING CHAI", "VADA PAV", "BANARASI PAAN", "BHEL PURI", "MANGO BOX",
                                    "BHUTTA", "BARAF GOLA"};
static const char* VENDOR_LINES[7] = {"Chai garam hai, girana mat!", "Ek vada pav, ekdum garam. Jaldi jaa!",
                                      "Meetha paan, special order.", "Bhel with extra chutney. Chal nikal!",
                                      "Hapus aambe, handle carefully!", "Nimbu masala bhutta. Garam hai!",
                                      "Kala khatta gola. Melts fast, jaldi!"};
static int g_carryItem = -1;  // ITEM index being carried, -1 none
static int g_jobDest = -1;
static float g_jobDist = 0;
static int g_styleTips = 0;

// ---------------------------------------------------------------- starting and ending missions

static void endMission() {
    g_mis.on = false;
    g_mis.goals.clear();
    g_carryItem = -1;
    for (size_t i = 0; i < g_friends.size(); i++) {
        g_friends[i].racing = false;
        if (!g_friend[i]) {
            g_friends[i].pos = g_friends[i].home;
            g_friends[i].yaw = g_friends[i].homeYaw;
            g_friends[i].vy = 0;
            g_friends[i].air = false;
        }
    }
}

static void failMission(const std::string& why) {
    banner("MISSION FAILED", why, C(1.0f, 0.4f, 0.35f));
    endMission();
}

static std::string mmssText(float secs) {
    int s = (int)ceilf(secs);
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%02d", s / 60, s % 60);
    return buf;
}

static void beginMission(int id, const std::string& title, float limit) {
    g_mis.on = true;
    g_mis.id = id;
    g_mis.title = title;
    g_mis.goals.clear();
    g_mis.timeLimit = g_mis.timeLeft = limit;
    g_mis.freshness = 1.0f;
    g_mis.score0 = g_score;
    g_mis.job = -1;
    g_styleTips = 0;
}

static int pickDest(const V3& from, float minD, float maxD, const std::vector<int>& avoid) {
    std::vector<int> ok;
    for (size_t i = 0; i < g_dests.size(); i++) {
        const Dest& d = g_dests[i];
        if (d.needLevel > g_unlockLevel) continue;
        if (std::find(avoid.begin(), avoid.end(), (int)i) != avoid.end()) continue;
        float dist = len(V3(d.pos.x - from.x, 0, d.pos.z - from.z));
        if (dist >= minD && dist <= maxD) ok.push_back((int)i);
    }
    if (ok.empty()) return g_dests.empty() ? -1 : 0;
    return ok[(size_t)(fxrand() * (float)ok.size()) % ok.size()];
}

static void startFriendMission(int fi) {
    Friend& f = g_friends[fi];
    switch (f.id) {
        case MIS_RAJU: {
            beginMission(MIS_RAJU, "RACE RAJU TO DADAR STATION", 0);
            g_mis.goals.push_back(mkGoal(GK_CHECKPOINT, "Pass the rings before Raju", "", (int)g_raceCps.size(), 0).tip("Hold W and ride through each orange ring in order.",
                                      "The arrow always points at the next ring. The route goes east across the cross road, along the edge of the construction site to Dadar station. Cut corners, stay off the busy road, and tap V to boost on the long straight."));
            f.racing = true;
            f.raceIdx = 0;
            f.raceDelay = 3.0f;
            f.pos = f.home;
            banner("3... 2... 1...", "Follow the orange rings to the station", C(1.0f, 0.7f, 0.2f), 3.0f);
            break;
        }
        case MIS_PRIYA:
            beginMission(MIS_PRIYA, "PRIYA'S TRICK CHALLENGE", 90 * D().timeMul);
            g_mis.goals.push_back(mkGoal(GK_TRICK, "Land a varial flip (L with J or K)", "Varial|360 Flip|Laser Flip", 1, 0).tip("In one jump press L, then J (or K) straight after.",
                                      "Ollie, press L to start the shove-it and then J while it is still spinning. The combo name changes to Varial Kickflip. With K instead of J you get a Varial Heelflip. Either counts."));
            if (g_diff == DIFF_EASY)
                g_mis.goals.push_back(mkGoal(GK_TRICK, "Land any grind", "Grind|slide", 1, 0).tip("Ollie onto any rail or ledge edge and land on it.",
                                      "The yellow plaza rail, the funbox edges, benches and planters all grind. Ride parallel, ollie and steer onto the edge."));
            else if (g_diff == DIFF_MEDIUM)
                g_mis.goals.push_back(mkGoal(GK_TRICK, "Land a fancy grind (boardslide, or J / K / L on a rail)",
                                             "Boardslide|Crooked|Nosegrind|5-0", 1, 0).tip("While grinding, press J, K or L to switch to a fancier grind.",
                                      "Land on any rail, then press J (nosegrind), K (5-0) or L (crooked) while on it. Or turn a quarter turn in the air before landing on the rail for a boardslide."));
            else
                g_mis.goals.push_back(mkGoal(GK_TRICK, "Land a boardslide (quarter turn in the air onto a rail)", "Boardslide", 1, 0).tip("Approach a rail head-on, turn 90 degrees in the air, land across it.",
                                      "Ride toward the rail at a slight angle, ollie, hold A or D briefly so the board is crosswise when you land on the rail. Too much turn gives a normal grind."));
            g_mis.goals.push_back(mkGoal(GK_COMBO, "Bank a {p} point combo", "", 1, 2000).tip("Link tricks before the blue bar runs out.",
                                      "Flip, land, ollie into another flip; add a grind or a manual in between. Each trick multiplies the total."));
            banner("TRICK CHALLENGE", mmssText(g_mis.timeLimit) + " on the clock. Priya is watching", C(1.0f, 0.5f, 0.7f));
            break;
        case MIS_SAM:
            beginMission(MIS_SAM, "FILM A LINE WITH SAM", 75 * D().timeMul);
            g_mis.goals.push_back(mkGoal(GK_GAP, "Drop off the slab edge", "Slab Drop", 1, 0).where(66.0f, 3.2f, -24.5f).tip("Up the plank ramp, then roll off the slab edge.",
                                      "The plank ramp inside the site takes you up to the first slab. Ride off any open edge and land on the ground."));
            g_mis.goals.push_back(mkGoal(GK_GRIND, "Grind the concrete pipe", "", 1, 0.3f).rail(RT_PIPE)
                                      .where(59.0f, 1.3f, -19.3f).tip("Ollie onto the big pipe near the site gate.",
                                      "Ride alongside the pipe and hop on. Keep balance with A and D."));
            g_mis.goals.push_back(mkGoal(GK_COMBO, "Bank a {p} point combo", "", 1, 2500).tip("Chain the slab drop, the pipe and flips in one combo.",
                                      "Try: flip off the slab edge, land and ollie straight onto the pipe, grind it, flip off the end."));
            banner("ROLLING!", mmssText(g_mis.timeLimit) + " of tape. Make it count", C(0.5f, 0.85f, 1.0f));
            break;
        case MIS_TUKARAM: {
            beginMission(MIS_TUKARAM, "TUKARAM'S DABBA RUN", 180 * D().timeMul);
            std::vector<int> used;
            V3 from = f.home;
            for (int k = 0; k < 3; k++) {
                int d = pickDest(from, 60.0f, 220.0f, used);
                if (d < 0) break;
                used.push_back(d);
                const Dest& dd = g_dests[(size_t)d];
                g_mis.goals.push_back(mkGoal(GK_DELIVER, "Dabba to " + dd.name, "", 1, 0)
                                          .where(dd.pos.x, dd.pos.y, dd.pos.z).tip("Follow the blue beam to the address. Don't bail.",
                                      "Each tiffin goes to the address under the blue beam, one after another. A bail knocks one off, so skip risky tricks."));
                from = dd.pos;
            }
            g_carryItem = 4;
            banner("DABBA RUN", "3 tiffins. A bail knocks one off the board", C(1.0f, 0.85f, 0.4f));
            break;
        }
        case MIS_CHINTU: {
            beginMission(MIS_CHINTU, "CHINTU'S LOOSE KITES", 150 * D().timeMul);
            int n = g_diff == DIFF_EASY ? 3 : (int)g_kitePickups.size();
            g_mis.goals.push_back(mkGoal(GK_COLLECT, "Pick up the loose kites", "", n, 0)
                                      .tip("Follow the arrow to each kite. Some need a jump to reach.",
                                           "Kites are caught on the viewing deck, above the kicker, on the manual pad, on top of the seawall and in the lane of shops. Ride through them; the high ones need a charged ollie or the kicker."));
            g_kiteGot.assign(g_kitePickups.size(), false);
            if (n < (int)g_kitePickups.size())
                for (size_t k = (size_t)n; k < g_kitePickups.size(); k++) g_kiteGot[k] = true;  // fewer on easy
            banner("KITE HUNT", "Follow the arrow to each loose kite", C(1.0f, 0.6f, 0.8f));
            break;
        }
        default: break;
    }
    for (size_t i = 0; i < g_mis.goals.size(); i++) scaleGoal(g_mis.goals[i]);
}

static void startDeliveryJob(int stallIdx, int dest) {
    const Stall& s = g_stalls[(size_t)stallIdx];
    const Dest& d = g_dests[(size_t)dest];
    g_jobDist = len(V3(d.pos.x - s.stand.x, 0, d.pos.z - s.stand.z));
    beginMission(MIS_DELIVERY, std::string("DELIVER ") + ITEM_NAMES[s.kind], (15.0f + g_jobDist / 5.0f) * D().timeMul);
    g_mis.goals.push_back(mkGoal(GK_DELIVER, std::string("Take it to ") + d.name, "", 1, 0)
                              .where(d.pos.x, d.pos.y, d.pos.z).tip("Ride to the blue beam before the timer runs out.",
                                      "The arrow points at the address. Stay on sidewalks, boost with V on straights. Bails damage the order; tricks on the way earn a tip."));
    g_carryItem = s.kind;
    g_jobDest = dest;
    banner(std::string(ITEM_NAMES[s.kind]) + " FOR " + d.name, s.kind == STALL_CHAI ? "Chai spills if you bail"
                                                                                  : "Tricks on the way earn a style tip",
           C(1.0f, 0.85f, 0.4f));
}

static void completeMission() {
    int id = g_mis.id;
    if (id == MIS_DELIVERY) {
        int base = 40 + (int)(g_jobDist * 0.6f);
        int speed = (int)(g_mis.timeLeft * 2.0f);
        int pay = (int)((float)(base + speed + g_styleTips) * (0.5f + 0.5f * g_mis.freshness));
        g_deliveries++;
        banner("DELIVERED!", "Paid RS " + std::to_string(pay) + "  (style tip RS " + std::to_string(g_styleTips) + ")",
               C(0.5f, 1.0f, 0.6f));
        g_rupees += pay;
        g_score += 250;
        endMission();
        evDelivery();
        addXP(80, "delivery");
    } else if (id == MIS_JOB) {
        const Job& j = g_jobs[std::max(0, g_jobSlot)];
        banner("JOB DONE: " + std::string(JOB_NAMES[j.type]), "Paid RS " + std::to_string(j.pay), C(0.5f, 1.0f, 0.6f));
        g_rupees += j.pay;
        g_score += 500;
        long xp = j.xp;
        endMission();
        addXP(xp, "job");
        if (g_jobSlot >= 0) rollJob(g_jobSlot);
    } else if (id == MIS_CHINTU) {
        bool first = !g_friend[id];
        int pay = first ? 300 : 120;
        banner("KITES RETURNED!", "Chintu gave you RS " + std::to_string(pay) + " from his gullak", C(0.5f, 1.0f, 0.6f));
        g_rupees += pay;
        g_score += first ? 1000 : 400;
        g_friend[id] = true;
        endMission();
        addXP(first ? 150 : 60, "kites");
    } else if (id >= 0 && id < 4) {
        bool first = !g_friend[id];
        int pay = first ? 250 + 100 * id : 120;
        banner("MISSION COMPLETE", "+RS " + std::to_string(pay), C(0.5f, 1.0f, 0.6f));
        g_rupees += pay;
        g_score += first ? 1500 : 500;
        if (first) {
            g_friend[id] = true;
            popup(std::string(FRIEND_NAMES[id]) + " JOINED YOUR CREW!", C(1.0f, 0.8f, 0.3f), 32, 3.0f);
            confetti(P.pos);
        }
        endMission();
        evMissionDone(MISSION_KEYS[id]);
        addXP(first ? 300 : 120, "mission");
    }
    saveGame();
}

// Bails spoil deliveries. Called from bail().
static void missionOnBail() {
    if (!g_mis.on || (g_mis.id != MIS_DELIVERY && g_mis.id != MIS_TUKARAM)) return;
    bool chai = g_mis.id == MIS_DELIVERY && g_carryItem == STALL_CHAI;
    g_mis.freshness -= chai ? D().chaiSpoil : D().spoil;
    if (g_mis.freshness <= 0.01f) {
        failMission(g_mis.id == MIS_TUKARAM ? "Dabbe gir gaye! All the tiffins hit the road"
                                            : (chai ? "The chai spilled everywhere" : "The order hit the road. Kaka is not happy"));
    } else {
        popup(g_mis.id == MIS_TUKARAM ? "A DABBA FELL OFF!" : "ORDER DAMAGED!", C(1.0f, 0.55f, 0.3f), 26, 2.0f);
    }
}

// Tricks landed while carrying an order add to the tip. Called from land().
static void missionOnLand(int tricks) {
    if (g_mis.on && g_mis.id == MIS_DELIVERY && tricks > 0) g_styleTips = std::min(120, g_styleTips + 8 * tricks);
}

// ---------------------------------------------------------------- job board
//
// Better-paying odd jobs that also build skill. Three are posted at a time;
// taking one posts a fresh one in its place.


struct SpotDef { const char* gap; float x, y, z; int need; };
static const SpotDef PHOTO_SPOTS[] = {
    {"Kicker Gap", -48.5f, 1.2f, -28.0f, 0},     {"Funbox Launch", -78.0f, 1.0f, -29.0f, 0},
    {"Sea Face Steps", 55.0f, 1.0f, 144.0f, 0},  {"Median Hop", 60.0f, 0.5f, 0.0f, 0},
    {"Main Road Gap", 0.0f, 1.6f, -11.0f, 0},    {"Garden Stair Gap", -106.0f, 1.0f, -30.0f, 1},
    {"Slab Drop", 66.0f, 3.2f, -24.5f, 2},       {"Skywalk Drop", -80.0f, 3.4f, -70.0f, 3},
};
struct TrickDef { const char* text; const char* key; int need; const char* how; };
static const TrickDef JOB_TRICK_LIST[] = {
    {"Land a Double Kickflip", "Double Kickflip", 0, "Ollie high, press J twice quickly."},
    {"Land a Varial Kickflip", "Varial Kickflip", UL_HEEL, "Ollie, press L then J in the same jump."},
    {"Land a 360 Flip", "360 Flip", UL_HEEL, "Big ollie, press L, L, then J before landing."},
    {"Land a Heelflip", "Heelflip", UL_HEEL, "Ollie, then K in the air."},
    {"Land a Crooked Grind", "Crooked", UL_HEEL, "Lock onto a rail, then press L while grinding."},
    {"Land a Nosegrind", "Nosegrind", UL_HEEL, "Lock onto a rail, then press J while grinding."},
    {"Land an Indy Grab", "Indy", UL_GRAB, "Ollie, hold I, let go before landing."},
    {"Land a Melon Grab", "Melon", UL_GRAB, "Ollie, hold U, let go before landing."},
    {"Land a Nose Manual", "Nose Manual", UL_MANUAL, "While rolling press M, balance with W and S."},
};

static void startJob(int slot) {
    const Job& j = g_jobs[slot];
    g_jobSlot = slot;
    float tm = D().timeMul;
    switch (j.type) {
        case JOB_PAPER: {
            beginMission(MIS_JOB, "PAPER ROUND", 170 * tm);
            std::vector<int> used;
            V3 from = g_boardPos;
            for (int k = 0; k < 5; k++) {
                int d = pickDest(from, 25.0f, 110.0f, used);
                if (d < 0) break;
                used.push_back(d);
                const Dest& dd = g_dests[(size_t)d];
                g_mis.goals.push_back(mkGoal(GK_DELIVER, "Paper to " + dd.name, "", 1, 0).where(dd.pos.x, dd.pos.y, dd.pos.z)
                                          .tip("Ride through the blue beam; no need to stop.",
                                               "Each paper goes to the address under the blue beam, one after another. Passing close is enough, so keep rolling and boost (V) on long stretches."));
                from = dd.pos;
            }
            g_carryItem = 7;
            break;
        }
        case JOB_SCORE: {
            beginMission(MIS_JOB, "SCORE ATTACK", 60 * tm);
            float target = 3000.0f * (1.0f + 0.1f * skillBonus());
            g_mis.goals.push_back(mkGoal(GK_SCORE, "Score {p} points before the clock runs out", "", 1, target)
                                      .tip("Chain long combos; only banked points count.",
                                           "Grinds and manuals between flips keep one combo going, and the multiplier grows with every trick. The plaza has the kicker, rails and funbox close together."));
            break;
        }
        case JOB_PHOTO: {
            beginMission(MIS_JOB, "SPONSOR PHOTO SHOOT", 150 * tm);
            std::vector<int> pick;
            int n = (int)(sizeof(PHOTO_SPOTS) / sizeof(PHOTO_SPOTS[0]));
            for (int tries = 0; tries < 60 && pick.size() < 3; tries++) {
                int k = (int)(fxrand() * (float)n) % n;
                if (PHOTO_SPOTS[k].need > g_unlockLevel) continue;
                if (std::find(pick.begin(), pick.end(), k) != pick.end()) continue;
                pick.push_back(k);
            }
            for (size_t i = 0; i < pick.size(); i++) {
                const SpotDef& sp = PHOTO_SPOTS[pick[i]];
                g_mis.goals.push_back(mkGoal(GK_GAP, std::string("Photo: ") + sp.gap, sp.gap, 1, 0).where(sp.x, sp.y, sp.z)
                                          .tip("Clear this gap under the beam while the camera rolls.",
                                               "Follow the arrow to the spot. Most gaps need speed and a charged ollie (hold SPACE, let go at the edge). Tab switches between the three spots."));
            }
            break;
        }
        case JOB_TRICKS: {
            beginMission(MIS_JOB, "TRICK LIST", 120 * tm);
            std::vector<int> pick;
            int n = (int)(sizeof(JOB_TRICK_LIST) / sizeof(JOB_TRICK_LIST[0]));
            for (int tries = 0; tries < 80 && pick.size() < 3; tries++) {
                int k = (int)(fxrand() * (float)n) % n;
                if (!trickOpen(JOB_TRICK_LIST[k].need)) continue;
                if (std::find(pick.begin(), pick.end(), k) != pick.end()) continue;
                pick.push_back(k);
            }
            if (pick.size() < 3) pick = {0, 0, 0};
            for (size_t i = 0; i < pick.size(); i++) {
                const TrickDef& t = JOB_TRICK_LIST[pick[i]];
                if (i > 0 && pick[i] == pick[0]) break;
                g_mis.goals.push_back(mkGoal(GK_TRICK, t.text, t.key, 1, 0).tip(t.how, std::string(t.how) +
                                          " Land it anywhere; it counts even inside a combo."));
            }
            break;
        }
        case JOB_COMBO:
            beginMission(MIS_JOB, "COMBO KING", 90 * tm);
            g_mis.goals.push_back(mkGoal(GK_COMBO_LEN, "Bank a {c} trick combo", "", 1, 5)
                                      .tip("Land, manual or powerslide, and trick again.",
                                           "Linking needs something between jumps: a manual (N), a powerslide (S at speed) or an ollie before the blue bar empties."));
            break;
        default:
            beginMission(MIS_JOB, "GRIND SESSION", 90 * tm);
            g_mis.goals.push_back(mkGoal(GK_GRIND, "Hold one grind for {s} sec", "", 1, 3.0f)
                                      .tip("Find a long rail: the seawall or the skywalk railing.",
                                           "Short rails end too soon. The Sea Face seawall and the skywalk railing are long enough. Keep the balance needle centred with A and D."));
            break;
    }
    g_mis.job = j.type;
    for (size_t i = 0; i < g_mis.goals.size(); i++) scaleGoal(g_mis.goals[i]);
    banner(JOB_NAMES[j.type], std::string("Pays RS ") + std::to_string(j.pay) + " and " + std::to_string(j.xp) + " XP",
           C(1.0f, 0.8f, 0.35f));
}

// ---------------------------------------------------------------- talking

static int nearFriend() {
    for (size_t i = 0; i < g_friends.size(); i++) {
        const Friend& f = g_friends[i];
        if (f.racing) continue;
        V3 d = f.pos - P.pos;
        if (d.x * d.x + d.z * d.z < 3.0f * 3.0f && fabsf(d.y) < 1.2f) return (int)i;
    }
    return -1;
}
static int nearStall() {
    for (size_t i = 0; i < g_stalls.size(); i++) {
        V3 d = g_stalls[i].stand - P.pos;
        if (d.x * d.x + d.z * d.z < 2.6f * 2.6f && fabsf(d.y) < 1.0f) return (int)i;
    }
    return -1;
}
static bool nearBoard() {
    V3 d = g_boardPos - P.pos;
    return d.x * d.x + d.z * d.z < 3.0f * 3.0f && fabsf(d.y) < 1.0f;
}
static bool nearShop() {
    V3 d = g_shopPos - P.pos;
    return d.x * d.x + d.z * d.z < 3.2f * 3.2f && fabsf(d.y) < 1.0f;
}

// Works out what E would do from here, for the on-screen prompt.
static void updateTalkTarget() {
    if (g_talk.open) {
        V3 d = g_talk.at - P.pos;
        if (d.x * d.x + d.z * d.z > 6.0f * 6.0f) g_talk.open = false;
        return;
    }
    g_talk.kind = TALK_NONE;
    if (P.state != P_GROUND || hspeed() > 4.0f) return;
    int f = nearFriend();
    if (f >= 0) { g_talk.kind = TALK_FRIEND; g_talk.who = f; return; }
    if (nearShop()) { g_talk.kind = TALK_SHOP; return; }
    if (nearBoard()) { g_talk.kind = TALK_BOARD; return; }
    int s = nearStall();
    if (s >= 0) { g_talk.kind = TALK_STALL; g_talk.who = s; }
}

static void pressTalk() {
    if (g_talk.open) {
        // second press accepts the offer
        g_talk.open = false;
        if (g_mis.on) return;
        if (g_talk.kind == TALK_FRIEND) {
            const Friend& f = g_friends[(size_t)g_talk.who];
            if (g_unlockLevel >= f.needLevel) startFriendMission(g_talk.who);
        } else if (g_talk.kind == TALK_STALL && g_talk.dest >= 0) {
            startDeliveryJob(g_talk.who, g_talk.dest);
        }
        return;
    }
    if (g_talk.kind == TALK_NONE) return;
    if (g_talk.kind == TALK_SHOP) {
        g_shopOpen = true;
        return;
    }
    if (g_talk.kind == TALK_BOARD && !g_mis.on) {
        g_talk.at = P.pos;
        g_talk.speaker = "JOB BOARD  (1-3 TAKE A JOB, Q CLOSE)";
        for (int i = 0; i < 3; i++) {
            const Job& j = g_jobs[i];
            g_talk.lines[i] = std::to_string(i + 1) + ":  " + JOB_NAMES[j.type] + "  (" + JOB_BLURBS[j.type] + ")    RS " +
                              std::to_string(j.pay) + "  +" + std::to_string(j.xp) + " XP";
        }
        g_talk.open = true;
        return;
    }
    g_talk.at = P.pos;
    g_talk.dest = -1;
    if (g_mis.on) {
        g_talk.speaker = "";
        g_talk.lines[0] = "You are already on a job: " + g_mis.title;
        g_talk.lines[1] = "Finish it first, or press Q to give up.";
        g_talk.lines[2] = "";
        g_talk.open = true;
        return;
    }
    if (g_talk.kind == TALK_FRIEND) {
        const Friend& f = g_friends[(size_t)g_talk.who];
        g_talk.speaker = f.name;
        if (g_unlockLevel < f.needLevel) {
            g_talk.lines[0] = f.locked;
            g_talk.lines[1] = "";
            g_talk.lines[2] = "";
        } else {
            for (int i = 0; i < 3; i++) g_talk.lines[i] = f.offer[i];
            if (g_friend[f.id]) g_talk.lines[1] = std::string("Again? Same deal, smaller prize. ") + f.offer[1];
        }
        g_talk.open = true;
    } else if (g_talk.kind == TALK_STALL) {
        const Stall& s = g_stalls[(size_t)g_talk.who];
        int d = pickDest(s.stand, 45.0f, 170.0f, std::vector<int>());
        g_talk.speaker = s.kind == STALL_CHAI ? "CHAIWALA" : (s.kind == STALL_PAAN ? "PAANWALA" : "KAKA");
        g_talk.lines[0] = VENDOR_LINES[s.kind];
        g_talk.lines[1] = d >= 0 ? std::string("Deliver to ") + g_dests[(size_t)d].name + "." : "No orders right now.";
        const Food& fd = FOODS[s.kind];
        g_talk.lines[2] = std::string(d >= 0 ? "E: DELIVER IT    " : "") + "F: " + fd.name + " RS " + std::to_string(fd.price) +
                          " (+" + std::to_string((int)fd.energy) + " ENERGY)    Q: NO THANKS";
        g_talk.dest = d;
        g_talk.open = true;
    }
}

// 1, 2 or 3 while the job board is open.
static void pressJob(int k) {
    if (!g_talk.open || g_talk.kind != TALK_BOARD || g_mis.on || k < 0 || k > 2) return;
    g_talk.open = false;
    startJob(k);
}

static void pressCancel() {
    if (g_talk.open) {
        g_talk.open = false;
        return;
    }
    if (g_mis.on) failMission("You gave up on " + g_mis.title);
}

// ---------------------------------------------------------------- task focus and hints
//
// One open task is "focused": the arrow points to it and the task panel shows
// how to do it. Tab moves the focus. G opens the full walkthrough, which costs
// a few rupees (free on easy, and the first one each chapter is free).

static int g_focus = -1;
static bool g_hintOpen = false;
static float g_hintT = 0;
static int g_freeHintLevel = -1;  // chapter whose free hint has been used

static std::vector<Goal>* focusList() {
    if (g_mis.on) return &g_mis.goals;
    Chapter* c = curChapter();
    return c ? &c->goals : nullptr;
}
static Goal* focusGoal() {
    std::vector<Goal>* l = focusList();
    if (!l || l->empty()) return nullptr;
    if (g_focus < 0 || g_focus >= (int)l->size() || goalDone((*l)[(size_t)g_focus])) {
        g_focus = -1;
        for (size_t i = 0; i < l->size(); i++)
            if (!goalDone((*l)[i])) {
                g_focus = (int)i;
                break;
            }
    }
    return g_focus >= 0 ? &(*l)[(size_t)g_focus] : nullptr;
}
static void cycleFocus() {
    std::vector<Goal>* l = focusList();
    if (!l || l->empty()) return;
    focusGoal();
    int n = (int)l->size();
    for (int k = 1; k <= n; k++) {
        int i = (g_focus + k + n) % n;
        if (!goalDone((*l)[(size_t)i])) {
            g_focus = i;
            break;
        }
    }
    g_hintOpen = false;
}
static int hintCost() { return g_diff == DIFF_EASY ? 0 : (g_diff == DIFF_MEDIUM ? 20 : 30); }

static void pressHint() {
    if (g_hintOpen) {
        g_hintOpen = false;
        return;
    }
    Goal* g = focusGoal();
    if (!g || g->hint.empty()) {
        popup("NO TASK TO GIVE A HINT FOR", C(0.8f, 0.8f, 0.85f), 22, 1.4f);
        return;
    }
    if (!g->hintPaid) {
        int cost = hintCost();
        if (cost == 0) {
            g->hintPaid = true;
        } else if (g_freeHintLevel != g_unlockLevel) {
            g_freeHintLevel = g_unlockLevel;
            g->hintPaid = true;
            popup("FIRST HINT THIS CHAPTER IS FREE", C(0.5f, 1.0f, 0.6f), 22, 1.8f);
        } else if (g_rupees >= cost) {
            g_rupees -= cost;
            g->hintPaid = true;
            popup("HINT  -RS " + std::to_string(cost), C(1.0f, 0.8f, 0.45f), 22, 1.6f);
        } else {
            popup("A HINT COSTS RS " + std::to_string(cost) + ". DELIVERIES PAY!", C(1.0f, 0.5f, 0.4f), 22, 2.0f);
            return;
        }
    }
    g_hintOpen = true;
    g_hintT = 0;
}

static void pressFood() {
    if (g_talk.kind == TALK_STALL && g_talk.who >= 0) buyFood(g_talk.who);
    else popup("STOP NEXT TO A FOOD STALL TO EAT OR DRINK", C(0.8f, 0.8f, 0.85f), 20, 1.4f);
}

// ---------------------------------------------------------------- buying your way past a task

static int skipCost() { return g_diff == DIFF_EASY ? 1000 : (g_diff == DIFF_MEDIUM ? 1500 : 2000); }
static bool g_skipArmed = false;
static float g_skipArmT = 0;

static void pressSkip() {
    Goal* g = focusGoal();
    if (!g) {
        popup("NO TASK TO SKIP", C(0.8f, 0.8f, 0.85f), 22, 1.4f);
        return;
    }
    int cost = skipCost();
    if (g_rupees < cost) {
        popup("SKIPPING A TASK COSTS RS " + withCommas(cost) + ". YOU HAVE RS " + withCommas(g_rupees),
              C(1.0f, 0.5f, 0.4f), 22, 2.2f);
        return;
    }
    if (!g_skipArmed) {
        g_skipArmed = true;
        g_skipArmT = 3.0f;
        popup("PRESS X AGAIN TO PAY RS " + withCommas(cost) + " AND SKIP THIS TASK", C(1.0f, 0.8f, 0.4f), 22, 2.8f);
        return;
    }
    g_skipArmed = false;
    g_rupees -= cost;
    bool mission = g_mis.on;
    popup("TASK SKIPPED  -RS " + withCommas(cost), C(1.0f, 0.8f, 0.45f), 24, 2.0f);
    bump(*g, g->need - g->have, mission);
    g_hintOpen = false;
}

// ---------------------------------------------------------------- shop

static void shopMove(int d) { g_shopSel = (g_shopSel + d + GEAR_COUNT) % GEAR_COUNT; }

static void shopBuy() {
    int i = g_shopSel;
    const GearDef& g = GEAR[i];
    if (g_unlockLevel < g.needLevel) {
        popup("FINISH CHAPTER " + std::to_string(g.needLevel) + " TO UNLOCK THIS", C(0.8f, 0.8f, 0.85f), 22, 1.8f);
        return;
    }
    if (!g_owned[i]) {
        if (g_rupees < g.price) {
            popup("NOT ENOUGH RUPEES. DO SOME DELIVERIES!", C(1.0f, 0.5f, 0.4f), 22, 1.8f);
            return;
        }
        g_rupees -= g.price;
        g_owned[i] = true;
        popup(std::string("BOUGHT ") + g.name, C(0.5f, 1.0f, 0.6f), 26, 2.0f);
    }
    int* slot = gearSlot(g.kind);
    if (slot) *slot = (*slot == i) ? -1 : i;  // wear it, or take it off again
    saveGame();
}

// ---------------------------------------------------------------- per-step update

static float g_friendCheerCd = 0;

// Kinematic skating along a path: steer toward the target, ride the ground,
// fly off drops, ollie now and then.
static void friendSkate(Friend& f, const V3& target, float speed, float dt) {
    V3 d = target - f.pos;
    d.y = 0;
    float want = dirYaw(d.x, d.z);
    f.yaw = wrapAngle(f.yaw + clampf(wrapAngle(want - f.yaw), -3.0f * dt, 3.0f * dt));
    f.speed = approach(f.speed, speed, 4.0f * dt);
    V3 fwd = yawDir(f.yaw);
    f.pos.x += fwd.x * f.speed * dt;
    f.pos.z += fwd.z * f.speed * dt;
    f.pushT += dt;
    float gy = groundAt(f.pos.x, f.pos.z, f.pos.y + 0.35f).h;
    if (f.air) {
        f.vy -= GRAVITY * dt;
        f.pos.y += f.vy * dt;
        f.roll = approach(f.roll, 0, 900.0f * dt);
        if (f.pos.y <= gy) {
            f.pos.y = gy;
            f.air = false;
            f.vy = 0;
            f.roll = 0;
        }
    } else if (gy < f.pos.y - 0.2f) {
        f.air = true;
        f.vy = 1.0f;
    } else {
        f.pos.y = gy;
        f.hopT -= dt;
        if (f.hopT <= 0) {
            f.hopT = fxrange(2.5f, 6.0f);
            f.air = true;
            f.vy = 5.5f;
            f.roll = fxrand() < 0.5f ? 360.0f : 0.0f;
        }
    }
}

static void updateFriends(float dt) {
    if (g_friendCheerCd > 0) g_friendCheerCd -= dt;
    for (size_t i = 0; i < g_friends.size(); i++) {
        Friend& f = g_friends[i];
        if (!f.skater) continue;
        if (f.racing) {
            if (f.raceDelay > 0) {
                f.raceDelay -= dt;
                continue;
            }
            if (f.raceIdx >= (int)g_raceCps.size()) continue;
            V3 cp = g_raceCps[(size_t)f.raceIdx];
            // on easy and medium Raju eases off when he gets too far ahead
            float spd = D().rajuSpeed;
            if (g_diff != DIFF_HARD && len(f.pos - P.pos) > 14.0f && f.raceIdx > 0) spd *= 0.6f;
            friendSkate(f, cp, spd, dt);
            if (len(V3(cp.x - f.pos.x, 0, cp.z - f.pos.z)) < 2.0f) {
                f.raceIdx++;
                if (f.raceIdx >= (int)g_raceCps.size()) failMission("Raju got there first. Chai is on you!");
            }
            continue;
        }
        if (g_friend[f.id] && !f.loop.empty() && !(g_talk.open && g_talk.kind == TALK_FRIEND && g_talk.who == (int)i)) {
            V3 t = f.loop[(size_t)f.wp % f.loop.size()];
            friendSkate(f, t, 6.5f, dt);
            if (len(V3(t.x - f.pos.x, 0, t.z - f.pos.z)) < 2.0f) f.wp = (f.wp + 1) % (int)f.loop.size();
        } else {
            f.speed = 0;
            f.pos.y = groundAt(f.pos.x, f.pos.z, f.pos.y + 0.35f).h;
            // face the player when they come close
            V3 d = P.pos - f.pos;
            if (d.x * d.x + d.z * d.z < 64.0f) f.yaw = wrapAngle(f.yaw + wrapAngle(dirYaw(d.x, d.z) - f.yaw) * std::min(1.0f, dt * 4.0f));
        }
    }
}

// Crew members nearby shout when a combo lands.
static void friendsCheer(long total) {
    if (total < 1500 || g_friendCheerCd > 0) return;
    for (size_t i = 0; i < g_friends.size(); i++) {
        const Friend& f = g_friends[i];
        if (!g_friend[f.id]) continue;
        if (len(f.pos - P.pos) > 35.0f) continue;
        popup(f.cheers[(int)(fxrand() * 2.99f)], C(0.9f, 0.9f, 1.0f), 22, 2.0f);
        g_friendCheerCd = 6.0f;
        return;
    }
}

static void missionsUpdate(float dt) {
    if (g_banner.life > 0) {
        g_banner.t += dt;
        if (g_banner.t > g_banner.life) g_banner.life = 0;
    }
    updateFriends(dt);
    if (g_skipArmT > 0) {
        g_skipArmT -= dt;
        if (g_skipArmT <= 0) g_skipArmed = false;
    }
    if (!g_careerOn) return;
    updateTalkTarget();
    if (!g_mis.on) return;
    if (g_mis.timeLimit > 0) {
        g_mis.timeLeft -= dt;
        if (g_mis.timeLeft <= 0) {
            failMission(g_mis.id == MIS_DELIVERY ? "Too late. The customer cancelled" : "Out of time");
            return;
        }
    }
    // drops and race rings: only the first open one counts, so they go in order
    for (size_t i = 0; i < g_mis.goals.size(); i++) {
        Goal& g = g_mis.goals[i];
        if (goalDone(g)) continue;
        if (g.kind == GK_DELIVER) {
            V3 d = g.at - P.pos;
            if (d.x * d.x + d.z * d.z < 3.2f * 3.2f && fabsf(d.y) < 1.8f && P.state != P_BAIL) {
                bump(g, 1, true);
                if (g_mis.id == MIS_TUKARAM) popup("DABBA DELIVERED!", C(1.0f, 0.85f, 0.4f), 26, 1.8f);
            }
            break;
        }
        if (g.kind == GK_SCORE && (float)(g_score - g_mis.score0) >= g.param) bump(g, 1, true);
        if (g.kind == GK_COLLECT) {
            for (size_t k = 0; k < g_kitePickups.size(); k++) {
                if (g_kiteGot[k]) continue;
                if (len(g_kitePickups[k] - (P.pos + V3(0, 0.9f, 0))) < 1.8f) {
                    g_kiteGot[k] = true;
                    bump(g, 1, true);
                    for (int s = 0; s < 30; s++) sparkle(g_kitePickups[k] - V3(0, 0.8f, 0), C(1.0f, 0.6f, 0.8f));
                }
            }
            break;
        }
        if (g.kind == GK_CHECKPOINT) {
            V3 cp = g_raceCps[(size_t)g.have];
            V3 d = cp - P.pos;
            if (d.x * d.x + d.z * d.z < 4.5f * 4.5f) {
                bump(g, 1, true);
                if (!goalDone(g)) popup("RING " + std::to_string(g.have) + "/" + std::to_string(g.need), C(1.0f, 0.7f, 0.3f), 22, 1.0f);
            }
            break;
        }
    }
    if (g_checkMission) {
        g_checkMission = false;
        bool all = true;
        for (size_t i = 0; i < g_mis.goals.size(); i++)
            if (!goalDone(g_mis.goals[i])) all = false;
        if (all) completeMission();
    }
}

// Where the HUD arrow should point: the next mission step, else the nearest open chapter task.
static bool objectiveTarget(V3& out, std::string& label) {
    if (tutTarget(out, label)) return true;
    if (g_mis.on) {
        Goal* fg = focusGoal();
        for (size_t i = 0; i < g_mis.goals.size(); i++) {
            const Goal& g = g_mis.goals[i];
            if (goalDone(g)) continue;
            if (fg && fg->hasAt && fg->kind != GK_DELIVER && &g != fg) continue;  // the focused task leads
            if (g.kind == GK_CHECKPOINT) { out = g_raceCps[(size_t)g.have]; label = "NEXT RING"; return true; }
            if (g.kind == GK_COLLECT) {
                float best = 1e9f;
                for (size_t k = 0; k < g_kitePickups.size(); k++)
                    if (!g_kiteGot[k] && len(g_kitePickups[k] - P.pos) < best) {
                        best = len(g_kitePickups[k] - P.pos);
                        out = g_kitePickups[k];
                    }
                label = "LOOSE KITE";
                return best < 1e8f;
            }
            if (g.hasAt) { out = g.at; label = g.text; return true; }
        }
        return false;
    }
    Chapter* c = curChapter();
    if (!c) return false;
    float best = 1e9f;
    bool found = false;
    auto consider = [&](const V3& p, const std::string& l) {
        float d = len(p - P.pos);
        if (d < best) { best = d; out = p; label = l; found = true; }
    };
    Goal* fg = focusGoal();
    for (size_t i = 0; i < c->goals.size(); i++) {
        const Goal& g = c->goals[i];
        if (goalDone(g)) continue;
        if (fg && &g != fg) continue;  // only the focused task (Tab picks another)
        if (g.hasAt) consider(g.at, g.text);
        if (g.kind == GK_LETTERS)
            for (size_t k = 0; k < g_letters.size(); k++)
                if (!g_letters[k].got) consider(g_letters[k].pos, std::string("LETTER ") + g_letters[k].ch);
        if (g.kind == GK_CHAI || g.kind == GK_DELIVERIES)
            for (size_t k = 0; k < g_stalls.size(); k++) {
                bool chai = g_stalls[k].kind == STALL_CHAI;
                if ((g.kind == GK_CHAI) == chai || g.kind == GK_DELIVERIES)
                    consider(g_stalls[k].stand, g.kind == GK_CHAI ? "CHAI STALL" : "FOOD STALL (DELIVERY JOB)");
            }
        if (g.kind == GK_MISSION)
            for (size_t k = 0; k < g_friends.size(); k++)
                if (g.key == MISSION_KEYS[g_friends[k].id]) consider(g_friends[k].pos, std::string("TALK TO ") + g_friends[k].name);
    }
    return found;
}
