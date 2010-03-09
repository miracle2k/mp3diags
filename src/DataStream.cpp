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


#include  <sstream>
#include  <iomanip>
#include  <cstdio>

#include  "DataStream.h"

#include  "Helpers.h"
#include  "MpegFrame.h"
#include  "MpegStream.h"
#include  "CommonData.h"


using namespace std;
using namespace pearl;



//======================================================================================================
//======================================================================================================


/*override*/ void SimpleDataStream::copy(std::istream& in, std::ostream& out)
{
    appendFilePart(in, out, m_pos, m_nSize);
}



UnknownDataStreamBase::UnknownDataStreamBase(int nIndex, NoteColl& notes, istream& in, streamoff nSize) : SimpleDataStream(nIndex, in.tellg(), nSize)
{
    StreamStateRestorer rst (in);

    STRM_ASSERT (in);
    STRM_ASSERT (nSize > 0);

    streamoff nBfrSize (min(streamoff(BEGIN_SIZE), nSize));
    read(in, m_begin, nBfrSize);

    streampos pos (m_pos);
    pos += nSize - 1;
    in.seekg(pos);
    char c;

    MP3_CHECK (1 == read(in, &c, 1), m_pos, unknTooShort, BadUnknownStream());

    rst.setOk();
}

/*override*/ std::string UnknownDataStreamBase::getInfo() const
{
    return "begins with: " + asHex(m_begin, min(int(BEGIN_SIZE), int(m_nSize)));
}



void UnknownDataStreamBase::append(const UnknownDataStreamBase& other)
{
    streampos pos (m_pos);
    pos += m_nSize;
    STRM_ASSERT (pos == other.m_pos);
    if (m_nSize < BEGIN_SIZE)
    {
        int n (min(BEGIN_SIZE - m_nSize, other.m_nSize));
        std::copy(other.m_begin, other.m_begin + n, m_begin + m_nSize);
    }
    m_nSize += other.m_nSize;
}



bool TruncatedMpegDataStream::hasSpace(std::streamoff nSize) const
{
    return nSize < getExpectedSize() - getSize(); // "<" rather than "<="; if it gets full why would it be called "truncated"
}



TruncatedMpegDataStream::TruncatedMpegDataStream(MpegStream* pPrevMpegStream, int nIndex, NoteColl& notes, std::istream& in, std::streamoff nSize) :
        UnknownDataStreamBase(nIndex, notes, in, nSize), m_pFrame(0)
{
    if (0 == pPrevMpegStream) { throw NotTruncatedMpegDataStream(); }
    in.seekg(m_pos);

    StreamStateRestorer rst (in);


    NoteColl tmpNotes (20);
    try
    {
        m_pFrame = new MpegFrameBase(tmpNotes, in);
    }
    catch (const MpegFrameBase::NotMpegFrame&)
    {
        throw NotTruncatedMpegDataStream();
    }


    if (!pPrevMpegStream->isCompatible(*m_pFrame))
    {
        delete m_pFrame;
        throw NotTruncatedMpegDataStream();
    }

    streampos pos (m_pos);
    pos += nSize;
    in.seekg(pos);
    rst.setOk();
}

TruncatedMpegDataStream::~TruncatedMpegDataStream()
{
    delete m_pFrame;
}

/*override*/ std::string TruncatedMpegDataStream::getInfo() const
{
    string s (decodeMpegFrame(m_begin, ", "));
    return s;
}


int TruncatedMpegDataStream::getExpectedSize() const
{
    return m_pFrame->getSize();
}


NullDataStream::NullDataStream(int nIndex, NoteColl& notes, std::istream& in) : DataStream(nIndex), m_nSize(0)
{
    StreamStateRestorer rst (in);

    streampos pos (in.tellg());
    m_pos = pos;

    const int BFR_SIZE (256);
    char bfr [BFR_SIZE];

    for (;;)
    {
        streamsize nRead (read(in, bfr, BFR_SIZE));
        for (int i = 0; i < nRead; ++i)
        {
            if (0 != bfr[i])
            {
                m_nSize += i;
                goto e1;
            }
        }
        m_nSize += nRead;
        if (nRead < BFR_SIZE) { break; }
    }
e1:
    MP3_CHECK_T (m_nSize >= 16, m_pos, "Not a NULL stream. File too short.", NotNullStream());
    pos += m_nSize;
    in.clear();
    in.seekg(pos);

    rst.setOk();
}


/*override*/ void NullDataStream::copy(std::istream&, std::ostream& out)
{
    const int BFR_SIZE (256);
    char bfr [BFR_SIZE];
    for (int i = 0; i < BFR_SIZE; ++i) { bfr[i] = 0; }

    streamoff n (m_nSize);
    while (n > BFR_SIZE)
    {
        out.write(bfr, BFR_SIZE);
        n -= BFR_SIZE;
    }

    out.write(bfr, n);
    CB_CHECK1 (out, WriteError());
}


/*override*/ std::string NullDataStream::getInfo() const
{
    return "";
}


UnsupportedDataStream::UnsupportedDataStream(int nIndex, NoteColl& notes, istream& in, streamoff nSize, const char* szBaseName, const string& strInfo) :
            UnknownDataStreamBase(nIndex, notes, in, nSize),
            m_strName(string("Unsupported ") + szBaseName),
            m_strBaseName(szBaseName),
            m_strInfo(UnknownDataStreamBase::getInfo())
{
    if (!strInfo.empty())
    {
        string s1 (strInfo);
        if (endsWith(s1, ".") || endsWith(s1, ";"))
        {
            s1.erase(s1.size() - 1);
        }
        m_strInfo = s1 + " - " + m_strInfo;
    }
}



//======================================================================================================
//======================================================================================================

//
/*static*/ /*bool TagReader::isTimeValid(const std::string& s)
{
}

yyy-MM-ddTHH:mm:ss
*/

TagTimestamp::TagTimestamp(const std::string& strVal)
{
    init(strVal);
}


TagTimestamp::TagTimestamp(const char* szVal /*= 0*/)
{
    init(0 != szVal ? szVal : "");
}



// if strVal is invalid it clears m_strVal, m_strYear and m_strDayMonth and throws InvalidTime
void TagTimestamp::init(std::string s)
{
    m_szVal[0] = 0;
    m_szYear[0] = 0;
    m_szDayMonth[0] = 0;


    int n (cSize(s));
    if (n >= 11 && 'T' == s[10])
    {
        s[10] = ' ';
    }

    static const char* szPatt ("****-**-** **:**:**X"); // 'X' is needed to avoid checking indexes
    CB_CHECK1 (n <= 19, InvalidTime());
    CB_CHECK1 (0 == n || '*' != szPatt[n], InvalidTime());

    for (int i = 0; i < n; ++i)
    {
        char c (szPatt[i]);
        if ('*' == c)
        {
            CB_CHECK1 (isdigit(s[i]), InvalidTime());
        }
        else
        {
            CB_CHECK1 (szPatt[i] == s[i], InvalidTime());
        }
    }

    //ttt2 check validity for years and dates

    strcpy(m_szVal, s.c_str());
    if (n > 0)
    {
        strcpy(m_szYear, s.substr(0, 4).c_str());
        if (n > 4)
        {
            strcpy(m_szDayMonth, ((n > 7 ? s.substr(8, 2) : "01") + s.substr(5, 2)).c_str());
        }
    }
}



// text representation for each Feature
/*static*/ const char* TagReader::getLabel(int n)
{
    static const char* s_szTitle[] = { "Title", "Artist", "Track #", "Time", "Genre", "Picture", "Album", "Rating", "Composer", "VA" };
    CB_ASSERT (n >= 0 && n < LIST_END);
    return s_szTitle[n];
}

                                    // orig: { TITLE, ARTIST, TRACK_NUMBER, TIME, GENRE, IMAGE, ALBUM, RATING, COMPOSER, VARIOUS_ARTISTS, LIST_END };
/*static*/ int TagReader::FEATURE_ON_POS[] = { TRACK_NUMBER, ARTIST, TITLE, ALBUM, VARIOUS_ARTISTS, TIME, GENRE, IMAGE, RATING, COMPOSER }; //ttt2 perhaps move to CommonData and make configurable, as long as discarding some columns (e.g. composer)


//    static int INV_FEATURE_ON_POS[]; // the "inverse" of FEATURE_ON_POS: what Feature appears in a given position
// /*static*/ int TagReader::INV_FEATURE_ON_POS[] = { 1, 0, 2, 3, 4, 5, 6, 7, 8 }; // INV_FEATURE_ON_POS ~ POS_OF_FEATURE

/*static*/ int TagReader::POS_OF_FEATURE[LIST_END];
namespace
{
    struct Init
    {
        Init()
        {
            for (int i = 0; i < TagReader::LIST_END; ++i)
            {
                TagReader::POS_OF_FEATURE[TagReader::FEATURE_ON_POS[i]] = i;
            }
        }
    };

    Init init;
}


// returns the corresponding feature converted to a string; if it's not supported, it returns an empty string; for IMAGE an empty string regardless of a picture being present or not
std::string TagReader::getValue(Feature f) const
{
    switch (f)
    {
    case TITLE: if (getSupport(TITLE)) { return getTitle(); } else { return ""; }
    case ARTIST: if (getSupport(ARTIST)) { return getArtist(); } else { return ""; }
    case TRACK_NUMBER: if (getSupport(TRACK_NUMBER)) { return getTrackNumber(); } else { return ""; }
    case TIME: if (getSupport(TIME)) { return getTime().asString(); } else { return ""; }
    case GENRE: if (getSupport(GENRE)) { return getGenre(); } else { return ""; }
    case IMAGE: return "";
    case ALBUM: if (getSupport(ALBUM)) { return getAlbumName(); } else { return ""; }
    case RATING:
        if (getSupport(RATING))
        {
            double r (getRating());
            if (r >= 0)
            {
                char a [10];
                sprintf(a, "%0.1f", r);
                return a;
            }
        }
        return "";

    case COMPOSER: if (getSupport(COMPOSER)) { return getComposer(); } else { return ""; }

    case VARIOUS_ARTISTS:
        if (getSupport(VARIOUS_ARTISTS))
        {
            int nVa (getVariousArtists());
            string s;
            if (nVa & VA_ITUNES) { s += "i"; }
            if (nVa & VA_WMP) { s += "w"; }
            return s;
        }
        else
        {
            return "";
        }

    default: return "";
    }
}


/*static*/ string TagReader::getVarArtistsValue() // what getValue(VARIOUS_ARTISTS) returns for VA tags, based on global configuration
{
    string s;
    const CommonData* pCommonData (getCommonData());
    if (pCommonData->m_bItunesVarArtists) { s += "i"; }
    if (pCommonData->m_bWmpVarArtists) { s += "w"; }
    return s;
}

/*virtual*/ TagReader::~TagReader()
{
//    qDebug("destroy TagReader at %p", this);
}

