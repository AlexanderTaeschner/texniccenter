#pragma once

namespace RunTimeHelper
{
	/// Gets a value indicating whether the running operating system
	/// is Windows Vista or higher.
	/// 
	/// \return \c true if Windows Vista or higher is running, \c false otherwise.
	inline bool IsVista()
	{
		OSVERSIONINFO ovi = {sizeof(OSVERSIONINFO)};
		BOOL bRet = ::GetVersionEx(&ovi);
		return ((bRet != FALSE) && (ovi.dwMajorVersion >= 6));
	}

	/// Gets a value indicating whether the running operating system
	/// is Window XP or higher.
	/// 
	/// \return \c true if Windows XP or higher is running, \c false otherwise.
	inline bool IsWindowsXPOrHigher()
	{
		OSVERSIONINFO ovi = {sizeof(OSVERSIONINFO)};
		BOOL bRet = ::GetVersionEx(&ovi);
		return ((bRet != FALSE) && (ovi.dwMajorVersion == 5 &&
			ovi.dwMinorVersion >= 1 || ovi.dwMajorVersion > 5));
	}
}