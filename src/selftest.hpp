#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ---------------------------------------------------------------- self test
// Scripted runs through the real physics, printed so behaviour can be checked
// without a human at the keyboard.

static Input noInput() {
    Input in;
    memset(&in, 0, sizeof(in));
    return in;
}

static void runFor(float secs, Input in, bool once = true) {
    int n = (int)(secs / SIM_DT);
    for (int i = 0; i < n; i++) {
        Input s = in;
        if (once && i > 0) {
            s.ollie = s.kick = s.heel = s.shove = s.indy = s.melon = s.manual = s.noseManual = false;
        }
        simStep(s);
    }
}

static int selfTest() {
    int fails = 0;
    g_diff = DIFF_HARD;  // the physics checks below use the original tuning
    applyDifficulty();
    auto check = [&](bool ok, const char* what, float val) {
        printf("[%s] %-46s %.2f\n", ok ? " ok " : "FAIL", what, val);
        if (!ok) fails++;
    };
    printf("solids %d, rails %d, peds %d\n", (int)g_solids.size(), (int)g_rails.size(), (int)g_peds.size());

    // pushing on flat ground
    resetPlayer(SPAWN_POS, SPAWN_YAW);
    Input push = noInput();
    push.up = true;
    runFor(2.0f, push, false);
    check(hspeed() > 8.0f && P.state == P_GROUND, "push 2s reaches speed (m/s)", hspeed());

    // flat ollie height and landing
    resetPlayer(V3(-24, 0, -34), SPAWN_YAW);
    P.vel = yawDir(P.yaw) * 6.0f;
    Input ol = noInput();
    ol.ollie = true;
    float maxY = 0;
    int landedAt = -1;
    simStep(ol);
    for (int i = 0; i < 240; i++) {
        simStep(noInput());
        maxY = std::max(maxY, P.pos.y);
        if (P.state == P_GROUND && landedAt < 0) landedAt = i;
    }
    check(maxY > 0.9f && maxY < 1.4f, "ollie apex (m)", maxY);
    check(landedAt > 0 && P.state == P_GROUND, "ollie lands clean (steps)", (float)landedAt);

    // kickflip lands and scores
    resetPlayer(V3(-24, 0, -34), SPAWN_YAW);
    P.vel = yawDir(P.yaw) * 6.0f;
    Input kf = noInput();
    kf.ollie = true;
    simStep(kf);
    runFor(0.05f, noInput());
    Input k2 = noInput();
    k2.kick = true;
    simStep(k2);
    long before = g_score;
    runFor(2.0f, noInput());
    check(P.state == P_GROUND && g_score > before, "kickflip lands and banks points", (float)(g_score - before));

    // two flips linked through the landing grace window bank as one x2 combo
    resetPlayer(V3(-24, 0, -34), SPAWN_YAW);
    P.vel = yawDir(P.yaw) * 7.0f;
    g_combo.list.clear();
    before = g_score;
    simStep(ol);
    runFor(0.05f, noInput());
    simStep(k2);
    for (int i = 0; i < 240 && P.state != P_GROUND; i++) simStep(noInput());
    simStep(ol);
    runFor(0.05f, noInput());
    Input h2 = noInput();
    h2.heel = true;
    simStep(h2);
    runFor(2.5f, noInput());
    check(g_score - before == 400, "kickflip + heelflip bank as x2 combo", (float)(g_score - before));

    // a half turn in the air lands fakie and scores the spin
    resetPlayer(V3(-24, 0, -34), SPAWN_YAW);
    P.vel = yawDir(P.yaw) * 7.0f;
    before = g_score;
    simStep(ol);
    Input spin = noInput();
    spin.left = true;
    runFor(0.33f, spin, false);
    runFor(2.0f, noInput());
    float alongV = dot(P.vel, yawDir(P.yaw));
    check(P.state == P_GROUND && g_score > before && alongV < 0, "180 spin lands fakie and scores", (float)(g_score - before));

    // stepping into the side of moving traffic knocks the skater down
    int vi = -1;
    for (size_t i = 0; i < g_vehicles.size(); i++)
        if (g_vehicles[i].lane == 1 && g_vehicles[i].speed > 3.0f && fabsf(g_vehicles[i].pos) < 150.0f) vi = (int)i;
    if (vi >= 0) {
        float cx, cz, vyaw;
        vehCenter(g_vehicles[vi], cx, cz, vyaw);
        resetPlayer(V3(cx + 1.0f, 0, cz - g_vehicles[vi].wid * 0.5f - 0.6f), 0.0f);
        P.vel = V3(0, 0, 6.0f);
        runFor(0.3f, noInput(), false);
        check(P.state == P_BAIL, "side of a moving vehicle causes a bail", (float)P.state);
    }

    // kicker launch toward the gap
    resetPlayer(V3(-36, 0, -28), SPAWN_YAW);
    P.vel = yawDir(P.yaw) * 11.0f;
    maxY = 0;
    for (int i = 0; i < 360; i++) {
        simStep(noInput());
        maxY = std::max(maxY, P.pos.y);
    }
    check(maxY > 1.2f, "kicker launches the skater (m)", maxY);

    // ollie onto the yellow flat bar and grind it
    resetPlayer(V3(-70, 0, -19.0f), PI * 0.5f);
    P.vel = yawDir(P.yaw) * 7.0f;
    runFor(0.45f, noInput());
    Input o2 = noInput();
    o2.ollie = true;
    simStep(o2);
    bool grinded = false;
    for (int i = 0; i < 200; i++) {
        simStep(noInput());
        if (P.state == P_GRIND) grinded = true;
    }
    check(grinded, "ollie onto flat bar starts a grind", grinded ? 1.0f : 0.0f);

    // roll down the platform steps
    resetPlayer(V3(-114, 1.6f, -30), PI * 0.5f);
    P.vel = yawDir(P.yaw) * 5.0f;
    runFor(2.5f, noInput(), false);
    check(P.pos.y < 0.05f && P.state == P_GROUND && hspeed() > 4.0f, "rolls down the steps (speed)", hspeed());

    // push up the skywalk ramp to the deck
    resetPlayer(V3(-140, 0, -70), PI * 0.5f);
    P.pos.x = -138.0f;
    P.vel = yawDir(P.yaw) * 10.0f;
    runFor(5.0f, push, false);
    check(P.pos.y > 3.3f, "push up the skywalk ramp (height)", P.pos.y);

    // wall stops the skater
    resetPlayer(V3(-24, 0, -80), PI);
    P.vel = yawDir(P.yaw) * 6.0f;
    runFor(3.0f, noInput(), false);
    check(P.pos.z > -85.4f, "mural wall blocks (z)", P.pos.z);

    // ---- new feel and career checks
    // holding Space crouches and pops higher on release
    resetPlayer(V3(-24, 0, -34), SPAWN_YAW);
    P.vel = yawDir(P.yaw) * 6.0f;
    Input hold = noInput();
    hold.ollieHeld = true;
    runFor(0.5f, hold, false);
    Input rel = noInput();
    rel.ollie = true;
    simStep(rel);
    maxY = 0;
    for (int i = 0; i < 200; i++) {
        simStep(noInput());
        maxY = std::max(maxY, P.pos.y);
    }
    check(maxY > 1.6f && P.state == P_GROUND, "charged ollie pops higher (m)", maxY);

    // ollie at the kicker lip clears the kicker gap
    resetPlayer(V3(-34, 0, -28), SPAWN_YAW);
    P.vel = yawDir(P.yaw) * 10.5f;
    g_combo.list.clear();
    bool gap = false;
    for (int i = 0; i < 400; i++) {
        Input in = noInput();
        if (P.state == P_GROUND && P.pos.x < -44.3f && P.pos.y > 0.8f) in.ollie = true;
        simStep(in);
        for (size_t k = 0; k < g_combo.list.size(); k++)
            if (g_combo.list[k].name == "Kicker Gap") gap = true;
    }
    check(gap, "ollie off the kicker scores Kicker Gap", gap ? 1.0f : 0.0f);

    // the road kicker throws the skater over the near bus lane
    resetPlayer(V3(0, 0, -45), 0.0f);
    P.vel = yawDir(P.yaw) * 10.5f;
    float overLane = 0;
    bool popped = false;
    for (int i = 0; i < 700; i++) {
        Input in = noInput();
        in.up = true;
        if (!popped && P.state == P_GROUND && P.pos.z > -9.8f && P.pos.y > 1.2f) { in.ollie = true; popped = true; }
        simStep(in);
        if (P.pos.z > -6.65f && P.pos.z < -4.15f) overLane = std::max(overLane, std::min(P.pos.y, overLane > 0 ? P.pos.y : 99.0f));
        if (P.pos.z > -6.65f && P.pos.z < -4.15f && P.pos.y < overLane) overLane = P.pos.y;
        if (P.pos.z > 0) break;
    }
    check(popped && overLane > 3.2f, "road kicker clears bus height over lane 0 (m)", overLane);

    // barricades block until their chapter is done
    g_unlockLevel = 0;
    resetPlayer(V3(46, SIDEWALK_H, -12), PI);
    P.vel = yawDir(P.yaw) * 6.0f;
    runFor(2.0f, noInput(), false);
    check(P.pos.z > -16.0f, "site gate blocks at chapter 1 (z)", P.pos.z);
    g_unlockLevel = 2;
    resetPlayer(V3(46, SIDEWALK_H, -12), PI);
    P.vel = yawDir(P.yaw) * 6.0f;
    runFor(2.0f, noInput(), false);
    check(P.pos.z < -18.0f, "site gate open after chapter 2 (z)", P.pos.z);

    // a locked trick does nothing
    g_unlockLevel = 0;
    resetPlayer(V3(-24, 0, -34), SPAWN_YAW);
    P.vel = yawDir(P.yaw) * 6.0f;
    g_combo.list.clear();
    simStep(ol);
    runFor(0.05f, noInput());
    simStep(h2);
    check(g_combo.list.empty(), "heelflip locked in chapter 1", (float)g_combo.list.size());
    runFor(2.0f, noInput());

    // chapter 1 completes from its events and opens the garden
    g_careerOn = true;
    g_noSave = true;
    g_unlockLevel = 0;
    for (size_t i = 0; i < g_chapters[0].goals.size(); i++) g_chapters[0].goals[i].have = 0;
    for (int k = 0; k < 3; k++) evTrick("Kickflip");
    evGap("Kicker Gap");
    evGrind(RT_PLAZA_BAR, 0.5f);
    std::vector<TrickEntry> fake(3);
    evCombo(1500, 3, false);
    evMissionDone("raju");
    careerUpdate(SIM_DT);
    check(g_unlockLevel == 1 && g_card == CARD_COMPLETE, "chapter 1 completes from its tasks", (float)g_unlockLevel);
    for (int i = 0; i < 700; i++) careerUpdate(SIM_DT);
    check(g_card == CARD_INTRO || g_card == CARD_NONE, "chapter 2 intro follows", (float)g_card);
    check(len(P.pos - SPAWN_GARDEN) < 1.0f, "skater moved to the garden spawn", len(P.pos - SPAWN_GARDEN));

    // a delivery job from a food stall pays when the skater reaches the door
    int stall = -1;
    for (size_t i = 0; i < g_stalls.size(); i++) if (g_stalls[i].kind == STALL_VADAPAV) stall = (int)i;
    int dest = pickDest(g_stalls[(size_t)stall].stand, 45.0f, 170.0f, std::vector<int>());
    int rupees0 = g_rupees, del0 = g_deliveries;
    startDeliveryJob(stall, dest);
    resetPlayer(g_dests[(size_t)dest].pos, 0);
    for (int i = 0; i < 5; i++) { missionsUpdate(SIM_DT); careerUpdate(SIM_DT); }
    check(!g_mis.on && g_rupees > rupees0 && g_deliveries == del0 + 1, "delivery pays on arrival (rupees)",
          (float)(g_rupees - rupees0));
    printf("       delivered to %s, %d drop points in the city\n", g_dests[(size_t)dest].name.c_str(), (int)g_dests.size());

    // a bail on a chai delivery spills it
    for (size_t i = 0; i < g_stalls.size(); i++) if (g_stalls[i].kind == STALL_CHAI) stall = (int)i;
    startDeliveryJob(stall, dest);
    resetPlayer(V3(-24, 0, -34), SPAWN_YAW);
    bail("test");
    check(!g_mis.on, "bailing spills a chai delivery", g_mis.on ? 1.0f : 0.0f);
    runFor(2.0f, noInput());

    // Raju's race: passing every ring before him wins and adds him to the crew
    g_friend[0] = false;
    startFriendMission(0);
    for (size_t k = 0; k < g_raceCps.size(); k++) {
        resetPlayer(g_raceCps[k], 0);
        missionsUpdate(SIM_DT);
    }
    missionsUpdate(SIM_DT);
    check(!g_mis.on && g_friend[0], "winning the race befriends Raju", g_friend[0] ? 1.0f : 0.0f);
    // and losing it: Raju reaches the station first
    startFriendMission(0);
    resetPlayer(V3(-60, 0, -60), 0);
    for (int i = 0; i < 120 * 40 && g_mis.on; i++) missionsUpdate(SIM_DT);
    check(!g_mis.on && g_banner.title == "MISSION FAILED", "Raju wins if you dawdle", g_mis.on ? 1.0f : 0.0f);

    // save and load round trip through a scratch HOME
    {
        const char* oldHome = getenv("HOME");
        std::string keep = oldHome ? oldHome : "";
        setenv("HOME", "/tmp", 1);
        g_noSave = false;
        g_rupees = 1234;
        g_owned[GEAR_WAX] = true;
        g_chapters[1].goals[0].have = 3;
        saveGame();
        g_rupees = 0;
        g_owned[GEAR_WAX] = false;
        g_chapters[1].goals[0].have = 0;
        bool ok = loadGame();
        check(ok && g_rupees == 1234 && g_owned[GEAR_WAX] && g_chapters[1].goals[0].have == 3,
              "save file round trip", (float)g_rupees);
        remove("/tmp/.mumbai_skate_save");
        g_noSave = true;
        setenv("HOME", keep.c_str(), 1);
    }
    // difficulty: easy opens every trick and shrinks the targets, text follows
    g_diff = DIFF_EASY;
    applyDifficulty();
    const Goal& heel = g_chapters[1].goals[0];
    check(heel.need == 3 && heel.text == "Land 3 heelflips (K)", "easy: 5 heelflips becomes 3", (float)heel.need);
    check(g_chapters[0].goals[3].param == 500.0f, "easy: 1,000 combo target becomes 500", g_chapters[0].goals[3].param);
    g_unlockLevel = 0;
    check(trickOpen(UL_GRAB) && trickOpen(UL_MANUAL), "easy: grabs and manuals open in chapter 1", 1.0f);
    g_diff = DIFF_MEDIUM;
    applyDifficulty();
    check(!trickOpen(UL_GRAB) && g_chapters[1].goals[0].need == 4, "medium: grabs locked, 4 heelflips", (float)g_chapters[1].goals[0].need);
    check(fabsf(g_chapters[5].timeLimit - 195.0f) < 0.5f, "medium: monsoon clock 3:15", g_chapters[5].timeLimit);

    // tutorial: steps advance from real play and it hands over to chapter 1
    g_diff = DIFF_HARD;
    applyDifficulty();
    startTutorial();
    Input w = noInput();
    w.up = true;
    for (int i = 0; i < 600 && g_tutStep == 0; i++) simStep(w);
    check(g_tutStep == 1, "tutorial: pushing to 15 km/h passes step 1", (float)g_tutStep);
    int steps0 = g_tutStep;
    for (int k = 0; k < TUT_COUNT; k++) {
        tutSkip();
        runFor(0.05f, noInput());
    }
    check(!g_tutorial && g_tutorialDone && g_tutStep >= TUT_COUNT && steps0 == 1, "tutorial: skipping to the end starts chapter 1",
          (float)g_tutStep);
    g_card = CARD_NONE;

    // boost spends energy and raises top speed; food refills energy and costs rupees
    g_careerOn = true;
    g_energy = 100;
    resetPlayer(V3(-24, 0, -34), SPAWN_YAW);
    Input bv = noInput();
    bv.boost = true;
    simStep(bv);
    float topSpeed = 0;
    for (int i = 0; i < 300; i++) {
        simStep(noInput());
        topSpeed = std::max(topSpeed, hspeed());
    }
    check(g_energy < 90.0f && topSpeed > 13.0f, "boost spends energy and passes 13 m/s", topSpeed);
    g_energy = 20;
    g_rupees = 50;
    int vp = -1;
    for (size_t i = 0; i < g_stalls.size(); i++) if (g_stalls[i].kind == STALL_VADAPAV) vp = (int)i;
    bool ate = buyFood(vp);
    check(ate && g_rupees == 35 && g_energy >= 69.0f, "vada pav costs RS 15 and gives 50 energy", g_energy);
    g_rupees = 5;
    check(!buyFood(vp) && g_rupees == 5, "no rupees, no vada pav", (float)g_rupees);

    // every chapter task has a how line and a hint; hints cost rupees after the free one
    int missing = 0;
    for (size_t c = 0; c < g_chapters.size(); c++)
        for (size_t k = 0; k < g_chapters[c].goals.size(); k++)
            if (g_chapters[c].goals[k].how.empty() || g_chapters[c].goals[k].hint.empty()) missing++;
    check(missing == 0, "every chapter task has a tip and a hint", (float)missing);
    g_diff = DIFF_MEDIUM;
    applyDifficulty();
    g_unlockLevel = 1;
    for (size_t k = 0; k < g_chapters[1].goals.size(); k++) g_chapters[1].goals[k].hintPaid = false;
    g_freeHintLevel = -1;
    g_rupees = 100;
    g_focus = -1;
    pressHint();
    g_hintOpen = false;
    cycleFocus();
    pressHint();
    check(g_hintOpen && g_rupees == 80, "first hint free, second costs RS 20", (float)g_rupees);
    g_hintOpen = false;
    g_diff = DIFF_HARD;
    applyDifficulty();

    // grinds count for "land a grind" tasks (Priya's challenge used to never finish)
    g_careerOn = true;
    g_unlockLevel = 1;
    g_diff = DIFF_EASY;
    applyDifficulty();
    startFriendMission(1);
    resetPlayer(V3(-70, 0, -19.0f), PI * 0.5f);
    P.vel = yawDir(P.yaw) * 7.0f;
    runFor(0.45f, noInput());
    simStep(ol);
    runFor(1.2f, noInput());
    bool grindDone = g_mis.on && g_mis.goals.size() > 1 && goalDone(g_mis.goals[1]);
    check(grindDone, "Priya's grind task completes on a real grind", grindDone ? 1.0f : 0.0f);
    endMission();

    // the push animation runs full strokes and stays on at top speed
    resetPlayer(V3(-24, 0, -34), SPAWN_YAW);
    int flips = 0;
    bool last = false;
    for (int i = 0; i < 600; i++) {
        simStep(w);
        if (i > 240 && P.pushing != last) flips++;
        last = P.pushing;
    }
    check(flips <= 2, "push animation does not flicker at top speed", (float)flips);

    // hitting a wall stops you; holding W then pushes forward, not backward
    resetPlayer(V3(-24, 0, -82.5f), PI);
    P.vel = yawDir(P.yaw) * 9.0f;
    g_diff = DIFF_EASY;  // no slam bail on easy
    runFor(1.0f, noInput(), false);
    float yaw0 = P.yaw;
    runFor(1.0f, w, false);
    float along2 = dot(P.vel, yawDir(P.yaw));
    check(along2 >= 0.0f && fabsf(wrapAngle(P.yaw - yaw0)) < 0.1f, "after a wall, W pushes forward", along2);

    // cross traffic turns around instead of vanishing
    bool turned = false;
    for (int i = 0; i < 120 * 60 && !turned; i++) {
        updateTraffic(SIM_DT);
        for (size_t k = 0; k < g_vehicles.size(); k++)
            if (g_vehicles[k].turning) turned = true;
    }
    check(turned, "cross traffic U-turns south of the crossing", turned ? 1.0f : 0.0f);

    // a job from the board pays rupees and XP; a skip marks a task done for a price
    g_diff = DIFF_MEDIUM;
    applyDifficulty();
    g_jobs[0].type = JOB_COMBO;
    g_jobs[0].pay = 350;
    g_jobs[0].xp = 200;
    long xp0 = g_xp;
    int rs0 = g_rupees;
    startJob(0);
    bump(g_mis.goals[0], 1, true);
    missionsUpdate(SIM_DT);
    check(!g_mis.on && g_rupees == rs0 + 350 && g_xp >= xp0 + 200, "job pays RS 350 and 200 XP", (float)(g_rupees - rs0));
    g_unlockLevel = 0;
    for (size_t k = 0; k < g_chapters[0].goals.size(); k++) g_chapters[0].goals[k].have = 0;
    g_focus = -1;
    g_rupees = 2000;
    pressSkip();
    pressSkip();
    check(g_rupees == 500 && goalDone(g_chapters[0].goals[0]), "X X skips a task for RS 1,500", (float)g_rupees);
    g_xp = 0;
    addXP(1300, "test");
    check(skillLevel() == 3, "1,300 XP is skill level 3", (float)skillLevel());
    g_xp = 0;
    g_diff = DIFF_HARD;
    applyDifficulty();

    g_careerOn = false;
    g_unlockLevel = 99;

    printf("%s: %d failing\n", fails ? "SELFTEST FAILED" : "SELFTEST PASSED", fails);
    return fails ? 1 : 0;
}

