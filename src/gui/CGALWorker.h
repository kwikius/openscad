#pragma once

#include <memory>

#include <QObject>

class Geometry;
class Tree;
class QThread;

class CGALWorker : public QObject
{
  Q_OBJECT;
public:
  CGALWorker();
  ~CGALWorker();

public slots:
  void start(const Tree& tree);

protected slots:
  void work();

signals:
  void done(std::shared_ptr<const Geometry>);

protected:

  QThread *thread;
  const Tree *tree;
};
