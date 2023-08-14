/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

#ifndef CAMERA_DEMO_HPP
#define CAMERA_DEMO_HPP

#include <QObject>
#include <QProcess>
#include <QQmlEngine>
#include <QJSEngine>

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

#include <opencv2/opencv.hpp>

// #include <opencv2/core.hpp>
// #include <opencv2/imgproc.hpp>
// #include <opencv2/highgui.hpp>

// #include <opencv2/mat.hpp>
// #include <opencv2/videoio.hpp>
#include <iostream>

class OpencvImageProvider : public QQuickImageProvider
{
    Q_OBJECT

public:
    OpencvImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    // QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

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

    Q_PROPERTY(QString cameraName
                   READ getCameraName
                       NOTIFY sensorChanged);
    Q_PROPERTY(QString framesize
                   READ getFramesize
                       NOTIFY framesizeChanged);
    Q_PROPERTY(bool autoExposure
                   READ getAutoExposure
                       NOTIFY autoExosureChanged);
    Q_PROPERTY(bool flipVertical
                   READ getFlipVertical
                       NOTIFY flipVerticalChanged);
    Q_PROPERTY(bool flipHorizontal
                   READ getFlipHorizontal
                       NOTIFY flipHorizontalChanged);
    Q_PROPERTY(int exposure
                   READ getExposure
                       NOTIFY exposureChanged);

public:
    CameraDemo(QObject *parent = nullptr);
    static QObject *singletontypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine);
    ~CameraDemo();
    void updateFrame();

private:
    QTimer tUpdate;
    cv::Mat frame;
    cv::VideoCapture cap;
    std::string v4l_subdev = "/dev/v4l-subdev3"; // TBD: get dynamically

    std::string CAM;
    std::string SENSOR;
    std::string FRAMESIZE;
    // std::string FRAMESIZE = "";

    int getSensor(); // get Sensor and Framesize
    int getCAM();
    void getControls();

    const char *subdevPath = "/dev/v4l-subdev3";
    int subdevFd = open(subdevPath, O_RDWR);

signals:
    void newImage(QImage &);
    void framesizeChanged();
    void sensorChanged();
    void autoExosureChanged();
    void flipVerticalChanged();
    void flipHorizontalChanged();
    void exposureChanged();

public slots:
    void openCamera();
    QString getCameraName() const;
    QString getFramesize() const;

    bool getAutoExposure();
    bool getFlipHorizontal();
    bool getFlipVertical();
    int getExposure();

    void setAutoExposure(bool value);
    void setFlipVertical(bool value);
    void setFlipHorizontal(bool value);
    void setExposure(int value);
};

#endif /* CAMERA_DEMO_HPP */
