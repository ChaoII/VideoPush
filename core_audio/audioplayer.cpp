#include "audioplayer.h"
#include "audiohelper.h"
#include "qthread.h"
#include "qvariant.h"
#include "qdebug.h"

AudioPlayer::AudioPlayer(QObject *parent) : QObject(parent) {
    volume = 100;
    muted = false;
    level = false;
    sampleSize = 16;

    audioThread = nullptr;
    audioInput = nullptr;
    deviceInput = nullptr;
    audioOutput = nullptr;
    deviceOutput = nullptr;

    sonicSize = 9600;
    sonicData = nullptr;
    sonicObj = nullptr;

    //注册数据类型
    qRegisterMetaType<const char *>("const char *");
}

AudioPlayer::~AudioPlayer() {
    this->closeAudioInput();
    this->closeAudioOutput();
    this->closeSonic();
}

void AudioPlayer::open() {
    //移到线程执行防止界面卡住导致声音卡住
    if (!audioThread) {
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        audioThread = new QThread;
        //线程结束的时候销毁当前类
        connect(audioThread, SIGNAL(finished()), this, SLOT(deleteLater()));
        this->moveToThread(audioThread);
        audioThread->start();
#endif
    }
}

void AudioPlayer::close() {
    if (audioThread) {
        audioThread->quit();
        audioThread = nullptr;
    } else {
        this->deleteLater();
    }
}

QAudioInputx *AudioPlayer::getAudioInput() {
    return this->audioInput;
}

QAudioOutputx *AudioPlayer::getAudioOutput() {
    return this->audioOutput;
}

bool AudioPlayer::getIsOk() const {
    bool isOk = false;
#ifdef multimedia
    if (audioOutput) {
        isOk = (audioOutput->state() != QAudio::StoppedState);
    }
#endif
    return isOk;
}

int AudioPlayer::getVolume() const {
    qreal value = 1.0;
#ifdef multimedia5
    if (audioOutput) {
        value = audioOutput->volume();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
        value = QAudio::convertVolume(value, QAudio::LinearVolumeScale, QAudio::LogarithmicVolumeScale);
#endif
    }
#endif
    return value * 100;
}

void AudioPlayer::setVolume(int volume) {
    //Qt5.8版本以下在静音状态下不能设置音量大小卧槽
    this->volume = volume;
    emit receiveVolume(volume);
#ifdef multimedia5
    if (audioOutput) {
        qreal value = (qreal) volume / 100;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))
        value = QAudio::convertVolume(value, QAudio::LogarithmicVolumeScale, QAudio::LinearVolumeScale);
#endif
        audioOutput->setVolume(value);
    }
#endif
}

bool AudioPlayer::getMuted() const {
    return this->muted;
    //return (getVolume() == 0);
}

void AudioPlayer::setMuted(bool muted) {
    //Qt6中的QAudioSink音频类用挂起和复位失效(需要用静音标志位来控制是否写入数据实现静音)
    this->muted = muted;
    emit receiveMuted(muted);
    //发现用标志位这个万能办法最完美无需继续
    return;

#ifdef multimedia
    //本身没有提供静音函数接口(采用挂起和复位来实现静音切换)
    if (muted) {
        audioOutput->suspend();
    } else {
        audioOutput->resume();
    }
#endif
}

bool AudioPlayer::getLevel() const {
    return this->level;
}

void AudioPlayer::setLevel(bool level) {
    this->level = level;
}

void AudioPlayer::readyRead() {
#ifdef multimedia
    //将音频输入设备数据发给音频输出设备播放
    if (deviceInput && deviceOutput) {
        QByteArray data = deviceInput->readAll();
        emit receiveInputData(data);
        playAudioData(data.constData(), data.length());
    }
#endif
}

void AudioPlayer::openAudioInput(int sampleRate, int channelCount, int sampleSize) {
    this->openAudioInput("", sampleRate, channelCount, sampleSize);
}

void AudioPlayer::openAudioInput(const QString &deviceName, int sampleRate, int channelCount, int sampleSize) {
#ifdef multimedia
    //先关闭设备
    this->closeAudioInput();
    //初始化音频格式
    QAudioFormat format;
    AudioHelper::initAudioFormat(format, sampleRate, channelCount, sampleSize);

    //找到指定设备并实例化音频类(为空则采用默认设备)
    if (deviceName.isEmpty()) {
        audioInput = new QAudioInputx(format, this);
    } else {
        QAudioDevice device = AudioHelper::getAudioDevice(deviceName, true);
        audioInput = new QAudioInputx(device, format, this);
    }

    //启动音频服务
    deviceInput = audioInput->start();
    //将音频输入设备的数据读取出来
    if (deviceInput) {
        connect(deviceInput, SIGNAL(readyRead()), this, SLOT(readyRead()));
    }
#endif
}

void AudioPlayer::closeAudioInput() {
#ifdef multimedia
    if (audioInput) {
        audioInput->reset();
        audioInput->stop();
        audioInput->deleteLater();
        audioInput = NULL;
    }
#endif
}

void AudioPlayer::openAudioOutput(int sampleRate, int channelCount, int sampleSize) {
    this->openAudioOutput("", sampleRate, channelCount, sampleSize);
}

void AudioPlayer::openAudioOutput(const QString &deviceName, int sampleRate, int channelCount, int sampleSize) {
#ifdef multimedia
    //先关闭设备
    this->closeAudioOutput();
    //初始化音频格式
    QAudioFormat format;
    AudioHelper::initAudioFormat(format, sampleRate, channelCount, sampleSize);
    this->sampleSize = sampleSize;

    //找到指定设备并实例化音频类(为空则采用默认设备)
    if (deviceName.isEmpty()) {
        audioOutput = new QAudioOutputx(format, this);
    } else {
        QAudioDevice device = AudioHelper::getAudioDevice(deviceName, false);
        audioOutput = new QAudioOutputx(device, format, this);
    }

    //启动音频服务
    deviceOutput = audioOutput->start();
    //重新设置音量大小
    this->setVolume(volume);
#endif
}

void AudioPlayer::closeAudioOutput() {
#ifdef multimedia
    if (audioOutput) {
        audioOutput->reset();
        audioOutput->stop();
        audioOutput->deleteLater();
        audioOutput = NULL;
    }
#endif
}

void AudioPlayer::openSonic(bool useSonic, int sampleRate, int channelCount, float speed, float pitch, float rate,
                            float volume) {
    this->closeSonic();
    if (useSonic) {
        //开辟临时存放运算后的音频数据内存空间
        sonicData = new float[sonicSize];
        //创建变速运算流
        sonicObj = SonicCpp::sonicCreateStream(sampleRate, channelCount);
        //设置音速
        SonicCpp::sonicSetSpeed(sonicObj, speed);
        //设置音调
        SonicCpp::sonicSetPitch(sonicObj, pitch);
        //设置语速
        SonicCpp::sonicSetRate(sonicObj, rate);
        //声音大小/可以通过改变声音数据强制提高音量
        SonicCpp::sonicSetVolume(sonicObj, volume);
    }
}

void AudioPlayer::closeSonic() {
    //释放变声库临时数据
    if (sonicData) {
        delete[] sonicData;
        sonicData = NULL;
    }

    //释放变声库对象
    if (sonicObj) {
        SonicCpp::sonicDestroyStream(sonicObj);
        sonicObj = NULL;
    }
}

void AudioPlayer::playSonicData(const char *data, qint64 len) {
    //为什么是这个尺寸 https://blog.csdn.net/qq_40170041/article/details/127727153
    //写入的时候用真实的数据长度除以这个值/取出的时候用运算后的真实数据长度乘以这个值
    //先将音频数据写入流/然后从流中取出运算后的数据
    int offset, numSamples;
    if (sampleSize == 8) {
        offset = SonicCpp::sonicGetNumChannels(sonicObj) * 1;
        SonicCpp::sonicWriteUnsignedCharToStream(sonicObj, (uchar *) data, len / offset);
        numSamples = SonicCpp::sonicReadUnsignedCharFromStream(sonicObj, (uchar *) sonicData, sonicSize);
    } else if (sampleSize == 16) {
        offset = SonicCpp::sonicGetNumChannels(sonicObj) * 2;
        SonicCpp::sonicWriteShortToStream(sonicObj, (short *) data, len / offset);
        numSamples = SonicCpp::sonicReadShortFromStream(sonicObj, (short *) sonicData, sonicSize);
    } else {
        offset = SonicCpp::sonicGetNumChannels(sonicObj) * 4;
        SonicCpp::sonicWriteFloatToStream(sonicObj, (float *) data, len / offset);
        numSamples = SonicCpp::sonicReadFloatFromStream(sonicObj, (float *) sonicData, sonicSize);
    }

    //写入运算后的音频数据
    deviceOutput->write((const char *) sonicData, numSamples * offset);
}

void AudioPlayer::playAudioData(const char *data, qint64 len) {
#ifdef multimedia
    if (deviceOutput) {
        //发送收到音频播放数据信号
        emit receiveOutputData(data, len);
        //限定静音状态下不写入数据减轻压力
        if (!muted) {
            if (sonicObj) {
                this->playSonicData(data, len);
            } else {
                deviceOutput->write(data, len);
            }
        }

        //获取音频数据振幅
        if (level) {
            qreal leftLevel, rightLevel;
            AudioHelper::getAudioLevel(audioOutput->format(), data, len, leftLevel, rightLevel);
            emit receiveLevel(leftLevel, rightLevel);
        }
    }
#endif
}
