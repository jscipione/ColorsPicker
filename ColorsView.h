/*
 * Copyright 2012-2024 John Scipione. All Rights Reserved.
 * Copyright 2009-2012 Haiku, Inc. All Rights Reserved.
 * Copyright 2001-2008 Werner Freytag.
 * Distributed under the terms of the MIT License.
 */
#ifndef COLORS_VIEW_H
#define COLORS_VIEW_H


#include <View.h>

#include "color_mode.h"


#define	MSG_RADIOBUTTON		'Rad0'
#define	MSG_TEXTCONTROL		'Txt0'
#define MSG_HEXTEXTCONTROL	'HTxt'


class ColorField;
class ColorSlider;
class ColorPreview;
class OutOfGamutSelector;
class WebSafeSelector;

class BRadioButton;
class BTextControl;


class ColorsView : public BView {
public:
								ColorsView();
	virtual						~ColorsView();

	virtual	void				AttachedToWindow();
	virtual	void				MessageReceived(BMessage* message);

	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 code,
									const BMessage* message);
	virtual	void				MouseUp(BPoint where);
	virtual	void				Pulse();

			rgb_color			Color();
			void				SetColor(rgb_color color);

			color_mode			ColorMode() const { return fColorMode; }
			void				SetColorMode(color_mode mode);

			void				SaveSettings();

private:
			void				_SetText(BTextControl* control,
									const char* text,
									bool* requiresUpdate);
			void				_UpdateColor(float value, float value1,
									float value2);
			void				_ForwardColorChangeToWindow(rgb_color);
			void				_UpdateTextControls();

			color_mode			fColorMode;

			float				fHue;
			float				fSat;
			float				fVal;

			float				fRed;
			float				fGreen;
			float				fBlue;

			float*				fPointer0;
			float*				fPointer1;
			float*				fPointer2;

			int32				fMouseButtons;
			BPoint				fMouseOffset;

			bool				fRequiresUpdate;
			bool				fInitialColorSet;

			ColorField*			fColorField;
			ColorSlider*		fColorSlider;
			ColorPreview*		fColorPreview;
			WebSafeSelector*	fWebSafeSelector;
			OutOfGamutSelector*	fOutOfGamutSelector;

			BRadioButton*		fRadioButton[6];
			BTextControl*		fTextControl[6];
			BTextControl*		fHexTextControl;
};

#endif	// COLORS_VIEW_H
