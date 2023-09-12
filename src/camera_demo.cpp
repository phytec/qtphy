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

#include "json.hpp"
using json = nlohmann::json;

// If on ISI: execute setup-pipeline-csi1
// Else if on ISP: complete the pipeline manually

// TBD: REMOVE THIS
// QObject *CameraDemo::singletontypeProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
// {
//     Q_UNUSED(engine);
//     Q_UNUSED(scriptEngine);

//     CameraDemo *cameraDemo = new CameraDemo();
//     return cameraDemo;
// }

CameraDemo::CameraDemo(QObject *parent) : QObject(parent)
{
    qDebug() << "CameraDemo constructor";
    connect(&tUpdate, &QTimer::timeout, this, &CameraDemo::updateFrame);
}

CameraDemo::~CameraDemo()
{
    close(subdevFd);
    cap.release();
    tUpdate.stop();

    // close(vd); // TBD: might throw error if not opened
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
        std::cerr << "ERROR: Could not read link" << std::endl;

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
            // emit sensorChanged();
            FRAMESIZE = "width=1280, height=800";
            // emit framesizeChanged();
            return 0;
        }
        else if (firstToken == "ar0521")
        {
            // std::string CAM_BW_FMT = "Y8_1X8";
            // std::string CAM_COL_FMT = "SGRBG8_1X8";
            // std::string SENSOR_RES = "2592x1944";
            // std::string OFFSET_SENSOR = "(0,0)";
            SENSOR = "ar0521";
            // emit sensorChanged();
            FRAMESIZE = "width=2592, height=1944";
            // emit framesizeChanged();
            return 1;
        }
        else
        {
            std::cerr << "ERROR: Unknown sensor (not a phycam?)" << std::endl;
            return -1;
        }
    }
    else
    {
        std::cerr << "ERROR: Could not read link" << std::endl;
        return -1;
    }
}

bool CameraDemo::getAutoExposure()
{
    // subdevFd not yet opened
    if (subdevFd < 0) {
        return false;
    }
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control)); // set memory of control to all zeros (for idempotenty)

    control.id = V4L2_CID_EXPOSURE_AUTO;
    if (ioctl(subdevFd, VIDIOC_G_CTRL, &control) == -1)
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

bool CameraDemo::getFlipHorizontal()
{
    // subdevFd not yet opened
    if (subdevFd < 0) {
        return false;
    }
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control)); // set memory of control to all zeros (for idempotenty)

    control.id = V4L2_CID_HFLIP;
    if (ioctl(subdevFd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: getting horizontal flip" << std::endl;
    }

    return control.value;
}

bool CameraDemo::getFlipVertical()
{
    // subdevFd not yet opened
    if (subdevFd < 0) {
        return false;
    }
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control)); // set memory of control to all zeros (for idempotenty)

    control.id = V4L2_CID_VFLIP;
    if (ioctl(subdevFd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: getting vertical flip" << std::endl;
    }

    return control.value;
}

int CameraDemo::getExposure()
{
    // subdevFd not yet opened
    if (subdevFd < 0) {
        return 0;
    }
    struct v4l2_control control;
    std::memset(&control, 0, sizeof(control)); // set memory of control to all zeros (for idempotenty)

    control.id = V4L2_CID_EXPOSURE;
    if (ioctl(subdevFd, VIDIOC_G_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: getting exposure" << std::endl;
    }

    return control.value;
}

void CameraDemo::setAutoExposure(bool value)
{
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

    if (ioctl(subdevFd, VIDIOC_S_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: setting auto exposure" << std::endl;
    }
}

void CameraDemo::setExposure(int value)
{
    struct v4l2_control control;
    control.id = V4L2_CID_EXPOSURE;
    control.value = value;

    if (ioctl(subdevFd, VIDIOC_S_CTRL, &control) == -1)
    {
        std::cerr << ("ERROR: setting exposure") << std::endl;
    }
}

void CameraDemo::setFlipVertical(bool value)
{
    struct v4l2_control control;
    control.id = V4L2_CID_VFLIP;
    control.value = value;

    if (ioctl(subdevFd, VIDIOC_S_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: setting vertical flip" << std::endl;
    }
}

void CameraDemo::setFlipHorizontal(bool value)
{
    struct v4l2_control control;
    control.id = V4L2_CID_HFLIP;
    control.value = value;

    if (ioctl(subdevFd, VIDIOC_S_CTRL, &control) == -1)
    {
        std::cerr << "ERROR: setting horizontal flip" << std::endl;
    }
}





int CameraDemo::isp_ioctl(const char *cmd, json& jsonRequest, json& jsonResponse) {
    if (vd_fd < 0) {
        std::cerr << "video device file descriptor < 0" << std::endl;
        return -1;
    }
    if (!cmd) {
        std::cerr << "cmd should not be null!" << std::endl;
        return -1;
    }

    jsonRequest["id"] = cmd;
    jsonRequest["streamid"] = streamid;

    struct v4l2_ext_controls ecs;
    struct v4l2_ext_control ec;
    memset(&ecs, 0, sizeof(ecs));
    memset(&ec, 0, sizeof(ec));
    ec.string = new char[64 * 1024];
    ec.id = V4L2_CID_VIV_EXTCTRL;
    ec.size = 0;
    ecs.controls = &ec;
    ecs.count = 1;

    ioctl(vd_fd, VIDIOC_G_EXT_CTRLS, &ecs);

    // --- initialized --- //
    std::cout << "a" << ec.string << std::endl;

    std::cout << "b" << jsonRequest.dump().c_str() << std::endl;

    strcpy(ec.string, jsonRequest.dump().c_str());
    std::cout << "c" << ec.string << std::endl;

    int ret = ioctl(vd_fd, VIDIOC_S_EXT_CTRLS, &ecs);
    std::cout << "d" << ec.string << std::endl;
    if (ret != 0) {
        std::cerr << "failed to set ext ctrl\n" << std::endl;
        goto end;
    } else {
        ioctl(vd_fd, VIDIOC_G_EXT_CTRLS, &ecs);
        std::cout << "e" << ec.string << std::endl;

        std::string res = ec.string;
        jsonResponse = json::parse(res);
        std::cout << "f" << res << std::endl;
        // Json::Reader reader;
        // reader.parse(ec.string, jsonResponse, true);
        delete[] ec.string;
        ec.string = NULL;
        return 0;
    }

end:
    delete ec.string;
    ec.string = NULL;
    // return S_EXT_FLAG;
    return 555;
}

int CameraDemo::isp_read_ioctl(const char *cmd, json& jsonRequest, json& jsonResponse) {
    std::cout << "READ IOCTL" << std::endl;
    if (vd_fd < 0) {
        std::cerr << "video device file descriptor < 0" << std::endl;
        return -1;
    }
    if (!cmd) {
        std::cerr << "cmd should not be null!" << std::endl;
        return -1;
    }

    jsonRequest["id"] = cmd;
    jsonRequest["streamid"] = streamid;

    struct v4l2_ext_controls ecs;
    struct v4l2_ext_control ec;
    memset(&ecs, 0, sizeof(ecs));
    memset(&ec, 0, sizeof(ec));
    ec.id = V4L2_CID_VIV_EXTCTRL;
    ec.size = 64 * 1024;
    ec.string = new char[ec.size];
    ecs.controls = &ec;
    ecs.count = 1;
    ecs.ctrl_class = V4L2_CTRL_CLASS_USER;

    ioctl(vd_fd, VIDIOC_G_EXT_CTRLS, &ecs);

    // --- initialized --- //




    strcpy(ec.string, jsonRequest.dump().c_str());
    
    int ret = ioctl(vd_fd, VIDIOC_S_EXT_CTRLS, &ecs);




    if (ret != 0) {
        std::cerr << "failed to set ext ctrl\n" << std::endl;
        goto end;
    } else {
        memset(ec.string, 0, ec.size);
        std::cout << "ee" << ec.string << std::endl;
        // std::cout << ecs << std::endl;
        ioctl(vd_fd, VIDIOC_G_EXT_CTRLS, &ecs);
        std::cout << "e" << ec.string << std::endl;

        std::string res = ec.string;
        jsonResponse = json::parse(res);
        std::cout << "f" << res << std::endl;
        // Json::Reader reader;
        // reader.parse(ec.string, jsonResponse, true);
        delete[] ec.string;
        ec.string = NULL;
        return 0;
    }

end:
    delete ec.string;
    ec.string = NULL;
    // return S_EXT_FLAG;
    return 555;
}



bool CameraDemo::getDwe()
{
    if (vd_fd >= 0 && VIDEO_SRC == ISP) {

    }
    else {
        return false;
    }
}
void CameraDemo::setDwe(bool value)
{
    json jRequest, jResponse;
    jRequest["bypass"] = !value;
    isp_ioctl("dwe.s.bypass", jRequest, jResponse);
    qDebug() << "set DWE";
    return;
}

bool CameraDemo::getAwb()
{
    if (vd_fd >= 0 && VIDEO_SRC == ISP) {

    }
    else {
        return false;
    }
}
void CameraDemo::setAwb(bool value)
{
    json jRequest, jResponse;
    jRequest["enable"] = value;
    isp_ioctl("awb.s.en", jRequest, jResponse);
    std::cout << jRequest << std::endl;
    qDebug() << "set AWB";

    // set white balance:

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

    // isp.write_json(request)

    return;
}

bool CameraDemo::getLsc()
{
    if (vd_fd >= 0 && VIDEO_SRC == ISP) {

    }
    else {
        return false;
    }
}

void CameraDemo::setLsc(bool value)
{
    json jRequest, jResponse;
    jRequest["enable"] = value;
    isp_ioctl("lsc.s.en", jRequest, jResponse);
    qDebug() << "set LSC";
    return;
}

bool CameraDemo::getAec()
{
    if (vd_fd >= 0 && VIDEO_SRC == ISP) {
        json jRequest, jResponse;
        // jRequest["enable"] = value;
        isp_read_ioctl("ae.g.en", jRequest, jResponse);
        qDebug() << "get AEC";
        std::cout << jResponse.dump() << std::endl;
        // return jResponse["enable"];
        return true;
    }
    else {
        return false;
    }
}

void CameraDemo::setAec(bool value)
{
    json jRequest, jResponse;
    jRequest["enable"] = value;
    isp_ioctl("ae.s.en", jRequest, jResponse);
    qDebug() << "set AEC";
    std::cout << jResponse.dump() << std::endl;

    return;
}


void CameraDemo::openCamera()
{
    cap.release();
    // DETECT CAMERA
    if (getCAM() == -1)
    {
        std::cerr << "ERROR: NO CAMERA FOUND" << std::endl;
        // execute detectCamera Script and open Error MessageBox
        QProcess process;

        process.start("/root/detectCamera.sh");
        process.waitForFinished(-1); // Warten, bis das Skript beendet ist

        // Ausgabe abrufen
        QString output = process.readAllStandardOutput();
        // QString errorOutput = process.readAllStandardError();

        // Rückgabewert abrufen
        int returnCode = process.exitCode();

        QString toRemove = "\nTry loading the following overlays by adding them to /boot/bootenv.txt and reboot\n";
        output.remove(toRemove);

        RECOMMENDED_OVERLAYS = output;
        qDebug() << "Ausgabe:" << RECOMMENDED_OVERLAYS;
        // qDebug() << "Ausgabe:" < output

        // qDebug() << "Fehlerausgabe:" << errorOutput;
        qDebug() << "Rückgabewert:" << returnCode;

        emit recommendedOverlaysChanged();
        ERROR = returnCode;
        emit errorDialogChanged();
        return;
    }

    subdevFd = open(CAM.c_str(), O_RDWR);
    if (subdevFd < 0)
    {
        // qCritical() << "Error: Could not open subdev";
        std::cerr << "ERROR: could not open subdev" << std::endl;
        return;
    }

    // GET SUBDEVICE AND INTERFACE
    if (CAM == "/dev/cam-csi1" || CAM == "/dev/cam-csi1-port0")
    {
        INTERFACE = "CSI1";
        vd = "/dev/video-csi1";
        if (system("/usr/bin/setup-pipeline-csi1") != 0) // call setup-pipeline script
        {
            std::cerr << "ERROR: setup-pipeline-csi1 failed" << std::endl;
            return;
        }
    }
    else if (CAM == "/dev/cam-csi1-port1")
    {
        INTERFACE = "CSI1, PORT 1";
        vd = "/dev/video-csi1";
        if (system("/usr/bin/setup-pipeline-csi1 -p 1") != 0) // call setup-pipeline script with port 1
        {
            std::cerr << "ERROR: setup-pipeline-csi1 failed" << std::endl;
            return;
        }
    }
    else if (CAM == "/dev/cam-csi2" || CAM == "/dev/cam-csi2-port0")
    {
        INTERFACE = "CSI2";
        vd = "/dev/video-csi2";
        if (system("/usr/bin/setup-pipeline-csi2") != 0) // call setup-pipeline script
        {
            std::cerr << "ERROR: setup-pipeline-csi2 failed" << std::endl;
            return;
        }
    }
    else if (CAM == "/dev/cam-csi2-port1")
    {
        INTERFACE = "CSI2, PORT 1";
        vd = "/dev/video-csi2";
        if (system("/usr/bin/setup-pipeline-csi2 -p 1") != 0) // call setup-pipeline script
        {
            std::cerr << "ERROR: setup-pipeline-csi2 failed" << std::endl;
            return;
        }
    }
    else // No camera found
    {
        // TBD: Show Text in Image Preview ("NO CAMERA FOUND")
        INTERFACE = "NO CAMERA FOUND";
        return;
    }
    // emit interfaceChanged();

    // GET SENSOR AND FRAMESIZE
    if (getSensor() < 0)
    {
        std::cerr << "ERROR: No sensor found (could not determine SENSOR and FRAMESIZE)" << std::endl;
        return;
    }

    // TBD: equivalent for isp-csi2
    // CHECK ISI / ISP, SET FORMAT AND VIDEO_SRC
    if (access("/dev/isp-csi1", F_OK) == 0) // ISP is used
    {
        FORMAT = "video/x-raw,format=YUY2"; // set format to YUY2
        VIDEO_SRC = ISP;
    }
    else // ISI is used
    {
        FORMAT = "video/x-bayer,format=grbg"; // set format to grbg
        VIDEO_SRC = ISI;
    }
    // emit videoSrcChanged();
    // emit formatChanged();

    // TBD: What if I use bayer2rgbneon to to bayer conversion with gstreamer instead of opencv
    // std::string pipeline = "v4l2src vd=" + vd + " ! " + FORMAT + ", " + FRAMESIZE + " ! bayer2rgbneon ! queue ! appsink";
    std::string pipeline = "v4l2src vd=" + vd + " ! " + FORMAT + ", " + FRAMESIZE + " ! appsink";
    qDebug() << "pipeline: " << pipeline.c_str();


    // init ISP
    if (VIDEO_SRC == ISP)
    {
        vd_fd = open(vd.c_str(), O_RDWR | O_NONBLOCK, 0);
        if (vd_fd == -1) {
            std::cerr << "Failed to open video device" << std::endl;
            return;
        }
        emit dweChanged();
        emit awbChanged();
        emit lscChanged();
        emit aecChanged();
    }



    // Rufen Sie Ihre Funktion toggle_features auf, um sie in Ihrem Projekt zu implementieren.
    // Stellen Sie sicher, dass Sie die erforderlichen Argumente wie waittime, red, green, blue usw. an Ihre Funktion übergeben.

    emit framesizeChanged();
    emit sensorChanged();
    emit autoExosureChanged();
    emit flipVerticalChanged();
    emit flipHorizontalChanged();
    emit exposureChanged();
    emit formatChanged();
    emit videoSrcChanged();
    emit interfaceChanged();


    cap = cv::VideoCapture(pipeline, cv::CAP_GSTREAMER); // generate VideoCapture object
    double fps = cap.get(cv::CAP_PROP_FPS); // get FPS
    qDebug() << "fps: " << fps;
    tUpdate.start(1000 / fps);
}

void CameraDemo::updateFrame()
{
    cv::Mat rawFrame;
    cap >> rawFrame;

    if (VIDEO_SRC == ISP)
    {
        cv::cvtColor(rawFrame, frame, cv::COLOR_YUV2BGR_YUY2);
        QImage image = QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
        emit newImage(image);
    }
    else {
        cv::cvtColor(rawFrame, frame, cv::COLOR_BayerGB2RGB);
        QImage image = QImage(frame.data, frame.cols, frame.rows, QImage::Format_RGB888);
        emit newImage(image);
    }
}

void CameraDemo::reloadOverlays()
{
    const char* substringsToRemove[] = {
        " imx8mp-isi-csi1.dtbo",
        " imx8mp-isi-csi2.dtbo",
        " imx8mp-isp-csi1.dtbo",
        " imx8mp-isp-csi2.dtbo",
        " imx8mp-vm016-csi1-fpdlink-port0.dtbo",
        " imx8mp-vm016-csi1-fpdlink-port1.dtbo",
        " imx8mp-vm016-csi1.dtbo",
        " imx8mp-vm016-csi2-fpdlink-port0.dtbo",
        " imx8mp-vm016-csi2-fpdlink-port1.dtbo",
        " imx8mp-vm016-csi2.dtbo",
        " imx8mp-vm017-csi1-fpdlink-port0.dtbo",
        " imx8mp-vm017-csi1-fpdlink-port1.dtbo",
        " imx8mp-vm017-csi1.dtbo",
        " imx8mp-vm017-csi2-fpdlink-port0.dtbo",
        " imx8mp-vm017-csi2-fpdlink-port1.dtbo",
        " imx8mp-vm017-csi2.dtbo",
        " imx8mp-vm020-csi1.dtbo",
        " imx8mp-vm020-csi2.dtbo"
    };
    const char* filename = "/boot/bootenv.txt";

    for (int i = 0; i < 18; i++) {
        std::string sed_command = "sed -i 's/";
        sed_command += substringsToRemove[i];
        sed_command += "/";
        // sed_command += replacement_string;
        sed_command += "/g' ";
        sed_command += filename;

        int exit_code = system(sed_command.c_str());

        if (exit_code < 0)
        {
            std::cerr << "Error modifying /boot/bootenv.txt" << std::endl;
            return;
        }
    }

    std::ofstream file(filename, std::ios::app); // open file in append mode
    if (!file) {
        std::cerr << "Error opening /boot/bootenv.txt" << std::endl;
        return;
    }

    qDebug() << RECOMMENDED_OVERLAYS;
    std::string overlays = RECOMMENDED_OVERLAYS.toStdString();
    overlays.pop_back(); // remove last \n
    overlays = overlays.substr(overlays.find_last_of('\n')+1);

    qDebug() << QString::fromStdString(overlays);

    // std::replace(overlays.begin(), overlays.end(), '\n', 'z');
    std::cout << overlays << std::endl;
    file << " " << overlays; // append overlays to bootenv.txt
    file.close();

    system("reboot now");
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
    return QString::fromStdString(FRAMESIZE);
}
QString CameraDemo::getFormat() const
{
    return QString::fromStdString(FORMAT);
}
QString CameraDemo::getInterface() const
{
    return QString::fromStdString(INTERFACE);
}
QString CameraDemo::getVideoSrc() const
{
    std::string ret = "";
    if (VIDEO_SRC == ISP) {
        ret = "ISP";
    }
    else if (VIDEO_SRC == ISI) {
        ret = "ISI";
    }
    else {
        ret = "";
    }
    return QString::fromStdString(ret);
}

QString CameraDemo::getRecommendedOverlays() const
{
    return RECOMMENDED_OVERLAYS;
}

int CameraDemo::getErrorDialog()
{
    return ERROR;
}


