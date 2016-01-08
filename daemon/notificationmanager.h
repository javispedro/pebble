#ifndef NOTIFICATIONMANAGER_H
#define NOTIFICATIONMANAGER_H

#include <QObject>
#include <QLoggingCategory>
#include "libwatchfish/notification.h"
#include "settings.h"

class NotificationManager : public QObject
{
    Q_OBJECT
    QLoggingCategory l;

public:
    explicit NotificationManager(Settings *settings, QObject *parent = 0);

Q_SIGNALS:
    void error(const QString &message);
    void smsNotify(const QString &sender, const QString &data);
    void twitterNotify(const QString &sender, const QString &data);
    void facebookNotify(const QString &sender, const QString &data);
    void emailNotify(const QString &sender, const QString &data,const QString &subject);

private Q_SLOTS:
    void handleNotification(const watchfish::Notification *notification);

private:
    QHash<uint, watchfish::Notification*> notifications;
    Settings *settings;
};

#endif // NOTIFICATIONMANAGER_H
