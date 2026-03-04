#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("todo_db");

    if (!db.open()) return;

    query = new QSqlQuery(db);

    if (!db.tables().contains("tasks")) {
        query -> clear();
        bool checkQ = query->exec(R"(
            CREATE TABLE tasks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT NOT NULL,
                time_added TEXT,
                time_done TEXT,
                done INTEGER DEFAULT 0,
                rating INTEGER DEFAULT 0 CHECK(rating >= 0 AND rating <= 100)
            )
        )");

        if (!checkQ) return;
    }

    model = new QSqlTableModel(this, db);
    model->setTable("tasks");
    model->select();
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    ui->tableView->setModel(model);
    ui->tableView->hideColumn(0);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);


}

MainWindow::~MainWindow()
{
    delete ui;
    delete query;
    delete model;
}


int MainWindow::getSelectedId()
{
    QModelIndexList selected = ui->tableView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        return -1;
    }

    int row = selected[0].row();
    QModelIndex idIndex = model->index(row, 0);
    return model->data(idIndex).toInt();
}



void MainWindow::on_deleteButton_clicked()
{

}


void MainWindow::on_editButton_clicked()
{
    int id = getSelectedId();
    if (id == -1) return;

    int row = ui->tableView->selectionModel()->selectedRows()[0].row();
    QString oldTitle = model->data(model->index(row, 1)).toString();

    QString newTitle = QInputDialog::getText(this, "Редактирование", "Измените название задачи:",
                                             QLineEdit::Normal, oldTitle, &ok).trimmed();

    if (!ok || newTitle.isEmpty()) {
        return;
    }

    query->prepare("UPDATE tasks SET title = :title WHERE id = :id");
    query->bindValue(":id", id);
    query->bindValue(":title", newTitle.trimmed());


    if (query->exec()) {
        model->select();
    }

    ok = false;
}


void MainWindow::on_addButton_clicked()
{
    QString title = QInputDialog::getText(this, "Новая задача", "Введите название задачи:",
                                          QLineEdit::Normal, "", &ok).trimmed();

    if (!ok || title.isEmpty()) {
        return;
    }

    query->prepare("INSERT INTO tasks (title, time_added, done, rating) VALUES (:title, :time, 0, 0)");
    query->bindValue(":title", title);
    query->bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));

    if (query->exec()) {
        model->select();
    }

    ok = false;
}


void MainWindow::on_markDoneButton_clicked()
{
    int id = getSelectedId();
    if (id == -1) return;

    query->prepare("UPDATE tasks SET done = 1, time_done = :time, WHERE id = :id");
    query->bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
    query->bindValue(":id", id);

    if (query->exec()) {
        model->select();
    }
}

