#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ---------------------------------------------------------------- HUD

static void hudBegin() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, g_winW, 0, g_winH, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);
    gLighting(false);
    glDisable(GL_FOG);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
}
static void hudEnd() {
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    gLighting(true);
    glEnable(GL_FOG);
}

static void drawBalanceMeter(float y, const char* label) {
    float cx = (float)g_winW * 0.5f, w = 320;
    rect2D(cx - w / 2 - 3, y - 3, cx + w / 2 + 3, y + 17, 0, 0, 0, 0.55f);
    rect2D(cx - w / 2, y, cx - w * 0.2f, y + 14, 0.8f, 0.2f, 0.2f, 0.7f);
    rect2D(cx + w * 0.2f, y, cx + w / 2, y + 14, 0.8f, 0.2f, 0.2f, 0.7f);
    rect2D(cx - w * 0.2f, y, cx + w * 0.2f, y + 14, 0.2f, 0.7f, 0.3f, 0.7f);
    float nx = cx + clampf(P.balance, -1, 1) * w / 2;
    rect2D(nx - 3, y - 6, nx + 3, y + 20, 1, 1, 1, 1);
    int lw = bmpWidth(label, GLUT_BITMAP_HELVETICA_12);
    rect2D(cx - (float)lw / 2 - 6, y + 19, cx + (float)lw / 2 + 6, y + 37, 0.04f, 0.05f, 0.08f, 0.55f);
    bmpTextShadow(cx - (float)lw / 2, y + 24, label, COL_WHITE, GLUT_BITMAP_HELVETICA_12);
}

// Filled rectangle with rounded corners.
static void roundRect(float x0, float y0, float x1, float y1, float r, float cr, float cg, float cb, float a) {
    r = std::min(r, std::min(x1 - x0, y1 - y0) * 0.5f);
    glColor4f(cr, cg, cb, a);
    glBegin(GL_QUADS);
    glVertex2f(x0 + r, y0); glVertex2f(x1 - r, y0); glVertex2f(x1 - r, y1); glVertex2f(x0 + r, y1);
    glVertex2f(x0, y0 + r); glVertex2f(x0 + r, y0 + r); glVertex2f(x0 + r, y1 - r); glVertex2f(x0, y1 - r);
    glVertex2f(x1 - r, y0 + r); glVertex2f(x1, y0 + r); glVertex2f(x1, y1 - r); glVertex2f(x1 - r, y1 - r);
    glEnd();
    float cx[4] = {x0 + r, x1 - r, x1 - r, x0 + r}, cy[4] = {y0 + r, y0 + r, y1 - r, y1 - r};
    for (int k = 0; k < 4; k++) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx[k], cy[k]);
        for (int i = 0; i <= 6; i++) {
            float a0 = PI * (1.0f + 0.5f * (float)k) + (PI * 0.5f) * (float)i / 6.0f;
            glVertex2f(cx[k] + cosf(a0) * r, cy[k] + sinf(a0) * r);
        }
        glEnd();
    }
}

static void panelBox(float x0, float y0, float x1, float y1, float a) {
    if (x1 - x0 > (float)g_winW - 4 && y1 - y0 > (float)g_winH - 4) {
        rect2D(x0, y0, x1, y1, 0, 0, 0, a);  // full-screen dimming stays square
        return;
    }
    roundRect(x0, y0, x1, y1, 9, 0.04f, 0.05f, 0.08f, a);
}

// A keyboard key drawn as a little cap.
static float drawKeyCap(float x, float y, const char* label) {
    float w = std::max(28.0f, (float)bmpWidth(label, GLUT_BITMAP_HELVETICA_12) + 16.0f);
    roundRect(x, y - 3, x + w, y + 25, 5, 0.1f, 0.1f, 0.12f, 0.9f);
    roundRect(x, y, x + w, y + 25, 5, 0.92f, 0.92f, 0.95f, 1.0f);
    glColor4f(0.1f, 0.1f, 0.14f, 1.0f);
    bmpText(x + (w - (float)bmpWidth(label, GLUT_BITMAP_HELVETICA_12)) * 0.5f, y + 8, label, GLUT_BITMAP_HELVETICA_12);
    return w;
}

// Soft darkening at the screen edges.
static void vignette(float W, float H) {
    const float e = 0.2f;
    glBegin(GL_QUADS);
    float t = H * e, s = W * e;
    glColor4f(0, 0, 0, 0.28f); glVertex2f(0, 0); glVertex2f(W, 0);
    glColor4f(0, 0, 0, 0); glVertex2f(W, t); glVertex2f(0, t);
    glColor4f(0, 0, 0, 0); glVertex2f(0, H - t); glVertex2f(W, H - t);
    glColor4f(0, 0, 0, 0.22f); glVertex2f(W, H); glVertex2f(0, H);
    glColor4f(0, 0, 0, 0.25f); glVertex2f(0, 0); glColor4f(0, 0, 0, 0); glVertex2f(s, 0); glVertex2f(s, H);
    glColor4f(0, 0, 0, 0.25f); glVertex2f(0, H);
    glColor4f(0, 0, 0, 0); glVertex2f(W - s, 0); glColor4f(0, 0, 0, 0.25f); glVertex2f(W, 0); glVertex2f(W, H);
    glColor4f(0, 0, 0, 0); glVertex2f(W - s, H);
    glEnd();
}

static void checkBox(float x, float y, bool done) {
    if (done) {
        rect2D(x, y, x + 11, y + 11, 0.35f, 0.9f, 0.45f, 1.0f);
    } else {
        rect2D(x, y, x + 11, y + 11, 0.85f, 0.85f, 0.85f, 0.9f);
        rect2D(x + 1.5f, y + 1.5f, x + 9.5f, y + 9.5f, 0.1f, 0.1f, 0.12f, 0.9f);
    }
}

static std::string mmss(float secs) {
    int s = std::max(0, (int)ceilf(secs));
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%02d", s / 60, s % 60);
    return buf;
}

// Splits text into lines no wider than maxW pixels.
static std::vector<std::string> wrapText(const std::string& text, int maxW, void* font = GLUT_BITMAP_HELVETICA_12) {
    std::vector<std::string> out;
    std::string line, word;
    size_t i = 0;
    while (i <= text.size()) {
        if (i == text.size() || text[i] == ' ') {
            std::string trial = line.empty() ? word : line + " " + word;
            if (!line.empty() && bmpWidth(trial.c_str(), font) > maxW) {
                out.push_back(line);
                line = word;
            } else {
                line = trial;
            }
            word.clear();
        } else {
            word += text[i];
        }
        i++;
    }
    if (!line.empty()) out.push_back(line);
    return out;
}

// Height a task line takes, including the how-to lines under the focused one.
static float goalLineHeight(const Goal& g, bool focused, float maxW) {
    if (!focused || goalDone(g) || g.how.empty()) return 18;
    return 18 + 15.0f * (float)wrapText(g.how, (int)maxW - 18).size() + 4;
}

// One task line: checkbox, text and progress. The focused task is highlighted
// and shows how to do it underneath. Returns the height used.
static float goalLine(const Goal& g, float x, float y, float maxW, bool focused = false) {
    bool done = goalDone(g);
    float h = goalLineHeight(g, focused, maxW);
    if (focused && !done) {
        roundRect(x - 6, y - h + 14, x + maxW + 18, y + 14, 5, 0.35f, 0.85f, 1.0f, 0.13f);
        roundRect(x - 6, y - h + 14, x - 3, y + 14, 1, 0.35f, 0.85f, 1.0f, 1.0f);
    }
    checkBox(x, y - 1, done);
    std::string t = g.text;
    if (!done && g.need > 1 && g.kind != GK_CHECKPOINT) t += "  " + std::to_string(g.have) + "/" + std::to_string(g.need);
    if (!done && g.kind == GK_CHECKPOINT) t += "  " + std::to_string(g.have) + "/" + std::to_string(g.need);
    if (!done && g.kind == GK_SCORE) t += "  " + withCommas(g_score - g_chapterScore0);
    while (bmpWidth(t.c_str(), GLUT_BITMAP_HELVETICA_12) > (int)maxW && t.size() > 4) t = t.substr(0, t.size() - 4) + "..";
    bmpTextShadow(x + 18, y, t.c_str(), done ? C(0.55f, 0.85f, 0.6f) : COL_WHITE, GLUT_BITMAP_HELVETICA_12);
    if (focused && !done && !g.how.empty()) {
        std::vector<std::string> lines = wrapText(g.how, (int)maxW - 18);
        float ly = y - 16;
        for (size_t i = 0; i < lines.size(); i++, ly -= 15)
            bmpTextShadow(x + 18, ly, lines[i].c_str(), C(0.5f, 0.88f, 1.0f), GLUT_BITMAP_HELVETICA_12);
    }
    return h;
}

static void drawTaskPanel(float W, float H) {
    const float pw = 370, x0 = W - pw - 12, top = H - 130;
    float y = top;
    std::vector<std::pair<std::string, Col>> head;
    Chapter* c = curChapter();
    // measure first so the box fits
    Goal* fg = focusGoal();
    float h = 22.0f;
    if (g_mis.on) {
        h += 36 + 10;
        for (size_t i = 0; i < g_mis.goals.size(); i++) h += goalLineHeight(g_mis.goals[i], &g_mis.goals[i] == fg, pw - 40);
    }
    if (c) {
        if (g_mis.on) h += 18;
        else {
            h += 38;
            if (!c->acts.empty()) h += 20;
            for (size_t i = 0; i < c->goals.size(); i++)
                if (actOk(c->goals[i])) h += goalLineHeight(c->goals[i], &c->goals[i] == fg, pw - 40);
        }
    } else {
        h += 36;
    }
    h += 18 + 18;  // key row and crew row
    panelBox(x0, top - h + 16, W - 12, top + 22, 0.45f);
    float x = x0 + 12;
    if (g_mis.on) {
        strokeFit2D(g_mis.title.c_str(), x, y, 15, pw - 24, 0, C(1.0f, 0.7f, 0.25f), 1.0f, false);
        y -= 22;
        std::string info;
        if (g_mis.timeLimit > 0) info += "TIME " + mmss(g_mis.timeLeft) + "   ";
        if (g_mis.id == MIS_DELIVERY || g_mis.id == MIS_TUKARAM) info += "CONDITION " + std::to_string((int)(g_mis.freshness * 100.0f + 0.5f)) + "%   ";
        if (g_mis.id == MIS_DELIVERY) info += "TIP RS " + std::to_string(g_styleTips);
        if (info.empty()) info = "Q to give up";
        bmpTextShadow(x, y, info.c_str(), C(1.0f, 0.9f, 0.6f), GLUT_BITMAP_HELVETICA_12);
        y -= 18;
        for (size_t i = 0; i < g_mis.goals.size(); i++) y -= goalLine(g_mis.goals[i], x, y, pw - 40, &g_mis.goals[i] == fg);
        y -= 10;
    }
    if (c) {
        char buf[128];
        int done = 0;
        for (size_t i = 0; i < c->goals.size(); i++) done += goalDone(c->goals[i]) ? 1 : 0;
        if (g_mis.on) {
            snprintf(buf, sizeof(buf), "CHAPTER %d: %s  (%d/%d)", g_unlockLevel + 1, c->name, done, (int)c->goals.size());
            bmpTextShadow(x, y, buf, C(0.8f, 0.8f, 0.85f), GLUT_BITMAP_HELVETICA_12);
            y -= 18;
        } else {
            snprintf(buf, sizeof(buf), "CHAPTER %d/%d  %s", g_unlockLevel + 1, (int)g_chapters.size(), c->name);
            strokeFit2D(buf, x, y, 15, pw - 24, 0, C(1.0f, 0.8f, 0.3f), 1.0f, false);
            y -= 20;
            bmpTextShadow(x, y, c->english, C(0.8f, 0.85f, 0.9f), GLUT_BITMAP_HELVETICA_12);
            y -= 18;
            if (!c->acts.empty() && g_act >= 1 && g_act <= (int)c->acts.size()) {
                snprintf(buf, sizeof(buf), "STOP %d/%d  %s", g_act, (int)c->acts.size(), c->acts[(size_t)g_act - 1].title);
                strokeFit2D(buf, x, y, 12, pw - 24, 0, C(0.6f, 0.9f, 1.0f), 1.0f, false);
                y -= 20;
            }
            for (size_t i = 0; i < c->goals.size(); i++)
                if (actOk(c->goals[i])) y -= goalLine(c->goals[i], x, y, pw - 40, &c->goals[i] == fg);
        }
    } else {
        strokeText2D("MUMBAI SKATE LEGEND", x, y, 15, 0, C(1.0f, 0.8f, 0.3f), 1.0f, false);
        y -= 20;
        bmpTextShadow(x, y, "Free skate. Deliveries and friend missions still pay.", C(0.8f, 0.85f, 0.9f),
                      GLUT_BITMAP_HELVETICA_12);
        y -= 18;
    }
    // keys for the panel
    std::string keys = "TAB  next     G  hint";
    if (fg && !fg->hintPaid) {
        int cost = hintCost();
        if (cost == 0 || g_freeHintLevel != g_unlockLevel) keys += " (free)";
        else keys += " (RS " + std::to_string(cost) + ")";
    }
    if (fg) keys += "     X  skip (RS " + withCommas(skipCost()) + ")";
    bmpTextShadow(x, y, keys.c_str(), C(0.75f, 0.78f, 0.85f), GLUT_BITMAP_HELVETICA_12);
    y -= 18;
    // crew
    std::string crew = "CREW " + std::to_string(friendCount()) + "/4";
    bmpTextShadow(x, y, crew.c_str(), C(0.7f, 0.7f, 0.75f), GLUT_BITMAP_HELVETICA_12);
    float cx = x + 64;
    for (int i = 0; i < 4; i++) {
        bmpTextShadow(cx, y, FRIEND_NAMES[i], g_friend[i] ? C(0.5f, 0.9f, 1.0f) : C(0.4f, 0.4f, 0.42f),
                      GLUT_BITMAP_HELVETICA_12);
        cx += (float)bmpWidth(FRIEND_NAMES[i], GLUT_BITMAP_HELVETICA_12) + 14;
    }
}

// The full walkthrough for the focused task, opened with G.
static void drawHintPanel(float W, float H) {
    (void)W;
    if (!g_hintOpen) return;
    Goal* g = focusGoal();
    if (!g || goalDone(*g)) {
        g_hintOpen = false;
        return;
    }
    float w = 440, x0 = 12;
    std::vector<std::string> lines = wrapText(g->hint, (int)w - 36);
    float h = 70 + 16.0f * (float)lines.size();
    float y1 = H - 170, y0 = y1 - h;
    panelBox(x0, y0, x0 + w, y1, 0.8f);
    roundRect(x0, y1 - 4, x0 + w, y1, 2, 0.35f, 0.85f, 1.0f, 1.0f);
    bmpTextShadow(x0 + 16, y1 - 24, "HINT", C(0.4f, 0.9f, 1.0f), GLUT_BITMAP_HELVETICA_12);
    bmpTextShadow(x0 + w - 86, y1 - 24, "G  close", C(0.6f, 0.6f, 0.65f), GLUT_BITMAP_HELVETICA_12);
    std::string title = g->text;
    while (bmpWidth(title.c_str()) > (int)w - 32 && title.size() > 4) title = title.substr(0, title.size() - 4) + "..";
    bmpTextShadow(x0 + 16, y1 - 46, title.c_str(), C(1.0f, 0.85f, 0.4f));
    float y = y1 - 68;
    for (size_t i = 0; i < lines.size(); i++, y -= 16) bmpTextShadow(x0 + 16, y, lines[i].c_str(), COL_WHITE, GLUT_BITMAP_HELVETICA_12);
}

// Streaks rushing past the edges of the screen while boosting.
static void drawBoostLines(float W, float H) {
    if (g_boostT <= 0) return;
    float a = std::min(1.0f, g_boostT * 2.0f) * 0.35f;
    glBegin(GL_LINES);
    for (int i = 0; i < 40; i++) {
        unsigned int h = (unsigned int)(i + 7) * 2654435761u;
        float ang = (float)(h & 1023) / 1023.0f * TWO_PI;
        float ph = fmodf((float)((h >> 10) & 1023) / 1023.0f + g_time * 2.2f, 1.0f);
        float r0 = 0.55f + ph * 0.5f, r1 = r0 + 0.12f;
        float cx = W * 0.5f, cy = H * 0.45f, rx = W * 0.5f, ry = H * 0.55f;
        glColor4f(1.0f, 0.95f, 0.85f, 0.0f);
        glVertex2f(cx + cosf(ang) * rx * r0, cy + sinf(ang) * ry * r0);
        glColor4f(1.0f, 0.95f, 0.85f, a);
        glVertex2f(cx + cosf(ang) * rx * r1, cy + sinf(ang) * ry * r1);
    }
    glEnd();
}

static void drawObjectiveArrow(float cx, float cy) {
    V3 t;
    std::string label;
    if (!objectiveTarget(t, label)) return;
    V3 d = t - P.pos;
    float dist = sqrtf(d.x * d.x + d.z * d.z);
    if (dist < 4.0f) return;
    float rel = wrapAngle(dirYaw(d.x, d.z) - g_camYaw);
    float sx = -sinf(rel), sy = cosf(rel);
    float px = -sy, py = sx;
    Col c = g_mis.on ? C(1.0f, 0.75f, 0.25f) : C(0.4f, 1.0f, 0.55f);
    // notched chevron, so the tip is unmistakable
    for (int pass = 0; pass < 2; pass++) {
        float o = pass == 0 ? 2.0f : 0.0f;
        if (pass == 0) glColor4f(0, 0, 0, 0.55f);
        else glColor4f(c.r, c.g, c.b, 0.95f);
        float tx = cx + sx * 26 + o, ty = cy + sy * 26 - o;
        float nx = cx - sx * 4 + o, ny = cy - sy * 4 - o;
        float lx = cx - sx * 16 + px * 13 + o, ly = cy - sy * 16 + py * 13 - o;
        float rx = cx - sx * 16 - px * 13 + o, ry = cy - sy * 16 - py * 13 - o;
        glBegin(GL_TRIANGLES);
        glVertex2f(tx, ty); glVertex2f(lx, ly); glVertex2f(nx, ny);
        glVertex2f(tx, ty); glVertex2f(nx, ny); glVertex2f(rx, ry);
        glEnd();
    }
    char buf[160];
    snprintf(buf, sizeof(buf), "%s  %d M", label.c_str(), (int)dist);
    int w = bmpWidth(buf, GLUT_BITMAP_HELVETICA_12);
    roundRect(cx - (float)w / 2 - 8, cy - 49, cx + (float)w / 2 + 8, cy - 31, 5, 0.04f, 0.05f, 0.08f, 0.55f);
    bmpTextShadow(cx - (float)w / 2, cy - 44, buf, c, GLUT_BITMAP_HELVETICA_12);
}

static void drawCards(float W, float H) {
    float a = 1.0f;
    if (g_card == CARD_INTRO) {
        a = clampf(std::min(g_cardT * 2.0f, (6.0f - g_cardT) * 1.5f), 0, 1);
        const Chapter& c = g_chapters[(size_t)std::min(g_unlockLevel, (int)g_chapters.size() - 1)];
        char buf[64];
        snprintf(buf, sizeof(buf), "CHAPTER %d OF %d", g_unlockLevel + 1, (int)g_chapters.size());
        panelBox(0, H * 0.58f, W, H * 0.9f, 0.35f * a);
        strokeText2D(buf, W / 2, H * 0.84f, 20, 0.5f, C(0.85f, 0.85f, 0.9f), a);
        strokeText2D(c.name, W / 2, H * 0.74f, 58, 0.5f, C(1.0f, 0.75f, 0.2f), a);
        strokeText2D(c.english, W / 2, H * 0.69f, 22, 0.5f, COL_WHITE, a);
        strokeText2D(c.brief, W / 2, H * 0.63f, 16, 0.5f, C(0.8f, 0.95f, 1.0f), a);
    } else if (g_card == CARD_COMPLETE) {
        a = clampf(std::min(g_cardT * 2.0f, (5.0f - g_cardT) * 1.5f), 0, 1);
        const Chapter& c = g_chapters[(size_t)g_cardChapter];
        panelBox(0, H * 0.56f, W, H * 0.9f, 0.35f * a);
        strokeText2D("CHAPTER COMPLETE", W / 2, H * 0.8f, 54, 0.5f, C(1.0f, 0.8f, 0.2f), a);
        strokeText2D(c.name, W / 2, H * 0.74f, 22, 0.5f, COL_WHITE, a);
        strokeText2D(c.reward1, W / 2, H * 0.68f, 18, 0.5f, C(0.5f, 1.0f, 0.6f), a);
        strokeText2D(c.reward2, W / 2, H * 0.63f, 18, 0.5f, C(0.5f, 1.0f, 0.6f), a);
        strokeText2D(g_cardBonus.c_str(), W / 2, H * 0.595f, 16, 0.5f, C(1.0f, 0.9f, 0.6f), a);
    } else if (g_card == CARD_LEGEND) {
        a = clampf(std::min(g_cardT * 2.0f, (LEGEND_CARD_SECS - g_cardT) * 1.0f), 0, 1);
        panelBox(0, H * 0.5f, W, H * 0.92f, 0.4f * a);
        float pulse = 1.0f + 0.04f * sinf(g_cardT * 4.0f);
        strokeText2D("MUMBAI SKATE LEGEND", W / 2, H * 0.82f, 56 * pulse, 0.5f, C(1.0f, 0.8f, 0.2f), a);
        strokeText2D("Aamchi galli, aamcha raja. The gold deck is yours.", W / 2, H * 0.745f, 20, 0.5f, COL_WHITE, a);
        // the crew, one name at a time
        const char* crew[5] = {"RAJU", "PRIYA", "SAM", "TUKARAM", "CHINTU"};
        float cx = W / 2 - 330;
        for (int i = 0; i < 5; i++) {
            float ai = a * clampf((g_cardT - 1.2f - 0.5f * (float)i) * 2.0f, 0, 1);
            strokeText2D(crew[i], cx + 165.0f * (float)i, H * 0.665f, 24, 0.5f, BULBS[(i * 2) % 6], ai);
        }
        float at = a * clampf((g_cardT - 4.0f) * 1.5f, 0, 1);
        strokeText2D("and the whole block, out on the Sea Face in the monsoon night", W / 2, H * 0.615f, 16, 0.5f,
                     C(0.8f, 0.95f, 1.0f), at);
        strokeText2D("Keep skating: deliveries, jobs, friend missions and the shop are still open.", W / 2, H * 0.565f, 16,
                     0.5f, C(0.8f, 0.95f, 1.0f), at);
        strokeText2D("Thanks for playing!", W / 2, H * 0.52f, 18, 0.5f, C(1.0f, 0.6f, 0.8f), at);
    }
}

static void drawTalkUI(float W) {
    if (g_talk.open) {
        float w = 640, x0 = W / 2 - w / 2, y0 = 190, h = 92;
        panelBox(x0, y0, x0 + w, y0 + h, 0.7f);
        rect2D(x0, y0 + h - 3, x0 + w, y0 + h, 1.0f, 0.75f, 0.25f, 0.9f);
        if (!g_talk.speaker.empty())
            strokeText2D(g_talk.speaker.c_str(), x0 + 14, y0 + h - 24, 15, 0, C(1.0f, 0.75f, 0.25f), 1.0f, false);
        for (int i = 0; i < 3; i++) {
            if (g_talk.lines[i].empty()) continue;
            Col c = (i == 2 && g_talk.kind != TALK_BOARD) ? C(0.5f, 1.0f, 0.6f) : COL_WHITE;
            bmpTextShadow(x0 + 14, y0 + h - 44 - 18.0f * (float)i, g_talk.lines[i].c_str(), c, GLUT_BITMAP_HELVETICA_12);
        }
        return;
    }
    if (g_talk.kind == TALK_NONE || g_shopOpen) return;
    std::string s;
    if (g_talk.kind == TALK_FRIEND) s = std::string("E  TALK TO ") + g_friends[(size_t)g_talk.who].name;
    if (g_talk.kind == TALK_SHOP) s = "E  SHOP AT SKATE CREW ADDA";
    if (g_talk.kind == TALK_BOARD) s = g_mis.on ? "FINISH YOUR CURRENT JOB FIRST" : "E  READ THE JOB BOARD";
    if (g_talk.kind == TALK_STALL) {
        const Food& fd = FOODS[g_stalls[(size_t)g_talk.who].kind];
        s = std::string("E  DELIVERY JOB      F  ") + fd.name + "  RS " + std::to_string(fd.price) + "  (+" +
            std::to_string((int)fd.energy) + " ENERGY)";
    }
    int w = bmpWidth(s.c_str());
    panelBox(W / 2 - (float)w / 2 - 12, 188, W / 2 + (float)w / 2 + 12, 214, 0.55f);
    bmpTextShadow(W / 2 - (float)w / 2, 195, s.c_str(), C(1.0f, 0.9f, 0.5f));
}

static void drawShop(float W, float H) {
    const int visible = std::min((int)GEAR_COUNT, std::max(6, (int)((H - 200) / 34)));
    int first = std::max(0, std::min(g_shopSel - visible / 2, (int)GEAR_COUNT - visible));
    float w = 660, rowH = 34, h = 110 + rowH * (float)visible;
    float x0 = W / 2 - w / 2, y1 = H / 2 + h / 2;
    panelBox(0, 0, W, H, 0.35f);
    panelBox(x0, y1 - h, x0 + w, y1, 0.85f);
    rect2D(x0, y1 - 4, x0 + w, y1, 1.0f, 0.4f, 0.75f, 1.0f);
    strokeText2D("SKATE CREW ADDA", x0 + 18, y1 - 40, 24, 0, C(1.0f, 0.5f, 0.8f), 1.0f, false);
    std::string money = "RS " + std::to_string(g_rupees);
    strokeText2D(money.c_str(), x0 + w - 18, y1 - 40, 22, 1.0f, C(0.5f, 1.0f, 0.6f), 1.0f, false);
    float y = y1 - 76;
    static const char* KIND_TAG[7] = {"UPGRADE", "DECK", "OUTFIT", "HAT", "GLASSES", "NECK", "BAG"};
    for (int i = first; i < first + visible; i++) {
        const GearDef& g = GEAR[i];
        bool sel = i == g_shopSel, locked = g_unlockLevel < g.needLevel, owned = g_owned[i];
        if (sel) rect2D(x0 + 8, y - 12, x0 + w - 8, y + rowH - 12, 1.0f, 1.0f, 1.0f, 0.12f);
        Col nc = locked ? C(0.45f, 0.45f, 0.48f) : COL_WHITE;
        bmpTextShadow(x0 + 20, y + 6, KIND_TAG[g.kind], C(1.0f, 0.5f, 0.8f), GLUT_BITMAP_HELVETICA_10);
        bmpTextShadow(x0 + 90, y + 6, g.name, nc, GLUT_BITMAP_HELVETICA_12);
        bmpTextShadow(x0 + 90, y - 8, g.desc, C(0.65f, 0.7f, 0.75f), GLUT_BITMAP_HELVETICA_10);
        std::string st;
        Col sc = C(1.0f, 0.85f, 0.4f);
        if (locked) { st = "AFTER CHAPTER " + std::to_string(g.needLevel); sc = C(0.5f, 0.5f, 0.55f); }
        else if (gearSlot(g.kind) && *gearSlot(g.kind) == i) { st = "WEARING  (ENTER TO TAKE OFF)"; sc = C(0.4f, 1.0f, 0.6f); }
        else if (owned) { st = g.kind == GEAR_UPGRADE ? "OWNED" : "OWNED  (ENTER TO WEAR)"; sc = C(0.6f, 0.85f, 1.0f); }
        else st = "RS " + std::to_string(g.price);
        int sw = bmpWidth(st.c_str(), GLUT_BITMAP_HELVETICA_12);
        bmpTextShadow(x0 + w - 20 - (float)sw, y, st.c_str(), sc, GLUT_BITMAP_HELVETICA_12);
        y -= rowH;
    }
    std::string foot = "W / S  choose     ENTER  buy or wear     E  close        " + std::to_string(g_shopSel + 1) + " / " +
                       std::to_string(GEAR_COUNT);
    bmpTextShadow(x0 + 20, y1 - h + 16, foot.c_str(), C(0.8f, 0.8f, 0.85f), GLUT_BITMAP_HELVETICA_12);
    if (first > 0) bmpTextShadow(x0 + w - 40, y1 - 58, "^", COL_WHITE);
    if (first + visible < GEAR_COUNT) bmpTextShadow(x0 + w - 40, y1 - h + 40, "v", COL_WHITE);
}

static void drawTutorial(float W, float H) {
    if (!g_tutorial || g_tutStep >= TUT_COUNT) return;
    const TutStep& s = TUT[g_tutStep];
    float w = 660, x0 = W / 2 - w / 2, y1 = H - 118, y0 = y1 - 150;
    panelBox(x0, y0, x0 + w, y1, 0.72f);
    roundRect(x0, y1 - 5, x0 + w, y1, 2, 0.35f, 0.85f, 1.0f, 1.0f);
    char buf[64];
    snprintf(buf, sizeof(buf), "TUTORIAL  %d / %d", g_tutStep + 1, TUT_COUNT);
    bmpTextShadow(x0 + 18, y1 - 24, buf, C(0.4f, 0.85f, 1.0f), GLUT_BITMAP_HELVETICA_12);
    bmpTextShadow(x0 + w - 110, y1 - 24, "T  skip step", C(0.6f, 0.6f, 0.65f), GLUT_BITMAP_HELVETICA_12);
    // progress dots
    for (int i = 0; i < TUT_COUNT; i++) {
        float dx = x0 + 150 + (float)i * 14;
        bool done = i < g_tutStep || (i == g_tutStep && g_tutDoneT >= 0);
        roundRect(dx, y1 - 22, dx + 9, y1 - 13, 4, done ? 0.4f : 0.3f, done ? 0.95f : 0.3f, done ? 0.6f : 0.34f, 1.0f);
    }
    strokeText2D(s.title, x0 + 18, y1 - 60, 28, 0, g_tutDoneT >= 0 ? C(0.4f, 1.0f, 0.6f) : C(1.0f, 0.8f, 0.3f), 1.0f);
    bmpTextShadow(x0 + 18, y1 - 84, s.line1, COL_WHITE, GLUT_BITMAP_HELVETICA_12);
    bmpTextShadow(x0 + 18, y1 - 102, s.line2, C(0.75f, 0.85f, 0.95f), GLUT_BITMAP_HELVETICA_12);
    // key caps
    float kx = x0 + 18;
    std::string keys = s.keys;
    size_t start = 0;
    while (start < keys.size()) {
        size_t sp = keys.find(' ', start);
        std::string k = keys.substr(start, sp == std::string::npos ? std::string::npos : sp - start);
        kx += drawKeyCap(kx, y0 + 12, k.c_str()) + 8;
        if (sp == std::string::npos) break;
        start = sp + 1;
    }
    if (s.unlockLevel > 0 && !D().allTricks && !has(s.unlockLevel)) {
        snprintf(buf, sizeof(buf), "Practice only for now: the story opens this at chapter %d", s.unlockLevel);
        bmpTextShadow(kx + 10, y0 + 20, buf, C(1.0f, 0.75f, 0.45f), GLUT_BITMAP_HELVETICA_12);
    }
    // live readouts for the timed steps
    std::string live;
    if (s.kind == TS_SPEED) live = std::to_string((int)(hspeed() * 3.6f)) + " / 15 km/h";
    if (s.kind == TS_GRIND && P.state == P_GRIND) live = fmtSecs(std::min(P.railTime, s.param)) + " / 1 sec";
    if (s.kind == TS_MANUAL && P.manual) live = fmtSecs(std::min(P.manualTime, s.param)) + " / 1.5 sec";
    if (s.kind == TS_POP && P.state == P_AIR) live = fmtSecs(std::max(0.0f, P.pos.y - P.takeoff.y)) + " m";
    if (!live.empty()) strokeText2D(live.c_str(), x0 + w - 18, y0 + 16, 18, 1.0f, C(0.4f, 0.9f, 1.0f), 1.0f);
}

static void menuRow(float x, float y, float w, bool sel, const std::string& label, const std::string& value) {
    if (sel) {
        roundRect(x - 14, y - 12, x + w, y + 30, 8, 1.0f, 0.75f, 0.25f, 0.18f);
        roundRect(x - 14, y - 12, x - 9, y + 30, 2, 1.0f, 0.75f, 0.25f, 1.0f);
    }
    strokeText2D(label.c_str(), x, y, 20, 0, sel ? C(1.0f, 0.85f, 0.4f) : COL_WHITE, 1.0f, false);
    if (!value.empty()) {
        std::string v = sel ? "<  " + value + "  >" : value;
        strokeText2D(v.c_str(), x + w - 20, y, 18, 1.0f, sel ? C(1.0f, 0.85f, 0.4f) : C(0.75f, 0.8f, 0.9f), 1.0f, false);
    }
}

static void drawTitle(float W, float H) {
    // dark band on the left, the city stays visible on the right
    glBegin(GL_QUADS);
    glColor4f(0.02f, 0.03f, 0.06f, 0.82f); glVertex2f(0, 0); glColor4f(0.02f, 0.03f, 0.06f, 0.0f); glVertex2f(W * 0.62f, 0);
    glVertex2f(W * 0.62f, H); glColor4f(0.02f, 0.03f, 0.06f, 0.82f); glVertex2f(0, H);
    glEnd();
    float x = 70, y = H - 150;
    strokeText2D("MUMBAI", x, y, 70, 0, C(1.0f, 0.62f, 0.15f), 1.0f);
    strokeText2D("SKATE", x + 8, y - 78, 70, 0, C(1.0f, 0.85f, 0.3f), 1.0f);
    strokeText2D("AAMCHI GALLI, AAMCHA SKATEPARK", x + 4, y - 112, 16, 0, C(0.8f, 0.9f, 1.0f), 1.0f, false);
    float my = y - 175, w = 420;
    int row;
    for (int i = 0; i < titleItems(); i++) {
        row = titleRow(i);
        std::string label, value;
        if (row == 0) {
            if (!g_hasProgress) label = "START";
            else label = g_unlockLevel >= (int)g_chapters.size() ? "CONTINUE  (LEGEND)"
                                                                 : "CONTINUE  (CHAPTER " + std::to_string(g_unlockLevel + 1) + ")";
        }
        if (row == ROW_DIFF) { label = "DIFFICULTY"; value = DIFFS[g_diff].name; }
        if (row == ROW_GFX) { label = "GRAPHICS"; value = GFXS[g_gfx].name; }
        if (row == ROW_TIME) { label = "TIME OF DAY"; value = TOD_MODE_NAMES[g_todMode]; }
        if (row == ROW_TUTORIAL) { label = "TUTORIAL"; value = g_tutorialChoice ? "ON" : "OFF"; }
        if (row == ROW_NEWGAME) label = g_newGameArmed ? "NEW GAME: PRESS ENTER AGAIN TO ERASE" : "NEW GAME";
        if (row == ROW_QUIT) label = "QUIT";
        menuRow(x, my, w, i == g_menuSel, label, value);
        my -= 46;
    }
    int selRow = titleRow(g_menuSel);
    const char* blurb = selRow == ROW_GFX ? GFXS[g_gfx].blurb : (selRow == ROW_TIME ? TOD_BLURBS[g_todMode] : DIFFS[g_diff].blurb);
    bmpTextShadow(x, my + 6, blurb, C(0.7f, 0.78f, 0.9f), GLUT_BITMAP_HELVETICA_12);
    if (g_hasProgress) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Saved: score %s    RS %d    crew %d/4    %d deliveries", withCommas(g_score).c_str(),
                 g_rupees, friendCount(), g_deliveries);
        bmpTextShadow(x, my - 14, buf, C(0.6f, 0.9f, 0.7f), GLUT_BITMAP_HELVETICA_12);
    }
    bmpTextShadow(x, 40, "W / S  choose      A / D  change      ENTER  select      ESC  quit", C(0.75f, 0.75f, 0.8f),
                  GLUT_BITMAP_HELVETICA_12);
}

static void drawPause(float W, float H) {
    panelBox(0, 0, W, H, 0.45f);
    float w = 480, h = 384, x0 = W / 2 - w / 2, y0 = H / 2 - h / 2;
    panelBox(x0, y0, x0 + w, y0 + h, 0.85f);
    roundRect(x0, y0 + h - 5, x0 + w, y0 + h, 2, 1.0f, 0.75f, 0.25f, 1.0f);
    strokeText2D("PAUSED", W / 2, y0 + h - 52, 36, 0.5f, COL_WHITE, 1.0f);
    const char* labels[6] = {"RESUME", "DIFFICULTY", "GRAPHICS", "TIME OF DAY", g_tutorial ? "SKIP TUTORIAL" : "PLAY TUTORIAL", "QUIT"};
    float y = y0 + h - 110;
    for (int i = 0; i < 6; i++) {
        std::string value = i == 1 ? DIFFS[g_diff].name : (i == 2 ? GFXS[g_gfx].name : (i == 3 ? TOD_MODE_NAMES[g_todMode] : ""));
        menuRow(x0 + 40, y, w - 60, g_menuSel == i, labels[i], value);
        y -= 42;
    }
    const char* blurb = g_menuSel == 2 ? GFXS[g_gfx].blurb : (g_menuSel == 3 ? TOD_BLURBS[g_todMode] : DIFFS[g_diff].blurb);
    bmpTextShadow(x0 + 40, y0 + 18, blurb, C(0.7f, 0.78f, 0.9f), GLUT_BITMAP_HELVETICA_10);
}

struct HelpLine { const char* text; int need; };
static const HelpLine HELP[] = {
    {"W / Up  push       S / Down  brake (at speed: powerslide)", 0},
    {"A D / arrows  turn   (air: spin, rail: balance)", 0},
    {"SPACE  ollie. Hold to crouch, let go to pop higher", 0},
    {"J  kickflip   (tap again mid-flip: double, triple)", 0},
    {"K  heelflip    L  pop shove-it   (mix for varials)", UL_HEEL},
    {"   J / K / L on a rail: nosegrind, 5-0, crooked", UL_HEEL},
    {"I  indy grab    U  melon grab   (hold, let go to land)", UL_GRAB},
    {"N  manual    M  nose manual   (W / S to balance)", UL_MANUAL},
    {"B  Bombay Backflip in the air (needs a full chai meter)", UL_CHAI},
    {"land, then powerslide, manual or ollie to keep a combo going", 0},
    {"V  boost (uses energy)    F  eat or drink at a stall", 0},
    {"E  talk / accept   Q  decline or quit a mission", 0},
    {"TAB  next task    G  hint for the task    T  skip tutorial step", 0},
    {"C camera   R respawn   H hide this   P / Esc menu", 0},
};

static void drawHelp() {
    int n = (int)(sizeof(HELP) / sizeof(HELP[0]));
    float x0 = 12, y0 = 12, lh = 16;
    panelBox(x0, y0, x0 + 400, y0 + lh * (float)(n + 1) + 14, 0.5f);
    bmpTextShadow(x0 + 10, y0 + 8 + lh * (float)n, "CONTROLS", C(1.0f, 0.8f, 0.3f), GLUT_BITMAP_HELVETICA_12);
    for (int i = 0; i < n; i++) {
        bool locked = !trickOpen(HELP[i].need);
        std::string t = HELP[i].text;
        if (locked) t += "   [CH " + std::to_string(HELP[i].need) + "]";
        bmpTextShadow(x0 + 10, y0 + 8 + lh * (float)(n - 1 - i), t.c_str(),
                      locked ? C(0.45f, 0.45f, 0.48f) : COL_WHITE, GLUT_BITMAP_HELVETICA_12);
    }
}

static void drawHUD() {
    hudBegin();
    float W = (float)g_winW, H = (float)g_winH;
    vignette(W, H);
    if (g_menu == MENU_TITLE) {
        drawTitle(W, H);
        hudEnd();
        return;
    }
    // score, money and meters
    float boxBottom = H - 162 - (trickOpen(UL_CHAI) ? 20.0f : 0.0f);
    panelBox(12, boxBottom, 310, H - 12, 0.35f);
    bmpTextShadow(24, H - 34, "SCORE", C(1.0f, 0.85f, 0.3f), GLUT_BITMAP_HELVETICA_12);
    strokeText2D(withCommas(g_score).c_str(), 24, H - 72, 30, 0, COL_WHITE, 1.0f);
    std::string best = "BEST COMBO  " + withCommas(g_bestCombo);
    bmpTextShadow(24, H - 94, best.c_str(), C(0.85f, 0.85f, 0.85f), GLUT_BITMAP_HELVETICA_12);
    std::string money = "RS " + std::to_string(g_rupees);
    strokeText2D(money.c_str(), 24, H - 114, 14, 0, C(0.5f, 1.0f, 0.6f), 1.0f, false);
    if (has(UL_MANUAL)) {
        const char* word = "CHAI";
        for (int i = 0; i < 4; i++) {
            bool got = false;
            for (size_t k = 0; k < g_letters.size(); k++) if (g_letters[k].ch == word[i] && g_letters[k].got) got = true;
            float x = 200 + (float)i * 24;
            rect2D(x, H - 118, x + 20, H - 96, got ? 1.0f : 0.25f, got ? 0.75f : 0.25f, got ? 0.2f : 0.25f, 0.9f);
            char s[2] = {word[i], 0};
            bmpTextShadow(x + 5, H - 112, s, got ? COL_BLACK : C(0.6f, 0.6f, 0.6f), GLUT_BITMAP_HELVETICA_12);
        }
    }
    {
        // energy: green when fresh, red when tired, orange while boosting
        float e = clampf(g_energy / 100.0f, 0, 1);
        roundRect(24, H - 136, 298, H - 124, 4, 0.12f, 0.12f, 0.14f, 0.85f);
        Col ec = g_boostT > 0 ? C(1.0f, 0.6f, 0.15f) : (tired() ? C(0.9f, 0.25f, 0.2f) : mixc(C(0.9f, 0.75f, 0.2f), C(0.3f, 0.85f, 0.4f), e));
        if (e > 0.01f) roundRect(24, H - 136, 24 + 274 * e, H - 124, 4, ec.r, ec.g, ec.b, 1.0f);
        rect2D(24 + 274 * (ENERGY_REST_CAP / 100.0f), H - 136, 25 + 274 * (ENERGY_REST_CAP / 100.0f), H - 124, 1, 1, 1, 0.35f);
        char eb[64];
        snprintf(eb, sizeof(eb), "%s %d%s", g_boostT > 0 ? "BOOST!" : "ENERGY", (int)g_energy,
                 tired() ? "   TIRED: EAT AT A STALL (F)" : (g_energy >= boostCost() ? "   V BOOST" : ""));
        bmpTextShadow(28, H - 134, eb, COL_WHITE, GLUT_BITMAP_HELVETICA_10);
    }
    {
        // skill level and progress to the next one
        int lv = skillLevel();
        float y0 = trickOpen(UL_CHAI) ? H - 176 : H - 156;
        float f = lv >= MAX_SKILL ? 1.0f
                                  : (float)(g_xp - xpForLevel(lv)) / (float)std::max(1L, xpForLevel(lv + 1) - xpForLevel(lv));
        roundRect(24, y0, 298, y0 + 12, 4, 0.12f, 0.12f, 0.14f, 0.85f);
        if (f > 0.01f) roundRect(24, y0, 24 + 274 * clampf(f, 0, 1), y0 + 12, 4, 0.35f, 0.6f, 1.0f, 1.0f);
        char sb[64];
        if (lv >= MAX_SKILL) snprintf(sb, sizeof(sb), "SKILL LV %d  (MAX)", lv);
        else snprintf(sb, sizeof(sb), "SKILL LV %d   %ld / %ld XP", lv, g_xp - xpForLevel(lv), xpForLevel(lv + 1) - xpForLevel(lv));
        bmpTextShadow(28, y0 + 2, sb, COL_WHITE, GLUT_BITMAP_HELVETICA_10);
    }
    if (trickOpen(UL_CHAI)) {
        float full = clampf(g_chai, 0, 1);
        roundRect(24, H - 156, 298, H - 144, 4, 0.12f, 0.12f, 0.14f, 0.85f);
        float glow = chaiFull() ? 0.8f + 0.2f * sinf(g_time * 5.0f) : 0.8f;
        if (full > 0.01f) roundRect(24, H - 156, 24 + 274 * full, H - 144, 4, 0.85f * glow, 0.5f * glow, 0.18f * glow, 1.0f);
        bmpTextShadow(28, H - 154, chaiFull() ? "CHAI POWER FULL  (B IN THE AIR)" : "CHAI POWER", COL_WHITE,
                      GLUT_BITMAP_HELVETICA_10);
    }
    // speed and stance
    char buf[128];
    float along = dot(P.vel, yawDir(P.yaw));
    snprintf(buf, sizeof(buf), "%d km/h", (int)(hspeed() * 3.6f + 0.5f));
    strokeText2D(buf, W - 24, H - 50, 26, 1.0f, COL_WHITE, 1.0f);
    const char* stance = P.state == P_GRIND ? "GRINDING" : (P.state == P_AIR ? "AIR" : (P.state == P_BAIL ? "" : (along < -0.5f ? "FAKIE" : "REGULAR")));
    int sw = bmpWidth(stance, GLUT_BITMAP_HELVETICA_12);
    bmpTextShadow(W - 24 - (float)sw, H - 72, stance, C(0.8f, 0.9f, 1.0f), GLUT_BITMAP_HELVETICA_12);
    const char* camNames[3] = {"CAM: CLOSE", "CAM: WIDE", "CAM: LOW"};
    sw = bmpWidth(camNames[g_camMode], GLUT_BITMAP_HELVETICA_12);
    bmpTextShadow(W - 24 - (float)sw, H - 90, camNames[g_camMode], C(0.7f, 0.7f, 0.7f), GLUT_BITMAP_HELVETICA_12);
    std::string clock = clockText();
    sw = bmpWidth(clock.c_str(), GLUT_BITMAP_HELVETICA_12);
    bmpTextShadow(W - 24 - (float)sw, H - 108, clock.c_str(), C(1.0f, 0.85f, 0.55f), GLUT_BITMAP_HELVETICA_12);
    if (g_careerOn && !g_tutorial) drawTaskPanel(W, H);

    // timed chapter clock, then the direction arrow
    Chapter* ch = curChapter();
    float arrowY = H - 60;
    if (ch && ch->timeLimit > 0 && g_card == CARD_NONE) {
        std::string t = mmss(g_chTimer);
        strokeText2D(t.c_str(), W / 2, H - 52, 34, 0.5f, g_chTimer < 20 ? C(1.0f, 0.4f, 0.3f) : COL_WHITE, 1.0f);
        arrowY = H - 110;
    }
    if (g_careerOn && g_card == CARD_NONE) drawObjectiveArrow(W / 2, arrowY);

    // live combo
    if (!g_combo.list.empty()) {
        std::string names;
        for (size_t i = 0; i < g_combo.list.size(); i++) {
            if (i) names += " + ";
            names += g_combo.list[i].name;
        }
        if (names.size() > 90) names = "... " + names.substr(names.size() - 86);
        int nw = bmpWidth(names.c_str());
        panelBox(W / 2 - (float)nw / 2 - 12, 58, W / 2 + (float)nw / 2 + 12, 140, 0.4f);
        bmpTextShadow(W / 2 - (float)nw / 2, 118, names.c_str(), COL_WHITE);
        snprintf(buf, sizeof(buf), "%s  x  %d", withCommas((long)comboBase()).c_str(), comboMult());
        strokeText2D(buf, W / 2, 70, 34, 0.5f, C(1.0f, 0.85f, 0.2f), 1.0f);
        if (P.state == P_GROUND && !P.manual && g_combo.grace > 0) {
            float f = g_combo.grace / comboGrace();
            rect2D(W / 2 - 100, 62, W / 2 - 100 + 200 * f, 66, 0.4f, 0.9f, 1.0f, 0.9f);
        }
    }
    if (P.state == P_GRIND) drawBalanceMeter(160, "BALANCE  (A / D)");
    if (P.state == P_GROUND && P.manual) drawBalanceMeter(160, "MANUAL BALANCE  (W / S)");
    // ollie charge
    if (P.ollieCharge > 0.05f && P.state != P_AIR && P.state != P_BAIL) {
        rect2D(W / 2 - 60, 32, W / 2 + 60, 40, 0, 0, 0, 0.5f);
        rect2D(W / 2 - 60, 32, W / 2 - 60 + 120 * P.ollieCharge, 40, 0.4f, 0.9f, 1.0f, 0.9f);
        bmpTextShadow(W / 2 - 14, 44, "POP", COL_WHITE, GLUT_BITMAP_HELVETICA_10);
    }
    drawTalkUI(W);

    // popups rise and fade
    for (size_t i = 0; i < g_popups.size(); i++) {
        const Popup& p = g_popups[i];
        float a = p.t > p.life * 0.7f ? 1.0f - (p.t - p.life * 0.7f) / (p.life * 0.3f) : 1.0f;
        float s = p.size * (p.t < 0.12f ? 0.7f + p.t * 2.5f : 1.0f);
        strokeText2D(p.text.c_str(), W / 2, H * (p.y + p.t * 0.03f), s, 0.5f, p.col, clampf(a, 0, 1));
    }
    // mission banner
    if (g_banner.life > 0) {
        float a = clampf(std::min(g_banner.t * 4.0f, (g_banner.life - g_banner.t) * 2.0f), 0, 1);
        // keep clear of the task panel on the right
        float maxW = 2.0f * (W - 394.0f - W / 2);
        strokeFit2D(g_banner.title.c_str(), W / 2, H * 0.74f, 34, maxW, 0.5f, g_banner.col, a);
        strokeFit2D(g_banner.sub.c_str(), W / 2, H * 0.70f, 16, maxW, 0.5f, COL_WHITE, a);
    }
    drawCards(W, H);
    drawTutorial(W, H);
    drawHintPanel(W, H);
    drawBoostLines(W, H);
    if (g_time < 6.0f && g_card == CARD_INTRO)
        strokeText2D("MUMBAI SKATE", W / 2, H * 0.93f, 30, 0.5f, C(1.0f, 0.55f, 0.2f), clampf(6.0f - g_time, 0, 1));

    if (g_showHelp) drawHelp();
    else {
        roundRect(8, 8, 136, 30, 5, 0.04f, 0.05f, 0.08f, 0.5f);
        bmpTextShadow(16, 14, "H  show controls", C(0.8f, 0.8f, 0.8f), GLUT_BITMAP_HELVETICA_12);
    }
    if (g_shopOpen) drawShop(W, H);
    if (g_menu == MENU_PAUSE) drawPause(W, H);
    hudEnd();
}

static float g_fov = 64.0f;

static void render() {
    computeSky();
    Col fog = g_sky.fog;
    glClearColor(fog.r, fog.g, fog.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    drawSky();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    g_fov = lerpf(g_fov, 64.0f + clampf(hspeed() - 6.0f, 0, 14) * 0.7f + (g_boostT > 0 ? 8.0f : 0.0f), 0.05f);
    float dist = G().drawDist;
    gluPerspective(g_fov, (double)g_winW / (double)std::max(1, g_winH), 0.2, (double)dist + 120.0);
#ifdef GL_MULTISAMPLE
    if (G().msaa) glEnable(GL_MULTISAMPLE);
    else glDisable(GL_MULTISAMPLE);
#endif
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    V3 shake(sinf(g_time * 47.0f), sinf(g_time * 39.0f + 1.0f), sinf(g_time * 53.0f + 2.0f));
    shake *= g_shake * 0.25f;
    V3 eye = g_camPos + shake;
    gluLookAt(eye.x, eye.y, eye.z, g_camLook.x, g_camLook.y, g_camLook.z, 0, 1, 0);
    g_frustum = currentFrustum();
    GLfloat lp[4] = {SUN_DIR.x, SUN_DIR.y, SUN_DIR.z, 0.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lp);
    GLfloat fp[4] = {0.5f, 0.35f, -0.6f, 0.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, fp);
    GLfloat dif[4] = {g_sky.sun.r, g_sky.sun.g, g_sky.sun.b + 0.05f * g_rain, 1};
    GLfloat amb[4] = {g_sky.amb.r, g_sky.amb.g, g_sky.amb.b, 1};
    GLfloat fill[4] = {g_sky.fill.r, g_sky.fill.g, g_sky.fill.b, 1};
    GLfloat gamb[4] = {g_sky.gamb.r, g_sky.gamb.g, g_sky.gamb.b, 1};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fill);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gamb);
    GLfloat fogc[4] = {fog.r, fog.g, fog.b, 1};
    glFogfv(GL_FOG_COLOR, fogc);
    // fog closes in before the draw distance so tiles never pop in visibly
    float fogEnd = std::min(290.0f - 150.0f * g_rain, dist * 0.95f);
    glFogf(GL_FOG_START, fogEnd * 0.25f);
    glFogf(GL_FOG_END, fogEnd);
    glEnable(GL_FOG);
    drawSun();
    gLighting(true);
    g_unlitShade = g_sky.night > 0.02f;
    g_drawnVerts = 0;
    drawChunks(g_worldChunks, g_frustum, g_camPos, dist);
    drawDynamicOpaque();
    if (G().worldShadows || G().dynShadows > 0) drawShadowPass();
    drawNightLights();
    drawDynamicTransparent();
    g_unlitShade = false;
    if (g_cut != CUT_NONE) {
        hudBegin();
        drawCutsceneOverlay((float)g_winW, (float)g_winH);
        hudEnd();
    } else {
        drawHUD();
    }
}
