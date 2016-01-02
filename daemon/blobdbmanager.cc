#include "blobdbmanager.h"
#include "unpacker.h"
#include "packer.h"

BlobDbManager::BlobDbManager(WatchConnector *watch, QObject *parent) :
    QObject(parent), l(metaObject()->className()), watch(watch),
    lastCookie(0)
{
    watch->setEndpointHandler(WatchConnector::watchBLOB_DB,
                              [this](const QByteArray &msg) {
        handleMessage(msg);
        return true;
    });
}

void BlobDbManager::insert(BlobDatabase db, const QUuid &key, const QByteArray &data, const SuccessCallback &successCallback, const ErrorCallback &errorCallback)
{
    QByteArray msg;
    Packer p(&msg);
    PendingTransfer transfer;
    transfer.cookie = ++lastCookie;
    transfer.successCallback = successCallback;
    transfer.errorCallback = errorCallback;

    p.writeLE<quint8>(WatchConnector::blobdbINSERT);
    p.writeLE<quint16>(transfer.cookie);
    p.writeLE<quint8>(db);

    p.writeLE<quint8>(16);
    p.writeUuid(key);

    p.writeLE<quint16>(data.size());
    msg.append(data);

    pending.insert(transfer.cookie, transfer);
    watch->sendMessage(WatchConnector::watchBLOB_DB, msg);
}

void BlobDbManager::remove(BlobDatabase db, const QUuid &key, const SuccessCallback &successCallback, const ErrorCallback &errorCallback)
{
    QByteArray msg;
    Packer p(&msg);
    PendingTransfer transfer;
    transfer.cookie = ++lastCookie;
    transfer.successCallback = successCallback;
    transfer.errorCallback = errorCallback;

    p.writeLE<quint8>(WatchConnector::blobdbINSERT);
    p.writeLE<quint16>(transfer.cookie);
    p.writeLE<quint8>(db);

    p.writeLE<quint8>(16);
    p.writeUuid(key);

    pending.insert(transfer.cookie, transfer);
    watch->sendMessage(WatchConnector::watchBLOB_DB, msg);
}

void BlobDbManager::clear(BlobDatabase db, const SuccessCallback &successCallback, const ErrorCallback &errorCallback)
{
    QByteArray msg;
    Packer p(&msg);
    PendingTransfer transfer;
    transfer.cookie = ++lastCookie;
    transfer.successCallback = successCallback;
    transfer.errorCallback = errorCallback;

    p.writeLE<quint8>(WatchConnector::blobdbINSERT);
    p.writeLE<quint16>(transfer.cookie);
    p.writeLE<quint8>(db);

    pending.insert(transfer.cookie, transfer);
    watch->sendMessage(WatchConnector::watchBLOB_DB, msg);
}

void BlobDbManager::handleMessage(const QByteArray &msg)
{
    Unpacker u(msg);
    Cookie cookie = u.readLE<quint16>();
    quint8 status = u.readLE<quint8>();

    PendingTransfer transfer = pending.take(cookie);
    if (transfer.cookie != cookie) {
        qCWarning(l) << "received a message for a cookie we don't know" << cookie;
        return;
    }

    if (status == 0 || status == WatchConnector::blobdbSUCCESS) {
        qCDebug(l) << "success on cookie" << cookie;
        if (transfer.successCallback) {
            transfer.successCallback();
        }
    } else {
        qCWarning(l) << "failure on cookie" << cookie << "with status" << status;
        if (transfer.errorCallback) {
            transfer.errorCallback(WatchConnector::BlobDbStatus(status));
        }
    }
}
