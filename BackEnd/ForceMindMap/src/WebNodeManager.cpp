#include "WebNodeManager.h"
#include <QJsonDocument>
#include <QJsonArray>

WebNodeManager::WebNodeManager(QObject *parent) : QObject(parent)
{
}

void WebNodeManager::addNode(const QString& id, const QString& text, const QString& parentId)
{
    if (m_nodes.contains(id)) {
        qWarning() << "Node with id" << id << "already exists";
        return;
    }

    QJsonObject node;
    node["id"] = id;
    node["text"] = text;
    node["parent"] = parentId;

    m_nodes.insert(id, node);
    emit nodeAdded(node);
    // 仅发送节点添加事件，避免每次小更改都发送完整图数据
}

void WebNodeManager::removeNode(const QString& id)
{
    if (m_nodes.contains(id)) {
        m_nodes.remove(id);

        // 移除相关链接
        QList<QJsonObject> linksToRemove;
        for (const auto& link : m_links) {
            if (link["source"].toString() == id || link["target"].toString() == id) {
                linksToRemove.append(link);
            }
        }

        for (const auto& link : linksToRemove) {
            m_links.removeOne(link);
            emit connectionRemoved(link["source"].toString(), link["target"].toString());
        }

        emit nodeRemoved(id);
        // 仅发送节点移除事件，避免每次小更改都发送完整图数据
    }
}

void WebNodeManager::updateNodeText(const QString& id, const QString& newText)
{
    if (m_nodes.contains(id)) {
        m_nodes[id]["text"] = newText;
        emit nodeUpdated(m_nodes[id]);
        // 仅发送节点更新事件，避免每次小更改都发送完整图数据
    }
}

void WebNodeManager::addConnection(const QString& sourceId, const QString& targetId)
{
    if (!m_nodes.contains(sourceId) || !m_nodes.contains(targetId)) {
        qWarning() << "Cannot add connection between non-existing nodes";
        return;
    }

    // 检查是否已存在连接
    for (const auto& link : m_links) {
        if (link["source"].toString() == sourceId && link["target"].toString() == targetId) {
            return; // 连接已存在
        }
    }

    QJsonObject link;
    link["source"] = sourceId;
    link["target"] = targetId;

    m_links.append(link);
    emit connectionAdded(link);
    // 仅发送连接添加事件，避免每次小更改都发送完整图数据
}

void WebNodeManager::removeConnection(const QString& sourceId, const QString& targetId)
{
    for (int i = 0; i < m_links.size(); i++) {
        const auto& link = m_links[i];
        if (link["source"].toString() == sourceId && link["target"].toString() == targetId) {
            m_links.removeAt(i);
            emit connectionRemoved(sourceId, targetId);
            // 仅发送连接移除事件，避免每次小更改都发送完整图数据
            return;
        }
    }
}

QJsonObject WebNodeManager::getFullGraph() const
{
    QJsonObject graph;
    QJsonArray nodesArray;
    QJsonArray linksArray;

    // 添加所有节点
    for (const auto& node : m_nodes) {
        nodesArray.append(node);
    }

    // 添加所有链接
    for (const auto& link : m_links) {
        linksArray.append(link);
    }

    graph["nodes"] = nodesArray;
    graph["links"] = linksArray;
    return graph;
}

void WebNodeManager::appReady()
{
    qDebug() << "Frontend application is ready, sending initial graph data...";
    
    // 发送完整的图数据给前端
    emit graphUpdated(getFullGraph());
}