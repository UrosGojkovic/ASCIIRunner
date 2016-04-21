#ifndef ASCIIRUNNER_H
#define ASCIIRUNNER_H
#include<QTimer>
#include<QObject>
#include<QCoreApplication>
#include"asciirenderengine.h"

namespace ASCIIRunner
{
  enum objectTypes_enum {player};

  class HumanControllable:public ASCIIRenderEngine::ActiveObject
  {
  public:
    virtual void collectInput(int key) {_key=key;} //stores the key that was input, ATM uses ncurses interface which is buffered and unsuitable for real time control
    virtual ~HumanControllable() {}

  protected:
    int _key;
  };

  class Game:public QObject //central class
  {
    Q_OBJECT
  public:
    Game();
    Game(QCoreApplication* parent=0);
    void quit();
    ~Game();

  public slots:
    void catchTick();

  private:
    vector<ASCIIRenderEngine::ActiveObject*> generateObjectList();
    ASCIIRenderEngine::Level testScroll(); //test paralax scrolling, used in early testing, can still be used is so desired
    void addCounter(); //add debug counters to the screen, uses overlay
    QTimer* _timer; //timer for ticks
    QCoreApplication* _parent;
    ASCIIRenderEngine::Renderer* _renderer; //instance of an engine
    //bool _doRender=false;
    int _counter=0;
    HumanControllable* _passInputTo; //which object is receiving input
  };


  class Camera:public HumanControllable
  {
  public:
    Camera(){}
    Camera(unsigned int startRow, unsigned int startCol);
    void tick();
    vector<vector<chtype> >* animate();
    void collision(ASCIIRenderEngine::direction_enum direction, ActiveObject *other) {}
  };
}
#endif // ASCIIRUNNER_H
