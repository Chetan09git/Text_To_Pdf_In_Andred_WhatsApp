#include "PdfGenerator.h"

#include <QPdfWriter>
#include <QPainter>
#include <QTextDocument>
#include <QDateTime>
#include <QRegularExpression>
#include <QDir>
#include <QStandardPaths>
#include <QPageSize>
#include <QPageLayout>
#include <QDebug>

PdfGenerator::PdfGenerator(QObject *parent)
    : QObject(parent)
    , m_isGenerating(false)
    , m_lastGeneratedFilePath("")
{
}

bool PdfGenerator::isGenerating() const
{
    return m_isGenerating;
}

QString PdfGenerator::lastGeneratedFilePath() const
{
    return m_lastGeneratedFilePath;
}

QString PdfGenerator::getOutputDirectory() const
{
    QString outputDir;

#if defined(Q_OS_ANDROID)
    outputDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (outputDir.isEmpty() || !QDir(outputDir).exists()) {
        outputDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    }
    if (outputDir.isEmpty() || !QDir(outputDir).exists()) {
        outputDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
#else
    outputDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    if (outputDir.isEmpty() || !QDir(outputDir).exists()) {
        outputDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/Downloads";
    }
    if (outputDir.isEmpty() || !QDir(outputDir).exists()) {
        outputDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    }
#endif

    if (outputDir.isEmpty()) {
        outputDir = QDir::tempPath();
    }

    QDir dir(outputDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    return outputDir;
}

QString PdfGenerator::buildHtmlContent(const QString &name,
                                       const QString &phone,
                                       const QString &address,
                                       const QString &additionalDetails) const
{
    QString currentDateTime = QDateTime::currentDateTime().toString("dd MMM yyyy, hh:mm AP");

    QString html;
    html += "<!DOCTYPE html><html><head><style>";
    html += "body { font-family: 'Helvetica Neue', Helvetica, Arial, sans-serif; margin: 30px; color: #2C3E50; }";
    html += ".header { text-align: center; border-bottom: 3px solid #128C7E; padding-bottom: 15px; margin-bottom: 25px; }";
    html += ".header h1 { color: #075E54; margin: 0; font-size: 26px; text-transform: uppercase; letter-spacing: 1px; }";
    html += ".header p { color: #7F8C8D; margin: 5px 0 0 0; font-size: 13px; }";
    html += ".section-title { font-size: 16px; font-weight: bold; color: #128C7E; margin-top: 20px; margin-bottom: 10px; border-bottom: 1px solid #E0E0E0; padding-bottom: 5px; }";
    html += ".info-table { width: 100%; border-collapse: collapse; margin-bottom: 20px; }";
    html += ".info-table td { padding: 10px 12px; font-size: 14px; vertical-align: top; }";
    html += ".info-table .label { font-weight: bold; color: #34495E; width: 35%; background-color: #F8F9FA; border-radius: 4px; }";
    html += ".info-table .value { color: #2C3E50; }";
    html += ".details-box { background-color: #F8F9FA; border-left: 4px solid #25D366; padding: 15px; border-radius: 4px; font-size: 14px; line-height: 1.6; color: #2C3E50; min-height: 80px; }";
    html += ".footer { margin-top: 40px; text-align: center; font-size: 11px; color: #95A5A6; border-top: 1px solid #ECEFF1; padding-top: 12px; }";
    html += "</style></head><body>";

    html += "<div class='header'>";
    html += "  <h1>Personal Details</h1>";
    html += "  <p>Generated via Text to PDF WhatsApp Application</p>";
    html += "</div>";

    html += "<div class='section-title'>Contact Information</div>";
    html += "<table class='info-table'>";
    html += "  <tr><td class='label'>Full Name:</td><td class='value'>" + (name.isEmpty() ? "N/A" : name.toHtmlEscaped()) + "</td></tr>";
    html += "  <tr><td class='label'>Phone Number:</td><td class='value'>" + (phone.isEmpty() ? "N/A" : phone.toHtmlEscaped()) + "</td></tr>";
    html += "  <tr><td class='label'>Address:</td><td class='value'>" + (address.isEmpty() ? "N/A" : address.toHtmlEscaped().replace("\n", "<br/>")) + "</td></tr>";
    html += "</table>";

    html += "<div class='section-title'>Additional Details</div>";
    html += "<div class='details-box'>";
    if (additionalDetails.trimmed().isEmpty()) {
        html += "<em>No additional details provided.</em>";
    } else {
        html += additionalDetails.toHtmlEscaped().replace("\n", "<br/>");
    }
    html += "</div>";

    html += "<div class='footer'>";
    html += "  <p>Report Generated on " + currentDateTime + "</p>";
    html += "</div>";

    html += "</body></html>";

    return html;
}

QString PdfGenerator::generatePdf(const QString &name,
                                  const QString &phone,
                                  const QString &address,
                                  const QString &additionalDetails)
{
    m_isGenerating = true;
    emit isGeneratingChanged();

    if (name.trimmed().isEmpty()) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        emit generationFailed("Name is required.");
        return "";
    }

    if (phone.trimmed().isEmpty()) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        emit generationFailed("Phone number is required.");
        return "";
    }

    QString outputDirectory = getOutputDirectory();
    QString sanitizedName = name.trimmed().toLower().replace(QRegularExpression("[^a-z0-9]"), "_");
    if (sanitizedName.isEmpty()) {
        sanitizedName = "personal_details";
    }
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString filename = QString("%1_%2.pdf").arg(sanitizedName, timestamp);
    QString localFilePath = QDir(outputDirectory).filePath(filename);

    QPdfWriter pdfWriter(localFilePath);
    pdfWriter.setPageSize(QPageSize(QPageSize::A4));
    pdfWriter.setPageOrientation(QPageLayout::Portrait);
    pdfWriter.setResolution(300);

    QPageLayout layout = pdfWriter.pageLayout();
    layout.setMargins(QMarginsF(15, 15, 15, 15));
    pdfWriter.setPageLayout(layout);

    QTextDocument document;
    document.setHtml(buildHtmlContent(name, phone, address, additionalDetails));
    document.setPageSize(QSizeF(pdfWriter.width(), pdfWriter.height()));

    QPainter painter;
    if (!painter.begin(&pdfWriter)) {
        m_isGenerating = false;
        emit isGeneratingChanged();
        emit generationFailed(QString("Failed to initialize PDF writer for path: %1").arg(localFilePath));
        return "";
    }

    document.drawContents(&painter);
    painter.end();

    m_isGenerating = false;
    emit isGeneratingChanged();

    m_lastGeneratedFilePath = localFilePath;
    emit lastGeneratedFilePathChanged();
    emit pdfGenerated(localFilePath);

    qDebug() << "PDF successfully generated at:" << localFilePath;
    return localFilePath;
}
