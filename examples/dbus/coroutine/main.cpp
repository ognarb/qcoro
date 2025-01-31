// SPDX-FileCopyrightText: 2021 Daniel Vrátil <dvratil@kde.org>
//
// SPDX-License-Identifier: MIT

#include "qcoro/coroutine.h"
#include "qcoro/dbus.h"
#include "qcoro/task.h"

#include "common/dbusserver.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QDateTime>
#include <QDebug>
#include <QTimer>

#include <chrono>
#include <iostream>
#include <thread>
#include <variant>

using namespace std::chrono_literals;

QCoro::Task<> dbusWorker() {
    auto bus = QDBusConnection::sessionBus();
    auto iface = QDBusInterface{DBusServer::serviceName, DBusServer::objectPath,
                                DBusServer::interfaceName, bus};
    qInfo() << "Sending PING";
    QDBusReply<QString> response = co_await iface.asyncCall(QStringLiteral("blockingPing"), 1);
    if (const auto &err = response.error(); err.isValid()) {
        qWarning() << "DBus call failed:" << err.message();
    }
    qInfo() << "Received response:" << response.value();
}

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    auto process = DBusServer::runStadaloneServer();

    QTimer tickTimer;
    QObject::connect(&tickTimer, &QTimer::timeout, &app, []() {
        std::cout << QDateTime::currentDateTime().toString(Qt::ISODateWithMs).toStdString()
                  << " Tick!" << std::endl;
    });
    tickTimer.start(400ms);

    QTimer dbusTimer;
    QObject::connect(&dbusTimer, &QTimer::timeout, &app, dbusWorker);
    dbusTimer.start(2s);

    return app.exec();
}
