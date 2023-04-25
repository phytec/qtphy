/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

#include <QtCore>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QQuickItem>
#include <QRunnable>

#include <QDebug>
#include <QtMultimedia>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QCommandLineParser>
#include <QQmlContext>

#include <opencv2/opencv.hpp>

// #include <opencv2/core.hpp>
// #include <opencv2/imgproc.hpp>
// #include <opencv2/highgui.hpp>

// #include <opencv2/mat.hpp>
// #include <opencv2/videoio.hpp>
#include <iostream>

#include <qqmlextensionplugin.h>

#include <qqmlengine.h>
#include <qquickimageprovider.h>
#include <QImage>
#include <QPainter>
#include "camera_demo.hpp"

// qInfo() << "std out!";

// // QCameraDevice cameras = QMediaDevices::videoInputs();
// // qInfo() << QMediaDevices::videoInputs();
// const QList<QCameraDevice> cameraDevices = QMediaDevices::videoInputs();
// for (const QCameraDevice &cameraDevice : cameraDevices) {
// 	qInfo() << "Camera: " << cameraDevice;
// 	qInfo() << "Supported Formats _start: ";
// 	for (const QCameraFormat &cameraFormat : cameraDevice.videoFormats()) {
// 		qInfo() << "   Format: ";
// 		qInfo() << "      maxFrameRate: " << cameraFormat.maxFrameRate();
// 		qInfo() << "      minFrameRate: " << cameraFormat.minFrameRate();
// 		qInfo() << "      pixelFormat: " << cameraFormat.pixelFormat();
// 		qInfo() << "      resolution: " << cameraFormat.resolution();
// 	}
// 	qInfo() << "Supported Formats _end: ";

// 	QCamera camera(cameraDevice);
// 	qInfo() << "Device: " << camera.cameraDevice();
// 	qInfo() << "Resolution: " << camera.cameraFormat().resolution();
// 	qInfo() << "Pixel Format: " << camera.cameraFormat().pixelFormat();
// 	qInfo() << "Error: " << camera.error();

// }

CameraDemo::CameraDemo(QObject *parent) : QObject(parent)
{
    connect(&tUpdate, &QTimer::timeout, this, &CameraDemo::updateFrame);
}

CameraDemo::~CameraDemo()
{
    cap.release();
    tUpdate.stop();
}

void CameraDemo::openCamera()
{
    std::string pipeline = "v4l2src device=/dev/video-csi1 ! video/x-bayer,format=grbg, width=1280, height=720 ! appsink";
    cap = cv::VideoCapture(pipeline, cv::CAP_GSTREAMER);

    // cap.open(1);
    double fps = cap.get(cv::CAP_PROP_FPS);
    qDebug() << "fps: " << fps;
    tUpdate.start(1000 / fps);
    // tUpdate.start(1000 / 30); // 30 fps
}

void CameraDemo::updateFrame()
{
    qDebug() << "updating frame";
    cv::Mat rawFrame;

    cap >> rawFrame;
    cv::cvtColor(rawFrame, frame, cv::COLOR_BayerGB2RGB);

    // cv::cvtColor(rawFrame2, frame, cv::COLOR_BGR2RGB);
    QImage image = QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888);

    // QImage image = QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888).rgbSwapped();
    emit newImage(image);
}

//  ------------

OpencvImageProvider::OpencvImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
// : QQuickImageProvider(QQuickImageProvider::Pixmap)
{
    image = QImage(200, 200, QImage::Format_RGB888);
    image.fill(QColor("blue"));
}

// QPixmap OpencvImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
// {
//     int width = 100;
//     int height = 50;

//     if (size)
//         *size = QSize(width, height);
//     QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
//                    requestedSize.height() > 0 ? requestedSize.height() : height);
//     pixmap.fill(QColor(id).rgba());
//     return pixmap;
// }

QImage OpencvImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(id);

    if (size)
    {
        *size = image.size();
    }

    if (requestedSize.width() > 0 && requestedSize.height() > 0)
    {
        image = image.scaled(requestedSize.width(), requestedSize.height(), Qt::KeepAspectRatio);
    }
    return image;
}

void OpencvImageProvider::updateImage(const QImage &image)
{
    // if (!image.isNull() && this->image != image)
    // {
    // }
    this->image = image;
    emit imageChanged();
}
