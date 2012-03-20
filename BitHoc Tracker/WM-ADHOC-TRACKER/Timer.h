/*
Copyright (C) 2008  Amir Krifa
*/
/*!
* \file Timer.h
* \author Amir Krifa, krifa_amir@hotmail.com
* \version 1.0 
*/

#pragma once

#ifndef TIMER_HEADER
#define TIMER_HEADER

#include <windows.h>

class Timer
{

	private:

		// Handle de la fenêtre parente
		HWND _Handle;
		// Identificateur du Timer
		UINT _TimerID;
		// Interval du Timer
		int _Interval;
		// Adresse de la procedure qui va recevoir le message WM_TIMER
		TIMERPROC _ProcAd;

	public:

		// Constructeur
		Timer();
		// Destructeur
		~Timer();

		// Initialisation du Timer
		void InitTimer(const HWND pHandle, const TIMERPROC ProcAd, const int TimerID);

		// Démarre le Timer
		bool StartTimer();
		// Stop le Timer
		bool StopTimer();

		// Changer l'interval
		void Interval(const int Value);
		// Connaître l'interval
		int Interval();

};

#endif

