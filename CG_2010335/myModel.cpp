
// g++ myModel.cpp -lglut -lGL -lGLU -lm

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>

#include <math.h>

#include <iostream>
#include <vector>

#include "texture_class.h"
#include "camera_rotate.h"

#define M_PI 3.14159265358979323846

// 光源について
int light_status = 0;
GLfloat light0pos[] = { 0.0, 3.0, 5.0, 1.0 };    // (0, 3, 5)に置く

// R, G, B, A の順に指定
GLfloat black[] = { 0, 0, 0, 1 };
GLfloat white[] = { 1, 1, 1, 1 };
GLfloat green[] = { 0, 1, 0, 1 };
GLfloat gray[] = { 0.3f, 0.3f, 0.3f, 1.0f };
GLfloat dark[] = { 0.1f, 0.1f, 0.1f, 1.0f };

// 背景色
GLclampf sky_blue[] = { .5f, .9f, 1.0f, 1.0f };
GLclampf sky_dark[] = { 0, 0, 0, 1 };

// 橋の頂点の配列
int parabola_verts_num;    // 放物線ケーブルの頂点の数
GLdouble** parabola_verts;    // 放物線ケーブルの頂点の配列
// 道路の頂点の配列
GLdouble** road_verts;

// 水面
sea_plane seaPlane("texture/water_256x256.raw", 256, 256);

// カメラ
camera_rotate cam_rotate;


int face[][4] = {
    { 0, 1, 2, 3 },
    { 1, 5, 6, 2 },
    { 5, 4, 7, 6 },
    { 4, 0, 3, 7 },
    { 4, 5, 1, 0 },
    { 3, 2, 6, 7 }
};



void print_vector3d(GLdouble v[])
{
    printf("(%f, %f, %f)\n", v[0], v[1], v[2]);
}


GLdouble** malloc_vector3(int numVerts)
{
    GLdouble **pp = (GLdouble**)malloc(numVerts * sizeof(GLdouble*));
    if (pp == NULL) { printf("malloc error (1)\n"); return NULL; }
    int i;
    for (i = 0; i < numVerts; i++)
    {
        pp[i] = (GLdouble*)malloc(sizeof(GLdouble) * 3);
        if (pp[i] == NULL) { printf("malloc error (2)\n"); return NULL; }
    }
    return pp;
}


void add3dv(GLdouble sum[], const GLdouble a[], const GLdouble b[])
{
    sum[0] = a[0] + b[0];
    sum[1] = a[1] + b[1];
    sum[2] = a[2] + b[2];
}

void sub3dv(GLdouble sum[], const GLdouble a[], const GLdouble b[])
{
    sum[0] = a[0] - b[0];
    sum[1] = a[1] - b[1];
    sum[2] = a[2] - b[2];
}

void mult3dv(GLdouble ans[], const GLdouble a[], GLdouble b)
{
    ans[0] = a[0] * b;
    ans[1] = a[1] * b;
    ans[2] = a[2] * b;
}


// v1からv2へ柱を描画する
// normalはポール先端の平面の法線ベクトル、'x''y''z'いずれか
// radiusは柱の半径
// polygon_nは断面の形
void draw_pole(char normal, GLdouble v1[], GLdouble v2[], float radius, int polygon_n)
{
    double theta_step = 2 * M_PI / polygon_n;
    //GLdouble center2[3];
    //add3dv(center2, v1, v2);    // 中心座標を計算（しかし計算量削減のため/2はしない）
    glBegin(GL_QUAD_STRIP);
    int i;
    for (i = 0; i <= polygon_n; i++)
    {
        double theta = theta_step * (i + 0.5);
        GLdouble p1[3], p2[3];
        double x = radius * cos(theta);
        double y = radius * sin(theta);
        if (normal == 'y')
        {
            p1[0] = v1[0] + x;  p2[0] = v2[0] + x;
            p1[1] = v1[1];      p2[1] = v2[1];
            p1[2] = v1[2] + y;  p2[2] = v2[2] + y;
        }
        else if (normal == 'x')
        {
            p1[0] = v1[0];      p2[0] = v2[0];
            p1[1] = v1[1] + y;  p2[1] = v2[1] + y;
            p1[2] = v1[2] + x;  p2[2] = v2[2] + x;
        }
        else
        {
            p1[0] = v1[0] + x;  p2[0] = v2[0] + x;
            p1[1] = v1[1] + y;  p2[1] = v2[1] + y;
            p1[2] = v1[2];  p2[2] = v2[2];
        }
        glVertex3dv(p1); glVertex3dv(p2);
    }
    glEnd();
}


// 双曲線を求める関数
float parabola_func(float a, float x)
{
    return a * x * x;
}


// 橋の頂点を登録する
// 一回実行すればOK
void calc_bridge_vertices()
{
    const float bridge_height = 0.4f;
    const float bridge_len = 3.0f;    // 支柱間距離の半分
    const float step = 0.1f;    // 縦のケーブル間隔
    const float cable_coef = 0.3f;    // ケーブルの放物線の係数

    // 頂点の登録
    int bridge_split = (int)(bridge_len / step) + 1;
    parabola_verts_num = bridge_split;    // 頂点の数を計算
    // 頂点の配列を確保
    parabola_verts = malloc_vector3(bridge_split);
    road_verts = malloc_vector3(bridge_split * 2);

    int j;
    for (j = 0; j < bridge_split; j++)    // 真ん中の支柱間
    {
        float x = step * j - (bridge_len / 2);
        double center_y = parabola_func(cable_coef, x);    // 放物線を計算
        parabola_verts[j][0] = x;
        parabola_verts[j][1] = center_y + bridge_height;
        parabola_verts[j][2] = 0;

        road_verts[2 * j][0] = road_verts[2 * j + 1][0] = x;
        road_verts[2 * j][1] = bridge_height;
        road_verts[2 * j + 1][1] = bridge_height + center_y;
        road_verts[2 * j][2] = road_verts[2 * j + 1][2] = 0;
    }
}


void brid_material()
{
    GLfloat brid_amb[] = { .1f, .1f, .1f, 1 };
    GLfloat brid_dif[] = { .8f, .8f, .8f, 1 };
    GLfloat brid_spec[] = { .5f, .5f, .5f, 1 };
    GLfloat brid_shine[] = { 4 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, brid_amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, brid_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, brid_spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, brid_shine);
}


void draw_cube()
{
    GLdouble v[][3] = {
        { -.5, -.5, -.5},
        { -.5, -.5,  .5 },
        {  .5, -.5,  .5 },
        {  .5, -.5, -.5 },
        { -.5,  .5, -.5 },
        { -.5,  .5,  .5 },
        {  .5,  .5,  .5 },
        {  .5,  .5, -.5 }
    };
    int i, j;
    for (i = 0; i < 6; i++)
    {
        glBegin(GL_QUADS);
        for (j = 0; j < 4; j++)
        {
            glVertex3dv(v[face[i][j]]);
        }
        glEnd();
    }
}


// 橋の柱
void bridge_pole(GLdouble x, double bridge_width, double height)
{
    double pole_width = bridge_width / 4.0f;
    int i, j, k;
    for (k = -1; k < 2; k += 2)
    {
        glPushMatrix();
        glTranslated(x, height / 2, k * bridge_width);
        glScaled(pole_width * 2, height, pole_width * 2);
        draw_cube();
        glPopMatrix();
    }

    GLdouble v2[][3] = {
        { -pole_width, height, -bridge_width + pole_width },
        {  pole_width, height, -bridge_width + pole_width },
        {  pole_width, height,  bridge_width - pole_width },
        { -pole_width, height,  bridge_width - pole_width },
        { -pole_width, height - (pole_width * 2), -bridge_width + pole_width },
        {  pole_width, height - (pole_width * 2), -bridge_width + pole_width },
        {  pole_width, height - (pole_width * 2),  bridge_width - pole_width },
        { -pole_width, height - (pole_width * 2),  bridge_width - pole_width },
    };
    // 両端の柱をつなぐ柱
    glPushMatrix();
    glTranslated(x, 0, 0);
    glBegin(GL_QUADS);
    for (i = 0; i < 6; i++)
    {
        for (j = 0; j < 4; j++)
        {
            glVertex3dv(v2[face[i][j]]);
        }
    }
    glEnd();
    glPopMatrix();
}


void brid_cable(int i, float floor_height)
{
    const float cable_radius1 = 0.015f;    // メインケーブルの太さ
    const float cable_radius2 = 0.005f;    // メインケーブルの太さ

    // 緑のライト
    if (light_status == 1)
    {
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, green);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, green);
        glPushMatrix();
        glTranslated(parabola_verts[i][0], parabola_verts[i][1], parabola_verts[i][2]);
        glScaled(0.025, 0.025, 0.025);
        draw_cube();
        glPopMatrix();
        glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, black);
    }

    brid_material();
    int j = 2 * i;
    // ケーブル
    draw_pole('x', parabola_verts[i], parabola_verts[i + 1], cable_radius1, 8);    // メインケーブル
    draw_pole('y', road_verts[j], road_verts[j + 1], cable_radius2, 8);    // 縦方向のケーブル
    // 1階と2階の間の柱
    GLdouble v[][3] = {
        { road_verts[j][0], road_verts[j][1], road_verts[j][2] },
        { road_verts[j][0], road_verts[j][1] - floor_height, road_verts[j][2] },
        { road_verts[j + 2][0], road_verts[j + 2][1], road_verts[j + 2][2] },
        { road_verts[j + 2][0], road_verts[j + 2][1] - floor_height, road_verts[j + 2][2] }
    };
    draw_pole('x', v[i % 2], v[(i + 1) % 2 + 2], cable_radius1 / 2, 4);
    draw_pole('x', v[i % 2], v[i % 2 + 1], cable_radius1 / 2, 4);
}


// 橋を描画する
void draw_bridge()
{
    const double bridge_width = 0.2f;      // 橋の幅
    double floor_diff = 0.1;    // 1階と2階の高さの差

    int i, k;
    // 登録された頂点を用いて中央のアーチ部分の描画
    for (k = -1; k < 2; k += 2)
    {
        glPushMatrix();
        glTranslated(0, 0, k * bridge_width);
        for (i = 0; i < parabola_verts_num - 1; i++)
        {
            brid_cable(i, (float)floor_diff);
        }
        glPopMatrix();
    }

    // 橋の両端部分の描画
    glPushMatrix();
    glTranslated(-road_verts[0][0] * 2, 0, 0);
    for (k = -1; k < 2; k += 2)
    {
        glPushMatrix();
        glTranslated(0, 0, k * bridge_width);
        for (i = 0; i < parabola_verts_num / 2; i++)
        {
            brid_cable(i, (float)floor_diff);
        }
        glPopMatrix();
    }
    glPopMatrix();

    glPushMatrix();
    glTranslated(road_verts[0][0] * 2, 0, 0);
    for (k = -1; k < 2; k += 2)
    {
        glPushMatrix();
        glTranslated(0, 0, k * bridge_width);
        for (i = parabola_verts_num / 2; i < parabola_verts_num - 1; i++)
        {
            brid_cable(i, (float)floor_diff);
        }
        glPopMatrix();
    }
    glPopMatrix();

    // 柱を描画
    brid_material();
    bridge_pole(road_verts[0][0], bridge_width, parabola_verts[0][1]);
    bridge_pole(-road_verts[0][0], bridge_width, parabola_verts[0][1]);

    // 道路を描画
    int end = 2 * parabola_verts_num - 2;
    GLdouble v1[] = { road_verts[0][0] * 2, road_verts[0][1], road_verts[0][2] - bridge_width };
    GLdouble v2[] = { road_verts[0][0] * 2, road_verts[0][1], road_verts[0][2] + bridge_width };
    GLdouble v3[] = { road_verts[end][0] * 2, road_verts[end][1], road_verts[end][2] + bridge_width };
    GLdouble v4[] = { road_verts[end][0] * 2, road_verts[end][1], road_verts[end][2] - bridge_width };
    for (k = 0; k < 2; k++)    // 2階建て道路
    {
        glPushMatrix();
        glTranslated(0, k * -floor_diff, 0);
        brid_material();
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, dark);
        glBegin(GL_QUADS);
        //GLdouble normal[] = { 0, 1, 0 }; glNormal3dv(normal);    // 法線
        glVertex3dv(v1); glVertex3dv(v2); glVertex3dv(v3); glVertex3dv(v4);
        glEnd();
        // 道路の周りの柱
        brid_material();
        draw_pole('x', v1, v4, 0.02, 4);
        draw_pole('x', v3, v2, 0.02, 4);
        glPopMatrix();
    }

    // ライトアップ用ライトの設定
    GLfloat center1[] = { (float)road_verts[0][0], 0.1f, 0, 1 };
    GLfloat center2[] = { -(float)road_verts[0][0], 0.1f, 0, 1 };
    GLfloat light_dir[] = { 0, 1, 0 };

    glLightfv(GL_LIGHT1, GL_POSITION, center1);
    glLightfv(GL_LIGHT2, GL_POSITION, center2);
}


void idle(void)
{
    // void glutPostRedisplay(void)
      // 再描画イベントを発生させる
      // このイベントの発生が発生すると, glutDisplayFunc()で指定されている描画関数が実行される
    glutPostRedisplay();
}


void display(void)
{
    // デプスバッファを消去？
    // glClear関数はバッファーをクリアする関数のよう
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /** モデルビュー変換行列を初期化 **/
    glLoadIdentity();

    /* ここで光源の位置を設定すると光源は視点と一緒に移動
     * 光源の方向を(0, 0, -1, 0)とすると自動車のヘッドライトのように */

     /** ビューイング変換 **/
     /* 視点位置と視線方向 */
    cam_rotate.move();

    /* ここで光源の位置を設定すると光源は図形の位置や視点によらず一定に */

    /* 光源の位置設定 */
    glLightfv(GL_LIGHT0, GL_POSITION, light0pos);

    /** モデリング変換 **/
    /* 図形の回転 */
    /* ここで光源の位置を設定すると、光源の位置は図形と一緒に移動 */
    
    brid_material();
    draw_bridge();

    seaPlane.scene();

    // ダブルバッファリングの2つのバッファを交換
    glutSwapBuffers();
}

void resize(int w, int h)
{
    glViewport(0, 0, w, h);

    /* 透視変換行列の設定 */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (double)w / (double)h, 1.0, 100.0);

    /* モデルビュー変換行列の設定 */
    glMatrixMode(GL_MODELVIEW);
}

void mouse_motion(int x, int y)
{
    cam_rotate.set(x, y);
}

void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN)
    {
        glutIdleFunc(idle);    // アニメーション開始
        switch (button)
        {
        case GLUT_LEFT_BUTTON:
            cam_rotate.set_mouse_status(1, x, y);
            break;
        case GLUT_MIDDLE_BUTTON:
            cam_rotate.set_mouse_status(2, x, y);
            break;
        case GLUT_RIGHT_BUTTON:
            cam_rotate.set_mouse_status(3, x, y);
            break;
        default:
            break;
        }
    }
    else
    {
        glutIdleFunc(0);    // アニメーション停止
        cam_rotate.set_mouse_status(0, x, y);    // マウスで動くの停止
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 'q':
    case 'Q':
    case '\033':  /* '\033' は ESC の ASCII コード */
        exit(0);

    case ' ':
        light_status++;
        if (light_status > 1) { light_status = 0; }
        printf("Light changed\n");
        if (light_status == 0)
        {
            glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
            glDisable(GL_LIGHT1);
            glDisable(GL_LIGHT2);
            glClearColor(sky_blue[0], sky_blue[1], sky_blue[2], sky_blue[3]);
        }
        else
        {
            glLightfv(GL_LIGHT0, GL_DIFFUSE, gray);
            glEnable(GL_LIGHT1);
            glEnable(GL_LIGHT2);
            glClearColor(sky_dark[0], sky_dark[1], sky_dark[2], sky_dark[3]);
        }
        break;

    default:
        break;
    }
}

void init(void)
{
    glClearColor(sky_blue[0], sky_blue[1], sky_blue[2], sky_blue[3]);

    // デプスバッファ（隠面消去処理のため）を使う（重くなるため必要なときだけ）
    glEnable(GL_DEPTH_TEST);

    // 陰影付けを有効にする
    glEnable(GL_LIGHTING);
    // GL_LIGHT0（0番目、必ず用意されている）を有効にする
    glEnable(GL_LIGHT0);

    // 以下teapots.cより
    GLfloat ambient[] = { 0.05f, 0.05f, 0.05f, 1.0f };
    GLfloat lmodel_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat local_view[] = { 0.0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

    GLfloat spot_dir[] = { 0, 1, 0 };
    GLfloat spot_diffuse[] = {3, 3, 3, 1};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, spot_diffuse);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, spot_diffuse);
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spot_dir);
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_dir);
    glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60.0);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 60.0);

    glEnable(GL_AUTO_NORMAL);    // 法線を自動計算
    glEnable(GL_NORMALIZE);
    glDepthFunc(GL_LESS);

    calc_bridge_vertices();    // 橋の頂点を計算

    seaPlane.init();
}

int main(int argc, char* argv[])
{
    glutInit(&argc, argv);
    // ダブルバッファリングの指定を追加
    // デプスバッファも指定
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(640, 480);
    glutCreateWindow(argv[0]);
    glutDisplayFunc(display);
    glutReshapeFunc(resize);
    glutMouseFunc(mouse);
    glutMotionFunc(mouse_motion);

    glutKeyboardFunc(keyboard);
    init();
    glutMainLoop();
    return 0;
}
