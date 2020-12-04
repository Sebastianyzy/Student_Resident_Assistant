#include "voicecommand.h"
#include "LanguageParser.h"
#include <QDebug>
#include <QString>

VoiceCommand::VoiceCommand(QObject *parent) : QObject(parent) {}

VoiceCommand::~VoiceCommand() {}

void VoiceCommand::receive_command(QString naturalLang)
{
    LanguageParser::Command command = LanguageParser::parseCommand(naturalLang.toStdString());

    bool error = false;
if(command.target == "timer"){
    switch(command.action){
        case LanguageParser::ACTIONS::CREATE:
        case LanguageParser::ACTIONS::START:
            if(command.param1Type == LanguageParser::COMPLEMENT_TYPE::TIME_REL) {   //Might cause problems
                emit create_timer(command.param1, QString::fromStdString(command.param3));
            } else { error = true; }
            break;
        case LanguageParser::ACTIONS::EDIT:
            break;
        default:
            error = true;
    }
} else if ( command.target == "appointment" || command.target == "event" || command.target == "reminder" ){
    switch(command.action){
        case LanguageParser::ACTIONS::CREATE:
            if(command.dayOfWeek != LanguageParser::DAY_OF_WEEK::NO_DATE) {
                emit create_calendarEvent(LanguageParser::dowToInt(command.dayOfWeek), QString::fromStdString(command.param3));
            } else { error = true; }
            break;
        case LanguageParser::ACTIONS::EDIT:
            break;
        default:
            error = true;
    }
} else if(command.target == "note"){
    switch(command.action){
        case LanguageParser::ACTIONS::CREATE:
            if(command.param3Type == LanguageParser::COMPLEMENT_TYPE::TEXT) {
                emit create_note(QString::fromStdString(command.param3));
            } else { error = true; }
            break;
        case LanguageParser::ACTIONS::EDIT:
            break;
        default:
            error = true;
    }
} else if(command.target == "study timer"){
    switch(command.action){
        case LanguageParser::ACTIONS::CREATE:
            //if(command.param1Type == LanguageParser::COMPLEMENT_TYPE::TIME_REL) {
                emit create_timer(command.param1, QString::fromStdString(command.param3));
           // } else { error = true; }
            break;
        case LanguageParser::ACTIONS::EDIT:
            break;
        case LanguageParser::ACTIONS::START:
            break;
        default:
            error = true;
    }
} else if(command.target == "checklist"){
    switch(command.action){
        case LanguageParser::ACTIONS::CREATE:
        if(command.param3Type == LanguageParser::COMPLEMENT_TYPE::TEXT) {
            emit create_checklistItem(QString::fromStdString(command.param3));
        } else { error = true; }
            break;
        case LanguageParser::ACTIONS::EDIT:
            break;
        default:
            error = true;
    }
}
if(error){
    //--TODO: Throw some error here--
}
}





/*
#include "voicecommand.h"
#include "LanguageParser.h"
#include <stdexcept>

#include <QDebug>
VoiceCommand::VoiceCommand(QObject *parent) : QObject(parent) {}

VoiceCommand::~VoiceCommand() {}

void VoiceCommand::receive_command(QString naturalLang)
{
    LanguageParser::Command command = LanguageParser::parseCommand(naturalLang.toStdString());
    bool error = false;
    if(command.target == "timer"){
        switch(command.action){
            case LanguageParser::LanguageParser::ACTIONS::CREATE:
                //if(command.param1Type == LanguageParser::LanguageParser::COMPLEMENT_TYPE::TIME_REL){
                    //Create a timer for command.param1 seconds
                    emit create_timer(command.param1, "New Timer");
                    break;
                //}else{ error = true; }
                break;
            case LanguageParser::LanguageParser::ACTIONS::DELETE:
                break;
            case LanguageParser::LanguageParser::ACTIONS::EDIT:
                break;
            case LanguageParser::LanguageParser::ACTIONS::START:
                break;
            case LanguageParser::LanguageParser::ACTIONS::STOP:
                break;
            case LanguageParser::LanguageParser::ACTIONS::REMIND:
            case LanguageParser::LanguageParser::ACTIONS::BADACTION:
                error = true;
                break;
        }
    }else if(command.target == "reminder"){
        switch(command.action){
            case LanguageParser::LanguageParser::ACTIONS::REMIND:
            case LanguageParser::LanguageParser::ACTIONS::CREATE:
                //create a reminder
// emit create_calendarEvent() ARGS?
                break;
            case LanguageParser::LanguageParser::ACTIONS::DELETE:
                break;
            case LanguageParser::LanguageParser::ACTIONS::EDIT:
                break;
            case LanguageParser::LanguageParser::ACTIONS::STOP:
            case LanguageParser::LanguageParser::ACTIONS::START:
            case LanguageParser::LanguageParser::ACTIONS::BADACTION:
                error = true;
                break;
        }
    }else if(command.target == "appointment" || command.target == "event"){
        switch(command.action){
            case LanguageParser::LanguageParser::ACTIONS::CREATE:
// emit create_calendarEvent() ARGS?
                break;
            case LanguageParser::LanguageParser::ACTIONS::DELETE:
                break;
            case LanguageParser::LanguageParser::ACTIONS::EDIT:
                break;
            case LanguageParser::LanguageParser::ACTIONS::START:
            case LanguageParser::LanguageParser::ACTIONS::STOP:
            case LanguageParser::LanguageParser::ACTIONS::REMIND:
            case LanguageParser::LanguageParser::ACTIONS::BADACTION:
                error = true;
                break;
        }
    }else if(command.target == "note"){
        switch(command.action){
            case LanguageParser::LanguageParser::ACTIONS::CREATE:
// emit create_note() ARGS?
                break;
            case LanguageParser::LanguageParser::ACTIONS::DELETE:
                break;
            case LanguageParser::LanguageParser::ACTIONS::EDIT:
                break;
            case LanguageParser::LanguageParser::ACTIONS::START:
            case LanguageParser::LanguageParser::ACTIONS::STOP:
            case LanguageParser::LanguageParser::ACTIONS::REMIND:
            case LanguageParser::LanguageParser::ACTIONS::BADACTION:
                error = true;
                break;
        }
    }else if(command.target == "study timer"){
        switch(command.action){
            case LanguageParser::LanguageParser::ACTIONS::CREATE:
// emit create_studyTimer() ARGS?
                break;
            case LanguageParser::LanguageParser::ACTIONS::DELETE:
                break;
            case LanguageParser::LanguageParser::ACTIONS::EDIT:
                break;
            case LanguageParser::LanguageParser::ACTIONS::START:
                break;
            case LanguageParser::LanguageParser::ACTIONS::STOP:
                break;
            case LanguageParser::LanguageParser::ACTIONS::REMIND:
            case LanguageParser::LanguageParser::ACTIONS::BADACTION:
                error = true;
                break;
        }
    }else if(command.target == "checklist"){
        switch(command.action){
            case LanguageParser::LanguageParser::ACTIONS::CREATE:
// emit create_checklistItem() ARGS?
                break;
            case LanguageParser::LanguageParser::ACTIONS::DELETE:
                break;
            case LanguageParser::LanguageParser::ACTIONS::EDIT:
                break;
            case LanguageParser::LanguageParser::ACTIONS::START:
            case LanguageParser::LanguageParser::ACTIONS::STOP:
            case LanguageParser::LanguageParser::ACTIONS::REMIND:
            case LanguageParser::LanguageParser::ACTIONS::BADACTION:
                error = true;
                break;
        }
    }
    if(error){
        //--TODO: Throw some error here--
    }

}
*/
