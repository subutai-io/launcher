#include "VirtualBox.h"

namespace SubutaiLauncher {

    const std::string VirtualBox::BIN = "vboxmanage";

    VirtualBox::VirtualBox() : _version("") {
        auto env = new Environment();
        String str(env->getVar("PATH", ""));
        std::vector<std::string> path;
        str.split(':', path);
        FileSystem fs;
        for (auto it = path.begin(); it != path.end(); it++) {
            fs.setPath((*it));
            if (fs.isFileExists(BIN)) {
                _installed = true;
                _path = (*it);
                _location = _path;
                _path.append(FileSystem::DELIM);
                _path.append(BIN);
                break;
            }
        }
    }

    VirtualBox::~VirtualBox() {

    }

    bool VirtualBox::isInstalled() {
        return _installed;
    }

    bool VirtualBox::isRunning() {
        return true;
    }

    bool VirtualBox::isUpdateRequired() {
        return false;
    }

    std::string VirtualBox::retrieveVersion() {
        if (_version != "") {
            return _version;
        }

    }

    void VirtualBox::getVms()
    {
        std::vector<std::string> args;
        args.push_back("list");
        args.push_back("vms");
        Process p;
        p.launch(BIN, args, _location);
        p.wait();
        auto out = p.getOutputBuffer();
    }

    std::vector<SubutaiVM> VirtualBox::parseVms(const std::string& buffer)
    {
        char vmname[150];
        char vmid[150];
        // Parsing output of vboxmanage:
        // "machine_name" {machine_id}
        std::vector<SubutaiVM> vms;
        String buf(buffer);
        std::vector<std::string> lines;
        buf.split('\n', lines);
        for (auto it = lines.begin(); it != lines.end(); it++) {
            const char* line = const_cast<char*>((*it).c_str());
            sscanf(line, "\"%s\" {%s}", vmname, vmid);
            SubutaiVM v;
            v.name = std::string(vmname);
            v.id = std::string(vmid);
            vms.push_back(v);
        }
        return vms;
    }

};
