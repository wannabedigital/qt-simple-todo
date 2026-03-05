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
    model->setHeaderData(1, Qt::Horizontal, "Название задачи");
    model->setHeaderData(2, Qt::Horizontal, "Дата создания");
    model->setHeaderData(3, Qt::Horizontal, "Дата завершения");
    model->setHeaderData(4, Qt::Horizontal, "Выполнено");
    model->setHeaderData(5, Qt::Horizontal, "Рейтинг");

    ui->tableView->setModel(model);

    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    ui->tableView->hideColumn(0);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->markDoneButton->setEnabled(false);
    ui->editButton->setEnabled(false);
    ui->deleteButton->setEnabled(false);
    ui->ratingButton->setEnabled(false);

    connect(ui->tableView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &MainWindow::updateState);
    updateState(ui->tableView->currentIndex());
}

MainWindow::~MainWindow()
{
    delete ui;
    delete query;
    delete model;
    db.close();
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
    int id = getSelectedId();
    if (id == -1) return;

    query->prepare("DELETE FROM tasks WHERE id = :id");
    query->bindValue(":id", id);

    if (query->exec()) {
        model->select();

    }

    updateState(ui->tableView->currentIndex());
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
    ui->tableView->clearSelection();
    ui->tableView->setCurrentIndex(QModelIndex());
    updateState(ui->tableView->currentIndex());
}


void MainWindow::on_ratingButton_clicked()
{
    int id = getSelectedId();
    if (id == -1) return;

    int row = ui->tableView->selectionModel()->selectedRows()[0].row();

    int oldRating = model->data(model->index(row, 5)).toInt();
    int newRating = QInputDialog::getInt(this, "Рейтинг", "Рейтинг выполнения (0-100):",
                                         oldRating, 0, 100, 1, &ok);
    if (!ok) return;

    query->prepare("UPDATE tasks SET rating = :rating WHERE id = :id");
    query->bindValue(":id", id);
    query->bindValue(":rating", newRating);

    if (query->exec()) {
        model->select();
    }

    ok = false;
    ui->tableView->clearSelection();
    ui->tableView->setCurrentIndex(QModelIndex());
    updateState(ui->tableView->currentIndex());
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

    updateState(ui->tableView->currentIndex());
    ok = false;
}


void MainWindow::on_markDoneButton_clicked()
{
    int id = getSelectedId();
    if (id == -1) return;

    int row = ui->tableView->selectionModel()->selectedRows()[0].row();
    bool taskStatus = model->data(model->index(row, 4)).toBool();

    if (!taskStatus) {
        query->prepare("UPDATE tasks SET done = 1, time_done = :time WHERE id = :id");
        query->bindValue(":time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"));
        query->bindValue(":id", id);
    } else {
        query->prepare("UPDATE tasks SET done = 0, time_done = :time WHERE id = :id");
        query->bindValue(":time", "");
        query->bindValue(":id", id);
    }


    if (query->exec()) {
        model->select();

    }

    updateState(ui->tableView->currentIndex());
}


void MainWindow::updateState(const QModelIndex &current)
{
    if (!current.isValid()) {
        ui->markDoneButton->setEnabled(false);
        ui->markDoneButton->setText("Выполнить");
        ui->editButton->setEnabled(false);
        ui->deleteButton->setEnabled(false);
        ui->ratingButton->setEnabled(false);
        ui->ratingButton->setText("Оценить");
        return;
    }

    ui->markDoneButton->setEnabled(true);
    ui->editButton->setEnabled(true);
    ui->deleteButton->setEnabled(true);
    ui->ratingButton->setEnabled(true);

    int row = current.row();
    bool taskStatus = model->data(model->index(row, 4)).toBool();
    int taskRating = model->data(model->index(row, 5)).toInt();

    if (!taskStatus) {
        ui->markDoneButton->setText("Выполнить");
    } else {
        ui->markDoneButton->setText("Отменить выполнение");
    }

    if (taskRating == 0) {
        ui->ratingButton->setText("Оценить");
    } else {
        ui->ratingButton->setText("Изменить оценку");
    }

    return;
}

void MainWindow::on_showUndoneRadioButton_toggled(bool checked)
{
    if (!checked) {
        return;
    }
    model->setFilter("done=0");
    model->select();
}


void MainWindow::on_showDoneRadioButton_toggled(bool checked)
{
    if (!checked) {
        return;
    }
    model->setFilter("done=1");
    model->select();
}


void MainWindow::on_resetFilterButton_clicked()
{
    ui->showUndoneRadioButton->setAutoExclusive(false);
    ui->showDoneRadioButton->setAutoExclusive(false);
    ui->showUndoneRadioButton->setChecked(false);
    ui->showDoneRadioButton->setChecked(false);
    ui->showUndoneRadioButton->setAutoExclusive(true);
    ui->showDoneRadioButton->setAutoExclusive(true);
    model->setFilter("");
    model->select();
}

