#include <GL/glut.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint backgroundTex;
GLuint bgTex;
static int slices = 16;
static int stacks = 16;

// ===== Player =====
float playerX = 0;
float walkTime = 0;
float legAngle = 0;
float armAngle = 0;
float targetX = 0;
float playerY = 0;
float velocityY = 0;
float gravity = -0.02;
int isJumping = 0;

// ===== Ground =====
float groundX = 0;
float groundZ = -20;

// ===== Rocket =====
float rocketx = 0;
float rocketz = -15;

// ===== Game =====
int score = 0;
int gameOver = 0;
int gameWon = 0;
int level = 1;

// ===== Menu =====
int gameState = 0;
const char* menuItems[] = { "Start Game", "Instructions", "Quit" };
const int menuCount = 3;
int selectedItem = 0;

// ================== Forward Declarations ==================
void drawText(float x, float y, char* string);
void drawInstructions();
void drawWinScreen();

// ================== Load Texture ==================
GLuint loadTexture(const char* filename)
{
    int width, height, channels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filename, &width, &height, &channels, STBI_rgb_alpha);

    if (!data)
    {
        std::cout << "STB LOAD ERROR: " << stbi_failure_reason() << std::endl;
        return 0;
    }

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return tex;
}

// ================== Draw Background ==================
void drawBackground()
{
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, backgroundTex);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0,0); glVertex2f(-1,-1);
        glTexCoord2f(1,0); glVertex2f( 1,-1);
        glTexCoord2f(1,1); glVertex2f( 1, 1);
        glTexCoord2f(0,1); glVertex2f(-1, 1);
    glEnd();

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glPopAttrib();
}

// ================== Init ==================
void init()
{
    srand(time(NULL));
    glClearColor(0, 0, 0, 1);
    backgroundTex = loadTexture("bk1.jpg");
    if (backgroundTex == 0)
        std::cout << "Background texture failed to load\n";
}

// ================== Draw Text ==================
void drawText(float x, float y, char* string)
{
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glRasterPos2f(x, y);
    for (char* c = string; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ================== Draw Menu Item ==================
void drawMenuItem(float x, float y, const char* label, bool isSelected)
{
    if (isSelected)
        glColor3f(1.0f, 1.0f, 0.0f);
    else
        glColor3f(1.0f, 1.0f, 1.0f);

    drawText(x, y, (char*)label);
}

// ================== Draw Menu ==================
void drawMenu()
{
    glColor3f(0.2f, 0.8f, 1.0f);
    drawText(300, 450, (char*)"Jumping Astro");

    float startY = 300.0f;
    for (int i = 0; i < menuCount; i++)
        drawMenuItem(340, startY - i * 50, menuItems[i], i == selectedItem);
}

// ================== Draw Instructions ==================
void drawInstructions()
{
    // Title
    glColor3f(0.2f, 0.8f, 1.0f);
    drawText(290, 520, (char*)"HOW TO PLAY");

    // Movement
    glColor3f(1.0f, 1.0f, 0.0f);
    drawText(180, 460, (char*)"MOVEMENT");
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(180, 430, (char*)"Left Arrow  - Move left");
    drawText(180, 400, (char*)"Right Arrow - Move right");
    drawText(180, 370, (char*)"Spacebar    - Jump");

    // Objective
    glColor3f(1.0f, 1.0f, 0.0f);
    drawText(180, 320, (char*)"OBJECTIVE");
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(180, 290, (char*)"Land on the platforms to score points.");
    drawText(180, 260, (char*)"Reach score 5 to unlock Level 2.");
    drawText(180, 230, (char*)"Reach score 15 to WIN the game!");

    // Dangers
    glColor3f(1.0f, 0.3f, 0.3f);
    drawText(180, 180, (char*)"DANGERS");
    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(180, 150, (char*)"Missing a platform = GAME OVER.");
    drawText(180, 120, (char*)"In Level 2, dodge incoming rockets!");

    // Return hint
    glColor3f(0.6f, 0.6f, 0.6f);
    drawText(270, 60, (char*)"Press ESC to return to menu");
}

// ================== Draw Level Transition ==================
void drawLevelTransition()
{
    glColor3f(1.0f, 1.0f, 0.0f);
    drawText(280, 350, (char*)"LEVEL 2 UNLOCKED!");

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(270, 300, (char*)"Rockets activated!");
    drawText(270, 250, (char*)"Press Enter to continue");
}

// ================== Draw Win Screen ==================
void drawWinScreen()
{
    glColor3f(0.0f, 1.0f, 0.0f);
    drawText(320, 400, (char*)"YOU WIN!");
    drawText(280, 350, (char*)"Final Score: 15");

    glColor3f(1.0f, 1.0f, 1.0f);
    drawText(290, 300, (char*)"Press R to play again");
    drawText(320, 250, (char*)"ESC for menu");
}

// ================== Draw Player ==================
void drawPlayer()
{
    glPushMatrix();
    glTranslatef(playerX, 0.3 + playerY, -0.5);

    // body
    glPushMatrix();
        glColor3f(0.85f, 0.88f, 0.92f);
        glScalef(0.72f, 1.05f, 0.52f);
       glutSolidCube(0.8);
    glPopMatrix();

    // vest details
    glPushMatrix();
        glColor3f(0.2f, 0.5f, 1.0f);
        glTranslatef(0.0f, 0.1f, 0.27f);
        glScalef(0.28f, 0.18f, 0.05f);
        glutSolidCube(1);
    glPopMatrix();

    //oxygen
    glPushMatrix();
        glColor3f(0.6f, 0.62f, 0.65f);
        glTranslatef(0.0f, 0.1f, -0.35f);
        glScalef(0.38f, 0.55f, 0.18f);
        glutSolidCube(1);
    glPopMatrix();
    // backpack detail stripe
    glPushMatrix();
        glColor3f(1.0f, 0.6f, 0.0f);
        glTranslatef(0.0f, 0.1f, -0.45f);
        glScalef(0.12f, 0.42f, 0.04f);
        glutSolidCube(1);
    glPopMatrix();

    //==== Helmet====
    glPushMatrix();
        glColor3f(0.9f, 0.92f, 0.95f);
        glTranslatef(0, 0.92f, 0);
        glutSolidSphere(0.33f, 20, 20);

        //  (dark tinted front glass)
        glPushMatrix();
            glColor3f(0.15f, 0.55f, 0.85f);
            glTranslatef(0.0f, 0.0f, 0.22f);
            glScalef(0.38f, 0.28f, 0.12f);
            glutSolidSphere(1.0f, 12, 12);
        glPopMatrix();

        // Helmet side light (left)
        glPushMatrix();
            glColor3f(1.0f, 0.9f, 0.2f);
            glTranslatef(-0.28f, 0.1f, 0.1f);
            glutSolidSphere(0.05f, 8, 8);
        glPopMatrix();

        // Helmet side light (right)
        glPushMatrix();
            glColor3f(1.0f, 0.9f, 0.2f);
            glTranslatef(0.28f, 0.1f, 0.1f);
            glutSolidSphere(0.05f, 8, 8);
        glPopMatrix();
    glPopMatrix();

    // --- Left Arm ---
    glPushMatrix();
        glColor3f(0.82f, 0.85f, 0.90f);
        glTranslatef(-0.46f, 0.35f, 0);
        glRotatef(-armAngle, 1, 0, 0);
        // upper arm
        glPushMatrix();
            glScalef(0.22f, 0.55f, 0.22f);
            glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        // elbow joint
        glPushMatrix();
            glColor3f(0.7f, 0.72f, 0.76f);
            glTranslatef(0, -0.28f, 0);
            glutSolidSphere(0.1f, 10, 10);
        glPopMatrix();
        // forearm
        glTranslatef(0, -0.42f, 0);
        glRotatef(-armAngle * 0.5f, 1, 0, 0);
        glPushMatrix();
            glColor3f(0.82f, 0.85f, 0.90f);
            glScalef(0.20f, 0.50f, 0.20f);
            glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        // glove
        glPushMatrix();
            glColor3f(0.2f, 0.5f, 1.0f);
            glTranslatef(0, -0.28f, 0);
            glScalef(0.18f, 0.14f, 0.18f);
            glutSolidSphere(1.0f, 10, 10);
        glPopMatrix();
    glPopMatrix();

    // --- Right Arm ---
    glPushMatrix();
        glColor3f(0.82f, 0.85f, 0.90f);
        glTranslatef(0.46f, 0.35f, 0);
        glRotatef(armAngle, 1, 0, 0);
        // upper arm
        glPushMatrix();
            glScalef(0.22f, 0.55f, 0.22f);
            glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        // elbow joint
        glPushMatrix();
            glColor3f(0.7f, 0.72f, 0.76f);
            glTranslatef(0, -0.28f, 0);
            glutSolidSphere(0.1f, 10, 10);
        glPopMatrix();
        // forearm
        glTranslatef(0, -0.42f, 0);
        glRotatef(armAngle * 0.5f, 1, 0, 0);
        glPushMatrix();
            glColor3f(0.82f, 0.85f, 0.90f);
            glScalef(0.20f, 0.50f, 0.20f);
            glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        // glove
        glPushMatrix();
            glColor3f(0.2f, 0.5f, 1.0f);
            glTranslatef(0, -0.28f, 0);
            glScalef(0.18f, 0.14f, 0.18f);
            glutSolidSphere(1.0f, 10, 10);
        glPopMatrix();
    glPopMatrix();

    // --- Left Leg ---
    glPushMatrix();
        glColor3f(0.78f, 0.80f, 0.85f);
        glTranslatef(-0.21f, -0.55f, 0);
        glRotatef(legAngle, 1, 0, 0);
        // thigh
        glPushMatrix();
            glScalef(0.26f, 0.55f, 0.26f);
            glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        // knee joint
        glPushMatrix();
            glColor3f(0.65f, 0.67f, 0.72f);
            glTranslatef(0, -0.28f, 0);
            glutSolidSphere(0.11f, 10, 10);
        glPopMatrix();
        // shin
        glTranslatef(0, -0.45f, 0);
        glRotatef(legAngle * 0.5f, 1, 0, 0);
        glPushMatrix();
            glColor3f(0.78f, 0.80f, 0.85f);
            glScalef(0.23f, 0.52f, 0.23f);
            glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        // boot
        glPushMatrix();
            glColor3f(0.15f, 0.15f, 0.18f);
            glTranslatef(0.0f, -0.32f, 0.05f);
            glScalef(0.26f, 0.16f, 0.32f);
            glutSolidCube(1);
        glPopMatrix();
    glPopMatrix();

    // --- Right Leg ---
    glPushMatrix();
        glColor3f(0.78f, 0.80f, 0.85f);
        glTranslatef(0.21f, -0.55f, 0);
        glRotatef(-legAngle, 1, 0, 0);
        // thigh
        glPushMatrix();
            glScalef(0.26f, 0.55f, 0.26f);
            glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        // knee joint
        glPushMatrix();
            glColor3f(0.65f, 0.67f, 0.72f);
            glTranslatef(0, -0.28f, 0);
            glutSolidSphere(0.11f, 10, 10);
        glPopMatrix();
        // shin
        glTranslatef(0, -0.45f, 0);
        glRotatef(-legAngle * 0.5f, 1, 0, 0);
        glPushMatrix();
            glColor3f(0.78f, 0.80f, 0.85f);
            glScalef(0.23f, 0.52f, 0.23f);
            glutSolidSphere(0.5f, 10, 10);
        glPopMatrix();
        // boot
        glPushMatrix();
            glColor3f(0.15f, 0.15f, 0.18f);
            glTranslatef(0.0f, -0.32f, 0.05f);
            glScalef(0.26f, 0.16f, 0.32f);
            glutSolidCube(1);
        glPopMatrix();
    glPopMatrix();

    glPopMatrix();
}

// ================== Draw Rocket ==================
void drawrocket()
{
    glPushMatrix();
    glRotatef(90,1,0,0);

    // Rocket top
    glPushMatrix();
        glColor3f(1.0f, 0.0f, 0.0f);
        glTranslatef(0.0f, 0.6f, 0.0f);
        glRotatef(-90, 1, 0, 0);
        glutSolidCone(0.25f, 0.5f, slices, stacks);
    glPopMatrix();

    // Rocket body
    glPushMatrix();
        glColor3f(1.0f, 0.0f, 1.0f);
        glScalef(0.25f, 0.6f, 0.25f);
        glutSolidSphere(1.0f, slices, stacks);
    glPopMatrix();

    // Left wing
    glPushMatrix();
        glColor3f(0.5f, 0.5f, 0.5f);
        glTranslatef(-0.35f, -0.3f, 0.0f);
        glScalef(0.3f, 0.4f, 0.08f);
        glutSolidCube(1);
    glPopMatrix();

    // Right wing
    glPushMatrix();
        glColor3f(0.5f, 0.5f, 0.5f);
        glTranslatef(0.35f, -0.3f, 0.0f);
        glScalef(0.3f, 0.4f, 0.08f);
        glutSolidCube(1);
    glPopMatrix();

    // Tail
    glPushMatrix();
        glColor3f(0.4f, 0.4f, 0.4f);
        glTranslatef(0.0f, -0.3f, -0.35f);
        glScalef(0.08f, 0.4f, 0.3f);
        glutSolidCube(1);
    glPopMatrix();

    // Flame
    glPushMatrix();
        glColor3f(1.0f, 0.4f, 0.0f);
        glTranslatef(0.0f, -0.85f, 0.0f);
        glRotatef(90, 1, 0, 0);
        glutSolidCone(0.09f, 0.35f, slices, stacks);
    glPopMatrix();

    glPopMatrix();
}

// ================== Resize ==================
static void resize(int width, int height)
{
    const float ar = (float)width / (float)height;
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-ar, ar, -1.2, 1.2, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// ================== Display ==================
static void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (gameState == 0)
    {
        drawBackground();
        drawMenu();
    }
    else if (gameState == 1)
    {
        drawBackground();

        glEnable(GL_LIGHTING);
        glLoadIdentity();

        gluLookAt(playerX, 2 + playerY, 3,
                  playerX, playerY, 0,
                  0, 1, 0);

        drawPlayer();

        // Ground platform
        glPushMatrix();
            glColor3f(0.5, 0.5, 0.5);
            glTranslatef(groundX, -5, groundZ);
            glRotatef(90, 1, 0, 0);
            glutSolidCone(3.2, 3.6, slices, stacks);
        glPopMatrix();

        // Rocket (level 2 only)
        if (level == 2)
        {
            glPushMatrix();
                glTranslatef(rocketx, -1.5, rocketz);
                drawrocket();
            glPopMatrix();
        }

        // HUD
        glDisable(GL_LIGHTING);
        char text[50];
        glColor3f(1, 1, 1);
        sprintf(text, "Score: %d", score);
        drawText(10, 570, text);
        sprintf(text, "Level: %d", level);
        drawText(10, 540, text);

        if (gameOver)
        {
            glColor3f(1, 0, 0);
            drawText(300, 300, (char*)"GAME OVER");
            glColor3f(1, 1, 1);
            drawText(320, 250, (char*)"Press R to restart");
        }

        if (gameWon)
            drawWinScreen();
    }
    else if (gameState == 2)
    {
        drawBackground();
        drawInstructions();  // Replaces drawOptions()
    }
    else if (gameState == 3)
    {
        drawBackground();
        drawLevelTransition();
    }

    glutSwapBuffers();
}

// ================== Reset Game ==================
void resetGame()
{
    score = 0;
    level = 1;
    gameOver = 0;
    gameWon = 0;
    playerX = 0;
    playerY = 0;
    velocityY = 0;
    groundX = 0;
    groundZ = -20;
    rocketx = 0;
    rocketz = -15;
    isJumping = 0;
}

// ================== Update ==================
void update(int value)
{
    if (gameState == 1 && !gameOver && !gameWon)
    {
        groundZ += 0.15;

        // Ground collision
        if (fabs(playerX - groundX) < 0.8 && fabs(groundZ) < 0.8)
        {
            score++;
            int lane = rand() % 5;
            groundX = (lane - 2) * 2.0;
            groundZ = -15;
        }

        // Level progression
        if (score == 5 && level == 1)
        {
            level = 2;
            gameState = 3;
        }

        // Win condition
        if (score == 15 && level == 2)
            gameWon = 1;

        // Rocket (level 2 only)
        if (level == 2)
        {
            rocketz += 0.2;

            if (fabs(playerX - rocketx) < 0.8 && fabs(rocketz) < 0.8)
                gameOver = 1;

            if (rocketz > 6)
            {
                rocketx = (rand() % 5 - 2) * 2.0;
                rocketz = -15;
            }
        }

        // Reset ground
        if (groundZ > 6)
        {
            groundX = (rand() % 5 - 2) * 2.0;
            groundZ = -15;
        }

        // Game over: missed platform
        if (groundZ > 2 && fabs(playerX - groundX) > 0.8)
            gameOver = 1;
    }

    walkTime += 0.01;
    legAngle = sin(walkTime * 8) * 30;
    armAngle = sin(walkTime * 8) * 30;

    playerY += velocityY;
    velocityY += gravity;

    if (playerY <= 0)
    {
        playerY = 0;
        velocityY = 0;
        isJumping = 0;
    }

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

// ================== Special Keys (Arrow Keys) ==================
void keyboard(int key, int x, int y)
{
    if (gameState != 1 || gameOver || gameWon) return;

    if (key == GLUT_KEY_LEFT)
        playerX -= 1.2;
    if (key == GLUT_KEY_RIGHT)
        playerX += 1.2;
}

// ================== Regular Keys ==================
static void key(unsigned char k, int x, int y)
{
    // Menu
    if (gameState == 0)
    {
        if (k == 'w' || k == 'W')
            selectedItem = (selectedItem - 1 + menuCount) % menuCount;
        if (k == 's' || k == 'S')
            selectedItem = (selectedItem + 1) % menuCount;
        if (k == 13) // Enter
        {
            if (selectedItem == 0) { resetGame(); gameState = 1; }
            if (selectedItem == 1) gameState = 2;  // Instructions
            if (selectedItem == 2) exit(0);
        }
        glutPostRedisplay();
        return;
    }

    // Level transition
    if (gameState == 3)
    {
        if (k == 13)
        {
            gameState = 1;
            groundZ = -15;
        }
        glutPostRedisplay();
        return;
    }

    // Instructions screen — ESC returns to menu
    if (gameState == 2)
    {
        if (k == 27)
            gameState = 0;
        glutPostRedisplay();
        return;
    }

    // In-game
    if (gameState == 1)
    {
        switch (k)
        {
            case 27:
                gameState = 0;
                break;
            case 'q': case 'Q':
                exit(0);
                break;
            case 'r': case 'R':
                if (gameOver || gameWon)
                    resetGame();
                break;
            case ' ':
                if (!isJumping && !gameOver && !gameWon)
                {
                    velocityY = 0.25;
                    isJumping = 1;
                }
                break;
            case '+':
                slices++; stacks++;
                break;
            case '-':
                if (slices > 3 && stacks > 3) { slices--; stacks--; }
                break;
        }
    }

    glutPostRedisplay();
}

// ================== Idle ==================
static void idle(void)
{
    glutPostRedisplay();
}

// ================== Lighting ==================
const GLfloat light_ambient[]  = { 0.0f, 0.0f, 0.0f, 1.0f };
const GLfloat light_diffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat light_position[] = { 2.0f, 5.0f, 5.0f, 0.0f };

const GLfloat mat_ambient[]    = { 0.7f, 0.7f, 0.7f, 1.0f };
const GLfloat mat_diffuse[]    = { 0.8f, 0.8f, 0.8f, 1.0f };
const GLfloat mat_specular[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
const GLfloat high_shininess[] = { 100.0f };

// ================== Main ==================
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(10, 10);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
    glutCreateWindow("Jumping Astro - Collect 15 to WIN!");

    init();

    glutReshapeFunc(resize);
    glutDisplayFunc(display);
    glutKeyboardFunc(key);
    glutSpecialFunc(keyboard);
    glutIdleFunc(idle);
    glutTimerFunc(0, update, 0);

    glClearColor(0, 0, 0, 1);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_LIGHTING);

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glutMainLoop();
    return EXIT_SUCCESS;
}
