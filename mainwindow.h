#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlTableModel>

#include <QDebug>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    QSqlDatabase db;  // 数据库对象
    QSqlQuery query;  // 数据库操作对象
    QSqlTableModel *model;  // 数据库显示对象

    // 记录上一个查询具体按了哪一个按钮
    // 0 none
    // 1 threedays
    // 2 lastweed
    // 3 all
    // 4 search
    int lastQueryStatus;

    // 三个查询按钮的槽函数
    void queryThreeDay();
    void queryLastWeek();
    void queryAll();

    // 按指定条件搜索的槽函数
    void search();

    // 查看详细信息的槽函数
    void tableviewact();
    void showdetail(QString);

    void deletedata();  // 删除数据的槽函数
    void insertdata();  // 新建数据的槽函数
    void savedata();  // 保存数据的槽函数
};

#endif // MAINWINDOW_H
