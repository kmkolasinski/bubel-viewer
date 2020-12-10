#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMimeData>
#include <glwidget.h>
#include <datareader.h>
#include <formleadparams.h>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);

    QSize sizeHint() const;

    ~MainWindow();

public
    slots:
            void receiveSpinBoxes(int);

    void receiveDoubleSpinBoxes(double);

    void receiveCheckBoxes(int);

    void updateWidgets();

    void toggleLead(unsigned int id, bool toggle);

    void toggleLeadArea(unsigned int id, bool toggle);

    void open();

    void close();

    void reopen();

    void resetPosition();

    void selectedAtomsInfo(int atomA, int atomB);

    void chooseColor();

    void chooseConnectionColor();

    void togglePerFlagDisplaySettings(bool);

    void updatePerFlagSettings(int);

    void dragEnterEvent(QDragEnterEvent *event);

    void dropEvent(QDropEvent *event);

private:
    bool bSkipSignals;
    bool bReopening;

    void updateGUI();

    Ui::MainWindow *ui;
    GLWidget *glWidget;
    DataReader xmlData;
    QString lastDir;


};

#endif // MAINWINDOW_H
