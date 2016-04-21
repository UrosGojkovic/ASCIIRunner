#include "asciirunner.h"
#include<QStringList>

using namespace ASCIIRunner;

Game::Game(QCoreApplication* parent) //entry point of the program
{
  //QStringList args(parent->arguments()); //not used at the moment
  _parent=parent;
  _timer=new QTimer();
  _timer->start(10); //set timer to 10ms (100 FPS)
  vector<ASCIIRenderEngine::ActiveObject*> activeObjects;
  _passInputTo=new Camera(9, 2); //define Camera as user-controled entity
  activeObjects.push_back(_passInputTo);
  ASCIIRenderEngine::InitParams ini;
  ini.activeObjectList=activeObjects;
  ini.levelPath="./static/"; //path to level files; leave this string empty and uncomment the next line to use scroll test level
  //ini.level=testScroll(); //ini.level is used only if levelPath is an empty string
  ini.mainLayer=3;
  ini.prefViewportSize.y=25;
  ini.prefViewportSize.x=80;
  ini.cameraRelPosition.y=14;
  ini.cameraRelPosition.x=25;
  ini.numberOfColorsUsed=0;
  ini.numberOfPairsUsed=0;
  _renderer=new ASCIIRenderEngine::Renderer(ini);
  connect(_timer, SIGNAL(timeout()), this, SLOT(catchTick()));
}

void Game::quit()
{
  _parent->quit();
}

Game::~Game()
{
  delete _timer;
  delete _renderer;
}

void Game::catchTick()
{
  _counter++;
  if(_counter>=100)
    _counter=0;
  int key=getch();
  flushinp();
  if(key==KEY_F(1))
  {
    quit();
  }
  else
  {
    _passInputTo->collectInput(key);
  }
  _renderer->doTick();
  _renderer->doCompositing();
  addCounter();
  _renderer->updateScreen();
}

vector<ASCIIRenderEngine::ActiveObject*> Game::generateObjectList()
{

}

ASCIIRenderEngine::Level Game::testScroll()
{
  vector<ASCIIRenderEngine::LevelLayer> level;
  vector<vector<chtype> > layer;
  vector<chtype> row;
  row.push_back('L'|COLOR_PAIR(34));
  row.push_back('a'|COLOR_PAIR(34));
  row.push_back('y'|COLOR_PAIR(34));
  row.push_back('e'|COLOR_PAIR(34));
  row.push_back('r'|COLOR_PAIR(34));
  row.push_back(' '|COLOR_PAIR(34));
  row.push_back('0'|COLOR_PAIR(34));
  layer.push_back(row);
  row.clear();
  for(int i=0; i<6; i++)
  {
    row.assign(7,' '|COLOR_PAIR(2));
    layer.push_back(row);
    row.clear();
  }
  level.push_back(ASCIIRenderEngine::LevelLayer(layer, 0.25, true, true));
  layer.clear();
  row.clear();
  row.push_back('L'|COLOR_PAIR(27));
  row.push_back('a'|COLOR_PAIR(27));
  row.push_back('y'|COLOR_PAIR(27));
  row.push_back('e'|COLOR_PAIR(27));
  row.push_back('r'|COLOR_PAIR(27));
  row.push_back(' '|COLOR_PAIR(27));
  row.push_back('1'|COLOR_PAIR(27));
  layer.push_back(row);
  row.clear();
  for(int i=0; i<6; i++)
  {
    row.assign(7,' '|COLOR_PAIR(3));
    layer.push_back(row);
    row.clear();
  }
  level.push_back(ASCIIRenderEngine::LevelLayer(layer, 0.5, true, true));
  layer.clear();
  row.clear();
  for(int i=0; i<75; i++)
  {
    row.assign(240, ' '|COLOR_PAIR(1));
    layer.push_back(row);
    row.clear();
  }
  level.push_back(ASCIIRenderEngine::LevelLayer(layer, 1, false, false));
  layer.clear();
  row.clear();
  row.push_back('L'|COLOR_PAIR(49));
  row.push_back('a'|COLOR_PAIR(49));
  row.push_back('y'|COLOR_PAIR(49));
  row.push_back('e'|COLOR_PAIR(49));
  row.push_back('r'|COLOR_PAIR(49));
  row.push_back(' '|COLOR_PAIR(49));
  row.push_back('3'|COLOR_PAIR(49));
  layer.push_back(row);
  row.clear();
  for(int i=0; i<6; i++)
  {
    row.assign(7,' '|COLOR_PAIR(1));
    layer.push_back(row);
    row.clear();
  }
  level.push_back(ASCIIRenderEngine::LevelLayer(layer, 2, true, true));
  layer.clear();
  row.clear();
  return level;
}

void Game::addCounter()
{
  int value=_counter%100;
  string text=to_string(value);
  _renderer->overlayLayer[0][0]=text[0];
  _renderer->overlayLayer[0][1]=text[1];
  int rowPos=_passInputTo->rowPos();
  int colPos=_passInputTo->colPos();
  string sRowPos=to_string(rowPos);
  string sColPos=to_string(colPos);
  for(int i=0; i<sRowPos.length(); i++)
    _renderer->overlayLayer[1][i]=sRowPos[i];
  for(int i=0; i<sColPos.length(); i++)
    _renderer->overlayLayer[2][i]=sColPos[i];
}

Camera::Camera(unsigned int startRow, unsigned int startCol)
{

  _pos.y=startRow;
  _pos.x=startCol;
  _size.y=3;
  _size.x=3;
  vector<chtype> row;
  row.assign(3, ' '|COLOR_PAIR(5));
  vector<vector<chtype> > frame;
  frame.push_back(row);
  frame.push_back(row);
  frame.push_back(row);
  _aniFrames.push_back(frame);
  _id=player;
}

void Camera::tick()
{
  if(_key==KEY_LEFT)
  {
    _pos.x--;
  }
  else if(_key==KEY_RIGHT)
  {
    _pos.x++;
  }
  else if(_key==KEY_UP)
  {
    _pos.y--;
  }
  else if(_key==KEY_DOWN)
  {
    _pos.y++;
  }
}

vector<vector<chtype> >* Camera::animate()
{
  return &_aniFrames[0];
}

