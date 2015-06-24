#include "PopTest.h"
#include <TParameters.h>
#include <SoyDebug.h>
#include <TProtocolCli.h>
#include <TProtocolHttp.h>
#include <SoyApp.h>
#include <PopMain.h>
#include <TJobRelay.h>
#include <SoyPixels.h>
#include <SoyString.h>
#include <TFeatureBinRing.h>
#include <SortArray.h>
#include <TChannelLiteral.h>





TPopTest::TPopTest() :
	TJobHandler	( static_cast<TChannelManager&>(*this), 10 )
{
	AddJobHandler("exit", TParameterTraits(), *this, &TPopTest::OnExit );

	TParameterTraits EchoTraits;
	EchoTraits.mAssumedKeys.PushBack( TJobParam::Param_Default );
	AddJobHandler("echo", EchoTraits, *this, &TPopTest::OnEcho );
}

bool TPopTest::AddChannel(std::shared_ptr<TChannel> Channel)
{
	if ( !TChannelManager::AddChannel( Channel ) )
		return false;
	TJobHandler::BindToChannel( *Channel );
	return true;
}


void TPopTest::OnExit(TJobAndChannel& JobAndChannel)
{
	mConsoleApp.Exit();
	
	//	should probably still send a reply
	TJobReply Reply( JobAndChannel );
	Reply.mParams.AddDefaultParam(std::string("exiting..."));
	TChannel& Channel = JobAndChannel;
	Channel.OnJobCompleted( Reply );
}



void TPopTest::OnEcho(TJobAndChannel& JobAndChannel)
{
	auto& Job = JobAndChannel.GetJob();
	auto DefaultParam = Job.mParams.GetDefaultParam();

	//	take the defaultparam and send it right back
	TJobReply Reply( JobAndChannel );
	Reply.mParams.AddParam( DefaultParam );
	
	TChannel& Channel = JobAndChannel;
	Channel.OnJobCompleted( Reply );
}



//	horrible global for lambda
std::shared_ptr<TChannel> gStdioChannel;


TPopAppError::Type PopMain(TJobParams& Params)
{
	TPopTest App;

	auto CommandLineChannel = std::shared_ptr<TChan<TChannelLiteral,TProtocolCli>>( new TChan<TChannelLiteral,TProtocolCli>( SoyRef("cmdline") ) );
	
	//	create stdio channel for commandline output
	gStdioChannel = CreateChannelFromInputString("std:", SoyRef("stdio") );
	
	App.AddChannel( CommandLineChannel );
	App.AddChannel( gStdioChannel );

	//	when the commandline SENDs a command (a reply), send it to stdout
	auto RelayFunc = [](TJobAndChannel& JobAndChannel)
	{
		if ( !gStdioChannel )
			return;
		TJob Job = JobAndChannel;
		Job.mChannelMeta.mChannelRef = gStdioChannel->GetChannelRef();
		Job.mChannelMeta.mClientRef = SoyRef();
		gStdioChannel->SendCommand( Job );
	};
	CommandLineChannel->mOnJobSent.AddListener( RelayFunc );
	
	//	run
	App.mConsoleApp.WaitForExit();

	gStdioChannel.reset();
	return TPopAppError::Success;
}




