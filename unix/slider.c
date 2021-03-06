// 11 june 2015
#include "uipriv_unix.h"

struct uiSlider {
	uiUnixControl c;
	GtkWidget *widget;
	GtkRange *range;
	GtkScale *scale;
	void (*onChanged)(uiSlider *, void *);
	void *onChangedData;
	gulong onChangedSignal;
};

uiUnixDefineControl(
	uiSlider,								// type name
	uiSliderType							// type function
)

static void onChanged(GtkRange *range, gpointer data)
{
	uiSlider *s = uiSlider(data);

	(*(s->onChanged))(s, s->onChangedData);
}

static void defaultOnChanged(uiSlider *s, void *data)
{
	// do nothing
}

intmax_t uiSliderValue(uiSlider *s)
{
	return (intmax_t) gtk_range_get_value(s->range);
}

void uiSliderSetValue(uiSlider *s, intmax_t value)
{
	// we need to inhibit sending of ::value-changed because this WILL send a ::value-changed otherwise
	g_signal_handler_block(s->range, s->onChangedSignal);
	gtk_range_set_value(s->range, value);
	g_signal_handler_unblock(s->range, s->onChangedSignal);
}

void uiSliderOnChanged(uiSlider *s, void (*f)(uiSlider *, void *), void *data)
{
	s->onChanged = f;
	s->onChangedData = data;
}

uiSlider *uiNewSlider(intmax_t min, intmax_t max)
{
	uiSlider *s;

	s = (uiSlider *) uiNewControl(uiSliderType());

	s->widget = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, min, max, 1);
	s->range = GTK_RANGE(s->widget);
	s->scale = GTK_SCALE(s->widget);

	// TODO needed?
	gtk_scale_set_digits(s->scale, 0);

	s->onChangedSignal = g_signal_connect(s->scale, "value-changed", G_CALLBACK(onChanged), s);
	uiSliderOnChanged(s, defaultOnChanged, NULL);

	uiUnixFinishNewControl(s, uiSlider);

	return s;
}
