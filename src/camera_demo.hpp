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

public:
    CameraDemo(QObject *parent = nullptr);
    ~CameraDemo();

public:
    void updateFrame();

private:
    QTimer tUpdate;
    cv::Mat frame;
    cv::VideoCapture cap;
    // public slots:
    // private slots:

signals:
    void newImage(QImage &);

public slots:
    void openCamera();
};

// ----------

#endif /* CAMERA_DEMO_HPP */
