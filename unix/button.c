// 10 june 2015
#include "uipriv_unix.h"

struct uiButton {
	uiUnixControl c;
	GtkWidget *widget;
	GtkButton *button;
	void (*onClicked)(uiButton *, void *);
	void *onClickedData;
};

uiUnixDefineControl(
	uiButton,								// type name
	uiButtonType							// type function
)

static void onClicked(GtkButton *button, gpointer data)
{
	uiButton *b = uiButton(data);

	(*(b->onClicked))(b, b->onClickedData);
}

static void defaultOnClicked(uiButton *b, void *data)
{
	// do nothing
}

char *uiButtonText(uiButton *b)
{
	return uiUnixStrdupText(gtk_button_get_label(b->button));
}

void uiButtonSetText(uiButton *b, const char *text)
{
	gtk_button_set_label(b->button, text);
	// changing the text might necessitate a change in the button's size
	uiControlQueueResize(uiControl(b));
}

void uiButtonOnClicked(uiButton *b, void (*f)(uiButton *, void *), void *data)
{
	b->onClicked = f;
	b->onClickedData = data;
}

uiButton *uiNewButton(const char *text)
{
	uiButton *b;

	b = (uiButton *) uiNewControl(uiButtonType());

	b->widget = gtk_button_new_with_label(text);
	b->button = GTK_BUTTON(b->widget);

	g_signal_connect(b->widget, "clicked", G_CALLBACK(onClicked), b);
	uiButtonOnClicked(b, defaultOnClicked, NULL);

	uiUnixFinishNewControl(b, uiButton);

	return b;
}
