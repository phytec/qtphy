#ifndef MULTIMEDIA_FORMATS_HPP
#define MULTIMEDIA_FORMATS_HPP

#include <QQuickItem>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <gst/gst.h>

struct ConversionData {
    GstElement *pipeline;
    GstElement *source;
    GstElement *decoder;
    GstElement *encoder;
    GstElement *sink;
    QStringList videoCodecs;
    QStringList audioCodecs;
};

struct CodecData {
    guint encodeRank;
    guint decodeRank;
    bool hardwareEncode;
    bool hardwareDecode;
    bool encode;
    bool decode;
};

struct ContainerData {
    QSet<QString> encodeCodecs;
    QSet<QString> decodeCodecs;
};

class MultimediaFormats : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool converting READ isConverting NOTIFY convertingChanged)

private:
    ConversionData convData;
    QMap<QString, CodecData> codecs;
    QMap<QString, ContainerData> containers;
    void findCodecs();
    void cleanConvPipeline();

public:
    explicit MultimediaFormats(QObject *parent = nullptr, int argc = 0, char *argv[] = nullptr);
    ~MultimediaFormats();
    bool isConverting();

public slots:
    void convertFile(QString sourceUri, QString destinationUri, QString containerFormat, QStringList videoCodecs, QStringList audioCodecs);
    gint64 getConvertDuration();
    gint64 getConvertPosition();
    bool isHardwareCodec(QString codec, bool encode);
    QString format(QString codec, QString prefix = "video/");
    QString formatWithDeco(QString codec, bool encode, QString prefix = "video/");
    QStringList formatList(QStringList codecs, QString prefix = "video/");
    int compareEncodeCodecs(QString codec1, QString codec2);
    guint getCodecRank(QString codec, bool encode);
    int getFileDiscoverResult(QString uri);
    QString getFileFormat(QString uri);
    QStringList getFileVideoCodec(QString uri);
    QStringList getFileAudioCodec(QString uri);
    QStringList getEncodeCodecs(bool onlyHardware = false, bool onlySoftware = false,
                                QString prefix = QString(), QString containerFormat = QString());
    QStringList getDecodeCodecs(bool onlyHardware = false, bool onlySoftware = false,
                                QString prefix = QString(), QString containerFormat = QString());
    QStringList getContainerFormats(QStringList codecs = QStringList(), bool all = false, bool encode = true);
    QStringList getExtensions(QString containerFormat);

signals:
    void convertingChanged();
    void conversionError(QString message);
};

#endif // MULTIMEDIA_FORMATS_HPP
