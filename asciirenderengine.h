#ifndef ASCIIRENDERENGINE_H
#define ASCIIRENDERENGINE_H
#include<vector>
#include<ncurses.h>
#include<iostream>
#include<fstream>
#include<string>
#include<cmath>

using namespace std;

namespace ASCIIRenderEngine
{
  enum direction_enum {None=-1, L, LU, U, UR, R, RD, D, DL};
                              //0, 1,  2, 3,  4, 5,  6, 7

  typedef struct _position
  {
    int x;
    int y;
  } Pos;

  typedef Pos Size;

  class ActiveObject
  {
  public:
    virtual void move(int rPos, int cPos); //updates the state of position variables, used by collision detection method
    virtual vector<vector<chtype> >* animate()=0; //returns the "sprite" for the next frame
    virtual void tick()=0; //a signal that the object should update its state
    int rowPos(); //returns the row (y) position of the object
    int colPos(); // return the column (x) position of the object
    //both coordinates are the top left corner of the object relative to top left corner of the level

    unsigned int height(); //returns height of the object in rows
    unsigned int width(); //returns width of the object in cols
    virtual void collision(direction_enum direction, ActiveObject* other)=0; //direction: Left, Left and Up, Up, Up and Right, Right, ..., -1 marks a special case where object didn't move, but there was collision, for example due to size changing or some other object hitting it; see .cpp file for usage
    void disable(); //disables the object (not rendered, no collision, but remains in memory)
    void enable(); //enables the object
    bool isEnabled(); //true if enabled
    void changeSprites(vector<vector<vector<chtype> > > frames); //changes the sprite array of an object on the fly, WORK IN PROGRESS
    int id(); //returns an integer ID of the object, used when resolving collision between AOs
    bool isGhost(); //true if noclip is true
    virtual ~ActiveObject() {}
    ActiveObject() {}

  protected:
    vector<vector<vector<chtype> > > _aniFrames; //each frame is a 2D vector of chtype, placed into a vector (much 3D. very advance. wow.); should be a pointer
    Pos _pos; //rows are y and columns are x
    Size _size; //rows are y and columns are x, x and y are signed, but the values should never be negative!
    bool _enabled=true;
    int _id;
    bool _noclip=false;
  };

  class LevelLayer
  {
  public:
    LevelLayer();
    LevelLayer(vector<vector<chtype> > layer, double rSpeed, bool tileHorizontally, bool tileVertically); //create a layer with specified paramaters
    unsigned int rows(); //returns number of rows
    unsigned int cols(); //returns number of columns
    double relativeSpeed(); //returns scrolling speed relative to the sprite (main) layer which has a speed of 1
    bool doTileHor(); //should layer wrap-around horizontaly
    bool doTileVer(); //should layer wrap-around verticaly
    vector<chtype> getRow(int row); //returns a single row from the layer

  private:
    vector<vector<chtype> > _layer; //actual chtype matrix
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
    Level(const string uncookedLevelPath, const string levelColorPath); //loads a level from files: first is the layout, second are color pairs, currently the parser is very brittle
    Level(const string cookedLevelPath); //loads a level file containing chtype array written as numbers, WORK IN PROGRESS
    Level(vector<LevelLayer> layers); //generates a level from an array of layers, meant for proceduraly generated levels
    Level(const Level& level); //copy constructor (?)
    LevelLayer* getLayer(unsigned int layer); //returns a specified layer, used by renderer
    unsigned int layerCount();

  private:
    vector<LevelLayer> _layers;
    unsigned int _numLayers; //stores number of layers when loading from a file
  };

  typedef struct _ini
  {
    vector<ActiveObject*> activeObjectList; //"camera" is always first in the list
    int mainLayer; //the layer where all/most action takes place, renamed from sprite layer in preparation for allowing AO to be placed on any layer; count starts from 0
    int levelType; //0 - characters are separate from color, 1 - each character is a chtype written as an integer in text, probably should be an enum
    string levelPath=""; //path to folder which contains the level files
    Level level; //optional, used for generated levels
    Size prefViewportSize; //used when testing the terminal for the right conditions
    Pos cameraRelPosition; //prefered position of the 0th element of activeObjectList relative to the viewport
    int colors[256][3]; //prefered colors; 1st coord: color number, 2nd coord: color channel: 0-red, 1-green, 2-blue, maybe should turn it into a structure, 256 colors are max that terminals can provide (xterm-256color)
    int colorPairs[256][2]; //custom color pairs; 1st coord: color pair, 2nd coord: 0-foreground color, 1-background color, 256 color pairs are max that chtype allows, maybe should turn it into a structure
    int numberOfPairsUsed=0;
    int numberOfColorsUsed=0;
  } InitParams;

  class Renderer
  {
  public:
    Renderer();
    Renderer(InitParams ini);
    void doCompositing(); //draws the level and AOs on the sprite level (for now)
    void doTick(); //calls tick() of every enabled element
    void updateScreen(); //draws the completed frame onto screen
    void updateRelCameraPos(unsigned int rPos, unsigned int cPos); //changes the location of camera relative to viewport (or vice-versa, if you prefer it that way)
    void loadNewLevel(string path); //loads the new level without recreating the renderer, WORK IN PROGRESS
    void loadNewLevel(Level level); //same
    void loadNewObjects(vector<ActiveObject*> newList); //loads a new set of objects to replace the existing without recreating the renderer, WORK IN PROGRESS
    void loadSprites(const string uncookedSpritesPath, const string spriteColorPath); //have no idea
    void loadSprites(const string cookedSpritesPath); //also have no idea
    ~Renderer();
    vector<vector<chtype> > overlayLayer; //allows you to print items above everything else and in fixed position relative to the viewport, currently used for debugging, but intended for scores and such, needs more features

  private:
    void drawObjects(); //draws AOs
    void checkCollision(unsigned int AOindex); //calls the next two methods
    void checkAObjectsCollision(unsigned int AOindex); //checks if there were collisions between AOs, WORK IN PROGRESS
    void checkLevelCollision(unsigned int AOindex); //cheks if AO tried to pass through a solid piece of a level
    int findColorPair(int fg, int bg); //used to determine what color pair has provided parameters
    chtype doTransparency(chtype bottom, chtype top); //changes color of objects with transparent properties, faulty implementation ATM
    void calculateViewportPos(); //calculate viewport position relative to the level, based on camera position
    vector<ActiveObject*> _activeItems;
    Level _currentLevel;
    Pos _viewportPos; //position of viewport relative to the main layer
    unsigned int _mainLayer;
    Pos _prefRelViewportPos; //remembers the prefered position of the viewport relative to the 0th AO, in cases where we hit the border of the level
    Size _viewportSize; //the size of a viewport
    Pos _oldObjectPos; //helper variable when determining collision
  };
}
#endif // ASCIIRENDERENGINE_H
