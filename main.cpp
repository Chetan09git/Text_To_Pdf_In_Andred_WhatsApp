#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "services/PdfGenerator.h"
#include "services/WhatsAppShare.h"

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);
    app.setApplicationName("TextToPDFWhatsApp");
    app.setOrganizationName("TextToPDFWhatsApp");

    PdfGenerator pdfGenerator;
    WhatsAppShare whatsAppShare;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("pdfGenerator", &pdfGenerator);
    engine.rootContext()->setContextProperty("whatsAppShare", &whatsAppShare);

    const QUrl url(QStringLiteral("qrc:/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return QGuiApplication::exec();
}
