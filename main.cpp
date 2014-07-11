#include<QTimer>
#include<QCoreApplication>
#include"asciirunner.h"

using namespace std;

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  ASCIIRunner::Game game(&a);
  return a.exec();
}
