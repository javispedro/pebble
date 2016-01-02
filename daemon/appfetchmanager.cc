#include "appfetchmanager.h"
#include "appmanager.h"
#include "uploadmanager.h"
#include "unpacker.h"
#include "packer.h"

AppFetchManager::AppFetchManager(WatchConnector *watch, UploadManager *upload, AppManager *apps, QObject *parent)
    : QObject(parent), l(metaObject()->className()),
      watch(watch), upload(upload), apps(apps)

{
    watch->setEndpointHandler(WatchConnector::watchAPP_FETCH,
                              [this](const QByteArray &data) {
        switch (data.at(0)) {
        case WatchConnector::appfetchREQUEST:
            handleAppFetchRequest(data);
            break;
        }

        return true;
    });
}

void AppFetchManager::handleAppFetchRequest(const QByteArray &msg)
{
    Unpacker u(msg);
    quint8 command = u.readLE<quint8>();
    QUuid uuid = u.readUuid();
    qint32 appId = u.readLE<qint32>();

    Q_ASSERT(command == WatchConnector::appfetchREQUEST);

    uploadApp(uuid, appId);
}

void AppFetchManager::sendAppFetchResponse(WatchConnector::AppFetchMessage command, WatchConnector::AppFetchStatus status)
{
    QByteArray msg;
    Packer p(&msg);

    p.writeLE<quint8>(command);
    p.writeLE<quint8>(status);

    watch->sendMessage(WatchConnector::watchAPP_FETCH, msg);
}

void AppFetchManager::uploadApp(const QUuid &uuid, quint32 appId)
{
    AppInfo info = apps->info(uuid);
    if (info.uuid() != uuid) {
        qCWarning(l) << "uuid" << uuid << "is not installed";
        sendAppFetchResponse(WatchConnector::appfetchREQUEST, WatchConnector::appfetchINVALID_UUID);
        return;
    }

    qCDebug(l) << "about to upload app" << info.shortName();

    QSharedPointer<QIODevice> binaryFile(info.openFile(AppInfo::APPLICATION));
    if (!binaryFile) {
        qCWarning(l) << "failed to open" << info.shortName() << "AppInfo::BINARY";
        sendAppFetchResponse(WatchConnector::appfetchREQUEST, WatchConnector::appfetchINVALID_UUID);
        return;
    }

    qCDebug(l) << "binary file size is" << binaryFile->size();

    upload->uploadAppBinaryId(appId, binaryFile.data(), info.crcFile(AppInfo::APPLICATION),
    [this, info, binaryFile, appId]() {
        qCDebug(l) << "app binary upload succesful";
        binaryFile->close();

        // Upload worker if present
        if (info.type() == "worker") {
            QSharedPointer<QIODevice> workerFile(info.openFile(AppInfo::WORKER));
            if (workerFile) {
                upload->uploadAppWorkerId(appId, workerFile.data(), info.crcFile(AppInfo::WORKER),
                [this, workerFile]() {
                    qCDebug(l) << "app worker upload succesful";
                    workerFile->close();
                }, [this, workerFile](int code) {
                    qCWarning(l) << "app worker upload failed" << code;
                    workerFile->close();
                });
            }
        }

        // Proceed to upload the resource file
        QSharedPointer<QIODevice> resourceFile(info.openFile(AppInfo::RESOURCES));
        if (resourceFile) {
            upload->uploadAppResourcesId(appId, resourceFile.data(), info.crcFile(AppInfo::RESOURCES),
            [this, resourceFile]() {
                qCDebug(l) << "app resources upload succesful";
                resourceFile->close();
                // Upload succesful
            }, [this, resourceFile](int code) {
                qCWarning(l) << "app resources upload failed" << code;
                resourceFile->close();
            });
        } else {
            // No resource file
        }
    }, [this, binaryFile](int code) {
        binaryFile->close();
        qCWarning(l) << "app binary upload failed" << code;
    });
}
