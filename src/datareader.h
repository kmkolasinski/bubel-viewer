#ifndef DATAREADER_H
#define DATAREADER_H

#include <iostream>
#include <vector>
#include <map>
#include <QVector3D>
#include <QDomDocument>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <string>
#include <sstream>

using namespace std;

struct Atom {
    QVector3D pos;
    int flag, flag0, flag1, active, no_states, no_bounds;

    int cnt_ptr;//id to connections list
    Atom() : flag(0), active(1), no_states(1), no_bounds(0) {
    }
};

struct AtomConnection {
    int atomA, atomB, spinA, spinB;
    double realC, imagC;

    AtomConnection() : atomA(0), atomB(0),
                       spinA(1), spinB(1),
                       realC(0), imagC(0) {
    }
};

enum LeadShapeType {
    SHAPE_NONE = 0,
    SHAPE_RECTANGLE_XY = 1,
    SHAPE_CONVEX_QUAD_XY = 2,
    SHAPE_RANGE_3D = 3
};

struct LeadShape {
    LeadShapeType type;
    QVector3D data[8];
    QVector3D otherData[2];
    unsigned int indices[12][2];

    LeadShape() {
        type = SHAPE_NONE;

        indices[0][0] = 0;
        indices[0][1] = 1;
        indices[1][0] = 1;
        indices[1][1] = 2;
        indices[2][0] = 2;
        indices[2][1] = 3;
        indices[3][0] = 3;
        indices[3][1] = 0;

        indices[4][0] = 4;
        indices[4][1] = 5;
        indices[5][0] = 5;
        indices[5][1] = 6;
        indices[6][0] = 6;
        indices[6][1] = 7;
        indices[7][0] = 7;
        indices[7][1] = 4;

        indices[8][0] = 0;
        indices[8][1] = 4;
        indices[9][0] = 1;
        indices[9][1] = 5;
        indices[10][0] = 2;
        indices[10][1] = 6;
        indices[11][0] = 3;
        indices[11][1] = 7;

    }
};


struct Lead {
    bool bShowLeadAtoms;
    bool bShowArea;
    LeadShape shape;
    vector<unsigned int> atoms;
    vector<unsigned int> nex_atoms;
    vector<AtomConnection> cnts;
    vector<AtomConnection> inner_cnts;

    Lead() : bShowLeadAtoms(true), bShowArea(true) {
    }
};

struct AtomData {
    QString dataname;
    vector<vector<double> > values;
    vector<double> max_values;
    vector<double> min_values;
    vector<int> spinIds;
    vector<int> atomIds;

    AtomData() : dataname("null") {
    }

    void clear() {
        values.clear();
        spinIds.clear();
        atomIds.clear();
        max_values.clear();
        min_values.clear();
        dataname = "null";
    }
};

struct AtomsStats {
    QVector3D mass_center;
    double ave_dist;
    double scale;
    QVector<int> flag_list;
    unsigned int no_atoms;
    int max_spin;
    QVector3D min_corner;
    QVector3D max_corner;
    double atom_radius;
    int filter_spin_A, filter_spin_B;

    AtomsStats() : mass_center(QVector3D(0, 0, 0)), ave_dist(0), no_atoms(0), max_spin(0), filter_spin_A(1),
                   filter_spin_B(1) {}
};

enum LoadingType {
    LOADING_FINISHED = -1,
    LOADING_STRUCTURE = 0,
    LOADING_LEADS = 2,
    LOADING_VALUES = 1
};

class DataReader {
public:
    DataReader();

    ~DataReader();

    void read_data(QString filename);

    void precalculate_data(bool bRecalculateVisualParams = true);

    void clear_data();

private:
    void read_atoms(QDomElement &root);

    void read_lattice(QDomElement &root);

    void read_connections(QDomElement &root);

    void read_lead(QDomElement &root);

    void read_atoms_data(QDomElement &root);

public:
    vector<Atom> atoms;
    vector<AtomConnection> connections;
    vector<Atom> p_atoms;
    vector<AtomConnection> p_connections;
    vector<Lead> leads;
    vector<Lead> p_leads;
    AtomData data;
    static AtomsStats atoms_stats;
    LoadingType loadingType;
};

#endif // DATAREADER_H
