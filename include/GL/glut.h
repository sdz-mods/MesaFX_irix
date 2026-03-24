/*
 * glut.h — GLUT API for IRIX/FX (fxMesa backend, no X11)
 *
 * Drop-in header for demos that #include <GL/glut.h>.
 * Implemented by glut_fxmesa.c.
 */

#ifndef __GLUT_H__
#define __GLUT_H__

#include <GL/gl.h>
#include <GL/glu.h>

#define GLUT_API_VERSION  3

/* Windows-only calling convention decorator — empty on all other platforms */
#ifndef GLUTCALLBACK
#define GLUTCALLBACK
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* --------------------------------------------------------------------------
 * Display mode flags (glutInitDisplayMode)
 * -------------------------------------------------------------------------- */
#define GLUT_RGB            0x0000
#define GLUT_RGBA           0x0000
#define GLUT_INDEX          0x0001
#define GLUT_SINGLE         0x0000
#define GLUT_DOUBLE         0x0002
#define GLUT_ACCUM          0x0004
#define GLUT_ALPHA          0x0008
#define GLUT_DEPTH          0x0010
#define GLUT_STENCIL        0x0020
#define GLUT_MULTISAMPLE    0x0080
#define GLUT_STEREO         0x0100
#define GLUT_LUMINANCE      0x0200

/* --------------------------------------------------------------------------
 * glutGet tokens
 * -------------------------------------------------------------------------- */
#define GLUT_WINDOW_X                100
#define GLUT_WINDOW_Y                101
#define GLUT_WINDOW_WIDTH            102
#define GLUT_WINDOW_HEIGHT           103
#define GLUT_WINDOW_BUFFER_SIZE      104
#define GLUT_WINDOW_STENCIL_SIZE     105
#define GLUT_WINDOW_DEPTH_SIZE       106
#define GLUT_WINDOW_RED_SIZE         107
#define GLUT_WINDOW_GREEN_SIZE       108
#define GLUT_WINDOW_BLUE_SIZE        109
#define GLUT_WINDOW_ALPHA_SIZE       110
#define GLUT_WINDOW_ACCUM_RED_SIZE   111
#define GLUT_WINDOW_ACCUM_GREEN_SIZE 112
#define GLUT_WINDOW_ACCUM_BLUE_SIZE  113
#define GLUT_WINDOW_ACCUM_ALPHA_SIZE 114
#define GLUT_WINDOW_DOUBLEBUFFER     115
#define GLUT_WINDOW_RGBA             116
#define GLUT_WINDOW_PARENT           117
#define GLUT_WINDOW_NUM_CHILDREN     118
#define GLUT_WINDOW_COLORMAP_SIZE    119
#define GLUT_WINDOW_NUM_SAMPLES      120
#define GLUT_WINDOW_STEREO           121
#define GLUT_WINDOW_CURSOR           122
#define GLUT_SCREEN_WIDTH            200
#define GLUT_SCREEN_HEIGHT           201
#define GLUT_SCREEN_WIDTH_MM         202
#define GLUT_SCREEN_HEIGHT_MM        203
#define GLUT_MENU_NUM_ITEMS          300
#define GLUT_DISPLAY_MODE_POSSIBLE   400
#define GLUT_INIT_WINDOW_X           500
#define GLUT_INIT_WINDOW_Y           501
#define GLUT_INIT_WINDOW_WIDTH       502
#define GLUT_INIT_WINDOW_HEIGHT      503
#define GLUT_INIT_DISPLAY_MODE       504
#define GLUT_ELAPSED_TIME            700

/* --------------------------------------------------------------------------
 * Mouse buttons and state
 * -------------------------------------------------------------------------- */
#define GLUT_LEFT_BUTTON    0
#define GLUT_MIDDLE_BUTTON  1
#define GLUT_RIGHT_BUTTON   2
#define GLUT_DOWN           0
#define GLUT_UP             1

/* --------------------------------------------------------------------------
 * Visibility state
 * -------------------------------------------------------------------------- */
#define GLUT_NOT_VISIBLE    0
#define GLUT_VISIBLE        1

/* --------------------------------------------------------------------------
 * Special key codes (glutSpecialFunc)
 * -------------------------------------------------------------------------- */
#define GLUT_KEY_F1         1
#define GLUT_KEY_F2         2
#define GLUT_KEY_F3         3
#define GLUT_KEY_F4         4
#define GLUT_KEY_F5         5
#define GLUT_KEY_F6         6
#define GLUT_KEY_F7         7
#define GLUT_KEY_F8         8
#define GLUT_KEY_F9         9
#define GLUT_KEY_F10        10
#define GLUT_KEY_F11        11
#define GLUT_KEY_F12        12
#define GLUT_KEY_LEFT       100
#define GLUT_KEY_UP         101
#define GLUT_KEY_RIGHT      102
#define GLUT_KEY_DOWN       103
#define GLUT_KEY_PAGE_UP    104
#define GLUT_KEY_PAGE_DOWN  105
#define GLUT_KEY_HOME       106
#define GLUT_KEY_END        107
#define GLUT_KEY_INSERT     108

/* --------------------------------------------------------------------------
 * Entry modifier keys (glutGetModifiers)
 * -------------------------------------------------------------------------- */
#define GLUT_ACTIVE_SHIFT   1
#define GLUT_ACTIVE_CTRL    2
#define GLUT_ACTIVE_ALT     4

/* --------------------------------------------------------------------------
 * Cursor shapes (glutSetCursor) — no-op on FX
 * -------------------------------------------------------------------------- */
#define GLUT_CURSOR_RIGHT_ARROW     0
#define GLUT_CURSOR_LEFT_ARROW      1
#define GLUT_CURSOR_NONE            101

/* --------------------------------------------------------------------------
 * Initialisation
 * -------------------------------------------------------------------------- */
extern void glutInit(int *argc, char **argv);
extern void glutInitDisplayMode(unsigned int mode);
extern void glutInitWindowSize(int width, int height);
extern void glutInitWindowPosition(int x, int y);

/* --------------------------------------------------------------------------
 * Window management (single window only on FX hardware)
 * -------------------------------------------------------------------------- */
extern int  glutCreateWindow(const char *title);
extern void glutDestroyWindow(int win);
extern void glutSetWindowTitle(const char *title);
extern int  glutGetWindow(void);

/* --------------------------------------------------------------------------
 * Event loop
 * -------------------------------------------------------------------------- */
extern void glutMainLoop(void);
extern void glutPostRedisplay(void);
extern void glutSwapBuffers(void);

/* --------------------------------------------------------------------------
 * Callback registration
 * -------------------------------------------------------------------------- */
extern void glutDisplayFunc(void (*func)(void));
extern void glutReshapeFunc(void (*func)(int width, int height));
extern void glutKeyboardFunc(void (*func)(unsigned char key, int x, int y));
extern void glutKeyboardUpFunc(void (*func)(unsigned char key, int x, int y));
extern void glutSpecialFunc(void (*func)(int key, int x, int y));
extern void glutSpecialUpFunc(void (*func)(int key, int x, int y));
extern void glutMouseFunc(void (*func)(int button, int state, int x, int y));
extern void glutMotionFunc(void (*func)(int x, int y));
extern void glutPassiveMotionFunc(void (*func)(int x, int y));
extern void glutIdleFunc(void (*func)(void));
extern void glutVisibilityFunc(void (*func)(int state));
extern void glutEntryFunc(void (*func)(int state));
extern void glutTimerFunc(unsigned int msecs, void (*func)(int val), int val);

/* --------------------------------------------------------------------------
 * Query
 * -------------------------------------------------------------------------- */
extern int  glutGet(GLenum type);
extern int  glutGetModifiers(void);
extern int  glutExtensionSupported(const char *extension);

/* --------------------------------------------------------------------------
 * Color index mode (no-op on FX — always RGBA)
 * -------------------------------------------------------------------------- */
extern void glutSetColor(int cell, GLfloat red, GLfloat green, GLfloat blue);
extern GLfloat glutGetColor(int cell, int component);

/* --------------------------------------------------------------------------
 * Misc
 * -------------------------------------------------------------------------- */
extern void glutSetCursor(int cursor);
extern void glutWarpPointer(int x, int y);
extern void glutFullScreen(void);

/* --------------------------------------------------------------------------
 * Menus (no-op on FX — no mouse/window system)
 * -------------------------------------------------------------------------- */
extern int  glutCreateMenu(void (*func)(int value));
extern void glutDestroyMenu(int menu);
extern int  glutGetMenu(void);
extern void glutSetMenu(int menu);
extern void glutAddMenuEntry(const char *label, int value);
extern void glutAddSubMenu(const char *label, int submenu);
extern void glutChangeToMenuEntry(int item, const char *label, int value);
extern void glutChangeToSubMenu(int item, const char *label, int submenu);
extern void glutRemoveMenuItem(int item);
extern void glutAttachMenu(int button);
extern void glutDetachMenu(int button);

/* --------------------------------------------------------------------------
 * GLUT geometry (sphere, torus, etc.) — implemented in glut_shapes.c
 * or provided by the full Mesa GLUT; stubs here for linking
 * -------------------------------------------------------------------------- */
extern void glutSolidSphere(GLdouble radius, GLint slices, GLint stacks);
extern void glutWireSphere(GLdouble radius, GLint slices, GLint stacks);
extern void glutSolidCube(GLdouble size);
extern void glutWireCube(GLdouble size);
extern void glutSolidTorus(GLdouble inner, GLdouble outer, GLint sides, GLint rings);
extern void glutWireTorus(GLdouble inner, GLdouble outer, GLint sides, GLint rings);
extern void glutSolidTeapot(GLdouble size);
extern void glutWireTeapot(GLdouble size);
extern void glutSolidCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
extern void glutWireCone(GLdouble base, GLdouble height, GLint slices, GLint stacks);
extern void glutSolidOctahedron(void);
extern void glutWireOctahedron(void);
extern void glutSolidTetrahedron(void);
extern void glutWireTetrahedron(void);
extern void glutSolidIcosahedron(void);
extern void glutWireIcosahedron(void);
extern void glutSolidDodecahedron(void);
extern void glutWireDodecahedron(void);

/* --------------------------------------------------------------------------
 * Bitmap / stroke fonts
 * -------------------------------------------------------------------------- */
typedef void *GLUTbitmapFont;
typedef void *GLUTstrokeFont;

extern void  *glutBitmapHelvetica10;
extern void  *glutBitmapHelvetica12;
extern void  *glutBitmapHelvetica18;
extern void  *glutBitmapTimesRoman10;
extern void  *glutBitmapTimesRoman24;
extern void  *glutBitmap8By13;
extern void  *glutBitmap9By15;

#define GLUT_BITMAP_9_BY_15         (&glutBitmap9By15)
#define GLUT_BITMAP_8_BY_13         (&glutBitmap8By13)
#define GLUT_BITMAP_TIMES_ROMAN_10  (&glutBitmapTimesRoman10)
#define GLUT_BITMAP_TIMES_ROMAN_24  (&glutBitmapTimesRoman24)
#define GLUT_BITMAP_HELVETICA_10    (&glutBitmapHelvetica10)
#define GLUT_BITMAP_HELVETICA_12    (&glutBitmapHelvetica12)
#define GLUT_BITMAP_HELVETICA_18    (&glutBitmapHelvetica18)

extern void  *glutStrokeRoman;
extern void  *glutStrokeMonoRoman;

#define GLUT_STROKE_ROMAN           (&glutStrokeRoman)
#define GLUT_STROKE_MONO_ROMAN      (&glutStrokeMonoRoman)

extern void glutBitmapCharacter(void *font, int character);
extern int  glutBitmapWidth(void *font, int character);
extern void glutStrokeCharacter(void *font, int character);
extern int  glutStrokeWidth(void *font, int character);

#ifdef __cplusplus
}
#endif

#endif /* __GLUT_H__ */
