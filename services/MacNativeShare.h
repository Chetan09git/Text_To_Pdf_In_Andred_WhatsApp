#ifndef MACNATIVESHARE_H
#define MACNATIVESHARE_H

#include <QString>

class MacNativeShare
{
public:
    static bool shareFile(const QString &filePath);
    static bool copyFileToClipboard(const QString &filePath);
    static bool attachAndSendWhatsApp(const QString &filePath, const QString &phoneNumber, const QString &message);
};

#endif // MACNATIVESHARE_H
