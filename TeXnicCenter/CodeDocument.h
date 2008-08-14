#pragma once

int ToScintillaMode(int m);
int FromScintillaMode(int m);

class SpellerBackgroundThread;
class CodeBookmark;

// CodeDocument document

class CodeDocument : public CScintillaDoc
{
	DECLARE_DYNAMIC(CodeDocument)

	int error_marker_handle_;

public:
	enum Encoding
	{
		ANSI,
		UTF8,
		UTF16LE,
		UTF16BE,
		UTF32LE,
		UTF32BE
	};

	enum Marker
	{
		Errormark,
		Bookmark
	};

private:
	Encoding encoding_;

	UINT code_page_;
	bool create_backup_file_;

	SpellerBackgroundThread* speller_thread_;

	Encoding DetectEncoding(const BYTE* data, SIZE_T& pos, SIZE_T size);
	static void ConvertToMultiByte(LPCWSTR input, int cch, std::vector<BYTE>& buffer, Encoding encoding, UINT cp);

protected:
	CodeDocument();
	virtual ~CodeDocument();

public:
#ifndef _WIN32_WCE
	void Serialize(CArchive& ar);   // overridden for document i/o
#endif
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	void SetModifiedFlag(BOOL modified = TRUE);

	virtual DWORD LoadFile(HANDLE file);
	DWORD LoadFile(LPCTSTR pszFileName);
	DWORD SaveFile(LPCTSTR pszFileName, bool bClearModifiedFlag = true);
	virtual DWORD SaveFile(HANDLE file);	
	DWORD SaveFile(HANDLE file, LPCWSTR text, int length);
	Encoding GetEncoding() const;

	UINT GetCodePage() const;
	void SetSpellerThread(SpellerBackgroundThread* t);
	SpellerBackgroundThread* GetSpellerThread() const;
	bool IsSpellerThreadAttached() const;
	void SetErrorMark(int line);
	bool HasErrorMark() const;

protected:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	afx_msg void OnEditToggleBookmark();

	bool ShowSaveDialog(LPCTSTR file) const;
	DECLARE_MESSAGE_MAP()	
	virtual void OnBookmarkAdded(const ::CodeBookmark& b);
	virtual void OnBookmarkRemoved(const ::CodeBookmark& b);
	virtual void OnRemovedAllBookmarks(void);
	afx_msg void OnEditClearAllBookmarks();

public:
	void RemoveAllBookmarks();
	void ToggleBookmark( int line );

	const BookmarkContainerType GetBookmarks();
	template<class I>
	void SetBookmarks(I first, I last);
};

template<class I>
inline void CodeDocument::SetBookmarks(I first, I last)
{
	RemoveAllBookmarks();

	for ( ; first != last; ++first)
		GetView()->GetCtrl().MarkerAdd(first->GetLine(),Bookmark);
}