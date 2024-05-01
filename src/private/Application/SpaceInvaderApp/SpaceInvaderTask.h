////////////////////////////////////////////////////////////////////////////////
// Authors: schalklab@HR18818.wucon.wustl.edu
// Description: SpaceInvaderTask header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_SPACEINVADERTASK_H  // makes sure this header is not included more than once
#define INCLUDED_SPACEINVADERTASK_H

#include "ApplicationBase.h"

class SpaceInvaderTask : public ApplicationBase
{
 public:
  SpaceInvaderTask();
  ~SpaceInvaderTask();
  void Publish() override;
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;
  void Halt() override;

 private:
   // Use this space to declare any SpaceInvaderTask-specific methods and member variables you'll need
   class QWidget *myWindow;
   class QGraphicsScene * myScene;
   class QGraphicsView* mySceneView;
   class QGraphicsSimpleTextItem* mySimpleTextItem;

   int score = 0;
   int key = 0;
   int life = 0;
   int shipX = 0;
   int enemyBulletX = 0;
   int enemyBulletY = 0;
   int shipBulletX = 0;
   int shipBulletY = 0;
   int enemyBottom = 0;
   int shipExplosion = 0;
   int enemyExplosion = 0;
   int mysteryEnter = 0;
   int mysteryEnemyExplosion = 0;


   void SetText(int x, RGBColor& color, QGraphicsSimpleTextItem* textItem, const int pos1, const int pos2, const std::string label);
   QGraphicsSimpleTextItem* instantTextItem();

};

#endif // INCLUDED_SPACEINVADERTASK_H
