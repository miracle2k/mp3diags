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


#ifndef ImageInfoPanelWdgImplH
#define ImageInfoPanelWdgImplH

#include  <QWidget>

#include  "ui_ImageInfoPanel.h"

#include  "TagWriter.h"
#include  "CommonTypes.h"


class ImageInfoPanelWdgImpl : public QFrame, private Ui::ImageInfoPanelWdg
{
    Q_OBJECT

    const TagWrtImageInfo m_tagWrtImageInfo;
    int m_nPos;

public:
    ImageInfoPanelWdgImpl(QWidget* pParent, const TagWrtImageInfo& tagWrtImageInfo, int nPos);
    ~ImageInfoPanelWdgImpl();
    /*$PUBLIC_FUNCTIONS$*/

    void setNormalBackground();
    void setHighlightBackground();

public slots:
    /*$PUBLIC_SLOTS$*/

protected:
    /*$PROTECTED_FUNCTIONS$*/

protected slots:
    /*$PROTECTED_SLOTS$*/
    void on_m_pFullB_clicked();
    void on_m_pAssignB_clicked() { emit assignImage(m_nPos); }
    void on_m_pEraseB_clicked() { emit eraseFile(m_nPos); }

signals:
    void assignImage(int);
    void eraseFile(int);
};

#endif // #ifndef ImageInfoPanelWdgImplH

