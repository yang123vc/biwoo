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

// XmlParseSetting.h file here
#ifndef __XmlParseSetting_h__
#define __XmlParseSetting_h__
#pragma warning(disable:4819)

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

typedef unsigned short  u_short;

class XmlParseSetting
{
public:
	XmlParseSetting(void)
		: m_address(_T("127.0.0.1:8010"))
		, m_fileserver(_T("127.0.0.1:8014"))
		, m_p2prtp(_T("127.0.0.1:8020"))
		, m_p2pudp(_T("127.0.0.1:8022"))
		, m_locale(""), m_langtext("")

	{}
	~XmlParseSetting(void)
	{}

public:
	void load(const std::string & filename)
	{
		// 建立空的property tree对象
		using boost::property_tree::ptree;
		ptree pt;

		// 把XML文件载入到property tree里. 如果出错    // (无法打开文件,解析错误), 抛出一个异常.
		read_xml(filename, pt);

		// 如果键里有小数点,我们可以在get方法中指定其它分隔符
		// 如果debug.filename没找到,抛出一个异常
		//m_sCgcpName = pt.get<std::string>("debug.filename");
		// "root.cgcp.*"
		m_address = pt.get("root.server.address", _T("127.0.0.1:8010"));
		m_fileserver = pt.get("root.server.fileserver", _T("127.0.0.1:8014"));
		m_p2prtp = pt.get("root.server.p2p_rtp", _T("127.0.0.1:8020"));
		m_p2pudp = pt.get("root.server.p2p_udp", _T("127.0.0.1:8022"));
		//m_nCgcpRank = pt.get("root.cgcp.rank", 0);

		m_locale = pt.get("root.locale", _T("chs"));
		m_langtext = pt.get("root.langtext", _T("langtext.xml"));

	}

	const tstring & address(void) const {return m_address;}
	const tstring & fileserver(void) const {return m_fileserver;}
	const tstring & p2prtp(void) const {return m_p2prtp;}
	const tstring & p2pudp(void) const {return m_p2pudp;}

	const tstring & locale(void) const {return m_locale;}
	const tstring & langtext(void) const {return m_langtext;}

private:
	std::string		m_address;
	std::string		m_fileserver;
	std::string		m_p2prtp;
	std::string		m_p2pudp;

	std::string		m_locale;
	std::string		m_langtext;

};

#endif // __XmlParseSetting_h__
