#include "multimedia_formats.hpp"
#include <QString>
#include <QRegularExpression>
#include <QStringList>
#include <QSet>
#include <QMap>
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>

MultimediaFormats::MultimediaFormats(QObject *parent, int argc, char *argv[])
    : QObject(parent), convData() {
    gst_init (&argc, &argv);

    findCodecs();
}

void MultimediaFormats::findCodecs() {
    //Find all encodable codecs
    GList *factories = gst_element_factory_list_get_elements(GST_ELEMENT_FACTORY_TYPE_ENCODER, GST_RANK_NONE);
    for (GList *l = factories; l != NULL; l = l->next) {
        GstElementFactory *factory = GST_ELEMENT_FACTORY(l->data);
        const gchar *name = gst_plugin_feature_get_name(factory);
        guint rank = gst_plugin_feature_get_rank(GST_PLUGIN_FEATURE(factory));
        const GList *pads = gst_element_factory_get_static_pad_templates(factory);
        bool hardware = g_str_has_prefix(name, "vpu") || rank > 256
                || gst_element_factory_list_is_type(factory, GST_ELEMENT_FACTORY_TYPE_HARDWARE)
                || QString(gst_element_factory_get_metadata(factory, GST_ELEMENT_METADATA_KLASS)).contains(GST_ELEMENT_FACTORY_KLASS_HARDWARE);
        while (pads) {
            GstStaticPadTemplate *padtemplate = (GstStaticPadTemplate *) pads->data;
            if (padtemplate->direction == GST_PAD_SRC) {
                GstCaps *caps = gst_static_pad_template_get_caps(padtemplate);
                for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                    GstStructure *structure = gst_caps_get_structure(caps, i);
                    QString codecName = QString(gst_structure_get_name(structure));
                    if (codecName.endsWith("/x-raw") || codecName.startsWith("unknown/"))
                        continue;
                    CodecData &codecData = codecs[codecName];
                    codecData.encode = true;
                    if (codecData.encodeRank < rank)
                        codecData.encodeRank = rank;
                    codecData.hardwareEncode |= hardware;
                }
                gst_caps_unref(caps);
            }
            pads = pads->next;
        }
    }
    gst_plugin_feature_list_free(factories);
    //Find all decodable codecs
    factories = gst_element_factory_list_get_elements(GST_ELEMENT_FACTORY_TYPE_DECODER, GST_RANK_NONE);
    for (GList *l = factories; l != NULL; l = l->next) {
        GstElementFactory *factory = GST_ELEMENT_FACTORY(l->data);
        const gchar *name = gst_plugin_feature_get_name(factory);
        guint rank = gst_plugin_feature_get_rank(GST_PLUGIN_FEATURE(factory));
        const GList *pads = gst_element_factory_get_static_pad_templates(factory);
        bool hardware = g_str_has_prefix(name, "vpu") || rank > 256
                || gst_element_factory_list_is_type(factory, GST_ELEMENT_FACTORY_TYPE_HARDWARE)
                || QString(gst_element_factory_get_metadata(factory, GST_ELEMENT_METADATA_KLASS)).contains(GST_ELEMENT_FACTORY_KLASS_HARDWARE);
        while (pads) {
            GstStaticPadTemplate *padtemplate = (GstStaticPadTemplate *) pads->data;
            if (padtemplate->direction == GST_PAD_SINK) {
                GstCaps *caps = gst_static_pad_template_get_caps(padtemplate);
                for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                    GstStructure *structure = gst_caps_get_structure(caps, i);
                    QString codecName = QString(gst_structure_get_name(structure));
                    if (codecName.endsWith("/x-raw") || codecName.startsWith("unknown/"))
                        continue;
                    CodecData &codecData = codecs[codecName];
                    codecData.decode = true;
                    if (codecData.decodeRank < rank)
                        codecData.decodeRank = rank;
                    codecData.hardwareDecode |= hardware;
                }
                gst_caps_unref(caps);
            }
            pads = pads->next;
        }
    }
    gst_plugin_feature_list_free(factories);
    //Find all container formats for encoding and their supported codecs
    factories = gst_element_factory_list_get_elements(GST_ELEMENT_FACTORY_TYPE_MUXER, GST_RANK_NONE);
    for (GList *l = factories; l != NULL; l = l->next) {
        GstElementFactory *factory = GST_ELEMENT_FACTORY(l->data);
        bool hasVideo = false;
        QSet<QString> avaiableCodecs;
        QSet<QString> containerFormats;
        const GList *pads = gst_element_factory_get_static_pad_templates(factory);
        while (pads) {
            GstStaticPadTemplate *padtemplate = (GstStaticPadTemplate *) pads->data;
            if (padtemplate->direction == GST_PAD_SINK) {
                GstCaps *caps = gst_static_pad_template_get_caps(padtemplate);
                for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                    GstStructure *structure = gst_caps_get_structure(caps, i);
                    QString codecName = QString(gst_structure_get_name(structure));
                    if (codecs.value(codecName).encode) {
                        avaiableCodecs.insert(codecName);
                        if (codecName.startsWith("video/")) {
                            hasVideo = true;
                        }
                    }
                }
                gst_caps_unref(caps);
            } else if (padtemplate->direction == GST_PAD_SRC) {
                GstCaps *caps = gst_static_pad_template_get_caps(padtemplate);
                for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                    GstStructure *structure = gst_caps_get_structure(caps, i);
                    gchar *structureStr = gst_structure_to_string(structure);
                    containerFormats.insert(QString(structureStr).section(';', 0, 0));
                    g_free(structureStr);
                }
                gst_caps_unref(caps);
            }
            pads = pads->next;
        }
        if (!avaiableCodecs.empty() && hasVideo) {
            for (const auto& format : containerFormats) {
                containers[format].encodeCodecs = avaiableCodecs;
            }
        }
    }
    gst_plugin_feature_list_free(factories);
    //Find all container formats for decoding and their supported codecs
    factories = gst_element_factory_list_get_elements(GST_ELEMENT_FACTORY_TYPE_DEMUXER, GST_RANK_NONE);
    for (GList *l = factories; l != NULL; l = l->next) {
        GstElementFactory *factory = GST_ELEMENT_FACTORY(l->data);
        bool hasVideo = false;
        QSet<QString> avaiableCodecs;
        QSet<QString> containerFormats;
        const GList *pads = gst_element_factory_get_static_pad_templates(factory);
        while (pads) {
            GstStaticPadTemplate *padtemplate = (GstStaticPadTemplate *) pads->data;
            if (padtemplate->direction == GST_PAD_SRC) {
                GstCaps *caps = gst_static_pad_template_get_caps(padtemplate);
                for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                    GstStructure *structure = gst_caps_get_structure(caps, i);
                    QString codecName = QString(gst_structure_get_name(structure));
                    if (codecs.value(codecName).encode) {
                        avaiableCodecs.insert(codecName);
                        if (codecName.startsWith("video/")) {
                            hasVideo = true;
                        }
                    }
                }
                gst_caps_unref(caps);
            } else if (padtemplate->direction == GST_PAD_SINK) {
                GstCaps *caps = gst_static_pad_template_get_caps(padtemplate);
                for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                    GstStructure *structure = gst_caps_get_structure(caps, i);
                    gchar *structureStr = gst_structure_to_string(structure);
                    containerFormats.insert(QString(structureStr).section(';', 0, 0));
                    g_free(structureStr);
                }
                gst_caps_unref(caps);
            }
            pads = pads->next;
        }
        if (!avaiableCodecs.empty() && hasVideo) {
            for (const auto& format : containerFormats) {
                containers[format].decodeCodecs = avaiableCodecs;
            }
        }
    }
    gst_plugin_feature_list_free(factories);
}

void MultimediaFormats::cleanConvPipeline() {
    if (!convData.pipeline) return;
    gst_element_set_state(convData.pipeline, GST_STATE_NULL);
    gst_object_unref(convData.pipeline);
    convData.pipeline = NULL;
    convData.source = NULL;
    convData.decoder = NULL;
    convData.encoder = NULL;
    convData.sink = NULL;
    emit convertingChanged();
}

MultimediaFormats::~MultimediaFormats() {
    cleanConvPipeline();
    gst_deinit();
}

void MultimediaFormats::convertFile(QString sourceUri, QString destinationUri, QString containerFormat, QStringList videoCodecs, QStringList audioCodecs) {
    cleanConvPipeline();
    convData.pipeline = gst_pipeline_new("convert-pipeline");
    convData.source = gst_element_factory_make("filesrc", "source");
    convData.decoder = gst_element_factory_make("decodebin", "decoder");
    convData.encoder = gst_element_factory_make("encodebin", "encoder");
    convData.sink = gst_element_factory_make("filesink", "sink");
    convData.videoCodecs = videoCodecs;
    convData.audioCodecs = audioCodecs;

    if (!convData.pipeline || !convData.source || !convData.decoder || !convData.encoder || !convData.sink) {
        qCritical() << "Not all elements for conversion pipeline could be created";
        emit conversionError("Not all elements for conversion pipeline could be created");
        cleanConvPipeline();
        return;
    }

    // Select the format of the container (e.g. mpeg/matroska) from the parameter
    GstCaps *caps = gst_caps_from_string(containerFormat.toStdString().c_str());
    GstEncodingContainerProfile *containerProfile = gst_encoding_container_profile_new("container", NULL, caps, NULL);
    gst_caps_unref(caps);

    // Select video codecs for encoding from parameter
    for (const auto &codec : videoCodecs) {
        if (codec.isEmpty())
            continue;
        caps = gst_caps_from_string(codec.toStdString().c_str());
        GstEncodingVideoProfile *videoProfile = gst_encoding_video_profile_new(caps, NULL, NULL, 1);
        gst_encoding_container_profile_add_profile(containerProfile, (GstEncodingProfile *) videoProfile);
        gst_caps_unref(caps);
    }

    // Select audio codecs for encoding from parameter
    for (const auto &codec : audioCodecs) {
        if (codec.isEmpty())
            continue;
        caps = gst_caps_from_string(codec.toStdString().c_str());
        GstEncodingAudioProfile *audioProfile = gst_encoding_audio_profile_new(caps, NULL, NULL, 1);
        gst_encoding_container_profile_add_profile(containerProfile, (GstEncodingProfile *) audioProfile);
        gst_caps_unref(caps);
    }

    g_object_set(convData.encoder, "profile", containerProfile, NULL);
    g_object_set(convData.source, "location", sourceUri.replace("file://", "").toStdString().c_str(), NULL);
    g_object_set(convData.sink, "location", destinationUri.replace("file://", "").toStdString().c_str(), NULL);

    // Build the pipeline
    gst_bin_add_many(GST_BIN(convData.pipeline), convData.source, convData.decoder, convData.encoder, convData.sink, NULL);
    // Link source to decoder, encoder to sink
    if (!gst_element_link(convData.source, convData.decoder) || !gst_element_link(convData.encoder, convData.sink)) {
        qCritical() << "Could not link all elements in conversion pipeline";
        emit conversionError("Could not link all elements in conversion pipeline");
        cleanConvPipeline();
        return;
    }
    // Dynamically link decoder and encoder
    g_signal_connect(convData.decoder, "pad-added", G_CALLBACK((+[](GstElement *src, GstPad *pad, MultimediaFormats *multimediaFormats) {
        GstCaps* caps = gst_pad_get_current_caps(pad);
        GstStructure* structure = gst_caps_get_structure(caps, 0);
        const gchar *name = gst_structure_get_name(structure);
        gst_caps_unref(caps);

        if ((g_str_has_prefix(name, "video") && multimediaFormats->convData.videoCodecs.takeLast().isEmpty())
            || (g_str_has_prefix(name, "audio") && multimediaFormats->convData.audioCodecs.takeLast().isEmpty())) {
            return;
        }
        GstPad *sinkPad = gst_element_get_compatible_pad(multimediaFormats->convData.encoder, pad, NULL);

        if (sinkPad) {
            gst_pad_link(pad, sinkPad);
            gst_object_unref(sinkPad);
        } else {
            qCritical() << "Failed to get matching pad for pad with name:" << name;
            emit multimediaFormats->conversionError("Could not dynamically link all encoder pads to decoder");
            g_idle_add(G_SOURCE_FUNC(+[](MultimediaFormats *multimediaFormats) {
                multimediaFormats->cleanConvPipeline();
                return G_SOURCE_REMOVE;
            }), multimediaFormats);
        }
    })), this);

    // Run the pipeline
    GstStateChangeReturn ret = gst_element_set_state(convData.pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        qCritical() << "Conversion pipeline doesn't want to pause";
        emit conversionError("Conversion pipeline doesn't want to pause");
        cleanConvPipeline();
        return;
    }

    // Listen for EOS or error from the pipeline
    GstBus *bus = gst_element_get_bus(convData.pipeline);
    gst_bus_add_watch(bus, +[](GstBus* bus, GstMessage* message, gpointer data) -> gboolean {
        auto *multimediaFormats = static_cast<MultimediaFormats *>(data);
        if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_EOS) {
            multimediaFormats->cleanConvPipeline();
            return G_SOURCE_REMOVE;
        } else if (GST_MESSAGE_TYPE(message) == GST_MESSAGE_ERROR) {
            GError* error;
            gchar* debug;
            gst_message_parse_error(message, &error, &debug);
            qCritical() << "Conversion pipeline exited with error:" << error->message;
            emit multimediaFormats->conversionError(QString("Conversion pipeline exited with error: ") + error->message);
            g_error_free(error);
            g_free(debug);
            multimediaFormats->cleanConvPipeline();
            return G_SOURCE_REMOVE;
        }
        return G_SOURCE_CONTINUE;
    }, this);
    gst_object_unref(bus);

    emit convertingChanged();
}

bool MultimediaFormats::isConverting() {
    return convData.pipeline;
}

gint64 MultimediaFormats::getConvertDuration() {
    gint64 duration;
    if (convData.pipeline && gst_element_query_duration(convData.pipeline, GST_FORMAT_TIME, &duration))
        return duration;
    return -1;
}

gint64 MultimediaFormats::getConvertPosition() {
    gint64 position;
    if (convData.pipeline && gst_element_query_position(convData.pipeline, GST_FORMAT_TIME, &position))
        return position;
    return -1;
}

bool MultimediaFormats::isHardwareCodec(QString codec, bool encode) {
    if (encode) {
        return codecs.value(codec).hardwareEncode;
    } else {
        return codecs.value(codec).hardwareDecode;
    }
}

QString MultimediaFormats::format(QString codec, QString prefix) {
    return codec.replace(QRegularExpression("^" + prefix + "(x-)?"), "");
}

QString MultimediaFormats::formatWithDeco(QString codec, bool encode, QString prefix) {
    if (isHardwareCodec(codec, encode)) {
        return format(codec, prefix);
    } else {
        return "<i>" + format(codec, prefix) + "</i>";
    }
}

QStringList MultimediaFormats::formatList(QStringList codecs, QString prefix) {
    return codecs.replaceInStrings(QRegularExpression("^" + prefix + "(x-)?"), "");
}

int MultimediaFormats::compareEncodeCodecs(QString codec1, QString codec2) {
    int hardware = (2 * isHardwareCodec(codec2, true) + isHardwareCodec(codec2, false)) - (2 * isHardwareCodec(codec1, true) + isHardwareCodec(codec1, false));
    if (hardware != 0)
        return hardware;
    return getCodecRank(codec2, true) - getCodecRank(codec1, true);
}

guint MultimediaFormats::getCodecRank(QString codec, bool encode) {
    if (encode) {
        return codecs.value(codec).encodeRank;
    } else {
        return codecs.value(codec).decodeRank;
    }
}

int MultimediaFormats::getFileDiscoverResult(QString uri) {
    GError *err = NULL;
    GstDiscoverer *discoverer = gst_discoverer_new(GST_SECOND, &err);
    GstDiscovererInfo *info = gst_discoverer_discover_uri(discoverer, uri.toStdString().c_str(), &err);
    int result = gst_discoverer_info_get_result(info);
    if (err) {
        g_error_free(err);
    }
    if (info) {
        gst_discoverer_info_unref(info);
    }
    g_object_unref(discoverer);
    return result;
}

QString MultimediaFormats::getFileFormat(QString uri) {
    GError *err = NULL;
    GstDiscoverer *discoverer = gst_discoverer_new(GST_SECOND, &err);
    GstDiscovererInfo *info = gst_discoverer_discover_uri(discoverer, uri.toStdString().c_str(), &err);
    QString format;
    if (info) {
        GstDiscovererStreamInfo* streamInfo = gst_discoverer_info_get_stream_info(info);
        if (streamInfo) {
            GstCaps* caps = gst_discoverer_stream_info_get_caps(streamInfo);
            if (caps) {
                gchar* format_name = gst_caps_to_string(caps);
                format = QString(format_name);
                g_free(format_name);
                gst_caps_unref(caps);
            }
        }
        gst_discoverer_info_unref(info);
    }
    if (err) {
        g_error_free(err);
    }
    g_object_unref(discoverer);
    return format;
}

QStringList MultimediaFormats::getFileVideoCodec(QString uri) {
    GError *err = NULL;
    GstDiscoverer *discoverer = gst_discoverer_new(GST_SECOND, &err);
    GstDiscovererInfo *info = gst_discoverer_discover_uri(discoverer, uri.toStdString().c_str(), &err);
    QStringList codecs;
    if (info) {
        GList *videoStreams = gst_discoverer_info_get_video_streams(info);
        for (GList *l = videoStreams; l != NULL; l = l->next) {
            GstDiscovererStreamInfo *streamInfo = (GstDiscovererStreamInfo *)l->data;
            GstCaps *caps = gst_discoverer_stream_info_get_caps(streamInfo);
            if (!caps)
                continue;
            for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                GstStructure *structure = gst_caps_get_structure(caps, i);
                codecs.append(QString(gst_structure_get_name(structure)));
            }
            gst_caps_unref(caps);
        }
        gst_discoverer_stream_info_list_free(videoStreams);
        gst_discoverer_info_unref(info);
    }
    if (err) {
        g_error_free(err);
    }
    g_object_unref(discoverer);
    return codecs;
}

QStringList MultimediaFormats::getFileAudioCodec(QString uri) {
    GError *err = NULL;
    GstDiscoverer *discoverer = gst_discoverer_new(GST_SECOND, &err);
    GstDiscovererInfo *info = gst_discoverer_discover_uri(discoverer, uri.toStdString().c_str(), &err);
    QStringList codecs;
    if (info) {
        GList *audioStreams = gst_discoverer_info_get_audio_streams(info);
        for (GList *l = audioStreams; l != NULL; l = l->next) {
            GstDiscovererStreamInfo *streamInfo = (GstDiscovererStreamInfo *)l->data;
            GstCaps *caps = gst_discoverer_stream_info_get_caps(streamInfo);
            if (!caps)
                continue;
            for (guint i = 0; i < gst_caps_get_size(caps); i++) {
                GstStructure *structure = gst_caps_get_structure(caps, i);
                codecs.append(QString(gst_structure_get_name(structure)));
            }
            gst_caps_unref(caps);
        }
        gst_discoverer_stream_info_list_free(audioStreams);
        gst_discoverer_info_unref(info);
    }
    if (err) {
        g_error_free(err);
    }
    g_object_unref(discoverer);
    return codecs;
}

QStringList MultimediaFormats::getEncodeCodecs(bool onlyHardware, bool onlySoftware, QString prefix, QString containerFormat) {
    QStringList selectedCodecs;
    if (containerFormat.isNull()) {
        for (auto [codec, data] : codecs.asKeyValueRange()) {
            if (data.encode && !(onlyHardware && !data.hardwareEncode)
                && !(onlySoftware && data.hardwareEncode) && codec.startsWith(prefix)) {
                selectedCodecs.append(codec);
            }
        }
    } else {
        for (const auto &codec : containers.value(containerFormat).encodeCodecs) {
            if (!codecs.contains(codec))
                continue;
            CodecData data = codecs.value(codec);
            if (data.encode && !(onlyHardware && !data.hardwareEncode)
                && !(onlySoftware && data.hardwareEncode) && codec.startsWith(prefix)) {
                selectedCodecs.append(codec);
            }
        }
    }
    return selectedCodecs;
}

QStringList MultimediaFormats::getDecodeCodecs(bool onlyHardware, bool onlySoftware, QString prefix, QString containerFormat) {
    QStringList selectedCodecs;
    if (containerFormat.isNull()) {
        for (auto [codec, data] : codecs.asKeyValueRange()) {
            if (data.decode && !(onlyHardware && !data.hardwareDecode)
                && !(onlySoftware && data.hardwareDecode) && codec.startsWith(prefix)) {
                selectedCodecs.append(codec);
            }
        }
    } else {
        for (const auto &codec : containers.value(containerFormat).decodeCodecs) {
            if (!codecs.contains(codec))
                continue;
            CodecData data = codecs.value(codec);
            if (data.decode && !(onlyHardware && !data.hardwareDecode)
                && !(onlySoftware && data.hardwareDecode) && codec.startsWith(prefix)) {
                selectedCodecs.append(codec);
            }
        }
    }
    return selectedCodecs;
}

QStringList MultimediaFormats::getContainerFormats(QStringList codecs, bool all, bool encode) {
    QStringList containerFormats;
    if (all) {
        for (auto [key, value] : containers.asKeyValueRange()) {
            containerFormats.append(key);
            for (const auto &codec : codecs) {
                if (!value.encodeCodecs.contains(codec) && !value.decodeCodecs.contains(codec)) {
                    containerFormats.removeLast();
                    break;
                }
            }
        }
    } else {
        for (auto [key, value] : containers.asKeyValueRange()) {
            if ((encode && value.encodeCodecs.isEmpty())
                || (!encode && value.decodeCodecs.isEmpty()))
                continue;
            containerFormats.append(key);
            for (const auto &codec : codecs) {
                if ((encode && !value.encodeCodecs.contains(codec))
                    || (!encode && !value.decodeCodecs.contains(codec))) {
                    containerFormats.removeLast();
                    break;
                }
            }
        }
    }
    return containerFormats;
}

QStringList MultimediaFormats::getExtensions(QString containerFormat) {
    QSet<QString> extensionList;
    GstCaps* caps = gst_caps_from_string(containerFormat.toStdString().c_str());
    GList *factories = gst_type_find_factory_get_list();
    for (GList* l = factories; l != NULL; l = l->next) {
        GstTypeFindFactory* factory = GST_TYPE_FIND_FACTORY(l->data);
        GstCaps* f_caps = gst_type_find_factory_get_caps(factory);

        if (f_caps && gst_caps_can_intersect(caps, f_caps)) {
            const gchar* const* extensions = gst_type_find_factory_get_extensions(factory);
            if (extensions) {
                for (int j = 0; extensions[j] != NULL; j++) {
                    extensionList.insert(extensions[j]);
                }
            }
        }
    }
    gst_caps_unref(caps);
    gst_plugin_feature_list_free(factories);
    return extensionList.values();
}
