#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ---------------------------------------------------------------- player physics

static void bail(const std::string& msg) {
    if (P.state == P_BAIL) return;
    P.state = P_BAIL;
    P.bailT = 0;
    P.manual = false;
    P.grab = 0;
    P.boardPos = P.pos + V3(0, 0.1f, 0);
    P.boardVel = P.vel * 0.9f + V3(0, 3.5f, 0);
    P.boardSpin = 0;
    P.backflipOn = false;
    P.sliding = false;
    P.vel.x *= 0.6f;
    P.vel.z *= 0.6f;
    popup("BAILED!", C(1.0f, 0.3f, 0.25f), 46, 1.8f);
    popup(msg, COL_WHITE, 22, 1.8f);
    loseCombo();
    g_combo.grace = 0;
    g_shake = 0.45f;
    dustBurst(P.pos, 18, 2.5f);
    missionOnBail();
    g_energy = std::max(0.0f, g_energy - 8.0f);
    g_boostT = 0;
}

// Letters float around the block from chapter 4 on.
static void updateLetters() {
    if (!has(UL_MANUAL)) return;  // letters are tied to the story, not the trick unlocks
    V3 c = P.pos + V3(0, 0.9f, 0);
    for (size_t i = 0; i < g_letters.size(); i++) {
        Letter& l = g_letters[i];
        if (l.got) continue;
        if (len(l.pos - c) < 1.5f) {
            l.got = true;
            g_lettersGot++;
            g_score += 1000;
            std::string s = "LETTER ";
            s += l.ch;
            s += "  +1,000";
            popup(s, C(0.4f, 1.0f, 0.9f), 30, 2.0f);
            for (int k = 0; k < 30; k++) sparkle(l.pos - V3(0, 0.8f, 0), C(0.4f, 1.0f, 0.9f));
            if (g_lettersGot == (int)g_letters.size()) {
                g_score += 5000;
                popup("C-H-A-I COMPLETE! CUTTING CHAI TIME  +5,000", C(1.0f, 0.6f, 0.2f), 30, 3.5f);
            }
            evLetters(g_lettersGot);
        }
    }
}

static void startAir() {
    P.state = P_AIR;
    P.airTime = 0;
    P.spinAccum = 0;
    P.spinVel = 0;
    P.maxY = P.pos.y;
    P.hopped.clear();
    P.airStartCount = (int)g_combo.list.size();
    P.roll = P.rollTarget = 0;
    P.shove = P.shoveTarget = 0;
    P.flipCode = -1;
    P.flipIdx = -1;
    P.grab = 0;
    P.manual = false;
    P.manualBuffer = 0;
    P.takeoff = P.pos;
    P.pushing = false;
    P.airGrabT = 0;
    P.backflipOn = false;
    P.backflip = 0;
    P.sliding = false;
}

// Tricks that only resolve when the air phase ends: spins, air time, big drops.
static void finalizeAir(float landY) {
    float deg = fabsf(P.spinAccum) * RAD2DEG;
    // same 55 degree tolerance the landing check allows, so a clean fakie landing always counts as a 180
    int n = (int)((deg + 55.0f) / 180.0f);
    if (n >= 1) {
        static const float SPIN_PTS[6] = {150, 400, 800, 1400, 2200, 3200};
        std::string name = std::string(P.spinAccum > 0 ? "FS " : "BS ") + std::to_string(n * 180);
        addTrick(name, SPIN_PTS[std::min(n, 6) - 1]);
    }
    if (P.airTime > 1.2f) addTrick("Big Air", 120.0f * P.airTime);
    if (P.maxY - landY > 3.0f) addTrick("Big Drop", 250.0f);
    P.grab = 0;
    P.spinAccum = 0;
}

// Resolves horizontal motion against solids, vehicles and pedestrians.
// Returns the speed of the hardest impact this step.
static float moveHoriz(float dt, float step) {
    float impact = 0;
    float nx = P.pos.x + P.vel.x * dt;
    if (blockedAt(nx, P.pos.z, P.pos.y, step)) {
        impact = std::max(impact, fabsf(P.vel.x));
        P.vel.x = 0;
    } else {
        P.pos.x = nx;
    }
    float nz = P.pos.z + P.vel.z * dt;
    if (blockedAt(P.pos.x, nz, P.pos.y, step)) {
        impact = std::max(impact, fabsf(P.vel.z));
        P.vel.z = 0;
    } else {
        P.pos.z = nz;
    }
    if (P.state == P_BAIL) return impact;
    for (size_t i = 0; i < g_vehicles.size(); i++) {
        const Vehicle& v = g_vehicles[i];
        float x0, z0, x1, z1;
        vehBounds(v, x0, z0, x1, z1);
        x0 -= PLAYER_R; z0 -= PLAYER_R; x1 += PLAYER_R; z1 += PLAYER_R;
        if (P.pos.x < x0 || P.pos.x > x1 || P.pos.z < z0 || P.pos.z > z1) continue;
        if (P.pos.y >= v.hgt - 0.05f) continue;
        if (v.speed > 1.5f || hspeed() > 7.0f) {
            bail(std::string("Hit by a ") + vehName(v.type) + "!");
            return impact;
        }
        // push out along the shallowest side
        float pen[4] = {P.pos.x - x0, x1 - P.pos.x, P.pos.z - z0, z1 - P.pos.z};
        int k = 0;
        for (int j = 1; j < 4; j++) if (pen[j] < pen[k]) k = j;
        if (k == 0) { P.pos.x = x0; P.vel.x = std::min(P.vel.x, 0.0f); }
        if (k == 1) { P.pos.x = x1; P.vel.x = std::max(P.vel.x, 0.0f); }
        if (k == 2) { P.pos.z = z0; P.vel.z = std::min(P.vel.z, 0.0f); }
        if (k == 3) { P.pos.z = z1; P.vel.z = std::max(P.vel.z, 0.0f); }
    }
    for (size_t i = 0; i < g_friends.size(); i++) {
        Friend& f = g_friends[i];
        if (!f.skater || f.follow || fabsf(f.pos.y - P.pos.y) > 1.2f) continue;  // the crew riding along gives way
        float dx = P.pos.x - f.pos.x, dz = P.pos.z - f.pos.z;
        float d2 = dx * dx + dz * dz;
        const float R = 0.6f;
        if (d2 > R * R) continue;
        float d = sqrtf(std::max(d2, 1e-6f));
        P.pos.x = f.pos.x + dx / d * R;
        P.pos.z = f.pos.z + dz / d * R;
        P.vel.x *= 0.5f;
        P.vel.z *= 0.5f;
        if (g_toastCooldown <= 0) {
            popup(std::string(f.name) + ": Oye, dhyaan se!", C(1.0f, 0.95f, 0.6f), 22, 1.4f);
            g_toastCooldown = 2.0f;
        }
    }
    for (size_t i = 0; i < g_peds.size(); i++) {
        Ped& p = g_peds[i];
        if (fabsf(p.pos.y - P.pos.y) > 1.2f) continue;
        float dx = P.pos.x - p.pos.x, dz = P.pos.z - p.pos.z;
        float d2 = dx * dx + dz * dz;
        const float R = 0.55f;
        if (d2 > R * R) continue;
        float d = sqrtf(std::max(d2, 1e-6f));
        p.bumpT = 0.9f;
        if (hspeed() > 7.5f && D().pedBail) {
            bail("Ran into a pedestrian. Sorry, uncle!");
            return impact;
        }
        P.pos.x = p.pos.x + dx / d * R;
        P.pos.z = p.pos.z + dz / d * R;
        P.vel.x *= 0.3f;
        P.vel.z *= 0.3f;
        if (g_toastCooldown <= 0) {
            popup(prand() < 0.5f ? "Arre! Dekh ke chala!" : "Oye, skateboard wale!", C(1.0f, 0.95f, 0.6f), 22, 1.4f);
            g_toastCooldown = 2.0f;
        }
    }
    return impact;
}

static void startGrind(int idx, float t, float along) {
    finalizeAir(P.pos.y);
    landedTricks(P.airStartCount);
    P.railTime = 0;
    P.backflipOn = false;
    const Rail& r = g_rails[idx];
    V3 ab = r.b - r.a;
    V3 dxz = norm(V3(ab.x, 0, ab.z));
    P.state = P_GRIND;
    P.rail = idx;
    P.railT = t;
    P.grindDir = along > 0 ? 1 : -1;
    P.grindSpeed = std::max(fabsf(along), 2.5f);
    float ry = dirYaw(dxz.x * (float)P.grindDir, dxz.z * (float)P.grindDir);
    float d = wrapAngle(P.yaw - ry);
    float ad = fabsf(d);
    if (ad < 0.7f) { P.grindType = G_5050; P.yaw = ry; }
    else if (ad > PI - 0.7f) { P.grindType = G_5050; P.yaw = wrapAngle(ry + PI); }
    else { P.grindType = G_BOARDSLIDE; P.yaw = wrapAngle(ry + signf(d) * PI * 0.5f); }
    P.grindTime = 0;
    P.balance = (prand() - 0.5f) * 0.3f;
    P.balVel = 0;
    P.roll = P.rollTarget = 0;
    P.shove = P.shoveTarget = 0;
    P.grab = 0;
    P.grindIdx = addTrick(GRIND_NAMES[P.grindType], GRIND_BASE[P.grindType]);
    evTrick(GRIND_NAMES[P.grindType]);  // "land a grind" tasks count the moment you lock on
    P.pos = r.a + ab * t;
    P.vel = norm(ab) * (P.grindSpeed * (float)P.grindDir);
}

static bool tryCatchRail() {
    if (P.vel.y > 2.5f) return false;
    if (P.backflipOn && P.backflip < 320.0f) return false;
    for (size_t i = 0; i < g_rails.size(); i++) {
        if ((int)i == P.lastRail && P.railCooldown > 0) continue;
        const Rail& r = g_rails[i];
        if (r.needLevel > g_unlockLevel) continue;
        V3 ab = r.b - r.a;
        float l2 = ab.x * ab.x + ab.z * ab.z;
        if (l2 < 0.01f) continue;
        float t = ((P.pos.x - r.a.x) * ab.x + (P.pos.z - r.a.z) * ab.z) / l2;
        if (t < 0.0f || t > 1.0f) continue;
        V3 c = r.a + ab * t;
        float dx = P.pos.x - c.x, dz = P.pos.z - c.z;
        float cr = D().catchR;
        if (dx * dx + dz * dz > cr * cr) continue;
        float dy = P.pos.y - c.y;
        if (dy < -0.4f * cr / 0.5f || dy > 0.45f * cr / 0.5f) continue;
        if (P.vel.y > 0.5f && dy < 0) continue;
        V3 dxz = norm(V3(ab.x, 0, ab.z));
        float along = P.vel.x * dxz.x + P.vel.z * dxz.z;
        if (fabsf(along) < 1.0f) continue;
        startGrind((int)i, t, along);
        return true;
    }
    return false;
}

static void land(const GroundInfo& g, float impactVy) {
    P.pos.y = g.h;
    P.vel.y = 0;
    float flipLeft = std::max(fabsf(P.roll - P.rollTarget), fabsf(P.shove - P.shoveTarget));
    if (flipLeft > D().catchDeg) {
        bail("Didn't catch the board!");
        return;
    }
    if (P.backflipOn && P.backflip < 320.0f) {
        bail("Landed on your head! Ouch.");
        return;
    }
    if (P.grab) {
        bail("Held the grab too long!");
        return;
    }
    if (impactVy < D().dropBail) {
        bail("That drop was too big!");
        return;
    }
    // small mistakes cost speed instead of a bail
    bool sketchy = flipLeft > 12.0f;
    float hs = hspeed();
    if (hs > 1.5f) {
        float vy = dirYaw(P.vel.x, P.vel.z);
        float d = fabsf(wrapAngle(P.yaw - vy));
        float off = std::min(d, PI - d);
        if (off > gearSketchy()) {
            bail("Landed sideways!");
            return;
        }
        if (off > 0.95f) sketchy = true;
        P.yaw = d < PI * 0.5f ? vy : wrapAngle(vy + PI);
    }
    finalizeAir(g.h);
    checkGaps(P.takeoff, P.pos);
    P.roll = P.rollTarget = P.shove = P.shoveTarget = 0;
    P.state = P_GROUND;
    P.landT = 0;
    P.spinVel = 0;
    P.flipIdx = -1;
    P.backflipOn = false;
    P.backflip = 0;
    int n = (int)g_combo.list.size();
    if (n > P.airStartCount) {
        std::string s;
        for (int i = P.airStartCount; i < n; i++) {
            if (!s.empty()) s += " + ";
            s += g_combo.list[i].name;
        }
        popup(s, C(0.55f, 1.0f, 0.55f), 24, 1.3f);
        landedTricks(P.airStartCount);
        missionOnLand(n - P.airStartCount);
    }
    if (P.airGrabT > 0) evGrab(P.airGrabT);
    dustBurst(P.pos, impactVy < -10.0f ? 16 : 8, 1.5f + fabsf(impactVy) * 0.12f);
    if (impactVy < -13.0f) g_shake = std::max(g_shake, 0.18f);
    if (sketchy) {
        float k = g_owned[GEAR_WHEELS] ? 0.8f : 0.65f;
        P.vel.x *= k;
        P.vel.z *= k;
        popup("SKETCHY!", C(1.0f, 0.7f, 0.4f), 24, 1.2f);
    }
    if (!g_combo.list.empty()) g_combo.grace = sketchy ? comboGrace() * 0.5f : comboGrace();
    if (P.manualBuffer > 0 && hs > 1.0f && trickOpen(UL_MANUAL)) {
        P.manual = true;
        P.noseManual = false;
        P.manualTime = 0;
        P.balance = 0;
        P.balVel = 0;
        P.manualIdx = addTrick("Manual", 50);
    }
}

static void flipPress(int key) {
    if (P.grab) return;
    if (key != 0 && !trickOpen(UL_HEEL)) {
        lockedMsg(key == 1 ? "HEELFLIP" : "POP SHOVE-IT", UL_HEEL);
        return;
    }
    bool inProgress = fabsf(P.roll - P.rollTarget) > 1.0f || fabsf(P.shove - P.shoveTarget) > 1.0f;
    int up = (inProgress && P.flipIdx >= 0) ? flipUpgrade(P.flipCode, key) : -1;
    if (key == 0) P.rollTarget += 360.0f;
    if (key == 1) P.rollTarget -= 360.0f;
    if (key == 2) P.shoveTarget += 180.0f;
    if (up >= 0) {
        P.flipCode = up;
        setTrick(P.flipIdx, FLIP_NAMES[up], FLIP_PTS[up]);
    } else {
        P.flipCode = key == 0 ? F_KICK : (key == 1 ? F_HEEL : F_SHOVE);
        P.flipIdx = addTrick(FLIP_NAMES[P.flipCode], FLIP_PTS[P.flipCode]);
    }
}

static void exitGrind(bool pop) {
    const Rail& r = g_rails[P.rail];
    P.lastRail = P.rail;
    P.railCooldown = 0.35f;
    V3 v = P.vel;
    float vy = dirYaw(v.x, v.z);
    float d = fabsf(wrapAngle(P.yaw - vy));
    P.yaw = d < PI * 0.5f ? vy : wrapAngle(vy + PI);
    startAir();
    P.vel = v;
    if (pop) P.vel.y = std::max(P.vel.y, 0.0f) + 1.2f;
    (void)r;
}

static void groundStep(float dt, const Input& in) {
    V3 fwd = yawDir(P.yaw);
    V3 right(-fwd.z, 0, fwd.x);
    float along = dot(P.vel, fwd), lat = dot(P.vel, right);
    lat *= expf(-gearGrip() * dt);
    // carve: the board turns and drags its velocity along with it
    float turn = (in.left ? 1.0f : 0.0f) - (in.right ? 1.0f : 0.0f);
    float rate = 2.3f / (1.0f + fabsf(along) * 0.05f);
    if (P.manual) rate *= 0.6f;
    P.yaw = wrapAngle(P.yaw + turn * rate * dt);
    P.lean = approach(P.lean, turn * clampf(fabsf(along) / 7.0f, 0, 1) * 16.0f, 90.0f * dt);
    fwd = yawDir(P.yaw);
    right = V3(-fwd.z, 0, fwd.x);
    // powerslide: brake at speed and the board kicks sideways; it links combos
    bool wantSlide = in.down && fabsf(along) > 5.0f && !P.manual;
    if (wantSlide && !P.sliding) {
        P.sliding = true;
        P.slideDir = turn != 0 ? turn : 1.0f;
        if (!g_combo.list.empty()) addTrick("Powerslide", 100);
    } else if (!wantSlide && P.sliding) {
        P.sliding = false;
        if (!g_combo.list.empty()) g_combo.grace = comboGrace();
    }
    P.slideAng = approach(P.slideAng, P.sliding ? 75.0f * P.slideDir : 0.0f, 420.0f * dt);
    if (!P.manual) {
        bool pushKey = in.up || (g_boostT > 0 && !in.down);  // boost pushes for you
        // W pushes toward the nose, unless you are already rolling fakie at speed
        float dir = along < -2.0f ? -1.0f : 1.0f;
        if (pushKey && fabsf(along) < gearPushMax()) along += dir * gearPushAcc() * dt;
        // the push animation runs whole strokes, so it never flickers at top speed
        bool wantStroke = pushKey && fabsf(along) < gearPushMax() - 0.3f;
        if (P.pushing) {
            P.pushPhase += dt / 0.7f;
            if (P.pushPhase >= 1.0f) {
                P.pushPhase = 0;
                P.pushing = wantStroke;
                P.pushDir = dir;
            }
        } else if (wantStroke) {
            P.pushing = true;
            P.pushPhase = 0;
            P.pushDir = dir;
        }
        if (in.down) along = approach(along, 0, (P.sliding ? BRAKE * 1.3f : BRAKE) * dt);
    }
    if (P.sliding && fxrand() < 0.5f) dustBurst(P.pos, 1, 1.2f);
    GroundInfo gi = groundAt(P.pos.x, P.pos.z, P.pos.y + 0.05f);
    float gradAlong = gi.gx * fwd.x + gi.gz * fwd.z;
    along -= SLOPE_G * gradAlong / sqrtf(1.0f + gradAlong * gradAlong) * dt;
    along = approach(along, 0, ((g_rain > 0.5f ? 0.18f : 0.3f) + 0.004f * along * along) * dt);
    along = clampf(along, -MAX_SPEED, MAX_SPEED);
    P.vel = fwd * along + right * lat;
    P.slopeVy = gi.gx * P.vel.x + gi.gz * P.vel.z;
    if (P.pushing) P.pushT += dt;

    // manual balance: W and S rock the board
    if (P.manual) {
        P.manualTime += dt;
        float diff = 1.0f + P.manualTime * 0.2f;
        float noise = sinf(P.manualTime * 3.1f) * 0.6f + sinf(P.manualTime * 7.3f + 1.0f) * 0.4f;
        P.balVel += (P.balance * 2.0f + noise * 0.9f * gearWobble()) * diff * dt;
        P.balVel += ((in.up ? 1.0f : 0.0f) - (in.down ? 1.0f : 0.0f)) * 4.5f * dt;
        P.balVel *= expf(-0.8f * dt);
        P.balance += P.balVel * dt;
        const char* name = P.noseManual ? "Nose Manual" : "Manual";
        setTrick(P.manualIdx, name, (P.noseManual ? 80.0f : 50.0f) + (P.noseManual ? 250.0f : 200.0f) * P.manualTime);
        if (fabsf(P.balance) > 1.0f) {
            bail(P.noseManual ? "Nose dived!" : "Scraped the tail!");
            return;
        }
        if (fabsf(along) < 1.0f || in.manual || in.noseManual) {
            P.manual = false;
            g_combo.grace = comboGrace();
        }
    } else if ((in.manual || in.noseManual) && !trickOpen(UL_MANUAL)) {
        lockedMsg("MANUALS", UL_MANUAL);
    } else if ((in.manual || in.noseManual) && fabsf(along) > 1.0f) {
        P.manual = true;
        P.noseManual = in.noseManual;
        P.manualTime = 0;
        P.balance = (prand() - 0.5f) * 0.2f;
        P.balVel = 0;
        P.manualIdx = addTrick(P.noseManual ? "Nose Manual" : "Manual", P.noseManual ? 80.0f : 50.0f);
        P.pushing = false;
    }

    float impact = moveHoriz(dt, STEP_UP);
    if (P.state == P_BAIL) return;
    if (impact > D().slam) {
        bail("Slammed into a wall!");
        return;
    }
    GroundInfo g2 = groundAt(P.pos.x, P.pos.z, P.pos.y + STEP_UP);
    float expected = P.pos.y + P.slopeVy * dt;
    if (g2.h < expected - 0.12f) {
        float drop = P.pos.y - g2.h;
        if (P.slopeVy > 0.6f || drop > 0.3f) {
            V3 v = P.vel;
            float svy = P.slopeVy;
            bool wasManual = P.manual;
            startAir();
            P.vel = v;
            P.vel.y = std::max(svy, 0.0f);
            if (wasManual) P.manualBuffer = 0.3f;
            return;
        }
    }
    P.pos.y = g2.h;
    P.vel.y = 0;
    P.lastSafe = P.pos;

    if (in.ollie) {
        V3 v = P.vel;
        float svy = P.slopeVy;
        startAir();
        P.vel = v;
        P.vel.y = std::max(svy, 0.0f) * 0.8f + OLLIE_V * gearOllie() * (1.0f + 0.25f * P.ollieCharge);
        P.crouch = 1.0f;
        return;
    }
    // bank the combo once the skater has rolled away cleanly; crouching for an
    // ollie or powersliding holds it open for a moment
    bool charging = in.ollieHeld && P.ollieHoldT < 1.0f;
    if (!P.manual && !P.sliding && !charging && !g_combo.list.empty()) {
        g_combo.grace -= dt;
        if (g_combo.grace <= 0) bankCombo();
    }
}

static void airStep(float dt, const Input& in) {
    P.airTime += dt;
    float yPrev = P.pos.y;
    P.vel.y -= GRAVITY * dt;
    float turn = (in.left ? 1.0f : 0.0f) - (in.right ? 1.0f : 0.0f);
    P.spinVel = approach(P.spinVel, turn * SPIN_RATE, 30.0f * dt);
    float dyaw = P.spinVel * dt;
    P.yaw = wrapAngle(P.yaw + dyaw);
    P.spinAccum += dyaw;
    if (in.kick) flipPress(0);
    if (in.heel) flipPress(1);
    if (in.shove) flipPress(2);
    if (in.backflip && !P.backflipOn) {
        if (!trickOpen(UL_CHAI)) {
            lockedMsg("BOMBAY BACKFLIP", UL_CHAI);
        } else if (!chaiFull()) {
            if (g_lockMsgT <= 0) {
                popup("CHAI METER IS EMPTY. BIG COMBOS OR A CUTTING CHAI FILL IT", C(1.0f, 0.75f, 0.45f), 22, 1.8f);
                g_lockMsgT = 1.5f;
            }
        } else if (!P.grab) {
            P.backflipOn = true;
            P.backflip = 0;
            g_chai = 0;
            P.vel.y = std::max(P.vel.y, 5.5f);  // the chai gives a little extra lift
            addTrick("Bombay Backflip", 2500);
            for (int k = 0; k < 40; k++) sparkle(P.pos, C(1.0f, 0.8f, 0.3f));
        }
    }
    if (P.backflipOn) P.backflip = approach(P.backflip, 360.0f, 720.0f * dt);
    P.roll = approach(P.roll, P.rollTarget, FLIP_RATE * dt);
    P.shove = approach(P.shove, P.shoveTarget, SHOVE_RATE * dt);
    bool flipping = fabsf(P.roll - P.rollTarget) > 1.0f || fabsf(P.shove - P.shoveTarget) > 1.0f;
    // grabs are held; the trick grows the longer you hold it
    if (!P.grab && (in.indy || in.melon) && !trickOpen(UL_GRAB)) {
        lockedMsg("GRABS", UL_GRAB);
    } else if (!P.grab && !flipping && (in.indy || in.melon)) {
        P.grab = in.indy ? 1 : 2;
        P.grabTime = 0;
        P.grabIdx = addTrick(P.grab == 1 ? "Indy Grab" : "Melon Grab", 100);
    }
    if (P.grab) {
        bool held = P.grab == 1 ? in.indyHeld : in.melonHeld;
        if (!held) P.grab = 0;
        else {
            P.grabTime += dt;
            P.airGrabT = std::max(P.airGrabT, P.grabTime);
            setTrick(P.grabIdx, P.grab == 1 ? "Indy Grab" : "Melon Grab", 100.0f + 350.0f * P.grabTime);
        }
    }
    if (in.manual && trickOpen(UL_MANUAL)) P.manualBuffer = 0.3f;
    P.manualBuffer -= dt;

    float impact = moveHoriz(dt, 0.05f);
    if (P.state == P_BAIL) return;
    if (impact > D().slam + 1.0f) {
        bail("Flew into a wall!");
        return;
    }
    P.pos.y += P.vel.y * dt;
    // head bump on anything overhead
    if (P.vel.y > 0) {
        for (size_t i = 0; i < g_solids.size(); i++) {
            const Solid& s = g_solids[i];
            if (!s.inside(P.pos.x, P.pos.z, 0.1f)) continue;
            if (s.y0 >= yPrev + PLAYER_H - 0.05f && s.y0 < P.pos.y + PLAYER_H) {
                P.pos.y = s.y0 - PLAYER_H;
                P.vel.y = 0;
            }
        }
    }
    P.maxY = std::max(P.maxY, P.pos.y);
    // hopping over traffic, and bouncing off roofs
    for (size_t i = 0; i < g_vehicles.size(); i++) {
        const Vehicle& v = g_vehicles[i];
        float x0, z0, x1, z1;
        vehBounds(v, x0, z0, x1, z1);
        if (P.pos.x < x0 || P.pos.x > x1 || P.pos.z < z0 || P.pos.z > z1) continue;
        if (P.pos.y < v.hgt && yPrev >= v.hgt - 0.05f && P.vel.y < 0) {
            P.pos.y = v.hgt;
            P.vel.y = 8.0f;
            addTrick("Roof Bounce", 300);
        }
        if (P.pos.y >= v.hgt - 0.05f &&
            std::find(P.hopped.begin(), P.hopped.end(), (int)i) == P.hopped.end()) {
            P.hopped.push_back((int)i);
            if (v.type == VEH_BUS || v.type == VEH_DOUBLE) addTrick("Bus Jump", 1000);
            else if (v.type == VEH_AUTO) addTrick("Auto Hop", 500);
            else if (v.type == VEH_TAXI) addTrick("Taxi Hop", 500);
            else addTrick("Car Hop", 400);
        }
    }
    if (tryCatchRail()) return;
    GroundInfo g = groundAt(P.pos.x, P.pos.z, yPrev + 0.05f);
    if (P.pos.y <= g.h && P.vel.y <= 0) land(g, P.vel.y);
}

static void grindStep(float dt, const Input& in) {
    const Rail& r = g_rails[P.rail];
    V3 ab = r.b - r.a;
    float L = len(ab);
    V3 d3 = ab * (1.0f / L);
    P.grindSpeed += -SLOPE_G * d3.y * (float)P.grindDir * dt - 0.4f * dt;
    P.grindTime += dt;
    P.railTime += dt;
    evGrind(g_rails[P.rail].tag, P.railTime);
    if (P.grindSpeed > 1.5f) sparks(P.pos + V3(0, 0.03f, 0), P.vel);
    setTrick(P.grindIdx, GRIND_NAMES[P.grindType], GRIND_BASE[P.grindType] + GRIND_RATE[P.grindType] * P.grindTime);
    // balance: A and D lean against the wobble
    float diff = 1.0f + P.grindTime * 0.25f;
    float noise = sinf(P.grindTime * 2.7f + (float)P.rail) * 0.7f + sinf(P.grindTime * 6.1f) * 0.3f;
    P.balVel += (P.balance * 2.2f + noise * 1.0f * gearWobble()) * diff * dt;
    P.balVel += ((in.right ? 1.0f : 0.0f) - (in.left ? 1.0f : 0.0f)) * 4.5f * dt;
    P.balVel *= expf(-0.8f * dt);
    P.balance += P.balVel * dt;
    if (fabsf(P.balance) > 1.0f) {
        bail("Lost balance on the rail!");
        return;
    }
    // switch grinds mid-rail for extra tricks
    int sw = -1;
    if (in.kick) sw = G_NOSEGRIND;
    if (in.heel) sw = G_50;
    if (in.shove) sw = G_CROOKED;
    if (sw >= 0 && !trickOpen(UL_HEEL)) {
        lockedMsg("GRIND SWITCHES", UL_HEEL);
        sw = -1;
    }
    if (sw >= 0 && sw != P.grindType) {
        P.grindType = sw;
        P.grindTime = 0;
        P.grindIdx = addTrick(GRIND_NAMES[sw], GRIND_BASE[sw]);
        evTrick(GRIND_NAMES[sw]);
        float ry = dirYaw(d3.x * (float)P.grindDir, d3.z * (float)P.grindDir);
        P.yaw = sw == G_CROOKED ? wrapAngle(ry + 0.25f) : ry;
    }
    P.railT += (float)P.grindDir * P.grindSpeed * dt / L;
    P.pos = r.a + ab * clampf(P.railT, 0, 1);
    P.vel = d3 * (P.grindSpeed * (float)P.grindDir);
    if (in.ollie) {
        exitGrind(false);
        P.vel.y = OLLIE_V * 0.85f * gearOllie() * (1.0f + 0.25f * P.ollieCharge);
        P.crouch = 1.0f;
        return;
    }
    if (P.grindSpeed < 0.4f) {
        exitGrind(false);
        return;
    }
    if (P.railT > 1.0f || P.railT < 0.0f) exitGrind(true);
}

static void bailStep(float dt) {
    P.bailT += dt;
    P.vel.y -= GRAVITY * dt;
    float k = expf(-2.5f * dt);
    P.vel.x *= k;
    P.vel.z *= k;
    float yPrev = P.pos.y;
    moveHoriz(dt, STEP_UP);
    P.pos.y += P.vel.y * dt;
    GroundInfo g = groundAt(P.pos.x, P.pos.z, yPrev + 0.1f);
    if (P.pos.y <= g.h) {
        P.pos.y = g.h;
        P.vel.y = 0;
    }
    // the board keeps going on its own
    P.boardVel.y -= GRAVITY * dt;
    P.boardPos += P.boardVel * dt;
    P.boardSpin += dt * 9.0f * std::max(0.0f, 1.0f - P.bailT);
    float bg = groundAt(P.boardPos.x, P.boardPos.z, P.boardPos.y + 0.3f).h;
    if (P.boardPos.y < bg) {
        P.boardPos.y = bg;
        P.boardVel.y = fabsf(P.boardVel.y) * 0.3f;
        P.boardVel.x *= 0.7f;
        P.boardVel.z *= 0.7f;
    }
    if (P.bailT > 1.8f) {
        V3 p = P.pos;
        if (blockedAt(p.x, p.z, p.y, STEP_UP)) p = P.lastSafe;
        float yaw = P.yaw;
        resetPlayer(p, yaw);
    }
}

static void playerStep(float dt, const Input& in) {
    if (P.railCooldown > 0) P.railCooldown -= dt;
    P.landT += dt;
    if (in.ollieHeld) {
        P.ollieHoldT += dt;
        P.ollieCharge = std::min(1.0f, P.ollieCharge + dt / 0.45f);
    }
    if (in.ollieHeld && P.state != P_AIR) P.crouch = approach(P.crouch, 0.3f + 0.7f * P.ollieCharge, dt * 6.0f);
    else P.crouch = approach(P.crouch, (in.down || P.state == P_GRIND) ? 0.5f : 0.0f, dt * 3.0f);
    if (P.state != P_GROUND) P.lean = approach(P.lean, 0, 60.0f * dt);
    // boost: a few seconds of extra push, paid for with energy
    if (in.boost && P.state != P_BAIL && g_boostT <= 0) {
        if (g_energy >= boostCost()) {
            g_energy -= boostCost();
            g_boostT = 2.5f;
            g_shake = std::max(g_shake, 0.12f);
        } else if (g_lockMsgT <= 0) {
            popup("TOO TIRED TO BOOST. EAT SOMETHING AT A STALL (F)", C(1.0f, 0.6f, 0.4f), 22, 1.8f);
            g_lockMsgT = 1.5f;
        }
    }
    if (g_boostT > 0 && P.state == P_GROUND && fxrand() < 0.3f) {
        V3 back = yawDir(P.yaw) * -0.35f;
        emit(P.pos + back + V3(fxrange(-0.1f, 0.1f), 0.08f, fxrange(-0.1f, 0.1f)), P.vel * -0.15f + V3(0, 0.3f, 0),
             C(1.0f, 0.75f, 0.35f), 0.3f, 0.03f, 0.0f, true, 2.0f);
    }
    switch (P.state) {
        case P_GROUND: groundStep(dt, in); break;
        case P_AIR: airStep(dt, in); break;
        case P_GRIND: grindStep(dt, in); break;
        default: bailStep(dt); break;
    }
    if (P.state != P_BAIL) updateLetters();
    if (!in.ollieHeld) {
        P.ollieCharge = 0;
        P.ollieHoldT = 0;
    }
}

