/*!

SerialPort - A wrapper library around QSerialPort with a worker thread handling the port communication.


MIT License

Copyright (c) 2026-now  Andreas Nicolai <andreas.nicolai@gmx.net>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include "SP_SerialPortWorker.h"

#include <QDebug>
#include <QDateTime>
#include <QMutexLocker>
#include <QTimer>

#include <QLoggingCategory>
// logging category for everything related to serial communication within the worker thread
Q_LOGGING_CATEGORY(lcSerialWorker, "serial.worker")

namespace SP {

SerialPortWorker::~SerialPortWorker() {
	if (m_serialPort != nullptr && m_serialPort->isOpen())
		m_serialPort->close();
	delete m_serialPort;
	qCDebug(lcSerialWorker) << "Destructor called";
}


void SerialPortWorker::onOpenPort(const QString & portName, int baudRate, int dataBits, int parity, int stopBits) {
	// Close any existing connection
	if (m_serialPort->isOpen()) {
		m_serialPort->close();
		emit connectionStatusChanged(false);
	}

	// Configure serial port
	m_serialPort->setPortName(portName);
	m_serialPort->setBaudRate(baudRate);
	m_serialPort->setDataBits((QSerialPort::DataBits)dataBits);
	m_serialPort->setParity((QSerialPort::Parity)parity);
	m_serialPort->setStopBits((QSerialPort::StopBits)stopBits);
	m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

	// Attempt to open - note: regardless of the Qt documentation, this may have a timeout included of several seconds
	// and block the thread in the meantime
	bool success = m_serialPort->open(QIODevice::ReadWrite);
	QString errorMsg = success ? QString() : m_serialPort->errorString();

	if (success) {
		qCDebug(lcSerialWorker) << tr("Serial port '%1'opened successfully").arg(portName);
		emit connectionStatusChanged(success);
	}
	else {
		emit errorOccurred(errorMsg);
		qCCritical(lcSerialWorker) << "Failed to open serial port:" << portName << "\nError:" << errorMsg;
	}
}


void SerialPortWorker::onClosePort() {
	if (m_serialPort != nullptr && m_serialPort->isOpen()) {
		m_serialPort->close();
		emit connectionStatusChanged(false);
		qCDebug(lcSerialWorker) << "Serial port closed";
	}
}


void SerialPortWorker::onWriteData(const QByteArray & binaryDataBlock) {
	Q_ASSERT(m_serialPort != nullptr);
	if (!m_serialPort->isOpen()) {
		qCCritical(lcSerialWorker) << "Serial port is not open, cannot write data";
		emit errorOccurred(tr("Cannot write: port not open"));
		return;
	}

	qint64 bytesWritten = m_serialPort->write(binaryDataBlock);

	if (bytesWritten == -1)
		emit errorOccurred(m_serialPort->errorString());
	else if (bytesWritten != binaryDataBlock.size()) {
		emit errorOccurred(tr("Partial write: %1 of %2 bytes written")
			.arg(bytesWritten).arg(binaryDataBlock.size()));
	} else {
		qCDebug(lcSerialWorker) << QDateTime::currentDateTime().toString() << "sent" << bytesWritten << "of data";
	}
}


void SerialPortWorker::onStartup() {
	m_serialPort = new QSerialPort(this);
	// Connect serial port signals
	connect(m_serialPort, SIGNAL(readyRead()),
			this, SLOT(onReadyRead()), Qt::DirectConnection);
	connect(m_serialPort, QOverload<QSerialPort::SerialPortError>::of(&QSerialPort::errorOccurred),
			this, &SerialPortWorker::onErrorOccurred, Qt::DirectConnection);
	qCDebug(lcSerialWorker) << "Startup complete";
}


void SerialPortWorker::onReadyRead() {
	QByteArray data = m_serialPort->readAll();
	qCDebug(lcSerialWorker) << data;

	emit dataReceived(data);
}


void SerialPortWorker::onErrorOccurred(QSerialPort::SerialPortError error) {
	// this function is also called when the port is closed
	if (error != QSerialPort::NoError) {
		QString errorMsg = m_serialPort->errorString();
		qCCritical(lcSerialWorker) << error << ":" << errorMsg;
		emit errorOccurred(errorMsg);

		// Close on any error except these transient ones
		if (error != QSerialPort::TimeoutError && error != QSerialPort::NotOpenError) {
			if (m_serialPort->isOpen()) {
				m_serialPort->close();
				emit connectionStatusChanged(false);
			}
		}
	}
}


} // namespace SP


