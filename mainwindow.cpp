#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qcustomplot.h"
// #include <iostream>
// #include <numeric>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer{new QTimer(this)}
    , UAR{}
    , krok{0}
    , krok_czas{0.0}
    , interwal_wykres_sec{0.0}
    , graph_x{}
    , uar_wy_y{}
    , uchyb_y{}
    , pid_y{}
    , p_y{}
    , i_y{}
    , d_y{}
{
    ui->setupUi(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(advance()));

    on_btnReset_clicked();

    initChartConfig(ui->graphUAR, "f", 1, "Czas, s", "Sygnał zadany i sterowany");
    initChartConfig(ui->graphUchyb, "f", 1, "Czas, s", "Uchyb");
    initChartConfig(ui->graphPidSum, "f", 1, "Czas, s", "Sygnał regulatora PID");
    initChartConfig(ui->graphPID, "f", 1, "Czas, s", "Składowe P, I, D");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::advance() {

    if (ui->groupBoxSkok->isChecked()) {
        UAR.liczSygnalSkok();

    } else if (ui->groupBoxKwad->isChecked()) {
        UAR.liczSygnalKwad();

    } else if (ui->groupBoxSin->isChecked()) {
        UAR.liczSygnalSin();
    }

    double wy;

    if (ui->radioStalaOut->isChecked())
        wy = UAR.symulujKrok_IConstOut();
    else
        wy = UAR.symulujKrok_IConstIn();

    // std::cerr << wy << '\n';

    //przypisanie wartości kroku
    graph_x.push_back(krok_czas);
    uar_wy_y.push_back(wy);
    uar_we_y.push_back(UAR.getSygn());
    uchyb_y.push_back(UAR.getUchyb());
    pid_y.push_back(UAR.getPID_output());
    p_y.push_back(UAR.getPID_P());
    i_y.push_back(UAR.getPID_I());
    d_y.push_back(UAR.getPID_D());

    // qDebug() << "START\n" << graph_x.size();
    // qDebug() << uar_wy_y.size();
    // qDebug() << uar_we_y.size();
    // qDebug() << uchyb_y.size();
    // qDebug() << pid_y.size();
    // qDebug() << p_y.size();
    // qDebug() << i_y.size();
    // qDebug() << d_y.size() << "\nEND\n";

    if (graph_x.size() > 100000) {
        graph_x.pop_front();
        uar_wy_y.pop_front();
        uar_we_y.pop_front();
        uchyb_y.pop_front();
        pid_y.pop_front();
        p_y.pop_front();
        i_y.pop_front();
        d_y.pop_front();
    }

    krok++;
    krok_czas += interwal_wykres_sec;

    //graf WE / WY
    ui->graphUAR->graph(0)->setData(graph_x, uar_we_y);
    ui->graphUAR->graph(1)->setData(graph_x, uar_wy_y);
    // ui->graphUAR->graph(0)->addData(krok_czas, UAR.getSygn());
    // ui->graphUAR->graph(1)->addData(krok_czas, wy);

    //graf uchyb
    ui->graphUchyb->graph(0)->setData(graph_x, uchyb_y);
    // ui->graphUchyb->graph(0)->addData(krok_czas, UAR.getUchyb());

    //graf pid (sumarycznie)
    ui->graphPidSum->graph(0)->setData(graph_x, pid_y);
    // ui->graphPidSum->graph(0)->addData(krok_czas, UAR.getPID_output());

    //graf pid (składowe)
    ui->graphPID->graph(0)->setData(graph_x, p_y);
    ui->graphPID->graph(1)->setData(graph_x, i_y);
    ui->graphPID->graph(2)->setData(graph_x, d_y);
    // ui->graphPID->graph(0)->addData(krok_czas, UAR.getPID_P());
    // ui->graphPID->graph(1)->addData(krok_czas, UAR.getPID_I());
    // ui->graphPID->graph(2)->addData(krok_czas, UAR.getPID_D());

    //przesunięcie OX
    // if (krok_czas > ui->spinBoxWidokKrokow->value() * interwal_wykres_sec){
    //     //ui->graphUAR->xAxis->moveRange(interwal_wykres_sec);
    //     ui->graphUchyb->xAxis->moveRange(interwal_wykres_sec);
    //     ui->graphPidSum->xAxis->moveRange(interwal_wykres_sec);
    //     ui->graphPID->xAxis->moveRange(interwal_wykres_sec);
    // }

    double a = ui->spinBoxWidokKrokow->value();
    if(krok > a){
        ui->graphUAR->xAxis->setRange(graph_x[krok - a], graph_x.last());
        ui->graphUchyb->xAxis->setRange(graph_x[krok - a], graph_x.last());
        ui->graphPidSum->xAxis->setRange(graph_x[krok - a], graph_x.last());
        ui->graphPID->xAxis->setRange(graph_x[krok - a], graph_x.last());
    }
    else{
        ui->graphUAR->xAxis->setRange(0, interwal_wykres_sec * (a - krok) + krok_czas);
        ui->graphUchyb->xAxis->setRange(0, interwal_wykres_sec * (a - krok) + krok_czas);
        ui->graphPidSum->xAxis->setRange(0, interwal_wykres_sec * (a - krok) + krok_czas);
        ui->graphPID->xAxis->setRange(0, interwal_wykres_sec * (a - krok) + krok_czas);
    }


    //wstępne skalowanie
    ui->graphUAR->graph(0)->rescaleValueAxis(false, true);
    ui->graphUchyb->graph(0)->rescaleValueAxis(false, true);
    ui->graphPidSum->graph(0)->rescaleValueAxis(false, true);
    ui->graphPID->graph(0)->rescaleValueAxis(false, true);

    //graf we / wy - skalowanie OY
    double min = findMinRange(uar_we_y);
    double max = findMaxRange(uar_we_y);

    if(min > findMinRange(uar_wy_y))
        min = findMinRange(uar_wy_y);
    if(max < findMaxRange(uar_wy_y))
        max = findMaxRange(uar_wy_y);

    double margin = (max - min) * 0.05;
    ui->graphUAR->yAxis->setRange(min - margin, max + margin);
    ui->graphUAR->replot();

    //graf uchyb - skalowanie OY
    min = findMinRange(uchyb_y);
    max = findMaxRange(uchyb_y);

    margin = (max - min) * 0.05;
    ui->graphUchyb->yAxis->setRange(min - margin, max + margin);
    ui->graphUchyb->replot();

    //graf pid (sumaryczny) - skalowanie OY
    min = findMinRange(pid_y);
    max = findMaxRange(pid_y);

    margin = (max - min) * 0.05;
    ui->graphPidSum->yAxis->setRange(min - margin, max + margin);
    ui->graphPidSum->replot();

    //graf pid (składowe) - skalowanie OY
    min = findMinRange(p_y);
    max = findMaxRange(p_y);

    if(min > findMinRange(i_y))
        min = findMinRange(i_y);
    if(max < findMaxRange(i_y))
        max = findMaxRange(i_y);

    if(min > findMinRange(d_y))
        min = findMinRange(d_y);
    if(max < findMaxRange(d_y))
        max = findMaxRange(d_y);

    margin = (max - min) * 0.05;
    ui->graphPID->yAxis->setRange(min - margin, max + margin);

    ui->graphUAR->replot();
    ui->graphUchyb->replot();
    ui->graphPidSum->replot();
    ui->graphPID->replot();
}

void MainWindow::on_btnStart_clicked()
{
    ui->btnStop->setEnabled(true);
    ui->btnStart->setEnabled(false);
    ui->btnReset->setEnabled(false);
    ui->labelStatus->setText("Włączona");

    this->passToSetters();

    timer->start();
}

void MainWindow::on_groupBoxKwad_toggled(bool arg1)
{
    if(arg1) {
        ui->groupBoxSin->setChecked(false);
        ui->groupBoxSkok->setChecked(false);
        ui->graphUAR->graph(0)->setLineStyle(QCPGraph::lsStepRight);     // linia równoległa do OY przy skoku
        on_btnZapisz_clicked();
    }
}

void MainWindow::on_groupBoxSin_toggled(bool arg1)
{
    if(arg1) {
        ui->groupBoxKwad->setChecked(false);
        ui->groupBoxSkok->setChecked(false);
        ui->graphUAR->graph(0)->setLineStyle(QCPGraph::lsLine);     // linia bezpośrednio pomiędzzy punktami
        on_btnZapisz_clicked();
    }
}

void MainWindow::on_groupBoxSkok_toggled(bool arg1)
{
    if(arg1) {
        ui->groupBoxKwad->setChecked(false);
        ui->groupBoxSin->setChecked(false);
        ui->graphUAR->graph(0)->setLineStyle(QCPGraph::lsStepRight);     // linia równoległa do OY przy skoku
        on_btnZapisz_clicked();
    }
}

void MainWindow::on_btnReset_clicked()
{   
    UAR.setPID_k(0.0);
    UAR.setPID_tI(0.0);
    UAR.setPID_tD(0.0);
    UAR.resetPID_I();
    UAR.resetPID_D();

    UAR.clearARX_a();
    UAR.clearARX_b();
    UAR.addARX_a(-0.4);
    UAR.addARX_b(0.6);
    UAR.setARX_k(1);
    UAR.setARX_z(false);
    UAR.setARX_z_val(0.0);
    UAR.clearARXBuffers();

    UAR.resetInternalKrok();
    UAR.resetUchyb();
    krok = 0;
    krok_czas = 0.0;

    ui->labelStatus->setText("Wyłączona");

    ui->groupBoxSkok->setChecked(true);
    ui->groupBoxSin->setChecked(false);
    ui->groupBoxKwad->setChecked(false);
    ui->spinBoxSkokAkt->setValue(1);
    ui->doubleSpinBoxSkokAmp->setValue(1.0);
    ui->doubleSpinBoxSinAmp->setValue(1.0);
    ui->spinBoxSinOkr->setValue(10);
    ui->spinBoxKwadAkt->setValue(10);
    ui->doubleSpinBoxKwadAmp->setValue(1.0);
    ui->doubleSpinBoxKwadWyp->setValue(0.5);

    ui->doubleSpinBoxP->setValue(0.3);
    ui->doubleSpinBoxI->setValue(6.0);
    ui->doubleSpinBoxD->setValue(0.0);

    insertIntoTextField(ui->lineEditARXView_A, UAR.getARX_a_vector());
    insertIntoTextField(ui->lineEditARXView_B, UAR.getARX_b_vector());
    ui->spinBoxARX_k->setValue(UAR.getARX_k());
    ui->doubleSpinBoxARX_z->setValue(UAR.getARX_z_std_dev());

    ui->spinBoxInterwal->setValue(100);
    ui->spinBoxWidokKrokow->setValue(200);
    interwal_wykres_sec = ui->spinBoxInterwal->value() / 1000.0;

    ui->graphUAR->clearGraphs();
    uar_we_y.clear();
    uar_wy_y.clear();
    graph_x.clear();
    ui->graphUAR->xAxis->setRange(0.0, ui->spinBoxWidokKrokow->value() * interwal_wykres_sec);

    ui->graphUchyb->clearGraphs();
    uchyb_y.clear();
    ui->graphUchyb->xAxis->setRange(0.0, ui->spinBoxWidokKrokow->value() * interwal_wykres_sec);

    ui->graphPidSum->clearGraphs();
    pid_y.clear();
    ui->graphPidSum->xAxis->setRange(0.0, ui->spinBoxWidokKrokow->value() * interwal_wykres_sec);

    ui->graphPID->clearGraphs();
    p_y.clear();
    i_y.clear();
    d_y.clear();
    ui->graphPID->xAxis->setRange(0.0, ui->spinBoxWidokKrokow->value() * interwal_wykres_sec);

    setUpGraphs();
}

void MainWindow::on_btnResetD_clicked()
{
    UAR.resetPID_D();
}

void MainWindow::on_btnResetI_clicked()
{
    UAR.resetPID_I();
}

void MainWindow::on_btnStop_clicked()
{
    timer->stop();
    ui->btnStop->setEnabled(false);
    ui->btnStart->setEnabled(true);
    ui->btnReset->setEnabled(true);
    ui->labelStatus->setText("Zatrzymana");
    ui->groupBoxSkok->setDisabled(false);
    ui->groupBoxKwad->setDisabled(false);
    ui->groupBoxSin->setDisabled(false);
}

void MainWindow::on_groupBoxSkok_clicked()
{
    // zapobieganie odkliknięciu checkboxa

    if (!ui->groupBoxSkok->isChecked()) {
        ui->groupBoxSkok->setChecked(true);
    }
}

void MainWindow::on_groupBoxKwad_clicked()
{
    // zapobieganie odkliknięciu checkboxa

    if (!ui->groupBoxKwad->isChecked()) {
        ui->groupBoxKwad->setChecked(true);
    }
}

void MainWindow::on_groupBoxSin_clicked()
{
    // zapobieganie odkliknięciu checkboxa

    if (!ui->groupBoxSin->isChecked()) {
        ui->groupBoxSin->setChecked(true);
    }
}

void MainWindow::on_btnZapisz_clicked()
{
    passToSetters();
}

void MainWindow::on_btnARX_clicked()
{
    ui->btnARX->setDisabled(true);

    DataDialog data;
    data.a = UAR.getARX_a_vector();
    data.b = UAR.getARX_b_vector();
    data.delay = UAR.getARX_k();
    data.czyZakl = UAR.getARX_isZ();
    data.zakl_std_dev = UAR.getARX_z_std_dev();

    dialog = new DialogARX(data, this);

    if (dialog->exec())
    {
        UAR.clearARX_a();
        UAR.clearARX_b();

        for (const auto &_a : data.a) {
            UAR.addARX_a(_a);
        }

        for (const auto &_b : data.b) {
            UAR.addARX_b(_b);
        }

        UAR.setARX_k(data.delay);
        UAR.setARX_z(data.czyZakl);
        UAR.setARX_z_val(data.zakl_std_dev);

        insertIntoTextField(ui->lineEditARXView_A, data.a);
        insertIntoTextField(ui->lineEditARXView_B, data.b);
        ui->spinBoxARX_k->setValue(data.delay);
        ui->doubleSpinBoxARX_z->setValue(data.zakl_std_dev);
    }

    ui->btnARX->setDisabled(false);

}

double MainWindow::findMinRange(QVector<double> &y_data) {

    double min = std::numeric_limits<double>::max();

    for (int i = y_data.size()-1; i > std::max({static_cast<int>(y_data.size()-1 - ui->spinBoxWidokKrokow->value()), 0}); i--) {
        if (y_data[i] < min) {
            min = y_data[i];
        }
    }

    return min;
}

double MainWindow::findMaxRange(QVector<double> &y_data) {

    double max = std::numeric_limits<double>::min();

    for (int i = y_data.size()-1; i > std::max({static_cast<int>(y_data.size()-1 - ui->spinBoxWidokKrokow->value()), 0}); i--) {
        if (y_data[i] > max) {
            max = y_data[i];
        }
    }

    return max;
}

void MainWindow::setUpGraphs() {
    ui->graphUAR->addGraph();  // 0 - graf WE
    ui->graphUAR->addGraph(ui->graphUAR->graph(0)->keyAxis(), ui->graphUAR->graph(0)->valueAxis());     // 1 - graf WY
    ui->graphUAR->graph(0)->setAntialiased(true);
    ui->graphUAR->graph(1)->setAntialiased(true);
    // if (graph_x.size()-1 - ui->spinBoxWidokKrokow->value() >= 0)
    //     ui->graphUAR->xAxis->setRange(krok_czas - graph_x.at(graph_x.size()-1 - ui->spinBoxWidokKrokow->value()), krok_czas);
    // else
        ui->graphUAR->xAxis->setRange(0.0, krok_czas);
    ui->graphUAR->graph(0)->setPen(QPen(QColor(33,0,255), 2));
    ui->graphUAR->graph(1)->setPen(QPen(QColor(255,33,0), 2));
    ui->graphUAR->graph(0)->setName("W. zadana");
    ui->graphUAR->graph(1)->setName("W. regulowana");
    ui->graphUAR->legend->setVisible(true);
    QFont legendFont = font();
    legendFont.setPointSize(9);
    ui->graphUAR->legend->setFont(legendFont);
    ui->graphUAR->legend->setBrush(QBrush(QColor(255,255,255,100)));
    ui->graphUAR->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);


    ui->graphUchyb->addGraph();      // 0 - graf Uchyb
    ui->graphUchyb->graph(0)->setAntialiased(true);
    // if (graph_x.size()-1 - ui->spinBoxWidokKrokow->value() >= 0)
    //     ui->graphUAR->xAxis->setRange(krok_czas - graph_x.at(graph_x.size()-1 - ui->spinBoxWidokKrokow->value()), krok_czas);
    // else
        ui->graphUAR->xAxis->setRange(0.0, krok_czas);
    ui->graphUchyb->graph(0)->setPen(QPen(QColor(33,0,255), 2));
    ui->graphUchyb->graph(0)->setName("Uchyb");
    ui->graphUchyb->legend->setVisible(true);
    QFont legendFont2 = font();
    legendFont2.setPointSize(7);
    ui->graphUchyb->legend->setFont(legendFont2);
    ui->graphUchyb->legend->setBrush(QBrush(QColor(255,255,255,100)));
    ui->graphUchyb->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);

    ui->graphPidSum->addGraph();      // 0 - graf PID
    ui->graphPidSum->graph(0)->setAntialiased(true);
    // if (graph_x.size()-1 - ui->spinBoxWidokKrokow->value() >= 0)
    //     ui->graphUAR->xAxis->setRange(krok_czas - graph_x.at(graph_x.size()-1 - ui->spinBoxWidokKrokow->value()), krok_czas);
    // else
        ui->graphUAR->xAxis->setRange(0.0, krok_czas);
    ui->graphPidSum->graph(0)->setPen(QPen(QColor(33,0,255), 2));
    ui->graphPidSum->graph(0)->setName("PID");
    ui->graphPidSum->legend->setVisible(true);
    ui->graphPidSum->legend->setFont(legendFont2);
    ui->graphPidSum->legend->setBrush(QBrush(QColor(255,255,255,100)));
    ui->graphPidSum->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);

    ui->graphPID->addGraph();      // 0 - graf P
    ui->graphPID->addGraph(ui->graphPID->graph(0)->keyAxis(), ui->graphPID->graph(0)->valueAxis());      // 1 - graf I
    ui->graphPID->addGraph(ui->graphPID->graph(0)->keyAxis(), ui->graphPID->graph(0)->valueAxis());      // 2 - graf D
    ui->graphPID->graph(0)->setAntialiased(true);
    ui->graphPID->graph(1)->setAntialiased(true);
    ui->graphPID->graph(2)->setAntialiased(true);
    // if (graph_x.size()-1 - ui->spinBoxWidokKrokow->value() >= 0)
    //     ui->graphUAR->xAxis->setRange(krok_czas - graph_x.at(graph_x.size()-1 - ui->spinBoxWidokKrokow->value()), krok_czas);
    // else
        ui->graphUAR->xAxis->setRange(0.0, krok_czas);
    ui->graphPID->graph(0)->setPen(QPen(QColor(33,0,255), 2));
    ui->graphPID->graph(1)->setPen(QPen(QColor(255,33,0), 2));
    ui->graphPID->graph(2)->setPen(QPen(QColor(0, 255, 33),2));
    ui->graphPID->graph(0)->setName("P");
    ui->graphPID->graph(1)->setName("I");
    ui->graphPID->graph(2)->setName("D");
    ui->graphPID->legend->setVisible(true);
    ui->graphPID->legend->setFont(legendFont2);
    ui->graphPID->legend->setBrush(QBrush(QColor(255,255,255,100)));
    ui->graphPID->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);
}

void MainWindow::keyPressEvent(QKeyEvent* event) {

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
        on_btnZapisz_clicked();
}

void MainWindow::passToSetters() {

    if (ui->groupBoxSkok->isChecked()) {
        UAR.setSygnAmp(ui->doubleSpinBoxSkokAmp->value());
        UAR.setSygnOkrAkt(ui->spinBoxSkokAkt->value());
        UAR.setSygnWyp(0.0);
        UAR.setSygnStala(0.0);

        ui->graphUAR->graph(0)->setLineStyle(QCPGraph::lsStepLeft);
        ui->groupBoxKwad->setChecked(false);
        ui->groupBoxSin->setChecked(false);

    } else if (ui->groupBoxKwad->isChecked()) {
        UAR.setSygnAmp(ui->doubleSpinBoxKwadAmp->value());
        UAR.setSygnOkrAkt(ui->spinBoxKwadAkt->value());
        UAR.setSygnWyp(ui->doubleSpinBoxKwadWyp->value());
        UAR.setSygnStala(ui->doubleSpinBoxKwadStala->value());

        ui->graphUAR->graph(0)->setLineStyle(QCPGraph::lsStepLeft);
        ui->groupBoxSin->setChecked(false);
        ui->groupBoxSkok->setChecked(false);

    } else if (ui->groupBoxSin->isChecked()) {
        UAR.setSygnAmp(ui->doubleSpinBoxSinAmp->value());
        UAR.setSygnOkrAkt(ui->spinBoxSinOkr->value());
        UAR.setSygnWyp(0.0);
        UAR.setSygnStala(ui->doubleSpinBoxSinStala->value());

        ui->graphUAR->graph(0)->setLineStyle(QCPGraph::lsLine);
        ui->groupBoxSkok->setChecked(false);
        ui->groupBoxKwad->setChecked(false);

    }

    UAR.setPID_k(ui->doubleSpinBoxP->value());
    UAR.setPID_tI(ui->doubleSpinBoxI->value());
    UAR.setPID_tD(ui->doubleSpinBoxD->value());

    timer->setInterval(ui->spinBoxInterwal->value());
    interwal_wykres_sec = ui->spinBoxInterwal->value() / 1000.0;

    if (krok_czas <= ui->spinBoxWidokKrokow->value() * interwal_wykres_sec) {
        ui->graphUAR->xAxis->setRange(0.0, ui->spinBoxWidokKrokow->value() * interwal_wykres_sec);
        ui->graphUchyb->xAxis->setRange(0.0, ui->spinBoxWidokKrokow->value() * interwal_wykres_sec);
        ui->graphPidSum->xAxis->setRange(0.0, ui->spinBoxWidokKrokow->value() * interwal_wykres_sec);
        ui->graphPID->xAxis->setRange(0.0, ui->spinBoxWidokKrokow->value() * interwal_wykres_sec);
    }
    else {
        ui->graphUAR->xAxis->setRange(krok_czas - ui->spinBoxWidokKrokow->value() * interwal_wykres_sec, krok_czas);
        ui->graphUchyb->xAxis->setRange(krok_czas - ui->spinBoxWidokKrokow->value() * interwal_wykres_sec, krok_czas);
        ui->graphPidSum->xAxis->setRange(krok_czas - ui->spinBoxWidokKrokow->value() * interwal_wykres_sec, krok_czas);
        ui->graphPID->xAxis->setRange(krok_czas - ui->spinBoxWidokKrokow->value() * interwal_wykres_sec, krok_czas);
    }
}

void MainWindow::insertIntoTextField(QLineEdit* field, const std::vector<double> &arx_params)
{
    field->clear();
    field->insert("{ ");
    for (const auto &p : arx_params) {
        field->insert(QString::number(p) + "; ");
    }
    field->insert("}");
}

void MainWindow::initChartConfig(QCustomPlot *chart, const QString OX_num_format, const int OX_num_precision, const QString OX_label, const QString OY_label)
{
    chart->xAxis->setNumberFormat(OX_num_format);
    chart->xAxis->setNumberPrecision(OX_num_precision);
    chart->yAxis->setLabel(OY_label);
    chart->xAxis->setLabel(OX_label);
}


void MainWindow::on_radioStalaOut_toggled(bool checked)
{
    UAR.setPID_adjust_mem_on_change(checked);
}

