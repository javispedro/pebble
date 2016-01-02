#include "timelinemanager.h"

TimelineManager::TimelineManager(WatchConnector *watch, BlobDbManager *blobdb, QObject *parent)
    : QObject(parent), l(metaObject()->className()), watch(watch), blobdb(blobdb)
{
}

