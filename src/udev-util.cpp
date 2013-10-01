#include <cor/util.hpp>
#include <cor/udev/util.hpp>

#include <string>
#include <list>
#include <vector>
#include <algorithm>
#include <sstream>

#include <linux/input.h>

namespace cor { namespace udevpp {

bool is_keyboard(Device const &dev)
{
    auto p = dev.attr("capabilities/key");
    if (!p)
        return false;

    std::string key(p);

    std::list<std::string> caps_strs;
    cor::split(key, " ", std::back_inserter(caps_strs));
    std::remove(caps_strs.begin(), caps_strs.end(), "");
    if (caps_strs.empty())
        return false;

    std::vector<unsigned long> caps;
    std::istringstream input;
    input.setf(std::ios_base::hex, std::ios_base::basefield);
    for(auto const &s : caps_strs) {
        unsigned long v;
        bool is_ok = false;
        input.str(s);
        input >> v;
        if (!is_ok)
            return false;
        caps.push_back(v);
    }
    size_t count = 0;
    for (unsigned i = KEY_Q; i <= KEY_P; ++i) {
        int pos = caps.size() - (i / sizeof(unsigned long));
        if (pos < 0)
            break;
        size_t bit = i % sizeof(unsigned long);
        if ((caps[pos] >> bit) & 1)
            ++count;
    }
    return (count == KEY_P - KEY_Q);
}

bool is_keyboard_available()
{
    Root udev;
    if (!udev)
        return false;

    Enumerate input(udev);
    if (!input)
        return false;

    input.subsystem_add("input");
    auto devs = input.devices();

    bool is_kbd_found = false;
    auto find_kbd = [&is_kbd_found, &udev](DeviceInfo const &e) -> bool {
        Device d(udev, e.path());
        is_kbd_found = is_keyboard(d);
        return !is_kbd_found;
    };
    devs.find(find_kbd);
    return is_kbd_found;
}

}}
