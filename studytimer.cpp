#include <QInputDialog>
#include <QMessageBox>

#include "studytimer.h"
#include "ui_studytimer.h"
#include "LanguageParser.h"

#include <QDebug>
StudyTimer::StudyTimer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::StudyTimer)
{
    ui->setupUi(this);
    ui->startStudyTimer->setEnabled(false);
    ui->stopStudyTimer->setEnabled(false);
    ui->timeEdit->setDisplayFormat("hh:mm:ss ap");

}

StudyTimer::~StudyTimer()
{
    delete ui;
}

void StudyTimer::setRef(StudyTimer* c)
{
    ref = c;
}

void StudyTimer::setAQ(std::vector<Alert*>* alertsQueue)
{
    aQ = alertsQueue;
}

void StudyTimer::setMutex(QMutex* m)
{
    mtx = m;
}

void StudyTimer::setCurrentListWidgetItem(QListWidgetItem* tItem)
{
    currentTItem = tItem;
}

Alert* StudyTimer::getAlert(QListWidgetItem* tItem)
{
    auto it = alertWidgetMap.find(tItem);
    if (it != alertWidgetMap.end())
    {
        return it->second;
    }
    return nullptr;
}

std::string StudyTimer::timeToAlert(QDateTime inputAlertTime)
{
    std::string str_time = "";
    int days_left;
    int hours_left;
    int minutes_left;
    int seconds_left;

    QDateTime now = QDateTime::currentDateTime();
    QDateTime alertDateTime = QDateTime::currentDateTime();

    QDate alertDate = alertDateTime.date();
    alertDate.setDate(inputAlertTime.date().year(), inputAlertTime.date().month(), inputAlertTime.date().day());
    alertDateTime.setDate(alertDate);

    QTime alertTime = alertDateTime.time();
    alertTime.setHMS(inputAlertTime.time().hour(), inputAlertTime.time().minute(), inputAlertTime.time().second());
    alertDateTime.setTime(alertTime);

    int secondsToInputTime = now.secsTo(alertDateTime);   //Gets the amount of seconds to the alert

    if(secondsToInputTime < 0) {
        str_time = "Event already done";
        return str_time;
    }

    minutes_left = (secondsToInputTime / 60);  //Gets the amount of full minutes able to be made
    hours_left = (minutes_left / 60);  //Gets the amount of full hours able to be made
    days_left = (hours_left / 24); //Gets the amount of full days able to be made
    hours_left = hours_left % 24;   //Gets the left over hours not able to be made into days
    minutes_left = minutes_left % 60;   //Gets the left over minutes not able to be made into hours
    seconds_left = secondsToInputTime % 60; //Gets leftover seconds

    if(days_left > 0) {
        str_time += std::to_string(days_left);
        str_time += " days, ";
    }

    str_time += std::to_string(hours_left);
    str_time += " hours, ";
    str_time += std::to_string(minutes_left);
    str_time += " minutes, ";
    str_time += std::to_string(seconds_left);
    str_time += " seconds left.";

    return str_time;
}
void StudyTimer::on_timeEdit_userTimeChanged(const QTime &time)
{
    ui->stopStudyTimer->setEnabled(false);
    ui->startStudyTimer->setEnabled(true);

    QDateTime now = QDateTime::currentDateTime();
    QDateTime endDateTime = QDateTime::currentDateTime();
    endDateTime.setTime(time);

    int secondsToInputTime = now.time().secsTo(endDateTime.time());
    if (secondsToInputTime <= 0) // timer is being set for the following day
    {
        endDateTime = endDateTime.addDays(1);
    }

    endtime.setDate(endDateTime.date());
    endtime.setTime(time);
}

void StudyTimer::on_startStudyTimer_clicked()
{
    bool ok;
    QString inputdescr = QInputDialog::getText(0,"New StudyTimer",
            "Enter StudyTimer Name", QLineEdit::Normal, QString(), &ok);
    if (!ok || inputdescr == "")
    {
        ui->startStudyTimer->setEnabled(false);
        ui->stopStudyTimer->setEnabled(false);
        return;
    }

    QListWidgetItem* listAlert = new QListWidgetItem;
    listAlert->setText(inputdescr);
    ui->listWidget->addItem(listAlert);

    std::string inputdescr_str = inputdescr.toStdString();
    Alert* newAlert = new Alert(inputdescr_str, endtime);

    bool acquired = false;
    while(!acquired)
    {
        if(mtx->try_lock())
        {
            acquired = true;
            aQ->push_back(newAlert);
            mtx->unlock();
        }
    }

    alertWidgetMap.insert({listAlert, newAlert});
    ui->startStudyTimer->setEnabled(false);
}

void StudyTimer::on_listWidget_itemClicked(QListWidgetItem *item)
{
    setCurrentListWidgetItem(item);
    ui->startStudyTimer->setEnabled(false);
    ui->stopStudyTimer->setEnabled(true);
}

void StudyTimer::on_stopStudyTimer_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                "Stop StudyTimer", "Are you sure you want to cancel this timer?",
                QMessageBox::Yes | QMessageBox:: No);

    if(reply == QMessageBox::No)
    {
        ui->stopStudyTimer->setEnabled(false);
        return;
    }

    Alert* oldAlert = getAlert(currentTItem);

    bool acquired = false;
    while(!acquired)
    {
        if(mtx->try_lock())
        {
            acquired = true;
            aQ->erase(std::find(aQ->begin(), aQ->end(), oldAlert));
            delete oldAlert;
            mtx->unlock();
        }
    }

    alertWidgetMap.erase(currentTItem);
    currentTItem->setHidden(true);
    ui->stopStudyTimer->setEnabled(false);

    delete currentTItem;
}

void StudyTimer::remove_sTItem(Alert* a)
{
    QListWidgetItem* finished_tItem = nullptr;

    bool found = false;
    for (auto it = alertWidgetMap.begin(); it != alertWidgetMap.end(); ++it)
    {
        if (it->second == a)
        {
            found = true;
            finished_tItem = it->first;
            alertWidgetMap.erase(it->first);
            break;
        }
        if (found) { break; }
    }

    if (finished_tItem != nullptr)
    {
        delete finished_tItem;
    }

    delete a;
}

void StudyTimer::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ui->stopStudyTimer->setEnabled(false);
    ui->startStudyTimer->setEnabled(false);

    Alert *a = getAlert(item);
    //AlertTime t = a->getAlertTime();
    QDateTime t = a->getAlertTime();

    QString str_time = QString::fromStdString(timeToAlert(t));
    QMessageBox::information(0,"StudyTimer:", str_time);
}

void StudyTimer::voice_create_studytimer(int inputSeconds, QString name)
{
qDebug() << "in create: " << name;
    QDateTime now = QDateTime::currentDateTime();
    QDateTime endDateTime = QDateTime::currentDateTime();
    endDateTime = now.addSecs(inputSeconds);

    endtime = endDateTime;

qDebug() << "attribute set";

    std::string inputdescr_str;

    if(name == "") {
           std::string inputdescr_str = "New StudyTimer";
       } else {
           std::string inputdescr_str = name.toStdString();
       }

    QString inputdescr = QString::fromStdString(inputdescr_str);

    QListWidgetItem* listAlert = new QListWidgetItem;

    listAlert->setText(name);
    ui->listWidget->addItem(listAlert);

qDebug() << "widget added";
    Alert* newAlert = new Alert(inputdescr_str, endtime);

    bool acquired = false;
    while(!acquired)
    {
        if(mtx->try_lock())
        {
            acquired = true;
            aQ->push_back(newAlert);
            mtx->unlock();
        }
    }

    alertWidgetMap.insert({listAlert, newAlert});
    ui->stopStudyTimer->setEnabled(false);
    ui->startStudyTimer->setEnabled(false);
qDebug() << "widget mapped";
}
/*
#include <QInputDialog>
#include <QMessageBox>

#include "studytimer.h"
#include "ui_studytimer.h"

#include <QDebug>
StudyStudyTimer::StudyStudyTimer(QWidget *parent) :
	    QDialog(parent),
        ui(new Ui::StudyStudyTimer)
{
        ui->setupUi(this);
        ui->startStudyStudyTimer->setEnabled(false);
        ui->stopStudyStudyTimer->setEnabled(false);
}

StudyStudyTimer::~StudyStudyTimer()
{
	    delete ui;
}

void StudyStudyTimer::setRef(StudyStudyTimer* c)
{
    ref = c;
}

void StudyStudyTimer::setAQ(std::vector<Alert*>* alertsQueue)
{
    aQ = alertsQueue;
}

void StudyStudyTimer::setMutex(QMutex* m)
{
    mtx = m;
}

void StudyStudyTimer::setCurrentListWidgetItem(QListWidgetItem* sTItem)
{
    currentSTItem = sTItem;
}

Alert* StudyStudyTimer::getAlert(QListWidgetItem* stItem)
{
    auto it = alertWidgetMap.find(stItem);
    if (it != alertWidgetMap.end()){
        return it->second;
    }
    return nullptr;
}

void StudyStudyTimer::on_timeEdit_userTimeChanged(const QTime &time)
{
    ui->stopStudyStudyTimer->setEnabled(false);
    ui->startStudyStudyTimer->setEnabled(true);

    QDateTime now = QDateTime::currentDateTime();

    endtime.setAlertYear(now.date().year());
    endtime.setAlertMonth(now.date().month());
    endtime.setAlertDay(now.date().dayOfYear());

    if (time.hour() < now.time().hour() ||
            ( time.hour() == now.time().hour() &&
                time.minute() < now.time().minute()) ) // timer is being set for the following day
    {
        endtime.setAlertDay(now.date().dayOfYear() + 1);


        bool last_day_of_month_not_dec = false;

        bool leap_year = false;
        if (now.date().year() % 4 == 0)
        {
            leap_year = true;
        }

        if(leap_year) // == 366 days
        {
            if ( now.date().dayOfYear() == 31 ||
                    now.date().dayOfYear() == 60 ||
                    now.date().dayOfYear() == 91 ||
                    now.date().dayOfYear() == 121 ||
                    now.date().dayOfYear() == 152 ||
                    now.date().dayOfYear() == 182 ||
                    now.date().dayOfYear() == 213 ||
                    now.date().dayOfYear() == 244 ||
                    now.date().dayOfYear() == 274 ||
                    now.date().dayOfYear() == 305 ||
                    now.date().dayOfYear() == 335 )
            {
                last_day_of_month_not_dec = true;
            }
        }
        else // not a leap year == 365 days
        {
            if ( now.date().dayOfYear() == 31 ||
                now.date().dayOfYear() == 59 ||
                now.date().dayOfYear() == 90 ||
                now.date().dayOfYear() == 120 ||
                now.date().dayOfYear() == 151 ||
                now.date().dayOfYear() == 181 ||
                now.date().dayOfYear() == 212 ||
                now.date().dayOfYear() == 242 ||
                now.date().dayOfYear() == 273 ||
                now.date().dayOfYear() == 304 ||
                now.date().dayOfYear() == 334 )
            {
                last_day_of_month_not_dec = true;
            }
        }

        if (last_day_of_month_not_dec)
        {
            endtime.setAlertMonth(now.date().month() + 1);
        }

        if (leap_year)
        {
            if (now.date().dayOfYear() == 366) // increment year, set month to 1, set day to 1
            {
                endtime.setAlertYear(now.date().year() + 1);
                endtime.setAlertMonth(1);
                endtime.setAlertDay(1);
            }
        }
        else // not a leap year
        {
            if (now.date().dayOfYear() == 365) // increment year, set month to 1, set day to 1
            {
                endtime.setAlertYear(endtime.getAlertYear() + 1);
                endtime.setAlertMonth(1);
                endtime.setAlertDay(1);
            }
        }
    }

    endtime.setAlertHour(time.hour());
    endtime.setAlertMin(time.minute());
    endtime.setAlertSec(0);
}

void StudyStudyTimer::on_startStudyStudyTimer_clicked()
{
    bool ok;
    QString inputdescr = QInputDialog::getText(0,"New StudyTimer",
            "Enter StudyTimer Name", QLineEdit::Normal, QString(), &ok);
    if (!ok || inputdescr == ""){
        ui->startStudyStudyTimer->setEnabled(false);
        ui->stopStudyStudyTimer->setEnabled(false);
        return;
    }

    QListWidgetItem* listAlert = new QListWidgetItem;
    listAlert->setText(inputdescr);
    ui->listWidget->addItem(listAlert);

    std::string inputdescr_str = inputdescr.toLocal8Bit().constData();
    Alert* newAlert = new Alert(inputdescr_str, endtime);

    bool acquired = false;
    while(!acquired)
    {
        if(mtx->try_lock())
        {
            acquired = true;
            aQ->push_back(newAlert);
            mtx->unlock();
        }
    }

    alertWidgetMap.insert({listAlert, newAlert});
    ui->startStudyStudyTimer->setEnabled(false);
}

void StudyStudyTimer::on_listWidget_itemClicked(QListWidgetItem *item)
{
    setCurrentListWidgetItem(item);
    ui->startStudyStudyTimer->setEnabled(false);
    ui->stopStudyStudyTimer->setEnabled(true);
}

void StudyStudyTimer::on_stopStudyStudyTimer_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                "Stop StudyTimer", "Are you sure you want to cancel this timer?",
                QMessageBox::Yes | QMessageBox:: No);

    if(reply == QMessageBox::No)
    {
        ui->stopStudyStudyTimer->setEnabled(false);
        return;
    }

    Alert* oldAlert = getAlert(currentSTItem);

    bool acquired = false;
    while(!acquired)
    {
        if(mtx->try_lock())
        {
            acquired = true;
            aQ->erase(std::find(aQ->begin(), aQ->end(), oldAlert));
            delete oldAlert;
            mtx->unlock();
        }
    }

    alertWidgetMap.erase(currentSTItem);
    currentSTItem->setHidden(true);
    ui->stopStudyStudyTimer->setEnabled(false);

    delete currentSTItem;
}

void StudyStudyTimer::remove_stItem(Alert* a)
{
    QListWidgetItem* finished_tItem = nullptr;

    bool found = false;
    for (auto it = alertWidgetMap.begin(); it != alertWidgetMap.end(); ++it)
    {
        if (it->second == a)
        {
            found = true;
            finished_tItem = it->first;
            alertWidgetMap.erase(it->first);
            break;
        }
        if (found) { break; }
    }

    if (finished_tItem != nullptr)
    {
        delete finished_tItem;
    }

    delete a;
}

void StudyStudyTimer::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ui->stopStudyStudyTimer->setEnabled(false);
    ui->startStudyStudyTimer->setEnabled(false);

    Alert *a = getAlert(item);
    AlertTime t = a->getAlertTime();

    QString str_time = QString::fromStdString(t.alertTimeToString(t));
    QMessageBox::information(0,"StudyTimer will go off at ", "", str_time);
}

void StudyStudyTimer::voice_create_studytimer(int timeTo)
{

}
*/
