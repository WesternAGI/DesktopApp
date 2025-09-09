#pragma once

#include <QObject>
#include <QString>
#include <QAudioSource>
#include <QAudioFormat>
#include <QIODevice>
#include <QTimer>
#include <QFile>
#include <QMediaDevices>
#include <memory>

namespace GadAI {

/**
 * @brief Audio recording service for voice input
 */
class AudioRecorder : public QObject
{
    Q_OBJECT

public:
    enum State {
        Stopped,
        Recording,
        Paused
    };

    explicit AudioRecorder(QObject *parent = nullptr);
    ~AudioRecorder();

    /**
     * @brief Check if audio recording is available
     */
    bool isAvailable() const;

    /**
     * @brief Start recording audio
     * @param outputPath Path where to save the recording
     * @return true if recording started successfully
     */
    bool startRecording(const QString &outputPath);

    /**
     * @brief Stop recording audio
     * @return true if recording stopped successfully
     */
    bool stopRecording();

    /**
     * @brief Pause recording
     */
    void pauseRecording();

    /**
     * @brief Resume recording
     */
    void resumeRecording();

    /**
     * @brief Get current recording state
     */
    State state() const { return m_state; }

    /**
     * @brief Get recording duration in milliseconds
     */
    qint64 recordingDuration() const;

    /**
     * @brief Get supported audio formats
     */
    static QStringList getSupportedFormats();

signals:
    void recordingStarted();
    void recordingStopped();
    void recordingPaused();
    void recordingResumed();
    void recordingError(const QString &error);
    void durationChanged(qint64 duration);
    void deviceChanged(const QString &description);

private slots:
    void updateDuration();

private:
    void setupAudioFormat();
    bool setupAudioInput();
    void writeWavHeader(quint32 dataLength);
    void handleDeviceChanged();

    State m_state;
    QString m_outputPath;
    QAudioFormat m_audioFormat;
    std::unique_ptr<QAudioSource> m_audioInput; // Placeholder for future implementation
    QIODevice *m_audioDevice;
    QTimer *m_durationTimer;
    qint64 m_startTime;
    qint64 m_pausedDuration;
    QFile m_outputFile;
    qint64 m_bytesRecorded = 0;
    bool m_deviceChangedPending = false;
    QMediaDevices m_mediaDevices; // monitors device changes (Qt6)
};

} // namespace GadAI
