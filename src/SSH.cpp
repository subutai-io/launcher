#include "SSH.h"
#define MAX_XFER_BUF_SIZE 16384

const std::string SubutaiLauncher::SSH::BIN = "ssh";

SubutaiLauncher::SSH::SSH() :
    _username("root"),
    _password(""),
    _host("127.0.0.1"),
    _port(22)
{
    _logger = &Poco::Logger::get("subutai");
    _ssh = ssh_new();
    if (_ssh == NULL) {
        throw SubutaiException("Failed to start SSH session");
    }
}

SubutaiLauncher::SSH::~SSH()
{
    _logger->debug("Destructing SSH instance");
    if (_connected) ssh_disconnect(_ssh);
    if (_ssh != NULL) {
        _logger->trace("Freeing SSH object");
        ssh_free(_ssh);
    }
    _logger->debug("SSH instance destructed");
}

void SubutaiLauncher::SSH::disconnect()
{
    if (_connected) {
        _logger->debug("Disconnecting SSH");
        ssh_disconnect(_ssh);
        _connected = false;
    }
}

bool SubutaiLauncher::SSH::findInstallation()
{
    auto env = new Environment();
    SubutaiLauncher::SubutaiString pathVar(env->getVar("PATH", ""));
    std::vector<std::string> path;
    pathVar.split(':', path);
    FileSystem fs;
    for (auto it = path.begin(); it != path.end(); it++) {
        fs.setPath((*it));
        if (fs.isFileExists(BIN)) {
            _installed = true;
            _path = (*it);
            _location = _path;
            _path.append(FileSystem::DELIM);
            _path.append(BIN);
            return true;
        }
    }
    return false;
}

void SubutaiLauncher::SSH::setUsername(const std::string& username, const std::string& password)
{
    _username = username;
    _password = password;
}

void SubutaiLauncher::SSH::setHost(const std::string& host, long port)
{
    _host = host;
    _port = port;
}

/*
std::string SubutaiLauncher::SSH::run(const std::string& command) const
{

    std::vector<std::string> args;
#if LAUNCHER_WINDOWS
    char usr[512];
    char port[64];
    sprintf_s(port, sizeof(port), "%lu", _port);
    sprintf_s(usr, sizeof(usr), "%s@%s", _username.c_str(), _host.c_str());
#else
    char* usr;
    char* port;
    std::sprintf(port, "%lu", _port);
    std::sprintf(usr, "%s@%s", _username.c_str(), _host.c_str());
#endif

    args.push_back("-p");
    args.push_back(port);
    args.push_back(std::string(usr));
    args.push_back(command);

    SubutaiProcess p;
    p.launch(BIN, args, _location);
    p.wait();
    return p.getOutputBuffer();
}
*/

void SubutaiLauncher::SSH::connect()
{
    int rc;
    ssh_options_set(_ssh, SSH_OPTIONS_HOST, _host.c_str());
    ssh_options_set(_ssh, SSH_OPTIONS_PORT, &_port);
    ssh_options_set(_ssh, SSH_OPTIONS_USER, _username.c_str());

    rc = ssh_connect(_ssh);

    _logger->debug("SSH connect status: %d", rc);
    if (rc != SSH_OK) {
        _connected = false;
        _logger->debug("SSH Connection failed");
        return;
    }
    _connected = true;
}

void SubutaiLauncher::SSH::authenticate()
{
    int rc = ssh_userauth_password(_ssh, NULL, _password.c_str());
    _logger->debug("SSH Authentication status: %d", rc);
    if (rc == SSH_AUTH_ERROR) {
        _authenticated = false;
        return;
    }
    _authenticated = true;
}

int SubutaiLauncher::SSH::verifyHost()
{
    int state;
    size_t hlen;
    unsigned char* hash = NULL;
    int gpkh_res;

    state = ssh_is_server_known(_ssh);

//    hlen = ssh_get_pubkey_hash(_ssh, &hash);
    if ((gpkh_res = ssh_get_publickey_hash(_ssh_key, SSH_PUBLICKEY_HASH_SHA1, &hash, &hlen)))
      return gpkh_res;


    if (hlen < 0) {
        return -1;
    }

    switch (state) {
        case SSH_SERVER_KNOWN_OK:
            break;

        case SSH_SERVER_KNOWN_CHANGED:
            break;

        case SSH_SERVER_FOUND_OTHER:
            free(hash);
            return -1;
            break;

        case SSH_SERVER_FILE_NOT_FOUND:

        case SSH_SERVER_NOT_KNOWN:

            if (ssh_write_knownhost(_ssh) < 0) {
                free(hash);
                return -1;
            }
            break;

        case SSH_SERVER_ERROR:
            free(hash);
            return -1;
            break;
    };
    free(hash);
    return 0;
}

std::string SubutaiLauncher::SSH::execute(const std::string& command)
{
    ssh_channel chan;
    int rc;
    char buffer[1024];
    int nbytes;
    memset(buffer, 0, 1024);

    chan = ssh_channel_new(_ssh);
    if (chan == NULL) 
        return "Error: Failed to open channel ";

    rc = ssh_channel_open_session(chan);
    if (rc != SSH_OK) {
        ssh_channel_close(chan);
        ssh_channel_free(chan);
        return "Error: Failed to open SSH session ";
    }

    rc = ssh_channel_request_exec(chan, command.c_str());
    if (rc != SSH_OK) {
        ssh_channel_close(chan);
        ssh_channel_free(chan);
        return "Error: Failed to execute command ";
    }

    nbytes = ssh_channel_read(chan, buffer, sizeof(buffer), 1);

    while (nbytes > 0) {
#if LAUNCHER_WINDOWS
        if (_write(1, buffer, nbytes) != (unsigned int)nbytes) {
#else
            if (fwrite(buffer, 1, nbytes, stdout) != nbytes) {
#endif
                ssh_channel_close(chan);
                ssh_channel_free(chan);
                return "Error: Failed to wtite to channel ";
            }
            nbytes = ssh_channel_read(chan, buffer, sizeof(buffer), 1);
        }

        if (nbytes < 0) {
            ssh_channel_close(chan);
            ssh_channel_free(chan);
            return "Error: Output channel is empty ";
        }

        ssh_channel_send_eof(chan);
        ssh_channel_close(chan);
        ssh_channel_free(chan);
        std::string ret(buffer);
        _logger->debug("SSH Execution output: %s", ret);
        return ret;
    }

    bool SubutaiLauncher::SSH::isConnected()
    {
        return _connected;
    }

    bool SubutaiLauncher::SSH::isAuthenticated()
    {
        return _authenticated;
    }

    std::string SubutaiLauncher::SSH::getPublicKey()
    {
        Environment env;
        auto var = env.getVar("HOME", "~");
        var.append("/.ssh/id_rsa.pub");
        try {
            Poco::FileInputStream fs(var);
            std::string buffer;
            Poco::StreamCopier::copyToString(fs, buffer);
            return buffer;
        } catch (Poco::FileNotFoundException e) {
            Poco::Logger::get("subutai").error("id_rsa.pub not found");
            return "";
        } catch (std::exception e) {
            Poco::Logger::get("subutai").error("Exception: %s", e.what());
            return "";
        }
    }
