#ifndef __CORE_H__
#define __CORE_H__

#include <iostream>
#include <vector>
#include <string>
#include <Python.h>
#include <curl/curl.h>

#include "SL.h"
#include "SLFunctions.h"

class Core {
    public:
        Core(std::vector<std::string> args);
        ~Core();
        void initializePython();
        void run();
        void parseArgs();
    private:
        std::vector<std::string> _args;
    protected:
        void handleTest();
};

#endif
