// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Chris Roberts

#include "SettingsWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <Resources.h>
#include <Roster.h>
#include <SeparatorView.h>
#include <StringView.h>
#include <TextControl.h>


enum {
	kFileTypesMessage = 'fyle',
	kHelpMessage = 'halp',
	kProjectMessage = 'prdg'
};

const char* kProjectUrl = "https://github.com/augiedoggie/NowOpen";
const char* kHelpUrl = "https://github.com/augiedoggie/NowOpen/wiki";

const char* kOpenTip = "When opening supported files by double clicking them or using the 'Open with...' menu.";

const char* kRegularTip = "When the launcher itself is double clicked.";
const char* kRegularWarning = "You will no longer see this window if you set this command.";

const char* kFileTypesTip = "Edit application signature, supported file types, and icon.";
const char* kFileTypesWarning = "Signature must be unique for each new launcher.";


SettingsWindow::SettingsWindow(BRect frame)
	:
	BWindow(frame, "Launcher Settings", B_TITLED_WINDOW, B_CLOSE_ON_ESCAPE | B_AUTO_UPDATE_SIZE_LIMITS | B_QUIT_ON_WINDOW_CLOSE | B_ASYNCHRONOUS_CONTROLS)
{
	BStringView* fileTypesWarningView = new BStringView("FileTypesWarningStringView", kFileTypesWarning);
	BFont font;
	fileTypesWarningView->GetFont(&font);
	font.SetFace(B_BOLD_FACE);
	fileTypesWarningView->SetFont(&font);
	fileTypesWarningView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BStringView* regularWarningView = new BStringView("RegularWarningStringView", kRegularWarning);
	regularWarningView->SetFont(&font);
	regularWarningView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BStringView* extraStringView = new BStringView("ExtraStringView", "Extra Attributes:");
	extraStringView->SetAlignment(B_ALIGN_RIGHT);
	extraStringView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BStringView* fileTypesStringView = new BStringView("FileTypesStringView", kFileTypesTip);
	fileTypesStringView->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	BButton* quitButton = new BButton("Quit", new BMessage(B_QUIT_REQUESTED));

	// clang-format off
	BLayoutBuilder::Group<>(this, B_VERTICAL, B_USE_WINDOW_SPACING)
		.SetInsets(B_USE_WINDOW_INSETS)
		.AddGrid(B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
			.AddTextControl(fCommandControl = new BTextControl("File Open Command:", "", NULL), 0, 0, B_ALIGN_RIGHT)
			.Add(BSpaceLayoutItem::CreateVerticalStrut(5), 0, 2, 2)
			.AddTextControl(fRegularControl = new BTextControl("Regular Launch Command:", "", NULL), 0, 3, B_ALIGN_RIGHT)
			.AddGroup(B_VERTICAL, 0.0, 1, 4)
				.Add(regularWarningView)
			.End()
			.Add(BSpaceLayoutItem::CreateVerticalStrut(5), 0, 5, 2)
			.AddGroup(B_HORIZONTAL, 0.0, 0, 6)
				.AddGlue()
				.Add(extraStringView)
				.AddStrut(5.0)
			.End()
			.AddGroup(B_HORIZONTAL, 5.0, 1, 6)
				.Add(new BButton("Open FileTypes", new BMessage(kFileTypesMessage)))
				.AddGlue()
			.End()
			.AddGroup(B_VERTICAL, 0.0, 1, 7)
				.Add(fileTypesStringView)
				.Add(fileTypesWarningView)
			.End()
		.End()
		.AddGlue(10.0)
		.Add(new BSeparatorView(B_HORIZONTAL))
		.AddGroup(B_HORIZONTAL)
			.Add(new BButton("Online Help", new BMessage(kHelpMessage)))
			.Add(new BButton("Github Project", new BMessage(kProjectMessage)))
			.AddGlue()
			.Add(quitButton)
		.End()
	.End();
	// clang-format on

	fCommandControl->SetToolTip(kOpenTip);
	fCommandControl->TextView()->SetExplicitMinSize(BSize(400, B_SIZE_UNSET));
	fCommandControl->TextView()->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNSET));

	fRegularControl->SetToolTip(kRegularTip);
	fRegularControl->TextView()->SetExplicitMinSize(BSize(400, B_SIZE_UNSET));

	InvalidateLayout();

	quitButton->MakeDefault(true);

	ResizeToPreferred();

	_LoadSettings();
}


void
SettingsWindow::MessageReceived(BMessage *message)
{
	switch(message->what) {
		case kFileTypesMessage:
			if (_LaunchFileTypes() != B_OK)
				(new BAlert("ERROR", "Error launching FileTypes!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
			break;
		case kHelpMessage:
		{
			const char* args[] = { kHelpUrl, NULL };
			status_t rc = be_roster->Launch("application/x-vnd.Be.URL.https", 1, args);
			if (rc != B_OK && rc != B_ALREADY_RUNNING)
				(new BAlert("Error", "Failed to launch URL", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
		}
			break;
		case kProjectMessage:
		{
			const char* args[] = { kProjectUrl, NULL };
			status_t rc = be_roster->Launch("application/x-vnd.Be.URL.https", 1, args);
			if (rc != B_OK && rc != B_ALREADY_RUNNING)
				(new BAlert("Error", "Failed to launch URL", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
		}
			break;
		default:
			BWindow::MessageReceived(message);
	}
}


bool
SettingsWindow::QuitRequested() {
	_SaveSettings();
	return BWindow::QuitRequested();
}


status_t
SettingsWindow::_LaunchFileTypes()
{
	app_info info;
	if (be_app->GetAppInfo(&info) != B_OK)
		return B_ERROR;

	BMessage message(B_REFS_RECEIVED);
	message.AddRef("refs", &info.ref);

	return be_roster->Launch("application/x-vnd.Haiku-FileTypes", &message);
}


status_t
SettingsWindow::_LoadSettings()
{
	// check for BFS attribute first
	app_info info;
	if (be_app->GetAppInfo(&info) != B_OK)
		return B_ERROR;

	BNode node(&info.ref);
	if (node.InitCheck() != B_OK)
		return B_ERROR;

	BString attrString;
	if (node.ReadAttrString(kCommandKey, &attrString) == B_OK) {
		fCommandControl->SetText(attrString);
		return B_OK;
	}

	// fall back to compiled in resources
	BResources* resources = be_app->AppResources();
	BString commandString(static_cast<const char*>(resources->LoadResource(B_STRING_TYPE, kCommandKey, NULL)));
	if (commandString.IsEmpty()) {
		(new BAlert("ERROR", "Error loading command from application resources!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
		return B_ERROR;
	}
	fCommandControl->SetText(commandString);
	return B_OK;
}


status_t
SettingsWindow::_SaveSettings()
{
	//TODO only write an attribute if the command has changed

	app_info info;
	if (be_app->GetAppInfo(&info) != B_OK)
		return B_ERROR;

	BNode node(&info.ref);
	if (node.InitCheck() != B_OK)
		return B_ERROR;

	BString commandString(fCommandControl->Text());
	// if commandString is empty then remove the attribute and use the resource
	if (commandString.IsEmpty()) {
		if (node.GetAttrInfo(kCommandKey, NULL) != B_OK)
			return B_OK;

		return node.RemoveAttr(kCommandKey);
	}

	if (node.WriteAttrString(kCommandKey, &commandString) != B_OK)
		return B_ERROR;

	// check if we should create a "finished" launcher
	commandString = fRegularControl->Text();
	if (commandString.IsEmpty())
		return B_OK;

	return node.WriteAttrString(kFinishedKey, &commandString);
}
