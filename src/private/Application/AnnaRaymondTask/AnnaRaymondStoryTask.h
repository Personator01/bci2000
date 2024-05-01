////////////////////////////////////////////////////////////////////////////////
// Authors: schalklab@HR18818.wucon.wustl.edu
// Description: AnnaRaymondStoryTask header
////////////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_ANNARAYMONDSTORYTASK_H  // makes sure this header is not included more than once
#define INCLUDED_ANNARAYMONDSTORYTASK_H

#include "ApplicationBase.h"
#include "WavePlayer.h"
#include <qwidget.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <qevent.h>
#include "qgraphicsitem.h"
#include <vector>
#include <regex>
#include <qlabel.h>
#include "myStimulus.h"
#include <qgridlayout.h>

class AnnaRaymondStoryTask : public ApplicationBase
{
 public:
  AnnaRaymondStoryTask();
  ~AnnaRaymondStoryTask();
  void Publish() override;
  void Preflight( const SignalProperties& Input, SignalProperties& Output ) const override;
  void Initialize( const SignalProperties& Input, const SignalProperties& Output ) override;
  void StartRun() override;
  void Process( const GenericSignal& Input, GenericSignal& Output ) override;
  void StopRun() override;
  void Halt() override;

 private:
	enum keyPressValue {
		key_space,
		key_enter,
		key_q,
		key_r,
		key_none,
	};
	QWidget* myWindow = new QWidget;
	unsigned int mBlockSize;
	std::map<int, std::vector<MyStimulus*> > stimulusMap;
	std::vector<int> mSequence;
	std::vector<int>::const_iterator mSequencePos;
	//SetOfMyStimuli mStimuli;
	int mBlocksInPhase;

	//void checkAudioPlayerErr();
	keyPressValue checkKeyPress();
	std::map<std::string, std::string> initStimulusMatrix();
	void checkAudioStimulusEnd();
	QString transColor(std::string scolor);
};

#endif // INCLUDED_ANNARAYMONDSTORYTASK_H
