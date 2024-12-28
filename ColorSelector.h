/*
 * Copyright 2012-2023 John Scipione. All Rights Reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef COLOR_SELECTOR_H
#define COLOR_SELECTOR_H


#include <Invoker.h>
#include <View.h>


#define MSG_COLOR_SELECTOR 'CSel'

#define INDICATOR_RECT BRect(8.0, 0.0, 20.0, 12.0)
#define COLOR_RECT BRect(4.0, 16.0, 22.0, 34.0)


class ColorSelector : public BView, public BInvoker {
public:
								ColorSelector();
	virtual						~ColorSelector();

	virtual	void				Draw(BRect updateRect);
	virtual	status_t			Invoke(BMessage* mesesage = NULL);
	virtual	void				MouseDown(BPoint where);
	virtual	void				MouseMoved(BPoint where, uint32 code,
									const BMessage* message);
	virtual	void				MouseUp(BPoint where);

	virtual	void				Hide();
	virtual void				Show();
			bool				IsHidden();

	virtual	rgb_color			Color() const;
	virtual	void				SetColor(rgb_color color);

protected:
			rgb_color			fColor;
			bool				fIsHidden : 1;
			bool				fMouseOver : 1;
};

#endif	// COLOR_SELECTOR_H
