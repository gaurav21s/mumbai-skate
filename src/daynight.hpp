#pragma once
// Part of Mumbai Skate. Included once, in order, from main.cpp.

// ============================================================================
// Time of day
//
// A clock runs while you skate: one game hour takes 90 real seconds, so a
// chapter usually sees the light change, and long sessions run into the
// evening and the night. Each chapter starts at its own hour (the finale is
// always at night), and the clock fast-forwards behind the chapter card.
// The TIME row in the menus can also pin it to day, evening or night.
//
// The sun keeps one direction all day because the city's shadows are baked
// for it. Only its colour and strength change, and at night it becomes a
// faint moonlight from the same side.
// ============================================================================

enum TodMode { TOD_AUTO, TOD_DAY, TOD_EVENING, TOD_NIGHT, TOD_MODES };
static const char* TOD_MODE_NAMES[TOD_MODES] = {"AUTO", "DAY", "EVENING", "NIGHT"};
static const float TOD_FIXED_HOUR[TOD_MODES] = {0.0f, 11.0f, 18.2f, 21.5f};
static const float TOD_HOURS_PER_SEC = 1.0f / 90.0f;
static const float TOD_FAST = 3.0f;  // hours per second while fast-forwarding
// the hour each chapter starts at; the last entry is free skate after the story
static const float CHAPTER_HOUR[7] = {8.5f, 11.0f, 14.5f, 17.0f, 19.0f, 21.0f, 20.0f};

static int g_todMode = TOD_AUTO;
static float g_tod = 8.5f;          // hours, 0..24
static float g_todTarget = -1.0f;   // fast-forward to this hour, or -1
static float g_skyHourOverride = -1.0f;  // cutscenes pick their own hour

// Everything the renderer needs for the current hour and weather.
struct Sky {
    Col sun, amb, fill, gamb;    // light colours
    Col low, mid, top, fog;      // sky gradient and fog
    Col sunDisc;                 // colour of the sun sprite
    float night;                 // 0 full day, 1 full night
    float lights;                // how bright the city's night lights are
    float stars, moon, sunA;     // sprite strengths
    float shadow;                // strength of sun shadows
};
static Sky g_sky;

struct SkyKey {
    float h;
    Col sun, amb, low, mid, top, fog, disc;
    float night;
};
static const SkyKey SKY_NIGHT = {0, {0.2f, 0.23f, 0.37f}, {0.16f, 0.17f, 0.26f}, {0.16f, 0.12f, 0.16f},
                                 {0.06f, 0.07f, 0.15f}, {0.02f, 0.03f, 0.09f}, {0.1f, 0.09f, 0.15f},
                                 {0.9f, 0.92f, 1.0f}, 1.0f};
static const SkyKey SKY_KEYS[] = {
    {0.0f, SKY_NIGHT.sun, SKY_NIGHT.amb, SKY_NIGHT.low, SKY_NIGHT.mid, SKY_NIGHT.top, SKY_NIGHT.fog, SKY_NIGHT.disc, 1.0f},
    {4.8f, SKY_NIGHT.sun, SKY_NIGHT.amb, SKY_NIGHT.low, SKY_NIGHT.mid, SKY_NIGHT.top, SKY_NIGHT.fog, SKY_NIGHT.disc, 1.0f},
    // dawn
    {6.0f, {0.72f, 0.52f, 0.45f}, {0.32f, 0.29f, 0.33f}, {0.95f, 0.66f, 0.55f}, {0.7f, 0.6f, 0.66f},
     {0.3f, 0.4f, 0.66f}, {0.74f, 0.62f, 0.6f}, {1.0f, 0.7f, 0.5f}, 0.5f},
    // day
    {7.6f, {0.82f, 0.77f, 0.66f}, {0.45f, 0.42f, 0.4f}, {0.93f, 0.84f, 0.7f}, {0.9f, 0.83f, 0.72f},
     {0.4f, 0.62f, 0.86f}, {0.9f, 0.83f, 0.72f}, {1.0f, 0.9f, 0.7f}, 0.0f},
    {16.2f, {0.82f, 0.77f, 0.66f}, {0.45f, 0.42f, 0.4f}, {0.93f, 0.84f, 0.7f}, {0.9f, 0.83f, 0.72f},
     {0.4f, 0.62f, 0.86f}, {0.9f, 0.83f, 0.72f}, {1.0f, 0.9f, 0.7f}, 0.0f},
    // golden hour
    {17.4f, {0.95f, 0.68f, 0.42f}, {0.42f, 0.36f, 0.35f}, {0.98f, 0.72f, 0.46f}, {0.94f, 0.7f, 0.55f},
     {0.42f, 0.52f, 0.78f}, {0.92f, 0.7f, 0.53f}, {1.0f, 0.62f, 0.3f}, 0.12f},
    // sunset
    {18.4f, {0.76f, 0.42f, 0.3f}, {0.34f, 0.27f, 0.32f}, {0.98f, 0.5f, 0.28f}, {0.76f, 0.43f, 0.47f},
     {0.24f, 0.26f, 0.52f}, {0.64f, 0.42f, 0.42f}, {1.0f, 0.45f, 0.2f}, 0.45f},
    // dusk
    {19.2f, {0.34f, 0.3f, 0.44f}, {0.24f, 0.22f, 0.32f}, {0.46f, 0.3f, 0.4f}, {0.24f, 0.2f, 0.38f},
     {0.08f, 0.1f, 0.26f}, {0.24f, 0.2f, 0.32f}, {0.95f, 0.5f, 0.3f}, 0.8f},
    {20.0f, SKY_NIGHT.sun, SKY_NIGHT.amb, SKY_NIGHT.low, SKY_NIGHT.mid, SKY_NIGHT.top, SKY_NIGHT.fog, SKY_NIGHT.disc, 1.0f},
    {24.0f, SKY_NIGHT.sun, SKY_NIGHT.amb, SKY_NIGHT.low, SKY_NIGHT.mid, SKY_NIGHT.top, SKY_NIGHT.fog, SKY_NIGHT.disc, 1.0f},
};
static const int SKY_KEY_COUNT = (int)(sizeof(SKY_KEYS) / sizeof(SKY_KEYS[0]));

static float wrapHour(float h) {
    while (h >= 24.0f) h -= 24.0f;
    while (h < 0.0f) h += 24.0f;
    return h;
}

// The hour the sky actually shows: the clock, or the pinned hour.
static float shownHour() {
    if (g_skyHourOverride >= 0) return g_skyHourOverride;
    return g_todMode == TOD_AUTO ? g_tod : TOD_FIXED_HOUR[g_todMode];
}

static void computeSky() {
    float h = wrapHour(shownHour());
    int k = 0;
    while (k + 1 < SKY_KEY_COUNT - 1 && SKY_KEYS[k + 1].h <= h) k++;
    const SkyKey& a = SKY_KEYS[k];
    const SkyKey& b = SKY_KEYS[k + 1];
    float t = clampf((h - a.h) / std::max(0.01f, b.h - a.h), 0, 1);
    t = t * t * (3.0f - 2.0f * t);
    Sky s;
    s.sun = mixc(a.sun, b.sun, t);
    s.amb = mixc(a.amb, b.amb, t);
    s.low = mixc(a.low, b.low, t);
    s.mid = mixc(a.mid, b.mid, t);
    s.top = mixc(a.top, b.top, t);
    s.fog = mixc(a.fog, b.fog, t);
    s.sunDisc = mixc(a.disc, b.disc, t);
    s.night = lerpf(a.night, b.night, t);
    float day = 1.0f - s.night;
    s.fill = mulc(C(0.16f, 0.19f, 0.26f), 0.3f + 0.7f * day);
    s.gamb = mixc(C(0.05f, 0.05f, 0.08f), C(0.12f, 0.12f, 0.14f), day);
    // the monsoon greys everything out, and darker after sunset
    float r = g_rain;
    float wetK = 0.18f + 0.82f * day;
    s.fog = mixc(s.fog, mulc(C(0.52f, 0.56f, 0.6f), wetK), r);
    s.low = mixc(s.low, mulc(C(0.5f, 0.54f, 0.58f), wetK), r);
    s.mid = mixc(s.mid, mulc(C(0.52f, 0.56f, 0.6f), wetK), r);
    s.top = mixc(s.top, mulc(C(0.36f, 0.4f, 0.46f), wetK), r);
    s.sun = mulc(s.sun, 1.0f - 0.35f * r);
    s.lights = clampf((s.night - 0.15f) / 0.55f, 0, 1);
    s.stars = clampf((s.night - 0.6f) / 0.4f, 0, 1) * (1.0f - r);
    s.moon = clampf((s.night - 0.5f) * 2.0f, 0, 1) * (1.0f - r);
    s.sunA = clampf(1.0f - s.night * 1.6f, 0, 1) * (1.0f - clampf((r - 0.3f) * 3.0f, 0, 1));
    s.shadow = (1.0f - 0.6f * s.night) * (1.0f - 0.7f * r);
    g_sky = s;
}

// Sets the clock for a chapter. With `fast`, the sky time-lapses there instead.
static void todForChapter(int level, bool fast) {
    float h = CHAPTER_HOUR[std::min(std::max(level, 0), 6)];
    if (fast) g_todTarget = h;
    else {
        g_tod = h;
        g_todTarget = -1.0f;
    }
}

// Advances the clock. finale: the last chapter, which stays at night.
static void todUpdate(float dt, bool finale) {
    if (g_todTarget >= 0) {
        float left = wrapHour(g_todTarget - g_tod);
        float step = TOD_FAST * dt;
        if (left <= step || left > 23.9f) {
            g_tod = g_todTarget;
            g_todTarget = -1.0f;
        } else {
            g_tod = wrapHour(g_tod + step);
        }
        return;
    }
    g_tod = wrapHour(g_tod + TOD_HOURS_PER_SEC * dt);
    if (finale && g_tod > 5.0f && g_tod < 20.5f) g_tod = 20.5f;
    if (finale && g_tod > 23.6f) g_tod = 23.6f;
}

static std::string clockText() {
    int mins = (int)(wrapHour(shownHour()) * 60.0f);
    int hh = mins / 60, mm = mins % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%02d %s", hh % 12 == 0 ? 12 : hh % 12, mm, hh < 12 ? "AM" : "PM");
    return buf;
}
