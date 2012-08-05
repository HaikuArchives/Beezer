/*
    URLView 2.11
    written by William Kakes of Tall Hill Software.

    This class provides an underlined and clickable BStringView
    that will launch the web browser, e-mail program, or FTP client
    when clicked on.  Other features include hover-highlighting,
    right-click    menus, and drag-and-drop support.

    You are free to use URLView in your own programs (both open-source
    and closed-source) free of charge, but a mention in your read me
    file or your program's about box would be appreciated.  See
    http://www.tallhill.com    for current contact information.

    URLView is provided as-is, with no warranties of any kind.  If
    you use it, you are on your own.

    Original Code: William Kakes
    Modifications: Ramshankar <v.ramshankar(at)gmail(dot)com>
                   Chris Roberts
        The alterations are mostly cosmetic in nature, i.e. the
        style of 'bracketing', indents, naming conventions have
        been changed to suit this project. The overall implementation
        remains un-altered.
*/

#ifndef _URL_VIEW_H
#define _URL_VIEW_H

#include <Cursor.h>
#include <List.h>
#include <Mime.h>
#include <PopUpMenu.h>
#include <String.h>
#include <StringView.h>

// This is the link's mouse cursor (a replica of NetPositive's link cursor).
const uint8 url_cursor[] =
{
    16, 1, 1, 2,
    // This is the cursor data.
    0x00, 0x00,    0x38, 0x00,    0x24, 0x00,    0x24, 0x00,
    0x13, 0xe0,    0x12, 0x5c,    0x09, 0x2a,    0x08, 0x01,
    0x3c, 0x21,    0x4c, 0x71,    0x42, 0x71,    0x30, 0xf9,
    0x0c, 0xf9,    0x02, 0x00,    0x01, 0x00,    0x00, 0x00,

    // This is the cursor mask.
    0x00, 0x00,    0x38, 0x00,    0x3c, 0x00,    0x3c, 0x00,
    0x1f, 0xe0,    0x1f, 0xfc,    0x0f, 0xfe,    0x0f, 0xff,
    0x3f, 0xff,    0x7f, 0xff,    0x7f, 0xff,    0x3f, 0xff,
    0x0f, 0xff,    0x03, 0xfe,    0x01, 0xf8,    0x00, 0x00,
};

const rgb_color blue =        { 0, 0, 255 };        // The default link color, blue.
const rgb_color red  =        { 255, 0, 0 };        // The default clicked-link color, red.
const rgb_color dark_blue =   { 0, 0, 120 };        // The default link hover color, dark blue.
const rgb_color gray =        { 100, 100, 100 };    // The default disabled color, gray.

class URLView : public BStringView
{
    public:
        URLView(BRect frame, const char* name, const char* label, const char* url,
                uint32 resizingMode = B_FOLLOW_LEFT | B_FOLLOW_TOP, uint32 flags = B_WILL_DRAW);
        ~URLView();

        // Inherited hooks
        virtual void        AttachedToWindow();
        virtual void        Draw(BRect updateRect);
        virtual void        MessageReceived(BMessage* message);
        virtual void        MouseDown(BPoint point);
        virtual void        MouseMoved(BPoint point, uint32 transit, const BMessage* message);
        virtual void        MouseUp(BPoint point);
        virtual void        WindowActivated(bool active);

        // Additional hooks
        virtual void        AddAttribute(const char* name, const char* value);
        virtual bool        IsEnabled();
        virtual void        SetColor(rgb_color color);
        virtual void        SetColor(uchar red, uchar green, uchar blue, uchar alpha = 255);
        virtual void        SetClickColor(rgb_color color);
        virtual void        SetClickColor(uchar red, uchar green, uchar blue, uchar alpha = 255);
        virtual void        SetDisabledColor(rgb_color color);
        virtual void        SetDisabledColor(uchar red, uchar green, uchar blue, uchar alpha = 255);
        virtual void        SetDraggable(bool draggable);
        virtual void        SetEnabled(bool enabled);
        virtual void        SetHoverColor(rgb_color color);
        virtual void        SetHoverColor(uchar red, uchar green, uchar blue, uchar alpha = 255);
        virtual void        SetHoverEnabled(bool hover);
        virtual void        SetIconSize(icon_size iconSize);
        virtual void        SetUnderlineThickness(int thickness);
        virtual void        SetURL(const char* url);

    private:
        // Private hooks
        void               CopyToClipboard();
        void               CreateBookmark(const BString* fullName, const BString* title);
        void               CreatePerson(const BString* fullName, const BString* title);
        BPopUpMenu*        CreatePopupMenu();
        void               DoBookmarkDrag();
        void               DoPersonDrag();
        BString            GetImportantURL();
        BRect              GetTextRect();
        BRect              GetURLRect();
        bool               IsEmailLink();
        bool               IsFTPLink();
        bool               IsHTMLLink();
        void               LaunchURL();
        void               Redraw();
        void               WriteAttributes(int fd);

        BString*           m_url;
        rgb_color          m_foreColor;
        rgb_color          m_clickColor;
        rgb_color          m_hoverColor;
        rgb_color          m_disabledColor;
        bool               m_enabled;
        bool               m_hoverEnabled;
        bool               m_draggable;
        int                m_underlineThickness;
        int                m_iconSize;
        bool               m_selected;
        bool               m_hovering;
        bool               m_draggedOut;
        bool               m_inPopup;
        const BCursor*      m_linkCursor;
        BPoint             m_dragOffset;
        BList*             m_attributes;

        typedef struct kp
        {
            BString*        key;
            BString*        value;
        } KeyPair;
};

#endif /* _URL_VIEW_H */
