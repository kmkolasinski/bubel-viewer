#ifndef CAMERA_H
#define CAMERA_H


#include <iostream>
#include <cmath>
#include <QMatrix4x4>
#include <QVector3D>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

using namespace std;

class Camera {
public:
    Camera();

    ~Camera();

    QMatrix4x4 updateCamera(void);

    void toggleFreeCamera(bool free);

    bool isFreeCamera() { return isFree; }

    void mouseWheelMove(int direction);

    void reset();

    void moveForward(float speed);  // ruch do przodu - zwykle klawisz W
    void moveBackward(float speed); // ruch do tylu - klawisz S
    void moveLeft(float speed);     // ruch w lewo - zwykle klawisz A
    void moveRight(float speed);    // ruch w prawo - zwykle klawisz D
    void moveUp(float speed);       // ruch do gory
    void moveDown(float speed);     // ruch w dol

    void rotateView(float delta_x, float delta_y);

    QVector3D get_world_position() { return QVector3D(position.x(), -position.z(), position.y()); }

    QVector3D get_position();

    QVector3D get_world_direction() { return QVector3D(direction.x(), -direction.z(), direction.y()); }

    QVector3D get_world_up() { return QVector3D(updown_direction.x(), -updown_direction.z(), updown_direction.y()); }

    void setMouseSensitivity(int value);

public:
    QMatrix3x3 rotM;
    QVector3D position;           // observer position
    QVector3D direction;          // view direction
    QVector3D side_direction;     // camera side view direction
    QVector3D updown_direction;
    float radius; // for isFree rotation moevement

private:
    float key_sens;   // key sensitivity
    float mouse_sens; // mouse
    bool isFree;  // whether we can move in the 3D with WSAD keys or not

    float rot_angles[3];
};

#endif // CAMERA_H
