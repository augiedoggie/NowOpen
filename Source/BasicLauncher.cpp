// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: 2023 Chris Roberts

#include "SettingsWindow.h"

#include <Alert.h>
#include <AppFileInfo.h>
#include <Application.h>
#include <Entry.h>
#include <File.h>
#include <Path.h>
#include <PathFinder.h>
#include <Resources.h>
#include <Roster.h>
#include <String.h>


class LauncherApp : public BApplication {
public:
	LauncherApp(const char* signature)
		:
		BApplication(signature),
		fRefsHandled(false)
	{}

	virtual void
	RefsReceived(BMessage* message)
	{
		entry_ref ref;

		fRefsHandled = true;

		BString commandString;

		// check if our command has been stored as a BFS attribute
		app_info info;
		if (be_app->GetAppInfo(&info) == B_OK) {
			BNode node(&info.ref);
			if (node.InitCheck() == B_OK)
				node.ReadAttrString(kCommandKey, &commandString);
		}

		// fall back to compiled in resources for the command
		if (commandString.IsEmpty()) {
			BResources* resources = be_app->AppResources();
			if (!resources->HasResource(B_STRING_TYPE, kCommandKey)) {
				(new BAlert("ERROR", "Error locating command in application resources!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
				return;
			}

			commandString = static_cast<const char*>(resources->LoadResource(B_STRING_TYPE, kCommandKey, NULL));
			if (commandString.IsEmpty()) {
				(new BAlert("ERROR", "Error loading command from application resources!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
				return;
			}
		}

		BString quotedFiles;
		bool multiLaunch = commandString.FindFirst("%file%") >= 0;
		for (int32 index = 0; message->FindRef("refs", index, &ref) == B_OK; index++) {
			BPath file(&ref);
			BString quotedFile(file.Path());
			// escape single quotes in the string
			quotedFile.ReplaceAll("'", "'\\''");

			if (multiLaunch) {
				// multi launch with string replacement
				BString realCommand(commandString);
				// surround it in single quotes
				quotedFile.Prepend('\'', 1);
				quotedFile.Append('\'', 1);
				realCommand.ReplaceAll("%file%", quotedFile);
				if (system(realCommand.String()) != 0) {
					(new BAlert("ERROR", "Error executing command!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
					return;
				}
			} else {
				// single launch with one large string of paths
				if (!quotedFiles.EndsWith(" "))
					quotedFiles << " ";

				quotedFiles << "'" << quotedFile << "'";
			}
		}

		if (multiLaunch)
			return;

		if (commandString.FindFirst("%files%") >= 0)
			commandString.ReplaceAll("%files%", quotedFiles);
		else
			commandString << " " << quotedFiles << " &";

		if (system(commandString.String()) != 0)
			(new BAlert("ERROR", "Error executing command!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();
	}

	virtual void
	ReadyToRun()
	{
		// if we were called via 'Open with...' or double clicking a file then we're finished
		if (fRefsHandled) {
			be_app->PostMessage(B_QUIT_REQUESTED);
			return;
		}

		BString commandString;

		// check if we are a "finished" launcher
		app_info info;
		if (be_app->GetAppInfo(&info) == B_OK) {
			BNode node(&info.ref);
			if (node.InitCheck() == B_OK)
				node.ReadAttrString(kFinishedKey, &commandString);
		}

		if (!commandString.IsEmpty()) {
			// make sure our command forks to the background
			if (!commandString.EndsWith("&"))
				commandString << " &";

			if (system(commandString.String()) != 0)
				(new BAlert("ERROR", "Error executing command!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();

			be_app->PostMessage(B_QUIT_REQUESTED);
			return;
		}

		// create a settings window if we're unfinished and haven't handled any files
		SettingsWindow* window = new SettingsWindow(BRect(200, 200, 500, 500));
		window->Lock();
		window->CenterOnScreen();
		window->Show();
		window->Unlock();
	}

private:
	bool		fRefsHandled;
};


int
main(int /*argc*/, char** /*argv*/)
{
	image_info info;
	int32 cookie = 0;

	// lookup our app signature from resources
	if (get_next_image_info(B_CURRENT_TEAM, &cookie, &info) == B_OK) {
		BFile imageFile(info.name, O_RDONLY);
		if (imageFile.InitCheck() == B_OK) {
			BAppFileInfo appInfo(&imageFile);
			if (appInfo.InitCheck() == B_OK) {
				char signature[B_MIME_TYPE_LENGTH];
				if (appInfo.GetSignature(signature) == B_OK) {
					LauncherApp app(signature);
					app.Run();
					return 0;
				}
			}
		}
	}

	(new BAlert("ERROR", "Error getting application signature from resources!", "Ok", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT))->Go();

	return 1;
}
