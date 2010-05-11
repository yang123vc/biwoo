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

#define libP2PAV_STATIC 0	// 0,1

#ifndef LIBP2PAV_CLASS
#if libP2PAV_STATIC
#define LIBP2PAV_CLASS 
#else
#ifdef WIN32
#ifdef libP2P_EXPORTS
#define LIBP2PAV_CLASS __declspec(dllexport)
#else
#define LIBP2PAV_CLASS __declspec(dllimport)
#endif // libDS_EXPORTS
#else
#define LIBP2PAV_CLASS 
#endif // WIN32
#endif // libP2PAV_STATIC
#endif // LIBP2PAV_CLASS
