#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
QT_FORWARD_DECLARE_CLASS(QPushButton)
QT_FORWARD_DECLARE_CLASS(QLineEdit)
QT_FORWARD_DECLARE_CLASS(QGridLayout)
QT_FORWARD_DECLARE_CLASS(QComboBox)
QT_FORWARD_DECLARE_CLASS(QCheckBox)
class MainWindow : public QWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void slotPlayBtnClicked();

private:
    QPushButton *playBtn_;
    QLineEdit *urlEdit_;
    QGridLayout *gridLay_;
    QComboBox *decoderBox_;
    QCheckBox *dCheck_;
    int playIndex = 0;
};

#endif // MAINWINDOW_H
