#include "camera.h"


Camera::Camera() {
    position = QVector3D(0, 0, 0);
    direction = QVector3D(0, 0, 1);
    side_direction = QVector3D(1, 0, 0);
    updown_direction = QVector3D(0, 1, 0);

    key_sens = 5.0f;
    mouse_sens = 0.3f;
    rot_angles[0] = rot_angles[1] = rot_angles[2] = 0;
    rotM.setToIdentity();
    rot_angles[0] = 0;
    rot_angles[1] = 0;
    isFree = false;
    radius = 1.0;

}

Camera::~Camera() {
}


QMatrix4x4 Camera::updateCamera(void) {
    QMatrix4x4 unit_mat;
    unit_mat.setToIdentity();
    if (isFree) {
        unit_mat.lookAt(position,
                        position + direction,
                        updown_direction
        );

    } else {

        unit_mat.lookAt(position - (radius) * direction,
                        position - (radius) * direction + direction,
                        updown_direction
        );
    }

    return unit_mat;
}

QVector3D Camera::get_position() {
    if (isFree) {
        return position;
    } else {
        return position - (radius) * direction;
    }
}

void Camera::toggleFreeCamera(bool free) {
    isFree = free;
}

void Camera::moveForward(float speed) {

    if (isFree)position += direction * speed * key_sens;

}

void Camera::moveBackward(float speed) {

    if (isFree)position -= direction * speed * key_sens;

}

void Camera::moveLeft(float speed) {

    if (isFree)position += side_direction * speed * key_sens;

}

void Camera::moveRight(float speed) {

    if (isFree)position -= side_direction * speed * key_sens;

}

void Camera::moveUp(float speed) {

    if (isFree)position += updown_direction * speed * key_sens;

}

void Camera::moveDown(float speed) {

    if (isFree)position -= updown_direction * speed * key_sens;

}


void Camera::rotateView(float z_angle, float x_angle) {

    double cosPhi = cos(mouse_sens * (-z_angle) / 180 * M_PI);
    double sinPhi = sin(mouse_sens * (-z_angle) / 180 * M_PI);

    direction = QVector3D(cosPhi * direction.x() + sinPhi * direction.z(), direction.y(),
                          cosPhi * direction.z() - sinPhi * direction.x());

    QMatrix4x4 rotMat;
    rotMat.setToIdentity();
    rotMat.rotate(mouse_sens * (-x_angle), QVector3D::crossProduct(direction, QVector3D(0, 1, 0)));
    QVector3D tmpVec = (rotMat * QVector4D(direction)).toVector3D();
    tmpVec.normalize();
    double angleTheta = QVector3D::dotProduct(tmpVec, QVector3D(0, 1, 0));
    if (qAbs(angleTheta) < 0.9) {
        rotMat.setToIdentity();
        rotMat.rotate(mouse_sens * (-x_angle) * (1 - qAbs(angleTheta)),
                      QVector3D::crossProduct(direction, QVector3D(0, 1, 0)));
        QVector3D tmpVec = (rotMat * QVector4D(direction)).toVector3D();
        tmpVec.normalize();
        direction = tmpVec;
    }

    side_direction = QVector3D(cosPhi * side_direction.x() + sinPhi * side_direction.z(), 0,
                               cosPhi * side_direction.z() - sinPhi * side_direction.x());

    updown_direction = QVector3D::crossProduct(direction, side_direction);

    direction.normalize();
    side_direction.normalize();
    updown_direction.normalize();

}

void Camera::reset() {

    position = QVector3D(0, 0, 0);
    direction = QVector3D(0, 0, 1);
    side_direction = QVector3D(1, 0, 0);
    updown_direction = QVector3D(0, 1, 0);
    radius = 5;
}

void Camera::mouseWheelMove(int direction) {
    radius += mouse_sens * 0.0025f * direction;
    if (radius < 0.3) radius = 0.3;
}

void Camera::setMouseSensitivity(int value) {
    mouse_sens = value / 100.0;
}
