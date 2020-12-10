#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow) {
    ui->setupUi(this);

    glWidget = new GLWidget;

    ui->horizontalLayoutGL->addWidget(glWidget);

    connect(ui->pushButtonOpen, SIGNAL(released()), this, SLOT(open()));
    connect(ui->pushButtonCloseFile, SIGNAL(released()), this, SLOT(close()));
    connect(ui->pushButtonReopenFile, SIGNAL(released()), this, SLOT(reopen()));

    QButtonGroup *group = new QButtonGroup(this);
    group->addButton(ui->radioButtonMainXY);
    group->addButton(ui->radioButtonMainXZ);
    group->addButton(ui->radioButtonMainYZ);
    connect(group, SIGNAL(buttonClicked(int)), this, SLOT(receiveCheckBoxes(int)));
    connect(ui->checkBoxOrthoProj, SIGNAL(stateChanged(int)), this, SLOT(receiveCheckBoxes(int)));

    connect(ui->doubleSpinBoxAtomSize, SIGNAL(valueChanged(double)), this, SLOT(receiveDoubleSpinBoxes(double)));
    connect(ui->spinBoxAtomQ, SIGNAL(valueChanged(int)), this, SLOT(receiveSpinBoxes(int)));
    connect(ui->pushButtonAtomColor, SIGNAL(released()), this, SLOT(chooseColor()));
    connect(ui->checkBoxHideAtoms, SIGNAL(stateChanged(int)), this, SLOT(receiveCheckBoxes(int)));

    connect(ui->checkBoxCntsHide, SIGNAL(stateChanged(int)), this, SLOT(receiveCheckBoxes(int)));
    connect(ui->checkBoxCntsUseLines, SIGNAL(stateChanged(int)), this, SLOT(receiveSpinBoxes(int)));
    connect(ui->doubleSpinBoxConnectionSize, SIGNAL(valueChanged(double)), this, SLOT(receiveDoubleSpinBoxes(double)));
    connect(ui->spinBoxConnectionQ, SIGNAL(valueChanged(int)), this, SLOT(receiveSpinBoxes(int)));
    connect(ui->pushButtonConnectionColour, SIGNAL(released()), this, SLOT(chooseConnectionColor()));

    connect(ui->checkBoxPerFlagSettings, SIGNAL(toggled(bool)), this, SLOT(togglePerFlagDisplaySettings(bool)));
    connect(ui->pushButtonHideAtom, SIGNAL(released()), this, SLOT(updateWidgets()));
    connect(ui->comboBoxFlags, SIGNAL(currentIndexChanged(int)), this, SLOT(updatePerFlagSettings(int)));

    connect(glWidget, SIGNAL(xPositionChanged(double)), ui->doubleSpinBoxRotX, SLOT(setValue(double)));
    connect(glWidget, SIGNAL(yPositionChanged(double)), ui->doubleSpinBoxRotY, SLOT(setValue(double)));
    connect(glWidget, SIGNAL(selectedAtoms(int, int)), this, SLOT(selectedAtomsInfo(int, int)));

    connect(ui->doubleSpinBoxRotX, SIGNAL(valueChanged(double)), glWidget, SLOT(setXPosition(double)));
    connect(ui->doubleSpinBoxRotY, SIGNAL(valueChanged(double)), glWidget, SLOT(setYPosition(double)));
    connect(ui->pushButtonResetRotations, SIGNAL(released()), this, SLOT(resetPosition()));

    // data vals
    connect(ui->comboBoxDatasetSpinFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(receiveSpinBoxes(int)));
    connect(ui->comboBoxDataSetFlagFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(receiveSpinBoxes(int)));
    connect(ui->listWidgetDatasetCols, SIGNAL(currentRowChanged(int)), this, SLOT(receiveSpinBoxes(int)));
    connect(ui->checkBoxDatasetUse, SIGNAL(stateChanged(int)), this, SLOT(receiveSpinBoxes(int)));
    connect(ui->horizontalSliderDataMinValue, SIGNAL(sliderReleased()), this, SLOT(updateWidgets()));
    connect(ui->horizontalSliderDataMaxValue, SIGNAL(sliderReleased()), this, SLOT(updateWidgets()));

    QHeaderView *header = ui->tableWidgetAtomInfo->horizontalHeader();
#if QT_VERSION <= QT_VERSION_CHECK(4, 8, 1)
    header->setResizeMode(QHeaderView::Stretch);
#else
    header->setSectionResizeMode(QHeaderView::Stretch);
#endif

    glWidget->atoms = &xmlData.p_atoms;
    glWidget->cnts = &xmlData.p_connections;
    glWidget->leads = &xmlData.p_leads;
    glWidget->data = &(xmlData.data);
    bSkipSignals = false;
    bReopening = false;

    updateGUI();

    glWidget->setAcceptDrops(true);
    setAcceptDrops(true);

}

MainWindow::~MainWindow() {
    delete glWidget;
    delete ui;
}

QSize MainWindow::sizeHint() const
{
    return QSize(1000, 650);
}


void MainWindow::updateGUI() {
    QString info;
    AtomsStats &stats = xmlData.atoms_stats;


    if (xmlData.loadingType != LOADING_VALUES) {
        bSkipSignals = true;

        if (!bReopening) {
            ui->doubleSpinBoxAtomSize->setValue(xmlData.atoms_stats.atom_radius);
            ui->doubleSpinBoxConnectionSize->setValue(xmlData.atoms_stats.atom_radius);

            ui->doubleSpinBoxAtomSize->setSingleStep(xmlData.atoms_stats.atom_radius / 10.0);
            ui->doubleSpinBoxConnectionSize->setSingleStep(xmlData.atoms_stats.atom_radius / 10.0);
        }

        while (ui->listWidgetLeads->count()) {
            QListWidgetItem *item = ui->listWidgetLeads->item(0);
            delete ui->listWidgetLeads->itemWidget(item);
            delete item;
        }

        for (unsigned i = 0; i < xmlData.leads.size(); i++) {
            FormLeadParams *lead = new FormLeadParams(i);
            QListWidgetItem *item = new QListWidgetItem();
            item->setSizeHint(QSize(30, 40));
            ui->listWidgetLeads->addItem(item);
            ui->listWidgetLeads->setItemWidget(item, lead);
            connect(lead, SIGNAL(emitToggleShowHide(uint, bool)), this, SLOT(toggleLead(uint, bool)));
            connect(lead, SIGNAL(emittoggleShowLeadArea(uint, bool)), this, SLOT(toggleLeadArea(uint, bool)));
        }


        info = QString("<b>Number of atoms:</b> ") + QString::number(stats.no_atoms);

        ui->tableWidgetDataStats->setRowCount(1);
        ui->tableWidgetDataStats->setColumnCount(2);
        ui->tableWidgetDataStats->setItem(0, 0, new QTableWidgetItem("No. atoms"));
        ui->tableWidgetDataStats->setItem(0, 1, new QTableWidgetItem(QString::number(stats.no_atoms)));

        QHeaderView *header = ui->tableWidgetDataStats->horizontalHeader();
#if QT_VERSION <= QT_VERSION_CHECK(4, 8, 1)
        header->setResizeMode(QHeaderView::Stretch);
#else
        header->setSectionResizeMode(QHeaderView::Stretch);
#endif

        info += QString("<br><b>Flags:</b> ");
        if (stats.flag_list.size() != glWidget->displayPerFlag.size()) {
            glWidget->displayPerFlag.clear();
            glWidget->flag2id.clear();
            ui->comboBoxFlags->clear();
            ui->comboBoxDataSetFlagFilter->clear();
            ui->comboBoxDataSetFlagFilter->addItem("All");
            for (int i = 0; i < stats.flag_list.size(); i++) {
                info += QString::number(stats.flag_list[i]) + ", ";
                glWidget->flag2id[stats.flag_list[i]] = i;
                DisplaySettings ds;
                ds.atom_quality = ui->spinBoxAtomQ->value();
                ds.atom_size = ui->doubleSpinBoxAtomSize->value();
                QPalette palette = ui->pushButtonAtomColor->palette();
                QColor color = palette.color(QPalette::Button);
                ds.color = color;

                glWidget->displayPerFlag.push_back(ds);
                ui->comboBoxFlags->addItem("Flag=" + QString::number(stats.flag_list[i]));
                ui->comboBoxDataSetFlagFilter->addItem(QString::number(stats.flag_list[i]));
            }
        } // display per flag

        info += QString("<br>");
        info += QString("<b>Max spin value:</b> ") + QString::number(stats.max_spin);
        info += QString("<br><b>Ave. dist:</b> ") + QString::number(stats.ave_dist);
        QString str;
        QDebug(&str) << QString("<br><b>Min. pos :</b> ") << stats.min_corner;
        QDebug(&str) << QString("<br><b>Max. pos :</b> ") << stats.max_corner;

        info += str;
        ui->textEditInfo->setText(info);

    }// end of if loading data

    // Data values
    ui->lineEditDatasetName->setText(xmlData.data.dataname);
    ui->comboBoxDatasetSpinFilter->clear();
    for (int i = 0; i < stats.max_spin; i++) {
        ui->comboBoxDatasetSpinFilter->addItem("Spin=" + QString::number(i + 1));
    }
    ui->listWidgetDatasetCols->clear();

    for (unsigned int i = 0; i < xmlData.data.max_values.size(); i++) {
        double min = xmlData.data.min_values[i];
        double max = xmlData.data.max_values[i];
        ui->listWidgetDatasetCols->addItem(
                "column#" + QString::number(i + 1) + " {" + QString::number(min) + "," + QString::number(max) + "}");
    }
    ui->listWidgetDatasetCols->setCurrentRow(0);
    bSkipSignals = false;

    glWidget->selectedAtomA = -1;
    glWidget->selectedAtomB = -1;
    glWidget->bSelectedAtomsConnected = false;
    updateWidgets();
}

void MainWindow::toggleLead(unsigned int id, bool toggle) {
    xmlData.p_leads[id].bShowLeadAtoms = toggle;
    xmlData.leads[id].bShowLeadAtoms = toggle;
    updateWidgets();
}

void MainWindow::toggleLeadArea(unsigned int id, bool toggle) {
    xmlData.p_leads[id].bShowArea = toggle;
    xmlData.leads[id].bShowArea = toggle;
    updateWidgets();
}

void MainWindow::receiveSpinBoxes(int) {
    updateWidgets();
}

void MainWindow::receiveDoubleSpinBoxes(double) {
    updateWidgets();
}

void MainWindow::receiveCheckBoxes(int toggle) {

    if (ui->radioButtonMainXY->isChecked()) glWidget->mainPlain = MAIN_PLAIN_XY;
    if (ui->radioButtonMainXZ->isChecked()) glWidget->mainPlain = MAIN_PLAIN_XZ;
    if (ui->radioButtonMainYZ->isChecked()) glWidget->mainPlain = MAIN_PLAIN_YZ;

    glWidget->bUseOrtho = ui->checkBoxOrthoProj->isChecked();
    glWidget->bHideConnections = ui->checkBoxCntsHide->isChecked();
    glWidget->bHideAtoms = ui->checkBoxHideAtoms->isChecked();

    glWidget->updateGL();
}

void MainWindow::updateWidgets() {
    if (bSkipSignals) return;
    glWidget->bCompileDisplayList = true;

    xmlData.precalculate_data();

    glWidget->displayConnections.atom_quality = ui->spinBoxConnectionQ->value();
    glWidget->displayConnections.atom_size = ui->doubleSpinBoxConnectionSize->value();
    glWidget->bUseLines = ui->checkBoxCntsUseLines->isChecked();
    QPalette palette = ui->pushButtonConnectionColour->palette();
    QColor color = palette.color(QPalette::Button);
    glWidget->displayConnections.color = color;

    if (!ui->checkBoxPerFlagSettings->isChecked()) {
        glWidget->displayAllSettings.atom_quality = ui->spinBoxAtomQ->value();
        glWidget->displayAllSettings.atom_size = ui->doubleSpinBoxAtomSize->value();
        QPalette palette = ui->pushButtonAtomColor->palette();
        QColor color = palette.color(QPalette::Button);
        glWidget->displayAllSettings.color = color;
    } else {
        int index = ui->comboBoxFlags->currentIndex();
        glWidget->displayPerFlag[index].atom_quality = ui->spinBoxAtomQ->value();
        glWidget->displayPerFlag[index].atom_size = ui->doubleSpinBoxAtomSize->value();
        glWidget->displayPerFlag[index].bHide = ui->pushButtonHideAtom->isChecked();
        QPalette palette = ui->pushButtonAtomColor->palette();
        QColor color = palette.color(QPalette::Button);
        glWidget->displayPerFlag[index].color = color;
    }

    // Data values
    qDebug() << ui->listWidgetDatasetCols->currentRow() << "::" << ui->comboBoxDatasetSpinFilter->currentIndex() ;
    glWidget->selectedDataColumn = ui->listWidgetDatasetCols->currentRow();
    glWidget->selectedDataSpin = ui->comboBoxDatasetSpinFilter->currentIndex();
    glWidget->selectDataFlag = ui->comboBoxDataSetFlagFilter->currentIndex();


    glWidget->displayAllSettings.min_cutoff = ui->horizontalSliderDataMinValue->value() / 300.0;
    glWidget->displayAllSettings.max_cutoff = ui->horizontalSliderDataMaxValue->value() / 300.0;
    glWidget->displayAllSettings.bShowDataValues = ui->checkBoxDatasetUse->isChecked();


    glWidget->updateGL();
}

void MainWindow::open() {
    QString fn = QFileDialog::getOpenFileName(this, tr("Open Bubel XML File"),
                                              lastDir, tr("xml (*.xml);;All Files (*)"));
    if (!fn.isEmpty()) {
        lastDir = fn;
        xmlData.read_data(fn);
        xmlData.precalculate_data();
        updateGUI();
    }
}

void MainWindow::close() {
    xmlData.clear_data();
    xmlData.loadingType = LOADING_STRUCTURE;
    xmlData.precalculate_data();
    updateGUI();
}

void MainWindow::reopen() {
    xmlData.read_data(lastDir);
    xmlData.precalculate_data();
    bReopening = true;
    updateGUI();
    bReopening = false;
}

void MainWindow::resetPosition() {
    ui->doubleSpinBoxRotX->setValue(0.0);
    ui->doubleSpinBoxRotY->setValue(0.0);
    glWidget->camera.reset();
    glWidget->xPosition = 0;
    glWidget->yPosition = 0;
    glWidget->repaint();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {

    if (event->mimeData()->hasFormat("text/plain"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
    QList <QUrl> urls = event->mimeData()->urls();
    if (urls.isEmpty())
        return;

    for (int i = 0; i < urls.size(); i++) {
        QString fileName = urls[i].toLocalFile();
        QFileInfo fileInfo(fileName);
        if (fileInfo.suffix() == "xml") {
            qDebug() << "Opening:" << fileName;
            lastDir = fileName;
            xmlData.read_data(fileName);
        }
    }
    xmlData.precalculate_data();
    updateGUI();

}


void MainWindow::chooseColor() {


    QPalette palette = ui->pushButtonAtomColor->palette();
    QColor color = palette.color(QPalette::Button);

    QColorDialog colorDialog;
    colorDialog.setCurrentColor(color);
    colorDialog.setParent(this);
    if (colorDialog.exec()) {
        color = colorDialog.selectedColor();
        palette.setColor(QPalette::Button, color);
        ui->pushButtonAtomColor->setAutoFillBackground(true);
        ui->pushButtonAtomColor->setPalette(palette);
        updateWidgets();
    }
}

void MainWindow::chooseConnectionColor() {
    QPalette palette = ui->pushButtonConnectionColour->palette();
    QColor color = palette.color(QPalette::Button);

    QColorDialog colorDialog;
    colorDialog.setCurrentColor(color);
    colorDialog.setParent(this);
    if (colorDialog.exec()) {
        color = colorDialog.selectedColor();
        palette.setColor(QPalette::Button, color);
        ui->pushButtonConnectionColour->setAutoFillBackground(true);
        ui->pushButtonConnectionColour->setPalette(palette);
        updateWidgets();
    }
}

void MainWindow::togglePerFlagDisplaySettings(bool toggle) {

    ui->comboBoxFlags->setEnabled(toggle);
    glWidget->bUseSettingsPerFlag = toggle;
    updateWidgets();

}

void MainWindow::updatePerFlagSettings(int row) {
    if (bSkipSignals) {
        return;
    }
    bSkipSignals = true;
    ui->doubleSpinBoxAtomSize->setValue(glWidget->displayPerFlag[row].atom_size);
    ui->spinBoxAtomQ->setValue(glWidget->displayPerFlag[row].atom_quality);
    ui->pushButtonHideAtom->setChecked(glWidget->displayPerFlag[row].bHide);

    QPalette palette;
    palette.setColor(QPalette::Button, glWidget->displayPerFlag[row].color);
    ui->pushButtonAtomColor->setPalette(palette);

    bSkipSignals = false;
    updateWidgets();
}


void MainWindow::selectedAtomsInfo(int atomA, int atomB) {

    if (atomA > -1) {
        ui->tableWidgetAtomInfo->setItem(0, 0, new QTableWidgetItem(QString::number(atomA + 1)));
        Atom &atom = xmlData.atoms[atomA];

        ui->tableWidgetAtomInfo->setItem(1, 0, new QTableWidgetItem(QString::number(atom.no_states)));
        ui->tableWidgetAtomInfo->setItem(2, 0, new QTableWidgetItem(QString::number(atom.no_bounds)));
        ui->tableWidgetAtomInfo->setItem(3, 0, new QTableWidgetItem(QString::number(atom.active)));
        ui->tableWidgetAtomInfo->setItem(4, 0, new QTableWidgetItem(QString::number(atom.flag)));
        ui->tableWidgetAtomInfo->setItem(5, 0, new QTableWidgetItem(QString::number(atom.flag0)));
        ui->tableWidgetAtomInfo->setItem(6, 0, new QTableWidgetItem(QString::number(atom.flag1)));
        QString pos = "(" + QString::number(atom.pos.x(), 'f', 2) + "," + QString::number(atom.pos.y(), 'f', 2) + "," +
                      QString::number(atom.pos.z(), 'f', 2) + ")";
        ui->tableWidgetAtomInfo->setItem(7, 0, new QTableWidgetItem(pos));
    } else {
        for (int i = 0; i < 9; i++)
            ui->tableWidgetAtomInfo->setItem(i, 0, new QTableWidgetItem(""));
    }

    if (atomB > -1) {
        Atom &atom = xmlData.atoms[atomB];

        ui->tableWidgetAtomInfo->setItem(0, 1, new QTableWidgetItem(QString::number(atomB + 1)));
        ui->tableWidgetAtomInfo->setItem(1, 1, new QTableWidgetItem(QString::number(atom.no_states)));
        ui->tableWidgetAtomInfo->setItem(2, 1, new QTableWidgetItem(QString::number(atom.no_bounds)));
        ui->tableWidgetAtomInfo->setItem(3, 1, new QTableWidgetItem(QString::number(atom.active)));
        ui->tableWidgetAtomInfo->setItem(4, 1, new QTableWidgetItem(QString::number(atom.flag)));
        ui->tableWidgetAtomInfo->setItem(5, 1, new QTableWidgetItem(QString::number(atom.flag0)));
        ui->tableWidgetAtomInfo->setItem(6, 1, new QTableWidgetItem(QString::number(atom.flag1)));
        QString pos = "(" + QString::number(atom.pos.x(), 'f', 2) + "," + QString::number(atom.pos.y(), 'f', 2) + "," +
                      QString::number(atom.pos.z(), 'f', 2) + ")";
        ui->tableWidgetAtomInfo->setItem(7, 1, new QTableWidgetItem(pos));
    } else {
        for (int i = 0; i < 9; i++)
            ui->tableWidgetAtomInfo->setItem(i, 1, new QTableWidgetItem(""));
    }

    ui->tableWidgetCouplings->clear();
    if (atomA > -1 && atomB > -1) {
        Atom &atA = xmlData.atoms[atomA];
        Atom &atB = xmlData.atoms[atomB];

        QVector3D r12 = atA.pos - atB.pos;
        ui->tableWidgetAtomInfo->setItem(8, 0, new QTableWidgetItem(QString::number(r12.length())));

        ui->tableWidgetCouplings->setRowCount(atA.no_states);
        ui->tableWidgetCouplings->setColumnCount(atB.no_states);
        vector<AtomConnection> atomAcnts;
        QFont fnt;
        fnt.setPointSize(8);
        fnt.setFamily("Arial");
        for (unsigned int i = 0; i < xmlData.connections.size(); i++) {
            if (xmlData.connections[i].atomA == atomA && xmlData.connections[i].atomB == atomB) {
                atomAcnts.push_back(xmlData.connections[i]);
                int s1 = xmlData.connections[i].spinA - 1;
                int s2 = xmlData.connections[i].spinB - 1;
                QString cpl = "(" + QString::number(xmlData.connections[i].realC) + "," +
                              QString::number(xmlData.connections[i].imagC) + ")";
                ui->tableWidgetCouplings->setItem(s1, s2, new QTableWidgetItem(cpl));
                ui->tableWidgetCouplings->item(s1, s2)->setFont(fnt);

            }
        } // end of for connections
    }// end of if coupling matrix

}
