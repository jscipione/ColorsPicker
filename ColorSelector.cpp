/*
 * Copyright 2012-2023 John Scipione. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		John Scipione <jscipione@gmail.com>
 */


#include "ColorSelector.h"

#include <InterfaceDefs.h>
#include <PickerProtocol.h>
#include <Point.h>
#include <Size.h>
#include <Window.h>


ColorSelector::ColorSelector()
	:
	BView("ColorSelector", B_WILL_DRAW),
	fIsHidden(false),
	fMouseOver(false)
{
	SetExplicitSize(BSize(COLOR_RECT.right + COLOR_RECT.left * 2,
		COLOR_RECT.Height() + 16));
}


ColorSelector::~ColorSelector()
{
}


void
ColorSelector::Draw(BRect updateRect)
{
	if (fIsHidden) {
		SetHighColor(ViewColor());
		FillRect(updateRect);
		return;
	}

	if (updateRect.Intersects(COLOR_RECT)) {
		rgb_color background = ui_color(B_PANEL_BACKGROUND_COLOR);
		rgb_color shadow = tint_color(background, B_DARKEN_1_TINT);
		rgb_color darkShadow = tint_color(background, B_DARKEN_3_TINT);
		rgb_color light = tint_color(background, B_LIGHTEN_MAX_TINT);

		BRect bounds = COLOR_RECT;

		BeginLineArray(4);
		AddLine(BPoint(bounds.left, bounds.bottom),
			BPoint(bounds.left, bounds.top), shadow);
		AddLine(BPoint(bounds.left + 1.0, bounds.top),
			BPoint(bounds.right, bounds.top), shadow);
		AddLine(BPoint(bounds.right, bounds.top + 1.0),
			BPoint(bounds.right, bounds.bottom), light);
		AddLine(BPoint(bounds.right - 1.0, bounds.bottom),
			BPoint(bounds.left + 1.0, bounds.bottom), light);
		EndLineArray();
		bounds.InsetBy(1.0, 1.0);

		BeginLineArray(4);
		AddLine(BPoint(bounds.left, bounds.bottom),
			BPoint(bounds.left, bounds.top), darkShadow);
		AddLine(BPoint(bounds.left + 1.0, bounds.top),
			BPoint(bounds.right, bounds.top), darkShadow);
		AddLine(BPoint(bounds.right, bounds.top + 1.0),
			BPoint(bounds.right, bounds.bottom), background);
		AddLine(BPoint(bounds.right - 1.0, bounds.bottom),
			BPoint(bounds.left + 1.0, bounds.bottom), background);
		EndLineArray();
		bounds.InsetBy(1.0, 1.0);

		SetHighColor(fColor);
		FillRect(bounds);
	}
}


status_t
ColorSelector::Invoke(BMessage* message)
{
	if (message == NULL)
		message = Message();

	message->AddData("be:value", B_RGB_COLOR_TYPE,
		&fColor, sizeof(fColor));
	message->AddInt64("when", (int64)system_time());
	message->AddPointer("source", (void*)Parent());
	message->AddMessenger("be:sender", BMessenger(Parent()));

	return BInvoker::Invoke(message);
}


void
ColorSelector::MouseDown(BPoint where)
{
	if (fIsHidden)
		return;

	BMessage message(MSG_COLOR_SELECTOR);
	ColorSelector::Invoke(&message);

	BView::MouseDown(where);
}


void
ColorSelector::MouseMoved(BPoint where, uint32 code, const BMessage* message)
{
	if (fIsHidden)
		return;

	switch (code) {
		case B_ENTERED_VIEW:
		case B_INSIDE_VIEW:
			fMouseOver = true;
			break;

		case B_EXITED_VIEW:
		case B_OUTSIDE_VIEW:
			fMouseOver = false;
			break;
	}

	Invalidate(INDICATOR_RECT);
}


void
ColorSelector::MouseUp(BPoint where)
{
	BView::MouseUp(where);
}



void
ColorSelector::Hide()
{
	fIsHidden = true;
	fMouseOver = false;
		// so that indicator won't be colored when shown
	Invalidate();
}


void
ColorSelector::Show()
{
	fIsHidden = false;
	Invalidate();
}


bool
ColorSelector::IsHidden()
{
	return fIsHidden;
}


rgb_color
ColorSelector::Color() const
{
	return fColor;
}


void
ColorSelector::SetColor(rgb_color color)
{
	color.alpha = 255;
	fColor = color;
	if (Window() && !IsHidden())
		Draw(COLOR_RECT);
}
