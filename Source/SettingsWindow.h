// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Chris Roberts

#ifndef _SETTINGSWINDOW_H_
#define _SETTINGSWINDOW_H_

#include <Window.h>


static const char* kCommandKey = "LaunchCommand";
static const char* kFinishedKey = "FinishedCommand";


class BTextControl;


class SettingsWindow : public BWindow {
public:
					SettingsWindow(BRect frame);
	virtual void	MessageReceived(BMessage* message);
	virtual bool	QuitRequested();

private:
	status_t		_LoadSettings();
	status_t		_SaveSettings();
	status_t		_LaunchFileTypes();

	BTextControl*	fCommandControl;
	BTextControl*	fRegularControl;
};

#endif // _SETTINGSWINDOW_H_
