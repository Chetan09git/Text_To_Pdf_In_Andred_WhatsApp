#ifndef PDFGENERATOR_H
#define PDFGENERATOR_H

#include <QObject>
#include <QString>

class PdfGenerator : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isGenerating READ isGenerating NOTIFY isGeneratingChanged)
    Q_PROPERTY(QString lastGeneratedFilePath READ lastGeneratedFilePath NOTIFY lastGeneratedFilePathChanged)

public:
    explicit PdfGenerator(QObject *parent = nullptr);

    bool isGenerating() const;
    QString lastGeneratedFilePath() const;

    Q_INVOKABLE QString generatePdf(const QString &name,
                                    const QString &phone,
                                    const QString &address,
                                    const QString &additionalDetails);

signals:
    void isGeneratingChanged();
    void lastGeneratedFilePathChanged();
    void pdfGenerated(const QString &filePath);
    void generationFailed(const QString &errorMessage);

private:
    bool m_isGenerating;
    QString m_lastGeneratedFilePath;

    QString getOutputDirectory() const;
    QString buildHtmlContent(const QString &name,
                             const QString &phone,
                             const QString &address,
                             const QString &additionalDetails) const;
};

#endif // PDFGENERATOR_H
