#include "MacNativeShare.h"

#ifdef __APPLE__
#import <Cocoa/Cocoa.h>
#import <AppKit/AppKit.h>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QRegularExpression>
#include <QDesktopServices>
#include <QProcess>
#include <QDebug>

bool MacNativeShare::shareFile(const QString &filePath)
{
    @autoreleasepool {
        NSString *nsPath = [NSString stringWithUTF8String:filePath.toUtf8().constData()];
        NSURL *fileUrl = [NSURL fileURLWithPath:nsPath];
        if (![[NSFileManager defaultManager] fileExistsAtPath:nsPath]) {
            return false;
        }

        NSArray *items = @[fileUrl];
        NSWindow *keyWindow = [NSApp keyWindow];
        NSView *contentView = [keyWindow contentView];

        if (!contentView) {
            return false;
        }

        NSSharingServicePicker *picker = [[NSSharingServicePicker alloc] initWithItems:items];
        NSRect rect = NSMakeRect(contentView.bounds.size.width / 2, contentView.bounds.size.height / 2, 1, 1);
        [picker showRelativeToRect:rect ofView:contentView preferredEdge:NSRectEdgeMinY];
        return true;
    }
}

bool MacNativeShare::copyFileToClipboard(const QString &filePath)
{
    @autoreleasepool {
        NSString *nsPath = [NSString stringWithUTF8String:filePath.toUtf8().constData()];
        NSURL *fileUrl = [NSURL fileURLWithPath:nsPath];
        if (![[NSFileManager defaultManager] fileExistsAtPath:nsPath]) {
            return false;
        }

        NSPasteboard *pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard clearContents];
        [pasteboard writeObjects:@[fileUrl]];
        return true;
    }
}

bool MacNativeShare::attachAndSendWhatsApp(const QString &filePath, const QString &phoneNumber, const QString &message)
{
    // 1. Copy the PDF file to macOS pasteboard
    copyFileToClipboard(filePath);

    // 2. Format phone number & WhatsApp URL
    QString cleanPhone = phoneNumber;
    cleanPhone.remove(QRegularExpression("[^0-9]"));

    QString encodedMsg = QString::fromUtf8(QUrl::toPercentEncoding(message));
    
    // 3. Open WhatsApp, activate, paste PDF, and send
    QString script;
    if (cleanPhone.length() >= 7) {
        script = QString(
            "set the clipboard to (POSIX file \"%1\")\n"
            "tell application \"WhatsApp\" to activate\n"
            "tell application \"System Events\"\n"
            "   open location \"whatsapp://send?phone=%2\"\n"
            "end tell\n"
            "delay 2.5\n"
            "tell application \"WhatsApp\" to activate\n"
            "delay 0.5\n"
            "tell application \"System Events\"\n"
            "   tell process \"WhatsApp\"\n"
            "       set frontmost to true\n"
            "       keystroke \"v\" using command down\n"
            "       delay 1.0\n"
            "       keystroke return\n"
            "   end tell\n"
            "end tell"
        ).arg(filePath, cleanPhone);
    } else {
        script = QString(
            "set the clipboard to (POSIX file \"%1\")\n"
            "tell application \"WhatsApp\" to activate\n"
            "delay 1.0\n"
            "tell application \"System Events\"\n"
            "   tell process \"WhatsApp\"\n"
            "       set frontmost to true\n"
            "       keystroke \"v\" using command down\n"
            "       delay 1.0\n"
            "       keystroke return\n"
            "   end tell\n"
            "end tell"
        ).arg(filePath);
    }

    QProcess::startDetached("osascript", QStringList() << "-e" << script);
    return true;
}
#else
bool MacNativeShare::shareFile(const QString &) { return false; }
bool MacNativeShare::copyFileToClipboard(const QString &) { return false; }
bool MacNativeShare::attachAndSendWhatsApp(const QString &, const QString &, const QString &) { return false; }
#endif
