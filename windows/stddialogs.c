// 22 may 2015
#include "uipriv_windows.h"

// notes:
// - FOS_SUPPORTSTREAMABLEITEMS doesn't seem to be supported on windows vista, or at least not with the flags we use
// - even with FOS_NOVALIDATE the dialogs will reject invalid filenames (at least on Vista, anyway)

// TODO
// - http://blogs.msdn.com/b/wpfsdk/archive/2006/10/26/uncommon-dialogs--font-chooser-and-color-picker-dialogs.aspx
// - when a dialog is active, tab navigation in other windows stops working
// - when adding uiOpenFolder(), use IFileDialog as well - https://msdn.microsoft.com/en-us/library/windows/desktop/bb762115%28v=vs.85%29.aspx

#define windowHWND(w) ((HWND) uiControlHandle(uiControl(w)))

char *commonItemDialog(HWND parent, REFCLSID clsid, REFIID iid, FILEOPENDIALOGOPTIONS optsadd)
{
	IFileDialog *d;
	FILEOPENDIALOGOPTIONS opts;
	IShellItem *result;
	WCHAR *wname;
	char *name;
	HRESULT hr;

	hr = CoCreateInstance(clsid,
		NULL, CLSCTX_INPROC_SERVER,
		iid, (LPVOID *) (&d));
	if (hr != S_OK)
		logHRESULT("error creating common item dialog in commonItemDialog()", hr);
	hr = IFileDialog_GetOptions(d, &opts);
	if (hr != S_OK)
		logHRESULT("error getting current options in commonItemDialog()", hr);
	opts |= optsadd;
	hr = IFileDialog_SetOptions(d, opts);
	if (hr != S_OK)
		logHRESULT("error setting options in commonItemDialog()", hr);
	hr = IFileDialog_Show(d, parent);
	if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		IFileDialog_Release(d);
		return NULL;
	}
	if (hr != S_OK)
		logHRESULT("error showing dialog in commonItemDialog()", hr);
	hr = IFileDialog_GetResult(d, &result);
	if (hr != S_OK)
		logHRESULT("error getting dialog result in commonItemDialog()", hr);
	hr = IShellItem_GetDisplayName(result, SIGDN_FILESYSPATH, &wname);
	if (hr != S_OK)
		logHRESULT("error getting filename in commonItemDialog()", hr);
	name = toUTF8(wname);
	CoTaskMemFree(wname);
	IShellItem_Release(result);
	IFileDialog_Release(d);
	return name;
}

char *uiOpenFile(uiWindow *parent)
{
	return commonItemDialog(windowHWND(parent),
		&CLSID_FileOpenDialog, &IID_IFileOpenDialog,
		FOS_NOCHANGEDIR | FOS_ALLNONSTORAGEITEMS | FOS_NOVALIDATE | FOS_PATHMUSTEXIST | FOS_FILEMUSTEXIST | FOS_SHAREAWARE | FOS_NOTESTFILECREATE | FOS_NODEREFERENCELINKS | FOS_FORCESHOWHIDDEN | FOS_DEFAULTNOMINIMODE);
}

char *uiSaveFile(uiWindow *parent)
{
	return commonItemDialog(windowHWND(parent),
		&CLSID_FileSaveDialog, &IID_IFileSaveDialog,
		// TODO strip FOS_NOREADONLYRETURN?
		FOS_OVERWRITEPROMPT | FOS_NOCHANGEDIR | FOS_ALLNONSTORAGEITEMS | FOS_NOVALIDATE | FOS_SHAREAWARE | FOS_NOTESTFILECREATE | FOS_NODEREFERENCELINKS | FOS_FORCESHOWHIDDEN | FOS_DEFAULTNOMINIMODE);
}

// TODO switch to TaskDialogIndirect()?

static void msgbox(HWND parent, const char *title, const char *description, TASKDIALOG_COMMON_BUTTON_FLAGS buttons, PCWSTR icon)
{
	WCHAR *wtitle, *wdescription;
	HRESULT hr;

	wtitle = toUTF16(title);
	wdescription = toUTF16(description);

	hr = TaskDialog(parent, NULL, NULL, wtitle, wdescription, buttons, icon, NULL);
	if (hr != S_OK)
		logHRESULT("error showing task dialog in msgbox()", hr);

	uiFree(wdescription);
	uiFree(wtitle);
}

void uiMsgBox(uiWindow *parent, const char *title, const char *description)
{
	msgbox(windowHWND(parent), title, description, TDCBF_OK_BUTTON, NULL);
}

void uiMsgBoxError(uiWindow *parent, const char *title, const char *description)
{
	msgbox(windowHWND(parent), title, description, TDCBF_OK_BUTTON, TD_ERROR_ICON);
}
