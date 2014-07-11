#ifndef ASCIIRUNNER_H
#define ASCIIRUNNER_H
#include<QTimer>
#include<QObject>
#include<QCoreApplication>
#include"asciirenderengine.h"

namespace ASCIIRunner
{
  enum objectTypes_enum {player, coin, spikes};

  class HumanControllable:public ASCIIRenderEngine::ActiveObject
  {
  public:
    virtual void collectInput(int key) {_key=key;}
    virtual ~HumanControllable() {}

  protected:
    int _key;
  };

  class Game:public QObject
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
    vector<ASCIIRenderEngine::ActiveObject*> generateObjects();
    ASCIIRenderEngine::Level splashScreen();
    ASCIIRenderEngine::Level newRandomLevel();
    ASCIIRenderEngine::Level testScroll();
    void addCounter();
    QTimer* _timer;
    QCoreApplication* _parent;
    ASCIIRenderEngine::Renderer* _renderer;
    //bool _doRender=false;
    int _counter=0;
    HumanControllable* _passInputTo;
  };

  class Player:public HumanControllable
  {
  public:
    Player(){}
    Player(int rPos, int cPos);
    void tick();
    vector<vector<chtype> >* animate();
    void move(unsigned int rPos, unsigned int cPos);
    void collision(ASCIIRenderEngine::direction_enum direction, ActiveObject *other);

  private:
    void jump();
    void moveRight();
    void moveLeft();
    void gravity();
    int _oldCoords[2];
    unsigned int _coinCount;
    unsigned int _selectFrame;
    bool _jumping=false;
    int _jumpHeight=0;
    bool _doGravity=true;
  };

  class Coin:public ASCIIRenderEngine::ActiveObject
  {
  public:
    Coin() {}
    Coin(unsigned int rPos, unsigned int cPos);
    void tick(){}
    vector<vector<chtype> >* animate() {return &this->_aniFrames[0];}
    //void move(unsigned int rPos, unsigned int cPos){_pos[0]=rPos; _pos[1]=cPos;}
    void collision(ASCIIRenderEngine::direction_enum direction, ActiveObject *other);
  };

  class Menu:public HumanControllable
  {
  public:
    Menu();
    void tick();
    vector<vector<chtype> >* animate();
    //void move(unsigned int rPos, unsigned int cPos) {_pos[0]=rPos; _pos[1]=cPos;}
    void collision(ASCIIRenderEngine::direction_enum direction, ActiveObject *other) {}

  private:
    int _selectedOption;
    void executeOption() { beep();}
  };

  class Camera:public HumanControllable
  {
  public:
    Camera(){}
    Camera(unsigned int startRow, unsigned int startCol);
    void tick();
    vector<vector<chtype> >* animate();
    //void move(unsigned int rPos, unsigned int cPos) {_pos[0]=rPos; _pos[1]=cPos;}
    void collision(ASCIIRenderEngine::direction_enum direction, ActiveObject *other) {}
  };
}
#endif // ASCIIRUNNER_H
