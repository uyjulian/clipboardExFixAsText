/////////////////////////////////////////////
//                                         //
//    Copyright (C) 2023-2023 Julian Uy    //
//  https://sites.google.com/site/awertyb  //
//                                         //
//   See details of license at "LICENSE"   //
//                                         //
/////////////////////////////////////////////

#include "ncbind/ncbind.hpp"

/* EmptyClipboard calls are added to empty the clipboard */
/* This avoids the issue where if there is an image on clipboard, */
/* text won't get pasted if the application prefers images */
static void TVPClipboardSetTextFix(const ttstr & text)
{
	if( ::OpenClipboard(0) ) {
		HGLOBAL ansihandle = NULL;
		HGLOBAL unicodehandle = NULL;
		::EmptyClipboard();
		try {
			// store ANSI string
			int ansistrlen = (int)((text.GetNarrowStrLen() + 1)*sizeof(char));
			ansihandle = ::GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, ansistrlen);
			if( !ansihandle ) TVPThrowExceptionMessage( TJS_W("copying to clipboard failed.") );

			char *mem = (char*)::GlobalLock(ansihandle);
			if(mem) text.ToNarrowStr(mem, ansistrlen);
			::GlobalUnlock(ansihandle);

			::SetClipboardData( CF_TEXT, ansihandle );
			ansihandle = NULL;

			// store UNICODE string
			unicodehandle = ::GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, (text.GetLen() + 1) * sizeof(tjs_char));
			if(!unicodehandle) TVPThrowExceptionMessage( TJS_W("copying to clipboard failed.") );

			tjs_char *unimem = (tjs_char*)::GlobalLock(unicodehandle);
			if(unimem) TJS_strcpy(unimem, text.c_str());
			::GlobalUnlock(unicodehandle);

			::SetClipboardData( CF_UNICODETEXT, unicodehandle );
			unicodehandle = NULL;
		} catch(...) {
			if(ansihandle) ::GlobalFree(ansihandle);
			if(unicodehandle) ::GlobalFree(unicodehandle);
			::EmptyClipboard();
			::CloseClipboard();
			throw;
		}
		::CloseClipboard();
	}
}

static bool TVPClipboardGetTextFix(ttstr & text)
{
	if(!::OpenClipboard(NULL)) return false;

	bool result = false;
	try
	{
		// select CF_UNICODETEXT or CF_TEXT
		UINT formats[2] = { CF_UNICODETEXT, CF_TEXT};
		int format = ::GetPriorityClipboardFormat(formats, 2);

		if(format == CF_UNICODETEXT)
		{
			// try to read unicode text
			HGLOBAL hglb = (HGLOBAL)::GetClipboardData(CF_UNICODETEXT);
			if(hglb != NULL)
			{
				const tjs_char *p = (const tjs_char *)::GlobalLock(hglb);
				if(p)
				{
					try
					{
						text = ttstr(p);
						result = true;
					}
					catch(...)
					{
						::GlobalUnlock(hglb);
						throw;
					}
					::GlobalUnlock(hglb);
				}
			}
		}
		else if(format == CF_TEXT)
		{
			// try to read ansi text
			HGLOBAL hglb = (HGLOBAL)::GetClipboardData(CF_TEXT);
			if(hglb != NULL)
			{
				const char *p = (const char *)::GlobalLock(hglb);
				if(p)
				{
					try
					{
						text = ttstr(p);
						result = true;
					}
					catch(...)
					{
						::GlobalUnlock(hglb);
						throw;
					}
					::GlobalUnlock(hglb);
				}
			}
		}
	}
	catch(...)
	{
		::CloseClipboard();
		throw;
	}
	::CloseClipboard();

	return result;
}

class ClipboardExFixAsText
{
public:

	static void setText(tTJSVariant data)
	{
		TVPClipboardSetTextFix(data);
	}

	static tTJSVariant getText(void)
	{
		tTJSVariant result;

		ttstr text;
		bool got = TVPClipboardGetTextFix(text);
		if(got)
			result = text;
		else
			result.Clear();
		return result;
	}

};

NCB_ATTACH_CLASS(ClipboardExFixAsText, Clipboard)
{
	NCB_PROPERTY(asText, getText, setText);
}
