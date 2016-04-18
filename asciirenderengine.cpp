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
  _viewportPos[0]=0;
  _viewportPos[0]=0;
  _viewportSize[0]=25;
  _viewportSize[1]=80;
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
    if(ini.numberOfColorsUsed!=0)
    {
      for(int i=0; i<ini.numberOfColorsUsed; i++) //definisemo boje
      {
        init_color(i, ini.colors[i][0], ini.colors[i][1], ini.colors[i][2]);
      }
    }
    int pair=1;
    for(int i=0; i<8; i++) //definisemo podrazumevane parove
    {
      for(int j=0; j<8; j++)
      {
        init_pair(pair, i, j);
        pair++;
      }
    }
    if(ini.numberOfPairsUsed!=0)
    {
      for(int i=0; i<ini.numberOfPairsUsed; i++) //prepisujemo prvih nekoliko parova sa user-defined vrednostima
      {
        init_pair(i, ini.colorPairs[i][0], ini.colorPairs[i][1]);
      }
    }
  }
  else
  {
    printw("Can't change colors, using default.\n");
  }
  _viewportSize[0]=ini.prefViewportSize[0];
  _viewportSize[1]=ini.prefViewportSize[1];
  _activeItems=ini.activeObjectList;
  _mainLayer=ini.mainLayer;
  _prefRelPlayerPos[0]=ini.playerRelPosition[0];
  _prefRelPlayerPos[1]=ini.playerRelPosition[1];

  if(ini.levelPath=="")
  {
    _currentLevel=ini.level;
  }
  else
  {
    ofstream trace;
    trace.open("traceLoadStaticLevel.txt");
    ifstream levelManifest;
    string fullPath=ini.levelPath+"level.manifest";
    levelManifest.open(fullPath);
    trace<<fullPath<<endl;
    trace.flush();
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
        trace<<"type: "<<type<<endl;
        trace.flush();
      }
      else if(tag=="c")
      {
        levelManifest>>colorPath;
        trace<<colorPath<<endl;
        trace.flush();
      }
      else if(tag=="l")
      {
        levelManifest>>levelPath;
        trace<<levelPath<<endl;
        trace.flush();
      }
      else if(tag=="lc")
      {
        levelManifest>>levelColorPath;
        trace<<levelColorPath<<endl;
        trace.flush();
      }
      else if(tag=="ao")
      {
        levelManifest>>aoPath;
        trace<<aoPath<<endl;
        trace.flush();
      }
      else if(tag=="aoc")
      {
        levelManifest>>aoColorPath;
        trace<<aoColorPath<<endl;
        trace.flush();
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
        trace<<ini.levelPath+levelPath<<" "<<ini.levelPath+levelColorPath<<endl;
        trace.flush();
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
  if(getmaxx(stdscr)!=ini.prefViewportSize[1] && getmaxy(stdscr)!=ini.prefViewportSize[0])
  {
    printw("This game is meant to be played in a %d x %d characters window, but the game can't resize it automatically.\nPlease resize the window, or hit Enter to quit.");
    while(getmaxx(stdscr)!=ini.prefViewportSize[1] && getmaxy(stdscr)!=ini.prefViewportSize[0])
    {
      if(getch()==KEY_ENTER)
      {
        endwin();
        exit(EXIT_FAILURE);
      }
    }
  }
  vector<chtype> row;
  for(unsigned int i=0; i<_viewportSize[0]; i++)
  {
    row.clear();
    for(unsigned int j=0; j<_viewportSize[1]; j++)
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
  string vpRowPos=to_string(_viewportPos[0]);
  string vpColPos=to_string(_viewportPos[1]);
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
    for(unsigned int rS=0; rS<_viewportSize[0]; rS++)
    {
      rL=(int)floor(_viewportPos[0]*layer->relativeSpeed());
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
        cL=(int)floor(_viewportPos[1]*layer->relativeSpeed());
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
  for(unsigned int i=0; i<_viewportSize[0]; i++)
  {
    for(unsigned int j=0; j<_viewportSize[1]; j++)
    {
      mvaddch(i, j, doTransparency(mvinch(i,j), overlayLayer[i][j]));
    }
  }
}

void Renderer::doTick()
{
  ofstream trace;
  trace.open("traceRendererDoTick.txt");
  for (unsigned int i=0; i<_activeItems.size(); i++)
  {
    if(_activeItems[i]->isAlive())
    {
      _oldObjectPos[0]=_activeItems[i]->rowPos();
      _oldObjectPos[1]=_activeItems[i]->colPos();
      _activeItems[i]->tick();
      trace<<"called tick for "<<i<<endl;
      trace.flush();
      checkCollision(i);
      trace<<"collision checked for "<<i<<endl;
      trace.flush();
    }
  }
  trace.close();
}

void Renderer::updateScreen()
{
  refresh();
}

void Renderer::updateRelCameraPos(unsigned int rPos, unsigned int cPos)
{
  _prefRelPlayerPos[0]=rPos;
  _prefRelPlayerPos[1]=cPos;
}

void Renderer::loadNewLevel(string path)
{
  ifstream levelManifest;
  levelManifest.open(path+"level.manifest");
  string tag;
  int type;
  string levelPath="";
  string levelColorPath="";
  while(levelManifest.good())
  {
    levelManifest>>tag;
    if(tag=="t")
    {
      levelManifest>>type;
    }
    else if(tag=="l")
    {
      levelManifest>>levelPath;
    }
    else if(tag=="lc")
    {
      levelManifest>>levelColorPath;
    }
  }
  levelManifest.close();

  if(levelPath!="")
  {
    if(type==0)
    {
      _currentLevel=Level(path+levelPath, path+levelColorPath);
    }
    else if(type==1)
    {
      _currentLevel=Level(path+levelPath);
    }
  }
}

void Renderer::loadNewLevel(Level level)
{
  _currentLevel=level;
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
  ofstream trace;
  trace.open("traceLoadSprites.txt");
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
  trace<<numOfObjects<<endl;
  trace.flush();
  for(int i=0; i<numOfObjects; i++)
  {
    sprites>>numOfFrames;
    aniFrames.clear();
    trace<<numOfFrames<<endl;
    trace.flush();
    for(int j=0; j<numOfFrames; j++)
    {
      sprites>>numOfRows;
      sprites>>numOfCols;
      singleFrame.clear();
      getline(sprites, row);
      trace<<numOfRows<<" "<<numOfCols<<endl;
      trace.flush();
      for(int k=0; k<numOfRows; k++);
      {
        getline(sprites, row);
        singleRow.clear();
        trace<<row<<endl;
        trace.flush();
        for(int l=0; l<numOfCols; l++)
        {
          spritesColor>>colorPair;
          trace<<"char and its color: "<<row[l]<<" "<<colorPair<<endl;
          trace.flush();
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
  trace.close();
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
  ofstream trace;
  trace.open("traceCheckCollision.txt");
  checkAObjectsCollision(AOindex);
  trace<<"done checking AO collision"<<endl;
  trace.flush();
  checkLevelCollision(AOindex);
  trace<<"done checking level collision"<<endl;
  trace.close();
}

Renderer::~Renderer()
{
  endwin();
}

void Renderer::drawObjects()
{
  ofstream trace;
  trace.open("traceDrawObjects.txt");
  for(unsigned int i=0; i<_activeItems.size(); i++)
  {
    trace<<"processing item "<<i<<endl;
    trace.flush();
    if(_activeItems[i]->isAlive())
    {
      trace<<"item is alive"<<endl;
      trace.flush();
      if(_activeItems[i]->rowPos()>=_viewportPos[0] && _activeItems[i]->rowPos()<=(_viewportPos[0]+_viewportSize[0]))
      {
        trace<<"item is in row range"<<endl;
        trace.flush();
        if(_activeItems[i]->colPos()>=_viewportPos[1] && _activeItems[i]->colPos()<=(_viewportPos[1]+_viewportSize[1]))
        {
          trace<<"item is in col range"<<endl;
          trace.flush();
          int relPosition[2];
          relPosition[0]=_activeItems[i]->rowPos()-_viewportPos[0];
          relPosition[1]=_activeItems[i]->colPos()-_viewportPos[1];
          trace<<"item's relative pos is "<<relPosition[0]<<","<<relPosition[1]<<endl;
          trace.flush();
          vector<vector<chtype> > frame=*_activeItems[i]->animate();
          trace<<"frame obtained"<<endl;
          trace.flush();
          trace<<"size of the frame is "<<frame.size()<<","<<frame[0].size()<<endl;
          trace.flush();
          for(unsigned int i=0; i<frame.size() && (i+relPosition[0])<(_viewportPos[0]+_viewportSize[0]); i++)
          {
            for(unsigned int j=0; j<frame[i].size() && (j+relPosition[1])<(_viewportPos[1]+_viewportSize[1]); j++)
            {
              mvaddch(relPosition[0]+i, relPosition[1]+j, doTransparency(mvinch(relPosition[0]+i, relPosition[1]+j), frame[i][j]));
              trace<<"printed character "<<i<<","<<j<<endl;
              trace.flush();
            }
          }
        }
      }
    }
  }
  trace.close();
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
  ofstream trace;
  trace.open("traceCheckLevelCollision.txt");
  trace<<"checking level collision for object "<<AOindex<<endl;
  trace.flush();
  trace<<"number of layers is "<<_currentLevel.layerCount()<<endl;
  trace.flush();
  trace<<"sprite layer is: "<<_mainLayer<<endl;
  trace.flush();
  LevelLayer layer=*_currentLevel.getLayer(_mainLayer);
  trace<<"got copy of layer"<<endl;
  trace.flush();
  vector<chtype> row;
  int colPos=_activeItems[AOindex]->colPos();
  trace<<"colPos: "<<colPos<<endl;
  trace.flush();
  int rowPos=_activeItems[AOindex]->rowPos();
  trace<<"rowPos: "<<rowPos<<endl;
  trace.flush();
  int rightEdge=colPos+_activeItems[AOindex]->width()-1;
  trace<<"rightEdge: "<<rightEdge<<endl;
  trace.flush();
  int bottomEdge=rowPos+_activeItems[AOindex]->height()-1;
  trace<<"bottomEdge: "<<bottomEdge<<endl;
  trace.flush();
  for(int i=rowPos; i<=bottomEdge; i++)
  {
    if(i>=0 && i<layer.rows())
    {
      trace<<"layer.rows(): "<<layer.rows()<<endl;
      trace.flush();
      row=layer.getRow(i);
      trace<<"got row of "<<i<<endl;
      trace.flush();
      for (int j=colPos; j<=rightEdge; j++)
      {
        if(j>=0 && j<layer.cols())
        {
          trace<<"layer.cols(): "<<layer.cols()<<endl;
          trace.flush();
          char t=row[j]&A_CHARTEXT;
          trace<<"character on location "<<i<<","<<j<<" is "<<t<<endl;
          if(t!=' ')
          {
            //collision detected
            trace<<"collision detected"<<endl;
            trace.flush();
            int horVector=colPos-_oldObjectPos[1];
            int verVector=rowPos-_oldObjectPos[0];
            trace<<"horVector is : "<<horVector;
            trace.flush();
            trace<<"verVector is : "<<verVector;
            trace.flush();
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
            trace<<"new location - row: "<<newRowPos<<" col: "<<newColPos<<endl;
            trace.close();
            return;
          }
        }
      }
    }
  }
  trace.close();
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
  int rawViewportPos[2];
  rawViewportPos[0]=_activeItems[0]->rowPos()-_prefRelPlayerPos[0];
  rawViewportPos[1]=_activeItems[0]->colPos()-_prefRelPlayerPos[1];
  //string vpRowPos=to_string(rawViewportPos[0]);
  //string vpColPos=to_string(rawViewportPos[1]);
  //for(int i=0; i<vpRowPos.length(); i++)
  //  overlayLayer[5][i]=vpRowPos[i];
  //for(int i=0; i<vpColPos.length(); i++)
  //  overlayLayer[6][i]=vpColPos[i];
  int rawViewportEdge[2];
  rawViewportEdge[0]=rawViewportPos[0]+_viewportSize[0];
  rawViewportEdge[1]=rawViewportPos[1]+_viewportSize[1];
  //string vpRowEdge=to_string(rawViewportEdge[0]);
  //string vpColEdge=to_string(rawViewportEdge[1]);
  //for(int i=0; i<vpRowEdge.length(); i++)
  //  overlayLayer[7][i]=vpRowEdge[i];
  //for(int i=0; i<vpColEdge.length(); i++)
  //  overlayLayer[8][i]=vpColEdge[i];
  if(rawViewportPos[0]<=0)
  {
    _viewportPos[0]=0;
    adjustedRow=true;
  }
  if(rawViewportPos[1]<=0)
  {
    _viewportPos[1]=0;
    adjustedCol=true;
  }
  if(rawViewportEdge[0]>=spriteLayer->rows())
  {
    _viewportPos[0]=spriteLayer->rows()-_viewportSize[0];
    adjustedRow=true;
  }
  if(rawViewportEdge[1]>=spriteLayer->cols())
  {
    _viewportPos[1]=spriteLayer->cols()-_viewportSize[1];
    adjustedCol=true;
  }
  if(adjustedRow==false)
  {
    _viewportPos[0]=_activeItems[0]->rowPos()-_prefRelPlayerPos[0];
  }
  if(adjustedCol==false)
  {
    _viewportPos[1]=_activeItems[0]->colPos()-_prefRelPlayerPos[1];
  }
}

void ActiveObject::move(int rPos, int cPos)
{
  _pos[0]=rPos;
  _pos[1]=cPos;
}

int ActiveObject::rowPos()
{
  return _pos[0];
}

int ActiveObject::colPos()
{
  return _pos[1];
}

unsigned int ActiveObject::height()
{
  return _size[0];
}

unsigned int ActiveObject::width()
{
  return _size[1];
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
  ofstream trace;
  trace.open("traceLevelConstructor.txt");
  ifstream level;
  ifstream levelColor;
  level.open(uncookedLevelPath);
  levelColor.open(levelColorPath);
  trace<<uncookedLevelPath<<" "<<levelColorPath<<endl;
  trace.flush();
  level>>_numLayers;
  trace<<"num of layers: "<<_numLayers<<endl;
  trace.flush();
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
    trace<<"height, width, relative speed, tileY, tileX: "<<height<<" "<<width<<" "<<relativeSpeed<<" "<<tileX<<" "<<tileY<<endl;
    getline(level, row);
    trace.flush();
    layer.clear();
    for(int j=0; j<height; j++)
    {
      singleRow.clear();
      getline(level, row);
      trace<<row<<endl;
      trace.flush();
      for(int k=0; k<width; k++)
      {
        levelColor>>colorPair;
        trace<<"char and its color pair: "<<row[k]<<" "<<colorPair<<endl;
        trace.flush();
        ch=row[k]|COLOR_PAIR(colorPair);
        singleRow.push_back(ch);
      }
      layer.push_back(singleRow);
    }
    _layers.push_back(LevelLayer(layer, relativeSpeed, (tileX==0?false:true), (tileY==0?false:true)));
  }

  level.close();
  levelColor.close();
  trace.close();
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
  ofstream trace;
  trace.open("traceLevelLayerConstructor");
  _rows=layer.size();
  trace<<"_rows: "<<_rows<<endl;
  _cols=layer[0].size();
  trace<<"_cols: "<<_cols<<endl;
  _relativeSpeed=rSpeed;
  trace<<"_relSpeed: "<<_relativeSpeed<<endl;
  _layer=layer;
  trace<<"layer copy success"<<endl;
  _tileHor=tileHorizontally;
  _tileVer=tileVertically;
  trace<<"tile settings success, done"<<endl;
  trace.close();
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
