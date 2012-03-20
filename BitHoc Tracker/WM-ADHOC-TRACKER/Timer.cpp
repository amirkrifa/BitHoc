#include "StdAfx.h"
#include "Timer.h"


// Constructeur
Timer::Timer(){}
// Destructeur
Timer::~Timer(){}

// Initialisation du Timer
void Timer::InitTimer(const HWND pHandle, const TIMERPROC ProcAd, const int TimerID)
{

	// Handle de la fen�tre parente
	_Handle = pHandle;
	// Identificateur du Timer
	_TimerID = TimerID;
	// Adresse de la proc�dure qui va recevoir le message WM_TIMER
	_ProcAd = ProcAd;

}

// D�marre le Timer
bool Timer::StartTimer()
{

	// Cr�e le Timer et le d�marre
	return (SetTimer(_Handle, _TimerID, (UINT)_Interval, _ProcAd) == TRUE);

}

// Stop le Timer
bool Timer::StopTimer()
{

	// D�truit le Timer
	return (KillTimer(_Handle, _TimerID) == TRUE);

}

// Modifier l'interval du Timer
void Timer::Interval(const int Value)
{

	// Si le nouvelle interval(Value) est diff�rent de l'ancien(_Interval)
	if (Value != _Interval)
	{

		// Sauvegarde le nouvelle
		_Interval = Value;

		// Stop(D�truit) le Timer
		StopTimer();
		// Recr�e le Timer avec le nouvelle Interval
		StartTimer();

	}

}

// Conna�tre l'interval du Timer
int Timer::Interval()
{

	return _Interval;

}

