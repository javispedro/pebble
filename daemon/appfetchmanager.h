#ifndef APPFETCHMANAGER_H
#define APPFETCHMANAGER_H

#include <QTimer>
#include <QUuid>
#include <QVector>
#include <QLoggingCategory>

#include "watchconnector.h"

class UploadManager;
class AppManager;

class AppFetchManager : public QObject
{
    Q_OBJECT
    QLoggingCategory l;

public:
    explicit AppFetchManager(WatchConnector *watch, UploadManager *upload, AppManager *apps, QObject *parent = 0);

private:
    void handleAppFetchRequest(const QByteArray &msg);
    void sendAppFetchResponse(WatchConnector::AppFetchMessage command, WatchConnector::AppFetchStatus status);

    void uploadApp(const QUuid &uuid, quint32 app_id);

private:
    WatchConnector *watch;
    UploadManager *upload;
    AppManager *apps;
};

#endif // APPFETCHMANAGER_H
