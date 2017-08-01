#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#if LAUNCHER_LINUX || LAUNCHER_MACOS
#include <unistd.h>
#include <sys/types.h>
#endif

#include "MainWindow.h"
#include "Splash.h"
#include "Poco/Logger.h"
#include <Python.h>

#include "../JuceLibraryCode/JuceHeader.h"
#include "../JuceLibraryCode/modules/juce_gui_extra/juce_gui_extra.h"
#include "../JuceLibraryCode/modules/juce_events/timers/juce_Timer.h"

class InitTimer;

class UIApplication : public juce::JUCEApplication
{
    public:
        UIApplication();
        const juce::String getApplicationName() override;
        const juce::String getApplicationVersion() override;
        bool moreThanOneInstanceAllowed() override;
        void initialise (const juce::String& commandLine) override;
        void shutdown() override;
        void systemRequestedQuit() override;
        void anotherInstanceStarted (const juce::String& commandLine) override;
    private:
        Poco::Logger* _logger;
        juce::ScopedPointer<MainWindow> _mainWindow;
        SubutaiLauncher::Core* _core;

        InitTimer* _initTimer;
        juce::SplashScreen *_splashScreen;

        void checkInitialization();
        void startMainWindow();

        std::thread getAssets();
        void getAssetsImpl();

};

#endif  // MAIN_H_INCLUDED
