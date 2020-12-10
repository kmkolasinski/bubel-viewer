#ifndef FORMLEADPARAMS_H
#define FORMLEADPARAMS_H

#include <QWidget>

namespace Ui {
class FormLeadParams;
}

class FormLeadParams : public QWidget
{
    Q_OBJECT
    
public:
    explicit FormLeadParams(unsigned int _id, QWidget *parent = 0);
    ~FormLeadParams();

public slots:
    void toggleShowHide(bool toggle);
    void toggleShowLeadArea(bool toggle);
signals:
    void emitToggleShowHide    (unsigned int id,bool toggle);
    void emittoggleShowLeadArea(unsigned int id,bool toggle);

private:
    unsigned int id;
    Ui::FormLeadParams *ui;
};

#endif // FORMLEADPARAMS_H
