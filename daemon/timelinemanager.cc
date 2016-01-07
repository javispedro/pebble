#include "timelinemanager.h"
#include "packer.h"

TimelineManager::TimelineManager(WatchConnector *watch, BlobDbManager *blobdb, QObject *parent)
    : QObject(parent), l(metaObject()->className()), watch(watch), blobdb(blobdb)
{
}

void TimelineManager::publish(const Item &item)
{
    QByteArray data;
    Packer p(&data);

    p.writeUuid(item.id);
    p.writeUuid(item.parent);
    p.writeLE<quint32>(item.dateTime.toTime_t());
    p.writeLE<quint16>(item.duration);
    p.writeLE<quint8>(item.type);
    p.writeLE<quint16>(item.flags);
    p.writeLE<quint8>(item.layout);

    QByteArray attributes = packAttributes(item.attributes);
    QByteArray actions = packActions(item.actions);

    p.writeLE<quint16>(attributes.size() + actions.size());
    p.writeLE<quint8>(item.attributes.size());
    p.writeLE<quint8>(item.actions.size());

    data.append(attributes);
    data.append(actions);

    blobdb->insert(BlobDbManager::BlobDbNotification, item.id, data);
}

QByteArray TimelineManager::packAttributes(const QList<Attribute>& attributes)
{
    QByteArray data;
    Packer p(&data);

    foreach (const Attribute &attr, attributes) {
        p.writeLE<quint8>(attr.type);
        p.writeLE<quint16>(attr.content.size());
        data.append(attr.content);
    }

    return data;
}

QByteArray TimelineManager::packActions(const QList<Action>& actions)
{
    QByteArray data;
    Packer p(&data);

    foreach (const Action &action, actions) {
        p.writeLE<quint8>(action.id);
        p.writeLE<quint8>(action.type);
        p.writeLE<quint8>(action.attributes.size());
        data.append(packAttributes(action.attributes));
    }

    return data;
}
