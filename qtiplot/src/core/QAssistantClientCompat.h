#ifndef QASSISTANTCLIENTCOMPAT_H
#define QASSISTANTCLIENTCOMPAT_H

#include <QObject>
#include <QProcess>
#include <QStringList>

class QAssistantClient : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool open READ isOpen)

public:
	QAssistantClient(const QString &path, QObject *parent = 0);
	~QAssistantClient();

	bool isOpen() const;
	void setArguments(const QStringList &args);

public slots:
	void openAssistant();
	void closeAssistant();
	void showPage(const QString &page);

signals:
	void assistantOpened();
	void assistantClosed();
	void error(const QString &msg);

private slots:
	void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void processError(QProcess::ProcessError processError);

private:
	QString assistantPath() const;
	QString candidatePath(const QString &binary) const;
	bool startAssistant(const QString &page);
	bool isAdpAssistant(const QString &path) const;
	bool sendRemoteCommand(const QString &page);

	QString d_path;
	QStringList d_arguments;
	QProcess *d_process;
};

#endif
