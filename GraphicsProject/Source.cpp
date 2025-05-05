#include <Windows.h>
#define STB_IMAGE_IMPLEMENTATION
#include<Gl/stb_image.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>

using namespace std;

#pragma region Global Varibles

//unsigned int texture;
int width, height, nrChannels;

// 3 modes ll game 
enum GameState {
    SPLASH_SCREEN,
    PLAYING,
    GAME_OVER
};
// car att
struct Car {
    float x;
    float y;
    float z;
    float speed;
    float rotation;
};

GameState gameState = SPLASH_SCREEN;
int score = 0;
Car playerCar = { 0.0f, 0.0f, 0.0f, 0.3f, 0.0f };  // Initialize with default values
vector<Car> trafficCars;
float baseSpeed = 0.3f;
bool altCameraView = false;
const char* gameOver = "Game_Over.png";
const char* splash = "Texture.jpg";
unsigned int splashTexture;
unsigned int backgroundTexture;
bool Change_Player_lighting = false; 
bool Change_Opp_lighting = false;

bool paused = false;
float
matamb[] = { .2125,.1275,.054,1 },
matdiff[] = { .714,.4284,.18144,1 },
matspec[] = { .393548,.271906,.166721,1 },
matshin[] = { 25.6 },
lightamb[] = { 1,0,1,1 },
lightdiff[] = { 0,.6,.6,1 },
lightspec[] = { .2,.2,.2,1 },
lightPos[] = { 0.0f, 0.0f, 0.0f, 0.0f };

#pragma endregion

#pragma region Functions ProtoType

string scoreToString(int);


void drawText(const char*, float, float);


void drawCar(float, float, float, bool);


void drawRoad();


void spawnTrafficCar();


void initGame();


void display();


void update();


void timer(int);


void keyboard(unsigned char, int, int);


void specialKeys(int, int, int);

void mouse(int, int, int, int);

void reshape(int, int);


void background();


void load(const char*, unsigned int&);
//void drawSplashScreen();


#pragma endregion

#pragma region Main Function

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    glutCreateWindow("Racing Game");
    glutFullScreen();


    background();
    load(gameOver, backgroundTexture);
    load(splash, splashTexture);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouse);

    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}

#pragma endregion

#pragma region Score Fucntion Dispaly 
// Function to convert score to string
string scoreToString(int score) {
    stringstream ss;
    ss << "Score: " << score;
    return ss.str();
}
#pragma endregion

#pragma region Draw
void drawText(const char* text, float x, float y) {
    // Save the current projection and modelview matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-5, 5, -5, 5); // Create an orthographic projection
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glColor3f(1.0f, 1.0f, 1.0f); // Ensure text is white
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    // Restore previous matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawCar(float x, float y, float z, bool isPlayer) {
    glPushMatrix();
    glTranslatef(x, y, z);

    // Car color
    if (isPlayer) {
        glColor3f(1.0f, 0.0f, 0.0f);  // Red for player
    }
    else {
        glColor3f(0.0f, 0.0f, 1.0f);  // Blue for traffic
    }
     if (isPlayer && Change_Player_lighting) {
        glEnable(GL_LIGHTING);
    }
     if (!isPlayer && Change_Opp_lighting) {
         glEnable(GL_LIGHTING);
     }
    // Main body
    glPushMatrix();
    glScalef(1.0f, 0.5f, 2.0f);
    glutSolidCube(1.0f);
    glPopMatrix();
   
   
    // Top
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, 0.0f);
    glScalef(0.8f, 0.4f, 1.2f);
    glutSolidCube(1.0f);
    glPopMatrix();
    if (isPlayer && Change_Player_lighting) {
        glDisable(GL_LIGHTING);
    }
    if (!isPlayer && Change_Opp_lighting) {
        glDisable(GL_LIGHTING);
    }

    // Wheels
    glColor3f(0.2f, 0.2f, 0.2f);
    for (float xOffset : {-0.6f, 0.6f}) {
        for (float zOffset : {-0.7f, 0.7f}) {
            glPushMatrix();
            glTranslatef(xOffset, -0.25f, zOffset);
            glutSolidCube(0.3f);
            glPopMatrix();
        }
    }

    glPopMatrix();
}

void drawRoad() {
    float startZ = playerCar.z - 100.0f;
    float endZ = playerCar.z + 100.0f;

    // Road surface : sabtha w al3arbyat hya aly btt7ark 
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_QUADS);
    glVertex3f(-5.0f, -0.5f, startZ);
    glVertex3f(5.0f, -0.5f, startZ);
    glVertex3f(5.0f, -0.5f, endZ);
    glVertex3f(-5.0f, -0.5f, endZ);
    glEnd();

    // Road lines
    glColor3f(1.0f, 1.0f, 1.0f);
    for (float z = startZ; z < endZ; z += 5.0f) {
        glBegin(GL_QUADS);
        glVertex3f(-0.1f, -0.48f, z);
        glVertex3f(0.1f, -0.48f, z);
        glVertex3f(0.1f, -0.48f, z + 3.0f);
        glVertex3f(-0.1f, -0.48f, z + 3.0f);
        glEnd();
    }
}

void createImage(unsigned int texture) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, 800, 0, 600);  // شاشة 2D مناسبة لمقاس الويندو

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();


    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(800.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(800.0f, 600.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 600.0f);
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glDisable(GL_TEXTURE_2D);
}


void drawSplashScreen() {
    createImage(splashTexture);
}


void drawGameOverScreen() {
    createImage(backgroundTexture);
}

#pragma endregion

#pragma region Put the cars in the Road
// بيعمل عربية وبعدين يخش ع اللوب ويكريت كل بيانات العربية و بعدها بيقارن بالعربيات اللي موجوده ف ال فيكتور وبيشوف لو في واحده قريبه منها ولا ولو فيه بيزود ال اتمبيت بواحد ويعيدي من الاول وولو ملقاش بيلف عشر مرات دة ال اتمبيت بتاعه 

void spawnTrafficCar() {
    Car car;
    bool positionValid = false; // don't crash 
    int attempts = 0;
    // MAKAN DA VALID WLA L2A W  LESS THAN 10 
    while (!positionValid && attempts < 10) {
        car.x = -4.0f + (rand() % 9);
        car.y = 0.0f;
        car.z = playerCar.z - 50.0f - (rand() % 30);
        car.speed = baseSpeed * 1.5f;
        car.rotation = 0.0f;

        positionValid = true;
        for (const auto& other : trafficCars) {
            // MKAN4 FE 3ARBYA GT MAKNHA 
            if (fabs(car.x - other.x) < 1.5f && fabs(car.z - other.z) < 4.0f) {
                positionValid = false;
                break;
            }
        }
        attempts++;
    }

    if (positionValid) {
        trafficCars.push_back(car); // IN Vector 
    }
}

#pragma endregion

#pragma region Inizlization
void initGame() {
    playerCar = { 0.0f, 0.0f, 0.0f, baseSpeed, 0.0f };
    trafficCars.clear();
    for (int i = 0; i < 6; i++) {
        spawnTrafficCar();
    }
    score = 0;
}

#pragma endregion

#pragma region Display
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    switch (gameState) {
    case SPLASH_SCREEN:
        glDisable(GL_LIGHTING);
        drawSplashScreen();
        /* drawText("Press Enter to Start", -1.0f, 0.0f);*/
        break;

    case PLAYING:

        if (altCameraView) {
            gluLookAt(playerCar.x, 5.0f, playerCar.z + 25.0f,
                playerCar.x, 0.0f, playerCar.z,
                0.0f, 1.0f, 0.0f);
        }
        else {


            gluLookAt(playerCar.x, 3.0f, playerCar.z + 10.0f,
                playerCar.x, 0.0f, playerCar.z,
                0.0f, 1.0f, 0.0f);
        }
        drawRoad(); // DRWA THE ROAD 
    
        drawCar(playerCar.x, playerCar.y, playerCar.z, true); // dRAW THE DRIVER CAR 
        //glDisable(GL_LIGHTING);

        // OPP CARS 
        for (const auto& car : trafficCars) {
            if (car.z < 9999.0f)
                drawCar(car.x, car.y, car.z, false);
        }

        // Score
        glLoadIdentity();
        glColor3f(1.0f, 1.0f, 1.0f);
        drawText(scoreToString(score).c_str(), -4.5f, 3.0f);
        break;

    case GAME_OVER:
   /*     glDisable(GL_LIGHTING);*/
        drawGameOverScreen();
      /*  glEnable(GL_LIGHTING);*/
        break;
    }

    glutSwapBuffers();
}

#pragma endregion

#pragma region Update
void update() {
    if (gameState != PLAYING || paused) return;

    playerCar.z -= playerCar.speed;

    for (auto& car : trafficCars) {
        car.z += car.speed;

        if (car.z > playerCar.z + 10.0f) {
            if ((rand() % 100) < 50) {
                car.z = playerCar.z - 50.0f - (rand() % 20);
                car.x = -4.0f + (rand() % 9);
                score += 10;
            }
            else {
                car.z = 9999.0f;
            }
        }
        float dx = playerCar.x - car.x;
        float dz = playerCar.z - car.z;
        if (sqrt(dx * dx + dz * dz) < 1.5f) {
            gameState = GAME_OVER;
            return;
        }
    }
}

#pragma endregion

#pragma region Timer
void timer(int value) {
    update();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

#pragma endregion

#pragma region Keyboard&Mouese
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27:
        exit(0);
        break;
    case 13:
        if (gameState == SPLASH_SCREEN || gameState == GAME_OVER) {
            gameState = PLAYING;
            initGame();
        }
        break;
    case 'v':
    case 'V':
        altCameraView = !altCameraView;
        gameState = PLAYING;
        break;
    case 'a':
    case 'A':
        if (playerCar.x > -4.0f)
            playerCar.x -= 0.3f;
        break;
    case 'd':
    case 'D':
        if (playerCar.x < 4.0f)
            playerCar.x += 0.3f;
        break;
    case 'L' :
    case'l':
        Change_Player_lighting = !Change_Player_lighting;
        break;
    case 'K':
    case'k':
        Change_Opp_lighting = !Change_Opp_lighting;
        break;
    }
}
void specialKeys(int key, int x, int y) {
    if (gameState != PLAYING) return;

    switch (key) {
    case GLUT_KEY_LEFT:
        if (playerCar.x > -4.0f)
            playerCar.x -= 0.3f;
        break;
    case GLUT_KEY_RIGHT:
        if (playerCar.x < 4.0f)
            playerCar.x += 0.3f;
        break;
    }
}
void mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        paused = !paused;

    }
}

#pragma endregion

#pragma region Reshape
void reshape(int w, int h) {
    if (h == 0) h = 1;
    float ratio = w * 1.0f / h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

}

#pragma endregion

#pragma region BackGround
void background() {
    glClearColor(0, 0, 0, 1);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightspec);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdiff);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glMaterialfv(GL_FRONT, GL_AMBIENT, matamb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matdiff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matspec);
    glMaterialfv(GL_FRONT, GL_SHININESS, matshin);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

}
#pragma endregion

#pragma region Texture

void load(const char* filename, unsigned int& textureID) {
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else {
        cout << "Failed to load texture: " << filename << endl;
    }

    stbi_image_free(data);
}


#pragma endregion

