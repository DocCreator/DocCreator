#include "RandomBackGroundChanger.hpp"
#include "Utils/RandomElement.hpp"
#include <context/backgroundcontext.h>

//#include <iostream>//DEBUG

RandomBackGroundChanger::RandomBackGroundChanger(int nb, QObject *parent)
  : QObject(parent)
  , _nb(nb)
{}

void
RandomBackGroundChanger::changeBackGround()
{

  for (int i = 0; i <= _nb; i++) {
    Context::BackgroundList list =
      Context::BackgroundContext::instance()->getBackgrounds();

    RandomElement randomer;
    QString chosenBackGround = QStringLiteral("newBackGroundTOTO.png");
    while (chosenBackGround == QLatin1String("newBackGroundTOTO.png"))
      chosenBackGround = list.at(randomer.randomInt(0, list.size() - 1));

    //std::cerr<<"# RandomBackGroundChanger::changeBackGround() new background="<<chosenBackGround.toStdString()<<"\n";

    Context::BackgroundContext::instance()->setCurrentBackground(
      chosenBackGround);

    emit backGroundChanged();
  }
}
