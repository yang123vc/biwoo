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

// libP2PAVDef.h file here
#ifndef __libP2PAVDef_h__
#define __libP2PAVDef_h__

namespace P2PAVConnect
{
	enum ConnectType_
	{
		CT_Audio = 1
		, CT_Video = 2
		, CT_Both = 3
	};
};

typedef P2PAVConnect::ConnectType_  P2PAVConnectType;


#endif // __libP2PAVDef_h__
