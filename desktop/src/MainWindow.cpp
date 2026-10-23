#include "MainWindow.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QSettings>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

#ifdef Q_OS_WIN
#define NOMINMAX
#include <windows.h>

namespace {
MainWindow *g_openBandWindow = nullptr;
HHOOK g_keyboardHook = nullptr;

LRESULT CALLBACK openBandKeyboardHook(int code, WPARAM message, LPARAM data)
{
    if (code == HC_ACTION && g_openBandWindow) {
        const auto *key = reinterpret_cast<KBDLLHOOKSTRUCT *>(data);
        if (key->vkCode == VK_RCONTROL && !(key->flags & LLKHF_INJECTED)) {
            if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN)
                QMetaObject::invokeMethod(g_openBandWindow, "globalPttDown", Qt::QueuedConnection);
            else if (message == WM_KEYUP || message == WM_SYSKEYUP)
                QMetaObject::invokeMethod(g_openBandWindow, "globalPttUp", Qt::QueuedConnection);
        }
    }
    return CallNextHookEx(g_keyboardHook, code, message, data);
}
}
#endif

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setWindowTitle("OpenBand");
    resize(520, 430);
    buildUi();
    loadPresets();
    registerHotkeys();
#ifdef Q_OS_WIN
    g_openBandWindow = this;
    g_keyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, openBandKeyboardHook, GetModuleHandleW(nullptr), 0);
    if (!g_keyboardHook) statusLabel_->setText("F5 is enabled, but global PTT hook could not be installed.");
#endif

    QSettings settings;
    callsignEdit_->setText(settings.value("callsign", "Operator").toString());
    volumeSlider_->setValue(settings.value("volume", 75).toInt());
    squelchSlider_->setValue(settings.value("squelch", 20).toInt());
    alwaysOnTop_->setChecked(settings.value("alwaysOnTop", false).toBool());
}

MainWindow::~MainWindow()
{
#ifdef Q_OS_WIN
    if (g_keyboardHook) UnhookWindowsHookEx(g_keyboardHook);
    g_keyboardHook = nullptr;
    g_openBandWindow = nullptr;
    UnregisterHotKey(reinterpret_cast<HWND>(winId()), 1);
#endif
}

void MainWindow::buildUi()
{
    auto *root = new QWidget(this);
    auto *layout = new QVBoxLayout(root);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(12);

    auto *heading = new QLabel("OPENBAND  /  VIRTUAL RADIO", root);
    QFont headingFont = heading->font();
    headingFont.setBold(true);
    headingFont.setPointSize(15);
    heading->setFont(headingFont);
    layout->addWidget(heading);

    channelLabel_ = new QLabel("NO CHANNEL SELECTED", root);
    channelLabel_->setStyleSheet("QLabel { background:#20252b; color:#e8edf2; padding:14px; font-size:18px; font-weight:600; border-radius:5px; }");
    layout->addWidget(channelLabel_);

    auto *form = new QFormLayout();
    callsignEdit_ = new QLineEdit(root);
    callsignEdit_->setPlaceholderText("Callsign");
    channelBox_ = new QComboBox(root);
    modeBox_ = new QComboBox(root);
    modeBox_->addItems({"Voice", "Data / text"});
    form->addRow("Callsign", callsignEdit_);
    form->addRow("Channel preset", channelBox_);
    form->addRow("Mode", modeBox_);
    layout->addLayout(form);

    auto *levels = new QFormLayout();
    volumeSlider_ = new QSlider(Qt::Horizontal, root);
    volumeSlider_->setRange(0, 100);
    squelchSlider_ = new QSlider(Qt::Horizontal, root);
    squelchSlider_->setRange(0, 100);
    levels->addRow("Receive volume", volumeSlider_);
    levels->addRow("Squelch", squelchSlider_);
    layout->addLayout(levels);

    pttButton_ = new QPushButton("HOLD TO TRANSMIT  ·  RIGHT CTRL", root);
    pttButton_->setMinimumHeight(48);
    pttButton_->setStyleSheet("QPushButton { background:#2c5d4b; color:white; font-weight:700; border:0; border-radius:5px; } QPushButton:pressed { background:#3c8066; }");
    layout->addWidget(pttButton_);

    auto *bottom = new QHBoxLayout();
    alwaysOnTop_ = new QCheckBox("Always on top", root);
    auto *saveButton = new QPushButton("Save settings", root);
    bottom->addWidget(alwaysOnTop_);
    bottom->addStretch();
    bottom->addWidget(saveButton);
    layout->addLayout(bottom);

    statusLabel_ = new QLabel("Offline · connect a Worker URL in Settings (network client pending)", root);
    statusLabel_->setWordWrap(true);
    statusLabel_->setStyleSheet("color:#8a929b;");
    layout->addWidget(statusLabel_);
    setCentralWidget(root);

    connect(channelBox_, &QComboBox::currentIndexChanged, this, &MainWindow::updateChannelLabel);
    connect(pttButton_, &QPushButton::pressed, this, [this] { setPtt(true); });
    connect(pttButton_, &QPushButton::released, this, [this] { setPtt(false); });
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::saveSettings);
    connect(alwaysOnTop_, &QCheckBox::toggled, this, [this](bool enabled) {
        setWindowFlag(Qt::WindowStaysOnTopHint, enabled);
        show();
    });
}

void MainWindow::loadPresets()
{
    QFile file("config/channel-presets.json");
    if (!file.open(QIODevice::ReadOnly)) {
        file.setFileName("../shared/channel-presets.json");
        if (!file.open(QIODevice::ReadOnly)) {
            channelBox_->addItem("General");
            return;
        }
    }
    const auto doc = QJsonDocument::fromJson(file.readAll());
    presets_ = doc.object().value("channels").toArray();
    for (const auto &entry : presets_) {
        const auto item = entry.toObject();
        channelBox_->addItem(item.value("name").toString("Unnamed channel"));
    }
    if (channelBox_->count() == 0) channelBox_->addItem("General");
    updateChannelLabel();
}

void MainWindow::registerHotkeys()
{
#ifdef Q_OS_WIN
    RegisterHotKey(reinterpret_cast<HWND>(winId()), 1, MOD_NOREPEAT, static_cast<UINT>(overlayHotkey_));
    // Registering Right Ctrl as a global PTT key can conflict with applications.
    // It is intentionally handled as a configurable future enhancement; holding the
    // in-window PTT button is supported now.
#else
    statusLabel_->setText("Global hotkeys depend on your desktop session; Wayland may restrict them.");
#endif
}

#ifdef Q_OS_WIN
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(eventType);
    auto *msg = static_cast<MSG *>(message);
    if (msg->message == WM_HOTKEY && msg->wParam == 1) {
        toggleOverlay();
        if (result) *result = 0;
        return true;
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}
#endif

void MainWindow::toggleOverlay()
{
    compactOverlay_ = !compactOverlay_;
    if (compactOverlay_) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::Tool);
        resize(340, 230);
        show();
        raise();
        activateWindow();
    } else {
        setWindowFlags(windowFlags() & ~Qt::Tool);
        resize(520, 430);
        show();
    }
}

void MainWindow::globalPttDown()
{
    setPtt(true);
}

void MainWindow::globalPttUp()
{
    setPtt(false);
}

void MainWindow::togglePtt()
{
    setPtt(!pttActive_);
}

void MainWindow::setPtt(bool pressed)
{
    pttActive_ = pressed;
    pttButton_->setText(pressed ? "TRANSMIT REQUESTED · RELEASE TO STOP" : "HOLD TO TRANSMIT  ·  RIGHT CTRL");
    statusLabel_->setText(pressed
        ? "PTT active locally. Audio transport is not implemented yet."
        : "Offline · no audio transmission is occurring.");
}

void MainWindow::updateChannelLabel()
{
    const int index = channelBox_->currentIndex();
    if (index >= 0 && index < presets_.size()) {
        const auto item = presets_.at(index).toObject();
        channelLabel_->setText(QString("%1  ·  %2")
            .arg(item.value("name").toString("CHANNEL"))
            .arg(item.value("label").toString("VIRTUAL")));
    } else {
        channelLabel_->setText(channelBox_->currentText().toUpper());
    }
}

void MainWindow::saveSettings()
{
    QSettings settings;
    settings.setValue("callsign", callsignEdit_->text().trimmed());
    settings.setValue("volume", volumeSlider_->value());
    settings.setValue("squelch", squelchSlider_->value());
    settings.setValue("alwaysOnTop", alwaysOnTop_->isChecked());
    settings.sync();
    statusLabel_->setText("Settings saved locally.");
}

