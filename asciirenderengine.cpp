#include "asciirenderengine.h"

using namespace ASCIIRenderEngine;
using namespace std;

Renderer::Renderer()
{
  //default constructor, not meant to be used
  initscr();
  if(start_color()==ERR)
    printw("Can't start colors.\n");
  raw();
  _viewportPos.y=0;
  _viewportPos.x=0;
  _viewportSize.y=25;
  _viewportSize.x=80;
  _mainLayer=0;
}

Renderer::Renderer(InitParams ini)
{
  initscr();
  if(start_color()==ERR)
    printw("Can't start colors.\n");
  noecho();
  curs_set(0);
  leaveok(stdscr, TRUE);
  raw();
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
  if(can_change_color())
  {
    if(ini.numberOfColorsUsed!=0) //initialize colors if possible
    {
      for(int i=0; i<ini.numberOfColorsUsed; i++)
      {
        init_color(i, ini.colors[i][0], ini.colors[i][1], ini.colors[i][2]);
      }
    }
  }
  else
  {
    printw("Can't change colors, using default.\n");
  }

  if(ini.numberOfPairsUsed!=0)
  {
    for(int i=0; i<ini.numberOfPairsUsed; i++) //prepisujemo prvih nekoliko parova sa user-defined vrednostima
    {
      init_pair(i, ini.colorPairs[i][0], ini.colorPairs[i][1]);
    }
  }
  else //define default 64 pairs, except 0th, since we can't do it through init_pair
  {
    int pair=1;
    for(int i=0; i<8; i++)
    {
      for(int j=0; j<8; j++)
      {
        init_pair(pair, i, j);
        pair++;
      }
    }
  }
  _viewportSize.y=ini.prefViewportSize.y;
  _viewportSize.x=ini.prefViewportSize.y;
  _activeItems=ini.activeObjectList;
  _mainLayer=ini.mainLayer;
  _prefRelViewportPos.y=ini.cameraRelPosition.y;
  _prefRelViewportPos.x=ini.cameraRelPosition.x;

  if(ini.levelPath=="")
  {
    _currentLevel=ini.level;
  }
  else
  {
    ifstream levelManifest;
    string fullPath=ini.levelPath+"level.manifest";
    levelManifest.open(fullPath);
    string tag;
    string colorPath="";
    string levelPath="";
    string levelColorPath="";
    string aoPath="";
    string aoColorPath="";
    int type;
    while(levelManifest.good())
    {
      levelManifest>>tag;
      if(tag=="t")
      {
        levelManifest>>type;
      }
      else if(tag=="c")
      {
        levelManifest>>colorPath;
      }
      else if(tag=="l")
      {
        levelManifest>>levelPath;
      }
      else if(tag=="lc")
      {
        levelManifest>>levelColorPath;
      }
      else if(tag=="ao")
      {
        levelManifest>>aoPath;
      }
      else if(tag=="aoc")
      {
        levelManifest>>aoColorPath;
      }
    }
    levelManifest.close();

    if(colorPath!="")
    {

      ifstream colors;
      colorPath=ini.levelPath+colorPath;
      colors.open(colorPath);
      string param;
      while(colors.good())
      {
        colors>>param;
        if(param=="c")
        {
          int numOfColors;
          colors>>numOfColors;
          for(int i=1; i<numOfColors; i++)
          {
            int red, green, blue;
            colors>>red;
            colors>>green;
            colors>>blue;
            init_color(i, red, green, blue);
          }
        }
        else if(param=="p")
        {
          int numOfPairs;
          colors>>numOfPairs;
          for(int i=1; i<numOfPairs; i++)
          {
            int fg, bg;
            colors>>fg;
            colors>>bg;
            init_pair(i, fg, bg);
          }
        }
        colors.close();
      }

    }
    if(levelPath!="")
    {
      if(type==0)
      {
        _currentLevel=Level(ini.levelPath+levelPath, ini.levelPath+levelColorPath);
      }
      else if(type==1)
      {
        _currentLevel=Level(ini.levelPath+levelPath);
      }
    }
    if(aoPath!="")
    {
      if(type==0)
      {
        loadSprites(ini.levelPath+aoPath, ini.levelPath+aoColorPath);
      }
      else if(type==1)
      {
        loadSprites(ini.levelPath+aoPath);
      }
    }
    trace.close();
  }
  if(getmaxx(stdscr)!=ini.prefViewportSize.x && getmaxy(stdscr)!=ini.prefViewportSize.y)
  {
    printw("This game is meant to be played in a %d x %d characters window, but the game can't resize it automatically.\nPlease resize the window, or hit Enter to quit.", ini.prefViewportSize.x, ini.prefViewportSize.y);
    while(getmaxx(stdscr)!=ini.prefViewportSize.x && getmaxy(stdscr)!=ini.prefViewportSize.y)
    {
      if(getch()==KEY_ENTER)
      {
        endwin();
        exit(EXIT_FAILURE);
      }
    }
  }
  vector<chtype> row;
  for(unsigned int i=0; i<_viewportSize.y; i++)
  {
    row.clear();
    for(unsigned int j=0; j<_viewportSize.x; j++)
    {
      row.push_back(' '|COLOR_PAIR(1));
    }
    overlayLayer.push_back(row);
  }
  calculateViewportPos();
}

void Renderer::doCompositing()
{
  calculateViewportPos();
  string vpRowPos=to_string(_viewportPos.y);
  string vpColPos=to_string(_viewportPos.x);
  for(int i=0; i<vpRowPos.length(); i++)
    overlayLayer[3][i]=vpRowPos[i];
  for(int i=0; i<vpColPos.length(); i++)
    overlayLayer[4][i]=vpColPos[i];
  vector<chtype> row;
  LevelLayer* layer;
  unsigned int rL;
  unsigned int cL;
  for(unsigned int l=0; l<_currentLevel.layerCount(); l++)
  {
    layer=_currentLevel.getLayer(l);
    for(unsigned int rS=0; rS<_viewportSize.y; rS++)
    {
      rL=(int)(_viewportPos.y*layer->relativeSpeed());
      rL+=rS;
      if(rL>=layer->rows())
      {
        if(layer->doTileHor())
        {
          rL=rL%layer->rows();
          row=layer->getRow(rL);
        }
        else
        {
          row.clear();
          row.push_back(' '|COLOR_PAIR(1));
        }
      }
      else
      {
        row=layer->getRow(rL);
      }
      for(unsigned int cS=0; cS<_viewportSize[1]; cS++)
      {
        cL=(int)(_viewportPos.x*layer->relativeSpeed());
        cL+=cS;
        if(cL>=layer->cols())
        {
          if(layer->doTileVer())
          {
            cL=cL%layer->cols();
          }
          else
          {
            row.push_back(' '|COLOR_PAIR(1));
          }
        }
        if(l==0)
        {
          if(row.size()!=1)
          {
            mvaddch(rS, cS, row[cL]);
          }
          else
          {
            mvaddch(rS, cS, row[0]);
          }
        }
        else
        {
          if(row.size()!=1)
          {
            mvaddch(rS, cS, doTransparency(mvinch(rS, cS), row[cL]));
          }
          else
          {
            mvaddch(rS, cS, doTransparency(mvinch(rS, cS), row[0]));
          }
        }
      }
    }
    if(l==_mainLayer)
    {
      drawObjects();
    }
  }
  for(unsigned int i=0; i<_viewportSize.y; i++)
  {
    for(unsigned int j=0; j<_viewportSize.x; j++)
    {
      mvaddch(i, j, doTransparency(mvinch(i,j), overlayLayer[i][j]));
    }
  }
}

void Renderer::doTick()
{
  for (unsigned int i=0; i<_activeItems.size(); i++)
  {
    if(_activeItems[i]->isEnabled())
    {
      _oldObjectPos.y=_activeItems[i]->rowPos();
      _oldObjectPos.x=_activeItems[i]->colPos();
      _activeItems[i]->tick();
    }
  }
}

void Renderer::updateScreen()
{
  refresh();
}

void Renderer::updateRelCameraPos(unsigned int rPos, unsigned int cPos)
{
  _prefRelViewportPos.y=rPos;
  _prefRelViewportPos.x=cPos;
}

void Renderer::loadNewLevel(string path)
{

}

void Renderer::loadNewLevel(Level level)
{

}

void Renderer::loadNewObjects(vector<ActiveObject *> newList)
{
  int size=_activeItems.size();
  for(int i=0; i<size; i++)
  {
    delete _activeItems[i];
  }
  _activeItems.clear();
  _activeItems=newList;
}

void Renderer::loadSprites(const string uncookedSpritesPath, const string spriteColorPath)
{
  ifstream sprites;
  ifstream spritesColor;
  sprites.open(uncookedSpritesPath);
  spritesColor.open(spriteColorPath);
  vector<vector<vector<chtype> > > aniFrames;
  vector<vector<chtype> > singleFrame;
  vector<chtype> singleRow;
  int numOfObjects;
  int numOfFrames;
  int numOfRows;
  int numOfCols;
  string row;
  int colorPair;
  chtype ch;

  sprites>>numOfObjects;
  for(int i=0; i<numOfObjects; i++)
  {
    sprites>>numOfFrames;
    aniFrames.clear();
    for(int j=0; j<numOfFrames; j++)
    {
      sprites>>numOfRows;
      sprites>>numOfCols;
      singleFrame.clear();
      getline(sprites, row);
      for(int k=0; k<numOfRows; k++);
      {
        getline(sprites, row);
        singleRow.clear();
        for(int l=0; l<numOfCols; l++)
        {
          spritesColor>>colorPair;
          ch=row[l]|COLOR_PAIR(colorPair);
          singleRow.push_back(ch);
        }
        singleFrame.push_back(singleRow);
      }
      aniFrames.push_back(singleFrame);
    }
    _activeItems[i]->changeSprites(aniFrames);
  }
  sprites.close();
  spritesColor.close();
}

void Renderer::loadSprites(const string cookedSpritesPath)
{
  ifstream sprites;
  sprites.open(cookedSpritesPath);
  vector<vector<vector<chtype> > > aniFrames;
  vector<vector<chtype> > singleFrame;
  vector<chtype> singleRow;
  int numOfObjects;
  int numOfFrames;
  int numOfRows;
  int numOfCols;
  chtype ch;

  sprites>>numOfObjects;
  for(int i=0; i<numOfObjects; i++)
  {
    sprites>>numOfFrames;
    aniFrames.clear();
    for(int j=0; j<numOfFrames; j++)
    {
      sprites>>numOfRows;
      sprites>>numOfCols;
      singleFrame.clear();
      for(int k=0; k<numOfRows; k++);
      {
        singleRow.clear();
        for(int l=0; l<numOfCols; l++)
        {
          sprites>>ch;
          singleRow.push_back(ch);
        }
        singleFrame.push_back(singleRow);
      }
      aniFrames.push_back(singleFrame);
    }
    _activeItems[i]->changeSprites(aniFrames);
  }
  sprites.close();
}

void Renderer::checkCollision(unsigned int AOindex)
{
  checkAObjectsCollision(AOindex);
  checkLevelCollision(AOindex);
}

Renderer::~Renderer()
{
  endwin();
}

void Renderer::drawObjects()
{
  for(unsigned int i=0; i<_activeItems.size(); i++)
  {
    if(_activeItems[i]->isEnabled())
    {
      if(_activeItems[i]->rowPos()>=_viewportPos.y && _activeItems[i]->rowPos()<=(_viewportPos.y+_viewportSize.y))
      {
        if(_activeItems[i]->colPos()>=_viewportPos.x && _activeItems[i]->colPos()<=(_viewportPos.x+_viewportSize.x))
        {
          Pos relPosition;
          relPosition.y=_activeItems[i]->rowPos()-_viewportPos.y;
          relPosition.x=_activeItems[i]->colPos()-_viewportPos.x;
          vector<vector<chtype> > frame=*_activeItems[i]->animate();
          for(unsigned int i=0; i<frame.size() && (i+relPosition.y)<(_viewportPos.y+_viewportSize.y); i++)
          {
            for(unsigned int j=0; j<frame[i].size() && (j+relPosition.x)<(_viewportPos.x+_viewportSize.x); j++)
            {
              mvaddch(relPosition.y+i, relPosition.x+j, doTransparency(mvinch(relPosition.y+i, relPosition.x+j), frame[i][j]));
            }
          }
        }
      }
    }
  }
}

void Renderer::checkAObjectsCollision(unsigned int AOindex)
{
  int rowPos=_activeItems[AOindex]->rowPos();
  int colPos=_activeItems[AOindex]->colPos();
  for(unsigned int i=0; i<_activeItems.size(); i++)
  {
    if(i!=AOindex)
    {
      if(_activeItems[i]->rowPos()>=rowPos && _activeItems[i]->rowPos()<=(rowPos+_activeItems[AOindex]->height()-1))
      {
        if(_activeItems[i]->colPos()>=colPos && _activeItems[i]->colPos()<=(colPos+_activeItems[AOindex]->width()-1))
        {
          //there is collision
          int horVector=_activeItems[i]->colPos()-_oldObjectPos[1];
          int verVector=_activeItems[i]->rowPos()-_oldObjectPos[0];
          if(verVector==0)
          {
            if(horVector<0)
            {
              _activeItems[AOindex]->collision(L, _activeItems[i]);
              _activeItems[i]->collision(R, _activeItems[AOindex]);
            }
            else if(horVector>0)
            {
              _activeItems[AOindex]->collision(R, _activeItems[i]);
              _activeItems[i]->collision(L, _activeItems[AOindex]);
            }
            else if(horVector==0)
            {
              _activeItems[AOindex]->collision(None, _activeItems[i]);
              _activeItems[i]->collision(None, _activeItems[AOindex]);
            }
          }
          else if (verVector<0)
          {
            if(horVector<0)
            {
              _activeItems[AOindex]->collision(LU, _activeItems[i]);
              _activeItems[i]->collision(RD, _activeItems[AOindex]);
            }
            else if(horVector>0)
            {
              _activeItems[AOindex]->collision(UR, _activeItems[i]);
              _activeItems[i]->collision(DL, _activeItems[AOindex]);
            }
            else if(horVector==0)
            {
              _activeItems[AOindex]->collision(U, _activeItems[i]);
              _activeItems[i]->collision(D, _activeItems[AOindex]);
            }
          }
          else if (verVector>0)
          {
            if(horVector<0)
            {
              _activeItems[AOindex]->collision(DL, _activeItems[i]);
              _activeItems[i]->collision(UR, _activeItems[AOindex]);
            }
            else if(horVector>0)
            {
              _activeItems[AOindex]->collision(RD, _activeItems[i]);
              _activeItems[i]->collision(LU, _activeItems[AOindex]);
            }
            else if(horVector==0)
            {
              _activeItems[AOindex]->collision(D, _activeItems[i]);
              _activeItems[i]->collision(U, _activeItems[AOindex]);
            }
          }
        }
      }
    }
  }
  return;
}

void Renderer::checkLevelCollision(unsigned int AOindex)
{
  LevelLayer layer=*_currentLevel.getLayer(_mainLayer);
  vector<chtype> row;
  int colPos=_activeItems[AOindex]->colPos();
  int rowPos=_activeItems[AOindex]->rowPos();
  int rightEdge=colPos+_activeItems[AOindex]->width()-1;
  int bottomEdge=rowPos+_activeItems[AOindex]->height()-1;
  for(int i=rowPos; i<=bottomEdge; i++)
  {
    if(i>=0 && i<layer.rows())
    {
      row=layer.getRow(i);
      for (int j=colPos; j<=rightEdge; j++)
      {
        if(j>=0 && j<layer.cols())
        {
          char t=row[j]&A_CHARTEXT;
          if(t!=' ')
          {
            //collision detected
            int horVector=colPos-_oldObjectPos[1];
            int verVector=rowPos-_oldObjectPos[0];
            int newColPos=colPos;
            int newRowPos=rowPos;
            if(horVector<0)
            {
              newColPos=colPos+(j-colPos)+1;
            }
            if(horVector>0)
            {
              newColPos=colPos-(rightEdge-j)-1;
            }
            if(verVector<0)
            {
              newRowPos=rowPos+(i-rowPos)+1;
            }
            if(verVector>0)
            {
              newRowPos=rowPos-(bottomEdge-i)-1;
            }
            _activeItems[AOindex]->move(newRowPos, newColPos);
            return;
          }
        }
      }
    }
  }
}

int Renderer::findColorPair(int fg, int bg)
{
  for(int i=0; i<COLOR_PAIRS; i++)
  {
    short int fore, back;
    pair_content(i, &fore, &back);
    if(fg==fore && bg==back)
      return i;
  }
  return -1;
}

chtype Renderer::doTransparency(chtype bottom, chtype top)
{
  if((top&A_CHARTEXT)==' ')
  {
    short int fg, bg;
    pair_content(PAIR_NUMBER(top), &fg, &bg);
    if(bg==0)
      return bottom;
    else
    {
      short int bottomFG, bottomBG;
      pair_content(PAIR_NUMBER(bottom), &bottomFG, &bottomBG);
      int matchingPair=findColorPair(bottomFG, bg);
      if(matchingPair>-1)
      {
        top=top&(~A_COLOR);
        top=top|COLOR_PAIR(matchingPair);
        return top;
      }
    }
  }
  else
  {
    short int fg,bg;
    pair_content(PAIR_NUMBER(top), &fg, &bg);
    if(fg==0 && bg!=0)
    {
      short int bottomFG, bottomBG;
      pair_content(PAIR_NUMBER(bottom), &bottomFG, &bottomBG);
      int matchingPair=findColorPair(bottomFG, bg);
      if(matchingPair>-1)
      {
        top=top&(~A_COLOR);
        top=top|COLOR_PAIR(matchingPair);
        return top;
      }
    }
    else if(bg==0 && fg!=0)
    {
      short int bottomFG, bottomBG;
      pair_content(PAIR_NUMBER(bottom), &bottomFG, &bottomBG);
      int matchingPair=findColorPair(fg, bottomBG);
      {
        if(matchingPair>-1)
        {
          top=top&(~A_COLOR);
          top=top|COLOR_PAIR(matchingPair);
          return top;
        }
      }
    }
    else if(bg==0 && fg==0)
    {
      top=top&(~A_COLOR);
      top=top|COLOR_PAIR(PAIR_NUMBER(bottom));
      return top;
    }
    else
    {
      return top;
    }
  }
  return top;
}

void Renderer::calculateViewportPos()
{
  LevelLayer* spriteLayer=_currentLevel.getLayer(_mainLayer);
  bool adjustedRow=false;
  bool adjustedCol=false;
  Pos rawViewportPos;
  rawViewportPos.y=_activeItems[0]->rowPos()-_prefRelViewportPos.y;
  rawViewportPos.x=_activeItems[0]->colPos()-_prefRelViewportPos.x;
  //string vpRowPos=to_string(rawViewportPos[0]);
  //string vpColPos=to_string(rawViewportPos[1]);
  //for(int i=0; i<vpRowPos.length(); i++)
  //  overlayLayer[5][i]=vpRowPos[i];
  //for(int i=0; i<vpColPos.length(); i++)
  //  overlayLayer[6][i]=vpColPos[i];
  Pos rawViewportEdge;
  rawViewportEdge.y=rawViewportPos.y+_viewportSize.y;
  rawViewportEdge.x=rawViewportPos.x+_viewportSize.x;
  //string vpRowEdge=to_string(rawViewportEdge[0]);
  //string vpColEdge=to_string(rawViewportEdge[1]);
  //for(int i=0; i<vpRowEdge.length(); i++)
  //  overlayLayer[7][i]=vpRowEdge[i];
  //for(int i=0; i<vpColEdge.length(); i++)
  //  overlayLayer[8][i]=vpColEdge[i];
  if(rawViewportPos.y<=0)
  {
    _viewportPos.y=0;
    adjustedRow=true;
  }
  if(rawViewportPos.x<=0)
  {
    _viewportPos.x=0;
    adjustedCol=true;
  }
  if(rawViewportEdge.y>=spriteLayer->rows())
  {
    _viewportPos.y=spriteLayer->rows()-_viewportSize.y;
    adjustedRow=true;
  }
  if(rawViewportEdge,x>=spriteLayer->cols())
  {
    _viewportPos.x=spriteLayer->cols()-_viewportSize.x;
    adjustedCol=true;
  }
  if(adjustedRow==false)
  {
    _viewportPos.y=_activeItems[0]->rowPos()-_prefRelViewportPos.y;
  }
  if(adjustedCol==false)
  {
    _viewportPos[1]=_activeItems[0]->colPos()-_prefRelViewportPos.x;
  }
}

void ActiveObject::move(int rPos, int cPos)
{
  _pos.y=rPos;
  _pos.x=cPos;
}

int ActiveObject::rowPos()
{
  return _pos.y;
}

int ActiveObject::colPos()
{
  return _pos.x;
}

unsigned int ActiveObject::height()
{
  return _size.y;
}

unsigned int ActiveObject::width()
{
  return _size.x;
}

void ActiveObject::disable()
{
  _enabled=false;
}

void ActiveObject::enable()
{
  _enabled=true;
}

bool ActiveObject::isEnabled()
{
  return _enabled;
}

void ActiveObject::changeSprites(vector<vector<vector<chtype> > > frames)
{
  _aniFrames=frames;
}

int ActiveObject::id()
{
  return _id;
}

Level::Level()
{
}

Level::Level(const string uncookedLevelPath, const string levelColorPath)
{
  ifstream level;
  ifstream levelColor;
  level.open(uncookedLevelPath);
  levelColor.open(levelColorPath);
  level>>_numLayers;
  vector<vector<chtype> > layer;
  vector<chtype> singleRow;
  string row;
  chtype ch;
  int colorPair;
  int width, height;
  double relativeSpeed;
  int tileX, tileY;
  for(unsigned int i=0; i<_numLayers; i++)
  {
    level>>height;
    level>>width;
    level>>relativeSpeed;
    level>>tileY;
    level>>tileX;
    getline(level, row);
    layer.clear();
    for(int j=0; j<height; j++)
    {
      singleRow.clear();
      getline(level, row);
      for(int k=0; k<width; k++)
      {
        levelColor>>colorPair;
        ch=row[k]|COLOR_PAIR(colorPair);
        singleRow.push_back(ch);
      }
      layer.push_back(singleRow);
    }
    _layers.push_back(LevelLayer(layer, relativeSpeed, (tileX==0?false:true), (tileY==0?false:true)));
  }

  level.close();
  levelColor.close();
}

Level::Level(const string cookedLevelPath)
{
  ifstream level;
  level.open(cookedLevelPath);
  level>>_numLayers;
  vector<vector<chtype> > layer;
  vector<chtype> singleRow;
  chtype ch;
  int width, height;
  double relativeSpeed;
  int tileX, tileY;
  for(unsigned int i=0; i<_numLayers; i++)
  {
    level>>height;
    level>>width;
    level>>relativeSpeed;
    level>>tileY;
    level>>tileX;
    for(int j=0; j<height; j++)
    {
      singleRow.clear();
      for(int k=0; k<width; k++)
      {
        level>>ch;
        singleRow.push_back(ch);
      }
      layer.push_back(singleRow);
    }
    _layers.push_back(LevelLayer(layer, relativeSpeed, (tileX==0?false:true), (tileY==0?false:true)));
  }
  level.close();
}

LevelLayer* Level::getLayer(unsigned int layer)
{
  return &_layers[layer];
}

unsigned int Level::layerCount()
{
  return _numLayers;
}

LevelLayer::LevelLayer()
{
  _cols=1;
  _rows=1;
  _relativeSpeed=1.0;
  _layer.push_back(vector<chtype>());
  _layer[0].push_back('.');
}

LevelLayer::LevelLayer(vector<vector<chtype> > layer, double rSpeed, bool tileHorizontally, bool tileVertically)
{
  _rows=layer.size();
  _cols=layer[0].size();
  _relativeSpeed=rSpeed;
  _layer=layer;
  _tileHor=tileHorizontally;
  _tileVer=tileVertically;
}

unsigned int LevelLayer::rows()
{
  return _rows;
}

unsigned int LevelLayer::cols()
{
  return _cols;
}

double LevelLayer::relativeSpeed()
{
  return _relativeSpeed;
}

bool LevelLayer::doTileHor()
{
  return _tileHor;
}

bool LevelLayer::doTileVer()
{
  return _tileVer;
}

vector<chtype> LevelLayer::getRow(int row)
{
  return _layer[row];
}


Level::Level(vector<LevelLayer> layers)
{
  _layers=layers;
  _numLayers=layers.size();
}

Level::Level(const Level &level)
{
  _numLayers=level._numLayers;
  _layers=level._layers;
}
