#ifndef GENERISI_H
#define GENERISI_H

#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <ncurses.h>
#include <fstream>
#include "asciirenderengine.h"

//TODO: Strukturu po mogucnosti prebaciti negde drugde. Mozda napraviti od nje klasu, radi lakseg/citljivijeg pomeranja tacki (redefinisati + operator)?
struct tacka{
    int x;
    int y;
};

void Generisi_pocetak_platforme(tacka *);

void Generisi_platformu(tacka *, std::vector<std::vector<chtype> > &);

ASCIIRenderEngine::LevelLayer Generisi_nivo();


#endif // GENERISI_H
