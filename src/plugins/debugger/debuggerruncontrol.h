/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#pragma once

#include "debugger_global.h"
#include "debuggerconstants.h"
#include "debuggerengine.h"

#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/devicesupport/deviceusedportsgatherer.h>

namespace Debugger {

class DebuggerStartParameters;

class DEBUGGER_EXPORT DebuggerRunTool : public ProjectExplorer::RunWorker
{
    Q_OBJECT

public:
    DebuggerRunTool(ProjectExplorer::RunControl *runControl); // Use.

    DebuggerRunTool(ProjectExplorer::RunControl *runControl,
                    const DebuggerStartParameters &sp); // Use rarely.
    DebuggerRunTool(ProjectExplorer::RunControl *runControl,
                    const Internal::DebuggerRunParameters &rp); // FIXME: Don't use.
    ~DebuggerRunTool();

    void setRunParameters(const Internal::DebuggerRunParameters &rp); // FIXME: Don't use.

    Internal::DebuggerEngine *engine() const { return m_engine; }
    Internal::DebuggerEngine *activeEngine() const;

    static DebuggerRunTool *createFromRunConfiguration(ProjectExplorer::RunConfiguration *runConfig);
    static DebuggerRunTool *createFromKit(ProjectExplorer::Kit *kit);
    void startRunControl();

    void showMessage(const QString &msg, int channel = LogDebug, int timeout = -1);

    void start() override;
    void stop() override;

    void startFailed();

    void notifyInferiorIll();
    Q_SLOT void notifyInferiorExited(); // Called from Android.
    void quitDebugger();
    void abortDebugger();
    void debuggingFinished();

    Internal::DebuggerRunParameters &runParameters();
    const Internal::DebuggerRunParameters &runParameters() const;

    void startDying() { m_isDying = true; }
    bool isDying() const { return m_isDying; }
    bool isCppDebugging() const { return m_isCppDebugging; }
    bool isQmlDebugging() const { return m_isQmlDebugging; }
    int portsUsedByDebugger() const;

    void setSolibSearchPath(const QStringList &list);
    void addSolibSearchDir(const QString &str);

    static void setBreakOnMainNextTime();

    void setInferior(const ProjectExplorer::Runnable &runnable);
    void setInferiorExecutable(const QString &executable);
    void setRunControlName(const QString &name);
    void appendInferiorCommandLineArgument(const QString &arg);
    void prependInferiorCommandLineArgument(const QString &arg);
    void addQmlServerInferiorCommandLineArgumentIfNeeded();

    void addExpectedSignal(const QString &signal);
    void addSearchDirectory(const QString &dir);

    void setStartMode(DebuggerStartMode startMode);
    void setCloseMode(DebuggerCloseMode closeMode);

    void setAttachPid(Utils::ProcessHandle pid);
    void setSysRoot(const QString &sysRoot);
    void setSymbolFile(const QString &symbolFile);
    void setRemoteChannel(const QString &channel);

    void setUseExtendedRemote(bool on);
    void setUseContinueInsteadOfRun(bool on);
    void setUseTargetAsync(bool on);
    void setContinueAfterAttach(bool on);
    void setSkipExecutableValidation(bool on);

    void setCommandsAfterConnect(const QString &commands);
    void setCommandsForReset(const QString &commands);

    void setQmlServer(const QUrl &qmlServer);

    void setIosPlatform(const QString &platform);
    void setDeviceSymbolsRoot(const QString &deviceSymbolsRoot);

    void setNeedFixup(bool on);

signals:
    void aboutToNotifyInferiorSetupOk();

private:
    void setupEngine();

    QPointer<Internal::DebuggerEngine> m_engine; // Master engine
    Internal::DebuggerRunParameters m_runParameters;
    QStringList m_errors;
    bool m_isDying = false;
    const bool m_isCppDebugging;
    const bool m_isQmlDebugging;
};

class DEBUGGER_EXPORT GdbServerPortsGatherer : public ProjectExplorer::RunWorker
{
    Q_OBJECT

public:
    explicit GdbServerPortsGatherer(ProjectExplorer::RunControl *runControl);
    ~GdbServerPortsGatherer();

    void setUseGdbServer(bool useIt) { m_useGdbServer = useIt; }
    bool useGdbServer() const { return m_useGdbServer; }
    Utils::Port gdbServerPort() const { return m_gdbServerPort; }
    QString gdbServerChannel() const;

    void setUseQmlServer(bool useIt) { m_useQmlServer = useIt; }
    bool useQmlServer() const { return m_useQmlServer; }
    Utils::Port qmlServerPort() const { return m_qmlServerPort; }
    QUrl qmlServer() const;

private:
    void start() override;
    void handlePortListReady();

    ProjectExplorer::DeviceUsedPortsGatherer m_portsGatherer;
    bool m_useGdbServer = false;
    bool m_useQmlServer = false;
    Utils::Port m_gdbServerPort;
    Utils::Port m_qmlServerPort;
};

class DEBUGGER_EXPORT GdbServerRunner : public ProjectExplorer::SimpleTargetRunner
{
    Q_OBJECT

public:
    explicit GdbServerRunner(ProjectExplorer::RunControl *runControl,
                             GdbServerPortsGatherer *portsGatherer);
    ~GdbServerRunner();

private:
    void start() override;

    GdbServerPortsGatherer *m_portsGatherer;
};

extern DEBUGGER_EXPORT const char GdbServerRunnerWorkerId[];
extern DEBUGGER_EXPORT const char GdbServerPortGathererWorkerId[];

} // namespace Debugger
