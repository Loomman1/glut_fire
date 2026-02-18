
#include <stdio.h>
#include <glut.h> 	
////#include <math.h>
//
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
//#include "Steck.h" 	
double M_PI = 3.141;



// Параметры частиц пламени
const int PARTICLE_COUNT = 200;
const int FLAME_VOLUME = 70; //Ширина у основания
const float FLAME_MAX_HEIGHT = 0.8f;
const float PARTICLE_SIZE_BASE = 0.0000001f;
const int PARTICLE_SIZE_FACTOR = 50;

// Параметры свечи
const float CANDLE_MOVE_SPEED = 0.008f;
float FireBordersDeviation=0.14f;
float FireOffset=0.0f;
float flameIntensity = 0.2f;
bool isDayTime = false; // Дневной свет по умолчанию
bool isParticlesVisible = true;
float candlePosX = 0.0f; // Позиция свечи по X
float flameWobble = 0.0f; // Для колыхания пламени
// Структура частицы пламени
struct Particle {
    float x, y, z;
    float speed;
    float life;
    float size;
    float sway;
};


Particle particles[PARTICLE_COUNT];

// Инициализация частиц
void initParticles() {
    srand(time(0));
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        particles[i].x = candlePosX+(rand() % FLAME_VOLUME - FLAME_VOLUME/2) / 1000.0f;
        particles[i].y = (rand() % FLAME_VOLUME - FLAME_VOLUME/2) / 1000.0f;
        particles[i].z = (rand() % FLAME_VOLUME - FLAME_VOLUME/2) / 1000.0f;
        particles[i].speed = 0.001f + (rand() % 100) / 1000.0f;
        particles[i].life = 0.5f + (rand() % 100) / 200.0f;
        particles[i].size = PARTICLE_SIZE_BASE + (rand() % PARTICLE_SIZE_FACTOR) / 2000.0f;
    }
}

// Рисуем цилиндр
void drawCylinder(float radius, float height, int segments) {
    float angleStep = 2.0f * 3.14159f / segments;

    // Боковая поверхность
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        glNormal3f(x, 0, z);
        glVertex3f(x, -height / 2, z);
        glVertex3f(x, height / 2, z);
    }
    glEnd();

    // Верхняя крышка
    glBegin(GL_POLYGON);
    glNormal3f(0, 1, 0);
    for (int i = 0; i < segments; i++) {
        float angle = i * angleStep;
        glVertex3f(radius * cos(angle), height / 2, radius * sin(angle));
    }
    glEnd();

    // Нижняя крышка
    glBegin(GL_POLYGON);
    glNormal3f(0, -1, 0);
    for (int i = 0; i < segments; i++) {
        float angle = i * angleStep;
        glVertex3f(radius * cos(angle), -height / 2, radius * sin(angle));
    }
    glEnd();
}

// Рисуем тор (для подсвечника)
void drawTorus(float innerRadius, float outerRadius, int sides, int rings) {
    for (int i = 0; i < sides; i++) {
        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= rings; j++) {
            for (int k = 1; k >= 0; k--) {
                double s = (i + k) % sides + 0.5;
                double t = j % (rings + 1);

                double x = (outerRadius + innerRadius * cos(s * 2 * M_PI / sides)) * cos(t * 2 * M_PI / rings);
                double y = (outerRadius + innerRadius * cos(s * 2 * M_PI / sides)) * sin(t * 2 * M_PI / rings);
                double z = innerRadius * sin(s * 2 * M_PI / sides);

                glNormal3f(2 * x, 2 * y, 2 * z);
                glVertex3f(x, z, y);
            }
        }
        glEnd();
    }
}

// Функция рисования свечи
void drawCandle() {
    glPushMatrix();
    glTranslatef(candlePosX, 0.0f, 0.0f); // Учет позиции свечи

    // Подсвечник (металлический)
    glColor3f(0.3f, 0.3f, 0.35f);

    glTranslatef(0.0f, -0.8f, 0.0f);
    drawTorus(0.05f, 0.15f, 20, 20);

    // Основание подсвечника
    glTranslatef(0.0f, -0.1f, 0.0f);
    drawCylinder(0.2f, 0.05f, 32);

    // Свеча (воск)
    glColor3f(0.95f, 0.95f, 0.9f);
    glTranslatef(0.0f, 0.4f, 0.0f);
    drawCylinder(0.08f, 1.0f, 32);

    // Фитиль
    glColor3f(0.2f, 0.2f, 0.2f);
    glTranslatef(0.0f, 0.35f, 0.0f);  // Увеличил высоту фитиля
    drawCylinder(0.01f, 0.5f, 8);      // Увеличил длину фитиля

    glPopMatrix();
}

// Функция рисования пламени
void drawFlame() {
    glPushMatrix();
    glTranslatef(candlePosX, 0.43f, 0.0f); // Учет позиции свечи

    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    flameWobble = sin(time * 5.0f) * 0.02f; // Колыхание пламени

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Основное пламя с колыханием
    glBegin(GL_TRIANGLE_FAN);
    glColor4f(1.0f, 1.0f, 0.1f, 0.9f);

    // Верхняя точка пламени с колебанием
    float flameTopX = FireOffset-flameWobble;
    float flameTopY = -0.1f + sin(time * 3.0f) * 0.001f;
    glVertex3f(flameTopX, flameTopY, 0.0f);

    glColor4f(1.0f, 0.6f, 0.8f, 0.4f);
    // Основание пламени с небольшими колебаниями
    for (int i = 0; i <= 360; i += 30) {
        float angle = i * 3.14159f / 180.0f;
        float wobbleFactor = 1.0f + sin(time * 10.0f + angle) * 0.1f;
        glVertex3f(cos(angle) * 0.04f * wobbleFactor,
            -0.41f + sin(time * 8.0f + angle) * 0.01f,
            0.001+sin(angle) * 0.04f * wobbleFactor);
    }
    glEnd();

    glDisable(GL_BLEND);
    glPopMatrix();
    glPushMatrix();

    // Частицы пламени
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    if (isParticlesVisible)
    {
        for (int i = 0; i < PARTICLE_COUNT; i++) {
            glPushMatrix();
            // Если частица привязана к свече, учитываем позицию свечи
            glTranslatef(particles[i].x, particles[i].y, particles[i].z);

            float alpha = particles[i].life * flameIntensity * 0.6f; //яркость
            glColor4f(1.0f, 0.3f + 0.5f * particles[i].life, 0.0f, alpha);

            glutSolidSphere(particles[i].size, 8, 3);

            glPopMatrix();
        }
    }
    glDisable(GL_BLEND);
    glPopMatrix();
}

// Обновление частиц
void updateParticles() {
    float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    for (int i = 0; i < PARTICLE_COUNT; i++) {
        if (particles[i].life <= 0.0f || particles[i].y > FLAME_MAX_HEIGHT) {
            // Респавн частиц у основания фитиля
            particles[i].x = candlePosX+(rand() % FLAME_VOLUME - FLAME_VOLUME/2) / 1000.0f;
            particles[i].y = (rand() % FLAME_VOLUME - FLAME_VOLUME/2) / 1000.0f;
            particles[i].z = (rand() % FLAME_VOLUME - FLAME_VOLUME/2) / 1000.0f;
            particles[i].life = 0.2f + (rand() % 50) / 100.0f;
            particles[i].speed = 0.004f + (rand() % 1000) / 60000.0f;
            //particles[i].sway = (rand() % 100) / 300.0f;
        }
        else {

            particles[i].x += 0;
            particles[i].y += particles[i].speed;
            particles[i].z += 0;
            particles[i].life -= 0.003f;
        }
    }
    if (FireOffset > FireBordersDeviation) {
        FireOffset -= CANDLE_MOVE_SPEED;
    }
    else if (FireOffset < -1 * FireBordersDeviation)
    {
        FireOffset += CANDLE_MOVE_SPEED;
    }else if (FireOffset > 0)
        FireOffset -= CANDLE_MOVE_SPEED/3;
    else if(FireOffset<0)
        FireOffset += CANDLE_MOVE_SPEED/3;
}

// Обновление анимации
void update(int value) {
    // Плавное изменение интенсивности пламени
    flameIntensity = 0.8f + 0.2f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.005f);

    updateParticles();
    glutPostRedisplay();
    glutTimerFunc(16, update, 0); // ~60 FPS
}

// Настройка освещения
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat lightPos[] = { 0.0f, 0.5f, 0.25f, 1.0f };
    GLfloat lightColor[] = { 1.0f, 0.8f, 0.5f, 1.0f };
    GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
}

// Настройка вида камеры
void setupCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 800.0 / 600.0, 0.1, 100.0); // Угол обзора 45 градусов

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0.0, 0.5, 2.5,  // Камера немного выше и дальше
        0.0, 0.0, 0.0,   // Смотрим в центр
        0.0, 1.0, 0.0);  // Вектор "вверх"
}

// Основная функция отрисовки
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    setupCamera();

    // Фон
    if (isDayTime) {
        glClearColor(0.7f, 0.8f, 1.0f, 1.0f);
    }
    else {
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
    }

    setupLighting();
    drawCandle();
    drawFlame();

    glutSwapBuffers();
}

// Обработка клавиш
void keyboard(unsigned char key, int x, int y) {
    if (key == 'd') {
        isDayTime = !isDayTime;
    }
    if (key == 'v') {
        isParticlesVisible = !isParticlesVisible;
    }
    else if (key == 27) {
        exit(0);
    }
}
void KeyboardSpec(int key, int x, int y) {
   if (key == GLUT_KEY_LEFT) {
        candlePosX -= CANDLE_MOVE_SPEED;
        //if (FireOffset < FireBordersDeviation)
        //{ 
            FireOffset += CANDLE_MOVE_SPEED;
        //}
    }
    else if (key == GLUT_KEY_RIGHT) {
        candlePosX += CANDLE_MOVE_SPEED;
        //if (FireOffset > -1*FireBordersDeviation)
        //{
            FireOffset -= CANDLE_MOVE_SPEED;
        //}
    }
    else if (key == 27) {
        exit(0);
    }
}

// Инициализация окна
void initWindow() {
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("3D Candle with Realistic Flame");
    glViewport(0, 0, 800, 600);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    initWindow();
    initParticles();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(KeyboardSpec);

    glutTimerFunc(0, update, 0);

    glutMainLoop();
    return 0;
}
