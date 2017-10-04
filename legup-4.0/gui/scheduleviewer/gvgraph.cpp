#ifdef DISPLAY_GRAPHS
#include "gvgraph.h"

inline int _agset(void *obj, const QString &name, const QString &value)
{
    return agset(obj, (char*)name.toStdString().c_str(), (char*)value.toStdString().c_str());
}

inline Agsym_t *_agnodeattr(Agraph_t *graph, const QString &name, const QString &value)
{
    return agnodeattr(graph, (char*)name.toStdString().c_str(), (char*)value.toStdString().c_str());
}

inline Agsym_t *_agedgeattr(Agraph_t *graph, const QString &name, const QString &value)
{
    return agedgeattr(graph, (char*)name.toStdString().c_str(), (char*)value.toStdString().c_str());
}


Agnode_t *_agnode(Agraph_t *_graph, QString name){
    return agnode(_graph, (char *)(name.toStdString().c_str()));
}

void GVGraph::setAttribute(QFont font, qreal node_size)
{
    //Set graph attributes
    _agset(_graph, "overlap", "prism");
    _agset(_graph, "splines", "true");
    _agset(_graph, "pad", "0,2");
    _agset(_graph, "dpi", "96,0");
    _agset(_graph, "nodesep", "0,4");

    //Set default attributes for the future nodes
    _agnodeattr(_graph, "fixedsize", "false");
    _agnodeattr(_graph, "label", "");
    //_agnodeattr(_graph, "regular", "true");
    _agnodeattr(_graph, "shape", "ellipse");

    //Divide the wanted width by the DPI to get the value in points
    //QString nodePtsWidth("%1").arg(node_size/_agget(_graph, "dpi", "96,0").toDouble());
    //GV uses , instead of . for the separator in floats
    //_agattr(_graph, "width", nodePtsWidth.replace('.', ",").toStrString().c_str());

    setFont(font);
}

GVGraph::GVGraph(FILE *fp, QFont font, qreal node_size) :
	_context(gvContext()),
	_graph(agread(fp))
{
    setAttribute(font, node_size);
}

GVGraph::GVGraph(QString name, QFont font, qreal node_size) :
        _context(gvContext()),
        _graph(agopen((char*)name.toStdString().c_str(), AGDIGRAPHSTRICT)) // Strict directed graph, see libcgraph doc
{
    setAttribute(font, node_size);
}

GVGraph::~GVGraph()
{
    gvFreeLayout(_context, _graph);
    agclose(_graph);
    gvFreeContext(_context);
}

void GVGraph::addNode(const QString& name)
{
    if(_nodes.find(name)!=_nodes.end()) return;
       // removeNode(name);
    Agnode_t * node = _agnode(_graph, name);
    _agset(node, "label", name);
    _nodes.insert(std::pair<QString, Agnode_t *>(name, node));
}

void GVGraph::addNodes(const QStringList& names)
{
    for(int i=0; i<names.size(); ++i)
        addNode(names.at(i));
}

void GVGraph::addEdge(const QString &source, const QString &target, const QString name)
{
    if(_nodes.find(source)!=_nodes.end() && _nodes.find(target)!=_nodes.end())
    {
        std::pair<QString, QString> key(source, target);
        if(_edges.find(key)==_edges.end())
            _edges.insert(std::pair<std::pair<QString, QString>, Agedge_t *>(key, agedge(_graph, _nodes[source], _nodes[target])));
    }
}

void GVGraph::applyLayout()
{
    gvFreeLayout(_context, _graph);
    gvLayout(_context, _graph, (char*)"dot");
    //gvRenderFilename (_context, _graph, "ps", "experiment.ps");
}

QList<GVNode> GVGraph::getNodes() const
{
    QList<GVNode> list;
    //qreal dpi=_agget(_graph, "dpi", "96,0").toDouble();
    qreal dpi = 96;

    Agnode_t *v;
    //for(std::map<QString, Agnode_t*>::const_iterator it=_nodes.begin(); it!=_nodes.end();++it)
    for (v = agfstnode(_graph); v; v = agnxtnode(_graph,v))
    {
        //Agnode_t *node=it->second;
        Agnode_t *node=v;
        GVNode object;

        //Set the name of the node
        object.name=node->u.label->text;

        //Fetch the X coordinate, apply the DPI conversion rate (actual DPI / 72, used by dot)
        qreal centerX=node->u.coord.x*(dpi/DotDefaultDPI);
        qreal labelX =node->u.label->pos.x*(dpi/DotDefaultDPI);
        //Translate the Y coordinate from bottom-left to top-left corner
        qreal centerY=(_graph->u.bb.UR.y - node->u.coord.y)*(dpi/DotDefaultDPI);
        qreal labelY =(_graph->u.bb.UR.y - node->u.label->pos.y)*(dpi/DotDefaultDPI);
        //object.centerPos=QPoint(x, y);

        //Transform the width and height from inches to pixels
        object.height=node->u.height*dpi;
        object.width=node->u.width*dpi;
        object.x = centerX - object.width/2;
        object.y = centerY - object.height/2;
        
        //note, the x/y coordinate of the labels will not be calculated before a call to
        //the gvrender function
        object.labelX = labelX - node->u.label->dimen.x/2;
        object.labelY = labelY - node->u.label->dimen.y/2;
	//printf("height and width of label are %f, %f\n", node->u.label->dimen.x, node->u.label->dimen.y);
	//printf("center position ofof label are %f, %f\n", node->u.label->pos.x, node->u.label->pos.y);
        list << object;
    }

    return list;
}

QList<GVEdge> GVGraph::getEdges() const
{
    QList<GVEdge> list;
    //qreal dpi=_agget(_graph, "dpi", "96,0").toDouble();
    qreal dpi = 96;

    //for(std::map<std::pair<QString, QString>, Agedge_t* >::const_iterator it=_edges.begin();
       // it!=_edges.end();++it)
    Agnode_t *v;
    Agedge_t *e;
    for (v = agfstnode(_graph); v; v = agnxtnode(_graph,v))
    for (e = agfstout(_graph,v); e; e = agnxtout(_graph,e))
    {
        Agedge_t *edge=e;
        GVEdge object;

        //Fill the source and target node names
        object.source=edge->tail->name;
        object.target=edge->head->name;
        

        //Calculate the path from the spline (only one spline, as the graph is strict. If it
        //wasn't, we would have to iterate over the first list too)
        //Calculate the path from the spline (only one as the graph is strict)
        if((edge->u.spl->list!=0) && (edge->u.spl->list->size%3 == 1))
        {
            //If there is a starting point, draw a line from it to the first curve point
            if(edge->u.spl->list->sflag)
            {
                object.path.moveTo(edge->u.spl->list->sp.x*(dpi/DotDefaultDPI),
                             (_graph->u.bb.UR.y - edge->u.spl->list->sp.y)*(dpi/DotDefaultDPI));
                object.path.lineTo(edge->u.spl->list->list[0].x*(dpi/DotDefaultDPI),
                        (_graph->u.bb.UR.y - edge->u.spl->list->list[0].y)*(dpi/DotDefaultDPI));
            }
            else
                object.path.moveTo(edge->u.spl->list->list[0].x*(dpi/DotDefaultDPI),
                        (_graph->u.bb.UR.y - edge->u.spl->list->list[0].y)*(dpi/DotDefaultDPI));

            //Loop over the curve points
            for(int i=1; i<edge->u.spl->list->size; i+=3)
                object.path.cubicTo(edge->u.spl->list->list[i].x*(dpi/DotDefaultDPI), 
                      (_graph->u.bb.UR.y - edge->u.spl->list->list[i].y)*(dpi/DotDefaultDPI),
                      edge->u.spl->list->list[i+1].x*(dpi/DotDefaultDPI),
                      (_graph->u.bb.UR.y - edge->u.spl->list->list[i+1].y)*(dpi/DotDefaultDPI),
                      edge->u.spl->list->list[i+2].x*(dpi/DotDefaultDPI),
                      (_graph->u.bb.UR.y - edge->u.spl->list->list[i+2].y)*(dpi/DotDefaultDPI));

            //If there is an ending point, draw a line to it
            if(edge->u.spl->list->eflag)
                object.path.lineTo(edge->u.spl->list->ep.x*(dpi/DotDefaultDPI),
                             (_graph->u.bb.UR.y - edge->u.spl->list->ep.y)*(dpi/DotDefaultDPI));
        }
        object.tailX = edge->u.spl->list->ep.x*(dpi/DotDefaultDPI);
        object.tailY = (_graph->u.bb.UR.y - edge->u.spl->list->ep.y)*(dpi/DotDefaultDPI);
        
        Agnode_t * tail = edge->head; //the destination is actually the head in graphviz
        object.tailNodeCenterX = tail->u.coord.x*(dpi/DotDefaultDPI);
        object.tailNodeCenterY = (_graph->u.bb.UR.y - tail->u.coord.y)*(dpi/DotDefaultDPI);

        list << object;
    }
    
    
    return list;
}

QRectF GVGraph::boundingRect() const
{
    //qreal dpi=_agget(_graph, "dpi", "96,0").toDouble();
    qreal dpi = 96;
    return QRectF(_graph->u.bb.LL.x*(dpi/DotDefaultDPI), _graph->u.bb.LL.y*(dpi/DotDefaultDPI),
                  _graph->u.bb.UR.x*(dpi/DotDefaultDPI), _graph->u.bb.UR.y*(dpi/DotDefaultDPI));
}

void GVGraph::setFont(QFont font)
{
    _font=font;

    _agset(_graph, "fontname", font.family());
    _agset(_graph, "fontsize", QString("%1").arg(font.pointSizeF()));

    _agnodeattr(_graph, "fontname", font.family());
    _agnodeattr(_graph, "fontsize", QString("%1").arg(font.pointSizeF()));

    _agedgeattr(_graph, "fontname", font.family());
    _agedgeattr(_graph, "fontsize", QString("%1").arg(font.pointSizeF()));
}

#endif //ifdef DISPLAY_GRAPHS
