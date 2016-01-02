#ifndef UPLOADMANAGER_H
#define UPLOADMANAGER_H

#include <functional>
#include <QQueue>
#include "watchconnector.h"

class UploadManager : public QObject
{
    Q_OBJECT
    QLoggingCategory l;

public:
    explicit UploadManager(WatchConnector *watch, QObject *parent = 0);

    typedef std::function<void()> SuccessCallback;
    typedef std::function<void(int)> ErrorCallback;
    typedef std::function<void(qreal)> ProgressCallback;

    uint upload(WatchConnector::UploadType type, int index, const QString &filename, QIODevice *device, int size = -1, quint32 crc = 0,
                SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());

    uint uploadAppBinaryInSlot(int slot, QIODevice *device, quint32 crc, SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());
    uint uploadAppResourcesInSlot(int slot, QIODevice *device, quint32 crc, SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());
    uint uploadAppWorkerInSlot(int slot, QIODevice *device, quint32 crc, SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());
    uint uploadAppBinaryId(int id, QIODevice *device, quint32 crc, SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());
    uint uploadAppResourcesId(int id, QIODevice *device, quint32 crc, SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());
    uint uploadAppWorkerId(int id, QIODevice *device, quint32 crc, SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());
    uint uploadFile(const QString &filename, QIODevice *device, quint32 crc, SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());
    uint uploadFirmwareBinary(bool recovery, QIODevice *device, quint32 crc, SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());
    uint uploadFirmwareResources(QIODevice *device, quint32 crc, SuccessCallback successCallback = SuccessCallback(), ErrorCallback errorCallback = ErrorCallback(), ProgressCallback progressCallback = ProgressCallback());

    void cancel(uint id, int code = 0);

signals:

public slots:


private:
    enum State {
        StateNotStarted,
        StateWaitForToken,
        StateInProgress,
        StateCommit,
        StateComplete
    };

    struct PendingUpload {
        uint id;

        WatchConnector::UploadType type;
        int index; // Used both as the slot number, or the AppID, depending on type
        QString filename;
        QIODevice *device;
        int size;
        int remaining;
        quint32 crc;

        SuccessCallback successCallback;
        ErrorCallback errorCallback;
        ProgressCallback progressCallback;
    };

    void startNextUpload();
    void handleMessage(const QByteArray &msg);
    bool uploadNextChunk(PendingUpload &upload);
    bool commit(PendingUpload &upload);
    bool complete(PendingUpload &upload);

private:
    WatchConnector *watch;
    QQueue<PendingUpload> _pending;
    uint _lastUploadId;
    State _state;
    quint32 _token;
};

#endif // UPLOADMANAGER_H
