#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSql>
#include <QInputDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_deleteButton_clicked();

    void on_editButton_clicked();

    void on_ratingButton_clicked();

    void on_addButton_clicked();

    void on_markDoneButton_clicked();

    int getSelectedId();

    void updateState(const QModelIndex &current);

    void on_showUndoneRadioButton_toggled(bool checked);

    void on_showDoneRadioButton_toggled(bool checked);

    void on_resetFilterButton_clicked();

    void on_minRatingSpinBox_valueChanged(int minValue);

    void on_maxRatingSpinBox_valueChanged(int maxValue);

    void applyFilter();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlQuery *query;
    QSqlTableModel *model;
    bool ok = false;
};
#endif // MAINWINDOW_H
