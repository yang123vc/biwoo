/*
    Biwoo is an enterprise communication software base on CGCP.
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

// messageinfo.h file here
#ifndef __messageinfo_h__
#define __messageinfo_h__

#include <ThirdParty/stl/stldef.h>
#include <ThirdParty/stl/lockmap.h>
#include <boost/shared_ptr.hpp>
#include <CGCBase/cgcobject.h>

class CMessageInfo
	: public cgc::cgcObject
{
public:
	typedef boost::shared_ptr<CMessageInfo> pointer;
	static CMessageInfo::pointer create(long mid, unsigned long total)
	{
		return CMessageInfo::pointer(new CMessageInfo(mid, total, 0, false));
	}
	static CMessageInfo::pointer create(long mid, unsigned long total, short type, bool newflag)
	{
		return CMessageInfo::pointer(new CMessageInfo(mid, total, type, newflag));
	}

	CMessageInfo(long mid, unsigned long total, short type, bool newflag)
		: m_messageid(mid), m_total(total)
		, m_type(type), m_newflag(newflag), m_time(0)
		, m_indexCount(0), m_filesize(0)
		, m_width(0), m_height(0)
		, m_counter(0)
	{
		if (m_total > 0)
		{
			m_buffer = new char[m_total+1];
			memset(m_buffer, 0, m_total+1);
		}else
		{
			m_buffer = NULL;
		}
	}
	~CMessageInfo(void)
	{
		clear();
	}

	int icounter(void) {return ++m_counter;}
	void resetcounter(void) {m_counter = 0;}

	void indexCount(size_t newv) {m_indexCount = newv;}
	size_t indexCount(void) const {return m_indexCount;}
	CLockMap<unsigned long, bool>	m_indexs;

public:
	void messageid(long newv) {m_messageid = newv;}
	long messageid(void) const {return m_messageid;}
	unsigned long total(void) const {return m_total;}

	void imageWH(size_t width, size_t height) {m_width = width; m_height = height;}
	void imageWidth(size_t newv) {m_width = newv;}
	size_t imageWidth(void) const {return m_width;}
	void imageHeight(size_t newv) {m_height = newv;}
	size_t imageHeight(void) const {return m_height;}
	void filesize(size_t newv) {m_filesize = newv;}
	size_t filesize(void) const {return m_filesize;}

	void resetdata(const char * data, unsigned long size)
	{
		clear();
		m_total = size;
		m_buffer = new char[m_total+1];
		memcpy(m_buffer, data, m_total);
		m_buffer[m_total] = '\0';
	}

	void setdata(const char * data, unsigned long size, long index)
	{
		if (m_buffer != NULL)
		{
			if (index + size > m_total)
				size = m_total - index;
			if (size > 0)
				memcpy(m_buffer+index, data, size);
		}
	}
	void setdata(char * data, unsigned long size)
	{
		clear();
		m_buffer = data;
		m_total = size;
	}
	const char * getdata(unsigned long & outsize) const
	{
		outsize = m_total;
		return m_buffer;
	}
	const char * getdata(void) const
	{
		return m_buffer;
	}
	std::string tostring(void) const {return (m_buffer != NULL && m_total > 0) ? std::string(m_buffer, m_total) : "";}


	void type(short newv) {m_type = newv;}
	short type(void) const {return m_type;}
	void newflag(bool newv) {m_newflag = newv;}
	bool newflag(void) const {return m_newflag;}

	void msgtime(time_t newv) {m_time = newv;}
	time_t msgtime(void) const {return m_time;}

private:
	void clear(void)
	{
		m_indexCount = 0;
		m_counter = 0;
		m_indexs.clear();
		if (m_buffer != NULL)
		{
			delete[] m_buffer;
			m_buffer = NULL;
		}
		m_total = 0;
	}
private:
	long	m_messageid;
	unsigned long	m_total;
	char *	m_buffer;
	short	m_type;
	bool	m_newflag;
	time_t	m_time;

	size_t	m_indexCount;
	size_t	m_filesize;
	size_t	m_width;
	size_t	m_height;

	int		m_counter;
};


#endif // __messageinfo_h__

