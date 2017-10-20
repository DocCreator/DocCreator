#ifndef RANDOMDOCUMENTEXPORTER_HPP
#define RANDOMDOCUMENTEXPORTER_HPP

#include <QObject>
class QImage;
class DocumentController;

class RandomDocumentExporter : public QObject
{
  Q_OBJECT

public:
  explicit RandomDocumentExporter(DocumentController *ctrl,
                                  const QString &path,
                                  QObject *parent = 0);

  static int _nbDocWritten;

  /*
    Width used to save number in filenames.
    If set to 0, no leading zeros will be added.

    For example for width=5, 193 will be written as 00193.
    For width=0, 193 will be written as 193.
   */
  void setNumberWidth(int width);

signals:
  void imageSaved(const QString &imageFilename);
  void propertiesSaved(const QString &filename);

public slots:

  void saveRandomDocument();

  void writeImage(const QImage &image);
  void saveProperties(const QMap<QString, QString> &properties);
  void saveTextFile();

private:
  QString getFilePath(const QString &prefix, const QString &extension) const;
  QString getDocImageFilePath() const;

private:
  QString _path;

  DocumentController *_ctrl;

  int _numberWidth;
};

#endif // RANDOMDOCUMENTEXPORTER_HPP
