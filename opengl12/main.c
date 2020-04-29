#include <GL/glew.h>
#include <GL/freeglut.h>
#include "util.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "m.h"
#include "cam.h"

GLuint vao;
GLuint vbo;
GLuint ibo;
GLuint projectionLoc, viewLoc;
GLuint modelLoc;
GLuint posAttr, colorAttr;

Cam cam;
float last_frame;
float delta_time;
int the_w, the_h;

void setUniformLocations() {
    modelLoc = glGetUniformLocation(prog, "model");
    viewLoc = glGetUniformLocation(prog, "view");
    projectionLoc = glGetUniformLocation(prog, "projection");

    posAttr = glGetAttribLocation(prog, "pos");
    colorAttr = glGetAttribLocation(prog, "color");
}

void createBuffer();
void initVao();

void init() {
    createBuffer();

    GLuint shaders[] = {
        createShader("shader.vs", GL_VERTEX_SHADER),
        createShader("shader.fs", GL_FRAGMENT_SHADER)
    };
    int len = sizeof(shaders) / sizeof(shaders[0]);
    createProg(shaders, len);
    int i = 0;
    for (; i < len; i++) {
        glDeleteShader(shaders[i]);
    }

    setUniformLocations();

    initVao();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(0.0f, 1.0f);

    //glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
}

void createBuffer() {
    GLushort indices[] = {
        0, 1, 2,
        0, 2, 3,
        3, 2, 6,
        3, 6, 7,
        4, 5, 1,
        4, 1, 0,
        7, 6, 5,
        7, 5, 4,
        1, 5, 6,
        1, 6, 2,
        4, 0, 3,
        4, 3, 7
    };
    float vertices[] = {
        // vertices
        // front vertices:
        -1.0f, -1.0f, 1.0f,// left-bottom
        -1.0f,  1.0f, 1.0f,// left-top
        1.0f,  1.0f, 1.0f,// right-top
        1.0f, -1.0f, 1.0f,// right-bottom
                          
        // back vertices:
        -1.0f, -1.0f, -1.0f,// left-bottom
        -1.0f,  1.0f, -1.0f,// left-top
        1.0f,  1.0f, -1.0f,// right-top
        1.0f, -1.0f, -1.0f,// right-bottom

        // colors
        1.0f, 0.0f, 0.0f, 1.0f, // red
        0.0f, 1.0f, 0.0f, 1.0f, // green
        1.0f, 0.0f, 1.0f, 1.0f, // purple
        1.0f, 1.0f, 0.0f, 1.0f, // yellow
        0.0f, 1.0f, 1.0f, 1.0f, // aqua
        0.5f, 0.0f, 0.0f, 1.0f, // dark-red
        0.0f, 0.5f, 0.0f, 1.0f, // dark-green
        0.0f, 0.0f, 0.5f, 1.0f  // dark-blue
    };
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void initVao() {
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // vertex array object will store next things:
    // 1. Binding to GL_ELEMENT_ARRAY_BUFFER
    // 2. Enabled vertex attrib arrays
    // 3. Vertex attrib pointer settings

    glEnableVertexAttribArray(posAttr);
    glEnableVertexAttribArray(colorAttr);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // glVertexAttribPointer will use GL_ARRAY_BUFFER
    // , which was set by glBindBuffer
    glVertexAttribPointer(posAttr, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(colorAttr, 4, GL_FLOAT, GL_FALSE, 0, (void*)(8 * 3 * sizeof(float)));
    // Binding to GL_ARRAY_BUFFER will not be stored in
    // the vertex array object,
    // but it was already defined for glVertexAttribPointer.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // This will be used by glDrawElements
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

    glBindVertexArray(0);

}

void set_model(float elapsed, mat4 dest, int pos_dir) {
    float part = fmodf(elapsed, 5000.0f) / 5000.0f;
    float angle = 3.14159f*2.0f * part;
    m_rotate_y_matr((pos_dir*2 - 1) * angle, dest);
}

void display() {
    float elapsed = glutGet(GLUT_ELAPSED_TIME);
    delta_time = elapsed - last_frame;
    last_frame = elapsed;
    cam_do_movement(&cam, delta_time);
    glClearColor(0.33f, 0.06f, 0.68f, 0.0f);
    glClearDepth(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(prog);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(vao);

    mat4 view;
    cam_view(&cam, view);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view);

    float step = 40.0f;
    for (int i = -5; i <= 5; i++) {
        for (int j = -5; j <= 5; j++) {
            for (int k = -5; k <= 5; k++) {
                mat4 model;
                set_model(elapsed, model, i%2);
                mat4 trans;
                m_translate_matr(i * step, j * step, k * step, trans);
                m_mat4_mul(trans, model, model);
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model);

                glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
            }
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
    glutSwapBuffers();
    glutPostRedisplay();
}

void reshape(int w, int h) {
    cam_reshape(&cam, w, h);
    the_w = w;
    the_h = h;
    float fovy = TO_RAD(45);
    float aspect = w / (float)h;
    float nearVal = 0.2f;
    float farVal = 1000.0f;

    mat4 projection;
    m_perspective(fovy, aspect, nearVal, farVal, projection);
    glUseProgram(prog);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projection);
    glUseProgram(0);
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

void keyboard(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    if (key == 27) {
        glutLeaveMainLoop();
    } else if (key == 'w') {
        cam.forward_activated = 1;
    } else if (key == 's') {
        cam.backward_activated = 1;
    } else if (key == 'a') {
        cam.left_activated = 1;
    } else if (key == 'd') {
        cam.right_activated = 1;
    }
    glutPostRedisplay();
}

void keyboard_release(unsigned char key, int x, int y) {
    (void)x;
    (void)y;
    if (key == 27) {
        glutLeaveMainLoop();
    } else if (key == 'w') {
        cam.forward_activated = 0;
    } else if (key == 's') {
        cam.backward_activated = 0;
    } else if (key == 'a') {
        cam.left_activated = 0;
    } else if (key == 'd') {
        cam.right_activated = 0;
    }
}

/*
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    main(__argc, __argv);
    return 0;
}
*/

int motion(int mx, int my) {
    cam_motion(&cam, mx, my, the_w, the_h, 0.10f);
}

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Game");

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(res));
        return 1;
    }
    printf("GL version: %s\n", glGetString(GL_VERSION));

    cam_init(&cam);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutFullScreen();
    glutSetCursor(GLUT_CURSOR_NONE);
    glutIgnoreKeyRepeat(1);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboard_release);
    glutMotionFunc(motion);
    glutPassiveMotionFunc(motion);
    init();
    glutMainLoop();
    return 0;
}
