/*
 * MUMBAI SKATE
 * A 3D skateboarding game set on a Mumbai street block.
 *
 * Everything is generated in code: no textures, models, fonts or sound files.
 * Rendering uses fixed-function OpenGL and GLUT. GLUT ships with macOS; on
 * Linux and Windows use freeglut.
 *
 * The source is split by topic into headers that this file includes once, in
 * order, so the whole game still compiles as one translation unit:
 *   core.hpp          math, colours, geometry batching, primitives, vector font
 *   collision.hpp     solids, ramps, rails, barricades
 *   actors.hpp        vehicles, trains, pedestrians, stalls and delivery doors
 *   city_*.hpp        the block itself: streets, shops, plaza, railway, site
 *   scoring.hpp       popups, combos, input state
 *   player.hpp        skater state and tuning
 *   traffic.hpp       traffic lights, cars, buses, trains, pedestrians
 *   city_seaface.hpp  the lane to the sea, the promenade and the Sea Link
 *   fx.hpp            particles, camera shake, rain
 *   daynight.hpp      the clock, sky colours and light for each hour
 *   career.hpp        chapters, tasks, gaps, unlocks, gear, save file
 *   missions.hpp      friends, side missions, deliveries, the skate shop
 *   tutorial.hpp      the tutorial, title screen and pause menu
 *   shadows.hpp       sun shadows baked for the city and projected for moving things
 *   physics.hpp       riding, jumping, tricks, grinding, landing
 *   render.hpp        skater, friends, markers and world drawing, camera
 *   nightlights.hpp   lit windows, lamps, headlights, festival lights, fireworks
 *   cutscene.hpp      the opening and ending films
 *   hud.hpp           everything drawn on top of the 3D view
 *   selftest.hpp      scripted checks run by --selftest
 *
 * Build
 *   make                (or run one of the lines below from this folder)
 *   macOS:   clang++ -std=c++17 -O2 src/main.cpp -o mumbai_skate -framework OpenGL -framework GLUT
 *   Linux:   g++ -std=c++17 -O2 src/main.cpp -o mumbai_skate -lglut -lGLU -lGL
 *   Windows: g++ -std=c++17 -O2 src/main.cpp -o mumbai_skate.exe -lfreeglut -lglu32 -lopengl32
 *
 * Run
 *   ./mumbai_skate                progress saves to ~/.mumbai_skate_save
 *   ./mumbai_skate --newgame      skip the saved game (the title screen also has NEW GAME)
 *
 * Developer options (none of these touch the save file, and all skip the title screen)
 *   --selftest              run scripted physics, career and mission checks, print results, quit
 *   --bench                 time the three graphics presets from fixed views, print, quit
 *   --level N               start with N chapters finished
 *   --unlockall             start as a legend with everything open
 *   --shot out.ppm 3        play for 3 seconds of game time, save a PPM screenshot, quit
 *   --at x y z yawDeg       start somewhere else (y is ground height, 0.18 on sidewalks)
 *   --push                  hold W during a --shot run
 *   --clean                 hide the controls panel (for screenshots)
 *   --graphics low|medium|high   pick the graphics preset for this run (also in the menus)
 *   --time 21.5             start the clock at this hour (0 to 24)
 *   --level 5 --stop 6      start the finale at one of its six stops
 *   --cutscene intro        play the opening film (or "ending"; use with --level 6)
 *
 * Controls are listed in the in-game panel (H hides it).
 */

#include "core.hpp"
#ifndef _WIN32
#include <unistd.h>
#endif
#include "collision.hpp"
#include "actors.hpp"
#include "city_streets.hpp"
#include "city_spots.hpp"
#include "city_seaface.hpp"
#include "scoring.hpp"
#include "player.hpp"
#include "traffic.hpp"
#include "fx.hpp"
#include "daynight.hpp"
#include "career.hpp"
#include "missions.hpp"
#include "tutorial.hpp"
#include "physics.hpp"
#include "shadows.hpp"
#include "render.hpp"
#include "nightlights.hpp"
#include "cutscene.hpp"
#include "hud.hpp"

// ============================================================================
// Simulation driver, input, GLUT glue
// ============================================================================

static Input readInput(bool first) {
    Input in;
    in.up = g_key['w'] || g_special[0];
    in.down = g_key['s'] || g_special[1];
    in.left = g_key['a'] || g_special[2];
    in.right = g_key['d'] || g_special[3];
    in.ollie = first && g_keyRel[' '];  // pops on release; holding crouches for more height
    in.ollieHeld = g_key[' '];
    in.backflip = first && g_keyHit['b'];
    in.boost = first && g_keyHit['v'];
    in.kick = first && g_keyHit['j'];
    in.heel = first && g_keyHit['k'];
    in.shove = first && g_keyHit['l'];
    in.indy = first && g_keyHit['i'];
    in.melon = first && g_keyHit['u'];
    in.manual = first && g_keyHit['n'];
    in.noseManual = first && g_keyHit['m'];
    in.indyHeld = g_key['i'];
    in.melonHeld = g_key['u'];
    return in;
}

static const float SIM_DT = 1.0f / 120.0f;

static void simStep(const Input& in) {
    updateTraffic(SIM_DT);
    updateTrains(SIM_DT);
    updatePeds(SIM_DT);
    playerStep(SIM_DT, in);
    tutorialUpdate(SIM_DT);
    careerUpdate(SIM_DT);
    missionsUpdate(SIM_DT);
    // fireworks over the sea: through the finale's show, all through the
    // legend card, and now and then on later nights at the Sea Face
    bool legend = g_card == CARD_LEGEND || (g_card == CARD_COMPLETE && g_cardChapter == FINALE_LEVEL);
    bool show = seaFaceShow() && g_sky.lights > 0.4f && P.pos.z > 40.0f;
    fireworksUpdate(SIM_DT, legend || show, legend ? 0.35f : (inFinale() ? 0.9f : 2.6f));
    updateParticles(SIM_DT);
    // steam from the chai kettles near the skater
    for (size_t i = 0; i < g_stalls.size(); i++)
        if (g_stalls[i].kind == STALL_CHAI && fxrand() < 0.05f && len(g_stalls[i].kettle - P.pos) < 50.0f)
            emit(g_stalls[i].kettle, V3(fxrange(-0.1f, 0.1f), 0.5f, fxrange(-0.1f, 0.1f)), C(0.95f, 0.95f, 0.95f), 1.6f,
                 0.12f, -0.3f, false, 0.5f);
    if (g_toastCooldown > 0) g_toastCooldown -= SIM_DT;
    for (size_t i = 0; i < g_popups.size();) {
        g_popups[i].t += SIM_DT;
        if (g_popups[i].t > g_popups[i].life) g_popups.erase(g_popups.begin() + (long)i);
        else i++;
    }
}

// R: back to the start of the current chapter. Missions stay active.
static void resetGame() {
    spawnAtChapter();
    g_camInit = false;
    updateCamera(0, true);
    g_teleport = false;
}

static int g_lastMs = -1;
static float g_accum = 0;
static const char* g_shotFile = nullptr;
static float g_shotAt = 3.0f;
static bool g_shotPush = false;

static void saveScreenshot(const char* path) {
    std::vector<unsigned char> px((size_t)g_winW * (size_t)g_winH * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadBuffer(GL_BACK);
    glReadPixels(0, 0, g_winW, g_winH, GL_RGB, GL_UNSIGNED_BYTE, px.data());
    FILE* f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", g_winW, g_winH);
    for (int y = g_winH - 1; y >= 0; y--) fwrite(&px[(size_t)y * (size_t)g_winW * 3], 1, (size_t)g_winW * 3, f);
    fclose(f);
    printf("saved %s (%dx%d)\n", path, g_winW, g_winH);
}

static int g_lastFrameMs = 0;  // when the last frame started, for the frame cap
static bool g_visible = true;

// --bench: renders fixed views as fast as possible and prints the frame time.
static bool g_bench = false;
// Vsync on (1) or off (0). Only wired up on macOS; elsewhere the driver decides.
static void setSwapInterval(int n) {
#ifdef __APPLE__
    GLint v = n;
    CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &v);
#else
    (void)n;
#endif
}

static void runBench() {
    setSwapInterval(0);
    struct Spot { V3 p; float yaw; const char* name; };
    const Spot spots[] = {{SPAWN_POS, SPAWN_YAW, "plaza"},
                          {V3(-60, 0.18f, 10), PI * 0.5f, "main road"},
                          {V3(60, 0, -30), PI, "site"},
                          {V3(0, 0.18f, 130), 0.0f, "sea face"},
                          {V3(-80, 3.4f, -70), PI * 0.5f, "skywalk"}};
    for (int gfx = GFX_LOW; gfx <= GFX_HIGH; gfx++) {
        g_gfx = gfx;
        float total = 0;
        size_t verts = 0;
        for (const Spot& s : spots) {
            resetPlayer(s.p, s.yaw);
            g_camInit = false;
            updateCamera(0, true);
            for (int i = 0; i < 5; i++) render();
            glFinish();
            int t0 = glutGet(GLUT_ELAPSED_TIME);
            const int N = 60;
            for (int i = 0; i < N; i++) {
                g_time += 1.0f / 60.0f;
                render();
                glutSwapBuffers();
            }
            glFinish();
            total += (float)(glutGet(GLUT_ELAPSED_TIME) - t0) / (float)N;
            verts += g_drawnVerts;
        }
        printf("%-6s  %6.2f ms per frame  (%3.0f fps uncapped, cap %d)   %6.0fk city vertices drawn\n", GFXS[gfx].name,
               total / 5.0f, 5000.0f / total, GFXS[gfx].fps, (float)verts / 5000.0f);
    }
    exit(0);
}

static void display() {
    if (g_bench) runBench();
    g_lastFrameMs = glutGet(GLUT_ELAPSED_TIME);
    int ms = glutGet(GLUT_ELAPSED_TIME);
    if (g_lastMs < 0) g_lastMs = ms;
    float dt = clampf((float)(ms - g_lastMs) / 1000.0f, 0.0f, 0.05f);
    g_lastMs = ms;
    if (g_shotFile && g_shotPush) g_key['w'] = true;
    if (g_quitRequested) exit(0);
    if (g_menu == MENU_TITLE) {
        // slow orbit over the plaza behind the title screen
        static float orbit = 0;
        orbit += dt * 0.08f;
        g_time += dt;
        updateTraffic(dt);
        updateTrains(dt);
        updatePeds(dt);
        updateParticles(dt);
        V3 c(-50.0f, 0, -30.0f);
        g_camPos = c + V3(sinf(orbit) * 42.0f, 13.0f, cosf(orbit) * 42.0f);
        g_camLook = c + V3(0, 2.0f, 0);
        g_camYaw = dirYaw(c.x - g_camPos.x, c.z - g_camPos.z);
    } else if (g_menu == MENU_NONE && g_cut != CUT_NONE) {
        g_time += dt;
        cutsceneStep(dt);
        if (g_cut != CUT_NONE) cutsceneCamera();
    } else if (g_menu == MENU_NONE && !g_shopOpen) {
        g_time += dt;
        g_accum += dt;
        bool first = true;
        int steps = 0;
        while (g_accum >= SIM_DT && steps < 8) {
            simStep(readInput(first));
            first = false;
            g_accum -= SIM_DT;
            steps++;
        }
        if (steps > 0) {
            memset(g_keyHit, 0, sizeof(g_keyHit));
            memset(g_keyRel, 0, sizeof(g_keyRel));
        }
        if (g_teleport) {
            g_teleport = false;
            g_camInit = false;
            updateCamera(0, true);
        }
        updateCamera(dt, false);
    }
    render();
    static int frames = 0;
    frames++;
    if (g_shotFile && g_time >= g_shotAt) {
        printf("frames %d, wall %.2fs, speed %.2f, pos %.2f %.2f %.2f state %d\n", frames, (float)ms / 1000.0f,
               hspeed(), P.pos.x, P.pos.y, P.pos.z, P.state);
        saveScreenshot(g_shotFile);
        exit(0);
    }
    glutSwapBuffers();
}

// Sleeps until the next frame is due instead of drawing flat out. Menus and
// the shop run at 20 fps; a hidden window draws nothing.
static void idle() {
    int cap = (g_menu != MENU_NONE || g_shopOpen) ? 20 : G().fps;
    if (g_shotFile || g_bench) cap = 1000;
    if (!g_visible) cap = 2;
    float target = 1000.0f / (float)cap;
    float since = (float)(glutGet(GLUT_ELAPSED_TIME) - g_lastFrameMs);
    if (since < target - 0.5f) {
        float wait = target - since - 0.5f;
#ifdef _WIN32
        Sleep((DWORD)wait);
#else
        usleep((useconds_t)(wait * 1000.0f));
#endif
    }
    if (g_visible) glutPostRedisplay();
}
static void visibility(int state) { g_visible = state == GLUT_VISIBLE; }

static void reshape(int w, int h) {
    g_winW = std::max(1, w);
    g_winH = std::max(1, h);
    glViewport(0, 0, g_winW, g_winH);
}

static void keyDown(unsigned char k, int, int) {
    unsigned char c = (unsigned char)tolower(k);
    if (g_shopOpen) {
        if (c == 'w') shopMove(-1);
        if (c == 's') shopMove(1);
        if (c == 13 || c == ' ') shopBuy();
        if (c == 'e' || c == 'q' || c == 27) g_shopOpen = false;
        return;
    }
    if (g_menu != MENU_NONE) {
        if (c == 'w') menuKey(0);
        if (c == 's') menuKey(1);
        if (c == 'a') menuKey(2);
        if (c == 'd') menuKey(3);
        if (c == 13 || c == ' ') menuKey(4);
        if (c == 27 || (c == 'p' && g_menu == MENU_PAUSE)) menuKey(5);
        return;
    }
    if (g_cut != CUT_NONE) {
        if (c == ' ' || c == 13 || c == 27) skipCutscene();
        return;
    }
    if (c == 27 || c == 'p') {
        g_menu = MENU_PAUSE;
        g_menuSel = 0;
        return;
    }
    if (!g_key[c]) g_keyHit[c] = true;
    g_key[c] = true;
    if (c == 'e') pressTalk();
    if (c == 'q') pressCancel();
    if (c == 't') tutSkip();
    if (c == 'g') pressHint();
    if (c == 'x') pressSkip();
    if (c >= '1' && c <= '3') pressJob(c - '1');
    if (c == 'f') pressFood();
    if (c == 9) cycleFocus();  // tab
    if (c == 'h') g_showHelp = !g_showHelp;
    if (c == 'c') g_camMode = (g_camMode + 1) % 3;
    if (c == 'r') resetGame();
}
static void keyUp(unsigned char k, int, int) {
    unsigned char c = (unsigned char)tolower(k);
    if (g_key[c]) g_keyRel[c] = true;
    g_key[c] = false;
}
static int specialIndex(int k) {
    switch (k) {
        case GLUT_KEY_UP: return 0;
        case GLUT_KEY_DOWN: return 1;
        case GLUT_KEY_LEFT: return 2;
        case GLUT_KEY_RIGHT: return 3;
        default: return -1;
    }
}
static void specialDown(int k, int, int) {
    int i = specialIndex(k);
    if (g_shopOpen) {
        if (i == 0) shopMove(-1);
        if (i == 1) shopMove(1);
        return;
    }
    if (g_menu != MENU_NONE) {
        if (i >= 0) menuKey(i);
        return;
    }
    if (i >= 0) g_special[i] = true;
}
static void specialUp(int k, int, int) { int i = specialIndex(k); if (i >= 0) g_special[i] = false; }

#include "selftest.hpp"

int main(int argc, char** argv) {
    bool test = false;
    V3 spawn = SPAWN_POS;
    float spawnYaw = SPAWN_YAW;
    bool customSpawn = false;
    bool newGame = false;
    int forceLevel = -1;
    int gfxArg = -1;
    float timeArg = -1.0f;
    int stopArg = 0;
    int cutArg = CUT_NONE;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--selftest")) test = true;
        else if (!strcmp(argv[i], "--bench")) g_bench = true;
        else if (!strcmp(argv[i], "--graphics") && i + 1 < argc) {
            std::string v = argv[++i];
            gfxArg = v == "low" ? GFX_LOW : (v == "high" ? GFX_HIGH : GFX_MEDIUM);
        }
        else if (!strcmp(argv[i], "--newgame")) newGame = true;
        else if (!strcmp(argv[i], "--time") && i + 1 < argc) timeArg = (float)atof(argv[++i]);
        else if (!strcmp(argv[i], "--stop") && i + 1 < argc) stopArg = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--clean")) g_showHelp = false;
        else if (!strcmp(argv[i], "--cutscene") && i + 1 < argc) cutArg = strcmp(argv[++i], "ending") ? CUT_INTRO : CUT_ENDING;
        else if (!strcmp(argv[i], "--unlockall")) forceLevel = 99;
        else if (!strcmp(argv[i], "--level") && i + 1 < argc) forceLevel = atoi(argv[++i]);
        else if (!strcmp(argv[i], "--shot") && i + 1 < argc) {
            g_shotFile = argv[++i];
            if (i + 1 < argc && argv[i + 1][0] != '-') g_shotAt = (float)atof(argv[++i]);
        } else if (!strcmp(argv[i], "--push")) g_shotPush = true;
        else if (!strcmp(argv[i], "--at") && i + 4 < argc) {
            spawn = V3((float)atof(argv[i + 1]), (float)atof(argv[i + 2]), (float)atof(argv[i + 3]));
            spawnYaw = (float)atof(argv[i + 4]) / RAD2DEG;
            customSpawn = true;
            i += 4;
        }
    }
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
    glutInitWindowSize(g_winW, g_winH);
    glutCreateWindow("Mumbai Skate");

    glEnable(GL_DEPTH_TEST);
    gLighting(true);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    GLfloat amb[4] = {0.45f, 0.42f, 0.4f, 1}, dif[4] = {0.82f, 0.77f, 0.66f, 1}, gamb[4] = {0.12f, 0.12f, 0.14f, 1};
    glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gamb);
    glEnable(GL_LIGHT1);
    GLfloat fillAmb[4] = {0, 0, 0, 1}, fillDif[4] = {0.16f, 0.19f, 0.26f, 1};
    glLightfv(GL_LIGHT1, GL_AMBIENT, fillAmb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, fillDif);
    glShadeModel(GL_SMOOTH);
    GLfloat fogc[4] = {0.9f, 0.83f, 0.72f, 1};
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogc);
    glFogf(GL_FOG_START, 70.0f);
    glFogf(GL_FOG_END, 290.0f);
#ifdef GL_MULTISAMPLE
    glEnable(GL_MULTISAMPLE);
#endif
    glLineWidth(1.5f);

    recBegin();
    buildWorld();
    bakeWorldShadows(g_recMesh);
    buildNightGlow();
    recEndChunks(g_worldChunks, 32.0f);
    if (g_shotFile || test)
        printf("world built at %.2fs, %d vertices in %d tiles, %d shadow vertices\n",
               (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f, (int)g_worldVerts, (int)g_worldChunks.size(), (int)g_shadowVerts);
    initChapters();
    initFriends();
    rollAllJobs();
    for (size_t i = 0; i < g_peds.size(); i++) buildPedLists(g_peds[i]);
    bakeGates();
    initTraffic();
    initTrains();

    if (test) {
        g_careerOn = false;
        g_unlockLevel = 99;
        return selfTest();
    }

    if (g_shotFile || forceLevel >= 0 || g_bench) g_noSave = true;
    if (g_bench) forceLevel = 3;
    if (forceLevel >= 0) {
        g_unlockLevel = std::min(forceLevel, (int)g_chapters.size());
        const int friendByChapter[4] = {1, 3, 4, 5};  // chapter whose tasks include that friend's mission
        for (int i = 0; i < 4; i++) g_friend[i] = g_unlockLevel >= friendByChapter[i];
    } else if (!newGame && !g_noSave && loadGame()) {
        printf("Loaded save from %s (chapter %d)\n", savePath().c_str(), g_unlockLevel + 1);
    }
    if (gfxArg >= 0) g_gfx = gfxArg;  // overrides the saved setting for this run
    if (forceLevel >= 0) todForChapter(g_unlockLevel, false);
    if (stopArg > 1 && forceLevel == FINALE_LEVEL) {
        // skip ahead in the finale: the stops before this one are done
        Chapter& fin = g_chapters[(size_t)FINALE_LEVEL];
        for (size_t k = 0; k < fin.goals.size(); k++)
            if (fin.goals[k].act < stopArg) fin.goals[k].have = fin.goals[k].need;
        g_act = stopArg;
        g_rain = stopArg <= 3 ? 1.0f : 0.0f;
    }
    if (timeArg >= 0) {
        g_todMode = TOD_AUTO;
        g_tod = wrapHour(timeArg);
    }
    if (!g_noSave) atexit(saveGame);
    g_hasProgress = g_unlockLevel > 0 || g_score > 0;
    g_tutorialChoice = !g_tutorialDone;
    // screenshots and --level runs go straight into the game
    if (g_shotFile || forceLevel >= 0) {
        g_card = curChapter() ? CARD_INTRO : CARD_NONE;
        g_cardT = 0;
        startChapterClock();
    } else {
        g_menu = MENU_TITLE;
        g_menuSel = 0;
    }
    resetGame();
    if (customSpawn) {
        resetPlayer(spawn, spawnYaw);
        updateCamera(0, true);
    }
    if (cutArg != CUT_NONE) {
        g_menu = MENU_NONE;
        g_card = CARD_NONE;
        startCutscene(cutArg);
    }
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutVisibilityFunc(visibility);
    setSwapInterval(1);  // never draw faster than the screen refreshes
    glutReshapeFunc(reshape);
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutMainLoop();
    return 0;
}
