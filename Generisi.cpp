#include "Generisi.h"

const int lines=25;
const int cols=80;

void Generisi_pocetak_platforme(tacka * Kraj){

    //TODO: izmestiti visinu skoka u klasu kretanja? Podesiti u pravilnom odnosu na velicinu prozora.
    int visina_skoka = 6;
    int bottom_y = -lines*9/20;
    //po potrebi dodatno korigovati koeficijent k. Iz nekog razloga ne prihvata definiciju oblika i/j...
    double k = 0.18;
    int xMax = (int) (sqrt(abs(lines - 2*visina_skoka)) / (k * sqrt(3)));
    //generise u opsegu [0,xMax]
    int x_pomeraj = (rand() % (int)(xMax + 1));
    //TODO: Po potrebi podesiti parametre formule. Problem je sto uglavnom daje negativne brojeve. Mozda cackati da cesce daje pozitivne?
    int yMax = (int) (-((k * x_pomeraj)*(k * x_pomeraj)) + (2/3*visina_skoka));
    //generise u opsegu [bottom_y, yMax]
    int y_pomeraj;
    if((yMax+bottom_y+1)==0)
    {
      y_pomeraj=bottom_y+(rand()%1);
    }
    else
    {
      y_pomeraj = bottom_y + (rand() % (int)(yMax + bottom_y + 1));
    }
    Kraj->x = Kraj->x + x_pomeraj;
    Kraj->y = Kraj->y - y_pomeraj; // Kraj->y - y_pomeraj ukoliko je obrnut koordinatni sistem

}

void Generisi_platformu(tacka * Kraj, std::vector<std::vector <chtype> > &matrica){

    Generisi_pocetak_platforme(Kraj);
    //printw("(%d, %d)\n", Kraj->x, Kraj->y);
    //Generise u opsegu [6,12]
    int duzina_platforme = 6 + rand()%7;
    int kraj_platforme = Kraj->x + duzina_platforme;

    while(Kraj->x < kraj_platforme){
        //TODO: Dodati slozenije platforme
        matrica[Kraj->y][Kraj->x] = '-'|COLOR_PAIR(29);
        Kraj->x++;
    }
}


ASCIIRenderEngine::LevelLayer Generisi_nivo(){
    srand(time(NULL));

    //Generise u opsegu [2000, 10000]

    //8001 = 10000-2000+1

    int duzina_nivoa = 2000 + rand()%2097;

    tacka Kraj = {0, lines/2};

    //od njih zavisi da li ce se dodavati novi redovi na matricu

    int max_platforma = lines/2;
    int min_platforma = lines/2;

    //inicijalizacija matrice

    std::vector <chtype> prazna_linija (duzina_nivoa, ' '|COLOR_PAIR(1));
    std::vector <std::vector<chtype> > matrica;

    do{
        matrica.push_back(prazna_linija);
    }while(matrica.size() < lines);

    //inicijalizacija pocetnog ekrana

    while(Kraj.x < cols){
        //TODO: Dodati slozenije platforme
        matrica[Kraj.y][Kraj.x] = '-'|COLOR_PAIR(29);
        Kraj.x++;
    }

    //Generisanje nivoa

    while(Kraj.x <= duzina_nivoa - cols){
        Generisi_platformu(&Kraj, matrica);
        //regulisanje broja redova matrice.
        if(Kraj.y < min_platforma){
            for(int i = min_platforma - Kraj.y; i>0; i--){
                matrica.insert(matrica.begin(), prazna_linija);
                Kraj.y++;
            }
            min_platforma = Kraj.y;
        }
        else if(Kraj.y > max_platforma){
            for(int i = Kraj.y - max_platforma; i>0; i--){
             matrica.push_back(prazna_linija);
             max_platforma = Kraj.y;
            }
        }
    }

    //Generisanje kraja nivoa

    while(Kraj.x < duzina_nivoa){
        //TODO: Dodati slozenije platforme
        matrica[Kraj.y][Kraj.x] = '-'|COLOR_PAIR(29);
        Kraj.x++;
    }

    /* testiranje generisanja platformi
    std::ofstream Fajl("nivo.txt");
    for(int i = 0; i < matrica.size()-1; i++){
        for(int j = 0; j < duzina_nivoa-1; j++)
            Fajl << matrica[i][j];
        Fajl << std::endl;
    }
    printw("(%d, %d)\n", Kraj.x, Kraj.y);
    printw("matrica size: %d\n", matrica.size());
    */


    //Konstruisanje LevelLayer i level objekata.
    return ASCIIRenderEngine::LevelLayer (matrica, 1, false, false);

}

/*Primer upotrebe funkcije Generisi_pocetak_platforme, koriscen pri testiranju:

int main(){

    initscr();

    Generisi_nivo();
    refresh();
    getch();

    endwin();

return 1;
}
*/
