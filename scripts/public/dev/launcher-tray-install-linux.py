import subutai
import zipfile
from time import sleep

def subutaistart():
    subutai.AddStatus("Download Tray application")

    tray = "SubutaiTray_libs.zip"

    subutai.download(tray)
    while subutai.isDownloadComplete() != 1:
        sleep(0.05)

    tmpDir = subutai.GetTmpDir()
    installDir = subutai.GetInstallDir()

    subutai.AddStatus("Unpacking Tray")

    zf = zipfile.ZipFile(tmpDir+"/"+tray, 'r')
    zf.extractall(insatllDir+"/bin/")
    zf.close()

    subutai.AddStatus("Creating Symlink")
    subutai.MakeLink(installDir+"/bin/SubutaiTray", "/usr/local/bin/SubutaiTray")

    subutai.Shutdown()
