/*
 * Copyright 2009-2012 Haiku, Inc. All Rights Reserved.
 * Copyright 2001-2008 Werner Freytag.
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *		Werner Freytag <freytag@gmx.de>
 * Authors:
 *		John Scipione <jscipione@gmail.com>
 */


#include "ColorsApplication.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <Alert.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <Path.h>
#include <PickerProtocol.h>

#include "ColorsView.h"
#include "ColorsWindow.h"


static const char* kSettingsFileName = "Colors!_settings";
static const char* kAppSig = "application/vnd.de.pecora-Colors!";


ColorsApplication::ColorsApplication()
	:
	BApplication(kAppSig),
	fColorsWindow(NULL)
{
}


ColorsApplication::~ColorsApplication()
{
}


void
ColorsApplication::AboutRequested()
{
	(new BAlert("About", "Colors! 3.0\n\n"
		"©2009-2023 John Scipione.\n"
		"©2001-2008 Werner Freytag\n"
		"Colors! icon by meanwhile\n\n"
		"History\n"
		"    3.0 (May 9, 2023) Integrate Colors! as a system color picker. \n"
		"    2.9 (Sep 7, 2021) Make Colors! into a modular color picker.\n"
		"    2.3 (Feb 23, 2013) Remove the ForeBackSelector control and "
		"replace it with more color containers.\n"
		"    2.2 (Dec 1, 2012) Added web-safe selector control. Degree and % "
		"symbols return.\n"
		"    2.1 (Aug 29, 2012) Updated look of controls, slider now updates "
		"live.\n"
		"    2.0 (Apr 30, 2012) Layout with the Haiku layout APIs, update "
		"the icon, add more color containers, update the interface, "
		"improve the fore back color container control.\n"
		"    1.6 (Sep 11, 2001) Added the eye-dropper to pick a color from "
		"anywhere on the desktop. Added a popup menu with some useful window "
		"specific settings.\n"
		"    1.5.1 (Sep 5, 2001) Fixed some minor bugs, improved the text "
		"color container\n"
		"    1.5 (Sep 3, 2001) Added color containers, removed some minor "
		"bugs, improved speed.\n"
		"    1.0 (Aug 3, 2001) Initial release. Updates to come!",
		"Close"))->Go();
}


void
ColorsApplication::MessageReceived(BMessage* message)
{
	if (message->what == B_PICKER_INITIATE_CONNECTION) {
		// This is the initial open message that ModuleProxy::Invoke
		// is sending us. Pass it on to the new color picker dialog
		// where all the details will be found.
		fColorsWindow = new ColorsWindow(new ColorsView(), message);
	}

	BApplication::MessageReceived(message);
}


void
ColorsApplication::ReadyToRun()
{
	_LoadSettings();

	if (fColorsWindow != NULL)
		fColorsWindow->Show();
	else {
		// create a window if run directly
		BWindow* window = new BWindow(BRect(100, 100, 100, 100),
			"Colors!", B_TITLED_WINDOW, B_NOT_ZOOMABLE
				| B_NOT_RESIZABLE | B_QUIT_ON_WINDOW_CLOSE
				| B_AUTO_UPDATE_SIZE_LIMITS);

		BLayoutBuilder::Group<>(window, B_VERTICAL, 0)
			.Add(new ColorsView())
			.End();
		window->Show();
	}
}


bool
ColorsApplication::QuitRequested()
{
	_SaveSettings();

	return BApplication::QuitRequested();
}


// #pragma mark -


void
ColorsApplication::_LoadSettings()
{
	// locate preferences file
	BFile prefsFile;
	if (_InitSettingsFile(&prefsFile, false) < B_OK) {
		printf("no preference file found.\n");
		return;
	}

	// unflatten settings data
	if (fSettings.Unflatten(&prefsFile) < B_OK) {
		printf("error unflattening settings.\n");
	}
}


void
ColorsApplication::_SaveSettings()
{
	// flatten entire archive and write to settings file
	BFile prefsFile;
	status_t ret = _InitSettingsFile(&prefsFile, true);
	if (ret < B_OK) {
		fprintf(stderr, "ColorsApplication::_SaveSettings() - "
			"error creating file: %s\n", strerror(ret));
		return;
	}

	ret = fSettings.Flatten(&prefsFile);
	if (ret < B_OK) {
		fprintf(stderr, "ColorsApplication::_SaveSettings() - error flattening "
			"to file: %s\n", strerror(ret));
		return;
	}
}


status_t
ColorsApplication::_InitSettingsFile(BFile* file, bool write)
{
	// find user settings directory
	BPath prefsPath;
	status_t result = find_directory(B_USER_SETTINGS_DIRECTORY, &prefsPath);
	if (result != B_OK)
		return result;

	BPath filePath(prefsPath);
	result = filePath.Append(kSettingsFileName);
	if (result != B_OK)
		return result;

	if (write) {
		result = file->SetTo(filePath.Path(),
			B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
	} else
		result = file->SetTo(filePath.Path(), B_READ_ONLY);

	return result;
}


extern "C" BColorPickerPanel*
instantiate_color_picker(BView* view, BMessage* message,
	BColorPickerPanel::color_cell_layout layout, const char* name,
	window_look look, window_feel feel, uint32 flags, uint32 workspace)
{
	return new ColorsWindow((ColorsView*)view, message);
}


int
main()
{
	ColorsApplication* app = new ColorsApplication();
	app->Run();

	return 0;
}
