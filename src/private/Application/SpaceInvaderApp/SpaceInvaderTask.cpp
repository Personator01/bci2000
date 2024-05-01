////////////////////////////////////////////////////////////////////////////////
// Authors: schalklab@HR18818.wucon.wustl.edu
// Description: SpaceInvaderTask implementation
////////////////////////////////////////////////////////////////////////////////

#include "SpaceInvaderTask.h"
#include "BCIStream.h"
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsSimpleTextItem>
#include "Localization.h"



RegisterFilter( SpaceInvaderTask, 3 );
     // Change the location if appropriate, to determine where your filter gets
     // sorted into the chain. By convention:
     //  - filter locations within SignalSource modules begin with "1."
     //  - filter locations within SignalProcessing modules begin with "2."
     //       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
     //  - filter locations within Application modules begin with "3."


SpaceInvaderTask::SpaceInvaderTask():
    myWindow(NULL),myScene(NULL),mySceneView(NULL), mySimpleTextItem(NULL)
{
  // C++ does not initialize simple types such as numbers, or pointers, by default.
  // Rather, they will have random values.
  // Take care to initialize any member variables here, so they have predictable values
  // when used for the first time.
}

SpaceInvaderTask::~SpaceInvaderTask()
{
  Halt();
  // If you have allocated any memory with malloc(), call free() here.
  // If you have allocated any memory with new[], call delete[] here.
  // If you have created any object using new, call delete here.
  // Either kind of deallocation will silently ignore null pointers, so all
  // you need to do is to initialize those to zero in the constructor, and
  // deallocate them here.
  if (myWindow)
      delete myWindow;
  myWindow = NULL;

  if (myScene)
      delete myScene;
  myScene = NULL;
}

void
SpaceInvaderTask::Publish()
{
  // Define any parameters that the filter needs....

    BEGIN_PARAMETER_DEFINITIONS
        "Application:Window int WindowWidth= 640 640 0 % "
        " // width of application window",
        "Application:Window int WindowHeight= 480 480 0 % "
        " // height of application window",
        "Application:Window int WindowLeft= 0 0 % % "
        " // screen coordinate of application window's left edge",
        "Application:Window int WindowTop= 0 0 % % "
        " // screen coordinate of application window's top edge",
    END_PARAMETER_DEFINITIONS


        // ...and likewise any state variables:

        myWindow = new QWidget;
        myWindow->setWindowFlag(Qt::FramelessWindowHint);
        myWindow->setWindowTitle("BCI2000 Space Invader");
        
        myScene = new QGraphicsScene;
        myScene->setBackgroundBrush(QBrush(Qt::black));

        mySceneView = new QGraphicsView(myWindow);
        mySceneView->setScene(myScene);
        mySceneView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mySceneView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        mySceneView->show();

        mySimpleTextItem = instantTextItem();

        myWindow->hide();        
}


void
SpaceInvaderTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
  // The user has pressed "Set Config" and we need to sanity-check everything.
  // For example, check that all necessary parameters and states are accessible:
  //
  // Parameter( "Milk" );
  // State( "Bananas" );
  //
  // Also check that the values of any parameters are sane:
  //
  // if( (float)Parameter( "Denominator" ) == 0.0f )
  //      bcierr << "Denominator cannot be zero";
  //
  // Errors issued in this way, during Preflight, still allow the user to open
  // the Config dialog box, fix bad parameters and re-try.  By contrast, errors
  // and C++ exceptions at any other stage (outside Preflight) will make the
  // system stop, such that BCI2000 will need to be relaunched entirely.

  // Note that the SpaceInvaderTask instance itself, and its members, are read-only during
  // this phase, due to the "const" at the end of the Preflight prototype above.
  // Any methods called by Preflight must also be "const" in the same way.
  
    //call these parameter atlest once,make sure check all the parameter

    Parameter("WindowHeight");
    Parameter("WindowWidth");
    Parameter("WindowLeft");
    Parameter("WindowTop");
    State("score");
    State("key");
    State("life");
    State("ship_x");
    State("enemy_bullet_x");
    State("enemy_bullet_y");
    State("ship_bullet_x");
    State("ship_bullet_y");
    State("enemy_bottom");
    State("ship_explosion");
    State("enemy_explosion");
    State("mystery_enter");
    State("mystery_enemy_explosion");


    bciout << "=====================Preflight" << std::endl;

}


void
SpaceInvaderTask::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
  // The user has pressed "Set Config" and all Preflight checks have passed.
  // The signal properties can no longer be modified, but the const limitation has gone, so
  // the SpaceInvaderTask instance itself can be modified. Allocate any memory you need, start any
  // threads, store any information you need in private member variables.

    myWindow->move(Parameter("WindowLeft"), Parameter("WindowTop"));
    myWindow->resize(Parameter("WindowWidth"), Parameter("WindowHeight"));
    mySceneView->resize(Parameter("WindowWidth"), Parameter("WindowHeight"));
    mySceneView->setSceneRect(0, 0, Parameter("WindowWidth"), Parameter("WindowHeight"));
    mySceneView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mySceneView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mySceneView->show();

    RGBColor textColor(RGBColor::Green);

    bciout << "=====================Initialize" << std::endl;
    
    myWindow->show();
}

void
SpaceInvaderTask::StartRun()
{
  // The user has just pressed "Start" (or "Resume")
  bciout << "=====================StartRun" << std::endl;
  RGBColor textColor(RGBColor::Red);

}


void
SpaceInvaderTask::Process( const GenericSignal& Input, GenericSignal& Output )
{
  // And now we're processing a single SampleBlock of data.
  // Remember not to take too much CPU time here, or you will break the real-time constraint.

    //bciout << "=====================Process" << std::endl;
    score = State("score");
    key = State("key");
    life = State("life");
    shipX = State("ship_x");
    enemyBulletX = State("enemy_bullet_x");
    enemyBulletY = State("enemy_bullet_y");
    shipBulletX = State("ship_bullet_x");
    shipBulletY = State("ship_bullet_Y");
    enemyBottom = State("enemy_bottom");
    shipExplosion = State("ship_explosion");
    enemyExplosion = State("enemy_explosion");
    mysteryEnter = State("mystery_enter");
    mysteryEnemyExplosion = State("mystery_enemy_explosion");


    //int pStep = 40;
    //show the states
    QFont labelFont;
    RGBColor textColor(RGBColor::Yellow);
    std::string sX = "Score: " + std::to_string(score) + "\n" +
        "Keyborad: " + std::to_string(key) + "\n" +
        "Lives: " + std::to_string(life) + "\n" +
        "Ship_x: " + std::to_string(shipX) + "\n" +
        "Ship_y: " + "540" + "\n" +
        "Enemy_bullet_x: " + std::to_string(enemyBulletX) + "\n" +
        "Enemy_bullet_y: " + std::to_string(enemyBulletY) + "\n" +
        "Ship_bullet_x: " + std::to_string(shipBulletX) + "\n" +
        "Ship_bullet_y: " + std::to_string(shipBulletY) + "\n" +
        "Enemy_bottom: " + std::to_string(enemyBottom) + "\n"
        "ship_explosion: " + std::to_string(shipExplosion) + "\n"
        "enemy_explosion: " + std::to_string(enemyExplosion) + "\n"
        "mystery_enter: " + std::to_string(mysteryEnter) + "\n"
        "mystery_enemy_explosion: " + std::to_string(mysteryEnemyExplosion)        
        ;
    char const* cX = sX.c_str();

    labelFont.fromString("Arial");
    labelFont.setPixelSize(myWindow->height() / 20);
    QFontMetrics fm(labelFont);
    mySimpleTextItem->setFont(labelFont);
    mySimpleTextItem->setPos(20, 10);
    mySimpleTextItem->setPen(QPen(QColor(textColor.R(), textColor.G(), textColor.B())));
    mySimpleTextItem->setBrush(QBrush(QColor(textColor.R(), textColor.G(), textColor.B())));
    mySimpleTextItem->setText(cX);
}

void
SpaceInvaderTask::StopRun()
{
  // The Running state has been set to 0, either because the user has pressed "Suspend",
  // or because the run has reached its natural end.
    bciout << "=====================StopRun" << std::endl;
    RGBColor textColor(RGBColor::Blue);
  // You know, you can delete methods if you're not using them.
  // Remove the corresponding declaration from SpaceInvaderTask.h too, if so.
}

void
SpaceInvaderTask::Halt()
{
  // Stop any threads or other asynchronous activity.
  // Good practice is to write the Halt() method such that it is safe to call it even *before*
  // Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
  // have already been deleted:  set them to NULL after deletion).
}


void SpaceInvaderTask::SetText(int x, RGBColor& color, QGraphicsSimpleTextItem *textItem, const int pos1, const int pos2, const std::string label)
{
    QFont labelFont;
    std::string sX = label + std::to_string(x) + "\n test";
    char const* cX = sX.c_str();

    labelFont.fromString("Arial");
    labelFont.setPixelSize(myWindow->height() / 20);
    QFontMetrics fm(labelFont);
    textItem->setFont(labelFont);
    textItem->setPos(pos1, pos2);
    textItem->setPen(QPen(QColor(color.R(), color.G(), color.B())));
    textItem->setBrush(QBrush(QColor(color.R(), color.G(), color.B())));
    textItem->setText(cX);
}

QGraphicsSimpleTextItem* SpaceInvaderTask::instantTextItem() {
    QGraphicsSimpleTextItem* textItem = new QGraphicsSimpleTextItem;
    myScene->addItem(textItem);
    textItem->show();
    return textItem;
}
