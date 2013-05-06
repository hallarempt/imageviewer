#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <QRectF>
class Node : public QObject
{
    Q_OBJECT
public:
    explicit Node(QObject *parent = 0);

signals:
    
    void dirty(const QRectF &rc);

public slots:
    
};

#endif // NODE_H
