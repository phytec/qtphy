/*
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021 PHYTEC Messtechnik GmbH
 */

#include <QtCore>
#include <QApplication>
#include <QQuickWindow>
#include <QImage>
#include <QPainter>
#include <QDebug>

#include <opencv2/opencv.hpp>

#include <iostream>
#include <string>
#include <limits.h>
#include <fstream>
#include <unistd.h>

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

#include "camera_demo.hpp"

#include <json.hpp>
using json = nlohmann::json;

Sensor SENSORS[] = {
    {"", "", false, 0, 0, 0, 0, 0, 0},                          // default
    {"VM016", "ar0144", true, 1280, 800, 1280, 800, 0, 0},      // vm016
    {"VM017", "ar0521", false, 2560, 1440, 1280, 720, 16, 252}, // vm017
    {"VM020", "ar0234", true, 2560, 1440, 1280, 720, 16, 252},  // vm020 (TBD)
    {"---", "---", false, 0, 0, 0, 0, 0, 0}                     // Sentinel value to indicate the end

};

PhyCam::PhyCam(int _interface, Phytec_board _host_hardware) : csi_interface(_interface), host_hardware(_host_hardware)
{
    // Check if camera device can be found in /dev
    device = "/dev/cam-csi" + std::to_string(csi_interface);
    if (access(device.c_str(), F_OK) == 0)
    { // phycam-M on csi-1
        port = -1;
    }
    else if (access((device + "-port0").c_str(), F_OK) == 0)
    { // phycam-L on csi-1, port0
        device += "-port0";
        port = 0;
    }
    else if (access((device + "-port1").c_str(), F_OK) == 0)
    { // phycam-L on csi-1, port1
        device += "-port1";
        port = 1;
    }
    else
    {
        status = UNCONNECTED;
        return;
    }
    if (host_hardware == IMX8MP_POLLUX)
    {
        // Check if isp and isi overlays are loaded
        if ((access(("/dev/video-isp-csi" + std::to_string(csi_interface)).c_str(), F_OK) != 0) ||
            (access(("/dev/video-isi-csi" + std::to_string(csi_interface)).c_str(), F_OK) != 0))
        {
            std::cerr << "ERROR: Please load isi and isp overlay for your camera" << std::endl;
            status = UNCONNECTED;
            return;
        }
    }
    // Open device_fd
    device_fd = open(device.c_str(), O_RDWR);
    if (device_fd == -1)
    {
        std::cerr << "ERROR: Could not open device fd" << std::endl;
        status = ERROR;
        return;
    }
    if (host_hardware == IMX8MP_POLLUX)
    {
        // Open isp_fd
        isp_fd = open(("/dev/video-isp-csi" + std::to_string(csi_interface)).c_str(), O_RDWR | O_NONBLOCK, 0);
        if (isp_fd == -1)
        {
            std::cerr << "ERROR: Could not open isp fd" << std::endl;
            status = ERROR;
            return;
        }
    }
    // Get sensor of connected camera (includes framesize etc.)
    if (getSensor() < 0)
    {
        std::cerr << "ERROR: Failed to get sensor" << std::endl;
        status = ERROR;
        return;
    }
    // Construct setup_pipeline_command
    setup_pipeline_command = "/usr/bin/setup-pipeline-csi" + std::to_string(csi_interface);
    if (port == 1)
    {
        setup_pipeline_command += " -p 1";
    }
    setup_pipeline_command += " -c " + std::to_string(sensor->sensor_width) + "x" + std::to_string(sensor->sensor_height);
    setup_pipeline_command += " -s " + std::to_string(sensor->frame_width) + "x" + std::to_string(sensor->frame_height);
    setup_pipeline_command += " -o \"(" + std::to_string(sensor->offset_x) + "," + std::to_string(sensor->offset_y) + ")\"";

    // Construct gstreamer pipelines:
    std::string framesize = "width=" + std::to_string(sensor->frame_width) + ", height=" + std::to_string(sensor->frame_height);
    if (host_hardware == IMX8MP_POLLUX)
    {
        isp_pipeline = "v4l2src device=/dev/video-isp-csi" + std::to_string(csi_interface) + " ! video/x-raw,format=YUY2, " + framesize + " ! appsink";
        isi_pipeline = "v4l2src device=/dev/video-isi-csi" + std::to_string(csi_interface) + " ! video/x-bayer,format=grbg, " + framesize + " ! appsink";
    }
    else if (host_hardware == IMX8MM_POLIS)
    {
        isi_pipeline = "v4l2src device=/dev/video-csi" + std::to_string(csi_interface) + " ! video/x-bayer,format=grbg, " + framesize + " ! appsink";
        isp_pipeline = "";
    }
    status = READY;
}
PhyCam::~PhyCam()
{
    close(device_fd);
    close(isp_fd);
}

int PhyCam::setup_pipeline()
{
    return system(setup_pipeline_command.c_str());
}

int PhyCam::getSensor()
{
    char v4l_subdev[PATH_MAX];
    ssize_t len = readlink(device.c_str(), v4l_subdev, sizeof(v4l_subdev) - 1);
    if (len != -1)
    {
        v4l_subdev[len] = '\0';
        std::string entityPath = "/sys/class/video4linux/";
        entityPath += v4l_subdev;
        entityPath += "/device/name";

        std::ifstream entityFile(entityPath);
        std::string sensor_name;

        if (entityFile.is_open())
        {
            std::getline(entityFile, sensor_name);
            entityFile.close();
        }
        for (int i = 0; SENSORS[i].name != "---"; i++)
        {
            if (SENSORS[i].name == sensor_name)
            {
                sensor = &SENSORS[i];
                return 0;
            }
        }
        std::cerr << "ERROR: Unknown sensor (not a phycam?)" << std::endl;
        return -1;
    }
    else
    {
        std::cerr << "ERROR: Could not read link" << std::endl;
        return -1;
    }
}

CameraDemo::CameraDemo(QObject *parent) : QObject(parent)
{
    connect(&tUpdate, &QTimer::timeout, this, &CameraDemo::updateFrame);
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0)
    {
        std::string hostStr(hostname);

        if (hostStr.find("phyboard-polis-imx8mm") != std::string::npos)
        {
            host_hardware = IMX8MM_POLIS;
            cam1 = new PhyCam(1, host_hardware);
            cam2 = cam1;
            CAM = cam1;
            emit hostHardwareChanged();
            return;
        }
        else if (hostStr.find("phyboard-pollux-imx8mp") != std::string::npos)
        {
            host_hardware = IMX8MP_POLLUX;
            cam1 = new PhyCam(1, host_hardware);
            cam2 = new PhyCam(2, host_hardware);
            CAM = cam1;
            emit hostHardwareChanged();
        }
        else
        {
            std::cerr << "ERROR: Unknown hostname, your hardware is not supported by the qtphy demo" << std::endl;
            // exit(-1);
            // TBD: return to start page
        }
    }
    else
    {
        std::cerr << "ERROR: Unable to get hostname" << std::endl;
        // exit(-1);
        // TBD: return to start page
    }
}

CameraDemo::~CameraDemo()
{
    tUpdate.stop();
    cap.release();
    if (cam2 == cam1)
    {
        delete cam1;
    }
    else
    {
        delete cam1;
        delete cam2;
    }
}

int CameraDemo::isp_ioctl(const char *cmd, json &jsonRequest, json &jsonResponse)
{
    if (CAM->isp_fd < 0)
    {
        std::cerr << "ERROR: ISP file descriptor < 0" << std::endl;
        return -1;
    }
    if (!cmd)
    {
        return -1;
    }

    jsonRequest["id"] = cmd;
    jsonRequest["streamid"] = 0;

    struct v4l2_ext_controls ecs;
    struct v4l2_ext_control ec;
    memset(&ecs, 0, sizeof(ecs));
    memset(&ec, 0, sizeof(ec));
    ec.string = new char[64 * 1024];
    ec.id = V4L2_CID_VIV_EXTCTRL;
    ec.size = 0;
    ecs.controls = &ec;
    ecs.count = 1;

    ioctl(CAM->isp_fd, VIDIOC_G_EXT_CTRLS, &ecs);

    // --- Initialized --- //

    strcpy(ec.string, jsonRequest.dump().c_str());

    // Set V4L2-control
    int ret = ioctl(CAM->isp_fd, VIDIOC_S_EXT_CTRLS, &ecs);
    if (ret != 0)
    {
        std::cerr << "ERROR: Failed to set ISP ext ctrl\n"
                  << std::endl;
        delete[] ec.string;
        ec.string = NULL;
        return 555;
    }
    else
    {
        // Get V4L2-control
        ioctl(CAM->isp_fd, VIDIOC_G_EXT_CTRLS, &ecs);

        std::string res = ec.string;
        jsonResponse = json::parse(res);
        delete[] ec.string;
        ec.string = NULL;
        return 0;
    }
}

void CameraDemo::setDwe(bool value)
{
    json jRequest, jResponse;
    jRequest["bypass"] = !value;
    isp_ioctl("dwe.s.bypass", jRequest, jResponse);
    return;
}

void CameraDemo::setAwb(bool value)
{
    // Enable AWB
    json jRequest, jResponse;
    jRequest["enable"] = value;
    isp_ioctl("awb.s.en", jRequest, jResponse);

    // Configure AWB parameters
    json request = json::parse(R"(
        {
        "matrix": [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0],
        "offset": {
            "blue": 0,
            "green": 0,
            "red": 0
        },
        "wb.gains": {
            "blue": 1.0,
            "green.b": 1.0,
            "green.r": 1.0,
            "red": "red"
        }
        }
    )");
    isp_ioctl("wb.s.cfg", jRequest, jResponse);
    return;
}

void CameraDemo::setLsc(bool value)
{
    json jRequest, jResponse;
    jRequest["enable"] = value;
    isp_ioctl("lsc.s.en", jRequest, jResponse);
    return;
}

void CameraDemo::setAec(bool value)
{
    json jRequest, jResponse;
    jRequest["enable"] = value;
    isp_ioctl("ae.s.en", jRequest, jResponse);
    return;
}

void CameraDemo::openCamera()
{
    if (cam1->status == ACTIVE || cam2->status == ACTIVE)
    {
        return;
    }
    if (cam1->status == READY && cam2->status == READY)
    {
        CAM = cam1;
        STATUS = DUAL_CAM;
    }
    else
    {
        QProcess process;
        QStringList arguments;
        arguments << "detectCamera"
                  << "-m";

        process.start("/bin/sh", arguments);
        process.waitForFinished(-1);

        QString output = process.readAllStandardOutput();
        int returnCode = process.exitCode();

        RECOMMENDED_OVERLAYS = output;

        emit recommendedOverlaysChanged();

        if (returnCode == 0)
        {
            // Additional cameras found (reload overlays to use them)
            STATUS = WRONG_OVERLAYS;
            emit statusChanged();
            return;
        }
        // else if (returnCode == 1) // no additional camera found
        else if (returnCode == 2)
        {
            // No camera connected
            STATUS = NO_CAM;
        }

        if (cam1->status == READY)
        {
            STATUS = SINGLE_CAM;
            CAM = cam1;
        }
        else if (cam2->status == READY)
        {
            STATUS = SINGLE_CAM;
            CAM = cam2;
        }
    }

    // Start capturing video
    if (host_hardware == IMX8MP_POLLUX)
    {
        CAM->video_src = ISP;
        cap = cv::VideoCapture(CAM->isp_pipeline, cv::CAP_GSTREAMER);
    }
    else
    {
        CAM->video_src = ISI;
        cap = cv::VideoCapture(CAM->isi_pipeline, cv::CAP_GSTREAMER);
    }
    double fps = cap.get(cv::CAP_PROP_FPS);
    tUpdate.start(1000 / fps);
    CAM->status = ACTIVE;

    // Emit signals to update GUI
    emit statusChanged();
    emit framesizeChanged();
    emit sensorChanged();
    emit autoExposureChanged();
    emit hasAutoExposureChanged();
    emit flipVerticalChanged();
    emit flipHorizontalChanged();
    emit exposureChanged();
    emit videoSrcChanged();
    emit interfaceChanged();

}

void CameraDemo::updateFrame()
{
    cv::Mat rawFrame;
    cap >> rawFrame;

    if (CAM->video_src == ISP)
    {
        cv::cvtColor(rawFrame, frame, cv::COLOR_YUV2RGB_YUY2);
        QImage image = QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
        emit newImage(image);
    }
    else
    {
        cv::cvtColor(rawFrame, frame, cv::COLOR_BayerGB2RGB);
        QImage image = QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
        emit newImage(image);
    }
}

void CameraDemo::reloadOverlays()
{
    std::string command = "/root/detectCamera.sh -s \"" + RECOMMENDED_OVERLAYS.toStdString() + "\"";
    system(command.c_str());
}

// ################# OpencvImageProvider #################
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
    std::string ret = CAM->sensor->camera_name + " (" + CAM->sensor->name + ")";
    return QString::fromStdString(ret);
}
QString CameraDemo::getFramesize() const
{
    std::string ret = std::to_string(CAM->sensor->frame_width) + "x" + std::to_string(CAM->sensor->frame_height);
    return QString::fromStdString(ret);
}
QString CameraDemo::getInterfaceString() const
{
    std::string ret = "CSI" + std::to_string(CAM->csi_interface);
    if (CAM->port >= 0)
    {
        ret += ", PORT" + std::to_string(CAM->port);
    }
    return QString::fromStdString(ret);
}
int CameraDemo::getInterface() const
{
    return CAM->csi_interface;
}
int CameraDemo::getVideoSrc() const
{
    return CAM->video_src;
}

QString CameraDemo::getRecommendedOverlays() const
{
    return RECOMMENDED_OVERLAYS;
}

Status CameraDemo::getStatus()
{
    return STATUS;
}

bool CameraDemo::getAutoExposure()
{
    if (CAM->device_fd < 0)
    {
        return false;
    }
    if (!CAM->sensor->hasAutoExposure)
    {
        return 0;
    }
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control));

    control.id = V4L2_CID_EXPOSURE_AUTO;
    if (ioctl(CAM->device_fd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: getting exposure mode" << std::endl;
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
        std::cerr << "ERROR: Unknown exposure mode" << std::endl;
        return false;
    }
}

bool CameraDemo::getHasAutoExposure()
{
    return CAM->sensor->hasAutoExposure;
}

bool CameraDemo::getFlipHorizontal()
{
    if (CAM->device_fd < 0)
    {
        return false;
    }
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control));

    control.id = V4L2_CID_HFLIP;
    if (ioctl(CAM->device_fd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: getting horizontal flip" << std::endl;
    }

    return control.value;
}

bool CameraDemo::getFlipVertical()
{
    if (CAM->device_fd < 0)
    {
        return false;
    }
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control));

    control.id = V4L2_CID_VFLIP;
    if (ioctl(CAM->device_fd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: getting vertical flip" << std::endl;
    }

    return control.value;
}

int CameraDemo::getExposure()
{
    if (CAM->device_fd < 0)
    {
        return 0;
    }
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control));

    control.id = V4L2_CID_EXPOSURE;
    if (ioctl(CAM->device_fd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: getting exposure" << std::endl;
    }

    return control.value;
}

int CameraDemo::getHostHardware()
{
    return host_hardware;
}

// ################# SLOTS (Called from UI) #################
void CameraDemo::setVideoSource(Video_srcs value)
{
    tUpdate.stop();
    cap.release();

    if (value == ISP)
    {
        CAM->video_src = ISP;
        cap = cv::VideoCapture(CAM->isp_pipeline, cv::CAP_GSTREAMER);
        double fps = cap.get(cv::CAP_PROP_FPS);
        tUpdate.start(1000 / fps);
    }
    else if (value == ISI)
    {
        CAM->video_src = ISI;
        CAM->setup_pipeline(); // setup pipeline every time ISP is switched to ISI
        // sleep(1);
        cap = cv::VideoCapture(CAM->isi_pipeline, cv::CAP_GSTREAMER);
        double fps = cap.get(cv::CAP_PROP_FPS);
        tUpdate.start(1000 / fps);
    }
    emit videoSrcChanged();
}

void CameraDemo::setInterface(CSI_interface value)
{
    if (CAM->status == ACTIVE)
    {
        tUpdate.stop();
        cap.release();
        CAM->status = READY;
    }

    if (value == CSI1 && cam1->status == READY)
    {
        CAM = cam1;
    }
    else if (value == CSI2 && cam2->status == READY)
    {
        CAM = cam2;
    }
    else
    {
        std::cerr << "ERROR: Camera not ready" << std::endl;
        return;
    }

    CAM->setup_pipeline();
    // sleep(1);

    if (CAM->video_src == ISP)
    {
        cap = cv::VideoCapture(CAM->isp_pipeline, cv::CAP_GSTREAMER);
    }
    else
    {
        cap = cv::VideoCapture(CAM->isi_pipeline, cv::CAP_GSTREAMER);
    }
    double fps = cap.get(cv::CAP_PROP_FPS);
    tUpdate.start(1000 / fps);
    CAM->status = ACTIVE;

    emit framesizeChanged();
    emit sensorChanged();
    emit autoExposureChanged();
    emit hasAutoExposureChanged();
    emit flipVerticalChanged();
    emit flipHorizontalChanged();
    emit exposureChanged();
    emit interfaceChanged();
    emit videoSrcChanged();
}

void CameraDemo::setAutoExposure(bool value)
{
    if (!CAM->sensor->hasAutoExposure)
    {
        std::cerr << "ERROR: This camera has no auto exposure" << std::endl;
        return;
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
        emit exposureChanged(); // if auto exposure is disabled, update the exposure slider
    }

    if (ioctl(CAM->device_fd, VIDIOC_S_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: Can't set auto exposure" << std::endl;
    }
    emit autoExposureChanged();
}

void CameraDemo::setExposure(int value)
{
    struct v4l2_control control;
    control.id = V4L2_CID_EXPOSURE;
    control.value = value;

    if (ioctl(CAM->device_fd, VIDIOC_S_CTRL, &control) == -1)
    {
        std::cerr << ("ERROR: Can't set exposure") << std::endl;
    }
}

void CameraDemo::setFlipVertical(bool value)
{
    struct v4l2_control control;
    control.id = V4L2_CID_VFLIP;
    control.value = value;

    if (ioctl(CAM->device_fd, VIDIOC_S_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: Can't set vertical flip" << std::endl;
    }
}

void CameraDemo::setFlipHorizontal(bool value)
{
    struct v4l2_control control;
    control.id = V4L2_CID_HFLIP;
    control.value = value;

    if (ioctl(CAM->device_fd, VIDIOC_S_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: Can't set horizontal flip" << std::endl;
    }
}