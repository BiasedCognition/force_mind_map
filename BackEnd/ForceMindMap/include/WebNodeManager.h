#ifndef WEBNODEMANAGER_H
#define WEBNODEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QList>
#include <QDebug>

class WebNodeManager : public QObject
{
    Q_OBJECT
public:
    explicit WebNodeManager(QObject *parent = nullptr);

    Q_INVOKABLE void addNode(const QString& id, const QString& text, const QString& parentId = "");
    Q_INVOKABLE void removeNode(const QString& id);
    Q_INVOKABLE void updateNodeText(const QString& id, const QString& newText);
    Q_INVOKABLE void addConnection(const QString& sourceId, const QString& targetId);
    Q_INVOKABLE void removeConnection(const QString& sourceId, const QString& targetId);
    Q_INVOKABLE QJsonObject getFullGraph() const;
    Q_INVOKABLE void appReady();

    signals:
        void nodeAdded(const QJsonObject& node);
    void nodeRemoved(const QString& id);
    void nodeUpdated(const QJsonObject& node);
    void connectionAdded(const QJsonObject& link);
    void connectionRemoved(const QString& sourceId, const QString& targetId);
    void graphUpdated(const QJsonObject& fullGraph);

private:
    QMap<QString, QJsonObject> m_nodes;
    QList<QJsonObject> m_links;
};

#endif // WEBNODEMANAGER_H