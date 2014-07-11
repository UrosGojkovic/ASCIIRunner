#include "asciirunner.h"
#include"Generisi.h"
#include<QStringList>

using namespace ASCIIRunner;

Game::Game(QCoreApplication* parent)
{
  ofstream trace;
  trace.open("traceGameConstructor.txt");
  QStringList args(parent->arguments());
  _parent=parent;
  _timer=new QTimer();
  _timer->start(10);
  trace<<"timer started"<<endl;
  trace.flush();
  vector<ASCIIRenderEngine::ActiveObject*> activeObjects;
  _passInputTo=new Camera(9, 2);
  trace<<"menu generated"<<endl;
  trace.flush();
  activeObjects.push_back(_passInputTo);
  trace<<"menu is the vector"<<endl;
  trace.flush();
  ASCIIRenderEngine::InitParams ini;
  ini.activeObjectList=activeObjects;
  trace<<"active objects copied to ini"<<endl;
  trace.flush();
  if(args.length()>=2)
  {
    if(args[1]=="r")
    {
      ini.levelPath="";
      ini.level=newRandomLevel();
      trace<<"splash screen generated"<<endl;
      trace.flush();
      ini.spriteLayer=1;
      ini.prefViewportSize[0]=25;
      ini.prefViewportSize[1]=80;
      ini.playerRelPosition[0]=14;
      ini.playerRelPosition[1]=25;
      ini.numberOfColorsUsed=0;
      ini.numberOfPairsUsed=0;
      _renderer=new ASCIIRenderEngine::Renderer(ini);
      trace<<"renderer started"<<endl;
      trace.flush();
      connect(_timer, SIGNAL(timeout()), this, SLOT(catchTick()));
      trace<<"done"<<endl;
      trace.flush();
      trace.close();
    }
  }
  else
  {
    ini.levelPath="./static/";
    trace<<"splash screen generated"<<endl;
    trace.flush();
    ini.spriteLayer=3;
    ini.prefViewportSize[0]=25;
    ini.prefViewportSize[1]=80;
    ini.playerRelPosition[0]=14;
    ini.playerRelPosition[1]=25;
    ini.numberOfColorsUsed=0;
    ini.numberOfPairsUsed=0;
    _renderer=new ASCIIRenderEngine::Renderer(ini);
    trace<<"renderer started"<<endl;
    trace.flush();
    connect(_timer, SIGNAL(timeout()), this, SLOT(catchTick()));
    trace<<"done"<<endl;
    trace.flush();
    trace.close();
  }
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
  ofstream trace;
  trace.open("traceGameCatchTick");
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
    trace<<"input passed"<<endl;
    trace.flush();
  }
  _renderer->doTick();
  trace<<"doTick complete"<<endl;
  trace.flush();
  _renderer->doCompositing();
  trace<<"doComposit complete"<<endl;
  trace.flush();
  addCounter();
  trace<<"counter added"<<endl;
  trace.flush();
  _renderer->updateScreen();
  trace<<"screen updated"<<endl;
  trace.close();

}

vector<ASCIIRenderEngine::ActiveObject *> Game::generateObjects()
{
  vector<ASCIIRenderEngine::ActiveObject*> objects;
  Player* player=new Player();
  objects.push_back(player);
  Coin* coin=new Coin(10, 10);
  objects.push_back(coin);
  return objects;
}

ASCIIRenderEngine::Level Game::splashScreen()
{
  vector<ASCIIRenderEngine::LevelLayer> level;
  vector<vector<chtype> > layer;
  vector<chtype> row;
  ofstream trace;
  trace.open("traceSplashScreen.txt");
  trace<<"atempting background"<<endl;
  for(int i=0; i<25; i++)
  {
    row.clear();
    for(int j=0; j<80; j++)
    {
      if((j+i)%2==0)
      {
        row.push_back('*');
      }
      else
      {
        row.push_back(' ');
      }
    }
    layer.push_back(row);
  }
  trace<<"background generated, trying to put it in level"<<endl;
  level.push_back(ASCIIRenderEngine::LevelLayer(layer, 0, false, false));
  trace<<"creating layer success"<<endl;
  row.clear();
  layer.clear();
  string r[]={"           /| ___   ___  _____ _____                                            ",
              "          / |/   \\ /   \\   I     I                                              ",
              "         /  |\\___  |       I     I                                              ",
              "        /   |    \\ |       I     I                                              ",
              "       / /| |\\___/ \\___/ __I__ __I__                                            ",
              "      / / | |                                                                   ",
              "     / /__| |                                                                   ",
              "    / ____  |___                ___  ___                                        ",
              "   / /    | |   \\ |  | | | | | |    |   \\                                       ",
              "  / /     | |___/ |  | |\\| |\\| |--  |___/                                       ",
              " /_/      |_|   \\ |__| | | | | |___ |   \\                                       ",
              "                                                                                ",
              "                                                                                ",
              "                                                                                ",
              "                                                                                ",
              "                                                                                ",
              "                                                                                ",
              "                                                                                ",
              "                                                                                ",
              "                                                                                ",
              "                                                                                ",
              " by Uros Gojkovic & Djordje Jovic, MATF, 2014                                   ",
              "                                                                                ",
              "                                                                                ",
              "                                                                                "};
  trace<<"attempting logo"<<endl;
  for(int i=0; i<25; i++)
  {
    row.clear();
    for(int j=0; j<80; j++)
    {
      row.push_back(r[i][j]);
    }
    layer.push_back(row);
  }
  trace<<"generating logo success, trying to put it in level"<<endl;
  level.push_back(ASCIIRenderEngine::LevelLayer(layer, 1, false, false));
  trace<<"creating layer success, done"<<endl;
  trace.close();
  return level;
}

ASCIIRenderEngine::Level Game::newRandomLevel()
{
  chtype background[25][80]={
    {1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1056,1056,1056,1056,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1056,1056,1056,1056,1056,1056,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1056,1056,1056,1056,1056,1056,1056,1056,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1056,1056,1056,1056,1056,1056,1056,1056,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1056,1056,1056,1056,1056,1056,1056,1056,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1056,1056,1056,1056,1056,1056,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1056,1056,1056,1056,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824},
    {1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,2080,1824,1824,1824,1824,1824,1824}
  };
  vector<vector<chtype> > layer;
  vector<chtype> row;
  for(int i=0; i<25; i++)
  {
    row.clear();
    for(int j=0; j<80; j++)
    {
      row.push_back(background[i][j]);
    }
    layer.push_back(row);
  }
  vector<ASCIIRenderEngine::LevelLayer> level;
  level.push_back(ASCIIRenderEngine::LevelLayer(layer,0, 0, 0));
  layer.clear();
  level.push_back(Generisi_nivo());
  return ASCIIRenderEngine::Level(level);
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

Player::Player(int rPos, int cPos)
{
  _size[0]=3;
  _size[1]=3;
  vector<vector<chtype> > animationFrame;
  vector<chtype> row;
  //idle
  row.push_back(' ');
  row.push_back('O');
  row.push_back(' ');
  animationFrame.push_back(row);
  row.clear();
  row.push_back('/');
  row.push_back('|');
  row.push_back('\\');
  animationFrame.push_back(row);
  row.clear();
  row.push_back('/');
  row.push_back(' ');
  row.push_back('\\');
  animationFrame.push_back(row);
  _aniFrames.push_back(animationFrame);
  animationFrame.clear();
  //right
  row.push_back(' ');
  row.push_back('O');
  row.push_back(' ');
  animationFrame.push_back(row);
  row.clear();
  row.push_back('<');
  row.push_back('|');
  row.push_back('/');
  animationFrame.push_back(row);
  row.clear();
  row.push_back('/');
  row.push_back(' ');
  row.push_back('>');
  animationFrame.push_back(row);
  _aniFrames.push_back(animationFrame);
  animationFrame.clear();
  //left
  row.push_back(' ');
  row.push_back('O');
  row.push_back(' ');
  animationFrame.push_back(row);
  row.clear();
  row.push_back('\\');
  row.push_back('|');
  row.push_back('>');
  animationFrame.push_back(row);
  row.clear();
  row.push_back('<');
  row.push_back(' ');
  row.push_back('\\');
  animationFrame.push_back(row);
  _aniFrames.push_back(animationFrame);
  animationFrame.clear();
  //up
  row.push_back(' ');
  row.push_back('O');
  row.push_back(' ');
  animationFrame.push_back(row);
  row.clear();
  row.push_back('/');
  row.push_back('|');
  row.push_back('\\');
  animationFrame.push_back(row);
  row.clear();
  row.push_back(' ');
  row.push_back('>');
  row.push_back('>');
  animationFrame.push_back(row);
  _aniFrames.push_back(animationFrame);
  animationFrame.clear();
  for(unsigned int i=0; i<_aniFrames.size(); i++)
  {
    for(unsigned int j=0; j<_aniFrames[i].size(); j++)
    {
      for(unsigned int k=0; k<_aniFrames[i][j].size(); k++)
      {
        _aniFrames[i][j][k]=_aniFrames[i][j][k]|COLOR_PAIR(33);
      }
    }
  }
  _pos[0]=rPos;
  _pos[1]=cPos;
  _id=player;
  _coinCount=0;
}

void Player::tick()
{
  if(_jumping)
  {
    if(_jumpHeight<=3)
    {
      _pos[0]=_pos[0]-1;
      _jumpHeight++;
    }
    else
    {
      _jumping=false;
    }
    if(_key==KEY_RIGHT)
    {
      moveRight();
      _key=0;
    }
    else if(_key==KEY_LEFT)
    {
      moveLeft();
      _key=0;
    }
  }
  else
  {
    if(_key==KEY_RIGHT)
    {
      moveRight();
      _key=0;
    }
    else if(_key==KEY_LEFT)
    {
      moveLeft();
      _key=0;
    }
    else if(_key==KEY_UP)
    {
      jump();
      _key=0;
    }
    else
    {
      gravity();
    }
  }
}

vector<vector<chtype> > *Player::animate()
{
  return &this->_aniFrames[_selectFrame];
}

void Player::move(unsigned int rPos, unsigned int cPos)
{
  this->_pos[0]=rPos;
  this->_pos[1]=cPos;
  if(_oldCoords[0]>=_pos[0])
  {
    _jumping=false;
    _jumpHeight=0;
  }
}

void Player::collision(ASCIIRenderEngine::direction_enum direction, ASCIIRenderEngine::ActiveObject *other)
{
  if(other->id()==coin)
  {
    _coinCount++;
  }
}

void Player::jump()
{
  _jumping=true;
  _jumpHeight=0;
  _selectFrame=3;
}

void Player::moveRight()
{
  _pos[1]=_pos[1]+1;
  _selectFrame=1;
}

void Player::moveLeft()
{
  _pos[1]=_pos[1]-1;
  _selectFrame=2;
}

void Player::gravity()
{
  this->_pos[0]=this->_pos[0]+1;
  _oldCoords[0]=_pos[0];
  _selectFrame=0;
}


Coin::Coin(unsigned int rPos, unsigned int cPos)
{
  _pos[0]=rPos;
  _pos[1]=cPos;
  vector<vector<chtype> > frame;
  vector<chtype> row;
  row.push_back('0'|COLOR_PAIR(25));
  frame.push_back(row);
  _aniFrames.push_back(frame);
  _size[0]=1;
  _size[1]=1;
  _id=coin;
}

void Coin::collision(ASCIIRenderEngine::direction_enum direction, ASCIIRenderEngine::ActiveObject *other)
{
  if(other->id()==player)
  {
    this->die();
  }
}


Menu::Menu()
{
  vector<vector<chtype> > frame;
  vector<chtype> row;
  string f1[]={" X Start random level                                    ",
               "                                                         ",
               "   Start static level                                    ",
               "                                                         ",
               " Press F1 to quit, arrow keys to select, Enter to confirm"};

  string f2[]={"   Start random level                                    ",
               "                                                         ",
               " X Start static level                                    ",
               "                                                         ",
               " Press F1 to quit, arrow keys to select, Enter to confirm"};
  this->_size[0]=3;
  this->_size[1]=3;
  for(int i=0; i<5; i++)
  {
    row.clear();
    for(unsigned int j=0; j<f1[0].length(); j++)
    {
      row.push_back(f1[i][j]);
    }
    frame.push_back(row);
  }
  this->_aniFrames.push_back(frame);
  frame.clear();
  for(int i=0; i<5; i++)
  {
    row.clear();
    for(unsigned int j=0; j<f2[0].length(); j++)
    {
      row.push_back(f2[i][j]);
    }
    frame.push_back(row);
  }
  this->_aniFrames.push_back(frame);
  _selectedOption=0;
  this->_pos[0]=13;
  this->_pos[1]=5;
  _id=player;
}

void Menu::tick()
{
  if(_key==KEY_UP)
  {
    _selectedOption=0;
  }
  else if(_key==KEY_DOWN)
  {
    _selectedOption=1;
  }
}

vector<vector<chtype> >* Menu::animate()
{
  return &_aniFrames[_selectedOption];
}


Camera::Camera(unsigned int startRow, unsigned int startCol)
{
  ofstream trace;
  trace.open("traceCameraConstructor.txt");
  _pos[0]=startRow;
  _pos[1]=startCol;
  _size[0]=3;
  _size[1]=3;
  vector<chtype> row;
  row.assign(3, ' '|COLOR_PAIR(5));
  trace<<"content of row: "<<row[0]<<endl;
  trace.flush();
  vector<vector<chtype> > frame;
  frame.push_back(row);
  frame.push_back(row);
  frame.push_back(row);
  trace<<"row is in the frame"<<endl;
  trace.flush();
  _aniFrames.push_back(frame);
  trace<<"frame is in the aniFrames"<<endl;
  trace.flush();
  trace<<"aniFrames size: "<<_aniFrames.size()<<endl;
  trace.flush();
  trace<<"aniFrames[0] size: "<<_aniFrames[0].size()<<endl;
  trace.flush();
  trace<<"aniFrames[0][0] size: "<<_aniFrames[0][0].size()<<endl;
  trace.flush();
  trace<<"aniFrames[0][0][0] content: "<<_aniFrames[0][0][0]<<endl;
  trace.close();
  _id=player;
}

void Camera::tick()
{
  if(_key==KEY_LEFT)
  {
    _pos[1]--;
  }
  else if(_key==KEY_RIGHT)
  {
    _pos[1]++;
  }
  else if(_key==KEY_UP)
  {
    _pos[0]--;
  }
  else if(_key==KEY_DOWN)
  {
    _pos[0]++;
  }
}

vector<vector<chtype> > *Camera::animate()
{
  return &_aniFrames[0];
}

