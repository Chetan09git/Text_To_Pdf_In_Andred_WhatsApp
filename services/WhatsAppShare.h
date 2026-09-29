#ifndef WHATSAPPSHARE_H
#define WHATSAPPSHARE_H

#include <QObject>
#include <QString>

class WhatsAppShare : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isSharing READ isSharing NOTIFY isSharingChanged)

public:
    explicit WhatsAppShare(QObject *parent = nullptr);

    bool isSharing() const;

    Q_INVOKABLE bool sharePdf(const QString &filePath, const QString &customMessage = "", const QString &phoneNumber = "");
    Q_INVOKABLE bool sendTextMessage(const QString &phoneNumber, const QString &message);
    Q_INVOKABLE void copyToClipboard(const QString &text);
    Q_INVOKABLE void startFileDrag(const QString &filePath);
    Q_INVOKABLE bool revealInFinder(const QString &filePath);

signals:
    void isSharingChanged();
    void shareSuccess();
    void shareError(const QString &errorMessage);

private:
    bool m_isSharing;

#if defined(Q_OS_ANDROID)
    bool shareOnAndroid(const QString &filePath, const QString &customMessage, const QString &phoneNumber);
#endif
    bool shareOnDesktop(const QString &filePath, const QString &customMessage, const QString &phoneNumber);
};

#endif // WHATSAPPSHARE_H
