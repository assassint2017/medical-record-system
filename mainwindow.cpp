#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow), db(QSqlDatabase::contains("qt_sql_default_connection")?QSqlDatabase::database("qt_sql_default_connection"):QSqlDatabase::addDatabase("QSQLITE")), query(db)
{
    ui->setupUi(this);

    QObject::connect(ui->queryThereDays, &QPushButton::clicked, this, &MainWindow::queryThreeDay);
    QObject::connect(ui->queryLastWeek, &QPushButton::clicked, this, &MainWindow::queryLastWeek);
    QObject::connect(ui->queryAll, &QPushButton::clicked, this, &MainWindow::queryAll);

    QObject::connect(ui->querySearch, &QPushButton::clicked, this, &MainWindow::search);

    QObject::connect(ui->tableView, &QTableView::clicked, this, &MainWindow::tableviewact);
    QObject::connect(ui->tableView, &QTableView::entered, this, &MainWindow::tableviewact);

    QObject::connect(ui->deleteButton, &QPushButton::clicked, this, &MainWindow::deletedata);
    QObject::connect(ui->insertButon, &QPushButton::clicked, this, &MainWindow::insertdata);
    QObject::connect(ui->saveButton, &QPushButton::clicked, this, &MainWindow::savedata);

    model = new QSqlTableModel;
    ui->tableView->setModel(model);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 设为不可编辑
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);  //设置选中模式为选中行
    ui->tableView->setSelectionMode( QAbstractItemView::SingleSelection);  //设置选中单个

    db.setDatabaseName("./patientInfo.db");
    if (db.open())
        qDebug() << "successfully connected to the database!";
    else
        qDebug() << "fial to connected to the database!";


    // 设置搜索条件
    ui->queryComboBox->addItem("病例编号");
    ui->queryComboBox->addItem("患者性别");

    lastQueryStatus = 0;
}

MainWindow::~MainWindow()
{
    delete ui;

    // 退出时关闭数据库
    db.close();
}

void MainWindow::queryThreeDay()
{
    model->setTable("基本病例");
    model->setFilter("julianday('now') - julianday(收录日期) <= 3");
    model->select();

    lastQueryStatus = 1;
}

void MainWindow::queryLastWeek()
{
    model->setTable("基本病例");
    model->setFilter("julianday('now') - julianday(收录日期) <= 7");
    model->select();

    lastQueryStatus = 2;
}

void MainWindow::queryAll()
{
    model->setTable("基本病例");
    model->select();

    lastQueryStatus = 3;
}

void MainWindow::search()
{
    // 按照病例编号进行查询
    if (ui->queryComboBox->currentIndex() == 0)
    {
        model->setTable("基本病例");
        model->setFilter("病例编号 = " + ui->queryLineEdit->text());
        model->select();
    }

    // 按照患者性别进行查询
    else
    {
        model->setTable("基本病例");
        model->setFilter("患者性别 = '" + ui->queryLineEdit->text() + "'");
        model->select();
    }

    lastQueryStatus = 4;
}

void MainWindow::tableviewact()
{
    int row = ui->tableView->currentIndex().row();
    QAbstractItemModel *model = ui->tableView->model();

    QModelIndex index = model->index(row, 0);
    showdetail(model->data(index).toString());
}

void MainWindow::showdetail(QString id)
{
    // 当处于浏览状态的时候，不允许编辑主键，但是允许编辑其他数据
    ui->patientId->setReadOnly(true);

    query.exec("select * from 基本病例 where 病例编号 = " + id);
    while (query.next())
    {
        ui->patientId->setText(query.value(0).toString());
        ui->patientGender->setText(query.value(1).toString());
        ui->patientPlan->setText(query.value(2).toString());
        ui->patientdate->setText(query.value(3).toString());
        ui->patientName->setText(query.value(4).toString());
        ui->patientAge->setText(query.value(5).toString());
        ui->patientattendingPhysician->setText(query.value(6).toString());
        ui->patientphoneNumber->setText(query.value(7).toString());
    }
    query.exec("select * from 规划信息 where 病例编号 = " + id);
    while (query.next())
    {
        ui->numtumor->setText(query.value(1).toString());
        ui->numneedle->setText(query.value(2).toString());
        ui->tumorSize->setText(query.value(3).toString());
        ui->temperature->setText(query.value(4).toString());
    }
    query.exec("select * from 术后评估 where 病例编号 = " + id);
    while (query.next())
    {
        ui->preoperativeLiverSize->setText(query.value(1).toString());
        ui->postoperativeLiverSize->setText(query.value(2).toString());
        ui->ablationSize->setText(query.value(3).toString());
        ui->ablationRate->setText(query.value(4).toString());
    }
}

void MainWindow::deletedata()
{
    int row = ui->tableView->currentIndex().row();
    QAbstractItemModel *model = ui->tableView->model();

    QModelIndex index = model->index(row, 0);

    db.transaction();
    query.prepare("delete from 基本病例 where 病例编号 = " + model->data(index).toString());
    query.prepare("delete from 规划信息 where 病例编号 = " + model->data(index).toString());
    query.prepare("delete from 术后评估 where 病例编号 = " + model->data(index).toString());
    bool status = query.execBatch();
    db.commit();

    if (status)
        QMessageBox::about(this, "提示", "删除数据成功!");
    else
        QMessageBox::about(this, "提示", "删除数据失败!");

    model->removeRow(row);
}

void MainWindow::insertdata()
{
    // 主键设置可以编辑
    ui->patientId->setReadOnly(false);

    // 清空之前的信息
    ui->patientId->clear();
    ui->patientGender->clear();
    ui->patientPlan->clear();
    ui->patientdate->clear();
    ui->patientName->clear();
    ui->patientAge->clear();
    ui->patientattendingPhysician->clear();
    ui->patientphoneNumber->clear();

    ui->numtumor->clear();
    ui->numneedle->clear();
    ui->tumorSize->clear();
    ui->temperature->clear();

    ui->preoperativeLiverSize->clear();
    ui->postoperativeLiverSize->clear();
    ui->ablationSize->clear();
    ui->ablationRate->clear();
}

void MainWindow::savedata()
{
    QString temp1 = ui->patientId->text();
    QString temp2 = ui->patientGender->text();
    QString temp3 = ui->patientPlan->text();
    QString temp4 = ui->patientdate->text();
    QString temp5 = ui->patientName->text();
    QString temp6 = ui->patientAge->text();
    QString temp7 = ui->patientattendingPhysician->text();
    QString temp8 = ui->patientphoneNumber->text();
    QString value1 = QString("(%1, '%2', '%3', '%4', '%5', %6, '%7', '%8')").arg(temp1).arg(temp2)
                                                                                .arg(temp3).arg(temp4)
                                                                                .arg(temp5).arg(temp6)
                                                                                .arg(temp7).arg(temp8);

    QString temp9 = ui->numtumor->text();
    QString temp10 = ui->numneedle->text();
    QString temp11 = ui->tumorSize->text();
    QString temp12 = ui->temperature->text();
    QString value2 = QString("(%1, %2, %3, %4, %5)").arg(temp1).arg(temp9).arg(temp10)
                                                    .arg(temp11).arg(temp12);

    QString temp13 = ui->preoperativeLiverSize->text();
    QString temp14 = ui->postoperativeLiverSize->text();
    QString temp15 = ui->ablationSize->text();
    QString temp16 = ui->ablationRate->text();
    QString value3 = QString("(%1, %2, %3, %4, %5)").arg(temp1).arg(temp13)
                                                    .arg(temp14).arg(temp15).arg(temp16);

    db.transaction();
    query.prepare("replace into 基本病例 values " + value1);
    query.prepare("replace into 规划信息 values " + value2);
    query.prepare("replace into 术后评估 values " + value3);
    bool status = query.execBatch();
    db.commit();

    // 打印状态信息
    if (!ui->patientId->isReadOnly())
    {
        if (status)
            QMessageBox::about(this, "提示", "新建数据成功!");
        else
            QMessageBox::about(this, "提示", "新建数据失败!");
    }
    else
    {
        if (status)
            QMessageBox::about(this, "提示", "更新数据成功!");
        else
            QMessageBox::about(this, "提示", "更新数据失败!");
    }

    // 更新简略信息
    if (lastQueryStatus == 0)
        return;
    else if (lastQueryStatus == 1)
        queryThreeDay();
    else if (lastQueryStatus == 2)
        queryLastWeek();
    else if (lastQueryStatus == 3)
        queryAll();
    else
        search();
}
