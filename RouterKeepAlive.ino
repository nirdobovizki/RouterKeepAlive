#include <Timer.h>
#include <StateMachine.h>
#include <State.h>
#include <NetworkDetect.h>
#include <DigitalWrite.h>
#include <DigitalRead.h>
#include <AnalogWrite.h>


State stateStart;
State stateOff;
State stateDetecting;
State stateNoInternet;
State stateReset;
State stateWaitAfterReset;
State stateAfterReset;
State stateGiveUp;

DigitalRead inOffSwitch;
DigitalWrite outLedRed;
DigitalWrite outLedGreen;
AnalogWrite outLedBlue;
DigitalWrite outRelay;
NetworkDetect inNetworkDetect;
Timer timeNetworkDetect;
Timer timeRecheckTimer;
Timer timeAfterReset;

void setup()
{
	Serial.begin(9600);

	inOffSwitch.Init(D2);
	outLedRed.Init(D3, LOW);
	outLedGreen.Init(D4, LOW);
	outLedBlue.Init(D5);
	outRelay.Init(D6, LOW);
	inNetworkDetect.Init("dobovizki");
	timeNetworkDetect.Init(120000);
	timeRecheckTimer.Init(60000);
	timeAfterReset.Init(60000);

	StateMachine.GlobalActivity(&inOffSwitch, DigitalRead::StartSampling);

	stateStart.Activity(&outLedGreen, DigitalWrite::TurnOn);
	stateStart.Activity(&timeNetworkDetect, Timer::CallOnce);
	stateStart.Transition(&inOffSwitch, DigitalRead::OnHigh, &stateOff);
	stateStart.Transition(&timeNetworkDetect, Timer::Tick, &stateDetecting);

	stateOff.Activity(&outLedBlue, AnalogWrite::SlowSweep);
	stateOff.Transition(&inOffSwitch, DigitalRead::OnLow, &stateStart);

	stateDetecting.Activity(&outLedGreen, DigitalWrite::SlowBlink);
	stateDetecting.Activity(&inNetworkDetect, NetworkDetect::DetectNow);
	stateDetecting.Transition(&inNetworkDetect, NetworkDetect::Found, &stateStart);
	stateDetecting.Transition(&inNetworkDetect, NetworkDetect::NotFound, &stateNoInternet);
	stateDetecting.Transition(&inOffSwitch, DigitalRead::OnHigh, &stateOff);

	stateNoInternet.Activity(&outLedRed, DigitalWrite::FastBlink);
	stateNoInternet.Activity(&timeRecheckTimer, Timer::CallOnce);
	stateNoInternet.Transition(&inOffSwitch, DigitalRead::OnHigh, &stateOff);
	stateNoInternet.Transition(&timeRecheckTimer, Timer::Tick, &stateReset);

	stateReset.Activity(&outRelay, DigitalWrite::SlowPulse);
	stateReset.Transition(&outRelay, DigitalWrite::EndPulse, &stateWaitAfterReset);

	stateWaitAfterReset.Activity(&outLedRed, DigitalWrite::SlowBlink);
	stateWaitAfterReset.Activity(&timeAfterReset, Timer::CallOnce);
	stateWaitAfterReset.Transition(&inOffSwitch, DigitalRead::OnHigh, &stateOff);
	stateWaitAfterReset.Transition(&timeAfterReset, Timer::Tick, &stateAfterReset);

	stateAfterReset.Activity(&outLedGreen, DigitalWrite::FastBlink);
	stateAfterReset.Activity(&inNetworkDetect, NetworkDetect::DetectNow);
	stateAfterReset.Transition(&inNetworkDetect, NetworkDetect::Found, &stateStart);
	stateAfterReset.Transition(&inNetworkDetect, NetworkDetect::NotFound, &stateGiveUp);
	stateAfterReset.Transition(&inOffSwitch, DigitalRead::OnHigh, &stateOff);

	stateGiveUp.Activity(&outLedRed, DigitalWrite::TurnOn);
	stateGiveUp.Transition(&inOffSwitch, DigitalRead::OnHigh, &stateOff);

	StateMachine.Start(&stateStart);
}

void loop()
{
	StateMachine.Loop();
}
