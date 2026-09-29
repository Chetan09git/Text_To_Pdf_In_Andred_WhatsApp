#include "WhatsAppShare.h"
#if defined(Q_OS_MACOS)
#include "MacNativeShare.h"
#endif

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QUrl>
#include <QDesktopServices>
#include <QRegularExpression>
#include <QDebug>
#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QProcess>
#include <QDrag>
#include <QPainter>
#include <QPixmap>
#include <QFont>
#include <QWindow>

#if defined(Q_OS_ANDROID)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QJniObject>
#include <QJniEnvironment>
#include <QCoreApplication>
#else
#include <QtAndroid>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
typedef QAndroidJniObject QJniObject;
typedef QAndroidJniEnvironment QJniEnvironment;
#endif

static bool clearJniExceptions()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QJniEnvironment env;
    if (env.checkAndClearExceptions()) {
        qWarning() << "WhatsAppShare: Cleared JNI exception in Qt6.";
        return true;
    }
#else
    QAndroidJniEnvironment env;
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        qWarning() << "WhatsAppShare: Cleared JNI exception in Qt5.";
        return true;
    }
#endif
    return false;
}
#endif

WhatsAppShare::WhatsAppShare(QObject *parent)
    : QObject(parent)
    , m_isSharing(false)
{
}

bool WhatsAppShare::isSharing() const
{
    return m_isSharing;
}

void WhatsAppShare::copyToClipboard(const QString &text)
{
    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        clipboard->setText(text);
        qDebug() << "WhatsAppShare: Copied to clipboard ->" << text;
    }
}

bool WhatsAppShare::sharePdf(const QString &filePath, const QString &customMessage, const QString &phoneNumber)
{
    m_isSharing = true;
    emit isSharingChanged();

    QString cleanPath = filePath.trimmed();
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }

    if (cleanPath.isEmpty() || (!cleanPath.startsWith("content://") && !QFile::exists(cleanPath))) {
        m_isSharing = false;
        emit isSharingChanged();
        emit shareError("PDF file does not exist or invalid path.");
        return false;
    }

    QString cleanPhone = phoneNumber;
    cleanPhone.remove(QRegularExpression("[^0-9]"));

#if !defined(Q_OS_ANDROID)
    if (!cleanPhone.isEmpty()) {
        copyToClipboard(cleanPhone);
    }
#endif

#if defined(Q_OS_ANDROID)
    bool result = shareOnAndroid(cleanPath, customMessage, cleanPhone);
#else
    bool result = shareOnDesktop(cleanPath, customMessage, cleanPhone);
#endif

    m_isSharing = false;
    emit isSharingChanged();

    if (result) {
        emit shareSuccess();
    }
    return result;
}

bool WhatsAppShare::sendTextMessage(const QString &phoneNumber, const QString &message)
{
    m_isSharing = true;
    emit isSharingChanged();

    QString cleanPhone = phoneNumber;
    cleanPhone.remove(QRegularExpression("[^0-9]"));

    if (cleanPhone.isEmpty()) {
        m_isSharing = false;
        emit isSharingChanged();
        emit shareError("Please enter a valid phone number.");
        return false;
    }

#if !defined(Q_OS_ANDROID)
    copyToClipboard(cleanPhone);
#endif

#if defined(Q_OS_ANDROID)
    clearJniExceptions();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QJniObject context = QNativeInterface::QAndroidApplication::context();
#else
    QJniObject context = QtAndroid::androidActivity();
    if (!context.isValid()) {
        context = QtAndroid::androidContext();
    }
#endif
    if (!context.isValid()) {
        m_isSharing = false;
        emit isSharingChanged();
        emit shareError("Android application context is not valid.");
        return false;
    }

    QString urlString = QString("https://api.whatsapp.com/send?phone=%1&text=%2")
        .arg(cleanPhone, QString::fromUtf8(QUrl::toPercentEncoding(message)));
    QJniObject javaUriString = QJniObject::fromString(urlString);
    QJniObject uri = QJniObject::callStaticObjectMethod(
        "android/net/Uri",
        "parse",
        "(Ljava/lang/String;)Landroid/net/Uri;",
        javaUriString.object<jstring>()
    );
    clearJniExceptions();

    QJniObject actionView = QJniObject::getStaticObjectField(
        "android/content/Intent",
        "ACTION_VIEW",
        "Ljava/lang/String;"
    );
    clearJniExceptions();

    QJniObject intent("android/content/Intent", "(Ljava/lang/String;Landroid/net/Uri;)V", actionView.object<jstring>(), uri.object<jobject>());
    clearJniExceptions();

    QJniObject whatsAppPkg = QJniObject::fromString("com.whatsapp");
    intent.callObjectMethod("setPackage", "(Ljava/lang/String;)Landroid/content/Intent;", whatsAppPkg.object<jstring>());
    clearJniExceptions();

    intent.callObjectMethod("addFlags", "(I)Landroid/content/Intent;", (jint)0x10000000); // FLAG_ACTIVITY_NEW_TASK
    clearJniExceptions();

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QtAndroid::startActivity(intent, 0);
#else
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([intent]() {
        QJniObject ctx = QNativeInterface::QAndroidApplication::context();
        if (ctx.isValid() && intent.isValid()) {
            ctx.callMethod<void>("startActivity", "(Landroid/content/Intent;)V", intent.object<jobject>());
        }
    });
#endif

    if (clearJniExceptions()) {
        // Fallback without package restriction
        intent.callObjectMethod("setPackage", "(Ljava/lang/String;)Landroid/content/Intent;", QJniObject().object<jstring>());
        clearJniExceptions();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        QtAndroid::startActivity(intent, 0);
#else
        QNativeInterface::QAndroidApplication::runOnAndroidMainThread([intent]() {
            QJniObject ctx = QNativeInterface::QAndroidApplication::context();
            if (ctx.isValid() && intent.isValid()) {
                ctx.callMethod<void>("startActivity", "(Landroid/content/Intent;)V", intent.object<jobject>());
            }
        });
#endif
        clearJniExceptions();
    }

    m_isSharing = false;
    emit isSharingChanged();
    emit shareSuccess();
    return true;
#else
    QString encodedMsg = QString::fromUtf8(QUrl::toPercentEncoding(message));
    QUrl appUrl(QString("whatsapp://send?phone=%1&text=%2").arg(cleanPhone, encodedMsg));
    
    bool opened = QDesktopServices::openUrl(appUrl);
    if (!opened) {
        QUrl webUrl(QString("https://web.whatsapp.com/send?phone=%1&text=%2").arg(cleanPhone, encodedMsg));
        opened = QDesktopServices::openUrl(webUrl);
    }

    m_isSharing = false;
    emit isSharingChanged();
    if (opened) {
        emit shareSuccess();
    } else {
        emit shareError("Unable to open WhatsApp.");
    }
    return opened;
#endif
}

#if defined(Q_OS_ANDROID)
bool WhatsAppShare::shareOnAndroid(const QString &filePath, const QString &customMessage, const QString &phoneNumber)
{
    clearJniExceptions();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QJniObject context = QNativeInterface::QAndroidApplication::context();
#else
    QJniObject context = QtAndroid::androidActivity();
    if (!context.isValid()) {
        context = QtAndroid::androidContext();
    }
#endif

    if (!context.isValid()) {
        emit shareError("Android application context is not valid.");
        return false;
    }

    QString resolvedPath = QFileInfo(filePath).canonicalFilePath();
    if (resolvedPath.isEmpty()) {
        resolvedPath = filePath;
    }

    QJniObject uri;
    if (resolvedPath.startsWith("content://")) {
        QJniObject javaUriString = QJniObject::fromString(resolvedPath);
        uri = QJniObject::callStaticObjectMethod(
            "android/net/Uri",
            "parse",
            "(Ljava/lang/String;)Landroid/net/Uri;",
            javaUriString.object<jstring>()
        );
        clearJniExceptions();
    } else {
        QJniObject javaFilePath = QJniObject::fromString(resolvedPath);
        QJniObject file("java/io/File", "(Ljava/lang/String;)V", javaFilePath.object<jstring>());
        if (!file.isValid() || clearJniExceptions()) {
            emit shareError("Unable to create Android File object for PDF.");
            return false;
        }

        QJniObject packageName = context.callObjectMethod("getPackageName", "()Ljava/lang/String;");
        clearJniExceptions();

        QString pkgNameStr = packageName.isValid() ? packageName.toString() : "org.qtproject.example.TextToPDFWhatsApp";
        QString authority = pkgNameStr + ".fileprovider";
        QJniObject javaAuthority = QJniObject::fromString(authority);

        uri = QJniObject::callStaticObjectMethod(
            "androidx/core/content/FileProvider",
            "getUriForFile",
            "(Landroid/content/Context;Ljava/lang/String;Ljava/io/File;)Landroid/net/Uri;",
            context.object<jobject>(),
            javaAuthority.object<jstring>(),
            file.object<jobject>()
        );

        if (clearJniExceptions() || !uri.isValid()) {
            qWarning() << "FileProvider getUriForFile failed for path:" << resolvedPath;
            emit shareError("FileProvider could not generate a secure sharing URI.");
            return false;
        }
    }

    if (!uri.isValid()) {
        emit shareError("Failed to obtain valid URI for sharing PDF.");
        return false;
    }

    // Explicitly grant URI permission to WhatsApp and WhatsApp Business
    context.callMethod<void>(
        "grantUriPermission",
        "(Ljava/lang/String;Landroid/net/Uri;I)V",
        QJniObject::fromString("com.whatsapp").object<jstring>(),
        uri.object<jobject>(),
        (jint)1 // FLAG_GRANT_READ_URI_PERMISSION
    );
    clearJniExceptions();

    context.callMethod<void>(
        "grantUriPermission",
        "(Ljava/lang/String;Landroid/net/Uri;I)V",
        QJniObject::fromString("com.whatsapp.w4b").object<jstring>(),
        uri.object<jobject>(),
        (jint)1
    );
    clearJniExceptions();

    QJniObject actionSend = QJniObject::getStaticObjectField(
        "android/content/Intent",
        "ACTION_SEND",
        "Ljava/lang/String;"
    );
    clearJniExceptions();

    QJniObject intent("android/content/Intent", "(Ljava/lang/String;)V", actionSend.object<jstring>());
    clearJniExceptions();

    QJniObject mimeType = QJniObject::fromString("application/pdf");
    intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;", mimeType.object<jstring>());
    clearJniExceptions();

    QJniObject extraStream = QJniObject::getStaticObjectField(
        "android/content/Intent",
        "EXTRA_STREAM",
        "Ljava/lang/String;"
    );
    clearJniExceptions();

    intent.callObjectMethod(
        "putExtra",
        "(Ljava/lang/String;Landroid/os/Parcelable;)Landroid/content/Intent;",
        extraStream.object<jstring>(),
        uri.object<jobject>()
    );
    clearJniExceptions();

    if (!customMessage.isEmpty()) {
        QJniObject extraText = QJniObject::getStaticObjectField(
            "android/content/Intent",
            "EXTRA_TEXT",
            "Ljava/lang/String;"
        );
        clearJniExceptions();
        QJniObject javaMessage = QJniObject::fromString(customMessage);
        intent.callObjectMethod(
            "putExtra",
            "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
            extraText.object<jstring>(),
            javaMessage.object<jstring>()
        );
        clearJniExceptions();
    }

    if (!phoneNumber.isEmpty()) {
        QString clean = phoneNumber;
        clean.remove(QRegularExpression("[^0-9]"));
        if (!clean.isEmpty()) {
            // Target specific WhatsApp contact directly using JID (<country_code><number>@s.whatsapp.net)
            QString jid = clean + "@s.whatsapp.net";
            QJniObject jidKey = QJniObject::fromString("jid");
            QJniObject jidVal = QJniObject::fromString(jid);
            intent.callObjectMethod(
                "putExtra",
                "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                jidKey.object<jstring>(),
                jidVal.object<jstring>()
            );
            clearJniExceptions();

            QJniObject phoneKey = QJniObject::fromString("android.intent.extra.PHONE_NUMBER");
            QJniObject phoneVal = QJniObject::fromString(clean);
            intent.callObjectMethod(
                "putExtra",
                "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
                phoneKey.object<jstring>(),
                phoneVal.object<jstring>()
            );
            clearJniExceptions();
        }
    }

    // Set ClipData for Android 10+ URI permission grant
    QJniObject clipData = QJniObject::callStaticObjectMethod(
        "android/content/ClipData",
        "newRawUri",
        "(Ljava/lang/CharSequence;Landroid/net/Uri;)Landroid/content/ClipData;",
        QJniObject::fromString("PDF").object<jstring>(),
        uri.object<jobject>()
    );
    clearJniExceptions();

    if (clipData.isValid()) {
        intent.callMethod<void>(
            "setClipData",
            "(Landroid/content/ClipData;)V",
            clipData.object<jobject>()
        );
        clearJniExceptions();
    }

    // Grant read permission (1 = FLAG_GRANT_READ_URI_PERMISSION, 0x10000000 = FLAG_ACTIVITY_NEW_TASK)
    const jint flags = 1 | 0x10000000;
    intent.callObjectMethod("addFlags", "(I)Landroid/content/Intent;", flags);
    clearJniExceptions();

    // Check if WhatsApp is directly available
    QJniObject packageManager = context.callObjectMethod("getPackageManager", "()Landroid/content/pm/PackageManager;");
    clearJniExceptions();

    QJniObject whatsAppPkg = QJniObject::fromString("com.whatsapp");
    intent.callObjectMethod("setPackage", "(Ljava/lang/String;)Landroid/content/Intent;", whatsAppPkg.object<jstring>());
    clearJniExceptions();

    QJniObject resolveInfo;
    if (packageManager.isValid()) {
        resolveInfo = packageManager.callObjectMethod(
            "resolveActivity",
            "(Landroid/content/Intent;I)Landroid/content/pm/ResolveInfo;",
            intent.object<jobject>(),
            (jint)0
        );
        clearJniExceptions();

        if (!resolveInfo.isValid()) {
            QJniObject w4bPkg = QJniObject::fromString("com.whatsapp.w4b");
            intent.callObjectMethod("setPackage", "(Ljava/lang/String;)Landroid/content/Intent;", w4bPkg.object<jstring>());
            clearJniExceptions();
            resolveInfo = packageManager.callObjectMethod(
                "resolveActivity",
                "(Landroid/content/Intent;I)Landroid/content/pm/ResolveInfo;",
                intent.object<jobject>(),
                (jint)0
            );
            clearJniExceptions();
        }
    }

    QJniObject targetIntent;
    if (resolveInfo.isValid()) {
        targetIntent = intent;
    } else {
        // Fallback to chooser without package lock
        intent.callObjectMethod("setPackage", "(Ljava/lang/String;)Landroid/content/Intent;", QJniObject().object<jstring>());
        clearJniExceptions();

        QJniObject chooserTitle = QJniObject::fromString("Share PDF via...");
        targetIntent = QJniObject::callStaticObjectMethod(
            "android/content/Intent",
            "createChooser",
            "(Landroid/content/Intent;Ljava/lang/CharSequence;)Landroid/content/Intent;",
            intent.object<jobject>(),
            chooserTitle.object<jstring>()
        );
        clearJniExceptions();
        if (targetIntent.isValid()) {
            targetIntent.callObjectMethod("addFlags", "(I)Landroid/content/Intent;", (jint)0x10000000);
            clearJniExceptions();
        } else {
            targetIntent = intent;
        }
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QtAndroid::startActivity(targetIntent, 0);
#else
    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([targetIntent]() {
        QJniObject ctx = QNativeInterface::QAndroidApplication::context();
        if (ctx.isValid() && targetIntent.isValid()) {
            ctx.callMethod<void>("startActivity", "(Landroid/content/Intent;)V", targetIntent.object<jobject>());
        }
    });
#endif

    if (clearJniExceptions()) {
        emit shareError("Error launching WhatsApp sharing activity.");
        return false;
    }

    return true;
}
#endif

bool WhatsAppShare::shareOnDesktop(const QString &filePath, const QString &customMessage, const QString &phoneNumber)
{
    qDebug() << "Sharing on desktop platform. Opening PDF file:" << filePath;
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    QClipboard *clipboard = QGuiApplication::clipboard();
    if (clipboard) {
        QMimeData *mimeData = new QMimeData();
        mimeData->setUrls(QList<QUrl>() << fileUrl);
        mimeData->setText(filePath);
        clipboard->setMimeData(mimeData);
    }

    QString cleanPhone = phoneNumber;
    cleanPhone.remove(QRegularExpression("[^0-9]"));

#if defined(Q_OS_MACOS)
    MacNativeShare::copyFileToClipboard(filePath);
    return MacNativeShare::attachAndSendWhatsApp(filePath, cleanPhone, customMessage);
#else
    QString urlStr;
    if (!cleanPhone.isEmpty()) {
        urlStr = QString("https://api.whatsapp.com/send?phone=%1").arg(cleanPhone);
        if (!customMessage.isEmpty()) {
            urlStr += QString("&text=%1").arg(QString::fromUtf8(QUrl::toPercentEncoding(customMessage)));
        }
    } else {
        urlStr = "https://web.whatsapp.com";
    }
    return QDesktopServices::openUrl(QUrl(urlStr));
#endif
}

void WhatsAppShare::startFileDrag(const QString &filePath)
{
    QString cleanPath = filePath.trimmed();
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }

    if (!QFile::exists(cleanPath)) {
        return;
    }

    QWindow *parentWindow = QGuiApplication::focusWindow();
    const auto topWindows = QGuiApplication::allWindows();
    if (!parentWindow && !topWindows.isEmpty()) {
        parentWindow = topWindows.constFirst();
    }

    QDrag *drag = new QDrag(parentWindow);
    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(cleanPath));
    mimeData->setText(cleanPath);
    drag->setMimeData(mimeData);

    QPixmap pixmap(160, 48);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setBrush(QColor(0x07, 0x5E, 0x54));
    painter.setPen(QPen(QColor(0x25, 0xD3, 0x66), 2));
    painter.drawRoundedRect(1, 1, 158, 46, 10, 10);

    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setBold(true);
    font.setPixelSize(12);
    painter.setFont(font);
    painter.drawText(QRect(12, 0, 140, 48), Qt::AlignVCenter | Qt::AlignLeft, "📄 Drop in WhatsApp");
    painter.end();

    drag->setPixmap(pixmap);
    drag->setHotSpot(QPoint(80, 24));
    drag->exec(Qt::CopyAction);
}

bool WhatsAppShare::revealInFinder(const QString &filePath)
{
    QString cleanPath = filePath.trimmed();
    if (cleanPath.startsWith("file://")) {
        cleanPath = QUrl(cleanPath).toLocalFile();
    }
    if (!QFile::exists(cleanPath)) {
        return false;
    }

#if defined(Q_OS_MACOS)
    QStringList args;
    args << "-e" << QString("tell application \"Finder\" to reveal POSIX file \"%1\"").arg(cleanPath);
    args << "-e" << "tell application \"Finder\" to activate";
    return QProcess::startDetached("osascript", args);
#elif defined(Q_OS_WIN)
    return QProcess::startDetached("explorer.exe", QStringList() << "/select," << QDir::toNativeSeparators(cleanPath));
#else
    QFileInfo fileInfo(cleanPath);
    return QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
#endif
}
