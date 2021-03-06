/*
 * Copyright (c) 2009, Ramshankar (aka Teknomancer)
 * Copyright (c) 2011, Chris Roberts
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * -> Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * -> Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * -> Neither the name of the author nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <Bitmap.h>
#include <Region.h>
#include <View.h>

#include <string.h>
#include <malloc.h>

#include "Preferences.h"
#include "PrefsFields.h"
#include "PrefsListItem.h"
#include "UIConstants.h"



PrefsListItem::PrefsListItem(const char* text, BBitmap* bmp, bool makeBold)
    : BStringItem(text)
{
    font_height fntHt;
    be_plain_font->GetHeight(&fntHt);
    m_fontDelta = fntHt.ascent / 2;
    m_makeBold = makeBold;
    m_bitmap = bmp;
    if (m_makeBold == false)
        m_fontDelta = 3;

    rgb_color actFore = _prefs_interface.FindColorDef(kPfActFore, K_ACTIVE_FORE_COLOR);
    rgb_color actBack = _prefs_interface.FindColorDef(kPfActBack, K_ACTIVE_SELECT_COLOR);

    m_selTextColor = actFore;
    m_selBackColor = actBack;
}



PrefsListItem::~PrefsListItem()
{
    // and NO we don't delete m_bitmap as its allocated/de-allocated by calling side
}



void PrefsListItem::DrawItem(BView* owner, BRect frame, bool complete)
{
    if (IsSelected() || complete)
    {
        rgb_color color;
        color = IsSelected() ? m_selBackColor : owner->ViewColor();
        owner->SetHighColor(color);
        owner->FillRect(frame);
        owner->SetLowColor(color);

        // lighten top 2 lines
        owner->BeginLineArray(4);
        owner->AddLine(BPoint(frame.left, frame.top), BPoint(frame.right, frame.top),
                       tint_color(owner->HighColor(), B_LIGHTEN_2_TINT));
        owner->AddLine(BPoint(frame.left, frame.top + 1), BPoint(frame.right, frame.top + 1),
                       tint_color(owner->HighColor(), B_LIGHTEN_1_TINT));

        // Darken bottom 2 liness (cool effect!!)
        rgb_color specialDark1 = tint_color(owner->HighColor(), B_DARKEN_1_TINT);
        specialDark1.red += 10; specialDark1.green += 10; specialDark1.blue += 10;
        owner->AddLine(BPoint(frame.left, frame.bottom - 1), BPoint(frame.right, frame.bottom - 1),
                       specialDark1);
        rgb_color specialDark = specialDark1;
        specialDark.red -= 20; specialDark.green -= 20; specialDark.blue -= 20;
        owner->AddLine(BPoint(frame.left, frame.bottom), BPoint(frame.right, frame.bottom), specialDark);
        owner->EndLineArray();
    }
    else
        owner->SetLowColor(owner->ViewColor());

    if (IsEnabled())
    {
        if (IsSelected())
        {
            owner->SetHighColor(m_selTextColor);
            if (m_makeBold)
                owner->SetFont(be_bold_font);
        }
        else
        {
            owner->SetHighColor(owner->ViewColor());
            owner->FillRect(frame);
            owner->SetHighColor(0, 0, 0, 255);
            if (m_makeBold)
                owner->SetFont(be_plain_font);
        }
    }
    else
        owner->SetHighColor(182, 182, 182, 255);

    if (m_bitmap)
    {
        float bmpWidth = m_bitmap->Bounds().Width();
        float bmpHeight = m_bitmap->Bounds().Height();
        float itemWidth = (frame.right - frame.left);

        // Draw bitmap at center of item
        owner->SetDrawingMode(B_OP_ALPHA);
        owner->MovePenTo(6, frame.bottom - m_fontDelta / 2 - bmpHeight - 1);
        owner->DrawBitmapAsync(m_bitmap);

        // Draw string at right of item
        owner->SetDrawingMode(B_OP_COPY);
        owner->MovePenTo(6 + bmpWidth + 6, frame.bottom - bmpHeight / 2 + 1);
        owner->DrawString(Text());
    }
    else
    {
        owner->SetDrawingMode(B_OP_COPY);
        owner->MovePenTo(frame.left + 6, frame.bottom - m_fontDelta);
        owner->DrawString(Text());
    }
}



float PrefsListItem::FontHeight() const
{
    return m_fontDelta * 2;
}


