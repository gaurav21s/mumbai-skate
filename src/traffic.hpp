#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ---------------------------------------------------------------- vehicles and people

static void vehCenter(const Vehicle& v, float& cx, float& cz, float& yaw) {
    if (v.turning) {
        // front bumper runs round a half circle from lane 4 into lane 5
        float fx = 3.5f * cosf(v.turnA), fz = UTURN_Z + 3.5f * sinf(v.turnA);
        V3 head(-sinf(v.turnA), 0, cosf(v.turnA));
        cx = fx - head.x * v.len * 0.5f;
        cz = fz - head.z * v.len * 0.5f;
        yaw = dirYaw(head.x, head.z);
        return;
    }
    const Lane& L = g_lanes[v.lane];
    float c = v.pos - (float)L.dir * v.len * 0.5f;
    if (L.axis == 0) {
        cx = c; cz = L.fixed;
        yaw = L.dir > 0 ? PI * 0.5f : -PI * 0.5f;
    } else {
        cx = L.fixed; cz = c;
        yaw = L.dir > 0 ? 0.0f : PI;
    }
}
static void vehBounds(const Vehicle& v, float& x0, float& z0, float& x1, float& z1) {
    float cx, cz, yaw;
    vehCenter(v, cx, cz, yaw);
    bool ax = g_lanes[v.lane].axis == 0;
    float hx = ax ? v.len * 0.5f : v.wid * 0.5f, hz = ax ? v.wid * 0.5f : v.len * 0.5f;
    if (v.turning) hx = hz = std::max(v.len, v.wid) * 0.5f;
    x0 = cx - hx; x1 = cx + hx; z0 = cz - hz; z1 = cz + hz;
}

static float g_lightT = 0;
static const float LIGHT_CYCLE = 32.0f;
// 0 green, 1 amber, 2 red for traffic moving along the given axis
static int lightState(int axis) {
    float t = fmodf(g_lightT, LIGHT_CYCLE);
    // both directions get a red phase in between so the crossing can clear
    if (axis == 0) return t < 13.0f ? 0 : (t < 15.5f ? 1 : 2);
    if (t < 19.0f) return 2;
    return t < 28.0f ? 0 : (t < 30.0f ? 1 : 2);
}

static const char* HONKS[] = {"PEEP PEEP!", "Arre, side de!", "Oye, dhyaan se!", "Bhau, hat na!", "PAAAN! PAAAN!"};

static void initTraffic() {
    g_vehicles.clear();
    const int routes[6] = {83, 84, 86, 124, 165, 202};
    for (int l = 0; l < 6; l++) {
        const Lane& L = g_lanes[l];
        int n = l < 4 ? 5 : 3;
        float span = L.maxP - L.minP;
        for (int i = 0; i < n; i++) {
            Vehicle v;
            v.lane = l;
            float r = prand();
            bool outer = (l == 0 || l == 3);
            if (outer && (i == 0 || i == 2 || r < 0.3f)) v.type = prand() < 0.3f ? VEH_DOUBLE : VEH_BUS;
            else if (r < 0.6f) v.type = VEH_AUTO;
            else if (r < 0.82f) v.type = VEH_TAXI;
            else v.type = VEH_CAR;
            switch (v.type) {
                case VEH_BUS: v.len = 11.0f; v.wid = 2.5f; v.hgt = 3.15f; v.cruise = 9.0f; break;
                case VEH_DOUBLE: v.len = 11.0f; v.wid = 2.5f; v.hgt = 4.4f; v.cruise = 8.5f; break;
                case VEH_AUTO: v.len = 2.7f; v.wid = 1.4f; v.hgt = 1.8f; v.cruise = 10.5f; break;
                case VEH_TAXI: v.len = 3.9f; v.wid = 1.6f; v.hgt = 1.55f; v.cruise = 12.0f; break;
                default: v.len = 4.2f; v.wid = 1.75f; v.hgt = 1.5f; v.cruise = 13.0f; break;
            }
            v.cruise += prand() * 2.0f - 1.0f;
            v.pos = L.minP + span * ((float)i + prand() * 0.5f) / (float)n;
            v.speed = v.cruise * 0.5f;
            v.col = SHIRTS[(int)(prand() * 9.99f)];
            v.honkCooldown = 0;
            v.route = routes[(int)(prand() * 5.99f)];
            v.wheelSpin = 0;
            v.turning = false;
            v.turnA = 0;
            recBegin();
            drawVehicleMesh(v);
            v.list = recEnd();
            g_vehicles.push_back(v);
        }
    }
}

// True when any part of the vehicle is inside the intersection box.
static bool inCrossing(const Vehicle& v) {
    float x0, z0, x1, z1;
    vehBounds(v, x0, z0, x1, z1);
    return x1 > -8.5f && x0 < 8.5f && z1 > -8.5f && z0 < 8.5f;
}

static void updateTraffic(float dt) {
    g_lightT += dt;
    bool playerLow = P.pos.y < 2.0f;
    bool crossBusy[2] = {false, false};
    for (size_t i = 0; i < g_vehicles.size(); i++)
        if (inCrossing(g_vehicles[i])) crossBusy[g_lanes[g_vehicles[i].lane].axis] = true;
    for (size_t i = 0; i < g_vehicles.size(); i++) {
        Vehicle& v = g_vehicles[i];
        const Lane& L = g_lanes[v.lane];
        float dirf = (float)L.dir;
        float desired = v.cruise;
        // keep a gap to the vehicle ahead
        float gap = 1e9f;
        for (size_t j = 0; j < g_vehicles.size(); j++) {
            if (j == i || g_vehicles[j].lane != v.lane) continue;
            const Vehicle& o = g_vehicles[j];
            float oRear = o.pos - dirf * o.len;
            float d = (oRear - v.pos) * dirf;
            if (d > -1.0f && d < gap) gap = d;
        }
        if (gap < 40.0f) desired = std::min(desired, std::max(0.0f, (gap - 3.0f) * 0.9f));
        // traffic lights
        int ls = lightState(L.axis);
        if (ls != 0) {
            float d = (L.stopAt - v.pos) * dirf;
            if (d > -0.5f && d < 35.0f && !(ls == 1 && d < v.speed * 0.9f))
                desired = std::min(desired, std::max(0.0f, (d - 0.5f) * 0.7f));
        }
        // never drive into the crossing while cross traffic is still in it
        if (crossBusy[1 - L.axis] && !inCrossing(v)) {
            float entry = -8.5f * dirf;
            float d = (entry - v.pos) * dirf;
            if (d > -0.5f && d < 20.0f) desired = std::min(desired, std::max(0.0f, (d - 0.5f) * 0.8f));
        }
        // brake and honk for the skater
        if (playerLow && P.state != P_BAIL) {
            float pAlong = L.axis == 0 ? P.pos.x : P.pos.z;
            float pLat = L.axis == 0 ? P.pos.z : P.pos.x;
            if (fabsf(pLat - L.fixed) < v.wid * 0.5f + 1.0f) {
                float d = (pAlong - v.pos) * dirf;
                if (d > -0.5f && d < 14.0f) {
                    desired = std::min(desired, std::max(0.0f, (d - 2.5f) * 0.8f));
                    if (d < 11.0f && v.honkCooldown <= 0 && g_toastCooldown <= 0) {
                        popup(HONKS[(int)(prand() * 4.99f)], C(1.0f, 0.95f, 0.6f), 24, 1.4f);
                        v.honkCooldown = 5.0f;
                        g_toastCooldown = 2.5f;
                    }
                }
            }
        }
        if (v.speed < desired) v.speed = std::min(desired, v.speed + 2.5f * dt);
        else v.speed = std::max(desired, v.speed - 9.0f * dt);
        v.pos += dirf * v.speed * dt;
        v.wheelSpin += v.speed * dt / 0.4f;
        v.honkCooldown -= dt;
        // southbound cross traffic turns around instead of vanishing
        if (v.lane == 4 && v.pos >= L.maxP) {
            v.pos = L.maxP;
            v.turning = true;
        }
        if (v.turning) {
            float s = std::min(v.speed, 5.0f);
            v.speed = s;
            v.turnA += s / 3.5f * dt;
            if (v.turnA >= PI) {
                v.turning = false;
                v.turnA = 0;
                v.lane = 5;
                v.pos = g_lanes[5].maxP;
            }
            continue;
        }
        // northbound cross traffic rejoins the southbound lane far out in the haze
        if (v.lane == 5 && v.pos < L.minP) {
            bool clear = true;
            for (size_t j = 0; j < g_vehicles.size(); j++)
                if (j != i && g_vehicles[j].lane == 4 && fabsf(g_vehicles[j].pos - g_lanes[4].minP) < 18.0f) clear = false;
            if (clear) {
                v.lane = 4;
                v.pos = g_lanes[4].minP;
            } else {
                v.speed = 0;
            }
            continue;
        }
        // loop back to the start of the lane once far out in the haze
        bool past = L.dir > 0 ? v.pos > L.maxP : v.pos < L.minP;
        if (past) {
            float start = L.dir > 0 ? L.minP : L.maxP;
            bool clear = true;
            for (size_t j = 0; j < g_vehicles.size(); j++)
                if (j != i && g_vehicles[j].lane == v.lane && fabsf(g_vehicles[j].pos - start) < 18.0f) clear = false;
            if (clear) v.pos = start;
            else v.speed = 0;
        }
    }
}

static void initTrains() {
    g_trains.clear();
    Train a = {-150.0f, 1, 21.0f, -50.0f, 9, 0, {}};
    Train b = {300.0f, -1, 18.0f, -46.0f, 9, 1, {}};
    g_trains.push_back(a);
    g_trains.push_back(b);
    for (size_t i = 0; i < g_trains.size(); i++) bakeTrain(g_trains[i]);
}

static void updateTrains(float dt) {
    for (size_t i = 0; i < g_trains.size(); i++) {
        Train& t = g_trains[i];
        t.x += (float)t.dir * t.speed * dt;
        float total = (float)t.cars * (TRAIN_CAR_L + TRAIN_GAP);
        if (t.dir > 0 && t.x - total > 430.0f) t.x = -430.0f;
        if (t.dir < 0 && t.x + total < -430.0f) t.x = 430.0f;
    }
}

static void updatePeds(float dt) {
    for (size_t i = 0; i < g_peds.size(); i++) {
        Ped& p = g_peds[i];
        if (p.bumpT > 0) { p.bumpT -= dt; continue; }
        if (!p.moving) { p.phase += dt; continue; }
        if (p.wait > 0) { p.wait -= dt; continue; }
        V3 tgt = p.target ? p.b : p.a;
        V3 d = tgt - p.pos;
        d.y = 0;
        float dist = len(d);
        if (dist < 0.3f) {
            p.target ^= 1;
            p.wait = prand() * 3.0f;
            continue;
        }
        V3 dir = d * (1.0f / dist);
        float want = dirYaw(dir.x, dir.z);
        p.yaw += wrapAngle(want - p.yaw) * std::min(1.0f, dt * 6.0f);
        V3 toP = P.pos - p.pos;
        toP.y = 0;
        if (len(toP) < 2.0f && dot(toP, dir) > 0 && fabsf(P.pos.y - p.pos.y) < 1.0f) continue;  // wait for the skater
        p.pos += dir * (p.speed * dt);
        p.phase += p.speed * dt * 5.5f;
    }
}


