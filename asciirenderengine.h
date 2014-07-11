#ifndef ASCIIRENDERENGINE_H
#define ASCIIRENDERENGINE_H
#include<vector>
#include<ncurses.h>
#include<iostream>
#include<fstream>
#include<string>
#include<cmath>
#include<fstream>

using namespace std;

namespace ASCIIRenderEngine
{
  enum direction_enum {None=-1, L, LU, U, UR, R, RD, D, DL};
                              //0, 1,  2, 3,  4, 5,  6, 7
  class ActiveObject
  {
  public:
    virtual void move(int rPos, int cPos); //azurira stanje internih promenjljivih
    virtual vector<vector<chtype> >* animate()=0; //vraca frejm rendereru koji treba da se prikaze
    virtual void tick()=0; //hvata update tick od internog casovnika
    int rowPos();
    int colPos();
    unsigned int height();
    unsigned int width();
    virtual void collision(direction_enum direction, ActiveObject* other)=0; //direction: 0-left, 1-left and up, 2-up, 3-up and right, 4-right, ...
    //-1 marks a special case where object didn't move, but there was collision due to size changing or some other cause
    void die();
    bool isAlive();
    void changeSprites(vector<vector<vector<chtype> > > frames);
    int id() {return _id;}
    virtual ~ActiveObject() {}
    ActiveObject() {}

  protected:
    vector<vector<vector<chtype> > > _aniFrames; //svaki frejm je 2D niz karaktera, a svi frejmovi cine jedan niz
    int _pos[2]; //rows first; gde se objekat nalazi na nivou (lokacija gornjeg levog ugla)
    unsigned int _size[2]; //rows first; velicina "bounding box-a"
    bool _alive=true;
    int _id;
  };

  class LevelLayer
  {
  public:
    LevelLayer();
    LevelLayer(vector<vector<chtype> > layer, double rSpeed, bool tileHorizontally, bool tileVertically);
    unsigned int rows();
    unsigned int cols();
    double relativeSpeed();
    bool doTileHor();
    bool doTileVer();
    vector<chtype> getRow(int row);

  private:
    vector<vector<chtype> > _layer;
    unsigned int _cols;
    unsigned int _rows;
    double _relativeSpeed;
    bool _tileHor;
    bool _tileVer;
  };

  class Level
  {
  public:
    Level();
    Level(const string uncookedlevelPath, const string levelColorPath); //ucitava direktno iz fajla, tip nivoa: cooked ili cist tekst
    Level(const string cookedLevelPath);
    Level(vector<LevelLayer> layers);
    Level(const Level& level);
    LevelLayer* getLayer(unsigned int layer);//koristi je renderer
    unsigned int layerCount();

  private:
    vector<LevelLayer> _layers;
    unsigned int _numLayers;
  };

  typedef struct _ini
  {
    vector<ActiveObject*> activeObjectList; //player is always first in the list
    int spriteLayer;
    int levelType;
    string levelPath="";
    Level level; //optional, koristi se za nasumicno generisane nivoe koji se ne ucitavaju iz fajla
    int prefViewportSize[2]; //rows first
    int playerRelPosition[2]; //rows first
    int colors[16][3]; //prefered colors; 1st coord: color number, 2nd coord: color channel: 0-red, 1-green, 2-blue
    int colorPairs[256][2]; //custom color pairs
    int numberOfPairsUsed=0;
    int numberOfColorsUsed=0;
  } InitParams;

  class Renderer
  {
  public:
    Renderer();
    Renderer(InitParams ini);
    void doCompositing(); //iscratava nivoe i aktivne elemente na sprajt nivou
    void doTick(); //pozive tick() svakog zivog elementa
    void updateScreen(); //zapravo iscrtava frejm na ekran
    void updateRelPlayerPos(unsigned int rPos, unsigned int cPos); //menja lokaciju igraca u odnosu na viewport
    void loadNewLevel(string path); //ucitava novi level bez restartovanja Renderera
    void loadNewLevel(Level level);
    void loadNewObjects(vector<ActiveObject*> newList); //ucitava nove objekte bez restartovanja Renderera
    void loadSprites(const string uncookedSpritesPath, const string spriteColorPath);
    void loadSprites(const string cookedSpritesPath);
    ~Renderer();
    vector<vector<chtype> > overlayLayer;

  private:
    void drawObjects();
    void checkCollision(unsigned int AOindex); //proverava ispravnost novih pozicija elementa
    void checkAObjectsCollision(unsigned int AOindex);
    void checkLevelCollision(unsigned int AOindex);
    int findColorPair(int fg, int bg);
    chtype doTransparency(chtype bottom, chtype top);
    void calculateViewportPos();
    vector<ActiveObject*> _activeItems;
    Level _currentLevel;
    int _viewportPos[2]; //rows first; relativno u odnosu na sprajt sloj
    unsigned int _spriteLayer;
    int _prefRelPlayerPos[2]; //rows first; koristi se za odredjivanje gde se nalazi igrac na ekranu relativno u odnosu na viewport
    unsigned int _viewportSize[2]; //rows first; velicina ekrana
    int _oldObjectPos[2]; //rows first; pozicija objekta pre tick() poziva
  };
}
#endif // ASCIIRENDERENGINE_H
