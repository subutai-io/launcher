import subutai
import hashlib
from time import sleep
import datetime
from subprocess import call
import zipfile


def subutaistart():

    sshlib = "ssh.zip"

    subutai.download(sshlib)
    while subutai.isDownloadComplete() != 1:
        sleep(0.05)

    tmpDir = subutai.GetTmpDir()
    installDir = subutai.GetInstallDir()

    zfl = zipfile.ZipFile(tmpDir+"/"+sshlib, 'r')
    zfl.extractall(installDir+"/bin")
    zfl.close()

    m = hashlib.md5()
    m.update(datetime.datetime.now().isoformat().encode('utf-8'))
    machineName = "subutai-w" + m.hexdigest()[:5]

    installDir = subutai.GetInstallDir()
    call([installDir+'/bin/ssh-keygen.exe', '-R', '[127.0.0.1]:4567'])

    subutai.SetSSHCredentials("subutai", "ubuntai", "127.0.0.1", 4567)

    if setupVm(machineName) != 0:
        subutai.RaiseError("Failed to install Virtual Machine. See the logs for details")
        subutai.Shutdown()
        return

    subutai.SetProgress(0.04)
    sleep(6)
    startVm(machineName)
    sleep(60)
    waitSSH()
    sleep(60)
    setupSSH()
    installSnapFromStore()
    subutai.SetProgress(0.10)
    sleep(60)
    initBtrfs()
    subutai.SetProgress(0.20)
    sleep(5)
    setAlias()
    subutai.SetProgress(0.30)
    sleep(10)
    installManagement()
    subutai.SetProgress(0.80)
    #subutai.AddStatus("Waiting for management container to download")
    #rc = waitManagementInstall()
    #if rc == 1:
    #    subutai.RaiseError("Failed to install management: Operating timed out")
    #    sleep(10)
    #    subutai.Shutdown()
    #    return

    subutai.SetProgress(0.42)
    sleep(30)
    stopVm(machineName)
    subutai.SetProgress(0.82)
    sleep(5)
    reconfigureNic(machineName)
    subutai.SetProgress(0.9)
    sleep(5)
    startVm(machineName)
    subutai.SetProgress(1.0)

    subutai.Shutdown()

    return 0


def waitSSH():
    subutai.log("info", "Waiting for machine to bring SSH")
    attempts = 0
    while subutai.TestSSH() != 0:
        sleep(1)
        attempts = attempts + 1
        if attempts == 30:
            subutai.log("error", "SSH timeout for 30 second")
            return
    subutai.log("info", "SSH Connected")
    return


def installManagement():
    ip = subutai.GetPeerIP()

    if ip == "":
        subutai.RaiseError("Failed to determine peer IP address")
        return

    ip = "127.0.0.1"

    subutai.AddStatus("Downloading Ubuntu")
    subutai.SSHRun("sudo subutai -d import ubuntu16 1>/tmp/ubuntu16-1.log 2>/tmp/ubuntu16-2.log")

    subutai.AddStatus("Downloading JVM")
    subutai.SSHRun("sudo subutai -d import openjre16 1>/tmp/openjre16-1.log 2>/tmp/openjre16-2.log")

    subutai.AddStatus("Installing Management Container")
    subutai.SSHRun("sudo subutai -d import management 1>/tmp/management-1.log 2>/tmp/management-2.log")

    attempts = 0
    while subutai.IsPeerReady(ip) != 0:
        sleep(2)
        attempts = attempts + 1
        if attempts >= 30:
            break

    return


def waitManagementInstall():
    rsize = subutai.GetRemoteTemplateSize("management-subutai-template_4.0.16-dev_amd64.tar.gz")
    dsize = subutai.GetPeerFileSize("/var/snap/subutai-dev/common/lxc/tmpdir/management-subutai-template_4.0.16-dev_amd64.tar.gz")

    timeout = datetime.datetime.now() + datetime.timedelta(0, 120)

    if rsize <= 0:
        return 1

    while rsize + 10 < dsize:
        sleep(0.1)
        percent = dsize / rsize * 100
        subutai.SetProgress(percent / 100)
        if datetime.datetime.now() > timeout:
            return 1

    return 0


def installSnapFromStore():
    subutai.AddStatus("Installing Subutai")
    subutai.log("info", "Installing subutai snap")
    subutai.SSHRun("sudo snap install --beta --devmode subutai-dev")

    return


def initBtrfs():
    subutai.log("info", "Initializing BTRFS")
    subutai.AddStatus("Initializing BTRFS")
    subutai.SSHRun("sudo subutai-dev.btrfsinit /dev/sdb")

    return


def setAlias():
    subutai.log("info", "Setting Alias")
    subutai.SSHRun("sudo bash -c 'snap alias subutai-dev subutai'")

    return


def setupSSH():
    subutai.log("info", "Setting up SSH")
    subutai.SSHRun("mkdir -p /home/subutai/.ssh")
    subutai.InstallSSHKey()

    return


def startVm(machineName):
    subutai.log("info", "Starting virtual machine")
    if subutai.CheckVMRunning(machineName) != 0:
        subutai.VBox("startvm --type headless " + machineName)

    return


def stopVm(machineName):
    subutai.SSHRun("sync")
    subutai.log("info", "Stopping Virtual machine")
    if subutai.CheckVMRunning(machineName) != 0:
        subutai.VBox("controlvm " + machineName + " poweroff soft")

    return


def setupVm(machineName):
    subutai.log("info", "Setting up a VM")
    subutai.AddStatus("Installing VM")
    if subutai.CheckVMExists(machineName) != 0:
        subutai.download("core.ova")
        while subutai.isDownloadComplete() != 1:
            sleep(0.05)
        subutai.VBox("import " +
                     subutai.GetTmpDir().replace(" ", "+++") + "core.ova")
        sleep(10)
        ret = subutai.VBoxS("modifyvm core --name " + machineName)
        if ret != 0:
            return 1

        cpus = subutai.GetCoreNum()
        mem = subutai.GetMemSize() * 1024

        subutai.VBox("modifyvm " + machineName + " --cpus " + str(cpus))
        subutai.VBox("modifyvm " + machineName + " --memory " + str(mem))
        subutai.VBox("modifyvm " + machineName + " --nic1 nat")
        subutai.VBox("modifyvm " + machineName + " --cableconnected1 on")
        subutai.VBox("modifyvm " + machineName + " --natpf1 ssh-fwd,tcp,,4567,,22 --natpf1 https-fwd,tcp,,9999,,8443")
        subutai.VBox("modifyvm " + machineName + " --rtcuseutc on")
        adapterName = subutai.GetVBoxHostOnlyInterface()
        adapterName = adapterName.replace(' ', '+++')
        subutai.VBox("modifyvm " + machineName + " --nic3 hostonly --hostonlyadapter3 " + adapterName)

    return 0


def reconfigureNic(machineName):
    subutai.AddStatus("Configuring Network")
    subutai.log("info", "Reconfiguring NIC")
    gateway = subutai.GetDefaultRoutingInterface()
    bridged = subutai.GetVBoxBridgedInterface(gateway)
    bridged = bridged.replace(' ', '+++')
    subutai.VBox("modifyvm " + machineName + ' --nic1 bridged --bridgeadapter1 "' + bridged + '"')
    subutai.VBox("modifyvm " + machineName + " --nic2 nat")
    subutai.VBox("modifyvm " + machineName + " --cableconnected2 on")
    subutai.VBox("modifyvm " + machineName + ' --natpf2 ssh-fwd,tcp,,4567,,22 --natpf2 https-fwd,tcp,,9999,,8443')

    adapterName = subutai.GetVBoxHostOnlyInterface()
    adapterName = adapterName.replace(' ', '+++')
    ret = subutai.VBoxS("hostonlyif ipconfig " + adapterName + " --ip 192.168.56.1")

    if ret == 1:
        subutai.VBox("hostonlyif create")
        subutai.VBox("hostonlyif ipconfig " + adapterName + " --ip 192.168.56.1")
        subutai.VBox("dhcpserver add --ifname " + adapterName + " --ip 192.168.56.1 --netmask 255.255.255.0 --lowerip 192.168.56.100 --upperip 192.168.56.200")
        subutai.VBox("dhcpserver modify --ifname " + adapterName + " --enable")

    subutai.VBox("modifyvm " + machineName + " --nic3 hostonly --hostonlyadapter3 " + adapterName)

    return


def waitSnap():

    return