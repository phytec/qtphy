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

#include "json.hpp"

#include <opencv2/opencv.hpp>

using json = nlohmann::json;


#define V4L2_CID_VIV_EXTCTRL 0x98F901

enum video_srcs{ISP, ISI};

struct camera {
    std::string device =  ""; // /dev/cam-csi1-port0   (CAM)
    int device_fd = -1; // file descriptor of device (subdevFd)

    std::string interface = "";
    std::string sensor = "";
    std::string framesize = "";
    std::string format = "";
    video_srcs video_src = ISP;

    std::string v4l_subdev = ""; // (v4l_subdev in getSensor())
    std::string pipeline_command = "";

    



    // std::string vd = ""; // video device (e.g. /dev/video-isp-csi1)

    
};

// class IspJson {
// public:
//     bool isInitialized = 0;


//     IspJson() {
//         ec.id = V4L2_CID_VIV_EXTCTRL;
//         ec.size = 64 * 1024;
//         ecs.controls = &ec;
//         ecs.count = 1;
//         ecs.ctrl_class = V4L2_CTRL_CLASS_USER;
//         ec.string = (char*)malloc(ec.size * sizeof(char));

//         // buffer = (char*)malloc(ec.size * sizeof(char));
//         if (!ec.string) {
//             std::cerr << "Malloc failed" << std::endl;
//             // Handle the error appropriately (e.g., throw an exception or return an error code).
//         }
//     }
//     ~IspJson() {
//         free(ec.string);
//     }
    
//     void init(int videodev){
//         vd = videodev;
//         isInitialized = true;

//         try {
//             ioctl(vd, VIDIOC_G_EXT_CTRLS, &ecs);
//         } catch (std::exception& e) {
//             std::cerr << "Error on INIT" << std::endl;
//         }
//     }

//     std::string read_json(std::string const& get_data) {
//         // exit if not initialized
//         std::cout << "bbb" << std::endl;
//         _set_ctrls(get_data);
//         return _get_ctrls();
//     }

//     std::string write_json(std::string const& set_data) {
//         // exit if not initialized
//         _set_ctrls(set_data);
//         return _get_ctrls();
//     }

// private:
//     struct v4l2_ext_control ec;
//     struct v4l2_ext_controls ecs;
//     int vd;
//     // char *buffer = 0;



//     std::string _get_ctrls() {
//         std::cout << "a" << ec.string << std::endl;

//         memset(ec.string, 0, ec.size);
//         std::cout << "b" << ec.string << std::endl;

//         ioctl(vd, VIDIOC_G_EXT_CTRLS, &ecs);

//         std::cout << "c" << ec.string << std::endl;
//         return "asdf";
//     }

//     void _set_ctrls(std::string const& data) {
//         memset(ec.string, 0, ec.size);
//         // const char* cstr = data.c_str();
//         const char* cstr = "";
//         // std::string s_data = data.toStyledString();
//         if (strlen(cstr) >= ec.size) {
//             std::cerr << "Data size exceeds buffer size" << std::endl;
//             // throw std::runtime_error("Data size exceeds buffer size");
//         }

//         std::cout << "d" << ec.string << std::endl;

//         std::memcpy(ec.string, cstr, strlen(cstr));
//         ioctl(vd, VIDIOC_S_EXT_CTRLS, &ecs);
//         std::cout << "e" << ec.string << std::endl;


//         // std::cout << ecs << std::endl;
//     }
// };


class OpencvImageProvider : public QQuickImageProvider
{
    Q_OBJECT

public:
    OpencvImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

    ~OpencvImageProvider()
    {
        qDebug() << "destroy OpencvImageProvider";
    }

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
    Q_PROPERTY(QString interface // Interface
                   READ getInterface
                       NOTIFY interfaceChanged);
    Q_PROPERTY(QString framesize // Framesize
                   READ getFramesize
                       NOTIFY framesizeChanged);
    Q_PROPERTY(QString format // Format
                   READ getFormat
                       NOTIFY formatChanged);
    Q_PROPERTY(QString videoSrc // Video Source (ISP or ISI)
                   READ getVideoSrc
                       NOTIFY videoSrcChanged);
    Q_PROPERTY(bool autoExposure // Auto Exposure
                   READ getAutoExposure
                       NOTIFY autoExosureChanged);
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
                       
    Q_PROPERTY(int errorDialog // No Camera Found
                READ getErrorDialog
                    NOTIFY errorDialogChanged);

public:
    CameraDemo(QObject *parent = nullptr);
    static QObject *singletontypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~CameraDemo();
    void updateFrame();
    Q_INVOKABLE void reloadOverlays();

private:
    QTimer tUpdate;
    cv::Mat frame;
    cv::VideoCapture cap;

    std::string CAM;
    std::string INTERFACE;
    std::string SENSOR;
    std::string FRAMESIZE;
    std::string FORMAT;
    video_srcs VIDEO_SRC = ISI;

    // std::string vd = "";
    int vd_fd = -1;
    int tmp = 0;

    int ERROR = -1;
    QString RECOMMENDED_OVERLAYS = "";

    int getSensor(); // get Sensor and Framesize
    int getCAM();
    void getControls();

    int subdevFd = -1;
    int streamid = 0;
    int isp_ioctl(const char *cmd, json& jsonRequest, json& jsonResponse);
    int isp_read_ioctl(const char *cmd, json& jsonRequest, json& jsonResponse);

signals:
    void newImage(QImage &);
    void framesizeChanged();
    void sensorChanged();
    void autoExosureChanged();
    void flipVerticalChanged();
    void flipHorizontalChanged();
    void exposureChanged();
    void formatChanged();
    void videoSrcChanged();
    void interfaceChanged();
    void recommendedOverlaysChanged();
    void errorDialogChanged();

public slots:
    void openCamera();
    QString getCameraName() const;
    QString getInterface() const;
    QString getFramesize() const;
    QString getFormat() const;
    QString getVideoSrc() const;
    QString getRecommendedOverlays() const;

    bool getAutoExposure();
    bool getFlipHorizontal();
    bool getFlipVertical();

    int getExposure();

    int getErrorDialog();


    void setAutoExposure(bool value);
    void setFlipVertical(bool value);
    void setFlipHorizontal(bool value);
    void setExposure(int value);

    void setVideoSource(video_srcs value);

    void setDwe(bool value);
    void setAwb(bool value);
    void setLsc(bool value);
    void setAec(bool value);
};

#endif /* CAMERA_DEMO_HPP */
