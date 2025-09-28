#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/* Globale Variablen */
const char* mainMessage = "DEADLINE";
float windowWidth = 800.0f;
float windowHeight = 600.0f;

/* Richtung der Bewegung */
float vx = 0.004f;
float vy = 0.004f;

/* Farbsystem für Farbwechsel */
float currentHue = 0.0f;
float textColorR = 1.0f;
float textColorG = 0.0f;
float textColorB = 0.0f;

/* Text-Position in Pixel-Koordinaten */
float textPosX = 0.0f;
float textPosY = 0.0f;

/* HSV zu RGB Konvertierung (kompatibel mit älteren Compilern) */
void hsvToRgb(float h, float s, float v, float* r, float* g, float* b) {
    if (s == 0.0f) {
        *r = *g = *b = v;
        return;
    }

    /* Normalisierung */
    while (h >= 1.0f) h -= 1.0f;
    while (h < 0.0f) h += 1.0f;

    h = h * 6.0f;
    int i = (int)h;
    float f = h - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch (i) {
    case 0: *r = v; *g = t; *b = p; break;
    case 1: *r = q; *g = v; *b = p; break;
    case 2: *r = p; *g = v; *b = t; break;
    case 3: *r = p; *g = q; *b = v; break;
    case 4: *r = t; *g = p; *b = v; break;
    default: *r = v; *g = p; *b = q; break;
    }
}

/* Farbwechsel bei Randberührung */
void changeColor() {
    currentHue += 0.15f;
    if (currentHue > 1.0f) currentHue -= 1.0f;
    hsvToRgb(currentHue, 0.9f, 1.0f, &textColorR, &textColorG, &textColorB);
}

/* Text-Breite und -Höhe in Pixeln berechnen (korrigiert) */
void getTextDimensions(float* width, float* height) {
    /* Aktuelle Skalierung */
    float scale = (windowWidth + windowHeight) / 6000.0f;
    if (scale < 0.08f) scale = 0.08f;
    if (scale > 0.6f) scale = 0.6f;

    /* Echte Textbreite mit GLUT messen */
    float totalWidth = 0.0f;
    const char* c;
    for (c = mainMessage; *c != '\0'; c++) {
        totalWidth += (float)glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
    }

    *width = totalWidth * scale;
    *height = 119.05f * scale; /* GLUT_STROKE_ROMAN Höhe */

    printf("Debug: Text '%s' - Width: %.1f, Height: %.1f, Scale: %.3f\n",
        mainMessage, *width, *height, scale);
}

/* Text zeichnen */
void drawText() {
    glPushMatrix();

    glTranslatef(textPosX, textPosY, 0.0f);

    /* Dynamische Skalierung */
    float scale = (windowWidth + windowHeight) / 6000.0f;
    if (scale < 0.08f) scale = 0.08f;
    if (scale > 0.6f) scale = 0.6f;

    glScalef(scale, scale, 1.0f);

    glColor3f(textColorR, textColorG, textColorB);

    /* Linienbreite an Bildschirmgröße anpassen */
    float lineWidth = windowHeight / 200.0f;
    if (lineWidth < 1.0f) lineWidth = 1.0f;
    if (lineWidth > 8.0f) lineWidth = 8.0f;
    glLineWidth(lineWidth);

    /* Text mit GLUT stroke font zeichnen */
    const char* c;
    for (c = mainMessage; *c != '\0'; c++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    }

    glPopMatrix();
}

/* Update-Funktion mit echten Bildschirmgrenzen */
void update() {
    /* Text-Dimensionen in Pixeln */
    float textW, textH;
    getTextDimensions(&textW, &textH);

    /* Bewegung in Pixeln pro Frame */
    float pixelVX = vx * windowWidth;
    float pixelVY = vy * windowHeight;

    /* Position updaten */
    textPosX += pixelVX;
    textPosY += pixelVY;

    /* Kollisionserkennung mit echten Bildschirmrändern */
    int colorChanged = 0;

    /* Linker Rand */
    if (textPosX <= 0.0f) {
        textPosX = 0.0f;
        vx = -vx;
        if (!colorChanged) { changeColor(); colorChanged = 1; }
        printf("Hit LEFT edge\n");
    }

    /* Rechter Rand - korrigiert */
    if (textPosX + textW >= windowWidth) {
        textPosX = windowWidth - textW;
        vx = -vx;
        if (!colorChanged) { changeColor(); colorChanged = 1; }
        printf("Hit RIGHT edge - textPosX: %.1f, textW: %.1f, windowWidth: %.1f\n",
            textPosX, textW, windowWidth);
    }

    /* Unterer Rand */
    if (textPosY <= 0.0f) {
        textPosY = 0.0f;
        vy = -vy;
        if (!colorChanged) { changeColor(); colorChanged = 1; }
        printf("Hit BOTTOM edge\n");
    }

    /* Oberer Rand */
    if (textPosY + textH >= windowHeight) {
        textPosY = windowHeight - textH;
        vy = -vy;
        if (!colorChanged) { changeColor(); colorChanged = 1; }
        printf("Hit TOP edge\n");
    }
}

/* Display-Callback */
void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawText();
    glutSwapBuffers();
}

/* Timer-Callback für Animation */
void timer(int value) {
    update();
    glutPostRedisplay();
    glutTimerFunc(33, timer, 0); /* ~30 FPS */
}

/* Reshape-Callback */
void reshape(int w, int h) {
    windowWidth = (float)w;
    windowHeight = (float)h;

    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, w, 0, h, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    /* Bewegungsgeschwindigkeit an Bildschirmgröße anpassen */
    float baseSpeed = 0.004f;
    float speedFactor = (windowWidth + windowHeight) / 1600.0f;
    if (speedFactor < 0.5f) speedFactor = 0.5f;
    if (speedFactor > 3.0f) speedFactor = 3.0f;

    vx = (vx > 0) ? baseSpeed * speedFactor : -baseSpeed * speedFactor;
    vy = (vy > 0) ? baseSpeed * speedFactor : -baseSpeed * speedFactor;
}

/* Keyboard-Callback */
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 27: /* ESC */
        exit(0);
        break;
    case ' ': /* Leertaste */
        changeColor();
        break;
    case 'f':
    case 'F': /* F für Fullscreen */
        glutFullScreen();
        break;
    case 'q':
    case 'Q': /* Q für Quit */
        exit(0);
        break;
    }
}

/* Special Key Callback */
void specialKeys(int key, int x, int y) {
    switch (key) {
    case GLUT_KEY_F11:
        glutFullScreen();
        break;
    }
}

/* Bildschirmauflösung automatisch erkennen */
void detectScreenResolution() {
    const char* resolutions[] = {
        "2560x1440:32@60",
        "1920x1080:32@60",
        "1680x1050:32@60",
        "1600x1200:32@60",
        "1440x900:32@60",
        "1280x1024:32@60",
        "1280x800:32@60",
        "1024x768:32@60",
        "800x600:32@60",
        "640x480:32@60",
        NULL
    };

    int i;
    for (i = 0; resolutions[i] != NULL; i++) {
        glutGameModeString(resolutions[i]);
        if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
            printf("Detected resolution: %s\n", resolutions[i]);

            /* Parse Auflösung aus String */
            int w, h;
#ifdef _MSC_VER
            if (sscanf_s(resolutions[i], "%dx%d", &w, &h) == 2) {
#else
            if (sscanf(resolutions[i], "%dx%d", &w, &h) == 2) {
#endif
                windowWidth = (float)w;
                windowHeight = (float)h;
                printf("Setting screen size to: %dx%d\n", w, h);
                return;
            }
            break;
            }
        }

    /* Fallback */
    printf("No specific resolution detected, using screen query...\n");
    windowWidth = (float)glutGet(GLUT_SCREEN_WIDTH);
    windowHeight = (float)glutGet(GLUT_SCREEN_HEIGHT);
    printf("Screen query result: %.0fx%.0f\n", windowWidth, windowHeight);

    if (windowWidth < 100.0f || windowHeight < 100.0f) {
        windowWidth = 1024.0f;
        windowHeight = 768.0f;
        printf("Using fallback resolution: 1024x768\n");
    }
    }

/* Initialisierung */
void init() {
    srand((unsigned int)time(NULL));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    if (glIsEnabled(GL_LINE_SMOOTH)) {
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    }

    /* Startfarbe setzen */
    currentHue = 0.0f;
    hsvToRgb(currentHue, 0.9f, 1.0f, &textColorR, &textColorG, &textColorB);

    /* Text-Position zentriert */
    textPosX = windowWidth * 0.5f;
    textPosY = windowHeight * 0.5f;
}

/* Main-Funktion */
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);

    /* Temporäres Fenster für Screen-Queries */
    glutInitWindowSize(100, 100);
    glutInitWindowPosition(-1000, -1000);
    glutCreateWindow("temp");

    /* Bildschirmauflösung erkennen */
    detectScreenResolution();

    /* Temporäres Fenster zerstören */
    glutDestroyWindow(glutGetWindow());

    /* Fullscreen mit erkannter Auflösung */
    char gameModeString[64];
#ifdef _MSC_VER
    sprintf_s(gameModeString, sizeof(gameModeString), "%.0fx%.0f:32@60", windowWidth, windowHeight);
#else
    snprintf(gameModeString, sizeof(gameModeString), "%.0fx%.0f:32@60", windowWidth, windowHeight);
#endif
    glutGameModeString(gameModeString);

    if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
        printf("Entering game mode with: %s\n", gameModeString);
        glutEnterGameMode();
    }
    else {
        printf("Game mode not possible, using windowed fullscreen\n");
        glutInitWindowSize((int)windowWidth, (int)windowHeight);
        glutInitWindowPosition(0, 0);
        glutCreateWindow("Deadline Demo");
        glutFullScreen();
    }

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);

    printf("Deadline Demo\n");
    printf("Controls:\n");
    printf("ESC/Q - Exit\n");
    printf("SPACE - Manual color change\n");
    printf("F/F11 - Toggle fullscreen\n");

    glutMainLoop();

    return 0;
}