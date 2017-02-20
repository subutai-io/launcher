/*
   ==============================================================================

LibraryComponent.cpp
Created: 5 Aug 2016 5:58:23pm
Author:  crioto

==============================================================================
*/

#include "LibraryComponent.h"
// ============================================================================

LibraryItem::LibraryItem(const  std::string& title, const std::string& desc, const std::string& is, const std::string& us, const std::string& rs) : 
    _title(title),
    _desc(desc),
    _installScript(is),
    _updateScript(us),
    _removeScript(rs)
{
    auto l = SubutaiLauncher::Log::instance()->logger();

    if (title != "") 
    {
        auto font = Font(24);
        _titleLabel.setText(_title, dontSendNotification);
        _titleLabel.setColour(Label::textColourId, Colours::white);
        _titleLabel.setBounds(0, 15, WIDTH, 40);
        _titleLabel.setFont(font);
        _titleLabel.setJustificationType(Justification::centred);
        _titleLabel.setInterceptsMouseClicks(false, true);
        addAndMakeVisible(_titleLabel);
    }

    auto fontPlus = Font(72);
    _plusLabel.setText("+", dontSendNotification);
    _plusLabel.setColour(Label::textColourId, Colours::white);
    if (title != "") 
    {
        _plusLabel.setBounds(0, 50, WIDTH, 100);
    } 
    else 
    {
        _plusLabel.setBounds(0, 0, WIDTH, 100);
    }
    _plusLabel.setInterceptsMouseClicks(false, true);
    _plusLabel.setFont(fontPlus);
    _plusLabel.setJustificationType(Justification::centred);
    addAndMakeVisible(_plusLabel);
     bool displayVersion = true;
    // TODO: This approach is a POS. We need to make it more universal
    if (title == "P2P")
    {
        SubutaiLauncher::P2P p2p;
        p2p.findInstallation();
	//l->debug() << "LibraryComponent::constructor p2p is installed: " << p2p.isInstalled() << std::endl;
        //if (p2p.isInstalled()) {
	if (p2p.findInstallation()) {
	    //l->debug() << "LibraryComponent::constructor p2p version: " << p2p.extractVersion() << std::endl;
            _version.setText("Version: " + p2p.extractVersion(), dontSendNotification);
            displayVersion = true;
	    addAndMakeVisible(_version);
        }
    } 
    else if (title == "Tray Client")
    {
        SubutaiLauncher::Tray tray;
        tray.findInstallation();
	l->debug() << "LibraryComponent::constructor tray is installed: " << tray.findInstallation() << std::endl;
        //if (tray.isInstalled()) {
	if (tray.findInstallation()) {
	    //l->debug() << "LibraryComponent::constructor tray version: " << tray.extractVersion() << std::endl;
            _version.setText("Version: " + tray.extractVersion(), dontSendNotification);
            displayVersion = true;
	    addAndMakeVisible(_version);
        }
    }
    else if (title == "Browser Plugin")
    {
	_version.setText("Version: Hello", dontSendNotification);
    }
    else if (title == "VBox")
    {
        SubutaiLauncher::VirtualBox vbox;
        vbox.findInstallation();
	l->debug() << "LibraryComponent::constructor vbox is installed: " << vbox.findInstallation() << std::endl;
        //if (vbox.isInstalled()) {
	if (vbox.findInstallation()) {
	    l->debug() << "LibraryComponent::constructor vbox version: " << vbox.extractVersion() << std::endl;
            _version.setText("Version: " + vbox.extractVersion(), dontSendNotification);
            displayVersion = true;
	    addAndMakeVisible(_version);
        }
    }
    auto verFont = Font(12);
    _version.setInterceptsMouseClicks(false, true);
    _version.setColour(Label::textColourId, Colours::white);
    _version.setBounds(0, HEIGHT-30, WIDTH, 40);
    _version.setFont(verFont);
    _version.setJustificationType(Justification::centredLeft);
    addAndMakeVisible(_version);

    if (displayVersion)
    {
        addAndMakeVisible(_version);
	_version.setVisible(true);
    }

}

LibraryItem::~LibraryItem()
{

}

void LibraryItem::paint(Graphics& g)
{
    g.setColour(Colours::white);
    g.drawRoundedRectangle(0, 0, WIDTH, HEIGHT, 4, 1);
}

void LibraryItem::resized()
{

}

void LibraryItem::mouseUp(const juce::MouseEvent& e)
{
    juce::PopupMenu menu;

    menu.addItem(1, "Install");
    menu.addItem(2, "Update");
    menu.addItem(3, "Remove");    

    const int res = menu.show();
    if (res == 0) {

    } 
    else if (res == 1)
    {
        std::string windowTitle = "Installing ";
        windowTitle.append(_title);
        LibraryActionThread t = new LibraryActionThread("install", _title, windowTitle);
	
	SubutaiLauncher::Log::instance()->logger()->debug() << "LibraryComponent::LibraryItem::mouseUp before Launch thread " <<std::endl;

        //t->launchThread();
	if (!t->runThread()){
	    SubutaiLauncher::Log::instance()->logger()->debug() << "LibraryComponent::LibraryItem::mouseUp cancelled " <<std::endl;
	    return;
	}

	SubutaiLauncher::Log::instance()->logger()->debug() << "LibraryComponent::LibraryItem::mouseUp after launch thread " <<std::endl;

/*
	while (t->isRunning()) {
            sleep(1);
	    //SubutaiLauncher::Log::instance()->logger()->debug() << "LibraryComponent::LibraryItem::mouseUp   thread running " <<std::endl;
        }
*/
	SubutaiLauncher::Log::instance()->logger()->debug() << "LibraryComponent::LibraryItem::mouseUp thread finished " <<std::endl;

    }
    else if (res == 2)
    {
        std::string windowTitle = "Updatiing ";
        windowTitle.append(_title);
        auto t = new LibraryActionThread("update", _title, windowTitle);
        t->launchThread();
        while (t->isRunning()) {
            sleep(1);
        }
	
    } 
    else if (res == 3)
    {
	std::string windowTitle = "Removing ";
        windowTitle.append(_title);
        auto t = new LibraryActionThread("uninstall", _title, windowTitle);
        t->launchThread();
        while (t->isRunning()) {
            sleep(1);
        }
    }
}

// ============================================================================

LibraryComponent::LibraryComponent() : 
    _installButton("Installation Wizard"),
    _nextButton("Next"),
    _backButton("Back"),
    _cancelButton("Cancel"),
    _step(INTRO)
{
    _installButton.addListener(this);
    _nextButton.addListener(this);
    _backButton.addListener(this);
    _cancelButton.addListener(this);
    _systemCheck = new LibrarySystemCheck();
    _systemConfigure = new LibrarySystemConfigure();
    _download = new LibraryDownload();
    addAndMakeVisible(_systemCheck);
    addAndMakeVisible(_systemConfigure);
    addAndMakeVisible(_download);
    addChildComponent(_nextButton);
    addChildComponent(_backButton);
    addChildComponent(_cancelButton);
}

LibraryComponent::~LibraryComponent() {
    delete(_systemCheck);
    delete(_systemConfigure);
    delete(_download);
}

void LibraryComponent::paint(Graphics& g) {
    //g.fillAll (Colour (0xff222222));
    g.fillAll (Colour (0xff333333));
    g.setFont (Font (16.0f));
    g.setColour (Colours::white);

    g.drawLine(10, 80, 800, 80, 1);
    g.drawLine(10, 340, 800, 340, 1);


    //g.drawText ("Library", getLocalBounds(), Justification::centred, true);
}

void LibraryComponent::resized() {
    onStepChange();
    switch(_step) {
        case SYSTEM_CHECK:
            drawSystemCheck();
            break;
        case SYSTEM_CONFIGURE:
            drawSystemConfigure();
            break;
        case DOWNLOAD:
            drawDownload();
            break;
        case PREINSTALL:
            drawPreinstall();
            break;
        case INSTALL:
            drawInstall();
            break;
        case POSTINSTALL:
            drawPostInstall();
            break;
        case FINISHED:
            drawFinished();
            break;
        default:
            drawIntro();
            break;
    };
}

void LibraryComponent::buttonClicked(Button* button) {
    if (button == &_installButton) {
        std::printf("Install button\n");
        _step = SYSTEM_CHECK;
        /*  
            _installButton.setEnabled(false);
            auto dialog = new InstallationDialog("Subutai Installation", Colours::blue, DocumentWindow::allButtons);
            dialog->setBounds(0, 0, 800, 640);
            dialog->centreWithSize(dialog->getWidth(), dialog->getHeight());
            dialog->setTitleBarHeight(0);
            dialog->setVisible(true);
            */
    } else if (button == &_nextButton) {
        nextStep();
    } else if (button == &_backButton) {
        previousStep();
    } else if (button == &_cancelButton) {
        // TODO: Show cancelation dialog
        std::printf("Canceling installation\n");
        _step = INTRO;
    }
    resized();
}

void LibraryComponent::drawIntro() {

    auto font = Font(24);
    _componentsSectionLabel.setText("Subutai Components", dontSendNotification);
    _componentsSectionLabel.setColour(Label::textColourId, Colours::white);
    _componentsSectionLabel.setBounds(15, 50, 300, 40);
    _componentsSectionLabel.setFont(font);
    _componentsSectionLabel.setJustificationType(Justification::top);
    addAndMakeVisible(_componentsSectionLabel);

    _peersSectionLabel.setText("Subutai Peers", dontSendNotification);
    _peersSectionLabel.setColour(Label::textColourId, Colours::white);
    _peersSectionLabel.setBounds(15, 310, 300, 40);
    _peersSectionLabel.setFont(font);
    _peersSectionLabel.setJustificationType(Justification::top);
    addAndMakeVisible(_peersSectionLabel);


  auto l = SubutaiLauncher::Log::instance()->logger();
    auto conf = SubutaiLauncher::Session::instance()->getConfManager();

    auto configs = conf->getConfigs();
    int i = 0;
    for (auto it = configs.begin(); it != configs.end(); it++) {
	std::string bs = (*it).file;
	size_t index = 0;
	std::string is = bs;
	index = bs.find("install");
	bs = bs.replace(index, 7, "update");
	std::string us = bs;
	index = bs.find("update");
	bs = bs.replace(index, 6, "uninstall");
	std::string rs = bs;

        auto c = new LibraryItem((*it).title, (*it).description, is, us, rs);
        c->setBounds(i*220+20, 100, LibraryItem::WIDTH, LibraryItem::HEIGHT);
        addAndMakeVisible(c);
        _components.push_back(c);
        ++i;
    }

    auto p = new LibraryItem("", "");
    p->setBounds(20, 400, LibraryItem::WIDTH, LibraryItem::HEIGHT);
    addAndMakeVisible(p);
    _peers.push_back(p);

    addAndMakeVisible(_installButton);
    _installButton.setVisible(true);
    _installButton.setBounds(600, 20, 150, 35);
    drawProgressButtons(false, false, false);


}

void LibraryComponent::hideIntro() {

    _componentsSectionLabel.setVisible(false);
    _peersSectionLabel.setVisible(false);

    auto conf = SubutaiLauncher::Session::instance()->getConfManager();

    int i = 0;
    auto l = SubutaiLauncher::Log::instance()->logger();
    for (auto c = _components.begin(); c != _components.end(); c++) 
    {
	(**c).setVisible(false);
        ++i;
    }

    for (auto p = _peers.begin(); p != _peers.end(); p++) 
    {
	(**p).setVisible(false);
        ++i;
    }
    
    drawProgressButtons(false, false, false);
    _installButton.setVisible(false);
}


void LibraryComponent::drawSystemCheck() {
    hideIntro();
    drawProgressButtons(true, false, true);
    _systemCheck->setVisible(true);
    _systemCheck->setBounds(20, 20, 1024-250-40, 768 - 60 - 40);
    drawProgressButtons(true, false, true);
    _systemConfigure->setVisible(false);
    _download->setVisible(false);
}

void LibraryComponent::drawSystemConfigure() {
    drawProgressButtons(true, true, true);
    _systemCheck->setVisible(false);
    _download->setVisible(false);
    _systemConfigure->setVisible(true);
    _systemConfigure->setBounds(20, 20, 1024-250-40, 768 - 60 - 40);
    drawProgressButtons(true, true, true);
}


void LibraryComponent::drawDownload() {
#if LAUNCHER_LINUX
    const std::string& file("launcher-linux-install.py_tt");
#elif LAUNCHER_WINDOWS
    const std::string& file("launcher-windows-install.py");
#elif LAUNCHER_MACOS
#error Not Implemented for this platform
#else
#error Unknown Platform
#endif
    _systemCheck->setVisible(false);
    _systemConfigure->setVisible(false);
    _download->setWithPeer(_systemConfigure->isPeerInstallChecked());
    _download->setVisible(true);
    _download->setBounds(20, 20, 1024-250-40, 768 - 60 - 40);
    _download->start();
    drawProgressButtons(false, false, true);
    auto t = waitDownloadComplete();
    t.detach();
}

void LibraryComponent::drawPreinstall() {

}

void LibraryComponent::drawInstall() {

}

void LibraryComponent::drawPostInstall() {

}

void LibraryComponent::drawFinished() {

}

void LibraryComponent::onStepChange() {
    _installButton.setVisible(false);
    _systemCheck->setVisible(false);
    _systemConfigure->setVisible(false);
    _download->setVisible(false);
}

void LibraryComponent::drawProgressButtons(bool next, bool back, bool cancel) {
    _cancelButton.setVisible(true);
    _nextButton.setVisible(true);
    _backButton.setVisible(true);
    if (next) {
        _nextButton.setBounds(660, 700, 100, 30);
        _nextButton.setEnabled(true);
    } else {
        _nextButton.setEnabled(false);
	_nextButton.setVisible(false);
    }
    if (back) {
        _backButton.setBounds(540, 700, 100, 30);
        _backButton.setEnabled(true);
    } else {
        _backButton.setEnabled(false);
	_backButton.setVisible(false);
    }
    if (cancel) {
        _cancelButton.setBounds(420, 700, 100, 30);
        _cancelButton.setEnabled(true);
    } else {
        _cancelButton.setEnabled(false);
        _cancelButton.setVisible(false);
    }
}

void LibraryComponent::nextStep() {
    switch (_step) {
        case SYSTEM_CHECK:
            _step = SYSTEM_CONFIGURE;
            break;
        case SYSTEM_CONFIGURE:
            _step = DOWNLOAD;
            break;
        case DOWNLOAD:
            _step = PREINSTALL;
            break;
        case PREINSTALL:
            _step = INSTALL;
            break;
        case INSTALL:
            _step = POSTINSTALL;
            break;
        case POSTINSTALL:
            _step = FINISHED;
            break;
    }; 
}

void LibraryComponent::previousStep() {
    switch (_step) {
        case FINISHED:
            _step = POSTINSTALL;
            break;
        case POSTINSTALL:
            _step = INSTALL;
            break;
        case INSTALL:
            _step = PREINSTALL;
            break;
        case PREINSTALL:
            _step = DOWNLOAD;
            break;
        case DOWNLOAD:
            _step = SYSTEM_CONFIGURE;
            break;
        case SYSTEM_CONFIGURE:
            _step = SYSTEM_CHECK;
            break;
    };
}

std::thread LibraryComponent::waitDownloadComplete() {
    return std::thread( [=] { waitDownloadCompleteImpl(); } );
}

void LibraryComponent::waitDownloadCompleteImpl() {
    while (!_download->isComplete()) {
#if LAUNCHER_LINUX
        sleep(1);
#elif LAUNCHER_WINDOWS
		Sleep(1000);
#endif
        if (_download->isCanceled()) {
            return;
        }
    }
    drawProgressButtons(true, false, true);	
}

// ============================================================================

LibrarySystemCheck::LibrarySystemCheck() : _numLines(1) 
{
    auto l = SubutaiLauncher::Log::instance()->logger();

    SubutaiLauncher::Environment env;
    
    //Notification note;
    addAndMakeVisible(_osField);
    addAndMakeVisible(_osValue);
    _osValue.setText(env.versionOS(), dontSendNotification);
    addAndMakeVisible(_osHint);
    addLine(&_osField, &_osValue, &_osHint, "OS vesion", "We need version > 16");

    addAndMakeVisible(_if64Field);
    addAndMakeVisible(_if64Value);
    int intBuf = 0;
    //intBuf = env.is64();
    //intBuf = env.osArch();
    //_if64Value.setText(std::to_string(intBuf), dontSendNotification);
    _if64Value.setText(env.cpuArch(), dontSendNotification);
    addAndMakeVisible(_if64Hint);
    addLine(&_if64Field, &_if64Value, &_if64Hint, "Processor architecture", "We need x64 architecture");

    addAndMakeVisible(_numCpuField);
    addAndMakeVisible(_numCpuValue);
    intBuf = 0;
    //intBuf = env.processorNum();
    intBuf = env.cpuNum();
    _numCpuValue.setText(std::to_string(intBuf), dontSendNotification);
    addAndMakeVisible(_numCpuHint);
    addLine(&_numCpuField, &_numCpuValue, &_numCpuHint, "Number of CPU Cores", "Each peer requires at least 2 CPU cores");

    addAndMakeVisible(_maxMemoryField);
    addAndMakeVisible(_maxMemoryValue);
    addAndMakeVisible(_maxMemoryHint);
    intBuf = env.ramSize();
    _maxMemoryValue.setText(std::to_string(intBuf), dontSendNotification);
    addLine(&_maxMemoryField, &_maxMemoryValue, &_maxMemoryHint, "Total System Memory", "4GB will be taken to each peer's virtual machine");

    addAndMakeVisible(_vtxField);
    addAndMakeVisible(_vtxValue);
    addAndMakeVisible(_vtxHint);
    _vtxValue.setText(env.vtxEnabled(), dontSendNotification);
    addLine(&_vtxField, &_vtxValue, &_vtxHint, "Hardware Virtualization Support", "VTx should be supported by your CPU");

    addAndMakeVisible(_vboxField);
    addAndMakeVisible(_vboxValue);
    addAndMakeVisible(_vboxHint);
    intBuf = env.versionVBox();
    _vboxValue.setText(std::to_string(intBuf), dontSendNotification);
    addLine(&_vboxField, &_vboxValue, &_vboxHint, "Oracle VirtualBox version", "We're running our peer on VirtualBox VMs");

/*
    addAndMakeVisible(_sshField);
    addAndMakeVisible(_sshValue);
    addAndMakeVisible(_sshHint);
    _sshValue.setText( env.versionSSH(), dontSendNotification);
    addLine(&_sshField, &_sshValue, &_sshHint, "SSH client version", "SSH client is used to configure peer during installation");
*/

    /* 
       _info.setText("ing...", dontSendNotification);
       _info.setColour(Label::textColourId, Colours::white);
       _info.setBounds(00, 0, 800, 440);
       _info.setFont(font);
       _info.setJustificationType(Justification::top);
       */
}

LibrarySystemCheck::~LibrarySystemCheck() {

}

void LibrarySystemCheck::addLine(Label* field, Label* value, Label* hint, std::string text, std::string hintText) {
    auto font = Font(16);
    field->setText(text, dontSendNotification);
    field->setColour(Label::textColourId, Colours::white);
    field->setBounds(0, _numLines * 50, 300, 50);
    field->setFont(font);
    field->setJustificationType(Justification::top);

    //value->setText("Checking...", dontSendNotification);
    value->setColour(Label::textColourId, Colours::white);
    value->setBounds(320, _numLines * 50, 800, 50);
    value->setFont(font);
    value->setJustificationType(Justification::top);

    hint->setText(hintText, dontSendNotification);
    hint->setColour(Label::textColourId, Colours::grey);
    hint->setBounds(320, _numLines * 50 + 20, 800, 50);
    hint->setFont(font);
    hint->setJustificationType(Justification::top);

    _numLines++;
}

void LibrarySystemCheck::paint(Graphics& g) {
    g.fillAll (Colour (0xff222222));
    g.setFont (Font (16.0f));
    g.setColour (Colours::white);
}

void LibrarySystemCheck::resized() {
}

// ============================================================================

LibrarySystemConfigure::LibrarySystemConfigure() {

    _installVmField.setText("Choose Installation Configuration", dontSendNotification);
    _installVmField.setColour(Label::textColourId, Colours::white);
    _installVmField.setBounds(0, 5, 500, 440);
    _installVmField.setJustificationType(Justification::top);

    auto conf = SubutaiLauncher::Session::instance()->getConfManager();

    auto configs = conf->getConfigs();
    int i = 0;
    for (auto it = configs.begin(); it != configs.end(); it++) {
        auto b = new ToggleButton((*it).title);
        b->setRadioGroupId(11);
        b->setBounds(300, i*30, 200, 30);
        b->setColour(ToggleButton::textColourId, Colours::white);
        if (i == 0) {
            b->triggerClick();
        }
        addAndMakeVisible(b);
        _installTypes.push_back(b);
        ++i;
    }

    /* 
       _installTray = new ToggleButton("Install only tray application");
       _installTray->setRadioGroupId(11);
       _installTray->setBounds(300, 0, 200, 30);
       _installTray->setColour(ToggleButton::textColourId, Colours::white);
       _installTray->triggerClick();
       _installVm = new ToggleButton("Install tray and setup a new peer");
       _installVm->setRadioGroupId(11);
       _installVm->setBounds(300, 30, 200, 30);
       _installVm->setColour(ToggleButton::textColourId, Colours::white);

    // Version

    _installTypeField.setText("Choose Version", dontSendNotification);
    _installTypeField.setColour(Label::textColourId, Colours::white);
    _installTypeField.setBounds(0, 65, 500, 440);
    _installTypeField.setJustificationType(Justification::top);



    _installMaster = new ToggleButton("Install latest stable release");
    _installMaster->setRadioGroupId(22);
    _installMaster->setBounds(300, 60, 400, 30);
    _installMaster->setColour(ToggleButton::textColourId, Colours::white);
    _installMaster->triggerClick();
    _installDev = new ToggleButton("Install current development snapshot");
    _installDev->setRadioGroupId(22);
    _installDev->setBounds(300, 90, 400, 30);
    _installDev->setColour(ToggleButton::textColourId, Colours::white);

    // Path

    _installPathField.setText("Installation path", dontSendNotification);
    _installPathField.setColour(Label::textColourId, Colours::white);
    _installPathField.setBounds(0, 140, 500, 440);
    _installPathField.setJustificationType(Justification::top);

    _installPathValue.setText("/opt/subutai");
    _installPathValue.setBounds(300, 140, 200, 25);

    // Tmp
    //
    _installTmpField.setText("Temporary directory", dontSendNotification);
    _installTmpField.setColour(Label::textColourId, Colours::white);
    _installTmpField.setBounds(0, 180, 500, 440);
    _installTmpField.setJustificationType(Justification::top);

    _installTmpValue.setText("/tmp");
    _installTmpValue.setBounds(300, 180, 200, 25);

    addAndMakeVisible(_installTypeField);
    addAndMakeVisible(_installMaster);
    addAndMakeVisible(_installDev);
    addAndMakeVisible(_installVmField);
    addAndMakeVisible(_installVm);
    addAndMakeVisible(_installTray);
    addAndMakeVisible(_installPathField);
    addAndMakeVisible(_installPathValue);
    addAndMakeVisible(_installTmpField);
    addAndMakeVisible(_installTmpValue);
    */
}

LibrarySystemConfigure::~LibrarySystemConfigure() {
    delete(_installTray);
    delete(_installVm);
    for (auto it = _installTypes.begin(); it != _installTypes.end(); it++) {
        delete (*it);
    }
}

void LibrarySystemConfigure::paint(Graphics& g) {
    g.fillAll (Colour (0xff222222));
    g.setFont (Font (16.0f));
    g.setColour (Colours::white);
}

void LibrarySystemConfigure::resized() {

}

bool LibrarySystemConfigure::isPeerInstallChecked() {
    return _installVm->getToggleState();
}

// ============================================================================

LibraryDownload::LibraryDownload(bool downloadPeer) {
    _withPeer = downloadPeer;
    _progressBar = new Slider();
    addAndMakeVisible(_progressBar);

    _progressBar->setRange(0.0, 100.0, 0.1);
    _progressBar->setValue(0.0);
    _progressBar->setSliderStyle(Slider::LinearBar);
    _progressBar->setBounds(0, 0, 1024-250-40, 40);

    _currentAction.setText("Calculating download size", dontSendNotification);
    _currentAction.setColour(Label::textColourId, Colours::white);
    _currentAction.setBounds(0, 40, 500, 30);
    _currentAction.setJustificationType(Justification::left);
    addAndMakeVisible(_currentAction);

    _sizeProgress.setText("", dontSendNotification);
    _sizeProgress.setColour(Label::textColourId, Colours::white);
    _sizeProgress.setBounds(500, 40, 1024-250-40-500, 30);
    _sizeProgress.setJustificationType(Justification::right);
    addAndMakeVisible(_sizeProgress);

    _downloader = SubutaiLauncher::Session::instance()->getDownloader();
    _inProgress = false;
    _isDownloadComplete = false;
    _isCanceled = false;

    _fileList.push_back("p2p");
    _fileList.push_back("SubutaiTray");
    _fileList.push_back("SubutaiTray_libs.tar.gz");
    _fileList.push_back("setup-tray.py");
    if (_withPeer) {
        _fileList.push_back("snappy.ova");
        _fileList.push_back("setup-peer.py");
    }
}

LibraryDownload::~LibraryDownload() {
    delete(_progressBar);
}

void LibraryDownload::paint(Graphics& g) {

}

void LibraryDownload::start() {
    _isCanceled = false;
    auto d = download();
    d.detach();
}

std::thread LibraryDownload::download() {
    return std::thread( [=] { downloadImpl(); } );
}

void LibraryDownload::downloadImpl() {
    if (!_inProgress) {
        _inProgress = true;
    } else {
        return;
    }
    auto total = calculateTotalSize();
    std::printf("Total download size: %lu\n", total);

    _progress = 0;
    _isDownloadComplete = false;
    for (auto it = _fileList.begin(); it != _fileList.end(); it++) {
        _currentAction.setText((*it), dontSendNotification);
        _downloader->setFilename((*it));
        auto t = _downloader->download();
        t.detach();
        while (!_downloader->isDone()) {
            updateProgress(_downloader->currentProgress());
#if LAUNCHER_LINUX
            usleep(1000);
#elif LAUNCHER_WINDOWS
	    Sleep(1000);
#elif LAUNCHER_MACOS
#error Not Implemented for this platform
#else
#error Unknown Platform
#endif
        }
        _progress += _downloader->currentProgress();
        _downloader->reset();
    }
    _currentAction.setText("Finalizing...", dontSendNotification);
    _sizeProgress.setText("Done", dontSendNotification);
    _progressBar->setValue(100.0);
    _inProgress = false;
    _isDownloadComplete = true;
}

void LibraryDownload::updateProgress(long p) {
    if (_progress + p > _totalSize) {
        _progressBar->setValue(100.0);
        return;
    }
    char val[500];
    std::sprintf(val, "%ld / %ld", _progress + p, _totalSize);
    _sizeProgress.setText(std::string(val), dontSendNotification);
    _progressBar->setValue((float)((_progress + p)/(_totalSize/100)));
    _progressBar->setVisible(true);
    resized();
}

void LibraryDownload::resized() {
    _currentAction.setBounds(0, 40, 500, 30);
    _sizeProgress.setBounds(500, 40, 1024-250-40-500, 30);
}

long LibraryDownload::calculateTotalSize() {
    _totalSize = 0;
    for (auto it = _fileList.begin(); it != _fileList.end(); it++) {
        _downloader->reset();
        _downloader->setFilename((*it));
        auto s = _downloader->info().size;
        _totalSize += s;
    }
    _downloader->reset();
    return _totalSize;
}

bool LibraryDownload::isComplete() {
    return _isDownloadComplete;
}

bool LibraryDownload::isCanceled() {
    return _isCanceled;
}

void LibraryDownload::setWithPeer(bool withPeer) {
    _withPeer = withPeer;
}
