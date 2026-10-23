#pragma once

#include <QMainWindow>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QSystemTrayIcon>

class QComboBox;
class QLabel;
class QPushButton;
class QSlider;
class QLineEdit;
class QCheckBox;

#ifdef Q_OS_WIN
#include <QAbstractNativeEventFilter>
#endif

class MainWindow final : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
#ifdef Q_OS_WIN
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;
#endif

private slots:
    void toggleOverlay();
    void togglePtt();
    void updateChannelLabel();
    void saveSettings();
    void globalPttDown();
    void globalPttUp();

private:
    void buildUi();
    void loadPresets();
    void registerHotkeys();
    void setPtt(bool pressed);

    QComboBox *channelBox_{};
    QComboBox *modeBox_{};
    QLineEdit *callsignEdit_{};
    QLabel *statusLabel_{};
    QLabel *channelLabel_{};
    QPushButton *pttButton_{};
    QSlider *volumeSlider_{};
    QSlider *squelchSlider_{};
    QCheckBox *alwaysOnTop_{};
    QJsonArray presets_;
    QNetworkAccessManager network_;
    bool pttActive_{false};
    bool compactOverlay_{false};
    int overlayHotkey_{0x74}; // VK_F5
    int pttHotkey_{0xA3};     // VK_RCONTROL
};
