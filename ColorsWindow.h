/*
 * Copyright 2013-2024 John Scipione. All Rights Reserved.
 * Copyright 2009-2013 Haiku, Inc. All Rights Reserved.
 * Copyright 2001-2008 Werner Freytag.
 * Distributed under the terms of the MIT License.
 */
#ifndef _COLORS_WINDOW_H
#define _COLORS_WINDOW_H


#include <ColorPickerPanel.h>


#define	MSG_POPUP_MENU		'PopU'
#define	MSG_AUTOBORDERLESS	'AutB'


class ColorsView;

class ColorsWindow : public BColorPickerPanel {
public:
							ColorsWindow(ColorsView* view, BMessage* message);
	virtual					~ColorsWindow();

	virtual	void			MessageReceived(BMessage* message);

private:
			ColorsView*		fColorsView;
};


#endif	// _COLORS_WINDOW_H
