#ifndef BLOBDBMANAGER_H
#define BLOBDBMANAGER_H

#include <functional>
#include "watchconnector.h"

class BlobDbManager : public QObject
{
    Q_OBJECT
public:
    explicit BlobDbManager(WatchConnector *watch, QObject *parent = 0);

    enum BlobDatabase {
        BlobDbTest = 0,
        BlobDbPin = 1,
        BlobDbApp = 2,
        BlobDbReminder = 3,
        BlobDbNotification = 4
    };

    typedef std::function<void()> SuccessCallback;
    typedef std::function<void(WatchConnector::BlobDbStatus)> ErrorCallback;

    void insert(BlobDatabase db, const QUuid &key, const QByteArray &data, const SuccessCallback &successCallback = SuccessCallback(), const ErrorCallback &errorCallback = ErrorCallback());
    void remove(BlobDatabase db, const QUuid &key, const SuccessCallback &successCallback = SuccessCallback(), const ErrorCallback &errorCallback = ErrorCallback());
    void clear(BlobDatabase db, const SuccessCallback &successCallback = SuccessCallback(), const ErrorCallback &errorCallback = ErrorCallback());

private:
    typedef quint16 Cookie;

    struct PendingTransfer {
        Cookie cookie;
        SuccessCallback successCallback;
        ErrorCallback errorCallback;
    };

private:
    void handleMessage(const QByteArray &msg);

private:
    QLoggingCategory l;
    WatchConnector *watch;

    QHash<Cookie, PendingTransfer> pending;
    Cookie lastCookie;
};

#endif // BLOBDBMANAGER_H
