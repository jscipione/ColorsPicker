/*
 * Copyright 2013-2024 John Scipione. All Rights Reserved.
 * Copyright 2009-2012 Haiku, Inc. All Rights Reserved.
 * Copyright 2001-2008 Werner Freytag.
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *		Werner Freytag <freytag@gmx.de>
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 *		John Scipione <jscipione@gmail.com>
 */


#include "ColorsWindow.h"

#include <Alert.h>
#include <Application.h>
#include <GroupLayout.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>

#include "ColorsApplication.h"
#include "ColorsView.h"


ColorsWindow::ColorsWindow(ColorsView* view, BMessage* message)
	:
	BColorPickerPanel((BView*)view, message, BColorPickerPanel::B_CELLS_2x20, "Colors!"),
	fColorsView(view)
{
	BMessage* settings = static_cast<ColorsApplication*>(be_app)->Settings();

	bool windowFloating;
	if (settings->FindBool("window_floating", &windowFloating) == B_OK && windowFloating)
		SetFeel(B_FLOATING_ALL_WINDOW_FEEL);
	else
		SetFeel(B_NORMAL_WINDOW_FEEL);

	bool windowAcceptsFirstClick;
	if (settings->FindBool("window_accept_first_click", &windowAcceptsFirstClick) == B_OK
		&& windowAcceptsFirstClick) {
		SetFlags(Flags() | B_WILL_ACCEPT_FIRST_CLICK);
	} else {
		SetFlags(Flags() & ~B_WILL_ACCEPT_FIRST_CLICK);
	}

	bool windowAllWorkspaces;
	if (settings->FindBool("window_all_workspaces", &windowAllWorkspaces) == B_OK
		&& windowAllWorkspaces) {
		SetWorkspaces(B_ALL_WORKSPACES);
	} else {
		SetWorkspaces(B_CURRENT_WORKSPACE);
	}
}


ColorsWindow::~ColorsWindow()
{
	fColorsView->SaveSettings();

	BMessage* settings = static_cast<ColorsApplication*>(be_app)->Settings();

	settings->RemoveName("window_floating");
	settings->AddBool("window_floating", Feel() == B_FLOATING_ALL_WINDOW_FEEL);

	settings->RemoveName("window_accept_first_click");
	settings->AddBool("window_accept_first_click", Flags() & B_WILL_ACCEPT_FIRST_CLICK);

	settings->RemoveName("window_all_workspaces");
	settings->AddBool("window_all_workspaces", Workspaces() == B_ALL_WORKSPACES);
}


void
ColorsWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case MSG_POPUP_MENU:
		{
			BPoint where;
			if (message->FindPoint("where", &where) != B_OK)
				return;

			BPopUpMenu* menu = new BPopUpMenu("", false, false);
			BMenuItem* item[5];

			item[0] = new BMenuItem("Always on top", NULL);
			item[0]->SetMarked(Feel() == B_FLOATING_ALL_WINDOW_FEEL);
			menu->AddItem(item[0]);

			item[1] = new BMenuItem("Accept first mouse click", NULL);
			item[1]->SetMarked(Flags() & B_WILL_ACCEPT_FIRST_CLICK);
			menu->AddItem(item[1]);

			item[2] = new BMenuItem("All workspaces", NULL);
			item[2]->SetMarked(Workspaces() == B_ALL_WORKSPACES);
			menu->AddItem(item[2]);

			item[3] = new BSeparatorItem();
			menu->AddItem(item[3]);

			item[4] = new BMenuItem("About Colors!", NULL);
			menu->AddItem(item[4]);

			menu->ResizeToPreferred();

			BMenuItem* selectedItem = menu->Go(where);

			if (selectedItem == item[0]) {
				bool floating = Feel() == B_FLOATING_ALL_WINDOW_FEEL;
				SetFeel(floating ? B_NORMAL_WINDOW_FEEL : B_FLOATING_ALL_WINDOW_FEEL);
			} else if (selectedItem == item[1]) {
				SetFlags(Flags() ^ B_WILL_ACCEPT_FIRST_CLICK);
			} else if (selectedItem == item[2]) {
				bool all = Workspaces() == B_ALL_WORKSPACES;
				SetWorkspaces(all ? B_CURRENT_WORKSPACE : B_ALL_WORKSPACES);
			} else if (selectedItem == item[4]) {
				static_cast<ColorsApplication*>(be_app)->AboutRequested();
			}

			delete menu;
			break;
		}

		default:
			BColorPickerPanel::MessageReceived(message);
			break;
	}
}
