#pragma once

#include <QObject>

class CGALWorker : public QObject
{
  Q_OBJECT;
public:
  CGALWorker();
  ~CGALWorker();

public slots:
  void start(const class Tree& tree);

protected slots:
  void work();

signals:
  void done(shared_ptr<const class Geometry>);

protected:

  class QThread *thread;
  const class Tree *tree;
};
