/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>
#include <QtOpenGL>

#include <math.h>

#include "glwidget.h"


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLWidget::GLWidget(QWidget *parent)
        : QGLWidget(QGLFormat(QGL::SampleBuffers), parent) {

    xPosition = 0;
    yPosition = 0;
    zPosition = 0;
    qtPurple = QColor(100, 100, 100);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainPlain = MAIN_PLAIN_XY;
    bUseSettingsPerFlag = false;
    zoom = 1.0;
    bUseOrtho = false;
    bPickAtom = false;
    bCompileDisplayList = true;
    bHideConnections = false;
    bHideAtoms = false;
    selectedDataColumn = 0;
    selectedDataSpin = 0;
    bUseLines = false;

    selectedAtomA = -1;
    selectedAtomB = -1;
    bSelectedAtomsConnected = false;
    bSwapAtomsPressed = false;

    camera.position = QVector3D(0, 0, 0);
    camera.radius = 5;
    camera.setMouseSensitivity(200);
    setFocus();
    qApp->installEventFilter(this);
}

GLWidget::~GLWidget() {

}

QSize GLWidget::minimumSizeHint() const {
    return QSize(50, 50);
}

QSize GLWidget::sizeHint() const {
    return QSize(600, 600);
}


void GLWidget::setXPosition(double angle) {
    xPosition = angle;    
    emit xPositionChanged(angle);
    updateGL();
}

void GLWidget::setYPosition(double angle) {
    yPosition = angle;
    emit yPositionChanged(angle);
    updateGL();

}


void GLWidget::initializeGL() {
    qglClearColor(qtPurple.dark());

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_MULTISAMPLE);

    int no_lights = 1;
    for (int light = 0; light < no_lights; light++) {
        glEnable(GL_LIGHT0 + light);
        float rad = 15.0;
        float x = ((light == 1) ? rad : 0.0 + (light == 2) ? -rad : 0.0);
        float y = ((light == 3) ? rad : 0.0 + (light == 4) ? -rad : 0.0);
        GLfloat lightPosition[4] = {x, y, 10.0, 1.0};
        float ampd = 0.7 / (no_lights);
        GLfloat lightDiffuse[4] = {ampd, ampd, ampd, 1.0};
        float ampa = 1.0 / no_lights;
        GLfloat lightAmbient[4] = {ampa, ampa, ampa, 1.0};

        glLightfv(GL_LIGHT0 + light, GL_POSITION, lightPosition);
        glLightfv(GL_LIGHT0 + light, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0 + light, GL_AMBIENT, lightAmbient);
    }

}


void renderCylinder(double x1, double y1, double z1, double x2, double y2, double z2, double radius, int subdivisions,
                    GLUquadricObj *quadric, double radius2 = 0.0) {

// This is the default direction for the cylinders to face in OpenGL
    QVector3D z = QVector3D(0, 0, 1);
// Get diff between two points you want cylinder along
    QVector3D p;
    if (qAbs(x1 - x2) + qAbs(y1 - y2) < 1.0e-4) {
        p = (QVector3D(x1 + 0.0001, y1, z1) - QVector3D(x2, y2, z2));
    } else p = (QVector3D(x1, y1, z1) - QVector3D(x2, y2, z2));


// Get CROSS product (the axis of rotation)
    QVector3D t = QVector3D::crossProduct(z, p);

// Get angle. LENGTH is magnitude of the vector
    double angle = 180.0 / M_PI * acos((QVector3D::dotProduct(z, p) / p.length()));

    glPushMatrix();
    glTranslated(x2, y2, z2);
    glRotated(angle, t.x(), t.y(), t.z());

    gluQuadricOrientation(quadric, GLU_OUTSIDE);

    if (radius2 == 0) radius2 = radius;

    gluCylinder(quadric, radius2, radius, p.length(), subdivisions, 1);
    glPopMatrix();

}

QVector3D HSL2RGB(float h, float sl, float l) {
    float v;
    float r, g, b;

    r = l;   // default to gray
    g = l;
    b = l;
    v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
    if (v > 0) {
        float m;
        float sv;
        int sextant;
        float fract, vsf, mid1, mid2;

        m = l + l - v;
        sv = (v - m) / v;
        h *= 6.0;
        sextant = int(h);
        fract = h - sextant;
        vsf = v * sv * fract;
        mid1 = m + vsf;
        mid2 = v - vsf;
        if (sextant == 0) {
            r = v;
            g = mid1;
            b = m;
        } else if (sextant == 1) {
            r = mid2;
            g = v;
            b = m;
        } else if (sextant == 2) {
            r = m;
            g = v;
            b = mid1;
        } else if (sextant == 3) {
            r = m;
            g = mid2;
            b = v;
        } else if (sextant == 4) {
            r = mid1;
            g = m;
            b = v;
        } else {
            r = v;
            g = m;
            b = mid2;
        }
    }
    return QVector3D(r, g, b);
}

void processHits(GLint hits, GLuint buffer[]) {

    GLuint names, *ptr;

    ptr = (GLuint *) buffer;
    if (hits < 0) return;
    for (int i = 0; i < hits; i++) { /*  for each hit  */
        names = *ptr;
        printf(" number of names for hit = %d\n", names);
        ptr++;
        printf("  z1 is %g;", (float) *ptr / 0x7fffffff);
        ptr++;
        printf(" z2 is %g\n", (float) *ptr / 0x7fffffff);
        ptr++;
        printf("   the name is ");
        for (int j = 0; j < names; j++) {     /*  for each name */
            printf("%d ", *ptr);
            ptr++;
        }
        printf("\n");
    }
}

#define BUFSIZE 512

//! [7]
void GLWidget::paintGL() {
    static GLuint displayIndex = glGenLists(1);
    static GLuint displayIndexCnts = glGenLists(1);
    static GLuint displayIndexLeads = glGenLists(1);
    makeCurrent();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    int viewport[4] = {0, 0, width(), height()};
    if (bPickAtom)gluPickMatrix(lastPos.x(), height() - lastPos.y(), 1, 1, &viewport[0]);

    double s = zoom;
    if (bUseOrtho)
        glOrtho(-0.5 * s * ratio, 0.5 * s * ratio, -0.5 * s, 0.5 * s, 1.0, 25.0);
    else
        glFrustum(-0.5 * s * ratio, 0.5 * s * ratio, -0.5 * s, 0.5 * s, 4.0, 25.0);


    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();


    QMatrix4x4 rotmat = camera.updateCamera();
    glMultMatrixf(rotmat.data());
    glRotatef(180, 0, 1, 0);

    switch (mainPlain) {
        case (MAIN_PLAIN_XY):
            break;
        case (MAIN_PLAIN_XZ):
            glRotatef(90.0, 0.0, 1.0, 0.0);
            break;
        case (MAIN_PLAIN_YZ):
            glRotatef(90.0, -1.0, 0.0, 0.0);
            break;
    }

    glTranslatef(-xPosition, -yPosition, -zPosition);

    GLfloat d1[4] = {0.4, 0.5, 0.9, 1.0};
    GLfloat d2[4] = {0.9, 0.5, 0.6, 1.0};
    GLfloat d3[4] = {0.4, 0.9, 0.2, 1.0};
    GLfloat d4[4] = {displayAllSettings.color.redF(),
                     displayAllSettings.color.greenF(),
                     displayAllSettings.color.blueF(), 1.0};
    GLfloat d5[4] = {displayConnections.color.redF(),
                     displayConnections.color.greenF(),
                     displayConnections.color.blueF(), 1.0f};
    GLfloat d6[4] = {0.8, 0.7, 3.0, 1.0};
    GLfloat d7[4] = {0.4, 0.4, 0.4, 1.0};
    GLfloat red[4] = {1.0, 0.0, 0.0, 1.0};
    GLfloat grn[4] = {0.0, 1.0, 0.0, 1.0};
    GLfloat orn[4] = {1.0, 0.5, 0.0, 1.0};
    glMaterialfv(GL_FRONT, GL_DIFFUSE, d4);


    if (bCompileDisplayList) {

        glNewList(displayIndex, GL_COMPILE);

        glInitNames();
        glPushName(0);
        GLUquadricObj *quadric = gluNewQuadric();

        double radius = displayAllSettings.atom_size * DataReader::atoms_stats.scale * 0.3;
        int no_subdivs = displayAllSettings.atom_quality;
        glMaterialfv(GL_FRONT, GL_DIFFUSE, d4);

        //
        // draw active atoms
        if (data->dataname == "null" || !displayAllSettings.bShowDataValues) {

            for (unsigned int i = 0; i < atoms->size(); i++) {


                Atom &atom = (*atoms)[i];
                if (!atom.active) continue;
                glPushMatrix();
                glLoadName(i);
                if (bUseSettingsPerFlag) {
                    int id = flag2id[atom.flag];
                    if (displayPerFlag[id].bHide) continue;
                    radius = displayPerFlag[id].atom_size * DataReader::atoms_stats.scale * 0.3;
                    no_subdivs = displayPerFlag[id].atom_quality;

                    GLfloat df[4] = {displayPerFlag[id].color.redF(),
                                     displayPerFlag[id].color.greenF(),
                                     displayPerFlag[id].color.blueF(), 1.0};
                    glMaterialfv(GL_FRONT, GL_DIFFUSE, df);
                }


                glTranslated(atom.pos.x(), atom.pos.y(), atom.pos.z());
                gluSphere(quadric, radius, no_subdivs, no_subdivs);
                glPopMatrix();
            }

        } else { // draw data values
            double min_value = data->min_values[selectedDataColumn];
            double max_value = data->max_values[selectedDataColumn];
            if (atoms->size() == 0) {
                QMessageBox::critical(NULL, "Atoms data is empty!",
                                      "Load file with atoms positions to see data values.",
                                      QMessageBox::Ok);
                bCompileDisplayList = false;
                glEndList();
                return;
            }
            for (unsigned int i = 0; i < data->atomIds.size(); i++) {

                if (data->spinIds[i] != selectedDataSpin) continue;
                Atom &atom = (*atoms)[data->atomIds[i]];


                if (flag2id[atom.flag] == selectDataFlag - 1) continue;

                if (bUseSettingsPerFlag) {
                    int id = flag2id[atom.flag];
                    if (displayPerFlag[id].bHide) continue;
                }

                glLoadName(data->atomIds[i]);
                double value = data->values[i][selectedDataColumn];
                value = qMax((value - min_value) / (max_value - min_value), 0.0);

                value = qMax(0.0, (value - displayAllSettings.min_cutoff) /
                                  (displayAllSettings.max_cutoff - displayAllSettings.min_cutoff));

                QVector3D color = HSL2RGB(value * 0.5, 1.0, 0.5);
                glPushMatrix();
                GLfloat df[4] = {color.x(), color.y(), color.z(), 1.0};
                glMaterialfv(GL_FRONT, GL_DIFFUSE, df);
                glTranslated(atom.pos.x(), atom.pos.y(), atom.pos.z());
                gluSphere(quadric, radius, no_subdivs, no_subdivs);
                glPopMatrix();
            }


        }// end of data set
        // draw not active atoms
        glMaterialfv(GL_FRONT, GL_DIFFUSE, d7);

        for (unsigned int i = 0; i < atoms->size(); i++) {

            Atom &atom = (*atoms)[i];
            if (atom.active) continue;
            glLoadName(i);
            glPushMatrix();
            glTranslated(atom.pos.x(), atom.pos.y(), atom.pos.z());
            gluSphere(quadric, radius, no_subdivs, no_subdivs);
            glPopMatrix();
        }

        glEndList();
        glNewList(displayIndexCnts, GL_COMPILE);


        radius = displayConnections.atom_size * DataReader::atoms_stats.scale * 0.3;
        no_subdivs = displayConnections.atom_quality;

        glMaterialfv(GL_FRONT, GL_DIFFUSE, d5);
        if (bUseLines) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_LINE_SMOOTH);
            glEnable(GL_POLYGON_SMOOTH);
            glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
            glLineWidth(radius * 500.0);
            glDisable(GL_LIGHTING);
            glBegin(GL_LINES);
            glColor3fv(d5);
        }
        int lastA = -1;
        int lastB = -1;
        int no_calls = 0;
        for (unsigned int i = 0; i < cnts->size(); i++) {
            AtomConnection &cnt = (*cnts)[i];
            Atom &atomA = (*atoms)[cnt.atomA];
            Atom &atomB = (*atoms)[cnt.atomB];
            if (cnt.atomA <= cnt.atomB) continue;
            if (cnt.atomA != lastA || cnt.atomB != lastB) {
                lastA = cnt.atomA;
                lastB = cnt.atomB;
            } else continue;
            no_calls++;

            if (bUseLines) {

                glVertex3d(atomA.pos.x(), atomA.pos.y(), atomA.pos.z());
                glVertex3d(atomB.pos.x(), atomB.pos.y(), atomB.pos.z());
            } else
                renderCylinder(atomA.pos.x(), atomA.pos.y(), atomA.pos.z(), atomB.pos.x(), atomB.pos.y(), atomB.pos.z(),
                               0.2 * radius, no_subdivs, quadric);

        }
        if (bUseLines) {

            glEnd();
            glEnable(GL_LIGHTING);
        }


        glEndList();
        glNewList(displayIndexLeads, GL_COMPILE);


        glMaterialfv(GL_FRONT, GL_DIFFUSE, d4);

        radius = displayAllSettings.atom_size * DataReader::atoms_stats.scale * 0.3;
        no_subdivs = displayAllSettings.atom_quality;

        glInitNames();
        glPushName(0);

        for (unsigned int l = 0; l < leads->size(); l++) {
            Lead &lead = (*leads)[l];
            if (!lead.bShowLeadAtoms) continue;


            glMaterialfv(GL_FRONT, GL_DIFFUSE, d1);
            for (unsigned int i = 0; i < lead.atoms.size(); i++) {
                glPushMatrix();
                Atom &atom = (*atoms)[lead.atoms[i]];
                if (bUseSettingsPerFlag) {
                    int id = flag2id[atom.flag];
                    radius = displayPerFlag[id].atom_size * DataReader::atoms_stats.scale * 0.3;
                    no_subdivs = displayPerFlag[id].atom_quality;
                }
                glLoadName(lead.atoms[i]);
                glTranslated(atom.pos.x(), atom.pos.y(), atom.pos.z());
                gluSphere(quadric, radius * 1.1, 5 + no_subdivs, 5 + no_subdivs);
                glPopMatrix();
            }
            glMaterialfv(GL_FRONT, GL_DIFFUSE, d2);
            for (unsigned int i = 0; i < lead.nex_atoms.size(); i++) {
                glPushMatrix();
                Atom &atom = (*atoms)[lead.nex_atoms[i]];
                if (bUseSettingsPerFlag) {
                    int id = flag2id[atom.flag];
                    radius = displayPerFlag[id].atom_size * DataReader::atoms_stats.scale * 0.3;
                    no_subdivs = displayPerFlag[id].atom_quality;
                }
                glLoadName(lead.nex_atoms[i]);
                glTranslated(atom.pos.x(), atom.pos.y(), atom.pos.z());
                gluSphere(quadric, radius * 1.1, 5 + no_subdivs, 5 + no_subdivs);
                glPopMatrix();
            }
            glMaterialfv(GL_FRONT, GL_DIFFUSE, d3);
            double cradius = displayConnections.atom_size * DataReader::atoms_stats.scale * 0.3;
            int cno_subdivs = displayConnections.atom_quality;
            for (unsigned int i = 0; i < lead.cnts.size(); i++) {
                AtomConnection &cnt = lead.cnts[i];
                Atom &atomA = (*atoms)[cnt.atomA];
                Atom &atomB = (*atoms)[cnt.atomB];
                renderCylinder(atomA.pos.x(), atomA.pos.y(), atomA.pos.z(), atomB.pos.x(), atomB.pos.y(), atomB.pos.z(),
                               0.25 * cradius, 5 + cno_subdivs, quadric);
            }
            glMaterialfv(GL_FRONT, GL_DIFFUSE, d6);
            for (unsigned int i = 0; i < lead.inner_cnts.size(); i++) {
                AtomConnection &cnt = lead.inner_cnts[i];
                Atom &atomA = (*atoms)[cnt.atomA];
                Atom &atomB = (*atoms)[cnt.atomB];
                renderCylinder(atomA.pos.x(), atomA.pos.y(), atomA.pos.z(), atomB.pos.x(), atomB.pos.y(), atomB.pos.z(),
                               0.27 * cradius, 5 + cno_subdivs, quadric);
            }
            if (!lead.bShowArea) continue;

            if (lead.shape.type == SHAPE_NONE) continue;
            // this is stupid :/
            lead.shape.indices[0][0] = 0;
            lead.shape.indices[0][1] = 1;
            lead.shape.indices[1][0] = 1;
            lead.shape.indices[1][1] = 2;
            lead.shape.indices[2][0] = 2;
            lead.shape.indices[2][1] = 3;
            lead.shape.indices[3][0] = 3;
            lead.shape.indices[3][1] = 0;

            lead.shape.indices[4][0] = 4;
            lead.shape.indices[4][1] = 5;
            lead.shape.indices[5][0] = 5;
            lead.shape.indices[5][1] = 6;
            lead.shape.indices[6][0] = 6;
            lead.shape.indices[6][1] = 7;
            lead.shape.indices[7][0] = 7;
            lead.shape.indices[7][1] = 4;

            lead.shape.indices[8][0] = 0;
            lead.shape.indices[8][1] = 4;
            lead.shape.indices[9][0] = 1;
            lead.shape.indices[9][1] = 5;
            lead.shape.indices[10][0] = 2;
            lead.shape.indices[10][1] = 6;
            lead.shape.indices[11][0] = 3;
            lead.shape.indices[11][1] = 7;

            for (int c = 0; c < 12; c++) {

                int idA = lead.shape.indices[c][0];
                int idB = lead.shape.indices[c][1];
                double scale = DataReader::atoms_stats.scale * 0.02;
                scale = qMax(scale, 0.0005);
                renderCylinder(lead.shape.data[idA].x(), lead.shape.data[idA].y(), lead.shape.data[idA].z(),
                               lead.shape.data[idB].x(), lead.shape.data[idB].y(), lead.shape.data[idB].z(),
                               scale, 10, quadric);

            }

        }


        gluDeleteQuadric(quadric);
        bCompileDisplayList = false;
        glEndList();
    } // end of compile display list;

    // ---------------------------------------------------------------
    if (!bHideAtoms)glCallList(displayIndex);
    glCallList(displayIndexLeads);


    if (!bPickAtom) {
        if (atoms->size() == 0) return;
        if (!bHideConnections)glCallList(displayIndexCnts);


        GLUquadricObj *quadric = gluNewQuadric();
        double radius = displayAllSettings.atom_size * DataReader::atoms_stats.scale * 0.3;

        if (selectedAtomA > -1) {
            glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
            Atom &atom = (*atoms)[selectedAtomA];
            glPushMatrix();

            glTranslated(atom.pos.x(), atom.pos.y(), atom.pos.z());
            gluSphere(quadric, radius * 1.16, 25, 25);
            glPopMatrix();

            int atomID = selectedAtomA;
            int cntID = atom.cnt_ptr;
            do {
                AtomConnection &cnt = (*cnts)[cntID--];
                atomID = cnt.atomA;
                if (atomID != selectedAtomA) break;
                Atom &atom = (*atoms)[cnt.atomB];
                glPushMatrix();
                glMaterialfv(GL_FRONT, GL_DIFFUSE, orn);
                glTranslated(atom.pos.x(), atom.pos.y(), atom.pos.z());
                gluSphere(quadric, radius * 1.155, 25, 25);
                glPopMatrix();


                if (cntID == 0) break;
            } while (atomID == selectedAtomA);


        }
        if (selectedAtomB > -1) {
            glMaterialfv(GL_FRONT, GL_DIFFUSE, grn);
            Atom &atom = (*atoms)[selectedAtomB];
            glPushMatrix();

            glTranslated(atom.pos.x(), atom.pos.y(), atom.pos.z());
            gluSphere(quadric, radius * 1.17, 25, 25);
            glPopMatrix();
        }
        radius = displayConnections.atom_size * DataReader::atoms_stats.scale * 0.3;
        if (bSelectedAtomsConnected) {
            glMaterialfv(GL_FRONT, GL_DIFFUSE, red);
            Atom &atomA = (*atoms)[selectedAtomA];
            Atom &atomB = (*atoms)[selectedAtomB];

            renderCylinder(atomB.pos.x(), atomB.pos.y(), atomB.pos.z(), atomA.pos.x(), atomA.pos.y(), atomA.pos.z(),
                           0.5 * radius, 15, quadric, 0.5 * radius);
        }

        gluDeleteQuadric(quadric);

    } // end of if picking

}

void GLWidget::resizeGL(int width, int height) {
    glViewport(0, 0, width, height);
    ratio = double(width) / height;
}

void GLWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        bMousePressed = true;
    } else if (event->button() == Qt::RightButton) {
        bSwapAtomsPressed = true;

    }
    lastPos = event->pos();
}


void GLWidget::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && bMousePressed) {
        bMousePressed = false;
        bPickAtom = true;
        GLuint selectBuf[BUFSIZE];
        GLint hits;
        glSelectBuffer(BUFSIZE, selectBuf);
        glRenderMode(GL_SELECT);
        updateGL();
        hits = glRenderMode(GL_RENDER);
        bPickAtom = false;
        GLuint *ptr;
        ptr = (GLuint *) selectBuf;
        int minid = 0, id = 0;
        float zmin = 1;
        // finding nearst one
        for (int i = 0; i < hits; i++) { /*  for each hit  */
            ptr++;
            float z1 = ((float) *ptr / 0x7fffffff);
            ptr++;
            ptr++;
            id = *ptr;
            ptr++;
            if (z1 < zmin) {
                minid = id;
                zmin = z1;
            }
        }
        selectBuf[3] = minid;

        if (hits > 0) {
            if (selectedAtomA == -1) selectedAtomA = selectBuf[3];
            else if (selectedAtomA > -1) selectedAtomB = selectBuf[3];
        } else {
            selectedAtomA = -1;
            selectedAtomB = -1;
        }
        emit selectedAtoms(selectedAtomA, selectedAtomB);
        bSelectedAtomsConnected = false;
        for (unsigned int i = 0; i < cnts->size(); i++) {
            AtomConnection &cnt = (*cnts)[i];
            if (cnt.atomA == selectedAtomA && cnt.atomB == selectedAtomB) {
                bSelectedAtomsConnected = true;
                break;
            }
            if (cnt.atomA == selectedAtomB && cnt.atomB == selectedAtomA) {
                bSelectedAtomsConnected = true;
                break;
            }
        }

    } else if (event->button() == Qt::RightButton && bSwapAtomsPressed) {

        int itmp = selectedAtomA;
        selectedAtomA = selectedAtomB;
        selectedAtomB = itmp;
        emit selectedAtoms(selectedAtomA, selectedAtomB);

    }
    updateGL();

}


void GLWidget::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();
    if (qAbs(dx) + qAbs(dy) > 1) {
        bMousePressed = false;
        bSwapAtomsPressed = false;
    }
    if (event->buttons() & Qt::LeftButton) {
        camera.rotateView(0.2 * dx, 0.2 * dy);
        updateGL();
    } else if (event->buttons() & Qt::RightButton) {

        setXPosition(xPosition - 0.003 * dx * zoom);
        setYPosition(yPosition + 0.003 * dy * zoom);
        updateGL();

    } else if (event->buttons() & Qt::MiddleButton) {
        zoom += (dx + dy) / 200.0 * zoom;
        zoom = min(max(0.001, zoom), 2.0);
        resizeGL(width(), height());
        updateGL();
    }

    lastPos = event->pos();
}


void GLWidget::wheelEvent(QWheelEvent *event) {
    float numDegrees = event->delta() / 100.0;

    zoom += numDegrees / 10.0 * zoom;
    zoom = min(max(0.001, zoom), 2.0);
    resizeGL(width(), height());
    updateGL();
    event->accept();
}

bool GLWidget::eventFilter(QObject *o, QEvent *e) {

    if (e->type() == QEvent::KeyPress) {
        if (((QKeyEvent *) e)->key() == Qt::Key_S) {
            keyPressEvent((QKeyEvent *) e);
            return true;
        } else {
            // standard event processing
            return QObject::eventFilter(o, e);
        }
    }
    return false;
}

void GLWidget::keyPressEvent(QKeyEvent *event) {

    if (event->key() == Qt::Key_S) {

        if (selectedAtomA > -1) {
            qDebug() << "Selecting camera position at atom: " << selectedAtomA;
            Atom &atomA = (*atoms)[selectedAtomA];
            xPosition = atomA.pos.x();
            yPosition = atomA.pos.y();
            zPosition = atomA.pos.z();
        } else {
            zPosition = 0;
        }
        updateGL();
    }
}

