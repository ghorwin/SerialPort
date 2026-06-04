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

#ifndef SP_SERIALPORTCOMMUNICATOR_H
#define SP_SERIALPORTCOMMUNICATOR_H

#include <QObject>
#include <QThread>
#include <QSerialPort>
#include <QMutex>

#include "SP_global.h"

namespace SP {

// Forward declarations
class SerialPortWorker;

/*! Wraps the actual communication thread.
	The public member functions open(), close(), write() are to be called by the users' thread,
	typically GUI thread. These functions return right away.

	Callers should connect to signals:
	- connectionStatusChanged()
	- dataReceived()
	- errorOccurred()

	For time-critical communications, re-implement the SerialPortWorker class and provide it in the constructor
	as your own worker implementation (will become owned by SerialPortCommunicator and destroyed).
*/
class SP_EXPORT SerialPortCommunicator : public QObject {
	Q_OBJECT
public:
	SerialPortCommunicator(QObject *parent = nullptr, SerialPortWorker * ownWorker = nullptr);
	~SerialPortCommunicator();

	/*! Attempts to open the communication (emits openPort()).
		\warning Do not call if port is open.
	*/
	virtual void open(const QString & portName, QSerialPort::BaudRate b,
			  QSerialPort::DataBits d, QSerialPort::Parity p, QSerialPort::StopBits s);

	/*! Close connection (emits closePort()). */
	void close();

	/*! Sends binary package to com port (emits writeData()). */
	void write(const QByteArray & binaryDataBlock);

	/*! Returns true, if the serial port connection is active. */
	bool isOpen() const { return m_isOpen; }

signals:
	/*! Emitted, when a data package was received. */
	void dataReceived(QByteArray);
	/*! Emitted when an error occured within communication worker. */
	void errorOccurred(QString);
	/*! Emitted, when communication states has changed. */
	void connectionStatusChanged(bool connected);

	// Internal signals for thread communication

	/*! Sent to worker thread when it should open a connection to QSerialPort. */
	void openPort(QString portName, int baudRate, int dataBits, int parity, int stopBits);
	/*! Sent to worker thread when it should close the QSerialPort connection. */
	void closePort();
	/*! Sent to worker thread when it should write data. */
	void writeData(QByteArray data);

private slots:
	/*! Connected to SerialPortWorker::connectionStatusChanged(), updates m_isOpen  */
	void onWorkerConnectionStatusChanged(bool connected);

private:
	/*! The actual worker thread (with own event loop) and the actual worker class that does all the work
		within the thread's event loop.
	*/
	QThread				*m_workerThread;
	SerialPortWorker	*m_worker;

	/*! Changed in onWorkerConnectionStatusChanged(). */
	bool				m_isOpen = false;
};


} // namespace SP

#endif // SP_SERIALPORTCOMMUNICATOR_H
