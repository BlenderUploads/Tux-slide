// Tux Slide - a tiny low-poly 3D game starring Tux the Linux mascot.
//
// Fixed-function OpenGL (GLFW for window/input, GLU for the projection/
// camera helpers) so there are no shaders or extension loaders to wrangle -
// keeps the whole game to a single, easy to read file.
//
// Controls:
//   W / S       - walk forward / back
//   A / D       - turn left / right
//   Space       - jump
//   Esc         - quit

#include <GLFW/glfw3.h>
#include <GL/glu.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

// ---------------------------------------------------------------------
// Small vector helper
// ---------------------------------------------------------------------
struct Vec3 {
    float x, y, z;
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 mul(const Vec3& o) const { return Vec3(x * o.x, y * o.y, z * o.z); }
    Vec3 scaled(float s) const { return Vec3(x * s, y * s, z * s); }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalized() const {
        float l = length();
        if (l < 1e-6f) return Vec3(0, 0, 0);
        return Vec3(x / l, y / l, z / l);
    }
};

static Vec3 cross(const Vec3& a, const Vec3& b) {
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

// ---------------------------------------------------------------------
// Unit icosahedron (12 verts / 20 faces) - the base shape we squash and
// stretch to build Tux's body, head, and the low-poly pine trees.
// ---------------------------------------------------------------------
static const float PHI = 1.61803398875f;

static const Vec3 ICO_VERTS_RAW[12] = {
    Vec3(-1,  PHI, 0), Vec3( 1,  PHI, 0), Vec3(-1, -PHI, 0), Vec3( 1, -PHI, 0),
    Vec3(0, -1,  PHI), Vec3(0,  1,  PHI), Vec3(0, -1, -PHI), Vec3(0,  1, -PHI),
    Vec3( PHI, 0, -1), Vec3( PHI, 0,  1), Vec3(-PHI, 0, -1), Vec3(-PHI, 0,  1)
};

static const int ICO_FACES[20][3] = {
    {0,11,5}, {0,5,1}, {0,1,7}, {0,7,10}, {0,10,11},
    {1,5,9}, {5,11,4}, {11,10,2}, {10,7,6}, {7,1,8},
    {3,9,4}, {3,4,2}, {3,2,6}, {3,6,8}, {3,8,9},
    {4,9,5}, {2,4,11}, {6,2,10}, {8,6,7}, {9,8,1}
};

static Vec3 icoVertsUnit[12];

static void initIco() {
    for (int i = 0; i < 12; i++) icoVertsUnit[i] = ICO_VERTS_RAW[i].normalized();
}

// Draws a flat-shaded triangle, computing its face normal on the fly.
static void flatTri(const Vec3& a, const Vec3& b, const Vec3& c) {
    Vec3 n = cross(b - a, c - a).normalized();
    glNormal3f(n.x, n.y, n.z);
    glVertex3f(a.x, a.y, a.z);
    glVertex3f(b.x, b.y, b.z);
    glVertex3f(c.x, c.y, c.z);
}

// Draws an icosahedron squashed/stretched by `scale` and offset by `center`.
static void drawIco(const Vec3& center, const Vec3& scale, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_TRIANGLES);
    for (auto& f : ICO_FACES) {
        Vec3 a = center + icoVertsUnit[f[0]].mul(scale);
        Vec3 bb = center + icoVertsUnit[f[1]].mul(scale);
        Vec3 c = center + icoVertsUnit[f[2]].mul(scale);
        flatTri(a, bb, c);
    }
    glEnd();
}

// ---------------------------------------------------------------------
// Tux
// ---------------------------------------------------------------------
static void drawTux(float x, float y, float z, float rotYdeg, float waddle) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotYdeg, 0, 1, 0);
    glRotatef(waddle, 0, 0, 1); // little side-to-side waddle while walking

    // Body: black egg shape
    drawIco(Vec3(0, 0.55f, 0), Vec3(0.42f, 0.62f, 0.38f), 0.05f, 0.05f, 0.05f);

    // Head: smaller black sphere on top
    drawIco(Vec3(0, 1.15f, 0.02f), Vec3(0.30f, 0.28f, 0.30f), 0.05f, 0.05f, 0.05f);

    // White belly patch (front-lower body), a small triangle fan pushed
    // slightly outward so it doesn't z-fight with the black body underneath.
    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_TRIANGLES);
    {
        Vec3 top(0, 0.85f, 0.34f);
        Vec3 p[6] = {
            Vec3(-0.22f, 0.55f, 0.28f), Vec3(-0.28f, 0.35f, 0.20f),
            Vec3(-0.18f, 0.10f, 0.24f), Vec3(0.0f, 0.02f, 0.30f),
            Vec3(0.18f, 0.10f, 0.24f),  Vec3(0.28f, 0.35f, 0.20f)
        };
        for (int i = 0; i < 6; i++) {
            Vec3 a = top, b = p[i], c = p[(i + 1) % 6];
            if (i == 5) c = Vec3(-0.22f, 0.55f, 0.28f);
            flatTri(a, b, c);
        }
    }
    glEnd();

    // White face patch
    glColor3f(0.95f, 0.95f, 0.95f);
    glBegin(GL_TRIANGLES);
    {
        Vec3 center(0, 1.12f, 0.26f);
        Vec3 p[5] = {
            Vec3(-0.16f, 1.22f, 0.20f), Vec3(-0.14f, 1.02f, 0.22f),
            Vec3(0.0f, 0.96f, 0.24f),
            Vec3(0.14f, 1.02f, 0.22f), Vec3(0.16f, 1.22f, 0.20f)
        };
        for (int i = 0; i < 4; i++) flatTri(center, p[i], p[i + 1]);
    }
    glEnd();

    // Eyes
    glColor3f(0.02f, 0.02f, 0.02f);
    glBegin(GL_TRIANGLES);
    {
        float eyeY = 1.16f, eyeZ = 0.33f;
        for (float side : {-1.0f, 1.0f}) {
            Vec3 a(0.06f * side, eyeY + 0.035f, eyeZ);
            Vec3 b(0.02f * side, eyeY - 0.035f, eyeZ);
            Vec3 c(0.10f * side, eyeY - 0.02f, eyeZ);
            flatTri(a, b, c);
        }
    }
    glEnd();

    // Beak: small orange pyramid
    glColor3f(0.95f, 0.55f, 0.05f);
    glBegin(GL_TRIANGLES);
    {
        Vec3 tip(0, 1.02f, 0.48f);
        Vec3 bl(-0.07f, 1.08f, 0.30f), br(0.07f, 1.08f, 0.30f), bb(0, 0.98f, 0.30f);
        flatTri(bl, br, tip);
        flatTri(br, bb, tip);
        flatTri(bb, bl, tip);
    }
    glEnd();

    // Flippers (arms)
    glColor3f(0.05f, 0.05f, 0.05f);
    glBegin(GL_TRIANGLES);
    for (float side : {-1.0f, 1.0f}) {
        Vec3 a(0.36f * side, 0.75f, 0.0f);
        Vec3 b(0.52f * side, 0.35f, 0.05f);
        Vec3 c(0.30f * side, 0.30f, -0.05f);
        flatTri(a, b, c);
    }
    glEnd();

    // Feet: orange flattened diamonds
    glColor3f(0.95f, 0.55f, 0.05f);
    glBegin(GL_TRIANGLES);
    for (float side : {-1.0f, 1.0f}) {
        Vec3 heel(0.16f * side, 0.0f, -0.08f);
        Vec3 outer(0.26f * side, 0.0f, 0.10f);
        Vec3 tip(0.10f * side, 0.0f, 0.34f);
        Vec3 inner(0.04f * side, 0.0f, 0.10f);
        flatTri(heel, outer, tip);
        flatTri(heel, tip, inner);
    }
    glEnd();

    glPopMatrix();
}

// ---------------------------------------------------------------------
// Low-poly pine tree: a stretched icosahedron "canopy" over a small brown
// trunk box.
// ---------------------------------------------------------------------
static void drawTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);

    // Trunk
    glColor3f(0.36f, 0.22f, 0.12f);
    glBegin(GL_QUADS);
    float hw = 0.12f, h = 0.6f;
    Vec3 t0(-hw, 0, -hw), t1(hw, 0, -hw), t2(hw, 0, hw), t3(-hw, 0, hw);
    Vec3 u0(-hw, h, -hw), u1(hw, h, -hw), u2(hw, h, hw), u3(-hw, h, hw);
    auto quad = [](Vec3 a, Vec3 b, Vec3 c, Vec3 d) {
        Vec3 n = cross(b - a, c - a).normalized();
        glNormal3f(n.x, n.y, n.z);
        glVertex3f(a.x, a.y, a.z); glVertex3f(b.x, b.y, b.z);
        glVertex3f(c.x, c.y, c.z); glVertex3f(d.x, d.y, d.z);
    };
    quad(t0, t1, u1, u0); quad(t1, t2, u2, u1);
    quad(t2, t3, u3, u2); quad(t3, t0, u0, u3);
    glEnd();

    // Canopy - two stacked stretched icosahedra for a layered pine look
    drawIco(Vec3(0, 1.0f, 0), Vec3(0.55f, 0.65f, 0.55f), 0.10f, 0.35f, 0.14f);
    drawIco(Vec3(0, 1.5f, 0), Vec3(0.36f, 0.5f, 0.36f), 0.12f, 0.4f, 0.16f);

    glPopMatrix();
}

// ---------------------------------------------------------------------
// Collectible fish - a tiny yellow octahedron that spins slowly.
// ---------------------------------------------------------------------
static void drawFish(float x, float y, float z, float spin) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(spin, 0, 1, 0);
    glColor3f(0.95f, 0.85f, 0.1f);
    float s = 0.16f;
    Vec3 top(0, s, 0), bottom(0, -s, 0);
    Vec3 r(s, 0, 0), l(-s, 0, 0), f(0, 0, s), bk(0, 0, -s);
    glBegin(GL_TRIANGLES);
    flatTri(top, f, r); flatTri(top, r, bk); flatTri(top, bk, l); flatTri(top, l, f);
    flatTri(bottom, r, f); flatTri(bottom, bk, r); flatTri(bottom, l, bk); flatTri(bottom, f, l);
    glEnd();
    glPopMatrix();
}

// ---------------------------------------------------------------------
// Terrain - a grid with gentle per-vertex height jitter, flat shaded, so
// it reads as icy low-poly ground rather than a perfectly flat plane.
// ---------------------------------------------------------------------
static const int GRID_N = 24;
static const float GRID_SIZE = 40.0f;
static float terrainHeight[GRID_N + 1][GRID_N + 1];

static void initTerrain() {
    for (int i = 0; i <= GRID_N; i++)
        for (int j = 0; j <= GRID_N; j++)
            terrainHeight[i][j] = ((float)rand() / RAND_MAX) * 0.15f;
}

static Vec3 terrainVert(int i, int j) {
    float step = (2 * GRID_SIZE) / GRID_N;
    float x = -GRID_SIZE + i * step;
    float z = -GRID_SIZE + j * step;
    return Vec3(x, terrainHeight[i][j], z);
}

static void drawTerrain() {
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < GRID_N; i++) {
        for (int j = 0; j < GRID_N; j++) {
            Vec3 a = terrainVert(i, j);
            Vec3 b = terrainVert(i + 1, j);
            Vec3 c = terrainVert(i + 1, j + 1);
            Vec3 d = terrainVert(i, j + 1);
            bool light = ((i + j) % 2) == 0;
            if (light) glColor3f(0.85f, 0.92f, 0.97f);
            else glColor3f(0.72f, 0.85f, 0.93f);
            flatTri(a, b, c);
            flatTri(a, c, d);
        }
    }
    glEnd();
}

// ---------------------------------------------------------------------
// Game state
// ---------------------------------------------------------------------
struct Player {
    float x = 0, z = 0, y = 0;
    float rotY = 0;
    float velY = 0;
    bool onGround = true;
    float walkCycle = 0;
} player;

struct Fish {
    float x, z;
    bool collected = false;
};

std::vector<Fish> fishes;
std::vector<std::pair<float, float>> trees;
int score = 0;

static void resetWorld() {
    fishes.clear();
    trees.clear();
    for (int i = 0; i < 18; i++) {
        float x = ((float)rand() / RAND_MAX) * 2 * GRID_SIZE - GRID_SIZE;
        float z = ((float)rand() / RAND_MAX) * 2 * GRID_SIZE - GRID_SIZE;
        fishes.push_back({x, z, false});
    }
    for (int i = 0; i < 14; i++) {
        float x = ((float)rand() / RAND_MAX) * 2 * GRID_SIZE - GRID_SIZE;
        float z = ((float)rand() / RAND_MAX) * 2 * GRID_SIZE - GRID_SIZE;
        trees.push_back({x, z});
    }
}

static void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    if (height == 0) height = 1;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)width / height, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

int main() {
    srand((unsigned)time(nullptr));
    initIco();
    initTerrain();
    resetWorld();

    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return 1;
    }

    GLFWwindow* window = glfwCreateWindow(1024, 768, "Tux Slide", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create window\n");
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel(GL_FLAT);
    glClearColor(0.55f, 0.75f, 0.9f, 1.0f);

    GLfloat lightPos[] = {8.0f, 20.0f, 10.0f, 1.0f};
    GLfloat lightAmbient[] = {0.35f, 0.35f, 0.4f, 1.0f};
    GLfloat lightDiffuse[] = {1.0f, 1.0f, 0.95f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    framebufferSizeCallback(window, fbw, fbh);

    double lastTime = glfwGetTime();
    double titleTimer = 0.0;
    const float speed = 6.0f;
    const float turnSpeed = 110.0f;
    const float gravity = -18.0f;
    const float jumpVel = 6.5f;
    bool spaceWasDown = false;

    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = (float)(now - lastTime);
        if (dt > 0.05f) dt = 0.05f; // clamp for stability on hitches
        lastTime = now;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        // --- input / movement ---
        bool moving = false;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) player.rotY += turnSpeed * dt;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) player.rotY -= turnSpeed * dt;

        float rad = player.rotY * (float)M_PI / 180.0f;
        float fwdX = std::sin(rad), fwdZ = std::cos(rad);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            player.x += fwdX * speed * dt;
            player.z += fwdZ * speed * dt;
            moving = true;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            player.x -= fwdX * speed * dt;
            player.z -= fwdZ * speed * dt;
            moving = true;
        }

        // keep player on the island
        if (player.x > GRID_SIZE - 1) player.x = GRID_SIZE - 1;
        if (player.x < -GRID_SIZE + 1) player.x = -GRID_SIZE + 1;
        if (player.z > GRID_SIZE - 1) player.z = GRID_SIZE - 1;
        if (player.z < -GRID_SIZE + 1) player.z = -GRID_SIZE + 1;

        // simple tree collision: push the player back out of trunks
        for (auto& t : trees) {
            float dx = player.x - t.first, dz = player.z - t.second;
            float d = std::sqrt(dx * dx + dz * dz);
            float minD = 0.55f;
            if (d < minD && d > 1e-4f) {
                float push = (minD - d);
                player.x += (dx / d) * push;
                player.z += (dz / d) * push;
            }
        }

        // jump
        bool spaceDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
        if (spaceDown && !spaceWasDown && player.onGround) {
            player.velY = jumpVel;
            player.onGround = false;
        }
        spaceWasDown = spaceDown;

        player.velY += gravity * dt;
        player.y += player.velY * dt;
        if (player.y <= 0.0f) {
            player.y = 0.0f;
            player.velY = 0.0f;
            player.onGround = true;
        }

        if (moving) player.walkCycle += dt * 10.0f;
        float waddle = moving ? std::sin(player.walkCycle) * 6.0f : 0.0f;

        // fish pickup
        for (auto& f : fishes) {
            if (f.collected) continue;
            float dx = player.x - f.x, dz = player.z - f.z;
            if (dx * dx + dz * dz < 0.6f * 0.6f) {
                f.collected = true;
                score++;
            }
        }

        // --- camera: third-person chase cam behind the player ---
        float camDist = 5.5f, camHeight = 2.8f;
        float camX = player.x - fwdX * camDist;
        float camZ = player.z - fwdZ * camDist;
        float camY = player.y + camHeight;

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(camX, camY, camZ,
                  player.x, player.y + 1.0, player.z,
                  0, 1, 0);

        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        drawTerrain();
        for (auto& t : trees) drawTree(t.first, t.second);
        for (auto& f : fishes) {
            if (!f.collected)
                drawFish(f.x, 0.35f, f.z, (float)now * 60.0f);
        }
        drawTux(player.x, player.y, player.z, player.rotY, waddle);

        glfwSwapBuffers(window);
        glfwPollEvents();

        titleTimer += dt;
        if (titleTimer > 0.25) {
            titleTimer = 0;
            char buf[64];
            std::snprintf(buf, sizeof(buf), "Tux Slide - Fish: %d / %zu", score, fishes.size());
            glfwSetWindowTitle(window, buf);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
