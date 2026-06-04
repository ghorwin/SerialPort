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

#include "SP_SerialPortCommunicator.h"

#include <QDebug>
#include <QDateTime>
#include <QMutexLocker>
#include <QTimer>

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(lcSerial, "serial.comm") // logging category for everything related to serial communication

#include "SP_SerialPortWorker.h"

namespace SP {


// ============================================================================
// SerialPortCommunicator Implementation
// ============================================================================

SerialPortCommunicator::SerialPortCommunicator(QObject *parent, SerialPortWorker * ownWorker)
	: QObject(parent), m_workerThread(nullptr), m_worker(nullptr), m_isOpen(false)
{
	// Create worker thread
	m_workerThread = new QThread(this);
	if (ownWorker != nullptr)
		m_worker = ownWorker;
	else
		m_worker = new SerialPortWorker();

	// Move worker to thread
	m_worker->moveToThread(m_workerThread);

	// CAUTION: we plan on working with derived worker classes and hence we cannot use the pointer-based
	//          connect statements. Instead, we use the SIGNAL() and SLOT() syntax, as this allows
	//          connections to overridden members in child classes.

	// Connect signals for thread communication
	connect(this, SIGNAL(openPort(QString,int,int,int,int)),	m_worker, SLOT(onOpenPort(QString,int,int,int,int)) );
	connect(this, SIGNAL(closePort()),							m_worker, SLOT(onClosePort()) );
	connect(this, SIGNAL(writeData(QByteArray)),				m_worker, SLOT(onWriteData(QByteArray)) );

	// Connect worker signals to our signals (just forwarded)
	connect(m_worker, SIGNAL(dataReceived(QByteArray)),			this, SIGNAL(dataReceived(QByteArray)) );
	connect(m_worker, SIGNAL(errorOccurred(QString)),			this, SIGNAL(errorOccurred(QString)) );

	// Connect worker signals to our slots
	connect(m_worker, SIGNAL(connectionStatusChanged(bool)),	this, SLOT(onWorkerConnectionStatusChanged(bool)) );

	// Connect startup signal
	connect(m_workerThread, SIGNAL(started()),					m_worker, SLOT(onStartup()));

	// Handle thread cleanup
	// Worker thread continues to live even if connection is broken. Worker thread is finished
	// only by destructor and then cleans up memory.
	connect(m_workerThread, &QThread::finished,					m_worker, &QObject::deleteLater);

	// Start the worker thread
	m_workerThread->start();
}



SerialPortCommunicator::~SerialPortCommunicator() {
	if (m_workerThread) {
		// Request thread to quit and wait for it to finish
		m_workerThread->quit();
		if (!m_workerThread->wait(3000)) {
			qCWarning(lcSerial) << "Worker thread did not finish within timeout, terminating...";
			m_workerThread->terminate();
			m_workerThread->wait(1000);
		}
	}
	qCDebug(lcSerial) << "Destructor called";
}


void SerialPortCommunicator::open(const QString & portName, QSerialPort::BaudRate b,
								  QSerialPort::DataBits d, QSerialPort::Parity p,
								  QSerialPort::StopBits s)
{
	if (m_isOpen) {
		qCWarning(lcSerial) << "Serial port already/still open. Close first!";
		return;
	}

	// Send request to worker thread
	emit openPort(portName, b, d, p, s);
}


void SerialPortCommunicator::close() {
	emit closePort();
}


void SerialPortCommunicator::write(const QByteArray & binaryDataBlock) {
	if (!m_isOpen) {
		qCWarning(lcSerial) << "Attempting to write to closed port";
		return;
	}

	emit writeData(binaryDataBlock);
}


void SerialPortCommunicator::onWorkerConnectionStatusChanged(bool connected) {
	m_isOpen = connected;
	emit connectionStatusChanged(connected);
}


} // namespace SP
