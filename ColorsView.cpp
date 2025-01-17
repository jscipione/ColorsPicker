/*
 * Copyright 2012-2024 John Scipione. All Rights Reserved.
 * Copyright 2009-2012 Haiku, Inc. All Rights Reserved.
 * Copyright 2001-2008 Werner Freytag. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 *
 * Original Author:
 *		Werner Freytag <freytag@gmx.de>
 *
 * Authors:
 *		Stephan Aßmus <superstippi@gmx.de>
 *		John Scipione <jscipione@gmail.com>
 */


#include "ColorsView.h"

#include <stdio.h>
#include <stdlib.h>

#include <Application.h>
#include <Bitmap.h>
#include <Beep.h>
#include <Font.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <Messenger.h>
#include <Message.h>
#include <RadioButton.h>
#include <Resources.h>
#include <PickerProtocol.h>
#include <Screen.h>
#include <Size.h>
#include <SpaceLayoutItem.h>
#include <StringView.h>
#include <TextControl.h>
#include <Window.h>

#include "ColorField.h"
#include "ColorsApplication.h"
#include "ColorPreview.h"
#include "ColorSlider.h"
#include "ColorsWindow.h"
#include "convert_rgb_hsv.h"
#include "OutOfGamutSelector.h"
#include "WebSafeSelector.h"


#define round(x) (int)(x+.5)
#define hexdec(str, offset) (int)(((str[offset]<60?str[offset]-48:(str[offset]|32)-87)<<4)|(str[offset+1]<60?str[offset+1]-48:(str[offset+1]|32)-87))


ColorsView::ColorsView()
	:
	BView("Colors!", B_WILL_DRAW | B_PULSE_NEEDED),
	fColorMode(S_SELECTED),
	fHue(0.0),
	fSat(1.0),
	fVal(1.0),
	fRed(1.0),
	fGreen(0.0),
	fBlue(0.0),
	fPointer0(NULL),
	fPointer1(NULL),
	fPointer2(NULL),
	fMouseButtons(0),
	fMouseOffset(B_ORIGIN),
	fRequiresUpdate(false),
	fInitialColorSet(false)
{
	SetViewUIColor(B_PANEL_BACKGROUND_COLOR);

	BMessage* settings = static_cast<ColorsApplication*>(be_app)->Settings();

	int32 int_color;
	if (settings->FindInt32("selected_color", &int_color) == B_OK) {
		fRed = (float)(int_color >> 16) / 255;
		fGreen = (float)((int_color >> 8) & 255) / 255;
		fBlue = (float)(int_color & 255) / 255;

		RGB_to_HSV(fRed, fGreen, fBlue, fHue, fSat, fVal);
	}

	fColorField = new ColorField(fColorMode, fSat);
	fColorSlider = new ColorSlider(fColorMode, fHue, fVal);
	fColorPreview = new ColorPreview();
	fOutOfGamutSelector = new OutOfGamutSelector();
	fWebSafeSelector = new WebSafeSelector();

	const char *title[] = { "H", "S", "V", "R", "G", "B" };

	for (int32 i = 0; i < 6; ++i) {
		fRadioButton[i] = new BRadioButton(NULL, title[i], new BMessage(MSG_RADIOBUTTON + i));
		fRadioButton[i]->SetExplicitMinSize(BSize(39.0f, B_SIZE_UNSET));
		fRadioButton[i]->SetExplicitMaxSize(BSize(39.0f, B_SIZE_UNSET));
		fRadioButton[i]->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));

		fTextControl[i] = new BTextControl(NULL, NULL, NULL,
			new BMessage(MSG_TEXTCONTROL + i));
	}

	int32 selectedMode;
	if (settings->FindInt32("selected_mode", &selectedMode) != B_OK) {
		// default to saturation mode
		selectedMode = 1;
	}
	fRadioButton[selectedMode]->SetValue(B_CONTROL_ON);
	switch (selectedMode) {
		case 0:
			SetColorMode(H_SELECTED);
			break;

		case 1:
			SetColorMode(S_SELECTED);
			break;

		case 2:
			SetColorMode(V_SELECTED);
			break;

		case 3:
			SetColorMode(R_SELECTED);
			break;

		case 4:
			SetColorMode(G_SELECTED);
			break;

		case 5:
			SetColorMode(B_SELECTED);
			break;
	}

	fHexTextControl = new BTextControl(NULL, "#", NULL, new BMessage(MSG_HEXTEXTCONTROL));

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.AddGlue()
		.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
			.Add(fColorField)
			.Add(fColorSlider)
			.AddGroup(B_VERTICAL, B_USE_SMALL_SPACING)
				.AddGroup(B_HORIZONTAL, B_USE_SMALL_SPACING)
					.AddGroup(B_VERTICAL, B_USE_SMALL_SPACING)
						.Add(fOutOfGamutSelector)
						.AddGlue()
						.Add(fWebSafeSelector)
					.End()
					.AddGroup(B_VERTICAL, 1)
						.Add(fColorPreview)
					.End()
				.End()
				.AddGrid(0.0f, 0.0f)
					.Add(fRadioButton[0], 0, 0)
					.Add(fTextControl[0], 1, 0)
					.Add(new BStringView("degree", "˚"), 2, 0)
					.Add(fRadioButton[1], 0, 1)
					.Add(fTextControl[1], 1, 1)
					.Add(new BStringView("percent", "%"), 2, 1)
					.Add(fRadioButton[2], 0, 2)
					.Add(fTextControl[2], 1, 2)
					.Add(new BStringView("percent", "%"), 2, 2)
					.Add(fRadioButton[3], 0, 3)
					.Add(fTextControl[3], 1, 3)
					.Add(fRadioButton[4], 0, 4)
					.Add(fTextControl[4], 1, 4)
					.Add(fRadioButton[5], 0, 5)
					.Add(fTextControl[5], 1, 5)
				.End()
				.Add(fHexTextControl)
			.End()
		.End()
		.AddGlue()
		.SetInsets(0);
}


ColorsView::~ColorsView()
{
}


void
ColorsView::AttachedToWindow()
{
	rgb_color selected_color = make_color(
		(int)(fRed * 255),
		(int)(fGreen * 255),
		(int)(fBlue * 255), 255
	);

	fColorField->SetMarkerToColor(selected_color);
	fColorField->SetTarget(this);

	fColorSlider->SetMarkerToColor(selected_color);
	fColorSlider->SetTarget(this);

	fColorPreview->SetNewColor(selected_color);
	fColorPreview->SetTarget(this);

	fOutOfGamutSelector->SetColor(selected_color);
	fOutOfGamutSelector->SetTarget(this);

	fWebSafeSelector->SetColor(selected_color);
	fWebSafeSelector->SetTarget(this);

	for (int32 i = 0; i < 6; ++i) {
		fRadioButton[i]->SetFontSize(9.0f);
		fRadioButton[i]->SetTarget(this);
		fTextControl[i]->SetTarget(this);

		BTextView* textView = fTextControl[i]->TextView();
		textView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_VERTICAL_CENTER));
		float textWidth = textView->StringWidth("99999");
		textView->SetExplicitSize(BSize(textWidth, B_SIZE_UNSET));
		// Only permit (max 3) decimal inputs
		textView->SetMaxBytes(3);
		for (int32 j = 32; j < 255; ++j) {
			if (j < '0' || j > '9')
				textView->DisallowChar(j);
		}
	}

	fHexTextControl->SetTarget(this);

	BTextView* hexTextView = fHexTextControl->TextView();
	float hexTextWidth = hexTextView->StringWidth("99999999");
	hexTextView->SetExplicitMaxSize(BSize(hexTextWidth, B_SIZE_UNSET));
	hexTextView->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT,
		B_ALIGN_VERTICAL_CENTER));
	// Only permit (max 6) hexidecimal inputs
	hexTextView->SetMaxBytes(6);
	for (int32 j = 32; j < 255; ++j) {
		if (!((j >= '0' && j <= '9') || (j >= 'a' && j <= 'f') || (j >= 'A' && j <= 'F')))
			hexTextView->DisallowChar(j);
	}

	_UpdateTextControls();
}


void
ColorsView::MessageReceived(BMessage* message)
{
	if (message->WasDropped()) {
		char* name;
		type_code type;
		rgb_color* color;
		ssize_t size;
		if (message->GetInfo(B_RGB_COLOR_TYPE, 0, &name, &type) == B_OK
			&& message->FindData(name, type, (const void**)&color, &size) == B_OK) {
			SetColor(*color);

			BMessenger sender;
			if (message->FindMessenger("be:sender", &sender) == B_OK
				&& sender != BMessenger(Window())) {
				Window()->PostMessage(message);
			}
		}
	}

	switch (message->what) {
		case MSG_COLOR_FIELD:
		{
			float value1;
			float value2;
			value1 = message->FindFloat("value");
			value2 = message->FindFloat("value", 1);
			_UpdateColor(-1, value1, value2);
			break;
		}

		case MSG_COLOR_SLIDER:
		{
			float value;
			message->FindFloat("value", &value);
			_UpdateColor(value, -1, -1);
			break;
		}

		case MSG_RADIOBUTTON:
			SetColorMode(H_SELECTED);
			break;

		case MSG_RADIOBUTTON + 1:
			SetColorMode(S_SELECTED);
			break;

		case MSG_RADIOBUTTON + 2:
			SetColorMode(V_SELECTED);
			break;

		case MSG_RADIOBUTTON + 3:
			SetColorMode(R_SELECTED);
			break;

		case MSG_RADIOBUTTON + 4:
			SetColorMode(G_SELECTED);
			break;

		case MSG_RADIOBUTTON + 5:
			SetColorMode(B_SELECTED);
			break;

		case MSG_TEXTCONTROL:
		case MSG_TEXTCONTROL + 1:
		case MSG_TEXTCONTROL + 2:
		case MSG_TEXTCONTROL + 3:
		case MSG_TEXTCONTROL + 4:
		case MSG_TEXTCONTROL + 5:
		{
			int nr = message->what - MSG_TEXTCONTROL;
			int value = atoi(fTextControl[nr]->Text());
			char string[4];

			switch (nr) {
				case 0:
				{
					value %= 360;
					sprintf(string, "%d", value);
					fHue = (float)value / 60;
					break;
				}

				case 1:
				{
					value = min_c(value, 100);
					sprintf(string, "%d", value);
					fSat = (float)value / 100;
					break;
				}

				case 2:
				{
					value = min_c(value, 100);
					sprintf(string, "%d", value);
					fVal = (float)value / 100;
					break;
				}

				case 3:
				{
					value = min_c(value, 255);
					sprintf(string, "%d", value);
					fRed = (float)value / 255;
					break;
				}

				case 4:
				{
					value = min_c(value, 255);
					sprintf(string, "%d", value);
					fGreen = (float)value / 255;
					break;
				}

				case 5:
				{
					value = min_c(value, 255);
					sprintf(string, "%d", value);
					fBlue = (float)value / 255;
					break;
				}
			}

			if (nr < 3) {
				// hsv-mode
				HSV_to_RGB(fHue, fSat, fVal, fRed, fGreen, fBlue);
			}

			rgb_color color = make_color(round(fRed * 255), round(fGreen * 255),
				round(fBlue * 255), 255);

			SetColor(color);
			_ForwardColorChangeToWindow(color);
			break;
		}

		case MSG_HEXTEXTCONTROL:
		{
			if (fHexTextControl->TextView()->TextLength() == 6) {
				const char* string = fHexTextControl->TextView()->Text();
				rgb_color color = { hexdec(string, 0), hexdec(string, 2),
					hexdec(string, 4), 255 };
				SetColor(color);
				_ForwardColorChangeToWindow(color);
			}
			break;
		}

		case MSG_COLOR_PREVIEW:
		case MSG_COLOR_SELECTOR:
		{
			char* name;
			type_code type;
			rgb_color* color;
			ssize_t size;
			if (message->GetInfo(B_RGB_COLOR_TYPE, 0, &name, &type) == B_OK
				&& message->FindData(name, type, (const void**)&color, &size) == B_OK) {
				color->alpha = 255;
				SetColor(*color);
				_ForwardColorChangeToWindow(*color);
			}
			break;
		}

		case B_VALUE_CHANGED:
		{
			char* name;
			type_code type;
			rgb_color* color;
			ssize_t size;
			if (message->GetInfo(B_RGB_COLOR_TYPE, 0, &name, &type) == B_OK
				&& message->FindData(name, type, (const void**)&color, &size) == B_OK) {
				SetColor(*color);

				// check if initial color
				if (!fInitialColorSet) {
					color->alpha = 255;
					fColorPreview->SetNewColor(*color);
					fInitialColorSet = true;
					break;
				}

				BMessenger sender;
				if (message->FindMessenger("be:sender", &sender) == B_OK
					&& sender != BMessenger(Window())) {
					Window()->PostMessage(message);
				}
			}
			break;
		}

		default:
			BView::MessageReceived(message);
	}
}


void
ColorsView::MouseDown(BPoint where)
{
	Window()->Activate();

	fMouseButtons = Window()->CurrentMessage()->GetInt32("buttons", 0);
	fMouseOffset = where;

	if (fMouseButtons == B_SECONDARY_MOUSE_BUTTON) {
		BPoint where;
		Window()->CurrentMessage()->FindPoint("where", &where);
		ConvertToScreen(&where);

		BMessage message(MSG_POPUP_MENU);
		message.AddPoint("where", where);
		Window()->PostMessage(&message);
	} else {
		fMouseButtons = B_PRIMARY_MOUSE_BUTTON;
		SetMouseEventMask(B_POINTER_EVENTS, B_NO_POINTER_HISTORY
			| B_SUSPEND_VIEW_FOCUS | B_LOCK_WINDOW_FOCUS);

		BView::MouseDown(where);
	}
}


void
ColorsView::MouseMoved(BPoint where, uint32 code, const BMessage *message)
{
	if (fMouseButtons == B_PRIMARY_MOUSE_BUTTON) {
		BPoint win_pos = Window()->Frame().LeftTop();
		Window()->MoveTo(win_pos.x + where.x - fMouseOffset.x,
			win_pos.y + where.y - fMouseOffset.y );
	} else
		BView::MouseMoved(where, code, message);
}


void
ColorsView::MouseUp(BPoint where)
{
	fMouseButtons = 0;

	BView::MouseUp(where);
}


void
ColorsView::Pulse()
{
	if (fRequiresUpdate)
		_UpdateTextControls();
}


// #pragma mark -


rgb_color
ColorsView::Color()
{
	if (fColorMode & (R_SELECTED | G_SELECTED | B_SELECTED))
		RGB_to_HSV(fRed, fGreen, fBlue, fHue, fSat, fVal);
	else
		HSV_to_RGB(fRed, fGreen, fBlue, fHue, fSat, fVal);

	rgb_color color;
	color.red = (uint8)round(fRed * 255.0);
	color.green = (uint8)round(fGreen * 255.0);
	color.blue = (uint8)round(fBlue * 255.0);
	color.alpha = 255;

	return color;
}


void
ColorsView::SetColor(rgb_color color)
{
	fRed = (float)color.red / 255;
	fGreen = (float)color.green / 255;
	fBlue = (float)color.blue / 255;
	RGB_to_HSV(fRed, fGreen, fBlue, fHue, fSat, fVal);

	fColorSlider->SetModeAndValues(fColorMode, *fPointer1, *fPointer2);
	fColorSlider->SetMarkerToColor(color);

	fColorField->SetModeAndValue(fColorMode, *fPointer0);
	fColorField->SetMarkerToColor(color);

	fColorPreview->SetColor(color);
	fOutOfGamutSelector->SetColor(color);
	fWebSafeSelector->SetColor(color);

	fRequiresUpdate = true;
}


void
ColorsView::SetColorMode(color_mode mode)
{
	fColorMode = mode;

	switch (mode) {
		case R_SELECTED:
			fPointer0 = &fRed;
			fPointer1 = &fGreen;
			fPointer2 = &fBlue;
			break;

		case G_SELECTED:
			fPointer0 = &fGreen;
			fPointer1 = &fRed;
			fPointer2 = &fBlue;
			break;

		case B_SELECTED:
			fPointer0 = &fBlue;
			fPointer1 = &fRed;
			fPointer2 = &fGreen;
			break;

		case H_SELECTED:
			fPointer0 = &fHue;
			fPointer1 = &fSat;
			fPointer2 = &fVal;
			break;

		case S_SELECTED:
			fPointer0 = &fSat;
			fPointer1 = &fHue;
			fPointer2 = &fVal;
			break;

		case V_SELECTED:
			fPointer0 = &fVal;
			fPointer1 = &fHue;
			fPointer2 = &fSat;
			break;
	}

	fColorSlider->SetModeAndValues(fColorMode, *fPointer1, *fPointer2);
	fColorField->SetModeAndValue(fColorMode, *fPointer0);
}


void
ColorsView::SaveSettings()
{
	BMessage* settings = static_cast<ColorsApplication*>(be_app)->Settings();

	settings->RemoveName("selected_color");
	int32 int_color = ((int)(fRed * 255) << 16)
		+ ((int)(fGreen * 255) << 8)
		+ ((int)(fBlue * 255));
	settings->AddInt32("selected_color", int_color);

	settings->RemoveName("selected_mode");
	for (int32 i = 0; i < 6; ++i) {
		if (fRadioButton[i]->Value() == B_CONTROL_ON) {
			settings->AddInt32("selected_mode", i);
			break;
		}
	}
}


// #pragma mark -


void
ColorsView::_UpdateColor(float value, float value1, float value2)
{
	if (value != -1) {
		fColorField->SetFixedValue(value);
		*fPointer0 = value;
	} else if (value1 != -1 && value2 != -1) {
		fColorSlider->SetOtherValues(value1, value2);
		*fPointer1 = value1; *fPointer2 = value2;
	}

	if (fColorMode & (R_SELECTED | G_SELECTED | B_SELECTED))
		RGB_to_HSV(fRed, fGreen, fBlue, fHue, fSat, fVal);
	else
		HSV_to_RGB(fHue, fSat, fVal, fRed, fGreen, fBlue);

	rgb_color color = make_color((int)(fRed * 255), (int)(fGreen * 255),
		(int)(fBlue * 255), 255);

	fColorPreview->SetColor(color);
	fOutOfGamutSelector->SetColor(color);
	fWebSafeSelector->SetColor(color);

	_ForwardColorChangeToWindow(color);

	fRequiresUpdate = true;
}


void
ColorsView::_ForwardColorChangeToWindow(rgb_color color)
{
	BMessage forward(B_VALUE_CHANGED);
	forward.AddData("be:value", B_RGB_COLOR_TYPE, &color, sizeof(color));
	forward.AddMessenger("be:sender", BMessenger(this));
	forward.AddPointer("source", this);
	forward.AddInt64("when", (int64)system_time());
	Window()->PostMessage(&forward);
}


void
ColorsView::_UpdateTextControls()
{
	Window()->DisableUpdates();

	char string[5];

	sprintf(string, "%ld", lround(fHue * 60));
	fTextControl[0]->TextView()->SetText(string);

	sprintf(string, "%ld", lround(fSat * 100));
	fTextControl[1]->TextView()->SetText(string);

	sprintf(string, "%ld", lround(fVal * 100));
	fTextControl[2]->TextView()->SetText(string);

	sprintf(string, "%ld", lround(fRed * 255));
	fTextControl[3]->TextView()->SetText(string);

	sprintf(string, "%ld", lround(fGreen * 255));
	fTextControl[4]->TextView()->SetText(string);

	sprintf(string, "%ld", lround(fBlue * 255));
	fTextControl[5]->TextView()->SetText(string);

	sprintf(string, "%.6X", (lround(fRed * 255) << 16)
		| (lround(fGreen * 255) << 8)
		| (lround(fBlue * 255)));
	fHexTextControl->TextView()->SetText(string);

	Window()->EnableUpdates();

	fRequiresUpdate = false;
}
