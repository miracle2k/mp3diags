/***************************************************************************
 *   MP3 Diags - diagnosis, repairs and tag editing for MP3 files          *
 *                                                                         *
 *   Copyright (C) 2009 by Marian Ciobanu                                  *
 *   ciobi@inbox.com                                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef __GNUC__


#include  "fstream_unicode.h"

#include  <sys/stat.h>


// might want to look at basic_file_stdio.cc or ext/stdio_filebuf.h

// http://www2.roguewave.com/support/docs/leif/sourcepro/html/stdlibref/basic-ifstream.html
// STLPort
// MSVC
// Dinkumware - not

// http://www.aoc.nrao.edu/~tjuerges/ALMA/STL/html/class____gnu__cxx_1_1stdio__filebuf.html




//ttt2 perhaps add flush()
/* ttt2 see about file flushing:
    http://support.microsoft.com/default.aspx/kb/148505
    - fdatasync() - like fsync() but doesn't change metadata (e.g. mtime) so it's faster
    - since the C++ Library has nothing to do flushing, perhaps this would work: standalone "commit(const ofstream_utf8&)" and/or "commit(const string&)" ; also, we don't want to commit all the files; or perhaps "commit(ofstream_utf8&)" is needed, which would first close the file
    - there's also out.rdbud()->pubsync(), but what it does is OS-dependent
    - use external disk / flash, to see the LED, for testing
*/



// converts __mode to flags that can be used by the POSIX open() function
int getOpenFlags(std::ios_base::openmode __mode)
{
    int nAcc;
    if (std::ios_base::in & __mode)
    {
        if (std::ios_base::out & __mode)
        {
            nAcc = O_RDWR | O_CREAT;
        }
        else
        {
            nAcc = O_RDONLY;
        }
    }
    else if (std::ios_base::out & __mode)
    {
        nAcc = O_RDWR | O_CREAT;
    }
    else
    {
        throw 1; // ttt2
    }

    #ifdef O_LARGEFILE
    nAcc |= O_LARGEFILE;
    #endif

    if (std::ios_base::trunc & __mode)
    {
        nAcc |= O_TRUNC;
    }

    if (std::ios_base::app & __mode)
    {
        nAcc |= O_APPEND;
    }

    #ifdef O_BINARY
    if (std::ios_base::binary & __mode)
    {
        nAcc |= O_BINARY;
    }
    #endif

    return nAcc;
}

/*
template<>
int unicodeOpenHlp(const QString& s, std::ios_base::openmode __mode)
{
    int nFd (open(s.toUtf8().constData(), getOpenFlags(__mode), S_IREAD | S_IWRITE));
    return nFd;
}
*/

#ifndef WIN32

    template<>
    int unicodeOpenHlp(const char* const& szUtf8Name, std::ios_base::openmode __mode)
    {
        int nAcc (getOpenFlags(__mode));
        int nFd (open(szUtf8Name, nAcc, S_IREAD | S_IWRITE));
        //qDebug("fd %d %s acc=%d", nFd, szUtf8Name, nAcc);
        return nFd;
    }

#if 0
    template<>
    int unicodeOpenHlp(const wchar_t* const& /*wszUtf16Name*/, std::ios_base::openmode /*__mode*/)
    {
        throw 1; //ttt2 add if needed
    }
#endif

#else // #ifndef WIN32


    #include  <windows.h>

    #include  <vector>
    #include  <string>

    using namespace std;


    static wstring wstrFromUtf8(const string& s)
    {
        vector<wchar_t> w (s.size() + 1);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &w[0], w.size());
        //inspect(&w[0], w.size()*2);
        return &w[0];
    }


    template<>
    int unicodeOpenHlp(const wchar_t* const& wszUtf16Name, std::ios_base::openmode __mode)
    {
        int nAcc (getOpenFlags(__mode));

        int nFd (_wopen(wszUtf16Name, nAcc, S_IREAD | S_IWRITE));
        //int nFd (open(szUtf8Name, nAcc, S_IREAD | S_IWRITE));
        //qDebug("fd %d %s acc=%d", nFd, szUtf8Name, nAcc);
        return nFd;
    }

    template<>
    int unicodeOpenHlp(const char* const& szUtf8Name, std::ios_base::openmode __mode)
    {
        return unicodeOpenHlp(wstrFromUtf8(szUtf8Name).c_str(), __mode);
    }

#endif // #ifndef WIN32 / #else



template<>
int unicodeOpenHlp(const int& fd, std::ios_base::openmode /*__mode*/)
{
    return fd;
}




//ttt2 review O_SHORT_LIVED

#else // #ifdef __GNUC__

// nothing to do for now; the MSVC version is fully inline and no ports to other compilers exist

#endif // #ifdef __GNUC__

