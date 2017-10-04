#ifdef DISPLAY_GRAPHS
#ifndef GVGRAPH_H_
#define GVGRAPH_H_

#include "common.h"
#include "gvc.h"
#include <QtCore>
#include <QtGui>

struct GVNode;
struct GVEdge;
// An object containing a libgraph graph and its associated nodes and edges
class GVGraph
{
public:
    /// Default DPI value used by dot (which uses points instead of pixels for coordinates)
    static const qreal DotDefaultDPI = 72;

    /*!
     * \brief Construct a Graphviz graph object
     * \param name The name of the graph, must be unique in the application
     * \param font The font to use for the graph
     * \param node_size The size in pixels of each node
     */
    void setAttribute(QFont font, qreal node_size);
    GVGraph(QString name, QFont font=QFont(), qreal node_size=50);
    GVGraph(FILE *fp, QFont font=QFont(), qreal node_size=50);
    ~GVGraph();

    /// Add and remove nodes
    void addNode(const QString& name);
    void addNodes(const QStringList& names);
    void removeNode(const QString& name);
    void clearNodes();

    /// Add and remove edges
    void addEdge(const QString& source, const QString& target, const QString name="");
    void removeEdge(const QString& source, const QString& target);

    /// Set the font to use in all the labels
    void setFont(QFont font);

    /// Retrieve nodes and edges
    QList<GVNode> getNodes() const;
    QList<GVEdge> getEdges() const;

    /// Finalization
    QRectF boundingRect() const;
    void applyLayout();

private:
    GVC_t *_context;
    Agraph_t *_graph;
    QFont _font;
    std::map<QString, Agnode_t*> _nodes;
    std::map<std::pair<QString, QString>, Agedge_t*> _edges;
};

struct GVNode
{
    /// The unique identifier of the node in the graph
    QString name;

    /// The position of the center point of the node from the top-left corner
    //QPoint centerPos;
    qreal x, y;
    qreal labelX, labelY;

    /// The size of the node in pixels
    qreal height, width;
};

/// A struct containing the information for a GVGraph's edge
struct GVEdge
{
    /// The source and target nodes of the edge
    QString source;
    QString target;

    /// Path of the edge's line
    QPainterPath path;
    qreal tailX;
    qreal tailY;
    qreal tailNodeCenterX;
    qreal tailNodeCenterY;
};

#endif //GVGRAPH_H
#endif //ifdef DISPLAY_GRAPHS
