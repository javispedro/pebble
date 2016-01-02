#ifndef TIMELINEMANAGER_H
#define TIMELINEMANAGER_H

#include "watchconnector.h"
#include "blobdbmanager.h"

class TimelineManager : public QObject
{
    Q_OBJECT

public:
    explicit TimelineManager(WatchConnector *watch, BlobDbManager *blobdb, QObject *parent = 0);

signals:

public slots:

private:
    QLoggingCategory l;
    WatchConnector *watch;
    BlobDbManager *blobdb;
};

#endif // TIMELINEMANAGER_H
