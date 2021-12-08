#ifndef LINKRBRAIN2019__SRC__LINKRBRAIN__SOCKET__ACTION_HPP
#define LINKRBRAIN2019__SRC__LINKRBRAIN__SOCKET__ACTION_HPP


namespace LinkRbrain::Socket {


    enum Action {
        None = 0x00,
        Status = 0x10,
        Stop = 0x20,
        Start = 0x30,
        Restart = 0x40,
    };

    const Action get_action_from_name(std::string action_name) {
        // put name to lower case
        std::transform(action_name.begin(), action_name.end(), action_name.begin(),
            [](unsigned char c){ return std::tolower(c); });
        // check name
        if (action_name == "status") {
            return Status;
        } else if (action_name == "stop") {
            return Stop;
        } else if (action_name == "start") {
            return Start;
        } else if (action_name == "restart") {
            return Restart;
        } else {
            return None;
        }
    }

    const std::string get_action_name(const Action action) {
        switch (action) {
            case None:
                return "None";
            case Status:
                return "Status";
            case Stop:
                return "Stop";
            case Start:
                return "Start";
            case Restart:
                return "Restart";
            default:
                return "(unknown)";
        }
    }


} // LinkRbrain::Socket


#endif // LINKRBRAIN2019__SRC__LINKRBRAIN__SOCKET__ACTION_HPP
