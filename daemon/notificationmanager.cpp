#include <QDebug>

#include "libwatchfish/notificationmonitor.h"

#include "notificationmanager.h"

using watchfish::Notification;
using watchfish::NotificationMonitor;

static NotificationMonitor *monitor = 0;

NotificationManager::NotificationManager(Settings *settings, QObject *parent)
    : QObject(parent), l(metaObject()->className()), settings(settings)
{
    if (!monitor) {
        monitor = new watchfish::NotificationMonitor;
    }
    connect(monitor, &NotificationMonitor::notification,
            this, &NotificationManager::handleNotification);
}

void NotificationManager::handleNotification(const watchfish::Notification *notification)
{
    QString appId = notification->appId();

    // Ignore notifcations from myself
    if (appId == "pebbled") {
        return;
    }

    // new place to check notification owner in Sailfish 1.1.6
    QString owner; // TODO hints.value("x-nemo-owner").toString();
    QHash<QString, QVariant> hints;
    QString summary = notification->summary();
    QString body = notification->body();


    if (appId == "messageserver5" || owner == "messageserver5") {
        if (!settings->property("notificationsEmails").toBool()) {
            qCDebug(l) << "Ignoring email notification because of setting!";
            return;
        }

        QString subject = hints.value("x-nemo-preview-summary", "").toString();
        QString data = hints.value("x-nemo-preview-body", "").toString();

        // Prioritize subject over data
        if (subject.isEmpty() && !data.isEmpty()) {
            subject = data;
            data = "";
        }

        if (!subject.isEmpty()) {
            emit this->emailNotify(subject, data, "");
        }
    } else if (appId == "commhistoryd" || owner == "commhistoryd") {
        if (summary == "" && body == "") {
            QString category = hints.value("category", "").toString();

            if (category == "x-nemo.call.missed") {
                if (!settings->property("notificationsMissedCall").toBool()) {
                    qCDebug(l) << "Ignoring MissedCall notification because of setting!";
                    return;
                }
            } else {
                if (!settings->property("notificationsCommhistoryd").toBool()) {
                    qCDebug(l) << "Ignoring commhistoryd notification because of setting!";
                    return;
                }
            }
            emit this->smsNotify(hints.value("x-nemo-preview-summary", "default").toString(),
                                 hints.value("x-nemo-preview-body", "default").toString()
                                );
        }
    } else if (appId == "harbour-mitakuuluu2-server" || owner == "harbour-mitakuuluu2-server") {
        if (!settings->property("notificationsMitakuuluu").toBool()) {
            qCDebug(l) << "Ignoring mitakuuluu notification because of setting!";
            return;
        }

        emit this->smsNotify(hints.value("x-nemo-preview-body", "default").toString(),
                             hints.value("x-nemo-preview-summary", "default").toString()
                            );
    } else if (appId == "twitter-notifications-client" || owner == "twitter-notifications-client") {
        if (!settings->property("notificationsTwitter").toBool()) {
            qCDebug(l) << "Ignoring twitter notification because of setting!";
            return;
        }

        emit this->twitterNotify(hints.value("x-nemo-preview-body", body).toString(),
                                 hints.value("x-nemo-preview-summary", summary).toString()
                                );
    } else {
        // Prioritize x-nemo-preview* over dbus direct summary and body
        QString subject = hints.value("x-nemo-preview-summary", "").toString();
        QString data = hints.value("x-nemo-preview-body", "").toString();
        QString category = hints.value("category", "").toString();
        QHash<QString,QString> categoryParams; // TODO
        int prio = categoryParams.value("x-nemo-priority", "0").toInt();

        qCDebug(l) << "MSG Prio:" << prio;

        if (!settings->property("notificationsAll").toBool() && prio <= 10) {
            qCDebug(l) << "Ignoring notification because of setting! (all)";
            return;
        }

        if (!settings->property("notificationsOther").toBool() && prio < 90) {
            qCDebug(l) << "Ignoring notification because of setting! (other)";
            return;
        }

        if (subject.isEmpty()) {
            subject = summary;
        }
        if (data.isEmpty()) {
            data = body;
        }

        //Prioritize data over subject
        if (data.isEmpty() && !subject.isEmpty()) {
            data = subject;
            subject = "";
        }

        //Never send empty data and subject
        if (data.isEmpty() && subject.isEmpty()) {
            qCWarning(l) << Q_FUNC_INFO << "Empty subject and data in dbus app:" << appId;
            return;
        }

        emit this->emailNotify(notification->appName(), data, subject);
    }
}
