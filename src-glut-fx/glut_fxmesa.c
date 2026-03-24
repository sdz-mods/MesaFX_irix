/*
 * glut_fxmesa.c — GLUT stub for IRIX 6.5 + 3Dfx Voodoo (no X11)
 *
 * Implements the GLUT API on top of fxMesaCreateBestContext.
 * The Voodoo hardware takes over the display directly.
 *
 * Keyboard input is read from stdin in non-blocking raw mode so demos
 * can be quit with 'q', 'Q', or Escape without X11.
 *
 * Part of MesaFX_irix — built into lib32/libglut.a by Makefile.irixfx.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/time.h>

#include <math.h>
#include <GL/gl.h>
#include <GL/fxmesa.h>
#include <GL/glut.h>

/* --------------------------------------------------------------------------
 * Internal state
 * -------------------------------------------------------------------------- */

static fxMesaContext fx_ctx     = NULL;
static int win_width            = 640;
static int win_height           = 480;
static unsigned int disp_mode   = GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH;
static int redisplay            = 1;

/* Callbacks */
static void (*cb_display)(void)                             = NULL;
static void (*cb_reshape)(int, int)                         = NULL;
static void (*cb_keyboard)(unsigned char, int, int)         = NULL;
static void (*cb_keyboard_up)(unsigned char, int, int)      = NULL;
static void (*cb_special)(int, int, int)                    = NULL;
static void (*cb_special_up)(int, int, int)                 = NULL;
static void (*cb_mouse)(int, int, int, int)                 = NULL;
static void (*cb_motion)(int, int)                          = NULL;
static void (*cb_idle)(void)                                = NULL;
static void (*cb_visibility)(int)                           = NULL;
static void (*cb_timer)(int)                                = NULL;
static int   cb_timer_val                                   = 0;
static unsigned int cb_timer_msecs                          = 0;
static struct timeval cb_timer_start;
static int   cb_timer_active                                = 0;

/* Terminal raw mode */
static struct termios orig_termios;
static int raw_mode_active  = 0;
static int cleanup_registered = 0;

/* Start time for GLUT_ELAPSED_TIME */
static struct timeval start_time;

/* --------------------------------------------------------------------------
 * Terminal raw mode — enables non-blocking keystroke reads from stdin
 * -------------------------------------------------------------------------- */

static void term_restore(void)
{
    if (raw_mode_active) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_mode_active = 0;
    }
}

static void term_raw(void)
{
    struct termios raw;
    if (!isatty(STDIN_FILENO))
        return;
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag    &= ~(ECHO | ICANON);   /* keep ISIG: Ctrl+C still raises SIGINT */
    raw.c_cc[VMIN]  = 0;   /* non-blocking */
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    raw_mode_active = 1;
}

/* Read one byte from stdin without blocking. Returns 1 if a byte was read. */
static int term_read(unsigned char *ch)
{
    return (int)(read(STDIN_FILENO, ch, 1) == 1);
}

/* --------------------------------------------------------------------------
 * Cleanup — restores VGA passthrough and terminal on any exit path
 * -------------------------------------------------------------------------- */

static void glut_cleanup(void)
{
    term_restore();
    if (fx_ctx) {
        fxMesaDestroyContext(fx_ctx);
        fx_ctx = NULL;
    }
}

static void on_sigint(int sig)
{
    (void)sig;
    exit(0);  /* triggers glut_cleanup via atexit */
}

/* --------------------------------------------------------------------------
 * Build fxMesa attrib list from GLUT display mode flags
 * -------------------------------------------------------------------------- */

static void build_attribs(GLint *attribs)
{
    int i = 0;
    if (disp_mode & GLUT_DOUBLE)
        attribs[i++] = FXMESA_DOUBLEBUFFER;
    if (disp_mode & GLUT_ALPHA) {
        attribs[i++] = FXMESA_ALPHA_SIZE;
        attribs[i++] = 8;
    }
    if (disp_mode & GLUT_DEPTH) {
        attribs[i++] = FXMESA_DEPTH_SIZE;
        attribs[i++] = 16;
    }
    if (disp_mode & GLUT_STENCIL) {
        attribs[i++] = FXMESA_STENCIL_SIZE;
        attribs[i++] = 8;
    }
    attribs[i] = FXMESA_NONE;
}

/* --------------------------------------------------------------------------
 * GLUT API — Initialisation
 * -------------------------------------------------------------------------- */

static void register_cleanup(void)
{
    if (!cleanup_registered) {
        cleanup_registered = 1;
        gettimeofday(&start_time, NULL);
        signal(SIGINT, on_sigint);
        atexit(glut_cleanup);
    }
}

void glutInit(int *argc, char **argv)
{
    (void)argc;
    (void)argv;
    register_cleanup();
}

void glutInitDisplayMode(unsigned int mode)
{
    disp_mode = mode;
}

void glutInitWindowSize(int width, int height)
{
    win_width  = width;
    win_height = height;
}

void glutInitWindowPosition(int x, int y)
{
    (void)x;
    (void)y;
    /* no-op: Voodoo always takes full screen */
}

/* --------------------------------------------------------------------------
 * GLUT API — Window
 * -------------------------------------------------------------------------- */

int glutCreateWindow(const char *title)
{
    GLint attribs[16];

    (void)title;

    register_cleanup();
    build_attribs(attribs);

    /* SST1 (Voodoo 1/2) reliably supports 640x480 only.
     * Always open at 640x480 @60Hz and update win_width/win_height so the
     * reshape callback receives the actual framebuffer dimensions. */
    fx_ctx = fxMesaCreateContext(0, GR_RESOLUTION_640x480, GR_REFRESH_60Hz,
                                 attribs);
    if (!fx_ctx) {
        fprintf(stderr, "glut_fxmesa: fxMesaCreateContext(640x480 @60Hz) failed\n");
        exit(1);
    }
    win_width  = 640;
    win_height = 480;

    fxMesaMakeCurrent(fx_ctx);

    /* Enable raw keyboard input */
    term_raw();

    return 1; /* single window, always ID 1 */
}

void glutDestroyWindow(int win)
{
    (void)win;
    term_restore();
    if (fx_ctx) {
        fxMesaDestroyContext(fx_ctx);
        fx_ctx = NULL;
    }
}

void glutSetWindowTitle(const char *title)
{
    (void)title;
}

int glutGetWindow(void)
{
    return fx_ctx ? 1 : 0;
}

void glutFullScreen(void)
{
    /* already full screen */
}

/* --------------------------------------------------------------------------
 * GLUT API — Event loop
 * -------------------------------------------------------------------------- */

void glutPostRedisplay(void)
{
    redisplay = 1;
}

void glutSwapBuffers(void)
{
    fxMesaSwapBuffers();
}

void glutMainLoop(void)
{
    /* Fire initial reshape */
    if (cb_reshape)
        cb_reshape(win_width, win_height);

    /* Fire initial visibility */
    if (cb_visibility)
        cb_visibility(GLUT_VISIBLE);

    for (;;) {
        unsigned char ch;

        /* --- keyboard poll --- */
        if (term_read(&ch)) {
            if (ch == 27) {
                /* Escape: check for escape sequence (arrow keys etc.) */
                unsigned char seq[2];
                if (term_read(&seq[0]) && seq[0] == '[' && term_read(&seq[1])) {
                    if (cb_special) {
                        int key = 0;
                        switch (seq[1]) {
                        case 'A': key = GLUT_KEY_UP;    break;
                        case 'B': key = GLUT_KEY_DOWN;  break;
                        case 'C': key = GLUT_KEY_RIGHT; break;
                        case 'D': key = GLUT_KEY_LEFT;  break;
                        }
                        if (key)
                            cb_special(key, 0, 0);
                    }
                } else {
                    /* Plain Escape */
                    if (cb_keyboard)
                        cb_keyboard(27, 0, 0);
                }
            } else {
                if (cb_keyboard)
                    cb_keyboard(ch, 0, 0);
            }
        }

        /* --- timer --- */
        if (cb_timer_active && cb_timer) {
            struct timeval now;
            unsigned int elapsed;
            void (*f)(int);
            int v;
            gettimeofday(&now, NULL);
            elapsed =
                (now.tv_sec  - cb_timer_start.tv_sec)  * 1000 +
                (now.tv_usec - cb_timer_start.tv_usec) / 1000;
            if (elapsed >= cb_timer_msecs) {
                f = cb_timer;
                v = cb_timer_val;
                cb_timer        = NULL;
                cb_timer_active = 0;
                f(v);
            }
        }

        /* --- idle then redisplay --- */
        if (cb_idle)
            cb_idle();

        if (redisplay && cb_display) {
            redisplay = 0;
            cb_display();
        }
    }
}

/* --------------------------------------------------------------------------
 * GLUT API — Callbacks
 * -------------------------------------------------------------------------- */

void glutDisplayFunc(void (*func)(void))               { cb_display     = func; }
void glutReshapeFunc(void (*func)(int, int))           { cb_reshape     = func; }
void glutKeyboardFunc(void (*func)(unsigned char, int, int)) { cb_keyboard = func; }
void glutKeyboardUpFunc(void (*func)(unsigned char, int, int)) { cb_keyboard_up = func; }
void glutSpecialFunc(void (*func)(int, int, int))      { cb_special     = func; }
void glutSpecialUpFunc(void (*func)(int, int, int))    { cb_special_up  = func; }
void glutMouseFunc(void (*func)(int, int, int, int))   { cb_mouse       = func; }
void glutMotionFunc(void (*func)(int, int))            { cb_motion      = func; }
void glutPassiveMotionFunc(void (*func)(int, int))     { (void)func; }
void glutIdleFunc(void (*func)(void))                  { cb_idle        = func; }
void glutVisibilityFunc(void (*func)(int))             { cb_visibility  = func; }
void glutEntryFunc(void (*func)(int))                  { (void)func; }

void glutTimerFunc(unsigned int msecs, void (*func)(int val), int val)
{
    cb_timer        = func;
    cb_timer_val    = val;
    cb_timer_msecs  = msecs;
    cb_timer_active = 1;
    gettimeofday(&cb_timer_start, NULL);
}

/* --------------------------------------------------------------------------
 * GLUT API — Query
 * -------------------------------------------------------------------------- */

int glutGet(GLenum type)
{
    struct timeval now;
    switch (type) {
    case GLUT_WINDOW_WIDTH:         return win_width;
    case GLUT_WINDOW_HEIGHT:        return win_height;
    case GLUT_SCREEN_WIDTH:         return win_width;
    case GLUT_SCREEN_HEIGHT:        return win_height;
    case GLUT_WINDOW_DOUBLEBUFFER:  return (disp_mode & GLUT_DOUBLE) ? 1 : 0;
    case GLUT_WINDOW_DEPTH_SIZE:    return (disp_mode & GLUT_DEPTH)  ? 16 : 0;
    case GLUT_WINDOW_RGBA:          return 1;
    case GLUT_ELAPSED_TIME:
        gettimeofday(&now, NULL);
        return (int)(
            (now.tv_sec  - start_time.tv_sec)  * 1000 +
            (now.tv_usec - start_time.tv_usec) / 1000);
    default:
        return 0;
    }
}

int glutGetModifiers(void)
{
    return 0;
}

int glutExtensionSupported(const char *extension)
{
    const char *exts = (const char *)glGetString(GL_EXTENSIONS);
    const char *p;
    size_t len;

    if (!exts || !extension)
        return 0;

    len = strlen(extension);
    p   = exts;
    while ((p = strstr(p, extension)) != NULL) {
        /* check it's a whole word */
        if ((p == exts || p[-1] == ' ') &&
            (p[len] == '\0' || p[len] == ' '))
            return 1;
        p += len;
    }
    return 0;
}

/* --------------------------------------------------------------------------
 * GLUT API — Color index (no-op: FX is always RGBA)
 * -------------------------------------------------------------------------- */

void glutSetColor(int cell, GLfloat red, GLfloat green, GLfloat blue)
{
    (void)cell; (void)red; (void)green; (void)blue;
}

GLfloat glutGetColor(int cell, int component)
{
    (void)cell; (void)component;
    return 0.0f;
}

/* --------------------------------------------------------------------------
 * GLUT API — Misc
 * -------------------------------------------------------------------------- */

void glutSetCursor(int cursor) { (void)cursor; }
void glutWarpPointer(int x, int y) { (void)x; (void)y; }

/* --------------------------------------------------------------------------
 * Menus — no-op stubs (no window system / mouse on FX)
 * -------------------------------------------------------------------------- */
int  glutCreateMenu(void (*func)(int value))               { (void)func; return 0; }
void glutDestroyMenu(int menu)                             { (void)menu; }
int  glutGetMenu(void)                                     { return 0; }
void glutSetMenu(int menu)                                 { (void)menu; }
void glutAddMenuEntry(const char *label, int value)        { (void)label; (void)value; }
void glutAddSubMenu(const char *label, int submenu)        { (void)label; (void)submenu; }
void glutChangeToMenuEntry(int item, const char *label, int value) { (void)item; (void)label; (void)value; }
void glutChangeToSubMenu(int item, const char *label, int submenu) { (void)item; (void)label; (void)submenu; }
void glutRemoveMenuItem(int item)                          { (void)item; }
void glutAttachMenu(int button)                            { (void)button; }
void glutDetachMenu(int button)                            { (void)button; }

/* --------------------------------------------------------------------------
 * Font data stubs (satisfy the linker; actual glyph data not needed
 * for most demos — they just print with glutBitmapCharacter which we stub)
 * -------------------------------------------------------------------------- */

void *glutStrokeRoman         = NULL;
void *glutStrokeMonoRoman     = NULL;

void *glutBitmapHelvetica10   = NULL;
void *glutBitmapHelvetica12   = NULL;
void *glutBitmapHelvetica18   = NULL;
void *glutBitmapTimesRoman10  = NULL;
void *glutBitmapTimesRoman24  = NULL;
void *glutBitmap8By13         = NULL;
void *glutBitmap9By15         = NULL;

void glutBitmapCharacter(void *font, int character)
{
    (void)font; (void)character;
}

int glutBitmapWidth(void *font, int character)
{
    (void)font; (void)character;
    return 0;
}

void glutStrokeCharacter(void *font, int character)
{
    (void)font; (void)character;
}

int glutStrokeWidth(void *font, int character)
{
    (void)font; (void)character;
    return 0;
}

/* --------------------------------------------------------------------------
 * Geometry — implemented directly with GL primitives (no GLU dependency)
 * -------------------------------------------------------------------------- */

static void sphere_vertex(double r, double a, double b)
{
    double x = r * cos(b) * cos(a);
    double y = r * cos(b) * sin(a);
    double z = r * sin(b);
    glNormal3d(cos(b)*cos(a), cos(b)*sin(a), sin(b));
    glVertex3d(x, y, z);
}

void glutSolidSphere(GLdouble radius, GLint slices, GLint stacks)
{
    int i, j;
    double pi = 3.14159265358979323846;
    for (j = 0; j < stacks; j++) {
        double b0 = pi * (-0.5 + (double)j / stacks);
        double b1 = pi * (-0.5 + (double)(j+1) / stacks);
        glBegin(GL_QUAD_STRIP);
        for (i = 0; i <= slices; i++) {
            double a = 2.0 * pi * (double)i / slices;
            sphere_vertex(radius, a, b0);
            sphere_vertex(radius, a, b1);
        }
        glEnd();
    }
}

void glutWireSphere(GLdouble radius, GLint slices, GLint stacks)
{
    int i, j;
    double pi = 3.14159265358979323846;
    for (j = 0; j <= stacks; j++) {
        double b = pi * (-0.5 + (double)j / stacks);
        glBegin(GL_LINE_LOOP);
        for (i = 0; i < slices; i++) {
            double a = 2.0 * pi * (double)i / slices;
            glVertex3d(radius*cos(b)*cos(a), radius*cos(b)*sin(a), radius*sin(b));
        }
        glEnd();
    }
}

void glutSolidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
{
    int i, j;
    double pi = 3.14159265358979323846;
    for (j = 0; j < stacks; j++) {
        double r0 = base * (1.0 - (double)j / stacks);
        double r1 = base * (1.0 - (double)(j+1) / stacks);
        double z0 = height * (double)j / stacks;
        double z1 = height * (double)(j+1) / stacks;
        glBegin(GL_QUAD_STRIP);
        for (i = 0; i <= slices; i++) {
            double a = 2.0 * pi * (double)i / slices;
            double nx = cos(a), ny = sin(a);
            glNormal3d(nx, ny, base/height);
            glVertex3d(r0*nx, r0*ny, z0);
            glVertex3d(r1*nx, r1*ny, z1);
        }
        glEnd();
    }
}

void glutWireCone(GLdouble base, GLdouble height, GLint slices, GLint stacks)
{
    int i, j;
    double pi = 3.14159265358979323846;
    for (j = 0; j <= stacks; j++) {
        double r = base * (1.0 - (double)j / stacks);
        double z = height * (double)j / stacks;
        glBegin(GL_LINE_LOOP);
        for (i = 0; i < slices; i++) {
            double a = 2.0 * pi * (double)i / slices;
            glVertex3d(r*cos(a), r*sin(a), z);
        }
        glEnd();
    }
}

void glutSolidTorus(GLdouble inner, GLdouble outer, GLint sides, GLint rings)
{
    int   i, j;
    float pi2 = 2.0f * 3.14159265f;
    for (i = 0; i < rings; i++) {
        glBegin(GL_QUAD_STRIP);
        for (j = 0; j <= sides; j++) {
            int   k;
            float s = (float)j / sides;
            for (k = 0; k <= 1; k++) {
                float t  = (float)(i + k) / rings;
                float x  = (float)((outer + inner * cos(s * pi2)) * cos(t * pi2));
                float y  = (float)((outer + inner * cos(s * pi2)) * sin(t * pi2));
                float z  = (float)(inner * sin(s * pi2));
                float nx = (float)(cos(s * pi2) * cos(t * pi2));
                float ny = (float)(cos(s * pi2) * sin(t * pi2));
                float nz = (float)(sin(s * pi2));
                glNormal3f(nx, ny, nz);
                glVertex3f(x, y, z);
            }
        }
        glEnd();
    }
}

void glutWireTorus(GLdouble inner, GLdouble outer, GLint sides, GLint rings)
{
    int   i, j;
    float pi2 = 2.0f * 3.14159265f;
    for (i = 0; i < rings; i++) {
        glBegin(GL_LINE_LOOP);
        for (j = 0; j < sides; j++) {
            float s = (float)j / sides;
            float t = (float)i / rings;
            float x = (float)((outer + inner * cos(s * pi2)) * cos(t * pi2));
            float y = (float)((outer + inner * cos(s * pi2)) * sin(t * pi2));
            float z = (float)(inner * sin(s * pi2));
            glVertex3f(x, y, z);
        }
        glEnd();
    }
}

void glutSolidCube(GLdouble size)
{
    float s = (float)size / 2.0f;
    glBegin(GL_QUADS);
    /* front */  glNormal3f( 0, 0, 1); glVertex3f(-s,-s, s); glVertex3f( s,-s, s); glVertex3f( s, s, s); glVertex3f(-s, s, s);
    /* back */   glNormal3f( 0, 0,-1); glVertex3f( s,-s,-s); glVertex3f(-s,-s,-s); glVertex3f(-s, s,-s); glVertex3f( s, s,-s);
    /* left */   glNormal3f(-1, 0, 0); glVertex3f(-s,-s,-s); glVertex3f(-s,-s, s); glVertex3f(-s, s, s); glVertex3f(-s, s,-s);
    /* right */  glNormal3f( 1, 0, 0); glVertex3f( s,-s, s); glVertex3f( s,-s,-s); glVertex3f( s, s,-s); glVertex3f( s, s, s);
    /* top */    glNormal3f( 0, 1, 0); glVertex3f(-s, s, s); glVertex3f( s, s, s); glVertex3f( s, s,-s); glVertex3f(-s, s,-s);
    /* bottom */ glNormal3f( 0,-1, 0); glVertex3f(-s,-s,-s); glVertex3f( s,-s,-s); glVertex3f( s,-s, s); glVertex3f(-s,-s, s);
    glEnd();
}

void glutWireCube(GLdouble size)
{
    float s = (float)size / 2.0f;
    glBegin(GL_LINE_LOOP); glVertex3f(-s,-s,-s); glVertex3f( s,-s,-s); glVertex3f( s, s,-s); glVertex3f(-s, s,-s); glEnd();
    glBegin(GL_LINE_LOOP); glVertex3f(-s,-s, s); glVertex3f( s,-s, s); glVertex3f( s, s, s); glVertex3f(-s, s, s); glEnd();
    glBegin(GL_LINES);
    glVertex3f(-s,-s,-s); glVertex3f(-s,-s, s);
    glVertex3f( s,-s,-s); glVertex3f( s,-s, s);
    glVertex3f( s, s,-s); glVertex3f( s, s, s);
    glVertex3f(-s, s,-s); glVertex3f(-s, s, s);
    glEnd();
}

/* Stubs for less common shapes */
void glutSolidTeapot(GLdouble size)    { (void)size; }
void glutWireTeapot(GLdouble size)     { (void)size; }
void glutSolidOctahedron(void)  {}
void glutWireOctahedron(void)   {}
void glutSolidTetrahedron(void) {}
void glutWireTetrahedron(void)  {}
void glutSolidIcosahedron(void) {}
void glutWireIcosahedron(void)  {}
void glutSolidDodecahedron(void){}
void glutWireDodecahedron(void) {}
