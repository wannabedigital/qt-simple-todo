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
