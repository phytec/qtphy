/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

#ifndef CAMERA_DEMO_HPP
#define CAMERA_DEMO_HPP

#include <QImage>
#include <QTimer>
#include <QQuickImageProvider>

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/videodev2.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#include "json.hpp"

#include <opencv2/opencv.hpp>

using json = nlohmann::json;
namespace fs = std::filesystem;


#define V4L2_CID_VIV_EXTCTRL 0x98F901

enum video_srcs{ISP, ISI};
enum camera_status{ACTIVE, READY, UNCONNECTED, ERROR};
enum status{SINGLE_CAM, DUAL_CAM, NO_CAM, WRONG_OVERLAYS};
enum csi_interface{CSI1, CSI2};

struct Sensor {
    std::string camera_name;
    std::string name;
    bool hasAutoExposure;
    int sensor_width;
    int sensor_height;
    int frame_width;
    int frame_height;
    int offset_x;
    int offset_y;
};


extern Sensor SENSORS[];

class PhyCam {
public:
    PhyCam(const int _interface);
    ~PhyCam();

    camera_status status = UNCONNECTED;

    std::string device =  ""; // /dev/cam-csi1-port0   (CAM)
    int device_fd = -1; // file descriptor of device (subdevFd)
    int isp_fd = -1;

    int csi_interface = -1;
    int port = -1;

    Sensor *sensor = &SENSORS[0];
    video_srcs video_src = ISP;
    cv::VideoCapture cap;
    std::string setup_pipeline_command = "";
    std::string isp_pipeline = "";
    std::string isi_pipeline = "";

    int getSensor();
    int setup_pipeline();
};

class OpencvImageProvider : public QQuickImageProvider
{
    Q_OBJECT

public:
    OpencvImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

public slots:
    void updateImage(const QImage &image);

signals:
    void imageChanged();

private:
    QImage image;
};

class CameraDemo : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString cameraName // Camera Name
                   READ getCameraName
                       NOTIFY sensorChanged);
    Q_PROPERTY(int interface // Interface
                   READ getInterface
                       NOTIFY interfaceChanged);
    Q_PROPERTY(QString interfaceString // Interface
                   READ getInterfaceString
                       NOTIFY interfaceChanged);
    Q_PROPERTY(QString framesize // Framesize
                   READ getFramesize
                       NOTIFY framesizeChanged);
    Q_PROPERTY(int videoSrc // Video Source (ISP or ISI)
                   READ getVideoSrc
                       NOTIFY videoSrcChanged);
    Q_PROPERTY(bool autoExposure // Auto Exposure
                   READ getAutoExposure
                       NOTIFY autoExposureChanged);
    Q_PROPERTY(bool hasAutoExposure // Auto Exposure availability on sensor
                   READ getHasAutoExposure
                       NOTIFY hasAutoExposureChanged);
    Q_PROPERTY(bool flipVertical // Flip Vertical
                   READ getFlipVertical
                       NOTIFY flipVerticalChanged);
    Q_PROPERTY(bool flipHorizontal // Flip Horizontal
                   READ getFlipHorizontal
                       NOTIFY flipHorizontalChanged);
    Q_PROPERTY(int exposure // Exposure
                   READ getExposure
                       NOTIFY exposureChanged);

    Q_PROPERTY(QString recommendedOverlays // Recommended overlays
                   READ getRecommendedOverlays
                       NOTIFY recommendedOverlaysChanged);

    Q_PROPERTY(int status // No Camera Found
                READ getStatus
                    NOTIFY statusChanged);

public:
    CameraDemo(QObject *parent = nullptr);
    ~CameraDemo();
    void updateFrame();
    Q_INVOKABLE void reloadOverlays();

private:
    QTimer tUpdate;
    cv::Mat frame;
    cv::VideoCapture cap;

    PhyCam cam1;
    PhyCam cam2;
    PhyCam* CAM = &cam1; // TBD: add default cam with empty strings

    int tmp = 0;

    status STATUS;
    // int ERROR = -1;
    QString RECOMMENDED_OVERLAYS = "";

    int isp_ioctl(const char *cmd, json& jsonRequest, json& jsonResponse);

signals:
    void newImage(QImage &);
    void framesizeChanged();
    void sensorChanged();
    void autoExposureChanged();
    void hasAutoExposureChanged();
    void flipVerticalChanged();
    void flipHorizontalChanged();
    void exposureChanged();
    void videoSrcChanged();
    void interfaceChanged();
    void recommendedOverlaysChanged();
    void statusChanged();
    void csiPortChanged();

public slots:
    void openCamera();
    QString getCameraName() const;
    QString getFramesize() const;
    QString getRecommendedOverlays() const;
    QString getInterfaceString() const;

    int getInterface() const;
    int getVideoSrc() const;

    bool getAutoExposure();
    bool getHasAutoExposure();
    bool getFlipHorizontal();
    bool getFlipVertical();

    int getExposure();
    int getStatus();

    void setAutoExposure(bool value);
    void setFlipVertical(bool value);
    void setFlipHorizontal(bool value);
    void setExposure(int value);

    void setVideoSource(video_srcs value);
    void setInterface(csi_interface value);

    void setDwe(bool value);
    void setAwb(bool value);
    void setLsc(bool value);
    void setAec(bool value);
};

#endif /* CAMERA_DEMO_HPP */
