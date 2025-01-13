/*
 * Copyright 2012-2024 John Scipione. All Rights Reserved.
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


#include "ColorPreview.h"

#include <algorithm>
#include <iostream>

#include <stdio.h>

#include <Alignment.h>
#include <Bitmap.h>
#include <Message.h>
#include <MessageRunner.h>
#include <PickerProtocol.h>
#include <Size.h>
#include <String.h>
#include <Window.h>


ColorPreview::ColorPreview()
	:
	BControl("color preview", "", new BMessage(MSG_COLOR_PREVIEW), B_WILL_DRAW),
	fMessageRunner(NULL)
{
	SetExplicitMinSize(BSize(56, 50));
	SetExplicitMaxSize(BSize(56, 50));
	SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_VERTICAL_CENTER));
}


ColorPreview::~ColorPreview()
{
}


void
ColorPreview::Draw(BRect updateRect)
{
	rgb_color bg = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color outer = tint_color(bg, B_DARKEN_1_TINT);
	rgb_color inner = tint_color(bg, B_DARKEN_3_TINT);
	rgb_color light = tint_color(bg, B_LIGHTEN_MAX_TINT);

	BRect bounds(Bounds());

	BeginLineArray(4);
	AddLine(BPoint(bounds.left, bounds.bottom),
		BPoint(bounds.left, bounds.top), outer);
	AddLine(BPoint(bounds.left + 1, bounds.top),
		BPoint(bounds.right, bounds.top), outer);
	AddLine(BPoint(bounds.right, bounds.top + 1),
		BPoint(bounds.right, bounds.bottom), light);
	AddLine(BPoint(bounds.right - 1, bounds.bottom),
		BPoint(bounds.left + 1, bounds.bottom), light);
	EndLineArray();
	bounds.InsetBy(1, 1);

	BeginLineArray(4);
	AddLine(BPoint(bounds.left, bounds.bottom),
		BPoint(bounds.left, bounds.top), inner);
	AddLine(BPoint(bounds.left + 1, bounds.top),
		BPoint(bounds.right, bounds.top), inner);
	AddLine(BPoint(bounds.right, bounds.top + 1),
		BPoint(bounds.right, bounds.bottom), bg);
	AddLine(BPoint(bounds.right - 1, bounds.bottom),
		BPoint(bounds.left + 1, bounds.bottom), bg);
	EndLineArray();
	bounds.InsetBy(1, 1);

	bounds.bottom = bounds.top + bounds.Height() / 2.0;
	SetHighColor(fColor);
	FillRect(bounds);

	bounds.top = bounds.bottom + 1;
	bounds.bottom = Bounds().bottom - 2;
	SetHighColor(fOldColor);
	FillRect(bounds);
}


status_t
ColorPreview::Invoke(BMessage* message)
{
	if (message == NULL)
		message = new BMessage(B_PASTE);

	message->AddData("RGBColor", B_RGB_COLOR_TYPE, &fColor, sizeof(fColor));

	return BControl::Invoke(message);
}


void
ColorPreview::MessageReceived(BMessage* message)
{
	if (message->WasDropped()) {
		char* name;
		type_code type;
		rgb_color* color;
		ssize_t size;
		if (message->GetInfo(B_RGB_COLOR_TYPE, 0, &name, &type) == B_OK
			&& message->FindData(name, type, (const void**)&color, &size)
				== B_OK) {
			BPoint where;
			bool droppedOnNewArea = false;
			if (message->FindPoint("_drop_point_", &where) == B_OK) {
				ConvertFromScreen(&where);
				if (where.y > Bounds().top + Bounds().Height() / 2)
					droppedOnNewArea = true;
			}

			if (droppedOnNewArea)
				SetNewColor(*color);
			else
				SetColor(*color);

			Invoke();
		}
	}

	switch (message->what) {
		case MSG_MESSAGERUNNER:
		{
			BPoint where;
			uint32 buttons;
			GetMouse(&where, &buttons);

			_DragColor(where);
			break;
		}

		default:
			BControl::MessageReceived(message);
			break;
	}
}


void
ColorPreview::MouseDown(BPoint where)
{
	Window()->Activate();

	fMessageRunner = new BMessageRunner(this, new BMessage(MSG_MESSAGERUNNER),
		300000, 1);

	SetMouseEventMask(B_POINTER_EVENTS,
		B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);

	BRect oldRect = Bounds().InsetByCopy(2, 2);
	oldRect.top = oldRect.bottom / 2.0 + 1;

	if (oldRect.Contains(where)) {
		fColor = fOldColor;
		Invalidate();
		Invoke();
	}

	BControl::MouseDown(where);
}


void
ColorPreview::MouseMoved(BPoint where, uint32 code, const BMessage* dragMessage)
{
	if (fMessageRunner != NULL)
		_DragColor(where);

	BControl::MouseMoved(where, code, dragMessage);
}


void
ColorPreview::MouseUp(BPoint where)
{
	delete fMessageRunner;
	fMessageRunner = NULL;

	BControl::MouseUp(where);
}


void
ColorPreview::SetColor(rgb_color color)
{
	color.alpha = 255;
	fColor = color;

	Invalidate();
}


void
ColorPreview::SetNewColor(rgb_color color)
{
	color.alpha = 255;
	fColor = color;
	fOldColor = color;

	Invalidate();
}


// #pragma mark -


void
ColorPreview::_DragColor(BPoint where)
{
	BString hexStr;
	hexStr.SetToFormat("#%.2X%.2X%.2X", fColor.red, fColor.green, fColor.blue);

	BMessage message(B_PASTE);
	message.AddData("text/plain", B_MIME_TYPE, hexStr.String(),
		hexStr.Length());
	message.AddData("RGBColor", B_RGB_COLOR_TYPE, &fColor, sizeof(fColor));

	BRect rect(0, 0, 16, 16);

	BBitmap* bitmap = new BBitmap(rect, B_RGB32, true);
	bitmap->Lock();

	BView* view = new BView(rect, "", B_FOLLOW_NONE, B_WILL_DRAW);
	bitmap->AddChild(view);

	view->SetHighColor(B_TRANSPARENT_COLOR);
	view->FillRect(view->Bounds());

	++rect.top;
	++rect.left;

	view->SetHighColor(0, 0, 0, 100);
	view->FillRect(rect);
	rect.OffsetBy(-1, -1);

	int32 red = std::min(255, (int)(1.2 * fColor.red + 40));
	int32 green = std::min(255, (int)(1.2 * fColor.green + 40));
	int32 blue = std::min(255, (int)(1.2 * fColor.blue + 40));

	view->SetHighColor(red, green, blue);
	view->StrokeRect(rect);

	++rect.left;
	++rect.top;

	red = (int32)(0.8 * fColor.red);
	green = (int32)(0.8 * fColor.green);
	blue = (int32)(0.8 * fColor.blue);

	view->SetHighColor(red, green, blue);
	view->StrokeRect(rect);

	--rect.right;
	--rect.bottom;

	view->SetHighColor(fColor.red, fColor.green, fColor.blue);
	view->FillRect(rect);
	view->Sync();

	bitmap->Unlock();

	DragMessage(&message, bitmap, B_OP_ALPHA, BPoint(14, 14));

	MouseUp(where);
}
