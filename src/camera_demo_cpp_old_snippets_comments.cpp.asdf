qInfo() << "std out!";

// QCameraDevice cameras = QMediaDevices::videoInputs();
// qInfo() << QMediaDevices::videoInputs();
const QList<QCameraDevice> cameraDevices = QMediaDevices::videoInputs();
for (const QCameraDevice &cameraDevice : cameraDevices) {
	qInfo() << "Camera: " << cameraDevice;
	qInfo() << "Supported Formats _start: ";
	for (const QCameraFormat &cameraFormat : cameraDevice.videoFormats()) {
		qInfo() << "   Format: ";
		qInfo() << "      maxFrameRate: " << cameraFormat.maxFrameRate();
		qInfo() << "      minFrameRate: " << cameraFormat.minFrameRate();
		qInfo() << "      pixelFormat: " << cameraFormat.pixelFormat();
		qInfo() << "      resolution: " << cameraFormat.resolution();
	}
	qInfo() << "Supported Formats _end: ";

	QCamera camera(cameraDevice);
	qInfo() << "Device: " << camera.cameraDevice();
	qInfo() << "Resolution: " << camera.cameraFormat().resolution();
	qInfo() << "Pixel Format: " << camera.cameraFormat().pixelFormat();
	qInfo() << "Error: " << camera.error();

}

// ----------------------------------------------------------------------------------------------------------------------------------------

QProcess process;

process.start("/usr/bin/setup-pipeline-csi1");
process.waitForFinished(-1); // Warten, bis das Skript beendet ist

// Ausgabe abrufen
QString output = process.readAllStandardOutput();
QString errorOutput = process.readAllStandardError();

// Rückgabewert abrufen
int returnCode = process.exitCode();

qDebug() << "Ausgabe:" << output;
qDebug() << "Fehlerausgabe:" << errorOutput;
qDebug() << "Rückgabewert:" << returnCode;

// ----------------------------------------------------------------------------------------------------------------------------------------

std::string pipeline = "v4l2src device=/dev/video-csi1 ! video/x-bayer,format=grbg, width=1280, height=720 ! appsink";

// ----------------------------------------------------------------------------------------------------------------------------------------

QPixmap OpencvImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) # Image Provider using QPixmap (??)
{
    int width = 100;
    int height = 50;

    if (size)
        *size = QSize(width, height);
    QPixmap pixmap(requestedSize.width() > 0 ? requestedSize.width() : width,
                   requestedSize.height() > 0 ? requestedSize.height() : height);
    pixmap.fill(QColor(id).rgba());
    return pixmap;
}

// ----------------------------------------------------------------------------------------------------------------------------------------

std::string filePath = "/boot/bootenv.txt";

// Öffne die Datei zum Lesen und Schreiben.
std::fstream file(filePath, std::ios::in | std::ios::out);

if (!file.is_open()) {
    std::cerr << "Fehler beim Öffnen der Datei " << filePath << std::endl;
}

// Konstantes Array der zu entfernenden Substrings.
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

std::string line;

while (std::getline(file, line)) {
    // Entferne jeden Substring aus der Zeile.
    for (const char* substring : substringsToRemove) {
        size_t found = line.find(substring);
        while (found != std::string::npos) {
            line.erase(found, std::strlen(substring));
            found = line.find(substring, found);
        }
    }

    // Schreibe die bearbeitete Zeile zurück in die Datei.
    file << line << std::endl;
}

// Datei schließen.
file.close();