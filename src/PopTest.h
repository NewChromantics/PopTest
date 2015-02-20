#pragma once
#include <ofxSoylent.h>
#include <SoyApp.h>
#include <TJob.h>
#include <TChannel.h>




class TPopTest : public TJobHandler, public TChannelManager
{
public:
	TPopTest();
	
	virtual void	AddChannel(std::shared_ptr<TChannel> Channel) override;

	void			OnExit(TJobAndChannel& JobAndChannel);
	void			OnEcho(TJobAndChannel& JobAndChannel);
	
public:
	Soy::Platform::TConsoleApp	mConsoleApp;
};



