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
#include <string>
#include <limits.h>
#include <fstream>
#include <unistd.h>

#include <qqmlextensionplugin.h>

#include <qqmlengine.h>
#include <qquickimageprovider.h>
#include <QImage>
#include <QPainter>
#include "camera_demo.hpp"

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

// If on ISI: execute setup-pipeline-csi1
// Else if on ISP: complete the pipeline manually

QObject *CameraDemo::singletontypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);

    CameraDemo *cameraDemo = new CameraDemo();
    return cameraDemo;
}

CameraDemo::CameraDemo(QObject *parent) : QObject(parent)
{
    qDebug() << "CameraDemo constructor";
    connect(&tUpdate, &QTimer::timeout, this, &CameraDemo::updateFrame);
}

CameraDemo::~CameraDemo()
{
    cap.release();
    tUpdate.stop();
}

int CameraDemo::getCAM()
{
    // CSI1
    if (access("/dev/cam-csi1", F_OK) == 0)
    { // phycam-M on csi-1
        CAM = "/dev/cam-csi1";
        return 1;
    }
    else if (access("/dev/cam-csi1-port0", F_OK) == 0)
    {
        CAM = "/dev/cam-csi1-port0";
        return 1;
    }
    else if (access("/dev/cam-csi1-port1", F_OK) == 0)
    {
        CAM = "/dev/cam-csi1-port1";
        return 1;
    }
    // CSI2
    else if (access("/dev/cam-csi2", F_OK) == 0)
    { // phycam-M on csi-2
        CAM = "/dev/cam-csi2";
        return 2;
    }
    else if (access("/dev/cam-csi2-port0", F_OK) == 0)
    {
        CAM = "/dev/cam-csi2-port0";
        return 2;
    }
    else if (access("/dev/cam-csi2-port1", F_OK) == 0)
    {
        CAM = "/dev/cam-csi2-port1";
        return 2;
    }
    // NO CAM FOUND
    else
    {
        return -1;
    }
}

int CameraDemo::getSensor()
{
    char buffer[PATH_MAX];
    ssize_t len = readlink(CAM.c_str(), buffer, sizeof(buffer) - 1);
    if (len != -1)
    {
        buffer[len] = '\0';
        std::string entityPath = "/sys/class/video4linux/";
        entityPath += buffer;
        entityPath += "/name";
        std::ifstream entityFile(entityPath);
        std::string entity;
        if (entityFile.is_open())
        {
            std::getline(entityFile, entity);
            entityFile.close();
        }
        std::istringstream iss(entity);
        std::string firstToken;
        iss >> firstToken;
        // TBD: GET COLOR FORMAT
        if (firstToken == "ar0144")
        {
            // std::string CAM_BW_FMT = "Y8_1X8";
            // std::string CAM_COL_FMT = "SGRBG8_1X8";
            // std::string SENSOR_RES = "1280x800";
            // std::string OFFSET_SENSOR = "(0,4)";
            SENSOR = "ar0144";
            emit sensorChanged();
            FRAMESIZE = "width=1280, height=800";
            emit framesizeChanged();
            return 0;
        }
        else if (firstToken == "ar0521")
        {
            // std::string CAM_BW_FMT = "Y8_1X8";
            // std::string CAM_COL_FMT = "SGRBG8_1X8";
            // std::string SENSOR_RES = "2592x1944";
            // std::string OFFSET_SENSOR = "(0,0)";
            SENSOR = "ar0521";
            emit sensorChanged();
            FRAMESIZE = "width=2592, height=1944";
            emit framesizeChanged();
            return 1;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        qDebug() << "Error: Could not read link";
        return -1;
    }
}

bool CameraDemo::getAutoExposure()
{
    qDebug() << "GETTING AUTO EXPOSURE";
    // int subdevFd = open(v4l_subdev.c_str(), O_RDONLY);
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control)); // set memory of control to all zeros (for idempotenty)

    control.id = V4L2_CID_EXPOSURE_AUTO;
    if (ioctl(subdevFd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "Fehler beim Einstellen des Belichtungsmodus" << std::endl;
        // close(subdevFd);
    }

    if (control.value == V4L2_EXPOSURE_AUTO)
    {
        return true;
    }
    else if (control.value == V4L2_EXPOSURE_MANUAL)
    {
        return false;
    }
    else
    {
        std::cerr << "Fehler beim Abrufen des Belichtungsmodus" << std::endl;
        return false;
    }
    // close(subdevFd);
}

bool CameraDemo::getFlipHorizontal()
{
    qDebug() << "GETTING FLIP HORIZONTAL";
    // int subdevFd = open(v4l_subdev.c_str(), O_RDONLY);
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control)); // set memory of control to all zeros (for idempotenty)

    control.id = V4L2_CID_HFLIP;
    if (ioctl(subdevFd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "Fehler beim Einstellen des Horizontal Flips" << std::endl;
        // close(subdevFd);
    }

    if (control.value == 0)
    {
        return false;
    }
    else if (control.value == 1)
    {
        return true;
    }
    else
    {
        std::cerr << "Fehler beim Abrufen des Horizontal Flips" << std::endl;
        return false;
    }
}

bool CameraDemo::getFlipVertical()
{
    qDebug() << "GETTING FLIP VERTICAL";
    // int subdevFd = open(v4l_subdev.c_str(), O_RDONLY);
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control)); // set memory of control to all zeros (for idempotenty)

    control.id = V4L2_CID_VFLIP;
    if (ioctl(subdevFd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "Fehler beim Einstellen des Vertical Flips" << std::endl;
        // close(subdevFd);
    }

    if (control.value == 0)
    {
        return false;
    }
    else if (control.value == 1)
    {
        return true;
    }
    else
    {
        std::cerr << "Fehler beim Abrufen des Vertical Flips" << std::endl;
        return false;
    }
}

int CameraDemo::getExposure()
{
    qDebug() << "GETTING EXPOSURE";
    // int subdevFd = open(v4l_subdev.c_str(), O_RDONLY);
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control)); // set memory of control to all zeros (for idempotenty)

    control.id = V4L2_CID_EXPOSURE;
    if (ioctl(subdevFd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "Fehler beim holen der Belichtungszeit" << std::endl;
        // close(subdevFd);
    }

    return control.value;
}

// 1 = Auto Exposure ; 0 = Manual Exposure
void CameraDemo::setAutoExposure(bool value)
{
    qDebug() << "Setting Auto Exposure to: " << value;
    // int subdevFd = open(subdevPath, O_RDWR);
    if (subdevFd == -1)
    {
        perror("Failed to open subdevice");
    }

    struct v4l2_control control;
    control.id = V4L2_CID_EXPOSURE_AUTO;
    if (value)
    {
        control.value = V4L2_EXPOSURE_AUTO;
    }
    else
    {
        control.value = V4L2_EXPOSURE_MANUAL;
        // if auto exposure is disabled, update the exposure slider
        emit exposureChanged();
    }
    qDebug() << "Setting ioctl to " << control.value;

    if (ioctl(subdevFd, VIDIOC_S_CTRL, &control) == -1)
    {
        perror("Failed to set auto exposure");
    }
}

void CameraDemo::setExposure(int value)
{
    qDebug() << "Setting Exposure to: " << value;
    // int subdevFd = open(subdevPath, O_RDWR);
    if (subdevFd == -1)
    {
        perror("Failed to open subdevice");
    }

    struct v4l2_control control;
    control.id = V4L2_CID_EXPOSURE;
    control.value = value;
    qDebug() << "Setting ioctl to " << control.value;

    if (ioctl(subdevFd, VIDIOC_S_CTRL, &control) == -1)
    {
        perror("Failed to set auto exposure");
    }
}

void CameraDemo::setFlipVertical(bool value)
{
    qDebug() << "Setting Flip Vertical to: " << value;
    // int subdevFd = open(subdevPath, O_RDWR);
    if (subdevFd == -1)
    {
        perror("Failed to open subdevice");
    }

    struct v4l2_control control;
    control.id = V4L2_CID_VFLIP;
    control.value = value;
    qDebug() << "Setting ioctl to " << control.value;

    if (ioctl(subdevFd, VIDIOC_S_CTRL, &control) == -1)
    {
        perror("Failed to set auto exposure");
    }
}

void CameraDemo::setFlipHorizontal(bool value)
{
    qDebug() << "Setting Flip Horizontal to: " << value;
    // int subdevFd = open(subdevPath, O_RDWR);
    if (subdevFd == -1)
    {
        perror("Failed to open subdevice");
    }

    struct v4l2_control control;
    control.id = V4L2_CID_HFLIP;
    control.value = value;
    qDebug() << "Setting ioctl to " << control.value;

    if (ioctl(subdevFd, VIDIOC_S_CTRL, &control) == -1)
    {
        perror("Failed to set auto exposure");
    }
}

// void CameraDemo::setBlackLevelCorrection(bool value)
// {
//     qDebug() << "Setting Black Level Correction to: " << value;
//     // int subdevFd = open(subdevPath, O_RDWR);
//     if (subdevFd == -1) {
//         perror("Failed to open subdevice");
//     }

//     struct v4l2_control control;
//     control.id = V4L2_CID_BLACK_LEVEL_CORRECTION;
//     control.value = value;
//     qDebug() << "Setting ioctl to " << control.value;

//     if (ioctl(subdevFd, VIDIOC_S_CTRL, &control) == -1) {
//         perror("Failed to set black level correction");
//     }
// }

void CameraDemo::openCamera()
{
    cap.release();
    if (getCAM() == -1)
    { // detect camera
        qDebug() << "ERROR: Camera not found";
        return;
    }

    std::string device = "";
    if (CAM == "/dev/cam-csi1" || CAM == "/dev/cam-csi1-port0")
    {
        qDebug() << "Camera found on CSI1";
        device = "/dev/video-csi1";
        if (system("/usr/bin/setup-pipeline-csi1") != 0) // call setup-pipeline script
        {
            qDebug() << "ERROR: setup-pipeline-csi1 failed";
            return;
        }
    }
    else if (CAM == "/dev/cam-csi1-port1")
    {
        qDebug() << "Camera found on CSI1, PORT 1";
        device = "/dev/video-csi1";
        if (system("/usr/bin/setup-pipeline-csi1 -p 1") != 0) // call setup-pipeline script with port 1
        {
            qDebug() << "ERROR: setup-pipeline-csi1 failed";
            return;
        }
    }
    else if (CAM == "/dev/cam-csi2" || CAM == "/dev/cam-csi2-port0")
    {
        qDebug() << "Camera found on CSI2";
        device = "/dev/video-csi2";
        if (system("/usr/bin/setup-pipeline-csi2") != 0) // call setup-pipeline script
        {
            qDebug() << "ERROR: setup-pipeline-csi2 failed";
            return;
        }
    }
    else if (CAM == "/dev/cam-csi2-port1")
    {
        qDebug() << "Camera found on CSI2, PORT 1";
        device = "/dev/video-csi2";
        if (system("/usr/bin/setup-pipeline-csi2 -p 1") != 0) // call setup-pipeline script
        {
            qDebug() << "ERROR: setup-pipeline-csi2 failed";
            return;
        }
    }
    else // No camera found
    {
        // TBD: Show Text in Image Preview ("NO CAMERA FOUND")
        qDebug() << "ERROR: No camera found";
        return;
    }

    // GET SENSOR AND FRAMESIZE
    if (getSensor() < 0)
    {
        qDebug() << "ERROR: No sensor found (could not determine FRAMESIZE and FORMAT)";
        return;
    }

    // TBD: GET COLOR FORMAT

    // CHECK ISI / ISP
    std::string format = "";
    if (access("/dev/isp-csi1", F_OK) == 0) // ISP is used
    {
        // format = "video/x-raw,format=YUY2"; // TBD: x-raw vs bayer
        format = "video/x-bayer,format=YUY2"; // set format to YUY2
    }
    else // ISI is used
    {
        format = "video/x-bayer,format=grbg"; // set format to grbg
    }

    // TBD: GET COLOR FORMAT
    // TBD: QProcess kann angeblich keine pipes
    // QProcess process;
    // // v4l2-ctl -d ${CAM} --get-subdev-fmt 0 | \
        // //      grep "Mediabus Code" |
    // //      sed 's/.*BUS_FMT_\([A-Z]*\).*/\1/g'

    // std::string command = "v4l2-ctl -d " + CAM + " --get-subdev-fmt 0 | grep \"Mediabus Code\" | sed 's/.*BUS_FMT_\([A-Z]*\).*/\1/g'";
    // // process.start("v4l2-ctl -d /dev/cam-csi1-port0 --get-subdev-fmt 0 | grep "Mediabus Code" | sed 's/.*BUS_FMT_\([A-Z]*\).*/\1/g'");
    // process.start(command.c_str());
    // process.waitForFinished(-1); // Warten, bis das Skript beendet ist

    // int returnCode = process.exitCode();
    // if (returnCode < 0)
    // {
    //     qDebug() << "ERROR: Could not get color";
    //     return;
    // }
    // QString output = process.readAllStandardOutput();
    // QString errorOutput = process.readAllStandardError();

    // QObject *cameraNameLabel = engine.rootObjects().first()->findChild<QObject*>("cameraNameLabel");
    // if (labelObject) {
    //     QQmlProperty::write(cameraNameLabel, "text", SENSOR.c_str());
    // }

    // TBD: What if I use bayer2rgbneon to to bayer conversion with gstreamer instead of opencv
    // std::string pipeline = "v4l2src device=" + device + " ! " + format + ", " + FRAMESIZE + " ! bayer2rgbneon ! queue ! appsink";
    std::string pipeline = "v4l2src device=" + device + " ! " + format + ", " + FRAMESIZE + " ! appsink";
    qDebug() << "pipeline: " << pipeline.c_str();

    cap = cv::VideoCapture(pipeline, cv::CAP_GSTREAMER); // generate VideoCapture object
    // cap.open(1);
    double fps = cap.get(cv::CAP_PROP_FPS);
    qDebug() << "fps: " << fps;
    // getControls();
    tUpdate.start(1000 / fps);
    // tUpdate.start(1000 / 30); // 30 fps
}

void CameraDemo::updateFrame()
{
    cv::Mat rawFrame;

    // cap >> frame; // TBD: When using gstreamer for bayser conversion
    cap >> rawFrame;
    cv::cvtColor(rawFrame, frame, cv::COLOR_BayerGB2RGB);

    QImage image = QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
    // QImage image = QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888).rgbSwapped();
    emit newImage(image);
}

OpencvImageProvider::OpencvImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    image = QImage(1280, 800, QImage::Format_RGB888);
    image.fill(QColor("blue"));
}

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
    this->image = image;
    emit imageChanged();
}



// ################# GETTER FUNCTIONS FOR UI #################
QString CameraDemo::getCameraName() const
{
    qDebug() << "GETTING CAMERA NAME";
    QString cameraName; // TBD: needed because const return parameter is expected
    if (SENSOR == "ar0144")
    {
        cameraName = "VM016 (ar0144)";
    }
    else if (SENSOR == "ar0521")
    {
        cameraName = "VM017 (ar0521)";
    }
    else
    {
        cameraName = "";
    }
    return cameraName;
}
QString CameraDemo::getFramesize() const
{
    qDebug() << "GETTING FRAMESIZE";
    return QString::fromStdString(FRAMESIZE);
}