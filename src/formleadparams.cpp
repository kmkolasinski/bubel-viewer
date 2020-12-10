#include "formleadparams.h"
#include "ui_formleadparams.h"

FormLeadParams::FormLeadParams(unsigned int _id,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormLeadParams)
{
    ui->setupUi(this);
    id = _id;
    ui->label->setText(QString::number(id+1)+".");

    connect(ui->pushButtonShowHide,SIGNAL(toggled(bool)),this,SLOT(toggleShowHide(bool)));
    connect(ui->pushButtonShowArea,SIGNAL(toggled(bool)),this,SLOT(toggleShowLeadArea(bool)));
}

FormLeadParams::~FormLeadParams()
{
    delete ui;
}


void FormLeadParams::toggleShowHide(bool toggle){
        emit emitToggleShowHide(id,toggle);
}

void FormLeadParams::toggleShowLeadArea(bool toggle){
        emit emittoggleShowLeadArea(id,toggle);
}
