/*
    StreamModule is a P2P and conference communication module base on CGCP.
    Copyright (C) 2009-2010  Akee Yang <akee.yang@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// dlldefine.h file here

#define libAVSCLIENT_STATIC 1	// 0,1

#ifndef LIBAVSCLIENT_CLASS
#if libAVSCLIENT_STATIC
#define LIBAVSCLIENT_CLASS 
#else
#ifdef WIN32
#ifdef libAVSCLIENT_EXPORTS
#define LIBAVSCLIENT_CLASS __declspec(dllexport)
#else
#define LIBAVSCLIENT_CLASS __declspec(dllimport)
#endif // libDS_EXPORTS
#else
#define LIBAVSCLIENT_CLASS 
#endif // WIN32
#endif // libAVSCLIENT_STATIC
#endif // LIBAVSCLIENT_CLASS
