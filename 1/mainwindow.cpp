#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "libpq-fe.h"
#include <QtWidgets>
#include <Windows.h>
#include <string>
#include <QMessageBox>
#include <vector>
#include <unordered_map>

using namespace std;

PGconn* conn = PQconnectdb("host=localhost port=5432 dbname=telephone user=postgres password=1");
bool check=false;
bool main_check=false;

QString temp_tab;
QString temp_id;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    SetConsoleCP (CP_UTF8);
    SetConsoleOutputCP (CP_UTF8);

    table=ui->tab;


    PGresult* res = PQexec(conn, "select TABLE_NAME FROM telephone.INFORMATION_SCHEMA.TABLES WHERE TABLE_TYPE = 'BASE TABLE' AND TABLE_schema='public'");

    int rows = PQntuples(res);
    int cols = PQnfields(res);

    ui->setupUi(this);

    for(int i=0;i<rows;i++){
        ui->comboBox->addItem(PQgetvalue(res, i, 0));
    }

    res = PQexec(conn, "select f_value FROM fam");

    for(int i=0;i<rows;i++){
        ui->comboBox_3->addItem(PQgetvalue(res, i, 0));
    }

    res = PQexec(conn, "select n_value FROM nam");

    for(int i=0;i<rows;i++){
        ui->comboBox_4->addItem(PQgetvalue(res, i, 0));
    }

    res = PQexec(conn, "select otc_value FROM otc");

    for(int i=0;i<rows;i++){
        ui->comboBox_5->addItem(PQgetvalue(res, i, 0));
    }

    PQclear(res);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_pushButton_clicked()
{
    ui->comboBox_2->clear();
    check=false;
    main_check=false;
    ui->tab->clear();
    auto b=ui->comboBox->currentText();
    temp_tab=b;
    QString a = "select * from "+b;
    //ui->label->setText(a);

    QString header_req_qs="select column_name from information_schema.columns where table_name='"+b+"'";
    //const char* header_req=header_req_qs.toStdString().c_str();
    PGresult* last = PQexec(conn, header_req_qs.toStdString().c_str());
    int rows = PQntuples(last);
    int coll = PQnfields(last);

    ui->tab->setColumnCount(rows);

    temp_id=PQgetvalue(last,0,0);

    for(int i=0;i<rows;i++){
        ui->tab->setHorizontalHeaderItem(i,new QTableWidgetItem(PQgetvalue(last,i,0)));
    }
    PQclear(last);

    header_req_qs=a;
    last = PQexec(conn, header_req_qs.toStdString().c_str());
    rows = PQntuples(last);
    coll = PQnfields(last);

    ui->tab->setRowCount(rows);



    for(int i=0;i<rows;i++){
        int temp=i+1;
        std::string t=std::to_string(temp);
        const char* temp_char=t.c_str();
        auto tempQT=new QTableWidgetItem(temp_char);

        ui->tab->setVerticalHeaderItem(i,tempQT);
    }

    for(int i=0;i<rows;i++){
        for(int j=0;j<coll;j++){
            auto temp=new QTableWidgetItem(PQgetvalue(last,i,j));
            ui->tab->setItem(i,j,temp);
        }
    }

    for(int i=0;i<rows;i++){
        ui->comboBox_2->addItem(PQgetvalue(last,i,0));
    }

    QMessageBox msg;
    msg.setText(header_req_qs);
    msg.exec();

    check=true;
}



void MainWindow::on_tab_cellChanged(int row, int column)
{
    if(check){
        auto Cell = ui->tab->item(row,column);
        auto Cell_str=Cell->text();
        QString request="update "+temp_tab+ " set "+ui->tab->takeHorizontalHeaderItem(column)->text()+"='"+Cell_str+"' where "+temp_id+"="+ui->tab->item(row,0)->text();
        PGresult* last = PQexec(conn, request.toStdString().c_str());
        QMessageBox msg;
        msg.setText(request);
        msg.exec();
    }
    if(main_check){
        auto Cell = ui->tab->item(row,column);
        auto Cell_str=Cell->text();
        auto id=ui->tab->item(row,0);
        auto id_str=id->text();
        std::unordered_map<int,QString> map;
        map[1]="fam";
        map[2]="nam";
        map[3]="otc";
        map[4]="telef";
        map[5]="street";
        map[6]="bldn";
        map[7]="apar";
        std::unordered_map<int,QString> stolb_name; //название столбца в main
        stolb_name[1]="fam";
        stolb_name[2]="name_";
        stolb_name[3]="otc";
        stolb_name[4]="telef";
        stolb_name[5]="street";
        stolb_name[6]="bldn";
        stolb_name[7]="apar";
        std::unordered_map<int,QString> stolb;// названия столбцов в других таблицах
        stolb[1]="f_";
        stolb[2]="n_";
        stolb[3]="otc_";
        stolb[5]="str_";
        auto str_tab=map[column];
        QString request="select "+stolb_name[column]+" from main where u_id="+id_str;
        PGresult* last = PQexec(conn, request.toStdString().c_str());
        auto not_main_id=PQgetvalue(last,0,0);
        if((column==4)||(column==6)||(column==7)){
            QString request="update main set "+str_tab+" = '"+Cell_str+"' where u_id = "+id_str;
            PGresult* last = PQexec(conn, request.toStdString().c_str());
            QMessageBox msg;
            msg.setText(request);
            msg.exec();
        }
        else{
            QString request="update "+str_tab+" set "+stolb[column]+"value = '"+Cell_str+"' where "+stolb[column]+"id = "+not_main_id;
            PGresult* last = PQexec(conn, request.toStdString().c_str());
            QMessageBox msg;
            msg.setText(request);
            msg.exec();
        }

        //1,2,3,5 switch
    }
}


void MainWindow::on_pushButton_3_clicked()
{
    auto b=ui->comboBox_2->currentText();
    QString request="delete from "+temp_tab+" where " + ui->tab->takeHorizontalHeaderItem(0)->text()+"="+b;
    PGresult* last = PQexec(conn, request.toStdString().c_str());

    MainWindow::on_pushButton_clicked();
    QMessageBox msg;
    msg.setText(request);
    msg.exec();
}


void MainWindow::on_pushButton_2_clicked()
{
    check=false;
    int rows=ui->tab->rowCount();
    rows+=1;
    ui->tab->setRowCount(rows);

    int temp=rows;
    std::string t=std::to_string(temp);
    const char* temp_char=t.c_str();
    auto tempQT=new QTableWidgetItem(temp_char);

    ui->tab->setVerticalHeaderItem(rows,tempQT);
}


void MainWindow::on_pushButton_4_clicked()
{
    int cols=ui->tab->columnCount();
    int rows=ui->tab->rowCount();
    rows-=1;

    QString req="(default,";

    for(int i=1;i<cols;i++){
        auto temp=ui->tab->item(rows,i);
        auto temp_str=temp->text();
        req+="'"+temp_str+"'";
        if((cols-1)!=i){
            req+=",";
        }
    }
    req+=")";

    QString request="insert into "+temp_tab+" values "+req;
    PGresult* last = PQexec(conn, request.toStdString().c_str());

    QMessageBox msg;
    msg.setText(request);
    msg.exec();

    MainWindow::on_pushButton_clicked();

    check = true;
}


void MainWindow::on_pushButton_5_clicked()
{
    main_check=false;
    ui->comboBox_2->clear();
    check=false;
    ui->tab->clear();
    QString b="main";
    temp_tab=b;
    QString a = "select * from "+b;
    //ui->label->setText(a);

    QString header_req_qs="select column_name from information_schema.columns where table_name='"+b+"'";
    //const char* header_req=header_req_qs.toStdString().c_str();
    PGresult* last = PQexec(conn, header_req_qs.toStdString().c_str());
    int rows = PQntuples(last);
    int coll = PQnfields(last);

    ui->tab->setColumnCount(rows);

    temp_id=PQgetvalue(last,0,0);

    /*for(int i=0;i<rows;i++){
        ui->tab->setHorizontalHeaderItem(i,new QTableWidgetItem(PQgetvalue(last,i,0)));
    }*/

    ui->tab->setHorizontalHeaderItem(0,new QTableWidgetItem("id"));
    ui->tab->setHorizontalHeaderItem(1,new QTableWidgetItem("Фамилия"));
    ui->tab->setHorizontalHeaderItem(2,new QTableWidgetItem("Имя"));
    ui->tab->setHorizontalHeaderItem(3,new QTableWidgetItem("Отчество"));
    ui->tab->setHorizontalHeaderItem(4,new QTableWidgetItem("Телефон"));
    ui->tab->setHorizontalHeaderItem(5,new QTableWidgetItem("Улица"));
    ui->tab->setHorizontalHeaderItem(6,new QTableWidgetItem("Дом"));
    ui->tab->setHorizontalHeaderItem(7,new QTableWidgetItem("Квартира"));

    PQclear(last);

    bool check_and=false;

    header_req_qs="select u_id,f_value,n_value,otc_value,telef,str_value,bldn,apar from main join fam on main.fam=fam.f_id join nam on main.name_=nam.n_id join otc on main.otc=otc.otc_id join street on main.street=street.str_id";
    auto fam=ui->comboBox_3->currentText();
    auto nam=ui->comboBox_4->currentText();
    auto otc=ui->comboBox_5->currentText();

    if(fam!="" || nam!="" || otc!=""){
        header_req_qs +=" where ";
    }
    if(fam!=""){
        header_req_qs +="f_value = '"+fam+"'" ;
        check_and=true;
    }
    if(nam!=""){
        if(check_and){
            header_req_qs +=" and " ;
        }
        header_req_qs +="n_value = '"+nam+"'" ;
    }
    if(otc!=""){
        if(check_and){
            header_req_qs +=" and " ;
        }
        header_req_qs +="otc_value = '"+otc+"'" ;
    }


    last = PQexec(conn, header_req_qs.toStdString().c_str());
    rows = PQntuples(last);
    coll = PQnfields(last);

    ui->tab->setRowCount(rows);

    for(int i=0;i<rows;i++){
        int temp=i+1;
        std::string t=std::to_string(temp);
        const char* temp_char=t.c_str();
        auto tempQT=new QTableWidgetItem(temp_char);

        ui->tab->setVerticalHeaderItem(i,tempQT);
    }

    for(int i=0;i<rows;i++){
        for(int j=0;j<coll;j++){
            auto temp=new QTableWidgetItem(PQgetvalue(last,i,j));
            ui->tab->setItem(i,j,temp);
        }
    }

    for(int i=0;i<rows;i++){
        ui->comboBox_2->addItem(PQgetvalue(last,i,0));
    }


    QMessageBox msg;
    msg.setText(header_req_qs);
    msg.exec();

    check=false;
    main_check=true;
}

