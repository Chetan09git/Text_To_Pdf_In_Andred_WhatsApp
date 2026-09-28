#include "WhatsAppShare.h"
#include "MacNativeShare.h"

#include <QFile>
#include <QFileInfo>
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

    // Extract digits for phone validation & clipboard
    QString cleanPhone = phoneNumber;
    cleanPhone.remove(QRegularExpression("[^0-9]"));

    if (!cleanPhone.isEmpty()) {
        // Automatically copy phone number to clipboard on share tap
        copyToClipboard(cleanPhone);
    }

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

#if defined(Q_OS_ANDROID)
bool WhatsAppShare::shareOnAndroid(const QString &filePath, const QString &customMessage, const QString &phoneNumber)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    QJniObject context = QNativeInterface::QAndroidApplication::context();
#else
    QJniObject context = QtAndroid::androidContext();
#endif

    if (!context.isValid()) {
        emit shareError("Android application context is not valid.");
        return false;
    }

    QJniObject uri;
    if (filePath.startsWith("content://")) {
        QJniObject javaUriString = QJniObject::fromString(filePath);
        uri = QJniObject::callStaticObjectMethod(
            "android/net/Uri",
            "parse",
            "(Ljava/lang/String;)Landroid/net/Uri;",
            javaUriString.object<jstring>()
        );
    } else {
        // 1. Create java.io.File object
        QJniObject javaFilePath = QJniObject::fromString(filePath);
        QJniObject file("java/io/File", "(Ljava/lang/String;)V", javaFilePath.object<jstring>());
        if (!file.isValid()) {
            emit shareError("Unable to create Android File object.");
            return false;
        }

        // 2. Get Package Name and FileProvider authority
        QJniObject packageName = context.callObjectMethod("getPackageName", "()Ljava/lang/String;");
        QString authority = packageName.toString() + ".fileprovider";
        QJniObject javaAuthority = QJniObject::fromString(authority);

        // 3. Get Content URI via androidx.core.content.FileProvider
        uri = QJniObject::callStaticObjectMethod(
            "androidx/core/content/FileProvider",
            "getUriForFile",
            "(Landroid/content/Context;Ljava/lang/String;Ljava/io/File;)Landroid/net/Uri;",
            context.object<jobject>(),
            javaAuthority.object<jstring>(),
            file.object<jobject>()
        );
    }

    if (!uri.isValid()) {
        emit shareError("Failed to obtain FileProvider URI for sharing.");
        return false;
    }

    // 4. Create ACTION_SEND Intent
    QJniObject actionSend = QJniObject::getStaticObjectField(
        "android/content/Intent",
        "ACTION_SEND",
        "Ljava/lang/String;"
    );

    QJniObject intent("android/content/Intent", "(Ljava/lang/String;)V", actionSend.object<jstring>());

    // 5. Set MIME Type to application/pdf
    QJniObject mimeType = QJniObject::fromString("application/pdf");
    intent.callObjectMethod("setType", "(Ljava/lang/String;)Landroid/content/Intent;", mimeType.object<jstring>());

    // 6. Attach Stream URI
    QJniObject extraStream = QJniObject::getStaticObjectField(
        "android/content/Intent",
        "EXTRA_STREAM",
        "Ljava/lang/String;"
    );
    intent.callObjectMethod(
        "putExtra",
        "(Ljava/lang/String;Landroid/os/Parcelable;)Landroid/content/Intent;",
        extraStream.object<jstring>(),
        uri.object<jobject>()
    );

    // 7. Attach optional text message
    if (!customMessage.isEmpty()) {
        QJniObject extraText = QJniObject::getStaticObjectField(
            "android/content/Intent",
            "EXTRA_TEXT",
            "Ljava/lang/String;"
        );
        QJniObject javaMessage = QJniObject::fromString(customMessage);
        intent.callObjectMethod(
            "putExtra",
            "(Ljava/lang/String;Ljava/lang/String;)Landroid/content/Intent;",
            extraText.object<jstring>(),
            javaMessage.object<jstring>()
        );
    }

    // 8. Set ClipData for URI permission grant (Required for Android 7.0+)
    QJniObject clipData = QJniObject::callStaticObjectMethod(
        "android/content/ClipData",
        "newRawUri",
        "(Ljava/lang/CharSequence;Landroid/net/Uri;)Landroid/content/ClipData;",
        QJniObject::fromString("PDF").object<jstring>(),
        uri.object<jobject>()
    );
    if (clipData.isValid()) {
        intent.callObjectMethod(
            "setClipData",
            "(Landroid/content/ClipData;)V",
            clipData.object<jobject>()
        );
    }

    // 9. Grant Read URI Permission & New Task flags
    jint flagGrantRead = QJniObject::getStaticField<jint>(
        "android/content/Intent",
        "FLAG_GRANT_READ_URI_PERMISSION"
    );
    jint flagNewTask = QJniObject::getStaticField<jint>(
        "android/content/Intent",
        "FLAG_ACTIVITY_NEW_TASK"
    );
    intent.callObjectMethod("addFlags", "(I)Landroid/content/Intent;", flagGrantRead | flagNewTask);

    // 10. Check if WhatsApp or WhatsApp Business is installed
    QJniObject packageManager = context.callObjectMethod("getPackageManager", "()Landroid/content/pm/PackageManager;");
    
    QJniObject whatsAppPkg = QJniObject::fromString("com.whatsapp");
    intent.callObjectMethod("setPackage", "(Ljava/lang/String;)Landroid/content/Intent;", whatsAppPkg.object<jstring>());
    
    QJniObject resolveInfo = packageManager.callObjectMethod(
        "resolveActivity",
        "(Landroid/content/Intent;I)Landroid/content/pm/ResolveInfo;",
        intent.object<jobject>(),
        0
    );

    if (!resolveInfo.isValid()) {
        // Try WhatsApp Business
        QJniObject w4bPkg = QJniObject::fromString("com.whatsapp.w4b");
        intent.callObjectMethod("setPackage", "(Ljava/lang/String;)Landroid/content/Intent;", w4bPkg.object<jstring>());
        resolveInfo = packageManager.callObjectMethod(
            "resolveActivity",
            "(Landroid/content/Intent;I)Landroid/content/pm/ResolveInfo;",
            intent.object<jobject>(),
            0
        );
    }

    if (resolveInfo.isValid()) {
        context.callMethod<void>("startActivity", "(Landroid/content/Intent;)V", intent.object<jobject>());
    } else {
        // Fallback: Clear package to let user choose any available sharing app (e.g., Drive, Gmail, Bluetooth)
        QJniObject nullString;
        intent.callObjectMethod("setPackage", "(Ljava/lang/String;)Landroid/content/Intent;", nullString.object<jstring>());

        QJniObject chooserTitle = QJniObject::fromString("Share PDF via...");
        QJniObject chooserIntent = QJniObject::callStaticObjectMethod(
            "android/content/Intent",
            "createChooser",
            "(Landroid/content/Intent;Ljava/lang/CharSequence;)Landroid/content/Intent;",
            intent.object<jobject>(),
            chooserTitle.object<jstring>()
        );
        chooserIntent.callObjectMethod("addFlags", "(I)Landroid/content/Intent;", flagNewTask);
        context.callMethod<void>("startActivity", "(Landroid/content/Intent;)V", chooserIntent.object<jobject>());
    }

    return true;
}
#endif

bool WhatsAppShare::shareOnDesktop(const QString &filePath, const QString &customMessage, const QString &phoneNumber)
{
    qDebug() << "Sharing on desktop platform. Opening PDF file:" << filePath;
    QUrl fileUrl = QUrl::fromLocalFile(filePath);

    // 1. Copy PDF file object to clipboard
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
    QString nativeUrl = QString("whatsapp://send?phone=%1").arg(cleanPhone);
    return QDesktopServices::openUrl(QUrl(nativeUrl));
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
    if (!parentWindow && !QGuiApplication::allWindows().isEmpty()) {
        parentWindow = QGuiApplication::allWindows().first();
    }

    QDrag *drag = new QDrag(parentWindow);
    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(cleanPath));
    mimeData->setText(cleanPath);
    drag->setMimeData(mimeData);

    // Create a drag thumbnail pixmap
    QPixmap pixmap(160, 48);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw rounded background
    painter.setBrush(QColor("#075E54"));
    painter.setPen(QPen(QColor("#25D366"), 2));
    painter.drawRoundedRect(1, 1, 158, 46, 10, 10);

    // Draw icon and text
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
    return QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(cleanPath).absolutePath()));
#endif
}

