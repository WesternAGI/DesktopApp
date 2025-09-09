#include "AudioRecorder.h"
#include <QMediaDevices>
#include <QAudioDevice>
#include <QAudioSource>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMediaDevices>

namespace GadAI {

AudioRecorder::AudioRecorder(QObject *parent)
    : QObject(parent)
    , m_state(Stopped)
    , m_audioDevice(nullptr)
    , m_startTime(0)
    , m_pausedDuration(0)
{
    setupAudioFormat();
    
    m_durationTimer = new QTimer(this);
    m_durationTimer->setInterval(100); // Update every 100ms
    connect(m_durationTimer, &QTimer::timeout, this, &AudioRecorder::updateDuration);
    // Device change monitoring (Qt6 QMediaDevices emits signals on changes)
    connect(&m_mediaDevices, &QMediaDevices::audioInputsChanged, this, &AudioRecorder::handleDeviceChanged);
}

AudioRecorder::~AudioRecorder()
{
    if (m_state != Stopped) {
        stopRecording();
    }
}

bool AudioRecorder::isAvailable() const
{
    // Qt6: Use QMediaDevices to query default audio input
    QAudioDevice device = QMediaDevices::defaultAudioInput();
    return device.isFormatSupported(m_audioFormat);
}

bool AudioRecorder::startRecording(const QString &outputPath)
{
    if (m_state != Stopped) {
        qWarning() << "Recording already in progress";
        return false;
    }
    
    if (!isAvailable()) {
        emit recordingError("No audio input device available");
        return false;
    }
    
    m_outputPath = outputPath;
    
    if (!setupAudioInput()) {
        emit recordingError("Failed to setup audio input");
        return false;
    }
    
    // Open output file
    m_outputFile.setFileName(m_outputPath);
    if (!m_outputFile.open(QIODevice::WriteOnly)) {
        emit recordingError("Cannot create output file");
        return false;
    }

    // Write temporary WAV header placeholder (44 bytes)
    writeWavHeader(0);
    
    m_state = Recording;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_pausedDuration = 0;
    m_bytesRecorded = 0;
    
    m_durationTimer->start();
    emit recordingStarted();
    
    qDebug() << "Audio recording started:" << outputPath;
    return true;
}

bool AudioRecorder::stopRecording()
{
    if (m_state == Stopped) {
        return true;
    }
    
    m_durationTimer->stop();
    
    if (m_audioInput) {
        m_audioInput->stop();
        m_audioDevice = nullptr;
        m_audioInput.reset();
    }

    // Fix header with final data size
    if (m_outputFile.isOpen()) {
        qint64 dataSize = m_outputFile.size() - 44;
        m_outputFile.seek(0);
        writeWavHeader(static_cast<quint32>(dataSize));
        m_outputFile.close();
    }
    
    m_audioDevice = nullptr;
    m_state = Stopped;
    
    emit recordingStopped();
    qDebug() << "Audio recording stopped";
    
    return true;
}

void AudioRecorder::pauseRecording()
{
    if (m_state != Recording) {
        return;
    }
    
    m_pausedDuration += QDateTime::currentMSecsSinceEpoch() - m_startTime;
    m_durationTimer->stop();
    
    if (m_audioInput) {
        m_audioInput->suspend();
    }
    
    m_state = Paused;
    emit recordingPaused();
}

void AudioRecorder::resumeRecording()
{
    if (m_state != Paused) {
        return;
    }
    
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    m_durationTimer->start();
    
    if (m_audioInput) {
        m_audioInput->resume();
    }
    
    m_state = Recording;
    emit recordingResumed();
}

qint64 AudioRecorder::recordingDuration() const
{
    if (m_state == Stopped) {
        return 0;
    }
    
    qint64 currentDuration = m_pausedDuration;
    if (m_state == Recording) {
        currentDuration += QDateTime::currentMSecsSinceEpoch() - m_startTime;
    }
    
    return currentDuration;
}

QStringList AudioRecorder::getSupportedFormats()
{
    return {"wav", "mp3", "m4a", "ogg"};
}

void AudioRecorder::updateDuration()
{
    emit durationChanged(recordingDuration());
}

void AudioRecorder::setupAudioFormat()
{
    m_audioFormat.setSampleRate(44100);
    m_audioFormat.setChannelCount(1); // Mono
    m_audioFormat.setSampleFormat(QAudioFormat::Int16);
    // 16-bit PCM mono 44.1kHz
}

bool AudioRecorder::setupAudioInput()
{
    QAudioDevice device = QMediaDevices::defaultAudioInput();
    if (!device.isFormatSupported(m_audioFormat)) {
        m_audioFormat = device.preferredFormat();
    }
    m_audioInput = std::make_unique<QAudioSource>(device, m_audioFormat, this);
    m_audioDevice = m_audioInput->start();
    if (!m_audioDevice) {
        qWarning() << "Failed to start audio input";
        return false;
    }
    connect(m_audioDevice, &QIODevice::readyRead, this, [this]() {
        QByteArray data = m_audioDevice->readAll();
        if (m_outputFile.isOpen()) {
            m_outputFile.write(data);
            m_bytesRecorded += data.size();
        }
    });
    return true;
}

void AudioRecorder::writeWavHeader(quint32 dataLength)
{
    // Basic PCM WAV header
    quint16 audioFormat = 1; // PCM
    quint16 numChannels = m_audioFormat.channelCount();
    quint32 sampleRate = m_audioFormat.sampleRate();
    quint16 bitsPerSample = 16; // Int16
    quint32 byteRate = sampleRate * numChannels * bitsPerSample / 8;
    quint16 blockAlign = numChannels * bitsPerSample / 8;
    quint32 chunkSize = 36 + dataLength;

    QDataStream out(&m_outputFile);
    out.setByteOrder(QDataStream::LittleEndian);
    out.writeRawData("RIFF", 4);
    out << chunkSize;
    out.writeRawData("WAVE", 4);
    out.writeRawData("fmt ", 4);
    out << quint32(16); // Subchunk1Size for PCM
    out << audioFormat;
    out << numChannels;
    out << sampleRate;
    out << byteRate;
    out << blockAlign;
    out << bitsPerSample;
    out.writeRawData("data", 4);
    out << dataLength;
}

void AudioRecorder::handleDeviceChanged()
{
    if (m_state == Recording) {
        if (m_deviceChangedPending) return; // debounce rapid changes
        m_deviceChangedPending = true;
        QTimer::singleShot(200, this, [this]() {
            m_deviceChangedPending = false;
            qWarning() << "Audio input devices changed during recording; attempting seamless switch";
            emit deviceChanged("Audio input devices changed; switching to new default");
            if (m_audioInput) {
                m_audioInput->stop();
                m_audioDevice = nullptr;
                m_audioInput.reset();
            }
            if (!setupAudioInput()) {
                emit recordingError("Audio device changed and reinitialization failed");
                stopRecording();
            }
        });
    } else if (m_state == Paused) {
        // Reinitialize in paused state so resume works with new device
        if (m_audioInput) {
            m_audioInput->stop();
            m_audioDevice = nullptr;
            m_audioInput.reset();
        }
        setupAudioInput();
    emit deviceChanged("Audio input device changed while paused; ready to resume");
    }
}

} // namespace GadAI
