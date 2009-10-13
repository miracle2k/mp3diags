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


#include  <cmath>

#include  <QBuffer>
#include  <QDialog>
#include  <QLabel>
#include  <QVBoxLayout>
#include  <QPainter>

#include  "CommonTypes.h"

#include  "Helpers.h"

using namespace std;


ImageInfo::ImageInfo(int nImageType, Status eStatus, Compr eCompr, QByteArray compressedImg, int nWidth, int nHeight) :
        m_eCompr(eCompr), m_eStatus(eStatus), m_nWidth(nWidth), m_nHeight(nHeight), m_compressedImg(compressedImg), m_nImageType(nImageType)
{
}


// the picture is scaled down, keeping the aspect ratio, if the limits are exceeded; 0 and negative limits are ignored;
// if nMaxWidth>0 and nMaxHeight<=0, nMaxHeight has the same value as nMaxWidth;
QPixmap ImageInfo::getPixmap(int nMaxWidth /*= -1*/, int nMaxHeight /*= -1*/) const
{
    CB_ASSERT (NO_PICTURE_FOUND != m_eStatus);

    if (nMaxHeight <= 0) { nMaxHeight = nMaxWidth; }
    if (nMaxWidth <= 0) { nMaxWidth = nMaxHeight; }

    QPixmap pic;

    if (USES_LINK == m_eStatus || ERROR_LOADING == m_eStatus || !pic.loadFromData(m_compressedImg)) //ttt2 not sure how loadFromData() handles huge images;
    {
        if (nMaxWidth < 0) { nMaxWidth = m_nWidth; }
        if (nMaxHeight < 0) { nMaxHeight = m_nHeight; }

        if (nMaxWidth <= 0) { nMaxWidth = 200; }
        if (nMaxHeight <= 0) { nMaxHeight = 200; }

        QPixmap errImg (nMaxWidth, nMaxHeight);
        QPainter pntr (&errImg);
        pntr.fillRect(0, 0, nMaxWidth, nMaxHeight, QColor(255, 128, 128));
        pntr.drawRect(0, 0, nMaxWidth - 1, nMaxHeight - 1);
        //pntr.drawText(5, nMaxHeight/2 + 10, USES_LINK == m_eStatus ? "Link" : (ERROR_LOADING == m_eStatus ?  "Error" : "Uncompr error"));
        pntr.drawText(QRectF(0, 0, nMaxWidth, nMaxHeight), Qt::AlignCenter | Qt::TextWordWrap, USES_LINK == m_eStatus ? "Link" : (ERROR_LOADING == m_eStatus ?  "Error" : "Uncompr error"));
        return errImg;
    }

    if (nMaxWidth <= 0 || (pic.width() <= nMaxWidth && pic.height() <= nMaxHeight))
    {
        return pic;
    }

    return pic.scaled(nMaxWidth, nMaxHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}


bool ImageInfo::operator==(const ImageInfo& other) const
{
    return m_eCompr == other.m_eCompr /*&& m_eStatus == other.m_eStatus*/ && m_nWidth == other.m_nWidth && m_compressedImg == other.m_compressedImg; // !!! related to Id3V230StreamWriter::addImage() status is ignored in both places //ttt2 review decision to ignore status
}


/*static*/ int ImageInfo::MAX_IMAGE_SIZE (100000);

ImageInfo::ImageInfo(int nImageType, Status eStatus, const QPixmap& pic) : m_eCompr(JPG), m_eStatus(eStatus), m_nImageType(nImageType)
{
    QPixmap scaledPic;
    compress(pic, scaledPic, m_compressedImg);

    m_nWidth = scaledPic.width();
    m_nHeight = scaledPic.height();
}



/*static*/ const char* ImageInfo::getImageType(int nImageType)
{
    switch (nImageType)
    {
    case 0x00: return "other";
    case 0x01: return "32x32 icon";
    case 0x02: return "other file icon";
    case 0x03: return "front cover";
    case 0x04: return "back cover";
    case 0x05: return "leaflet page";
    case 0x06: return "media";
    case 0x07: return "lead artist";
    case 0x08: return "artist";
    case 0x09: return "conductor";
    case 0x0a: return "band";
    case 0x0b: return "composer";
    case 0x0c: return "lyricist";
    case 0x0d: return "recording location";
    case 0x0e: return "during recording";
    case 0x0f: return "during performance";
    case 0x10: return "screen capture";
    //case 0x11: return "a bright coloured fish";
    case 0x12: return "illustration";
    case 0x13: return "band/artist logotype";
    case 0x14: return "publisher/studio logotype";
    default:
        return "unknown";
    }
}


const char* ImageInfo::getImageType() const
{
    return getImageType(m_nImageType);
}


/*static*/ const char* ImageInfo::getComprStr(Compr eCompr)
{
    switch (eCompr)
    {
    case INVALID: return "invalid";
    case JPG: return "JPEG";
    case PNG: return "PNG";
    }
    CB_ASSERT (false);
}


// scales down origPic and stores the pixmap in scaledPic, as well as a compressed version in comprImg; the algorithm coninues until comprImg becomes smaller than MAX_IMAGE_SIZE or until the width and the height of scaledPic get smaller than 150; no scaling is done if comprImg turns out to be small enough for the original image;
/*static*/ void ImageInfo::compress(const QPixmap& origPic, QPixmap& scaledPic, QByteArray& comprImg)
{
    const int QUAL (-1); //ttt2 hard-coded

    //QPixmap scaledImg;
    int n (max(origPic.width(), origPic.height()));
//qDebug("-------------");
    for (int i = n;;)
    {
        scaledPic = origPic.scaled(i, i, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        comprImg.clear();
        QBuffer bfr (&comprImg);
        //nWidth = scaledPic.width(); nHeight = scaledPic.height();

        scaledPic.save(&bfr, "jpg", QUAL);

        int nSize (comprImg.size());
//qDebug("width=%d, size=%d", i, nSize);
        if (nSize <= MAX_IMAGE_SIZE) { break; }

        //double d (min(4.0/5, 1/sqrt(nSize*1.0/MAX_IMAGE_SIZE)));
        double d (min(4.0/5, sqrt(MAX_IMAGE_SIZE*1.0/nSize))); // ttt2 review this; the "1.0" is quite wrong in some cases

        i = int(i*d);

        if (i <= 150) { break; }
    }
}




QString ImageInfo::getTextDescr(const QString& qstrSep /*= "\n"*/) const
{
    QString s;
    s.sprintf("%dx%d", getWidth(), getHeight());
    s += qstrSep + getImageType();
    return s;
}


void ImageInfo::showFull(QWidget* pParent) const
{
    QDialog dlg (pParent, getNoResizeWndFlags());

    QVBoxLayout* pLayout (new QVBoxLayout(&dlg));
    //dlg.setLayout(pGridLayout);
    QLabel* p (new QLabel(&dlg));
    p->setPixmap(getPixmap()); //ttt2 see if it should limit size (IIRC QLabel scaled down once a big image)
    pLayout->addWidget(p, 0, Qt::AlignHCenter);

    p = new QLabel(getTextDescr(), &dlg);
    p->setAlignment(Qt::AlignHCenter);
    pLayout->addWidget(p, 0, Qt::AlignHCenter);

    dlg.exec();
}


ostream& operator<<(ostream& out, const AlbumInfo& inf)
{
    out << "title: \"" << inf.m_strTitle << /*"\", artist: \"" << inf.m_strArtist << "\", composer: \"" << inf.m_strComposer <<*/ /*"\", format: \"" << inf.m_strFormat <<*/ "\", genre: \"" << inf.m_strGenre << "\", released: \"" << inf.m_strReleased << "\", var artists: \"" << int(inf.m_eVarArtists) << "\"\n\nnotes: " << inf.m_strNotes << endl;

    /*for (int i = 0, n = cSize(inf.m_vstrImageNames); i < n; ++i)
    {
        out << inf.m_vstrImageNames[i] << endl;
    }*/

    out << "\ntracks:" << endl;
    for (int i = 0, n = cSize(inf.m_vTracks); i < n; ++i)
    {
        out << "pos: \"" << inf.m_vTracks[i].m_strPos << "\", artist: \"" << inf.m_vTracks[i].m_strArtist << "\", title: \"" << inf.m_vTracks[i].m_strTitle << "\", composer: \"" << inf.m_vTracks[i].m_strComposer << "\"" << endl;
    }

    return out;
}


