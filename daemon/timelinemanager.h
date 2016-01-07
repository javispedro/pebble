#ifndef TIMELINEMANAGER_H
#define TIMELINEMANAGER_H

#include "watchconnector.h"
#include "blobdbmanager.h"

class TimelineManager : public QObject
{
    Q_OBJECT

public:
    explicit TimelineManager(WatchConnector *watch, BlobDbManager *blobdb, QObject *parent = 0);

    enum ItemType {
        TimelineNotification = 1,
        TimelinePin = 2,
        TimelineReminder = 3
    };

    enum AttributeType {
        AttributeSender = 1,
        AttributeSubject = 2,
        AttributeMessage = 3,
        AttributeSource = 4
    };

    enum NotificationSource {
        SourceUnknown = 1,
        SourceEmail = 19,
        SourceFacebook = 11,
        SourceSMS = 45,
        SourceTwitter = 6
    };

    struct Attribute {
        AttributeType type;
        QByteArray content;
    };

    struct Action {
        quint8 id;
        quint8 type; // TODO
        QList<Attribute> attributes;
    };

    struct Item {
        QUuid id;
        QUuid parent;
        QDateTime dateTime;
        unsigned short duration;
        ItemType type;
        quint16 flags;
        quint8 layout;
        QList<Attribute> attributes;
        QList<Action> actions;
    };

public slots:
    void publish(const Item &item);

private:
    static QByteArray packAttributes(const QList<Attribute>& attributes);
    static QByteArray packActions(const QList<Action>& actions);

private:
    QLoggingCategory l;
    WatchConnector *watch;
    BlobDbManager *blobdb;
};

#endif // TIMELINEMANAGER_H
